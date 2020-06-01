#include <epworkercntl/epprocess-win.h>

DWORD WINAPI process_control_win::run_control_thread(void* vptr)
{
    auto* wcntl = static_cast<process_control_win*>(vptr);

    wcntl->listen_loop();

    return 0;
}

bool process_control_win::is_running()
{
    bool result = false;
    auto res    = WaitForSingleObject(server_handle, 0);

    switch (res)
    {
    case WAIT_OBJECT_0:
        break;

    case WAIT_FAILED:
        break;

    case WAIT_TIMEOUT:
        result = true;
        break;
    }

    return result;
}

void process_control_win::start()
{
    // Control server
    server_handle = CreateThread(
        NULL,               // default security attributes
        0,                  // use default stack size
        run_control_thread, // thread function name
        this,               // argument to thread function
        0,                  // use default creation flags
        &server_thread_id);
}

void process_control_win::stop()
{
}

void process_control_win::setup()
{
    // The initial loop creates several instances of a named pipe
    // along with an event object for each instance.  An
    // overlapped ConnectNamedPipe operation is started for
    // each instance.

    for (int i = 0; i < INSTANCES; i++)
    {
        // Create an event object for this instance.

        // read event
        hEvents[i] = CreateEvent(
            NULL,  // default security attribute
            TRUE,  // manual-reset event
            TRUE,  // initial state = signaled
            NULL); // unnamed event object

        // write event
        hEvents[i + INSTANCES] = CreateEvent(
            NULL,  // default security attribute
            TRUE,  // manual-reset event
            TRUE,  // initial state = signaled
            NULL); // unnamed event object

        if (hEvents[i + INSTANCES] == NULL)
        {
            spdlog::debug("CreateEvent failed with {}.\n", GetLastError());

            // TODO: KP throw exception
            return;
        }

        if (hEvents[i] == NULL)
        {
            spdlog::debug("CreateEvent failed with {}.\n", GetLastError());

            // TODO: KP throw exception
            return;
        }

        woverlapped[i].hEvent             = hEvents[i];
        woverlapped[i + INSTANCES].hEvent = hEvents[i + INSTANCES];

        Pipe[i] = CreateNamedPipeA(
            pipe_name.c_str(),          // pipe name
            PIPE_ACCESS_DUPLEX |        // read/write access
                FILE_FLAG_OVERLAPPED,   // overlapped mode
            PIPE_TYPE_MESSAGE |         // message-type pipe
                PIPE_READMODE_MESSAGE | // message-read mode
                PIPE_WAIT,              // blocking mode
            INSTANCES,                  // number of instances
            8192 * sizeof(TCHAR),       // output buffer size
            8192 * sizeof(TCHAR),       // input buffer size
            PIPE_TIMEOUT,               // client time-out
            NULL);                      // default security attributes

        if (Pipe[i] == INVALID_HANDLE_VALUE)
        {
            spdlog::debug("CreateNamedPipe failed with {}.\n", GetLastError());
            // TODO: KP throw exception
            return;
        }

        // Call the subroutine to connect to the new client

        connect_to_new_client(Pipe[i], &woverlapped[i]);
        connect_to_new_client(Pipe[i], &woverlapped[i + INSTANCES]);
    }
}

void process_control_win::listen_loop()
{
    DWORD i, dwWait, cbRet, dwErr;
    BOOL  fSuccess;

    while (1)
    {
        // Wait for the event object to be signaled, indicating
        // completion of an overlapped read, write, or
        // connect operation.

        dwWait = WaitForMultipleObjects(
            INSTANCES * 2, // number of event objects
            hEvents,       // array of event objects
            FALSE,         // does not wait for all
            INFINITE);     // waits indefinitely

        // dwWait shows which pipe completed the operation.

        i = dwWait - WAIT_OBJECT_0; // determines which pipe
        if (i < 0 || i > (INSTANCES * 2 - 1))
        {
            spdlog::debug("Index out of range.\n");
            return;
        }

        int pipe_id = i - INSTANCES;

        auto res1 = GetOverlappedResult(
            Pipe[pipe_id],   // handle to pipe
            &woverlapped[i], // OVERLAPPED structure
            &cbRet,          // bytes transferred
            FALSE);
        if (!res1)
        {
            spdlog::debug("ERR: GetOverlappedResult(): {}", geterror_to_string());
        }

        if (i < INSTANCES)
        {
            DWORD             cbRead;
            std::vector<char> input_buffer(50000);

            // Read
            fSuccess = ReadFile(Pipe[i], input_buffer.data(), input_buffer.size(), &cbRead, &woverlapped[i]);

            // The read operation is still pending.

            dwErr = GetLastError();
            if (!fSuccess && (dwErr == ERROR_IO_PENDING))
            {
                ;
            }
            else
            {
                // An error occurred; disconnect from the client.
                DisconnectAndReconnect(i);
            }
        }
        else
        {
            bool write = false;
            if (res1)
            {
                if (pending_write[pipe_id] == nullptr)
                {
                    // Nothing pending
                    write = true;
                }
                else
                {
                    if (pending_write[pipe_id]->size() == cbRet)
                    {
                        // The write completed
                        pending_write[pipe_id] = nullptr;
                        write                  = true;
                    }
                    else
                    {
                        // Not done writing
                        ;
                    }
                }
            }

            if (write)
            {
                if (write_queue[pipe_id].empty())
                {
                    SleepEx(100, true);
                }
                else
                {
                    // Write
                    pending_write[pipe_id] = std::move(write_queue[pipe_id].front());
                    write_queue[pipe_id].pop();

                    auto& b = pending_write[pipe_id];

                    fSuccess = WriteFile(Pipe[pipe_id], b->data(), b->size(), &cbRet, &woverlapped[i]);

                    // The write operation is still pending.

                    dwErr = GetLastError();
                    if (!fSuccess && (dwErr == ERROR_IO_PENDING))
                    {
                        ;
                    }
                    else
                    {
                        // An error occurred; disconnect from the client.
                        spdlog::debug("Error writing to file. {}", geterror_to_string());

                        DisconnectAndReconnect(i);
                    }
                }
            }
        }
    }
}

void process_control_win::send(int i, control_worker::process_message_base& obj)
{
    BOOL fSuccess = FALSE;

    auto buff = process_message_base::pack(obj);

    write_queue[i].push(std::move(buff));
}

void process_control_win::DisconnectAndReconnect(DWORD i)
{
    // Disconnect the pipe instance.

    if (!DisconnectNamedPipe(Pipe[i]))
    {
        spdlog::debug("DisconnectNamedPipe failed with {}.\n", GetLastError());
    }

    // Call a subroutine to connect to the new client.

    connect_to_new_client(Pipe[i], &woverlapped[i]);
    connect_to_new_client(Pipe[i], &woverlapped[i + INSTANCES]);
}

BOOL process_control_win::connect_to_new_client(HANDLE hPipe, LPOVERLAPPED lpo)
{
    BOOL fConnected, fPendingIO = FALSE;

    // Start an overlapped connection for this pipe instance.
    fConnected = ConnectNamedPipe(hPipe, lpo);

    // Overlapped ConnectNamedPipe should return zero.
    if (fConnected)
    {
        spdlog::debug("ConnectNamedPipe failed with {}.\n", GetLastError());
        return 0;
    }

    switch (GetLastError())
    {
        // The overlapped connection in progress.
    case ERROR_IO_PENDING:
        fPendingIO = TRUE;
        break;

        // Client is already connected, so signal an event.

    case ERROR_PIPE_CONNECTED:
        if (SetEvent(lpo->hEvent))
            break;

        // If an error occurs during the connect operation...
    default:
    {
        spdlog::debug("ConnectNamedPipe failed with {}.\n", GetLastError());
        return 0;
    }
    }

    return fPendingIO;
}
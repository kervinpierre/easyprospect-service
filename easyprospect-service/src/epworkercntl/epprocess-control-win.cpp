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
            FALSE,  // initial state = signaled
            NULL); // unnamed event object

        // write event
        hEvents[i + INSTANCES] = CreateEvent(
            NULL,  // default security attribute
            TRUE,  // manual-reset event
            FALSE,  // initial state = signaled
            NULL); // unnamed event object

        if (hEvents[i + INSTANCES] == NULL)
        {
            spdlog::debug("CreateEvent failed with {}.\n", GetLastError());

            throw std::logic_error("CreateEvent failed");
        }

        if (hEvents[i] == NULL)
        {
            spdlog::debug("CreateEvent failed with {}.\n", GetLastError());

            throw std::logic_error("CreateEvent failed");
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
            throw std::logic_error("CreateNamedPipe failed");
        }

        // Call the subroutine to connect to the new client

        connect_to_new_client(Pipe[i], &woverlapped[i]);
        connect_to_new_client(Pipe[i], &woverlapped[i + INSTANCES]);
    }
}

void process_control_win::listen_loop()
{
    DWORD ovlp_id_, dwWait, cbRet, dwErr;
    BOOL  fSuccess;

    std::vector<unsigned char> input_buffer(50000);
    DWORD             read_bytes;
    bool              stop    = false;
    
    bool pipe_read_ready[INSTANCES] = {false};
    bool pipe_write_ready[INSTANCES] = {false};
    bool read_pending[INSTANCES]     = {false};
    bool write_pending[INSTANCES]    = {false};

    // Only for debugging
    int64_t debug_read_count = 0;
    int64_t debug_write_count = 0;

    do
    {
        for( auto i=0; i<INSTANCES; i++ )
        {
            if(pipe_read_ready[i])
            {
                spdlog::trace("Read pipe : {}, pipe_id {}", debug_read_count++, i);

                read_pending[i] = true;
                pipe_read_ready[i] = false;

                // Read
                DWORD read_res = ReadFile(
                    Pipe[i], input_buffer.data(), input_buffer.size(), &read_bytes, &woverlapped[i]);

                // The read operation is still pending.

                DWORD read_err = GetLastError();
                if (!read_res)
                {
                    switch (read_err)
                    {
                    case ERROR_IO_PENDING:
                        spdlog::trace("Read pipe IO_PENDING");
                        break;

                    default:
                        spdlog::error("ReadFile() Error: {}", geterror_to_string(true, read_err));
                        throw std::logic_error("ReadFile() error");
                        break;
                    }
                }
                else
                {
                    spdlog::trace("ReadFile returned non-zero");
                };
            }
        }

        for (auto i = 0; i < INSTANCES; i++)
        {
            if (pipe_write_ready[i])
            {
                spdlog::trace("Write pipe : {}", debug_write_count++);

                if (write_queue[i].empty())
                {
                    if (!ResetEvent(&(woverlapped[i+INSTANCES].hEvent)))
                    {
                        spdlog::error("ResetEvent failed. {}", geterror_to_string());
                    }
                    SleepEx(100, true);
                }
                else
                {
                    // Write
                    pending_write[i] = std::move(write_queue[i].front());
                    write_queue[i].pop();

                    auto& b = pending_write[i];

                    fSuccess = WriteFile(Pipe[i], b->data(), b->size(), &cbRet, &woverlapped[i+INSTANCES]);

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

                        DisconnectAndReconnect(ovlp_id_);
                    }
                }
            }
        }

        for( auto i = 0; i<INSTANCES; i++ )
        {
            // Read pending
            if (read_pending[i])
            {
                read_pending[i] = false;

                auto gor_res = GetOverlappedResult(Pipe[i], &woverlapped[i], &read_bytes, false);
                auto gor_err = GetLastError();
                if (!gor_res)
                {
                    switch (gor_err)
                    {
                    case ERROR_HANDLE_EOF:
                        spdlog::debug("EOF when reading");
                        break;

                    case ERROR_IO_INCOMPLETE:
                        //read_pending[i] = true;
                        spdlog::debug("IO_INCOMPLETE {}", i);
                        if(!ResetEvent(&(woverlapped[i].hEvent)))
                        {
                            spdlog::error("ResetEvent failed. {}", geterror_to_string());
                        }
                        break;

                    default:
                    {
                        spdlog::debug("GetOverlappedResult() error");
                    }
                    }
                }
                else
                {
                    // Successful completion
                    std::string o(input_buffer.begin(), input_buffer.end());
                    if (read_bytes < buffsize)
                    {
                        o.resize(read_bytes);
                    }

                    spdlog::debug("client read ended : '{}'", o);

                    if (read_bytes > 0)
                    {
                        msgpack::object_handle oh = msgpack::unpack(o.data(), o.size());

                        msgpack::object deserialized = oh.get();

                        std::stringstream os;

                        os << deserialized << std::endl;

                        spdlog::debug("msgpack: '{}'", os.str());
                    }
                }
            }
        }

        // Wait for the event object to be signaled, indicating
        // completion of an overlapped read, write, or
        // connect operation.

        dwWait = WaitForMultipleObjects(
            INSTANCES * 2, // number of event objects
            hEvents,       // array of event objects
            FALSE,         // does not wait for all
            5000);     // waits indefinitely

        switch (dwWait)
        {
        case WAIT_TIMEOUT:
            spdlog::trace("listen_loop(): WAIT_TIMEOUT");
            break;

        case WAIT_FAILED:
        {
            std::stringstream err;
            err << "Wait for process failed:\n" << geterror_to_string();
            spdlog::error(err.str());
            stop = true;
        }
        break;

        default:
            {
                ovlp_id_ = dwWait - WAIT_OBJECT_0; // determines which pipe
                spdlog::trace("Event signalled: {}", ovlp_id_);

                if (ovlp_id_ < 0 || ovlp_id_ > (INSTANCES * 2 - 1))
                {
                    spdlog::debug("Index out of range.\n");
                    return;
                }

                if (ovlp_id_ < INSTANCES)
                {
                    pipe_read_ready[ovlp_id_] = true;
                }
                else
                {
                    pipe_write_ready[ovlp_id_-INSTANCES] = true;
                }
            }
            break;
        }
    } while (!stop);
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
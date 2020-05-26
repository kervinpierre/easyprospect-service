#include <epworkercntl/epprocess-win.h>
#include <sstream>

bool process_win::is_running() const
{
    bool result = false;
    auto res    = WaitForSingleObject(proc_thread_handle, 0);

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

void process_win::setup()
{
    ;
}

DWORD WINAPI process_win::run_process_thread(void* vptr)
{
    auto* wcntl = static_cast<process_win*>(vptr);

    wcntl->listen_loop();

    return 0;
}

void process_win::start()
{
    proc_thread_handle = CreateThread(
        NULL,               // default security attributes
        0,                  // use default stack size
        run_process_thread, // thread function name
        this,               // argument to thread function
        0,                  // use default creation flags
        &proc_thread_id);
}

void process_win::listen_loop()
{
    STARTUPINFOA        si;
    SECURITY_ATTRIBUTES sa;

    // https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output?redirectedfrom=MSDN

    sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle       = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&output_handles_a[0], &output_handles_b[0], &sa, 0))
    {
        spdlog::error("Error creating pipe for stdout");
        throw std::logic_error("Pipe error");
    }

    if (!CreatePipe(&output_handles_a[1], &output_handles_b[1], &sa, 0))
    {
        spdlog::error("Error creating pipe for stderr");
        throw std::logic_error("Pipe error");
    }

    if (!CreatePipe(&input_handles[0], &input_handles[1], &sa, 0))
    {
        spdlog::error("Error creating pipe for stdin");
        throw std::logic_error("Pipe error");
    }

    if (!SetHandleInformation(output_handles_a[0], HANDLE_FLAG_INHERIT, 0))
    {
        spdlog::error("Error setting flag on stdout: {}", geterror_to_string());
        throw std::logic_error("Stdout error");
    }

    if (!SetHandleInformation(output_handles_a[1], HANDLE_FLAG_INHERIT, 0))
    {
        spdlog::error("Error setting flag on sterr");
        throw std::logic_error("Stderr error");
    }

    if (!SetHandleInformation(input_handles[1], HANDLE_FLAG_INHERIT, 0))
    {
        spdlog::error("Error setting flag on stdin");
        throw std::logic_error("Stdin error");
    }

    output_ovlp[0]        = {0};
    output_ovlp[1]        = {0};
    output_ovlp[2]        = {0};
    io_events[0]          = CreateEvent(NULL, TRUE, FALSE, NULL);
    io_events[1]          = CreateEvent(NULL, TRUE, FALSE, NULL);
    io_events[2]          = CreateEvent(NULL, TRUE, FALSE, NULL);
    output_ovlp[0].hEvent = io_events[0];
    output_ovlp[1].hEvent = io_events[1];
    output_ovlp[2].hEvent = io_events[2];

    ZeroMemory(&si, sizeof(si));
    si.cb         = sizeof(si);
    si.hStdOutput = output_handles_b[0];
    si.hStdError  = output_handles_b[1];
    si.hStdInput  = input_handles[1];
    si.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&pi, sizeof(pi));

    std::vector<char> cmd(command_line.begin(), command_line.end());
    cmd.push_back(0);

    spdlog::debug("Command line:\n'{}'", command_line);

    // Start the child process.
    if (!CreateProcessA(
            NULL,             // No module name (use command line)
            &cmd[0],          // Command line
            NULL,             // Process handle not inheritable
            NULL,             // Thread handle not inheritable
            TRUE,             // Set handle inheritance
            CREATE_NO_WINDOW, // No creation flags
            NULL,             // Use parent's environment block
            NULL,             // Use parent's starting directory
            &si,              // Pointer to STARTUPINFO structure
            &pi)              // Pointer to PROCESS_INFORMATION structure
    )
    {
        spdlog::debug("CreateProcess failed ({}).\n", GetLastError());
        return;
    }

    bool stop    = false;
    io_events[3] = pi.hProcess;
    std::vector<char> stdout_buff(buffsize);
    std::vector<char> stderr_buff(buffsize);
    DWORD             stdout_bytes;
    DWORD             stderr_bytes;

    bool stdout_pending = false;
    bool stderr_pending = false;

    bool stdout_process = true;
    bool stderr_process = true;

    do
    {
        if (stdout_process && !stdout_pending)
        {
            stdout_pending = true;
            stdout_process = false;

            bool so_read =
                ReadFile(output_handles_a[0], stdout_buff.data(), stdout_buff.size(), &stdout_bytes, &output_ovlp[0]);
            auto read_res = GetLastError();
            if (!so_read)
            {
                switch (read_res)
                {
                case ERROR_IO_PENDING:
                    break;

                default:
                    spdlog::error("ReadFile() Error: {}", geterror_to_string(true, read_res));
                    throw std::logic_error("ReadFile() error");
                    break;
                }
            }
        }

        if (stdout_pending)
        {
            stdout_pending = false;

            auto gor_res = GetOverlappedResult(output_handles_a[0], &output_ovlp[0], &stdout_bytes, false);
            auto gor_err = GetLastError();
            if (!gor_res)
            {
                switch (gor_err)
                {
                case ERROR_HANDLE_EOF:
                    spdlog::debug("EOF when reading");
                    break;

                case ERROR_IO_INCOMPLETE:
                    stdout_pending = true;
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
                std::string o(stdout_buff.begin(), stdout_buff.end());
                if ( stdout_bytes < buffsize )
                {
                    o.resize(stdout_bytes);
                }

                spdlog::debug("stdout read ended : '{}'", o);
            }
        }

        // auto se_read =
        //    ReadFile(output_handles_a[1], stderr_buff.data(), stderr_buff.size(), &stderr_bytes, &output_ovlp[1]);
        // if (!so_read)
        //{
        //    auto err = GetLastError();
        //    switch (err)
        //    {
        //    case ERROR_HANDLE_EOF:
        //        spdlog::debug("EOF found on ReadFile");
        //        break;
        //    case ERROR_IO_PENDING:
        //        {
        //
        //        }
        //        break;
        //    default:
        //        spdlog::error("Error reading process stderr");
        //        break;
        //    }
        //}

        auto waitRes = WaitForMultipleObjects(4, io_events, FALSE, 5000);
        switch (waitRes)
        {
        case WAIT_OBJECT_0 + 0:
            // Stdout
            {
                stdout_process = true;
            }
            break;

        case WAIT_OBJECT_0 + 1:
            // Stderr
            {
                stderr_process = true;
            }
            break;

        case WAIT_OBJECT_0 + 2:
            // Stdin
            break;

        case WAIT_OBJECT_0 + 3:
            // Process ended
            stop = true;
            break;

        case WAIT_TIMEOUT:
            break;

        case WAIT_FAILED:
        {
            std::stringstream err;
            err << "Wait for process failed:\n" << geterror_to_string();
            spdlog::debug(err.str());
            throw std::logic_error(err.str());
        }
        break;

        default:
            break;
        }
    } while (!stop);
}

void process_win::stop() const
{
    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    CloseHandle(output_handles_b[0]);
    CloseHandle(output_handles_b[1]);
    CloseHandle(input_handles[1]);
}

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

    // while (!pb->is_running())
    //     Sleep(1000);

    // WaitForMultipleObjects(1, h, TRUE, INFINITE);

    // CloseHandle(h[0]);
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

auto geterror_to_string()
{
    char buf[256];
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buf,
        (sizeof(buf) / sizeof(char)),
        NULL);

    std::string res = buf;

    return res;
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

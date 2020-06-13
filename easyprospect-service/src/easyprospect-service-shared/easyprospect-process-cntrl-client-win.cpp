
#include <easyprospect-service-shared/easyprospect-process-cntrl-client-win.h>

DWORD easyprospect::service::shared::process_cntrl_client_win::
run_process_cntrl_thread(void* vptr)
{
    auto* wcntl = static_cast<process_cntrl_client_win*>(vptr);

    try
    {
    	wcntl->listen_loop();
    }
    catch (std::logic_error& ex)
    {
        spdlog::error("Listen loop failed. {}", ex.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void easyprospect::service::shared::process_cntrl_client_win::
start()
{
    process_control_thread_handle_ = CreateThread(
        NULL, // default security attributes
        0, // use default stack size
        run_process_cntrl_thread, // thread function name
        this, // argument to thread function
        0, // use default creation flags
        &process_control_thread_id_);
}

void easyprospect::service::shared::process_cntrl_client_win::
stop()
{
    ;
}

void easyprospect::service::shared::process_cntrl_client_win::
listen_loop()
{
    DWORD dwWait;
    bool stop = false;
    bool read_pipe = true;
    bool write_pipe = true;
    bool read_pending = false;
    bool write_pending = false;
    std::vector<unsigned char> input_buffer(50000);
    DWORD read_bytes;
    DWORD write_bytes;
    BOOL                       fSuccess = FALSE;
    DWORD                      dwMode;

    woverlapped[0] = {0};
    woverlapped[1] = {0};

    wevent[0] = CreateEvent(
        NULL,  // default security attribute
        FALSE, // manual-reset event
        FALSE, // initial state = signaled
        NULL); // unnamed event object

    // write event
    wevent[1] = CreateEvent(
        NULL,  // default security attribute
        FALSE, // manual-reset event
        FALSE, // initial state = signaled
        NULL); // unnamed event object

    woverlapped[0].hEvent = wevent[0];
    woverlapped[1].hEvent = wevent[1];

    while (1)
    {
        hPipe = CreateFileA(
            pipe_name.c_str(), 
            GENERIC_READ | GENERIC_WRITE, 
            0, NULL, 
            OPEN_EXISTING, 
            FILE_FLAG_OVERLAPPED,
            NULL);

        auto gle = GetLastError();

        if (hPipe != INVALID_HANDLE_VALUE)
            break;

        switch (gle)
        {
        case ERROR_FILE_NOT_FOUND:
            spdlog::error("Missing pipe. ( is the server running? )");
            throw std::logic_error("Pipe not found error");
            break;

        case ERROR_PIPE_BUSY:
            break;

        case ERROR_SUCCESS:
            break;

        default:
            spdlog::error("Could not open pipe. '{}'\n", geterror_to_string(true, gle));
            throw std::logic_error("Pipe error");
            break;
        }

        DWORD dwMode_ = PIPE_READMODE_MESSAGE;
        if (!SetNamedPipeHandleState(hPipe, &dwMode_, NULL, NULL))
        {
            spdlog::error("Error creating client pipe");
            throw std::logic_error("Client pipe error");
        }

        if (!WaitNamedPipeA(pipe_name.c_str(), 20000))
        {
            spdlog::error("Could not open pipe: 20 second wait timed out.");
            return;
        }
    }

    dwMode   = PIPE_READMODE_MESSAGE;
    fSuccess = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
    if (!fSuccess)
    {
        spdlog::error("SetNamedPipeHandleState failed. GLE={}\n", GetLastError());
        return;
    }

    InterlockedExchange(&initialized, 1);

    do
    {
        if (read_pipe)
        {
            read_pending = true;
            read_pipe = false;

            // Read
            DWORD read_res = 
                ReadFile(hPipe, input_buffer.data(), input_buffer.size(), &read_bytes, &woverlapped[0]);

            // The read operation is still pending.

            DWORD read_err = GetLastError();
            if (!read_res)
            {
                switch (read_err)
                {
                case ERROR_IO_PENDING:
                    break;

                default:
                    spdlog::error("ReadFile() Error: {}", geterror_to_string(true, read_err));
                    throw std::logic_error("ReadFile() error");
                    break;
                }
            }
        }

        if (read_pending)
        {
            read_pending = false;

            auto gor_res = GetOverlappedResult(hPipe, &woverlapped[0], &read_bytes, false);
            auto gor_err = GetLastError();
            if (!gor_res)
            {
                switch (gor_err)
                {
                case ERROR_HANDLE_EOF:
                    spdlog::debug("EOF when reading");
                    break;

                case ERROR_IO_INCOMPLETE:
                    read_pending = true;
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

                auto res = control::process_message_base::process_input(input_buffer, read_bytes);


                //std::string o(input_buffer.begin(), input_buffer.end());
                //if (read_bytes < buffsize)
                //{
                //    o.resize(read_bytes);
                //}

                //spdlog::debug("client read ended : '{}'", o);

                //msgpack::object_handle oh
                //= msgpack::unpack(o.data(), o.size());

                //msgpack::object deserialized = oh.get();

                //std::stringstream os;

                //os << deserialized << std::endl;

                //spdlog::debug("msgpack: '{}'", os.str());
            }
        }

        if (write_pipe && !write_queue.empty())
        {
            write_pipe = false;
            write_pending = true;

            auto b = std::move(write_queue.front());
            write_queue.pop();

            auto write_res = WriteFile(hPipe, b->data(), b->size(), &write_bytes, &woverlapped[1]);
            if (!write_res)
            {
                spdlog::error("WriteFile to pipe failed. GLE={}\n", GetLastError());
                auto write_err = GetLastError();
                if (write_err == ERROR_IO_PENDING)
                {
                    write_pending = true;
                }
                else
                {
                    std::stringstream err;
                    err << "WriteFile failed:\n" << geterror_to_string();
                    spdlog::error(err.str());
                }
            }
        }

        if (write_pending)
        {
            write_pending = false;

            auto gor_res = GetOverlappedResult(hPipe, &woverlapped[1], &write_bytes, true);
            auto gor_err = GetLastError();
            if (!gor_res)
            {
                switch (gor_err)
                {
                case ERROR_HANDLE_EOF:
                    spdlog::debug("EOF when writing");
                    break;

                case ERROR_IO_INCOMPLETE:
                    write_pending = true;
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
                spdlog::debug("write completed");
            }
        }

        dwWait = WaitForMultipleObjects(2, &wevent[0], FALSE, 5000);
        int i = dwWait - WAIT_OBJECT_0;
        switch (dwWait)
        {
        case WAIT_OBJECT_0 + 0:
            {
                read_pipe = true;
            }
            break;

        case WAIT_OBJECT_0 + 1:
            // Write
            {
                write_pipe = true;
            }
            break;

        case WAIT_TIMEOUT:
            // Debugging, exit after waiting for a bit
            // if(++cycles>6)
            //   stop=true;
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
            break;
        }
    }
    while (!stop);
}

void easyprospect::service::shared::process_cntrl_client_win::
setup()
{

}

void easyprospect::service::shared::process_cntrl_client_win::
send(control::process_message_base& obj)
{
    if( !is_running() )
    {
        throw std::logic_error("Control thread not running.");
    }

    BOOL fSuccess = FALSE;

    auto buff = control::process_message_base::pack(obj);

    // FIXME: How should we extend the life of this object?

    write_queue.push(std::move(buff));
}

void easyprospect::service::shared::process_cntrl_client_win::
register_handler()
{
}

bool easyprospect::service::shared::process_cntrl_client_win::
is_running()
{
    bool result = false;

    if (!InterlockedExchangeAdd(&initialized, 0))
    {
        return false;
    }

    auto res    = WaitForSingleObject(process_control_thread_handle_, 0);

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

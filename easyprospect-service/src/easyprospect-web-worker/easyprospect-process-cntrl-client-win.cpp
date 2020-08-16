
#include <easyprospect-web-worker/easyprospect-process-cntrl-client-win.h>
#include <boost/asio/post.hpp>

#include "easyprospect-web-worker/easyprospect-process-message-actions.h"

DWORD easyprospect::service::web_worker::process_cntrl_client_win::
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

void easyprospect::service::web_worker::process_cntrl_client_win::
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

void easyprospect::service::web_worker::process_cntrl_client_win::
stop()
{
    ;
}

void easyprospect::service::web_worker::process_cntrl_client_win::
listen_loop()
{
    DWORD dwWait;
    bool stop = false;

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
        TRUE, // manual-reset event
        TRUE, // initial state = signaled
        NULL); // unnamed event object

    // write event
    wevent[1] = CreateEvent(
        NULL,  // default security attribute
        TRUE, // manual-reset event
        TRUE, // initial state = signaled
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
            throw std::logic_error("Client pipe open timeout");

        }
    }

    dwMode   = PIPE_READMODE_MESSAGE;
    fSuccess = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
    if (!fSuccess)
    {
        spdlog::error("SetNamedPipeHandleState failed. GLE={}\n", GetLastError());
        throw std::logic_error("SetNamedPipeHandleState failed");
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
            if (read_res)
            {
                spdlog::trace("ReadFile returned non-zero");
            }
            else
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

        if ( write_pipe )
        {
            {
                std::lock_guard<std::mutex> lock(write_mutex);

                if (write_queue.empty())
                {
                    if (!ResetEvent(wevent[1]))
                    {
                        spdlog::error("ResetEvent failed. {}", geterror_to_string());
                    }
                    write_pipe = false;
                    write_pending = false;
                }
                else
                {
                    write_pipe = true;
                    write_pending = true;
                }
            }

            if (write_pipe)
            {
                auto b = std::move(write_queue.front());
                write_queue.pop();

                auto write_res = WriteFile(hPipe, b->data(), b->size(), &write_bytes, &woverlapped[1]);
                DWORD write_err     = GetLastError();

                if (write_res)
                {
                    write_pending = false;
                    spdlog::debug("\nWrite to server completed.");
                }
                else
                {
                    if (write_err == ERROR_IO_PENDING)
                    {
                        write_pending = true;
                    }
                    else
                    {
                        spdlog::error("WriteFile to pipe failed. GLE={}\n", GetLastError());     

                        std::stringstream err;
                        err << "WriteFile failed:\n" << geterror_to_string();
                        spdlog::error(err.str());
                    }
                }
            }
        }

        dwWait = WaitForMultipleObjects(2, &wevent[0], FALSE, 5000);
        switch (dwWait)
        {
        case WAIT_TIMEOUT:
            // Debugging, exit after waiting for a bit
            // if(++cycles>6)
            //   stop=true;
            break;

        case WAIT_FAILED:
        {
            std::stringstream err;
            err << "Wait for client IO failed:\n" << geterror_to_string();
            spdlog::error(err.str());
            stop = true;
        }
        break;

        default:
            {
                DWORD ovlp_id_ = dwWait - WAIT_OBJECT_0; // determines which pipe
                spdlog::trace("Event signalled: {}", ovlp_id_);

                if (ovlp_id_== 0)
                {
                    read_pipe = true;

                    if (read_pending)
                    {
                        read_pending = false;

                        auto gor_res = GetOverlappedResult(hPipe, &woverlapped[0], &read_bytes, false);
                        auto gor_err = GetLastError();
                        if (gor_res)
                        {
                            // Successful completion

                            if (read_bytes > 0)
                            {
                                auto res
                                = control::process_message_base::process_input(input_buffer, read_bytes);

                                spdlog::debug(
                                    "process_cntrl_client_win::listen_loop() : \n{}",
                                    control::process_message_base::to_string(*res));

                                {
                                    std::lock_guard<std::mutex> lock(read_mutex);
                                    read_queue.push(std::move(res));
                                }

                                // TODO: KP. Callback for boost::asio::post to ASIO here
                                boost::asio::post([=]()
                                {
                                    // Handle the message sent to this client
                                    std::unique_ptr<control::process_message_base> msg;
                                    {
                                        std::lock_guard<std::mutex> lock(read_mutex);
                                        msg = std::move(this->read_queue.front());
                                        read_queue.pop();
                                    }
                                    std::vector<std::unique_ptr<control::process_message_base>> arg;
                                    arg.push_back(std::move(msg));
                                    process_message_actions::do_action(std::move(arg), app_shutdown_func);
                                });
                            }

                            // std::string o(input_buffer.begin(), input_buffer.end());
                            // if (read_bytes < buffsize)
                            //{
                            //    o.resize(read_bytes);
                            //}

                            // spdlog::debug("client read ended : '{}'", o);

                            // msgpack::object_handle oh
                            //= msgpack::unpack(o.data(), o.size());

                            // msgpack::object deserialized = oh.get();

                            // std::stringstream os;

                            // os << deserialized << std::endl;

                            // spdlog::debug("msgpack: '{}'", os.str());
                        }
                        else
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
                    }
                }
                else if (ovlp_id_== 1)
                {
                    write_pipe = true;

                    if (write_pending)
                    {
                        write_pending = false;

                        auto gor_res = GetOverlappedResult(hPipe, &woverlapped[1], &write_bytes, true);
                        auto gor_err = GetLastError();
                        if (gor_res)
                        {
                            // Successful completion
                            spdlog::debug("write completed");
                        }
                        else
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
                    }
                }
                else
                {
                    spdlog::warn("Invalid overlap : {}", ovlp_id_);
                }
            }
            break;
        }
    }
    while (!stop);
}

void easyprospect::service::web_worker::process_cntrl_client_win::setup(std::function<void()> asf)
{
    app_shutdown_func = asf;
}

void easyprospect::service::web_worker::process_cntrl_client_win::
send(control::process_message_base& obj)
{
    if( !is_running() )
    {
        throw std::logic_error("Control thread not running.");
    }

    spdlog::debug("process_cntrl_client_win::send() : \n{}", control::process_message_base::to_string(obj));

    std::lock_guard<std::mutex> lock(write_mutex);
    auto buff = control::process_message_base::pack(obj);

    write_pipe = true;
    write_queue.push(std::move(buff));
    if (!SetEvent(wevent[1]))
    {
        spdlog::error("SetEvent failed. {}", geterror_to_string());
    }
}

void easyprospect::service::web_worker::process_cntrl_client_win::
register_handler()
{
}

bool easyprospect::service::web_worker::process_cntrl_client_win::
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

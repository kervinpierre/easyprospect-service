#pragma once
#include <cstdio>
#include <string>
#include <queue>
#include <tchar.h>

#include <easyprospect-service-shared/easyprospect-process-base.h>
#include <windows.h>

#include <epworkercntl/epprocess-message.h>

#define BUFSIZE     4096

namespace easyprospect
{
namespace service
{
    namespace shared
    {
        class process_win final : process_base
        {
          private:
            std::string pipe_name;
            OVERLAPPED  woverlapped[2];
            HANDLE      wevent[2];
            HANDLE      hPipe;
            std::queue<std::unique_ptr<msgpack::sbuffer>> write_queue;

          public:
            process_win()
            {
                pipe_name = "\\\\.\\pipe\\easyprospect_pipe_01";
            }

            void listen_loop()
            {
                DWORD dwWait;

                for (;;)
                {
                    dwWait = MsgWaitForMultipleObjectsEx(2, &wevent[0], INFINITE, QS_ALLINPUT, 0);
                    int i  = dwWait - WAIT_OBJECT_0;
                    switch (i)
                    {
                    case 0:
                    {
                        // Read
                        DWORD             dw;
                        std::vector<char> input_buffer(50000);

                        if (!ReadFile(hPipe, input_buffer.data(), input_buffer.size(), NULL, &woverlapped[0]))
                        {
                            // err = GetLastError();
                            // if (err == ERROR_IO_PENDING)
                            //{
                            //    wait_for_object();
                            //    if (!GetOverlappedResult(hPipe, &woverlapped[0], &dw, FALSE))
                            //    {
                            //        // srvfail(L"Read from pipe failed asynchronously.", GetLastError());
                            //    }
                            //}
                            // else
                            //{
                            //    // srvfail(L"Read from pipe failed synchronously.", GetLastError());
                            //}
                        }
                        else
                        {
                            if (!GetOverlappedResult(hPipe, &woverlapped[0], &dw, FALSE))
                            {
                                // srvfail(L"GetOverlappedResult failed reading from pipe.", GetLastError());
                            }
                        }
                    }
                        break;

                    case 1:
                        // Write
                        do
                        {
                            DWORD cbWritten;
                            auto& b = write_queue.front();
                            write_queue.pop();

                            auto fSuccess = WriteFile(hPipe, b->data(), b->size(), &cbWritten, &woverlapped[1]);

                            if (!fSuccess)
                            {
                                _tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());
                                // return -1;
                                if (GetLastError() != ERROR_IO_PENDING)
                                    return;
                            }
                        } while (0);
                        break;

                     default:
                        break;
                    }
                   // srvfail(L"sleep() messageloop", GetLastError());
                }
            }

            void setup() override
            {
                BOOL   fSuccess = FALSE;
                DWORD  dwMode;

                wevent[0] = CreateEvent(
                    NULL,  // default security attribute
                    TRUE,  // manual-reset event
                    TRUE,  // initial state = signaled
                    NULL); // unnamed event object

                // write event
                wevent[1] = CreateEvent(
                    NULL,  // default security attribute
                    TRUE,  // manual-reset event
                    TRUE,  // initial state = signaled
                    NULL); // unnamed event object

                woverlapped[0].hEvent = wevent[0];
                woverlapped[1].hEvent = wevent[1];

                while (1)
                {
                    hPipe = CreateFileA(
                        pipe_name.c_str(),
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_OVERLAPPED,
                        NULL);

                    if (hPipe != INVALID_HANDLE_VALUE)
                        break;

                    if (GetLastError() != ERROR_PIPE_BUSY)
                    {
                        _tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
                        return;
                    }

                    if (!WaitNamedPipeA(pipe_name.c_str(), 20000))
                    {
                        printf("Could not open pipe: 20 second wait timed out.");
                        return;
                    }
                }

                dwMode   = PIPE_READMODE_MESSAGE;
                fSuccess = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
                if (!fSuccess)
                {
                    _tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
                    return;
                }
            }

            void send(control_worker::process_message_base& obj) override
            {
                BOOL  fSuccess = FALSE;

                auto buff = control_worker::process_message_base::pack(obj);

                // FIXME: How should we extend the life of this object?

                write_queue.push(std::move(buff)); 
            }
        };
    } // namespace shared
} // namespace service
} // namespace easyprospect

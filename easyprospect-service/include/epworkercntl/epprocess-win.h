#pragma once
#include <epworkercntl/epprocess-base.h>
#include <queue>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <windows.h>

// https://docs.microsoft.com/en-us/windows/win32/ipc/named-pipe-server-using-overlapped-i-o?redirectedfrom=MSDN

#define CONNECTING_STATE 0
#define READING_STATE    1
#define WRITING_STATE    2
#define INSTANCES        4
#define PIPE_TIMEOUT     5000

using namespace easyprospect::service::control_worker;

namespace easyprospect
{
namespace service
{
    namespace control_worker
    {
        const uint32_t          buffsize = 32;

        inline std::string geterror_to_string(bool pass_error = false, DWORD erro = 0)
        {
            DWORD curr = erro;
            if (!pass_error)
            {
                curr = GetLastError();
            }

            char* buf = nullptr;
            FormatMessageA(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                NULL,
                curr,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPSTR>(&buf),
                0,
                NULL);

            std::string res(buf);
            LocalFree(buf);

            return res;
        }

        class process_win final : public process_base
        {
          public:
            PROCESS_INFORMATION pi;
            HANDLE              proc_thread_handle;
            DWORD               proc_thread_id;

            HANDLE     input_handles[2];
            HANDLE     output_handles_a[2];
            HANDLE     output_handles_b[2];
            HANDLE     io_events[4];
            OVERLAPPED output_ovlp[3];

            process_win(std::string c, std::string cl = "", std::string a = "", long p = 0) : process_base(c, cl, a, p)
            {
                ;
            }

            bool                is_running() const;
            void                setup();
            void                listen_loop();
            static DWORD WINAPI run_process_thread(void* vptr);
            void                start();
            void                stop() const;

            std::string str() const
            {
                std::stringstream so;

                so << "Process ID: " << pi.dwProcessId << ", Thread ID: " << proc_thread_id;

                return so.str();
            }
        };

        class process_control_win final : process_control_base
        {
          private:
            HANDLE     server_handle;
            HANDLE     Pipe[INSTANCES];
            HANDLE     hEvents[INSTANCES * 2];
            OVERLAPPED woverlapped[INSTANCES * 2];

            DWORD       server_thread_id;
            std::string pipe_name;

            std::queue<std::unique_ptr<msgpack::sbuffer>> write_queue[INSTANCES];
            std::unique_ptr<msgpack::sbuffer>             pending_write[INSTANCES];

          public:
            process_control_win()
            {
                pipe_name = "\\\\.\\pipe\\easyprospect_pipe_01";
            }

            bool is_running();

            void start();

            void stop();

            void setup() override;

            void listen_loop() override;

            static DWORD WINAPI run_control_thread(void* vptr);

            void send(int i, control_worker::process_message_base& obj) override;

            // DisconnectAndReconnect(DWORD)
            // This function is called when an error occurs or when the client
            // closes its handle to the pipe. Disconnect from this client, then
            // call ConnectNamedPipe to wait for another client to connect.

            VOID DisconnectAndReconnect(DWORD i);

            // connect_to_new_client(HANDLE, LPOVERLAPPED)
            // This function is called to start an overlapped connect operation.
            // It returns TRUE if an operation is pending or FALSE if the
            // connection has been completed.

            BOOL connect_to_new_client(HANDLE hPipe, LPOVERLAPPED lpo);
        };

    } // namespace control_worker
} // namespace service
} // namespace easyprospect
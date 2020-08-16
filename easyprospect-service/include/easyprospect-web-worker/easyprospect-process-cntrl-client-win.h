#pragma once
#include <cstdio>
#include <queue>
#include <string>
#include <tchar.h>

#include <easyprospect-web-worker/easyprospect-process-cntrl-client-base.h>
#include <windows.h>

#include <easyprospect-service-control/epprocess-message.h>
#include <spdlog/spdlog.h>


#include "epworkercntl/epprocess-win.h"

#define BUFSIZE 4096

namespace easyprospect
{
namespace service
{
    namespace web_worker
    {
        class process_cntrl_client_win final : public process_cntrl_client_base
        {
          private:
            std::string                                   pipe_name;

            volatile bool                                 read_pipe  = true;
            volatile bool                                 write_pipe = true;

            OVERLAPPED                                    woverlapped[2];
            HANDLE                                        wevent[2];
            HANDLE                                        hPipe;
            HANDLE                                        process_control_thread_handle_;
            DWORD                                         process_control_thread_id_;
            DWORD                                          initialized = 0;
            std::mutex write_mutex;
            std::mutex read_mutex;
            std::queue<std::unique_ptr<msgpack::sbuffer>>  write_queue;
            std::queue<std::unique_ptr<control::process_message_base>> read_queue;

            std::function<void()> app_shutdown_func;

        public:
            process_cntrl_client_win()
            {
                pipe_name = R"(\\.\pipe\easyprospect_pipe_01)";
            }

            static DWORD WINAPI run_process_cntrl_thread(void* vptr);

            ~process_cntrl_client_win()
            {
                ;
            }

            void start() override;
            void stop() override;
            void listen_loop() override;
            void setup(std::function<void()> asf) override;
            void send(control::process_message_base& obj) override;
            void register_handler() override;
            bool is_running() override;
        };
    } // namespace shared
} // namespace service
} // namespace easyprospect

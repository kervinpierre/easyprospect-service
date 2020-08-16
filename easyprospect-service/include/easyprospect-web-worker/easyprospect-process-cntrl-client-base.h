#pragma once
#include <epworkercntl/epprocess-base.h>

namespace easyprospect
{
namespace service
{
    namespace web_worker
    {
        class process_cntrl_client_base
        {
          public:
            virtual ~process_cntrl_client_base()
            {
                ;
            }
            virtual void listen_loop()                                   = 0;
            virtual void send(control::process_message_base& obj) = 0;
            virtual void setup(std::function<void()> app_shutdown_func)  = 0;
            virtual void stop()                                         = 0;
            virtual void start()                                         = 0;
            virtual void register_handler()                              = 0;
            virtual bool is_running()                                    = 0;
        };
    } // namespace shared
} // namespace service
} // namespace easyprospect
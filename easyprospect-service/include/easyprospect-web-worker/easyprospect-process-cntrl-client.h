#pragma once
#include <boost/predef.h>

#ifdef BOOST_OS_WINDOWS
  #include <easyprospect-web-worker/easyprospect-process-cntrl-client-win.h>
#else
  #include <easyprospect-service-shared/easyprospect-process-cntrl-client-linux.h>
#endif

namespace easyprospect
{
namespace service
{
    namespace web_worker
    {
        class process_cntrl_client final
        {
#ifdef BOOST_OS_WINDOWS
            process_cntrl_client_win p_obj_;
#else
            process_cntrl_client_linux p_obj_;
#endif

          public:
            void setup()
            {
                p_obj_.setup();
            }

            void start()
            {
                p_obj_.start();
            }

            void register_handler()
            {
                p_obj_.register_handler();
            }

            void send(control::process_message_base& obj)
            {
                p_obj_.send(obj);
            }

            bool is_running()
            {
                return p_obj_.is_running();
            }
        };
    } // namespace control_worker
} // namespace service
} // namespace easyprospect
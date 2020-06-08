#pragma once
#include <boost/predef.h>

#ifdef BOOST_OS_WINDOWS
#include <epworkercntl/epprocess-win.h>
#else
#include <epworkercntl/epprocess-linux.h>
#endif

namespace easyprospect
{
namespace service
{
    namespace control_worker
    {
        class process final
        {
#ifdef BOOST_OS_WINDOWS
            process_win p_obj_;
#else
            process_linux         p_obj_;
#endif

          public:
            process(std::string c, std::string cl = "", std::string a = "", long p = 0) : p_obj_(c, cl, a, p)
            {
            }

            void setup();

            void start()
            {
                p_obj_.start();
            }

            bool is_running()
            {
                return p_obj_.is_running();
            }

            std::string str()
            {
                return p_obj_.str();
            }
        };

        void process::setup()
        {
            p_obj_.setup();
        }

        class process_control final
        {
#ifdef BOOST_OS_WINDOWS
            process_control_win p_obj_;
#else
            process_control_linux p_obj_;
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

            void send(int i, control_worker::process_message_base& obj)
            {
                p_obj_.send(i, obj);
            }

            void register_handler()
            {
                p_obj_.register_handler();
            }

            bool is_running()
            {
                return p_obj_.is_running();
            }
        };
    } // namespace control_worker
} // namespace service
} // namespace easyprospect
#pragma once
#include <epworkercntl/epprocess-message.h>

namespace easyprospect
{
namespace service
{
    namespace control_worker
    {
        class process_base
        {
        protected:
            long pid;
            std::string command;
            std::string command_line;
            std::string args;

        public:
            process_base(std::string c, std::string cl, std::string a, long p = 0)
            {
                command = c;
                command_line = cl;
                args         = a;
                pid          = p;
            }

            long get_pid()
            {
                return pid;
            }
        };

        class process_control_base
        {
          public:
            virtual ~process_control_base()                      = default;
            virtual void listen_loop()                       = 0;
            virtual void send(int i, process_message_base& msg) = 0;
            virtual void setup()                         = 0;
        };
    } // namespace control_worker
} // namespace service
} // namespace easyprospect
#pragma once
#include <epworkercntl/epprocess-base.h>

namespace easyprospect
{
namespace service
{
    namespace shared
    {
        class process_base
        {
          public:
            virtual ~process_base() = default;
            virtual void listen_loop()      = 0;
            virtual void send(control_worker::process_message_base& obj) = 0;
            virtual void setup()    = 0;
        };
    }
} // namespace service
} // namespace easyprospect
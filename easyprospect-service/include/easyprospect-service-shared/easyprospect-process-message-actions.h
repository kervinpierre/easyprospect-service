#pragma once
#include <easyprospect-service-control/epprocess-message.h>

namespace easyprospect
{
namespace service
{
    namespace shared
    {
        class process_message_actions final
        {
          public:
            static void do_action(control::process_message_base& msg)
            {
            }
        };
    } // namespace shared
} // namespace service
} // namespace easyprospect
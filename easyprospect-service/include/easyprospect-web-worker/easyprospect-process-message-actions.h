#pragma once
#include <easyprospect-service-control/epprocess-message.h>

namespace easyprospect
{
namespace service
{
    namespace web_worker
    {
        class process_message_actions final
        {
          public:
            static void do_action(const control::process_message_base& msg)
            {
                std::stringstream os;

                switch (msg.type)
                {
                case control::process_message_type::NONE: break;
                case control::process_message_type::BASE: break;
                case control::process_message_type::START: break;
                case control::process_message_type::PING: break;
                case control::process_message_type::PONG: break;
                case control::process_message_type::CMD_STOP: break;
                case control::process_message_type::CMD_RESULT:
                    {
                        spdlog::debug("Processing CMD_RESULT");         
                    }
                    break;
                default: ;
                }

            }
        };
    } // namespace web_worker
} // namespace service
} // namespace easyprospect
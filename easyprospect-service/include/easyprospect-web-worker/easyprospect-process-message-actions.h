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
            static std::vector<std::unique_ptr<control::process_message_base>>
            do_action_cmd_result(
                std::vector<std::unique_ptr<control::process_message_base>> msg)
            {
                std::vector<std::unique_ptr<control::process_message_base>> res;

                for ( auto& m : msg )
                {
                    spdlog::debug("do_action_cmd_result():\n{}", control::process_message_base::to_string(*m));
                }

                return res;
            }

            static std::vector<std::unique_ptr<control::process_message_base>> do_action_start(
                std::vector<std::unique_ptr<control::process_message_base>> msg)
            {
                std::vector<std::unique_ptr<control::process_message_base>> res;

                for (auto& m : msg)
                {
                    spdlog::debug("do_action_start():\n{}", control::process_message_base::to_string(*m));
                }

                return res;
            }

            static std::vector<std::unique_ptr<control::process_message_base>> do_action_stop(
                std::vector<std::unique_ptr<control::process_message_base>> msg,
                std::function<void()> shutdown_func)
            {
                std::vector<std::unique_ptr<control::process_message_base>> res;

                for (auto& m : msg)
                {
                    spdlog::debug("do_action_stop():\n{}", control::process_message_base::to_string(*m));

                    shutdown_func();
                }

                return res;
            }

            static std::vector<std::unique_ptr<control::process_message_base>>
            do_action(
                std::vector<std::unique_ptr<control::process_message_base>> msg,
                std::function<void()> shutdown_func)
            {
                std::vector<std::unique_ptr<control::process_message_base>> res;
                std::stringstream os;

                switch (msg[0]->type)
                {
                case control::process_message_type::NONE: break;
                case control::process_message_type::BASE: break;
                case control::process_message_type::START:
                {
                    spdlog::debug("Processing START");
                    res = do_action_start(std::move(msg));
                }
                    break;

                case control::process_message_type::PING: break;
                case control::process_message_type::PONG: break;
                case control::process_message_type::CMD_STOP:
                {
                    spdlog::debug("Processing STOP");
                    res = do_action_stop(std::move(msg), shutdown_func);
                }
                break;

                case control::process_message_type::CMD_RESULT:
                    {
                        spdlog::debug("Processing CMD_RESULT");
                        res = do_action_cmd_result(std::move(msg));
                    }
                    break;

                default:
                    {
                        spdlog::error("do_action: Invalid message type");
                        throw std::runtime_error("do_action: invalid msg type");
                    }
                    break;
                }

                return res;
            }
        };
    } // namespace web_worker
} // namespace service
} // namespace easyprospect
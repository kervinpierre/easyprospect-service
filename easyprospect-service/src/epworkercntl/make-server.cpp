#include <epworkercntl/epprocess.h>
#include <epworkercntl/make-server.h>

extern volatile int ep_full_exit = 0;

namespace easyprospect
{
namespace service
{
    namespace control_worker
    {
        void make_server(config::easyprospect_config_wcntl_core curr_config)
        {
            std::string command, command_line, command_args;
            std::string args = curr_config.get_worker_args();
            auto cmd = curr_config.get_worker_exe();
            auto wc = curr_config.get_worker_conf();
            auto wa = curr_config.get_worker_args_file();

            int proc_count = 1;

            if ( !cmd )
            {
                // Missing command
                throw std::logic_error("Missing worker command.");
            }

            command = cmd->generic_string();
            if ( !args.empty() )
            {
                command_args += " " + args    ;
            }
            else if ( wc )
            {
                command_args += " --conf " + wc->generic_string();
            }
            else if (wa)
            {
                command_args += " --arg-file " + wa->generic_string();
            }

            command_line = command + " " + command_args;

            auto pcntl = std::make_unique<process_control>();
            pcntl->register_handler();
            pcntl->setup();
            pcntl->start();

            auto proc_list = std::vector<std::unique_ptr<process>>();
            auto proc_start_messages = std::vector<std::vector<std::unique_ptr<control::process_message_startup>>>(proc_count);

            for (auto i=0; i<proc_count; i++ )
            {
                proc_list.emplace_back(std::make_unique<process>(command, command_line, command_args));
                proc_list[i]->setup();
                proc_list[i]->start();
            }

            do
            {

                for (auto i = 0; i < proc_count; i++)
                {
                    if(proc_list[i]==nullptr)
                    {
                        // Maybe deleted/terminated process?
                        // start a new one?
                        ep_full_exit = 1;
                    }
                    else if (!proc_list[i]->is_running())
                    {
                        spdlog::debug("Process stopped unexpectedly. {}", proc_list[i]->str());

                        proc_list[i].reset();

                        ep_full_exit = 1;
                    }

                     auto m = pcntl->next_message(i);
                    if ( m == nullptr )
                    {
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        continue;
                    }

                    switch (m->type)
                    {
                    case control::process_message_type::NONE:
                        break;
                    case control::process_message_type::BASE:
                        break;
                    case control::process_message_type::START:
                    {
                        auto s = std::unique_ptr<control::process_message_startup>(
                            static_cast<control::process_message_startup*>(m.release()));
                        proc_start_messages[i].push_back(std::move(s));

                        auto res = std::make_unique<control::process_message_cmd_result>();
                        pcntl->send(i, *res);
                    }
                    break;
                    case control::process_message_type::PING:
                        break;
                    case control::process_message_type::PONG:
                        break;
                    case control::process_message_type::CMD_STOP:
                        break;
                    case control::process_message_type::CMD_RESULT:
                        break;
                    default:;
                    }
                }

                if (!pcntl->is_running())
                {
                    spdlog::debug("Control thread stopped.");
                    ep_full_exit = 1;
                }

            } while (!ep_full_exit) ;

            spdlog::debug("Control Server stopped.");
        }


    } // namespace control_worker
} // namespace service
} // namespace easyprospect
#include <epworkercntl/epprocess.h>
#include <epworkercntl/make-server.h>

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
            auto proc  = std::make_unique<process>(command, command_line, command_args);
            auto pcntl = std::make_unique<process_control>();

            pcntl->setup();
            pcntl->start();

            // TODO: KP. Start the specified number of processes
            proc->setup();
            proc->start();

            bool exit = false;

            do
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));

                if (!proc->is_running())
                {
                    spdlog::debug("Process stopped unexpectedly. {}", proc->str());

                    // start a new one?
                    proc.reset();
                    exit = true;
                }

                if (!pcntl->is_running())
                {
                    spdlog::debug("Control thread stopped.");
                    exit = true;
                }
            } while (!exit) ;
        }
    } // namespace control_worker
} // namespace service
} // namespace easyprospect
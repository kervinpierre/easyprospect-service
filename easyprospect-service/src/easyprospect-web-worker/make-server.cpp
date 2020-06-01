
#include <memory>
#include <easyprospect-service-shared/externs.h>
#include <easyprospect-service-shared/server.h>
#include <easyprospect-web-worker/worker-server-app.h>
#include <easyprospect-service-shared/easyprospect-process-cntrl-client.h>

namespace easyprospect
{
namespace service
{
    namespace web_worker
    {
        std::unique_ptr<shared::server> make_server(
            config::easyprospect_config_service_core curr_config)
        {
            beast::error_code ec;

            // Read the server configuration
            std::unique_ptr<application_impl> srv;
            {
                try
                {
                    // Create the server
                    srv = boost::make_unique<easyprospect::service::web_worker::application_impl>(curr_config);
                }
                catch (beast::system_error const& e)
                {
                    spdlog::debug("server_config: {}", e.code().message());

                    return nullptr;
                }
            }

            // Add services
            make_blackjack_service(*srv);

            // Create listeners
            {
                auto ls = curr_config.get_listeners();
                for (auto& e : *ls)
                {
                    try
                    {
                        if (!run_listener(*srv, e))
                            return nullptr;
                    }
                    catch (beast::system_error const& ex)
                    {
                        spdlog::debug("listener_config: {}", ex.code().message());

                        return nullptr;
                    }
                }
            }

            return srv;
        }

        std::unique_ptr<shared::process_cntrl_client> make_control_server(
            config::easyprospect_config_service_core curr_config)
        {
            auto pcntl = std::make_unique<shared::process_cntrl_client>();
            pcntl->register_handler();
            pcntl->setup();
            pcntl->start();

            // test
            std::this_thread::sleep_for(std::chrono::seconds(1));

            process_message_startup st;
            pcntl->send(st);

            // do
            //{
            //    std::this_thread::sleep_for(std::chrono::seconds(1));
            //    if (!pcntl->is_running())
            //    {
            //        spdlog::debug("Control thread stopped.");
            //        ep_full_exit = 1;
            //    }
            //} while (!ep_full_exit);
            //

            return pcntl;
        }


    } // namespace web_worker
} // namespace service
} // namespace easyprospect
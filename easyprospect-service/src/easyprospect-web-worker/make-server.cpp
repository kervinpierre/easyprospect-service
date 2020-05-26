
#include <memory>
#include <easyprospect-service-shared/externs.h>
#include <easyprospect-service-shared/server.h>
#include <easyprospect-web-worker/worker-server-app.h>

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

    } // namespace web_worker
} // namespace service
} // namespace easyprospect
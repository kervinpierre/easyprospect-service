#pragma once

#include <boost/asio/signal_set.hpp>

#include <easyprospect-config/easyprospect-config-service.h>
#include <easyprospect-service-shared/server.h>

#include <easyprospect-config/easyprospect-registry.h>
#include <easyprospect-config/easyprospect-config-server.h>

#include "easyprospect-server-downstream.h"

namespace easyprospect
{
    namespace service
    {
        namespace web_server
        {
            std::unique_ptr<shared::server>
                make_server(
                    config::easyprospect_config_server_core curr_config,
                    std::shared_ptr<config::easyprospect_registry> reg);

            class server_impl : public shared::application_impl_base
            {
                using clock_type = std::chrono::steady_clock;
                using time_point = clock_type::time_point;

                // Used in initialization, leave on top
                boost::asio::io_context                       downstream_ioc_;
                boost::asio::io_context                       upstream_ioc_;

                config::easyprospect_config_server_core       cfg_;
                std::vector<std::unique_ptr<shared::service>> services_;
                boost::asio::basic_waitable_timer<clock_type, boost::asio::wait_traits<clock_type>, executor_type>
                                                             timer_;
                boost::asio::basic_signal_set<executor_type> signals_;
                std::condition_variable                      cv_;
                std::mutex                                   mutex_;
                time_point                                   shutdown_time_;
                bool                                         running_ = false;
                std::atomic<bool>                            stop_;

                std::unique_ptr<easyprospect_server_downstream> downstream_;


                static std::chrono::steady_clock::time_point never() noexcept
                {
                    return (time_point::max)();
                }

              public:
                //------------------------------------------------------------------------------
                explicit server_impl(
                    config::easyprospect_config_server_core cfg);

                ~server_impl()
                {
                }

                void insert(std::unique_ptr<shared::service> sp) override;
                void run() override;
                bool is_shutting_down() override;
                void shutdown(std::chrono::seconds cooldown) override;
                void on_timer(boost::beast::error_code ec = {});
                void on_signal(boost::beast::error_code ec, int signum);
                void stop() override;
                const boost::optional<boost::filesystem::path> get_doc_root() const override;
                std::vector<std::regex>                        get_epjs_url_path_regex() const override;

                boost::asio::io_context&                        get_upstream_ioc()
                {
                    return upstream_ioc_;
                }
            };
        }
    }
}

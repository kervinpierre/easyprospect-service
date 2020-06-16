#pragma once

#include <easyprospect-service-shared/server.h>
#include <easyprospect-web-worker/worker-server.h>
#include <boost/asio/basic_signal_set.hpp>
//
#include <easyprospect-service-shared/utility.hpp>

namespace easyprospect
{
namespace service
{
    namespace web_worker
    {
        class application_impl : public easyprospect::service::shared::application_impl_base,
                                 public boost::enable_shared_from
        {
            using clock_type = std::chrono::steady_clock;
            using time_point = clock_type::time_point;

            easyprospect::service::config::easyprospect_config_service_core                            cfg_;
            std::vector<std::unique_ptr<easyprospect::service::shared::service>>                       services_;
            boost::asio::basic_waitable_timer<clock_type, boost::asio::wait_traits<clock_type>, executor_type> timer_;
            boost::asio::basic_signal_set<executor_type>                                                      signals_;
            std::condition_variable                                                                    cv_;
            std::mutex                                                                                 mutex_;
            time_point                                                                                 shutdown_time_;
            bool                                                                                       running_ = false;
            std::atomic<bool>                                                                          stop_;
            std::unique_ptr<process_cntrl_client>                                                      control_client_;

            std::unique_ptr<channel_list_impl> channel_list_;

            static std::chrono::steady_clock::time_point never() noexcept
            {
                return (time_point::max)();
            }

          public:
            explicit application_impl(
                easyprospect::service::config::easyprospect_config_service_core
                cfg);

            ~application_impl()
            {
            }

            // void dispatch(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess) override
            //{
            //    // Validate and extract the channel id
            //    auto const cid =
            //        shared::checked_value(rpc.params, "cid")
            //        .get<size_t>();

            //    spdlog::debug("channel_list::dispatch() called for cid {}", cid);

            //    // Lookup cid
            //    auto c = channel_list().at(cid);
            //    if (!c)
            //        rpc.fail(
            //            rpc_code::invalid_params,
            //            "Unknown cid");

            //    // Dispatch the request
            //    c->dispatch(rpc, u, sess);
            //}

            void insert(std::unique_ptr<shared::service> sp) override;

            void run() override;

            //--------------------------------------------------------------------------
            //
            // shutdown / stop
            //
            //--------------------------------------------------------------------------

            bool is_shutting_down() override
            {
                return stop_.load();
            }

            void shutdown(std::chrono::seconds cooldown) override
            {
                // Get on the strand
                if (!timer_.get_executor().running_in_this_thread())
                    return boost::asio::post(
                        timer_.get_executor(), boost::beast::bind_front_handler(&application_impl::shutdown, this, cooldown));

                // Only callable once
                if (timer_.expiry() != never())
                    return;

                shutdown_time_ = clock_type::now() + cooldown;
                on_timer();
            }

            void on_timer(boost::beast::error_code ec = {})
            {
                if (ec == boost::asio::error::operation_aborted)
                    return;

                auto const remain =
                    easyprospect::service::shared::ceil<std::chrono::seconds>(shutdown_time_ - clock_type::now());

                // Countdown finished?
                if (remain.count() <= 0)
                {
                    stop();
                    return;
                }

                std::chrono::seconds amount(remain.count());
                if (amount.count() > 10)
                    amount = std::chrono::seconds(10);

                // Notify users of impending shutdown
                auto           c = this->channel_list_->at(1);
                nlohmann::json jv;
                jv["verb"]    = "say";
                jv["cid"]     = c->cid();
                jv["name"]    = c->name();
                jv["message"] = "Server is shutting down in " + std::to_string(remain.count()) + " seconds";
                c->send(jv);
                timer_.expires_after(amount);
                timer_.async_wait(boost::beast::bind_front_handler(&application_impl::on_timer, this));
            }

            void on_signal(boost::beast::error_code ec, int signum)
            {
                if (ec == boost::asio::error::operation_aborted)
                    return;

                spdlog::debug("application_impl::on_signal: #{}, {}\n", signum, ec.message());

                if (timer_.expiry() == never())
                {
                    // Capture signals again
                    signals_.async_wait(boost::beast::bind_front_handler(&application_impl::on_signal, this));

                    this->shutdown(std::chrono::seconds(30));
                }
                else
                {
                    // second time hard stop
                    stop();
                }
            }

            void stop() override
            {
                // Get on the strand
                if (!timer_.get_executor().running_in_this_thread())
                    return boost::asio::post(timer_.get_executor(), boost::beast::bind_front_handler(&application_impl::stop, this));

                // Only callable once
                if (stop_)
                    return;

                // Set stop_ and unblock the main thread
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    stop_ = true;
                    cv_.notify_all();
                }

                // Cancel our outstanding I/O
                timer_.cancel();
                boost::beast::error_code ec;
                signals_.cancel(ec);
            }

            //--------------------------------------------------------------------------

            const boost::optional<boost::filesystem::path> get_doc_root() const override
            {
                return cfg_.get_webroot_dir();
            }

            channel_list_impl& channel_list()
            {
                return *channel_list_;
            }

            std::vector<std::regex> get_epjs_url_path_regex() const override
            {
                return cfg_.get_epjs_url_path_regex();
            }
        };

    } // namespace web_worker
} // namespace service
} // namespace easyprospect
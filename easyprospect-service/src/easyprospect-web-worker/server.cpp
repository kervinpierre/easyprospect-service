//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

#include <easyprospect-web-worker/blackjack.hpp>
#include "easyprospect-web-worker/channel.hpp"
#include "easyprospect-web-worker/channel_list.hpp"
#include "easyprospect-web-worker/listener.hpp"
#include "easyprospect-web-worker/server.hpp"
#include "easyprospect-web-worker/service.hpp"
#include "easyprospect-web-worker/utility.hpp"
#include <easyprospect-config/easyprospect-config-service.h>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/asio/basic_signal_set.hpp>
#include <boost/assert.hpp>
#include <boost/make_unique.hpp>
#include <boost/throw_exception.hpp>
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <uriparser/Uri.h>
#include <easyprospect-web-worker/system_util.h>


//------------------------------------------------------------------------------

//extern
//void
//make_blackjack_service(server&);
//
//extern
//std::unique_ptr<channel_list>
//make_channel_list(server&);
//
//extern
//void
//make_system_channel(server&);

//------------------------------------------------------------------------------


//listener_config::
//listener_config(easyprospect::service::config::easyprospect_config_service_listener_conf& conf)
//    : name(conf.get_name())
//    , address(json::value_cast<net::ip::address>(conf.get_address()))
//    , port_num(json::number_cast<unsigned short>(conf.get_port()))
//{
//}

//------------------------------------------------------------------------------

namespace easyprospect
{
    namespace service
    {
        namespace web_worker
        {
            class server_impl_base : public server
            {
            public:
                net::io_context ioc_;

                // This function is in a base class because `server_impl`
                // needs to call it from the ctor-initializer list, which
                // would be undefined if the member function was in the
                // derived class.

                executor_type
                    make_executor() override
                {
#ifdef LOUNGE_USE_SYSTEM_EXECUTOR
                    return net::make_strand(
                        net::system_executor{});
#else
                    return net::make_strand(
                        ioc_.get_executor());
#endif
                }
            };

            class server_impl
                : public server_impl_base
            {
                using clock_type = std::chrono::steady_clock;
                using time_point = clock_type::time_point;

                easyprospect::service::config::easyprospect_config_service_core cfg_;
                std::vector<std::unique_ptr<service>> services_;
                net::basic_waitable_timer<
                    clock_type,
                    boost::asio::wait_traits<clock_type>,
                    executor_type> timer_;
                asio::basic_signal_set<executor_type> signals_;
                std::condition_variable cv_;
                std::mutex mutex_;
                time_point shutdown_time_;
                bool running_ = false;
                std::atomic<bool> stop_;

                std::unique_ptr<easyprospect::service::web_worker::channel_list> channel_list_;

                static
                    std::chrono::steady_clock::time_point
                    never() noexcept
                {
                    return (time_point::max)();
                }

            public:
                explicit
                    server_impl(
                        easyprospect::service::config::easyprospect_config_service_core cfg)
                    : cfg_(std::move(cfg))
                    , timer_(this->make_executor())
                    , signals_(
                        timer_.get_executor(),
                        SIGINT,
                        SIGTERM)
                    , shutdown_time_(never())
                    , stop_(false)
                    , channel_list_(make_channel_list(*this))
                {
                    timer_.expires_at(never());

                    // TODO: KP. Move URL parsing to some place needed.
                    // Base
                    // UriUriW baseUri;
                    //int res = uriParseSingleUriW(&baseUri, L"example.com", NULL);
                    //if (res != 0)
                    //{
                    //    ;
                    //}

                    easyprospect::service::web_worker::system_util::
                    make_system_channel(*this);
                }

                ~server_impl()
                {
                }

                void
                    insert(std::unique_ptr<service> sp) override
                {
                    if (running_)
                        throw std::logic_error(
                            "server already running");

                    services_.emplace_back(std::move(sp));
                }

                void
                    run() override
                {
                    if (running_)
                        throw std::logic_error(
                            "server already running");

                    running_ = true;

                    // Start all agents
                    for (auto const& sp : services_)
                        sp->on_start();

                    // Capture SIGINT and SIGTERM to perform a clean shutdown
                    signals_.async_wait(
                        beast::bind_front_handler(
                            &server_impl::on_signal,
                            this));

#ifndef LOUNGE_USE_SYSTEM_EXECUTOR
                    std::vector<std::thread> vt;
                    while (vt.size() < cfg_.get_num_threads())
                        vt.emplace_back(
                            [this]
                            {
                                this->ioc_.run();
                            });
#endif
                    // Block the main thread until stop() is called
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        cv_.wait(lock, [this] { return stop_.load(); });
                    }

                    // Notify all agents to stop
                    auto agents = std::move(services_);
                    for (auto const& sp : agents)
                        sp->on_stop();

                    // services must be kept alive until after
                    // all executor threads are joined.

                    // If we get here, then the server has
                    // stopped, so join the threads before
                    // destroying them.

#ifdef LOUNGE_USE_SYSTEM_EXECUTOR
                    net::system_executor{}.context().join();
#else
                    for (auto& t : vt)
                        t.join();
#endif
                }

                //--------------------------------------------------------------------------
                //
                // shutdown / stop
                //
                //--------------------------------------------------------------------------

                bool
                    is_shutting_down() override
                {
                    return stop_.load();
                }

                void
                    shutdown(std::chrono::seconds cooldown) override
                {
                    // Get on the strand
                    if (!timer_.get_executor().running_in_this_thread())
                        return net::post(
                            timer_.get_executor(),
                            beast::bind_front_handler(
                                &server_impl::shutdown,
                                this,
                                cooldown));

                    // Only callable once
                    if (timer_.expiry() != never())
                        return;

                    shutdown_time_ = clock_type::now() + cooldown;
                    on_timer();
                }

                void
                    on_timer(beast::error_code ec = {})
                {
                    if (ec == net::error::operation_aborted)
                        return;

                    auto const remain =
                        easyprospect::service::web_worker::ceil<std::chrono::seconds>(
                            shutdown_time_ - clock_type::now());

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
                    auto c = this->channel_list().at(1);
                    nlohmann::json jv;
                    jv["verb"] = "say";
                    jv["cid"] = c->cid();
                    jv["name"] = c->name();
                    jv["message"] = "Server is shutting down in " +
                        std::to_string(remain.count()) + " seconds";
                    c->send(jv);
                    timer_.expires_after(amount);
                    timer_.async_wait(
                        beast::bind_front_handler(
                            &server_impl::on_timer,
                            this));
                }

                void
                    on_signal(beast::error_code ec, int signum)
                {
                    if (ec == net::error::operation_aborted)
                        return;

                    spdlog::debug("server_impl::on_signal: #{}, {}\n", signum, ec.message());

                    if (timer_.expiry() == never())
                    {
                        // Capture signals again
                        signals_.async_wait(
                            beast::bind_front_handler(
                                &server_impl::on_signal,
                                this));

                        this->shutdown(std::chrono::seconds(30));
                    }
                    else
                    {
                        // second time hard stop
                        stop();
                    }
                }

                void
                    stop() override
                {
                    // Get on the strand
                    if (!timer_.get_executor().running_in_this_thread())
                        return net::post(
                            timer_.get_executor(),
                            beast::bind_front_handler(
                                &server_impl::stop,
                                this));

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
                    beast::error_code ec;
                    signals_.cancel(ec);
                }

                //--------------------------------------------------------------------------

                const boost::optional<boost::filesystem::path>
                    doc_root() const override
                {
                    return cfg_.get_webroot_dir();
                }

                easyprospect::service::web_worker::channel_list&
                    channel_list() override
                {
                    return *channel_list_;
                }
            };

//------------------------------------------------------------------------------

std::unique_ptr<server>
make_server(
    easyprospect::service::config::easyprospect_config_service_core curr_config)
{
    beast::error_code ec;

    // Read the server configuration
    std::unique_ptr<server_impl> srv;
    {
        try
        {
            // Create the server
            srv = boost::make_unique<server_impl>(
                curr_config);
        }
        catch(beast::system_error const& e)
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
        for(auto& e : *ls)
        {
            try
            {
                if(! run_listener(*srv, e))
                    return nullptr;
            }
            catch(beast::system_error const& ex)
            {
                spdlog::debug("listener_config: {}", ex.code().message());

                return nullptr;
            }
        }
    }

    return srv;
}

        }
    }
}
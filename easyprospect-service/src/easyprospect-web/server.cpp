//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

#include <atomic>
#include <boost/asio/basic_signal_set.hpp>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/system/config.hpp>

#include <boost/assert.hpp>
#include <boost/make_unique.hpp>
//#include <boost/process.hpp>
#include "easyprospect-web/server.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include <easyprospect-service-shared/server.h>

#include <easyprospect-config/easyprospect-config-service.h>
#include <easyprospect-config/logging.h>
#include <easyprospect-service-shared/externs.h>
#include <easyprospect-service-shared/service.hpp>
#include <easyprospect-service-shared/utility.hpp>
#include <uriparser/Uri.h>

#include "easyprospect-config/easyprospect-config-server.h"
#include "easyprospect-web/easyprospect-server-downstream.h"

namespace easyprospect
{
namespace service
{
    namespace web_server
    {
        server_impl::server_impl(config::easyprospect_config_server_core cfg):
            cfg_(std::move(cfg)), timer_(boost::asio::make_strand(upstream_ioc_.get_executor())),
            signals_(timer_.get_executor(), SIGINT, SIGTERM),
            shutdown_time_(never()), stop_(false)
        {
            spdlog::debug(BOOST_CURRENT_FUNCTION);

            timer_.expires_at(never());

            downstream_ = std::make_unique<easyprospect_server_downstream>();

            if (cfg.get_backends())
            {
                downstream_->set_backends(cfg.get_backends().get());
            }

            // TODO: kp. initialize downstream backends.

            set_send_worker_req_impl(
                [this](
                std::shared_ptr<shared::easyprospect_http_request_builder>& req,
                boost::beast::error_code& ec,
                std::function<void(
                    boost::beast::http::response<boost::beast::http::string_body
                    >)>
                send_res)
                {
                    shared::easyprospect_http_request_result res;

                    // Launch the asynchronous operation
                    // boost::asio::spawn(
                    //    this->upstream_ioc_,
                    //    std::bind(
                    //        &proxy_action,
                    //        std::string(host),
                    //        std::string(port),
                    //        std::string(target),
                    //        version,
                    //        this->upstream_ioc_,
                    //        std::placeholders::_1));

                    // std::function<>

                    // TODO: kp. Spawn on a "downstream io_context".  This thread pool will ensure we can
                    //       use a mutex around a shared collection.
                    int backend_id = 0;
                    boost::asio::spawn(
                        this->downstream_ioc_,
                        [this, req, backend_id, send_res](
                        boost::asio::yield_context yield)
                        {
                            this->downstream_->start_request(
                                send_res, backend_id, req,
                                this->downstream_ioc_, yield);
                        });

                    // return res         ;
                });
            // TODO: KP. Move URL parsing to some place needed.
            // Base
            // UriUriW baseUri;
            // int res = uriParseSingleUriW(&baseUri, L"example.com", NULL);
            // if (res != 0)
            //{
            //    ;
            //}
        }

        void server_impl::insert(std::unique_ptr<shared::service> sp)
        {
            if (running_)
                throw std::logic_error("server already running");

            services_.emplace_back(std::move(sp));
        }

        void server_impl::run()
        {
            if (running_)
                throw std::logic_error("server already running");

            running_ = true;

            // Start all agents
            for (auto const& sp : services_)
                sp->on_start(boost::asio::make_strand(upstream_ioc_.get_executor()));

            // Capture SIGINT and SIGTERM to perform a clean shutdown
            signals_.async_wait(boost::beast::bind_front_handler(&server_impl::on_signal, this));

            // Keep context running, even when there's no work
            auto work_guard_appl =
                std::make_unique<boost::asio::executor_work_guard
                    <boost::asio::io_context::executor_type>>(application_ioc_.get_executor());
            auto work_guard_upstream =
                std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
                    upstream_ioc_.get_executor());
            auto work_guard_downstream =
                std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
                    downstream_ioc_.get_executor());

            std::vector<std::thread> app_vt;
            while (app_vt.size() < cfg_.get_num_threads())
                app_vt.emplace_back([this] { this->application_ioc_.run(); });

            auto upstream_thread   = std::thread([this] { this->upstream_ioc_.run(); });
            auto downstream_thread = std::thread([this] { this->downstream_ioc_.run(); });

            // std::vector<boost::process::child> pt;
            // boost::process::group              ptg;

            //// Create process pool here.

            //// TODO: KP. Start the processes, pass in the arguments, save their PIDs
            //// How can we get the child to dump to our console?
            //// How do I monitor the process group?
            //
            //// while (pt.size() < cfg_.get_num_threads())
            // while (pt.size() < 2)
            //{
            //    // boost::process::child c(boost::process::search_path("epwebworker"),
            //    // "--conf", "../../../data/test01/args01.txt");
            //    //  ptg.add(c);

            //    // while (c.running())
            //    //    do_some_stuff();

            //    // c.wait(); //wait for the process to exit
            //    // int result = c.exit_code();
            //    // pt.emplace_back(boost::process::search_path("epwebworker"), "--conf",
            //    // "../../../data/test01/args01.txt");
            //    pt.emplace_back("epworker.exe", "--conf", "../../../data/test01/argsClient01.txt");
            //    boost::process::child& c = pt.back();

            //    ptg.add(c);
            //}

            // Block the main thread until stop() is called
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] { return stop_.load(); });
            }

            // Notify all agents to stop
            auto agents = std::move(services_);
            for (auto const& sp : agents)
                sp->on_stop();

            // allow the contexts to end
            work_guard_appl->reset();
            work_guard_upstream->reset();
            work_guard_downstream->reset();

            // services must be kept alive until after
            // all executor threads are joined.

            // If we get here, then the server has
            // stopped, so join the threads before
            // destroying them.

            downstream_thread.join();
            upstream_thread.join();

            for (auto& t : app_vt)
                t.join();
        }

        //--------------------------------------------------------------------------
        //
        // shutdown / stop
        //
        //--------------------------------------------------------------------------

        bool server_impl::is_shutting_down()
        {
            return stop_.load();
        }

        void server_impl::shutdown(std::chrono::seconds cooldown)
        {
            // Get on the strand
            if (!timer_.get_executor().running_in_this_thread())
                return boost::asio::post(
                    timer_.get_executor(), boost::beast::bind_front_handler(&server_impl::shutdown, this, cooldown));

            // Only callable once
            if (timer_.expiry() != never())
                return;

            shutdown_time_ = clock_type::now() + cooldown;
            on_timer();
        }

        void server_impl::on_timer(boost::beast::error_code ec)
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

            // TODO: KP. Replace this shutdown notice?

            // auto c = this->channel_list().at(1);
            // nlohmann::json jv;
            // jv["verb"] = "say";
            // jv["cid"] = c->cid();
            // jv["name"] = c->name();
            // jv["message"] = "Server is shutting down in " +
            //    std::to_string(remain.count()) + " seconds";
            // c->send(jv);

            timer_.expires_after(amount);
            timer_.async_wait(boost::beast::bind_front_handler(&server_impl::on_timer, this));
        }

        void server_impl::on_signal(boost::beast::error_code ec, int signum)
        {
            if (ec == boost::asio::error::operation_aborted)
                return;

            spdlog::debug("application_impl::on_signal: #{}, {}\n", signum, ec.message());

            if (timer_.expiry() == never())
            {
                // Capture signals again
                signals_.async_wait(boost::beast::bind_front_handler(&server_impl::on_signal, this));

                this->shutdown(std::chrono::seconds(30));
            }
            else
            {
                // second time hard stop
                stop();
            }
        }

        void server_impl::stop()
        {
            // Get on the strand
            if (!timer_.get_executor().running_in_this_thread())
                return boost::asio::post(
                    timer_.get_executor(), boost::beast::bind_front_handler(&server_impl::stop, this));

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

        const boost::optional<boost::filesystem::path> server_impl::get_doc_root() const
        {
            return cfg_.get_webroot_dir();
        }

        std::vector<std::regex> server_impl::get_epjs_url_path_regex() const
        {
            return cfg_.get_epjs_url_path_regex();
        }

        // void dispatch(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t&
        // sess) override
        //{
        //    throw std::logic_error("The method or operation is not implemented.");
        //}

        /** Create and run a listening socket to accept connections.

        @returns `true` on success
        */
        //------------------------------------------------------------------------------

        std::unique_ptr<shared::server> make_server(
            config::easyprospect_config_server_core        curr_config,
            std::shared_ptr<config::easyprospect_registry> curr_reg)
        {
            boost::beast::error_code ec;

            // Read the server configuration
            std::unique_ptr<web_server::server_impl> srv;
            {
                try
                {
                    // Create the server
                    srv = boost::make_unique<web_server::server_impl>(curr_config);
                }
                catch (boost::beast::system_error const& e)
                {
                    spdlog::debug("server_config: {}", e.code().message());

                    return nullptr;
                }
            }

            // Create listeners
            {
                auto ls = curr_config.get_listeners();
                for (auto& e : *ls)
                {
                    try
                    {
                        if (!run_listener(*srv, e, curr_reg, srv->get_upstream_ioc()))
                            return nullptr;
                    }
                    catch (boost::beast::system_error const& ex)
                    {
                        spdlog::debug("listener_config: {}", ex.code().message());

                        return nullptr;
                    }
                }
            }

            return srv;
        }

    } // namespace web_server
} // namespace service
} // namespace easyprospect

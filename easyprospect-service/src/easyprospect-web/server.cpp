//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

#include "easyprospect-service-shared/server.h"

#include <easyprospect-config/easyprospect-config-service.h>
#include <easyprospect-config/logging.h>
#include <uriparser/Uri.h>

#include <atomic>
#include <boost/asio/basic_signal_set.hpp>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/assert.hpp>
#include <boost/make_unique.hpp>
#include <boost/process.hpp>
#include <boost/throw_exception.hpp>
#include <condition_variable>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "easyprospect-service-shared/externs.h"
#include "easyprospect-service-shared/listener.h"
#include "easyprospect-service-shared/service.hpp"
#include "easyprospect-service-shared/utility.hpp"

namespace easyprospect {
namespace service {
namespace web_server {

//------------------------------------------------------------------------------

class server_impl : public shared::application_impl_base {
  using clock_type = std::chrono::steady_clock;
  using time_point = clock_type::time_point;

  easyprospect::service::config::easyprospect_config_service_core cfg_;
  std::vector<std::unique_ptr<shared::service>> services_;
  net::basic_waitable_timer<clock_type, boost::asio::wait_traits<clock_type>,
                            executor_type>
      timer_;
  asio::basic_signal_set<executor_type> signals_;
  std::condition_variable cv_;
  std::mutex mutex_;
  time_point shutdown_time_;
  bool running_ = false;
  std::atomic<bool> stop_;

  static std::chrono::steady_clock::time_point never() noexcept {
    return (time_point::max)();
  }

 public:
  explicit server_impl(
      easyprospect::service::config::easyprospect_config_service_core cfg)
      : cfg_(std::move(cfg)),
        timer_(this->make_executor()),
        signals_(timer_.get_executor(), SIGINT, SIGTERM),
        shutdown_time_(never()),
        stop_(false) {
    timer_.expires_at(never());

    // TODO: KP. Move URL parsing to some place needed.
    // Base
    // UriUriW baseUri;
    // int res = uriParseSingleUriW(&baseUri, L"example.com", NULL);
    // if (res != 0)
    //{
    //    ;
    //}
  }

  ~server_impl() {}

  void insert(std::unique_ptr<shared::service> sp) override {
    if (running_) throw std::logic_error("server already running");

    services_.emplace_back(std::move(sp));
  }

  void run() override {
    if (running_) throw std::logic_error("server already running");

    running_ = true;

    // Start all agents
    for (auto const &sp : services_) sp->on_start();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    signals_.async_wait(
        beast::bind_front_handler(&server_impl::on_signal, this));

#ifndef LOUNGE_USE_SYSTEM_EXECUTOR
    std::vector<std::thread> vt;
    while (vt.size() < cfg_.get_num_threads())
      vt.emplace_back([this] { this->ioc_.run(); });
#endif

    std::vector<boost::process::child> pt;
    boost::process::group ptg;

    // Create process pool here.

    // while (pt.size() < cfg_.get_num_threads())
    while (pt.size() < 2) {
      // boost::process::child c(boost::process::search_path("epwebworker"),
      // "--conf", "../../../data/test01/args01.txt");
      //  ptg.add(c);

      // while (c.running())
      //    do_some_stuff();

      // c.wait(); //wait for the process to exit
      // int result = c.exit_code();
      // pt.emplace_back(boost::process::search_path("epwebworker"), "--conf",
      // "../../../data/test01/args01.txt");
      pt.emplace_back("epworker.exe");
      boost::process::child &c = pt.back();

      ptg.add(c);
    }

    // Block the main thread until stop() is called
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait(lock, [this] { return stop_.load(); });
    }

    // Notify all agents to stop
    auto agents = std::move(services_);
    for (auto const &sp : agents) sp->on_stop();

      // services must be kept alive until after
      // all executor threads are joined.

      // If we get here, then the server has
      // stopped, so join the threads before
      // destroying them.

#ifdef LOUNGE_USE_SYSTEM_EXECUTOR
    net::system_executor{}.context().join();
#else
    for (auto &t : vt) t.join();
#endif
  }

  //--------------------------------------------------------------------------
  //
  // shutdown / stop
  //
  //--------------------------------------------------------------------------

  bool is_shutting_down() override { return stop_.load(); }

  void shutdown(std::chrono::seconds cooldown) override {
    // Get on the strand
    if (!timer_.get_executor().running_in_this_thread())
      return net::post(
          timer_.get_executor(),
          beast::bind_front_handler(&server_impl::shutdown, this, cooldown));

    // Only callable once
    if (timer_.expiry() != never()) return;

    shutdown_time_ = clock_type::now() + cooldown;
    on_timer();
  }

  void on_timer(beast::error_code ec = {}) {
    if (ec == net::error::operation_aborted) return;

    auto const remain =
        easyprospect::service::shared::ceil<std::chrono::seconds>(
            shutdown_time_ - clock_type::now());

    // Countdown finished?
    if (remain.count() <= 0) {
      stop();
      return;
    }

    std::chrono::seconds amount(remain.count());
    if (amount.count() > 10) amount = std::chrono::seconds(10);

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
    timer_.async_wait(beast::bind_front_handler(&server_impl::on_timer, this));
  }

  void on_signal(beast::error_code ec, int signum) {
    if (ec == net::error::operation_aborted) return;

    spdlog::debug("application_impl::on_signal: #{}, {}\n", signum,
                  ec.message());

    if (timer_.expiry() == never()) {
      // Capture signals again
      signals_.async_wait(
          beast::bind_front_handler(&server_impl::on_signal, this));

      this->shutdown(std::chrono::seconds(30));
    } else {
      // second time hard stop
      stop();
    }
  }

  void stop() override {
    // Get on the strand
    if (!timer_.get_executor().running_in_this_thread())
      return net::post(timer_.get_executor(),
                       beast::bind_front_handler(&server_impl::stop, this));

    // Only callable once
    if (stop_) return;

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

  const boost::optional<boost::filesystem::path> doc_root() const override {
    return cfg_.get_webroot_dir();
  }

  // void dispatch(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t&
  // sess) override
  //{
  //    throw std::logic_error("The method or operation is not implemented.");
  //}
};

/** Create and run a listening socket to accept connections.

@returns `true` on success
*/
//------------------------------------------------------------------------------

std::unique_ptr<shared::server> make_server(
    config::easyprospect_config_service_core curr_config) {
  beast::error_code ec;

  // Read the server configuration
  std::unique_ptr<server_impl> srv;
  {
    try {
      // Create the server
      srv = boost::make_unique<server_impl>(curr_config);
    } catch (beast::system_error const &e) {
      spdlog::debug("server_config: {}", e.code().message());

      return nullptr;
    }
  }

  // Create listeners
  {
    auto ls = curr_config.get_listeners();
    for (auto &e : *ls) {
      try {
        if (!run_listener(*srv, e)) return nullptr;
      } catch (beast::system_error const &ex) {
        spdlog::debug("listener_config: {}", ex.code().message());

        return nullptr;
      }
    }
  }

  return srv;
}

}  // namespace web_server
}  // namespace service
}  // namespace easyprospect

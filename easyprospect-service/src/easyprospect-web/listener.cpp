//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

#include "easyprospect-web/listener.hpp"
#include "easyprospect-web/server.hpp"
#include "easyprospect-web/server_certificate.hpp"
#include "easyprospect-web/service.hpp"
#include "easyprospect-web/utility.hpp"
#include <boost/beast/core/detect_ssl.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/make_unique.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <iostream>
#include <mutex>
#include <vector>

using namespace easyprospect::service::config;

extern
void
run_http_session(
    server& srv,
    listener& lst,
    stream_type stream,
    endpoint_type ep,
    flat_storage storage);

extern
void
run_https_session(
    server& srv,
    listener& lst,
    asio::ssl::context& ctx,
    stream_type stream,
    endpoint_type ep,
    flat_storage storage);


namespace {

// Detects the SSL opening handshake and launches either
// a plain HTTP session or a Secure HTTP session.

class detector
    : public boost::asio::coroutine
    , public session
{
    server& srv_;
    listener& lst_;
    asio::ssl::context& ctx_;
    stream_type stream_;
    endpoint_type ep_;
    flat_storage storage_;

public:
    detector(
        server& srv,
        listener& lst,
        asio::ssl::context& ctx,
        socket_type sock,
        endpoint_type ep)
        : srv_(srv)
        , lst_(lst)
        , ctx_(ctx)
        , stream_(std::move(sock))
        , ep_(ep)
    {
        lst_.insert(this);
    }

    ~detector()
    {
        lst_.erase(this);
    }

    void
    run()
    {
        // Use post to get on to our strand.
        net::post(
            stream_.get_executor(),
            bind_front(this));
    }

    void
    on_stop() override
    {
        net::post(
            stream_.get_executor(),
            beast::bind_front_handler(
                &detector::do_stop,
                this));
    }

    void
    do_stop()
    {
        // Cancel pending I/O, this causes an immediate
        // completion with error::operation_aborted.
        stream_.cancel();
    }

#include <boost/asio/yield.hpp>
    void
    operator()(
        beast::error_code ec = {},
        bool is_tls = false)
    {
        reenter(*this)
        {
            // Set the expiration
            stream_.expires_after(
                std::chrono::seconds(30));

            // See if a TLS handshake is requested
            yield beast::async_detect_ssl(
                stream_,
                storage_,
                bind_front(this));

            // Report any error
            if(ec)
                return fail(ec, "async_detect_ssl");

            if(is_tls)
            {
                // launch the HTTPS session
                return run_https_session(
                    srv_, lst_, ctx_,
                    std::move(stream_),
                    ep_,
                    std::move(storage_));
            }
            else
            {
                // launch the plain HTTP session
                return run_http_session(
                    srv_, lst_,
                    std::move(stream_),
                    ep_,
                    std::move(storage_));
            }
        }
    }
#include <boost/asio/unyield.hpp>

    void
    fail(beast::error_code ec, char const* what)
    {
        if(ec == net::error::operation_aborted)
            spdlog::debug("{} \t {}", what, ec.message());
          //  LOG_TRC(log_, what, '\t', ec.message());
        else
            spdlog::debug("{} \t {}", what, ec.message());
            //LOG_INF(log_, what, '\t', ec.message());
    }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener_impl
    : public boost::asio::coroutine
    , public service
    , public listener
{
    // This hack works around a bug in basic_socket_acceptor
    // which uses the wrong socket type here:
    // https://github.com/boostorg/asio/blob/c7bbd30491c377ebe12f6e33a0992a3280d71fa4/include/boost/asio/detail/reactive_socket_accept_op.hpp#L198

    struct tcp_ex : net::ip::tcp
    {
        tcp_ex(tcp const& t)
            : tcp(t)
        {
        }

        using socket =
            net::basic_stream_socket<
                tcp, executor_type>;
    };

    server& srv_;
    std::mutex mutex_;
    easyprospect::service::config::easyprospect_config_service_listener_conf cfg_;
    asio::ssl::context ctx_;
    net::basic_socket_acceptor<
        tcp_ex, executor_type> acceptor_;
    boost::container::flat_set<
        session*> sessions_;
    endpoint_type ep_;

public:
    listener_impl(
        server& srv,
        easyprospect::service::config::easyprospect_config_service_listener_conf cfg)
        : srv_(srv)
        , ctx_(asio::ssl::context::tlsv12)
        , acceptor_(srv_.make_executor())
    {
        cfg_ = easyprospect_config_service_listener_conf(cfg.get_name(), cfg.get_address(), cfg.get_port(), 
            easyprospect_config_service_listener_conf::to_string(listener_kind::allow_tls));

        // This holds the self-signed certificate used by the server
        load_server_certificate(ctx_);
    }

    ~listener_impl()
    {
        BOOST_ASSERT(sessions_.empty());
    }

    bool
    open()
    {
        beast::error_code ec;

        spdlog::debug("creating address '{}'", cfg_.get_address());
        spdlog::debug("creating port '{}'", cfg_.get_port());
        auto addr = net::ip::make_address(cfg_.get_address());
        auto port = std::stoi(cfg_.get_port());
        endpoint_type ep(addr, port);

        // Open the acceptor
        acceptor_.open(ep.protocol(), ec);
        if(ec)
        {
            spdlog::debug("acceptor_.open: {}", ec.message());

            return false;
        }

        // Allow address reuse
        acceptor_.set_option(
            net::socket_base::reuse_address(true));
        if(ec)
        {
            spdlog::debug("acceptor_.set_option: {}", ec.message());

            return false;
        }

        // Bind to the server address
        acceptor_.bind(ep, ec);
        if(ec)
        {
            spdlog::debug("acceptor_.bind: {}", ec.message());

            return false;
        }

        // Start listening for connections
        acceptor_.listen(
            net::socket_base::max_listen_connections, ec);
        if(ec)
        {
            spdlog::debug("acceptor_.listen: {}", ec.message());

            return false;
        }

        return true;
    }

    void
    do_stop()
    {
        spdlog::debug("listener::do_stop");

        // Close the acceptor
        beast::error_code ec;
        acceptor_.close(ec);

        // Stop all the sessions
        std::vector<
            boost::weak_ptr<session>> v;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            v.reserve(sessions_.size());
            for(auto p : sessions_)
                v.emplace_back(boost::weak_from(p));
            sessions_.clear();
            sessions_.shrink_to_fit();
        }
        for(auto& e : v)
            if(auto sp = e.lock())
                sp->on_stop();
    }

#include <boost/asio/yield.hpp>
    void
    operator()(
        beast::error_code ec,
        socket_type sock)
    {
        reenter(*this)
        {
            for(;;)
            {
                // Report the error, if any
                if(ec)
                    return fail(ec, "listener::acceptor_.async_accept");

                // If the acceptor is closed it means we stopped
                if(! acceptor_.is_open())
                    return;

                // Launch a new session for this connection
                if(cfg_.get_kind() == "no_tls")
                {
                    run_http_session(
                        srv_,
                        *this,
                        stream_type(std::move(sock)),
                        ep_,
                        {});
                }
                else if(cfg_.get_kind() == "allow_tls")
                {
                    auto sp = boost::make_shared<detector>(
                        srv_,
                        *this,
                        ctx_,
                        std::move(sock),
                        ep_);
                    sp->run();
                }
                else
                {
                    run_https_session(
                        srv_,
                        *this,
                        ctx_,
                        stream_type(std::move(sock)),
                        ep_,
                        {});
                }

                // Accept the next connection
                yield acceptor_.async_accept(
                    srv_.make_executor(),
                    ep_,
                    bind_front(this));
            }
        }
    }
#include <boost/asio/unyield.hpp>

    // Report a failure
    void
    fail(beast::error_code ec, char const* what)
    {
        if(ec == net::error::operation_aborted)
            spdlog::debug("{} \t {}", what, ec.message());
            //LOG_TRC(log_, what, '\t', ec.message());
        else
            spdlog::debug("{} \t {}", what, ec.message());
            //LOG_INF(log_, what, '\t', ec.message());
    }

    //--------------------------------------------------------------------------
    //
    // listener
    //
    //--------------------------------------------------------------------------

    void
    insert(session* p) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sessions_.insert(p);
    }

    void
    erase(session* p) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sessions_.erase(p);
    }

    //--------------------------------------------------------------------------
    //
    // service
    //
    //--------------------------------------------------------------------------

    /// Called when the server starts
    void
    on_start() override
    {
        // Accept the first connection
        acceptor_.async_accept(
            srv_.make_executor(),
            ep_,
            bind_front(this));
    }

    /// Called when the server stops
    void
    on_stop() override
    {
        // Call do_stop from within the strand
        net::post(
            acceptor_.get_executor(),
            beast::bind_front_handler(
                &listener_impl::do_stop,
                this));
    }
};

} // (anon)

//------------------------------------------------------------------------------

bool
run_listener(
    server& srv,
    easyprospect::service::config::easyprospect_config_service_listener_conf cfg)
{
    auto sp = boost::make_unique<listener_impl>(
            srv, cfg);
    bool open = sp->open();
    srv.insert(std::move(sp));
    return open;
}

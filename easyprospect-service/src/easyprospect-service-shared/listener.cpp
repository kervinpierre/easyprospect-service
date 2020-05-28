#include <boost/asio/coroutine.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/core/detect_ssl.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/make_unique.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <iostream>
#include <mutex>
#include <vector>
#include <spdlog/spdlog.h>
#include <easyprospect-service-shared/listener.h>
#include <easyprospect-service-shared/utility.hpp>
#include <easyprospect-service-shared/session.hpp>
#include <easyprospect-service-shared/server_certificate.hpp>
#include <easyprospect-service-shared/externs.h>

namespace easyprospect
{
    namespace service
    {
        namespace shared
        {

            // Detects the SSL opening handshake and launches either
            // a plain HTTP session or a Secure HTTP session.

            class detector
                : public boost::asio::coroutine
                , public session
            {
                application_impl_base& srv_;
                listener& lst_;
                asio::ssl::context& ctx_;
                stream_type stream_;
                endpoint_type ep_;
                flat_storage storage_;

            public:
                detector(
                    application_impl_base& srv,
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
                        if (ec)
                            return fail(ec, "async_detect_ssl");

                        if (is_tls)
                        {
                            // launch the HTTPS session
                            return easyprospect::service::shared::run_https_session(
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
                    if (ec == net::error::operation_aborted)
                        spdlog::trace("{} \t {}", what, ec.message());
                        // LOG_TRC(log_, what, '\t', ec.message());
                    else
                        spdlog::info("{} \t {}", what, ec.message());
                        // LOG_INF(log_, what, '\t', ec.message());
                }

                void send(nlohmann::json const& jv) override
                {
                    throw std::logic_error("The method or operation is not implemented.");
                }


                void send(shared::message m) override
                {
                    throw std::logic_error("The method or operation is not implemented.");
                }

            };


            inline void listener_impl::fail(beast::error_code ec, char const* what)
            {
                if (ec == net::error::operation_aborted)
                    spdlog::debug("{} \t {}", what, ec.message());
                //LOG_TRC(log_, what, '\t', ec.message());
                else
                    spdlog::debug("{} \t {}", what, ec.message());
                //LOG_INF(log_, what, '\t', ec.message());
            }

            void listener_impl::insert(session* p)
            {
                std::lock_guard<std::mutex> lock(mutex_);
                sessions_.insert(p);
            }

            void listener_impl::erase(session* p)
            {
                std::lock_guard<std::mutex> lock(mutex_);
                sessions_.erase(p);
            }

            void listener_impl::on_start()
            {
                // Accept the first connection
                acceptor_.async_accept(
                    srv_.make_executor(),
                    ep_,
                    bind_front(this));
            }

            void listener_impl::on_stop()
            {
                // Call do_stop from within the strand
                net::post(
                    acceptor_.get_executor(),
                    beast::bind_front_handler(
                        &listener_impl::do_stop,
                        this));
            }

            listener_impl::listener_impl(application_impl_base& srv,
                                         easyprospect::service::config::easyprospect_config_service_listener_conf cfg):
                srv_(srv)
                , ctx_(asio::ssl::context::tlsv12)
                , acceptor_(srv_.make_executor())
            {
                cfg_ = config::easyprospect_config_service_listener_conf(
                    cfg.get_name(), cfg.get_address(), cfg.get_port(),
                    cfg.get_min_port(), cfg.get_max_port(),
                    config::easyprospect_config_service_listener_conf::to_string(config::listener_kind::allow_tls));

                // This holds the self-signed certificate used by the server
                load_server_certificate(ctx_);
            }

            listener_impl::~listener_impl()
            {
                BOOST_ASSERT(sessions_.empty());
            }

            bool listener_impl::open_port(asio::ip::address addr, int port, beast::error_code ec)
            {
                endpoint_type ep(addr, port);

                // Open the acceptor
                acceptor_.open(ep.protocol(), ec);
                if (ec)
                {
                    spdlog::debug("acceptor_.open: {}", ec.message());

                    return false;
                }

                // Allow address reuse
                acceptor_.set_option(
                    net::socket_base::reuse_address(true));
                if (ec)
                {
                    spdlog::debug("acceptor_.set_option: {}", ec.message());

                    return false;
                }

                // Bind to the server address
                acceptor_.bind(ep, ec);
                if (ec)
                {
                    spdlog::debug("acceptor_.bind: {}", ec.message());

                    return false;
                }

                spdlog::debug("open_port(): {}:{} succeeded", addr.to_string(), port);
                return true;
            }

            bool listener_impl::open()
            {
                beast::error_code ec;

                spdlog::debug("creating address '{}'", cfg_.get_address());
                spdlog::debug("creating port '{}' or random from '{}' to '{}'", cfg_.get_port(), cfg_.get_min_port(),
                              cfg_.get_max_port());
                auto addr = net::ip::make_address(cfg_.get_address());

                int port;
                std::stringstream portStream(cfg_.get_port());
                if (portStream >> port)
                {
                    open_port(addr, port, ec);
                }
                else
                {
                    portStream.str(cfg_.get_min_port());
                    int min_port = std::stoi(portStream.str());

                    portStream.str(cfg_.get_max_port());
                    int max_port = std::stoi(portStream.str());

                    if (min_port < 1 || min_port >= std::numeric_limits<uint16_t>::max())
                    {
                        // Invalid min port
                        std::stringstream ss;
                        ss << "Error: invalid minimum port " << min_port << std::endl;
                        spdlog::error(ss.str());
                        throw std::logic_error(ss.str());
                    }

                    if (max_port < min_port || max_port >= std::numeric_limits<uint16_t>::max())
                    {
                        // Invalid max port
                        std::stringstream ss;
                        ss << "Error: invalid maximum port " << max_port << std::endl;
                        spdlog::error(ss.str());
                        throw std::logic_error(ss.str());
                    }

                    for (port = min_port; port <= max_port && !open_port(addr, port, ec); port++);

                    if (port > max_port)
                    {
                        // We ran out of ports
                        std::stringstream ss;
                        ss << "Error: Ran out of ports in range " << min_port << " and " << max_port << std::endl;
                        spdlog::error(ss.str());
                        throw std::logic_error(ss.str());
                    }
                }

                // Start listening for connections
                acceptor_.listen(
                    net::socket_base::max_listen_connections, ec);
                if (ec)
                {
                    spdlog::debug("acceptor_.listen: {}", ec.message());

                    return false;
                }

                return true;
            }

            void listener_impl::do_stop()
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
                    for (auto p : sessions_)
                        v.emplace_back(boost::weak_from(p));
                    sessions_.clear();
                    sessions_.shrink_to_fit();
                }
                for (auto& e : v)
                    if (auto sp = e.lock())
                        sp->on_stop();
            }
#include <boost/asio/yield.hpp>

            void
                listener_impl::operator()(
                    beast::error_code ec,
                    socket_type sock)
            {
                reenter(*this)
                {
                    for (;;)
                    {
                        // Report the error, if any
                        if (ec)
                            return fail(ec, "listener::acceptor_.async_accept");

                        // If the acceptor is closed it means we stopped
                        if (!acceptor_.is_open())
                            return;

                        // Launch a new session for this connection
                        if (cfg_.get_kind() == "no_tls")
                        {
                            run_http_session(
                                srv_,
                                *this,
                                stream_type(std::move(sock)),
                                ep_,
                                {});
                        }
                        else if (cfg_.get_kind() == "allow_tls")
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



            bool run_listener(application_impl_base& srv,
                config::easyprospect_config_service_listener_conf cfg)
            {
                auto sp = boost::make_unique<listener_impl>(
                    srv, cfg);
                bool open = sp->open();
                if (!open)
                    return false;

                srv.insert(std::move(sp));
                return open;
            }
        }
    }
}

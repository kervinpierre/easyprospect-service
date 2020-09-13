#pragma once

#include <boost/asio/spawn.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <easyprospect-service-shared/server.h>

#include <easyprospect-config/easyprospect-config-server.h>
#include <easyprospect-config/easyprospect-registry.h>

namespace easyprospect
{
namespace service
{
    namespace web_server
    {
        class easyprospect_server_downstream_connection
        {
        private:
            boost::uuids::uuid                          id_;
            std::shared_ptr<boost::beast::tcp_stream>   stream_;
            boost::beast::error_code                    last_ec_;
            boost::uuids::uuid                          current_session_;
            bool                                        initialized_ = false;

        public:
            std::mutex stream_lock;

            easyprospect_server_downstream_connection()
            {
                boost::uuids::random_generator gen;
                id_ = gen();
                current_session_ = boost::uuids::nil_uuid();
            }

            bool acquire(boost::uuids::uuid s)
            {
                std::lock_guard<std::mutex> lock(stream_lock);

                if ( current_session_ == boost::uuids::nil_uuid())
                {
                    current_session_ = s;
                    return true;
                }

                return false;
            }

            void init(
                config::easyprospect_config_service_backend_conf const&  be,
                boost::asio::io_context&    ioc,
                boost::asio::yield_context yield)
            {
                boost::beast::error_code ec;
                spdlog::debug(BOOST_CURRENT_FUNCTION);

                std::string host    = be.get_address();
                std::string port    = be.get_port();

                stream_ = std::make_shared<boost::beast::tcp_stream>(ioc);

                                // These objects perform our I/O
                boost::asio::ip::tcp::resolver resolver(ioc);

                // Look up the domain name
                // auto const results = resolver.async_resolve(host, port, yield[ec]);
                auto const results = resolver.async_resolve(host, port, yield[ec]);
                if (ec)
                {
                    spdlog::debug("resolve : {}", ec.message());
                    return;
                }

                // Set the timeout.
                stream_->expires_after(std::chrono::seconds(30));

                // Make the connection on the IP address we get from a lookup
                stream_->async_connect(results, yield[ec]);
                if (ec)
                {
                    spdlog::debug("connect : {}", ec.message());
                    return;
                }

                initialized_ = true;
            }

            void close()
            {
                // Gracefully close the socket
                stream_->socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, last_ec_);
            }

            auto get_stream()
            {
                return stream_;
            }

            boost::uuids::uuid get_current_session() const
            {
                return current_session_;
            }
            void set_current_session(boost::uuids::uuid val)
            {
                current_session_ = val;
            }
            bool is_initialized() const
            {
                return initialized_;
            }

            void set_initialized(bool val)
            {
                initialized_ = val;
            }
        };

        /**
         * Represents a single downstream connection
         */
        class easyprospect_server_downstream_session
        {
        private:
            boost::uuids::uuid id_;
            std::shared_ptr<easyprospect_server_downstream_connection> conn_;

        public:
            easyprospect_server_downstream_session()
            {
                boost::uuids::random_generator gen;
                id_ = gen();
            }

            auto get_conn()
            {
                return conn_;
            }

            boost::uuids::uuid get_id() const
            {
                return id_;
            }

            void set_id(boost::uuids::uuid val)
            {
                id_ = val;
            }

            void set_connection(std::shared_ptr<easyprospect_server_downstream_connection> c)
            {
                conn_ = c;
            }
        };

        class easyprospect_server_downstream
        {
          private:
            // Backend id to vector of connections to that backend
            std::vector<std::vector<std::shared_ptr<easyprospect_server_downstream_connection>>> connections_;

            std::map<boost::uuids::uuid, std::shared_ptr<easyprospect_server_downstream_session>> sessions_;

            std::vector<config::easyprospect_config_service_backend_conf> backends_;

        public:
            easyprospect_server_downstream()
            {
            }

            /*
             * Return a session by id, create a new one with a new id if not found.
             */
            std::shared_ptr<easyprospect_server_downstream_session>
            get_session()
            {
                return get_session(boost::uuids::nil_uuid());
            }

            std::shared_ptr<easyprospect_server_downstream_session>
            get_session(
                boost::uuids::uuid sess,
                bool               throw_on_missing = false)
            {
                auto res = sessions_.find(sess);
                if( res == sessions_.end())
                {
                    if (throw_on_missing)
                    {
                        throw std::logic_error("session not found.");
                    }

                    auto s = std::make_shared<easyprospect_server_downstream_session>();
                    sessions_[s->get_id()] = s;

                    return s;
                }

                return res->second;
            }

            auto get_connection(int backend_id, boost::uuids::uuid sess)
            {
                if (backend_id >= connections_.size())
                {
                    std::string msg = fmt::format("{} larger than connections size {}", backend_id, connections_.size());
                    spdlog::error(msg);

                    throw std::logic_error(msg);
                }

                // Get the connections for the current backend
                auto con = connections_.at(backend_id);

                // Get the first unused connection in the list
                std::shared_ptr<easyprospect_server_downstream_connection> res;

                auto res_itr
                = std::find_if(
                    con.begin(), con.end(), [this,sess](std::shared_ptr<easyprospect_server_downstream_connection> c) {
                        return c->acquire(sess);
                    });

                // If none found, create one
                if ( res_itr == con.end() )
                {
                    auto c = std::make_shared<easyprospect_server_downstream_connection>();

                    c->acquire(sess);
                    con.push_back(c);

                    res = c;
                }
                else
                {
                    res = *res_itr;
                }

                res->set_current_session(sess);

                return res;
            }

            std::vector<config::easyprospect_config_service_backend_conf> &get_backends()
            {
                return backends_;
            }

            void set_backends(const std::vector<config::easyprospect_config_service_backend_conf> &val)
            {
                backends_ = val;

                connections_.resize(backends_.size());

            }

            void start_request( std::function<void(boost::beast::http::response<boost::beast::http::string_body>)> send_res,
                                int backend_id, 
                                std::shared_ptr<shared::easyprospect_http_request_builder>                                 req,
                                boost::asio::io_context &ioc,
                                boost::asio::yield_context yield)
            {
                boost::beast::error_code ec;
                spdlog::debug(BOOST_CURRENT_FUNCTION);
                // spdlog::debug(req.to_string());

                auto curr_sess = get_session();
                auto conn = get_connection(backend_id, curr_sess->get_id());
                if ( !conn->is_initialized() )
                {
                    auto be = get_backends()[backend_id];
                    conn->init(be,ioc, yield);
                }

                // Set up an HTTP GET request message
                auto be_req = req->to_request().to_beast_request();

                // Write the message to standard out
                std::stringstream reqStr;
                reqStr << be_req << std::endl;
                spdlog::debug("proxy req: {}", reqStr.str());

                // Set the timeout.
                conn->get_stream()->expires_after(std::chrono::seconds(30));

                // Send the HTTP request to the remote host
                // TODO: kp. Use async_write_some, leave the stream open for new bytes
                boost::beast::http::async_write(*(conn->get_stream()), be_req, yield[ec]);
                if (ec)
                {
                    // TODO: kp. Possibly reopen the connection and try again
                    spdlog::debug("write : {}", ec.message());
                    return;
                }

                // This buffer is used for reading and must be persisted
                boost::beast::flat_buffer b;

                // Declare a container to hold the response
                boost::beast::http::response<boost::beast::http::string_body> res;

                // Receive the HTTP response
                boost::beast::http::async_read(*conn->get_stream(), b, res, yield[ec]);
                if (ec)
                {
                    spdlog::debug("read : {}", ec.message());
                    return;
                }

                // Write the message to standard out
                std::stringstream resStr;
                resStr << res;
                spdlog::debug("proxy response: {}", resStr.str());

                // not_connected happens sometimes
                // so don't bother reporting it.
                //
                 if (ec && ec != boost::beast::errc::not_connected)
                {
                    spdlog::debug("shutdown : {}", ec.message());
                    return;
                }

                // If we get here then the connection is closed gracefully
                // Connect downstream

                //// Send request
                //boost::beast::http::response<boost::beast::http::string_body> res;
                //res.version(req.version());
                //res.result(boost::beast::http::status::ok);
                //res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                //// res.set(boost::beast::http::field::content_type, mime_type(curr_path.generic_string()));
                //// res.content_length(size);
                //res.keep_alive(req.keep_alive());
                //res.body() = output;
                //res.prepare_payload();

                return send_res(std::move(res));
            }
        };
    }
} // namespace service
} // namespace easyprospect
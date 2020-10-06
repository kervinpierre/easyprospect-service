//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

#pragma once

// TODO: KP. Clean up these headers, there are too many header-included files

#include <easyprospect-service-shared/config.hpp>
#include <easyprospect-service-shared/message.hpp>
#include <easyprospect-service-shared/utility.hpp>

#include <boost/asio/post.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/yield.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/beast/http/file_body.hpp>

#include <easyprospect-service-shared/types.hpp>

//#include <easyprospect-service-shared/externs.h>
#include <easyprospect-service-shared/listener.h>
#include <easyprospect-service-shared/rpc.hpp>
#include <easyprospect-service-shared/server.h>
#include <easyprospect-service-shared/user_base.hpp>
#include <spdlog/spdlog.h>


namespace easyprospect
{
namespace service
{
    namespace shared
    {
        class listener;
        class ssl_http_session_impl;
        class plain_http_session_impl;
        class ws_session_t;
        // plain_ws_session_impl
        // ssl_ws_session_impl

        using dispatch_impl_type = std::function<void(rpc_call&, shared::user&, ws_session_t&)>;

        class session : public boost::enable_shared_from
        {
          protected:
            std::mutex                               mutex_;
            boost::optional<boost::filesystem::path> doc_root_;

          public:
            virtual ~session() = default;

            /** Called when the server stops.

                This will be called at most once.
            */
            virtual void on_stop() = 0;

            /** Send a JSON message

                Messages are queued as needed.
                May be called from any thread.
            */
            virtual void send(nlohmann::json const& jv) = 0;

            /// Send a message
            virtual void                             send(shared::message m) = 0;
            boost::optional<boost::filesystem::path> doc_root() const;
        };

        template <class Derived>
        class ws_session_base : public boost::asio::coroutine, public shared::session
        {
          protected:
            boost::asio::ip::tcp::endpoint                   ep_;
            boost::beast::flat_buffer                    msg_;
            std::vector<shared::message>    mq_;
            boost::shared_ptr<ws_session_t> wrapper_;
            dispatch_impl_type              dispatch_impl_;
            epjs_process_req_impl_type      epjs_process_req_impl_;
            send_worker_req_impl_type       send_worker_req_impl_;

          public:
            ws_session_base(boost::asio::ip::tcp::endpoint ep) : ep_(ep)
            {
            }

            ~ws_session_base()
            {
            }

            // The CRTP pattern
            Derived* impl()
            {
                return static_cast<Derived*>(this);
            }

            void                            set_wrapper(boost::shared_ptr<ws_session_t>);
            boost::shared_ptr<ws_session_t> get_wrapper();

            //--------------------------------------------------------------------------
            //
            // ws_session
            //
            //--------------------------------------------------------------------------

            void run(boost::beast::websocket::request_type req);

            void operator()(boost::beast::error_code ec = {}, std::size_t bytes_transferred = 0);

            // Report a failure
            void fail(boost::beast::error_code ec, char const* what);

            //--------------------------------------------------------------------------
            //
            // session
            //
            //--------------------------------------------------------------------------

            void on_stop() override;

            void do_stop();

            //--------------------------------------------------------------------------
            //
            // user
            //
            //--------------------------------------------------------------------------

            void send(nlohmann::json const& jv) override;

            void send(shared::message m) override;

            void do_send(shared::message m);

            void do_write();

            void on_write(std::size_t idx, boost::beast::error_code ec, std::size_t);
            std::function<void(rpc_call&, shared::user&, ws_session_t&)> get_dispatch_impl() const
            {
                return dispatch_impl_;
            }

            void set_dispatch_impl(std::function<void(rpc_call&, shared::user&, ws_session_t&)> val)
            {
                dispatch_impl_ = val;
            }

            void set_epjs_process_req_impl(epjs_process_req_impl_type val)
            {
                epjs_process_req_impl_ = val;
            }

            epjs_process_req_impl_type get_epjs_process_req_impl() const
            {
                return epjs_process_req_impl_;
            }
        };

        // class ws_session_abstract_t
        //{

        //};

        // template<class T>
        // class ws_session_wrap_t
        //{

        //};

        template <class Derived>
        class http_session_base : public boost::asio::coroutine, public session
        {
          protected:
            application_impl_base&                   srv_;
            listener&                                lst_;
            boost::asio::ip::tcp::endpoint                            ep_;
            boost::beast::flat_buffer                             storage_;
            boost::optional<boost::beast::http::request_parser<boost::beast::http::buffer_body>> pr_;
            char                                                                                 pr_buffer_[4096];
            std::shared_ptr<const easyprospect_http_request>                                     current_req;
            std::vector<std::regex>                  epjs_exts_;
            boost::optional<boost::filesystem::path> doc_root_;

            std::function<void(rpc_call&, shared::user&, ws_session_t&)> dispatch_impl_;
            epjs_process_req_impl_type                                   epjs_process_req_impl_;
            send_worker_req_impl_type                                   send_worker_req_impl_;

            // ep_session_ssl_function_type run_ws_session_ssl_func;
            //  ep_session_plain_function_type run_ws_session_plain_func;


          public:
            http_session_base(application_impl_base& srv, listener& lst, boost::asio::ip::tcp::endpoint ep, boost::beast::flat_buffer storage);

            ~http_session_base();

            // The CRTP pattern
            Derived* impl()
            {
                return static_cast<Derived*>(this);
            }

            // void set_run_session_functions(
            //    ep_session_ssl_function_type   f1,
            //    ep_session_plain_function_type f2 )
            //{
            //    this->run_ws_session_ssl_func = f1;
            //    this->run_ws_session_plain_func = f2;
            //}

            void run_ws_session(
                boost::optional<boost::filesystem::path> doc_root,
                listener&                                lst,
                stream_type                              str,
                boost::asio::ip::tcp::endpoint                            ep,
                boost::beast::websocket::request_type                  req);

            void run_ws_session(
                boost::optional<boost::filesystem::path> doc_root,
                listener&                                lst,
                boost::beast::ssl_stream<stream_type>           str,
                boost::asio::ip::tcp::endpoint           ep,
                boost::beast::websocket::request_type                  req)
            {
                // shared::run_ws_session(srv, lst, std::move(str), ep, req);

                //       run_ws_session_ssl_func(srv, lst, std::move(str), ep, req);
            }

            void run_proxy_session(
                boost::beast::http::request_parser<boost::beast::http::string_body>&&,
                boost::asio::const_buffer,
                stream_type,
                boost::beast::error_code&);

            void run_proxy_session(
                boost::beast::http::request_parser<boost::beast::http::string_body>&&,
                boost::asio::const_buffer,
                boost::beast::ssl_stream<stream_type>,
                boost::beast::error_code&);
            //--------------------------------------------------------------------------
            //
            // session
            //
            //--------------------------------------------------------------------------

            void on_stop() override
            {
                boost::asio::post(
                    impl()->stream().get_executor(),
                    boost::beast::bind_front_handler(&http_session_base::do_stop, boost::shared_from(this)));
            }

            void do_stop()
            {
                boost::beast::error_code ec;
                boost::beast::close_socket(boost::beast::get_lowest_layer(impl()->stream()));
            }

            //--------------------------------------------------------------------------
            //
            // http_session
            //
            //--------------------------------------------------------------------------

            //// TODO: KP. Replace with a generic lambda
            //// We only require C++11, this helper is
            //// the equivalent of a C++14 generic lambda.
            //struct send_lambda
            //{
            //    http_session_base& self_;

            //    template <bool isRequest, class Body, class Fields>
            //    void operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg) const
            //    {
            //        std::stringstream sstr;
            //        sstr << "send_lambda() : " << msg.base() << std::endl;

            //        spdlog::debug(sstr.str());

            //        // The lifetime of the message has to extend
            //        // for the duration of the async operation so
            //        // we use a shared_ptr to manage it.
            //        auto sp = std::make_shared<boost::beast::http::message<isRequest, Body, Fields>>(std::move(msg));

            //        // Write the response
            //        auto self = bind_front(&self_);
            //        boost::beast::http::async_write(
            //            self_.impl()->stream(), *sp, [self, sp](boost::beast::error_code ec, std::size_t bytes_transferred) {
            //                self(ec, bytes_transferred, sp->need_eof());
            //            });
            //    }
            //};
            
            void operator()(boost::beast::error_code ec = {}, std::size_t bytes_transferred = 0, bool need_eof = false);

            std::function<void(rpc_call&, shared::user&, ws_session_t&)> get_dispatch_impl() const
            {
                return dispatch_impl_;
            }

            void set_dispatch_impl(std::function<void(rpc_call&, shared::user&, ws_session_t&)> val)
            {
                dispatch_impl_ = val;
            }

            epjs_process_req_impl_type get_epjs_process_req_impl() const
            {
                return epjs_process_req_impl_;
            }

            void set_epjs_process_req_impl(epjs_process_req_impl_type val)
            {
                epjs_process_req_impl_ = val;
            }

            send_worker_req_impl_type send_worker_process_req_impl() const
            {
                return send_worker_req_impl_;
            }

            void set_send_worker_req_impl(send_worker_req_impl_type val)
            {
                send_worker_req_impl_ = val;
            }

            void handle_worker_request(
                std::shared_ptr<const easyprospect_http_request>                                 first_req,
                int position,
                std::shared_ptr<const easyprospect_http_request_continuation>  req_continuation,
                boost::beast::error_code&                                                          ec,
                std::function<void(boost::beast::http::response<boost::beast::http::string_body>)> send_res) const
            {
                send_worker_req_impl_(first_req, position, req_continuation, ec, send_res);
            }
        };

        enum class ws_session_kind
        {
            none  = 0,
            plain = 1,
            ssl   = 2
        };

        //------------------------------------------------------------------------------

        class plain_ws_session_impl : public shared::ws_session_base<plain_ws_session_impl>
        {
            boost::beast::websocket::stream<stream_type> ws_;

          public:
            plain_ws_session_impl(
                boost::optional<boost::filesystem::path> doc_root,
                stream_type                              stream,
                boost::asio::ip::tcp::endpoint                            ep) :
                ws_session_base(ep),
                ws_(std::move(stream))
            {
                doc_root_ = doc_root;
            }

            boost::beast::websocket::stream<stream_type>& ws()
            {
                return ws_;
            }

            // Report a failure
            void fail(boost::beast::error_code ec, char const* what)
            {
                if (ec == boost::asio::error::operation_aborted)
                    spdlog::trace("{} \t {}", what, ec.message());
                // LOG_TRC(log_, what, '\t', ec.message());
                else
                    spdlog::info("{} \t {}", what, ec.message());
                // LOG_INF(log_, what, '\t', ec.message());
            }
        };

        class ssl_ws_session_impl : public ws_session_base<ssl_ws_session_impl>
        {
            boost::beast::websocket::stream<boost::beast::ssl_stream<stream_type>> ws_;

          public:
            ssl_ws_session_impl(
                boost::optional<boost::filesystem::path> doc_root,
                boost::beast::ssl_stream<stream_type>           stream,
                boost::asio::ip::tcp::endpoint                            ep) :
                ws_session_base(ep),
                ws_(std::move(stream))
            {
                doc_root_ = doc_root;
            }

            boost::beast::websocket::stream<boost::beast::ssl_stream<stream_type>>& ws()
            {
                return ws_;
            }

            // Report a failure
            void fail(boost::beast::error_code ec, char const* what)
            {
                // ssl::error::ws_truncated, also known as an SSL "short read",
                // indicates the peer closed the connection without performing the
                // required closing handshake (for example, Google does this to
                // improve performance). Generally this can be a security issue,
                // but if your communication protocol is self-terminated (as
                // it is with both HTTP and WebSocket) then you may simply
                // ignore the lack of close_notify.
                //
                // https://github.com/boostorg/beast/issues/38
                //
                // https://security.stackexchange.com/questions/91435/how-to-handle-a-malicious-ssl-tls-shutdown
                //
                // When a short read would cut off the end of an HTTP message,
                // Beast returns the error boost::beast::http::error::partial_message.
                // Therefore, if we see a short read here, it has occurred
                // after the message has been completed, so it is safe to ignore it.

                if (ec == boost::asio::ssl::error::stream_truncated)
                    return;

                if (ec == boost::asio::error::operation_aborted)
                    spdlog::trace("{} \t {}", what, ec.message());
                // LOG_TRC( log_, what, '\t', ec.message() );
                else
                    spdlog::info("{} \t {}", what, ec.message());
                // LOG_INF( log_, what, '\t', ec.message() );
            }
        };
        //------------------------------------------------------------------------------

        // TODO: Use type erasure : https://quuxplusone.github.io/blog/2019/03/18/what-is-type-erasure/
        class ws_session_t : public boost::enable_shared_from
        {
            boost::shared_ptr<plain_ws_session_impl> plain_ptr_;
            boost::shared_ptr<ssl_ws_session_impl>   ssl_ptr_;
            boost::shared_ptr<shared::user>          primary_user_;

          public:
            ws_session_t(boost::optional<boost::filesystem::path> doc_root, stream_type stream, boost::asio::ip::tcp::endpoint ep)
            {
                plain_ptr_    = boost::make_shared<shared::plain_ws_session_impl>(doc_root, std::move(stream), ep);
                primary_user_ = boost::make_shared<shared::user>();
            }

            ws_session_t(
                boost::optional<boost::filesystem::path> doc_root,
                boost::beast::ssl_stream<stream_type>           stream,
                boost::asio::ip::tcp::endpoint                            ep)
            {
                ssl_ptr_      = boost::make_shared<shared::ssl_ws_session_impl>(doc_root, std::move(stream), ep);
                primary_user_ = boost::make_shared<shared::user>();
            }

            void send(nlohmann::json const& jv);
            void send(shared::message m);

            void run(boost::beast::websocket::request_type req);

            void set_dispatch_impl(std::function<void(rpc_call&, shared::user&, ws_session_t&)> val);
            void set_epjs_process_req_impl(epjs_process_req_impl_type val);

            void set_wrapper(boost::shared_ptr<ws_session_t>);

            auto get_primary_user() const;
        };

        class plain_http_session_impl : public http_session_base<plain_http_session_impl>
        {
            stream_type stream_;

          public:
            plain_http_session_impl(
                application_impl_base& srv,
                listener&              lst,
                stream_type            stream,
                boost::asio::ip::tcp::endpoint          ep,
                boost::beast::flat_buffer           storage);

            stream_type& stream()
            {
                return stream_;
            }

            void expires_after(std::chrono::seconds n)
            {
                stream_.expires_after(n);
            }

            void expires_never()
            {
                stream_.expires_never();
            }

            void run()
            {
                // Use post to get on to our strand.
                boost::asio::post(stream_.get_executor(), bind_front(this));
            }

            void do_close()
            {
                boost::beast::error_code ec;
                stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
            }

            // Report a failure
            void fail(boost::beast::error_code ec, char const* what)
            {
                if (ec == boost::asio::error::operation_aborted)
                    // LOG_TRC(log_, what, '\t', ec.message());
                    spdlog::debug("{} \t {}", what, ec.message());
                else
                    spdlog::debug("{} \t {}", what, ec.message());
                //  LOG_INF(log_, what, '\t', ec.message());
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

        //------------------------------------------------------------------------------

        class ssl_http_session_impl : public http_session_base<ssl_http_session_impl>
        {
            boost::beast::ssl_stream<stream_type> stream_;

          public:
            ssl_http_session_impl(
                application_impl_base& srv,
                listener&              lst,
                boost::asio::ssl::context&    ctx,
                stream_type            stream,
                boost::asio::ip::tcp::endpoint          ep,
                boost::beast::flat_buffer           storage);

            boost::beast::ssl_stream<stream_type>& stream()
            {
                return stream_;
            }

            void expires_after(std::chrono::seconds n)
            {
                stream_.next_layer().expires_after(n);
            }

            void expires_never()
            {
                stream_.next_layer().expires_never();
            }

            void run()
            {
                // Use post to get on to our strand.
                boost::asio::post(
                    stream_.get_executor(),
                    boost::beast::bind_front_handler(&ssl_http_session_impl::do_run, boost::shared_from(this)));
            }

            void do_run()
            {
                // Set the expiration
                impl()->expires_after(std::chrono::seconds(30));

                // Perform the TLS handshake in the server role
                stream_.async_handshake(
                    boost::asio::ssl::stream_base::server,
                    storage_.data(),
                    boost::beast::bind_front_handler(&ssl_http_session_impl::on_handshake, boost::shared_from(this)));
            }

            void on_handshake(boost::beast::error_code ec, std::size_t bytes_transferred)
            {
                // Adjust the buffer for what the handshake used
                storage_.consume(bytes_transferred);

                // Report the error if any
                if (ec)
                    return fail(ec, "async_handshake");

                // Process HTTP
                (*this)();
            }

            void do_close()
            {
                // Set the expiration
                expires_after(std::chrono::seconds(30));

                // Perform the TLS closing handshake
                stream_.async_shutdown(
                    boost::beast::bind_front_handler(&ssl_http_session_impl::on_shutdown, boost::shared_from(this)));
            }

            void on_shutdown(boost::beast::error_code ec)
            {
                if (ec)
                    return fail(ec, "async_shutdown");
            }

            // Report a failure
            void fail(boost::beast::error_code ec, char const* what)
            {
                // ssl::error::stream_truncated, also known as an SSL "short read",
                // indicates the peer closed the connection without performing the
                // required closing handshake (for example, Google does this to
                // improve performance). Generally this can be a security issue,
                // but if your communication protocol is self-terminated (as
                // it is with both HTTP and WebSocket) then you may simply
                // ignore the lack of close_notify.
                //
                // https://github.com/boostorg/beast/issues/38
                //
                // https://security.stackexchange.com/questions/91435/how-to-handle-a-malicious-ssl-tls-shutdown
                //
                // When a short read would cut off the end of an HTTP message,
                // Beast returns the error boost::beast::http::error::partial_message.
                // Therefore, if we see a short read here, it has occurred
                // after the message has been completed, so it is safe to ignore it.

                if (ec == boost::asio::ssl::error::stream_truncated)
                    return;

                if (ec == boost::asio::error::operation_aborted)
                    // LOG_TRC(log_, what, '\t', ec.message());
                    spdlog::debug("{} \t {}", what, ec.message());
                else
                    spdlog::debug("{} \t {}", what, ec.message());
                // LOG_INF(log_, what, '\t', ec.message());
            }

            void on_stop() override
            {
                throw std::logic_error("The method or operation is not implemented.");
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

        template <class Derived>
        void ws_session_base<Derived>::run(boost::beast::websocket::request_type req)
        {
            std::stringstream ss;
            ss << "ws_user::run(): " << req << std::endl;

            spdlog::trace(ss.str());

            // Apply settings to stream
            impl()->ws().set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));

            // Limit the maximum incoming message size
            impl()->ws().read_message_max(64 * 1024);

            // TODO check credentials in `req`

            // Perform the WebSocket handshake in the server role
            impl()->ws().async_accept(std::move(req), bind_front(impl()));
        }

        template <class Derived>
        void ws_session_base<Derived>::fail(boost::beast::error_code ec, char const* what)
        {
            if (ec == boost::asio::error::operation_aborted)
                spdlog::debug("{} \t {}", what, ec.message());
            // LOG_TRC(log_, what, '\t', ec.message());
            else
                spdlog::debug("{} \t {}", what, ec.message());
            // LOG_INF(log_, what, '\t', ec.message());
        }

        template <class Derived>
        void ws_session_base<Derived>::on_stop()
        {
            boost::asio::post(
                impl()->ws().get_executor(),
                boost::beast::bind_front_handler(&ws_session_base::do_stop, boost::shared_from(this)));
        }

        template <class Derived>
        void ws_session_base<Derived>::do_stop()
        {
            boost::beast::error_code ec;
            boost::beast::close_socket(boost::beast::get_lowest_layer(impl()->ws()));
        }

        template <class Derived>
        void ws_session_base<Derived>::send(nlohmann::json const& jv)
        {
            if (spdlog::default_logger()->level() <= spdlog::level::trace)
            {
                std::stringstream ss;

                ss << "send():\n" << jv.dump(1) << "\n";

                spdlog::trace(ss.str());
            }

            send(shared::make_message(jv));
        }

        template <class Derived>
        void ws_session_base<Derived>::send(message m)
        {
            boost::asio::dispatch(
                impl()->ws().get_executor(),
                boost::beast::bind_front_handler(&ws_session_base::do_send, boost::shared_from(this), std::move(m)));
        }

        template <class Derived>
        void ws_session_base<Derived>::do_send(message m)
        {
            if (!boost::beast::get_lowest_layer(impl()->ws()).socket().is_open())
                return;
            mq_.emplace_back(std::move(m));
            if (mq_.size() == 1)
                do_write();
        }

        template <class Derived>
        void ws_session_base<Derived>::do_write()
        {
            BOOST_ASSERT(!mq_.empty());

            // auto mq_var = mq_.back();
            // std::stringstream sstr;
            // sstr << "send_lambda() : "
            //    << mq_var << std::endl;

            // spdlog::debug(sstr.str());

            impl()->ws().async_write(
                mq_.back(),
                boost::beast::bind_front_handler(&ws_session_base::on_write, boost::shared_from(this), mq_.size() - 1));
        }

        template <class Derived>
        void ws_session_base<Derived>::on_write(std::size_t idx, boost::beast::error_code ec, std::size_t)
        {
            BOOST_ASSERT(!mq_.empty());
            if (ec)
                return fail(ec, "on_write");
            auto const last = mq_.size() - 1;
            if (idx != last)
                swap(mq_[idx], mq_[last]);
            mq_.resize(last);
            if (!mq_.empty())
                do_write();
        }

        template <class Derived>
        http_session_base<Derived>::http_session_base(
            application_impl_base& srv,
            listener&              lst,
            boost::asio::ip::tcp::endpoint          ep,
            boost::beast::flat_buffer           storage) :
            srv_(srv),
            lst_(lst), ep_(ep), storage_(std::move(storage))
        {
            lst_.insert(this);

            // set_run_session_functions(shared::run_https_session, shared::run_http_session);
        }

        template <class Derived>
        http_session_base<Derived>::~http_session_base()
        {
            lst_.erase(this);
        }

        boost::beast::string_view mime_type(boost::beast::string_view path);



        // This function produces an HTTP response for the given
        // request. The type of the response object depends on the
        // contents of the request, so the interface requires the
        // caller to pass a generic lambda for receiving the response.
        template <class Body, class Allocator, class Send>
        void handle_request(
            boost::optional<boost::filesystem::path> doc_root,
            std::vector<std::regex>                  epjs_extensions,
            std::function<std::string(std::string resolved_path, std::string doc_root, std::string target)>
                                                                 epjs_process_req_impl_,
            boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& req,
            Send&&                                               send)
        {
            // TODO. KP. Block here and send file or callback()

            std::stringstream sstr;

           // sstr << "\nweb_root: " << doc_root->generic_string() << std::endl << req << std::endl;
            sstr << "\nweb_root: " << doc_root->generic_string() << std::endl;

            spdlog::debug(sstr.str());

            // Returns a bad request response
            auto const bad_request = [&req](boost::beast::string_view why) {
                boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::bad_request, req.version()};
                res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(boost::beast::http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = why.to_string();
                res.prepare_payload();
                return res;
            };

            // Returns a not found response
            auto const not_found = [&req](boost::beast::string_view target) {
                boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::not_found, req.version()};
                res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(boost::beast::http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = "The resource '" + target.to_string() + "' was not found.";
                res.prepare_payload();
                return res;
            };

            // Returns a server error response
            auto const server_error = [&req](boost::beast::string_view what) {
                boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::internal_server_error, req.version()};
                res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(boost::beast::http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = "An error occurred: '" + what.to_string() + "'";
                res.prepare_payload();
                return res;
            };

            // Make sure we can handle the method
            if (req.method() != boost::beast::http::verb::get && req.method() != boost::beast::http::verb::head)
                return send(bad_request("Unknown HTTP-method"));

            // Request path must be absolute and not contain "..".
            if (req.target().empty() || req.target()[0] != '/' || req.target().find("..") != boost::beast::string_view::npos)
                return send(bad_request("Illegal request-target"));

            // Build the path to the requested file
            auto curr_path = *doc_root / std::string(req.target());
            if (req.target().back() == '/')
                curr_path.append("index.html");

            std::string curr_path_str = curr_path.generic_string().c_str();
            bool        epjs_process  = false;
            for (auto r : epjs_extensions)
            {
                std::smatch sm;
                if (std::regex_match(curr_path_str, sm, r))
                {
                    epjs_process = true;
                    break;
                }
            }

            boost::beast::error_code ec;

            if (epjs_process)
            {
                // TODO: KP. Instead of opening a file, run it through a V8 parser callback then return the content.

                // TODO: KP. execute injected callback function here ( which calls V8 )

                std::string output;

                // Call the injected implementation here
                if (epjs_process_req_impl_ != nullptr)
                {
                    output =
                        epjs_process_req_impl_(curr_path_str, doc_root->generic_string(), std::string(req.target()));
                }
                else
                {
                    // Missing implementation
                    throw std::logic_error("Missing epjs_process_req");
                }

                boost::beast::http::response<boost::beast::http::string_body> res;
                res.version(req.version());
                res.result(boost::beast::http::status::ok);
                res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                // res.set(boost::beast::http::field::content_type, mime_type(curr_path.generic_string()));
                // res.content_length(size);
                res.keep_alive(req.keep_alive());
                res.body() = output;
                res.prepare_payload();

                return send(std::move(res));
            }
            else
            {
                // Attempt to open the file
                boost::beast::http::file_body::value_type body;
                body.open(curr_path_str.c_str(), boost::beast::file_mode::scan, ec);

                // Handle the case where the file doesn't exist
                if (ec == boost::system::errc::no_such_file_or_directory)
                    return send(not_found(req.target()));

                // Handle an unknown error
                if (ec)
                    return send(server_error(ec.message()));

                // Cache the size since we need it after the move
                auto const size = body.size();

                // Respond to HEAD request
                if (req.method() == boost::beast::http::verb::head)
                {
                    boost::beast::http::response<boost::beast::http::empty_body> res{boost::beast::http::status::ok, req.version()};
                    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                    res.set(boost::beast::http::field::content_type, mime_type(curr_path.generic_string()));
                    res.content_length(size);
                    res.keep_alive(req.keep_alive());
                    return send(std::move(res));
                }

                // Respond to GET request
                boost::beast::http::response<boost::beast::http::file_body> res{std::piecewise_construct,
                                                    std::make_tuple(std::move(body)),
                                                    std::make_tuple(boost::beast::http::status::ok, req.version())};
                res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(boost::beast::http::field::content_type, mime_type(curr_path.generic_string()));
                res.content_length(size);
                res.keep_alive(req.keep_alive());
                return send(std::move(res));
            }
        }

        void ep_make_req(boost::beast::http::request_parser<boost::beast::http::string_body>& pr);

        template <class Derived>
        void ws_session_base<Derived>::operator()(boost::beast::error_code ec, std::size_t bytes_transferred)
        {
#include <boost/asio/yield.hpp>
            boost::ignore_unused(bytes_transferred);
            reenter(*this)
            {
                // Report any handshaking errors
                if (ec)
                    return fail(ec, "async_accept");

                for (;;)
                {
                    // Read the next message
                    yield impl()->ws().async_read(msg_, bind_front(impl()));

                    auto const cb       = msg_.data();
                    auto       data_obj = static_cast<char const*>(cb.data());

                    // std::stringstream sstr;
                    // sstr << "wu_user() : "
                    //    << data_obj << std::endl;

                    // spdlog::debug(sstr.str());

                    // Report any errors reading
                    if (ec)
                        return fail(ec, "async_read");

                    // Parse the buffer into JSON
                    auto js_str = std::string(data_obj, cb.size());

                    spdlog::debug("ws_user() : JS string received\n{}\n", js_str);

                    nlohmann::basic_json<> jv;

                    try
                    {
                        jv = nlohmann::json::parse(js_str);
                    }
                    catch (nlohmann::json::parse_error& e)
                    {
                        // output exception information
                        std::stringstream err_str;
                        err_str << "parse-json: " << e.what() << '\n'
                                << "exception id: " << e.id << '\n'
                                << "byte position of error: " << e.byte << std::endl;

                        spdlog::error(err_str.str());

                        ec = boost::system::errc::make_error_code(boost::system::errc::invalid_argument);

                        return fail(ec, err_str.str().c_str());
                    }

                    // Validate and extract the JSON-RPC request
                    shared::rpc_call rpc;
                    rpc.extract(std::move(jv), ec);
                    try
                    {
                        if (ec)
                            rpc.fail(rpc_code::invalid_request, ec.message());

                        // Dispatch via callback from worker
                        auto d    = get_dispatch_impl();
                        auto sess = get_wrapper();
                        d(rpc, *(sess->get_primary_user()), *sess);
                    }
                    catch (shared::rpc_error const& e)
                    {
                        std::function<void(nlohmann::json)> f1 = [this](nlohmann::json j) { this->send(j); };

                        rpc.complete(e, f1);
                    }

                    // Clear the buffer for the next message
                    msg_.clear();
                }
            }
#include <boost/asio/unyield.hpp>
        }
        /** Base for polymorphic connections

        Every session must be owned by one listener
        */

    } // namespace shared
} // namespace service
} // namespace easyprospect

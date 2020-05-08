

#include <easyprospect-service-shared/session.hpp>

#include <boost/asio/yield.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <iostream>

#include <spdlog/spdlog.h>
#include <easyprospect-service-shared/easyprospect-http-request.h>

using namespace easyprospect::service::shared;

namespace easyprospect
{
namespace service
{
    namespace shared
    {
        template <class Derived>
        void ws_session_base<Derived>::set_wrapper(boost::shared_ptr<ws_session_t> w)
        {
            wrapper_ = w;
        }

        template <class Derived>
        boost::shared_ptr<ws_session_t> ws_session_base<Derived>::get_wrapper()
        {
            return wrapper_;
        }

        boost::optional<boost::filesystem::path> session::doc_root() const
        {
            return doc_root_;
        }

        void ws_session_t::run(websocket::request_type req)
        {
            if (plain_ptr_)
            {
                plain_ptr_->run(req);
            }
        }

        void ws_session_t::set_dispatch_impl(std::function<void(rpc_call&, shared::user&, ws_session_t&)> val)
        {
            if (plain_ptr_)
            {
                plain_ptr_->set_dispatch_impl(val);
            }
        }

        void ws_session_t::set_epjs_process_req_impl(epjs_process_req_impl_type val)
        {
            if (plain_ptr_)
            {
                plain_ptr_->set_epjs_process_req_impl(val);
            }
        }

        void ws_session_t::set_wrapper(boost::shared_ptr<ws_session_t> w)
        {
            if (plain_ptr_)
            {
                plain_ptr_->set_wrapper(w);
            }
        }

        auto ws_session_t::get_primary_user() const
        {
            return primary_user_;
        }

        plain_http_session_impl::plain_http_session_impl(
            application_impl_base& srv,
            listener&              lst,
            stream_type            stream,
            endpoint_type          ep,
            flat_storage           storage) :
            http_session_base(srv, lst, ep, std::move(storage)),
            stream_(std::move(stream))
        {

            doc_root_ = srv.get_doc_root();
            epjs_exts_ = srv.get_epjs_url_path_regex();
            set_dispatch_impl(srv.get_dispatch_impl());
            set_epjs_process_req_impl( srv.get_epjs_process_req_impl());
            set_send_worker_req_impl( srv.get_send_worker_req_impl());
        }

        ssl_http_session_impl::ssl_http_session_impl(
            application_impl_base& srv,
            listener&              lst,
            asio::ssl::context&    ctx,
            stream_type            stream,
            endpoint_type          ep,
            flat_storage           storage) :
            http_session_base(srv, lst, ep, std::move(storage)),
            stream_(std::move(stream), ctx)
        {
            doc_root_ = srv.get_doc_root();
        }

        void ep_make_req(http::request_parser<http::string_body>& pr)
        {
            std::stringstream out;

            out << pr.get().base();
            spdlog::debug(out.str());

            // std::stringstream rawStr;
            // rawStr << pr;

            boost::optional<uint64_t> cl;

            auto rq = pr.get();
            
            rq.content_length(cl);
            auto rt = rq[http::field::content_type];

            auto b = rq.body();

            auto u = rq.target();

            spdlog::debug(b);
        }

        void ws_session_t::send(nlohmann::json const& jv)
        {
            if (plain_ptr_)
            {
                plain_ptr_->send(jv);
            }
        }

        void ws_session_t::send(message m)
        {
            if (plain_ptr_)
            {
                plain_ptr_->send(m);
            }
        }

         template <class Derived>
        void http_session_base<Derived>::operator()(beast::error_code ec, std::size_t bytes_transferred, bool need_eof)
        {
            boost::ignore_unused(bytes_transferred);
            reenter(*this)
            {
                // Set the expiration
                impl()->expires_after(std::chrono::seconds(30));

                // A new HTTP parser is required for each message
                pr_.emplace();

                // Set some limits to discourage attackers.
                pr_->body_limit(64 * 1024);
                pr_->header_limit(2048);

                //// 1. Read from impl()->stream()
                //// 2. Save in a local buffer and also send the the Beast request parser
                //// 3. Check the parser for an arbitrary header
                //// 4. Decide to if to stop parsing the stream
                //// 5. If parsing the stream is done send all data to a selected open socket
                //// 6. continue reading impl()->stream(), and passing data directly to that open socket
                //// bool header_found = false;
                // do
                //{
                //    // auto& strm = impl()->stream();
                //    // beast::ssl_stream<stream_type> strm = impl()->stream();
                //
                //    yield impl()->stream().async_read_some(storage_.prepare(100), bind_front(this));
                //    if (ec)
                //    {
                //        spdlog::debug(ec.message());
                //    }
                //    storage_.commit(bytes_transferred);
                //
                //    spdlog::debug(storage_.size());
                //
                //    ec = boost::system::error_code();
                //
                //    // TODO: KP. check error code
                //    // auto pr_res = pr_->put(storage_, ec);
                //    pr_->put(storage_.cdata(), ec);
                //    if (ec)
                //    {
                //        spdlog::debug(ec.message());
                //    }
                //    spdlog::debug((char*)(storage_.data().data()));
                //    spdlog::debug(storage_.capacity());
                //    spdlog::debug(storage_.size());
                //    // ep_make_req(*pr_);
                //    // TODO: KP. Check for ec code, keep in mind header may be invalid
                //    // TODO: KP. Check header for arbitrary header, set "header_found" boolean variable or break
                //    ;
                //    // } while (!(pr_->is_header_done() && header_found));
                //} while (!(pr_->is_header_done()));

                // // Read the next HTTP request
                yield http::async_read(impl()->stream(), storage_, *pr_, bind_front(this));

                // This means they closed the connection
                if (ec == http::error::end_of_stream)
                {
                    return impl()->do_close();
                }

                // Handle the error, if any
                if (ec)
                    return impl()->fail(ec, "http::async_read");

                // TODO: KP. PROXY the HTTP request to a process here
                if (send_worker_req_impl_)
                {
                    send_worker_req_impl_(easyprospect_http_request_builder{*pr_}.to_request(), ec);
                }

                // if (1)
                //{
                //    // Write the existing read data
                //    // TODO: Kp. Pass the parser to get_proxy_stream for routing logic
                //  //  yield this->get_proxy_stream(impl()->stream()).async_write(storage_.cdata(), bind_front(this));
                //    if (ec)
                //    {
                //        spdlog::debug(ec.message());
                //    }

                //    while (!ec) // Stop when message is done
                //    {
                //        // Read then write loop?

                //        // TODO: Kp. How do we detect HTTP message boundaries?
                //        yield impl()->stream().async_read_some(storage_.prepare(100), bind_front(this));
                //        if (ec)
                //        {
                //            spdlog::debug(ec.message());
                //        }
                //        storage_.commit(bytes_transferred);

                //       // yield this->get_proxy_stream(impl()->stream())
                //        //    .async_write_some(storage_.cdata(), bind_front(this));
                //        if (ec)
                //        {
                //            spdlog::debug(ec.message());
                //        }
                //    }
                //}

                // From here and below is really about the worker

                // See if it is a WebSocket Upgrade
                if (websocket::is_upgrade(pr_->get()))
                {
                    // Turn off the expiration timer
                    impl()->expires_never();

                    // Convert the request type
                    websocket::request_type req(pr_->release());

                    // Create a WebSocket session by transferring the socket

                    // EPSRV run_ws_session simply forwards to workers
                    // WORKER run_ws_session does some actual work

                    return this->run_ws_session(doc_root_, lst_, std::move(impl()->stream()), ep_, std::move(req));

                    // return run_ws_session(
                    //    srv_, lst_,
                    //    std::move(impl()->stream()),
                    //    ep_,
                    //    std::move(req));
                }
                // Send the response
                // yield handle_request(doc_root_, epjs_exts_, epjs_process_req_impl_, pr_->release(),
                // send_lambda{*this});
                yield handle_request(doc_root_, epjs_exts_, epjs_process_req_impl_, pr_->release(), [this](auto&& msg) {
                    std::stringstream sstr;
                    sstr << "send_lambda() : " << msg.base() << std::endl;

                    spdlog::debug(sstr.str());

                    // The lifetime of the message has to extend
                    // for the duration of the async operation so
                    // we use a shared_ptr to manage it.
                    // auto sp = std::make_shared<http::message<isRequest, Body, Fields>>(std::move(msg));
                    auto sp =
                        std::make_shared<std::remove_reference_t<decltype(msg)>>(std::forward<decltype(msg)>(msg));

                    // Write the response
                    auto self = bind_front(this);
                    http::async_write(
                        this->impl()->stream(), *sp, [self, sp](beast::error_code ec, std::size_t bytes_transferred) {
                            self(ec, bytes_transferred, sp->need_eof());
                        });
                });

                // Handle the error, if any
                if (ec)
                    return impl()->fail(ec, "http::async_write");

                if (need_eof)
                {
                    // This means we should close the connection, usually because
                    // the response indicated the "Connection: close" semantic.
                    return impl()->do_close();
                }
            }
        }

        
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
            http::request<Body, http::basic_fields<Allocator>>&& req,
            Send&&                                               send)
        {
            // TODO. KP. Block here and send file or callback()

            std::stringstream sstr;

            sstr << "\nweb_root: " << doc_root->generic_string() << std::endl << req << std::endl;

            spdlog::debug(sstr.str());

            // Returns a bad request response
            auto const bad_request = [&req](beast::string_view why) {
                http::response<http::string_body> res{http::status::bad_request, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = why.to_string();
                res.prepare_payload();
                return res;
            };

            // Returns a not found response
            auto const not_found = [&req](beast::string_view target) {
                http::response<http::string_body> res{http::status::not_found, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = "The resource '" + target.to_string() + "' was not found.";
                res.prepare_payload();
                return res;
            };

            // Returns a server error response
            auto const server_error = [&req](beast::string_view what) {
                http::response<http::string_body> res{http::status::internal_server_error, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = "An error occurred: '" + what.to_string() + "'";
                res.prepare_payload();
                return res;
            };

            // Make sure we can handle the method
            if (req.method() != http::verb::get && req.method() != http::verb::head)
                return send(bad_request("Unknown HTTP-method"));

            // Request path must be absolute and not contain "..".
            if (req.target().empty() || req.target()[0] != '/' || req.target().find("..") != beast::string_view::npos)
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

            beast::error_code ec;

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

                http::response<http::string_body> res;
                res.version(req.version());
                res.result(http::status::ok);
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                // res.set(http::field::content_type, mime_type(curr_path.generic_string()));
                // res.content_length(size);
                res.keep_alive(req.keep_alive());
                res.body() = output;
                res.prepare_payload();

                return send(std::move(res));
            }
            else
            {
                // Attempt to open the file
                http::file_body::value_type body;
                body.open(curr_path_str.c_str(), beast::file_mode::scan, ec);

                // Handle the case where the file doesn't exist
                if (ec == boost::system::errc::no_such_file_or_directory)
                    return send(not_found(req.target()));

                // Handle an unknown error
                if (ec)
                    return send(server_error(ec.message()));

                // Cache the size since we need it after the move
                auto const size = body.size();

                // Respond to HEAD request
                if (req.method() == http::verb::head)
                {
                    http::response<http::empty_body> res{http::status::ok, req.version()};
                    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                    res.set(http::field::content_type, mime_type(curr_path.generic_string()));
                    res.content_length(size);
                    res.keep_alive(req.keep_alive());
                    return send(std::move(res));
                }

                // Respond to GET request
                http::response<http::file_body> res{std::piecewise_construct,
                                                    std::make_tuple(std::move(body)),
                                                    std::make_tuple(http::status::ok, req.version())};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, mime_type(curr_path.generic_string()));
                res.content_length(size);
                res.keep_alive(req.keep_alive());
                return send(std::move(res));
            }
        }

        
        // Return a reasonable mime type based on the extension of a file.
        beast::string_view mime_type(beast::string_view path)
        {
            using beast::iequals;
            auto const ext = [&path] {
                auto const pos = path.rfind(".");
                if (pos == beast::string_view::npos)
                    return beast::string_view{};
                return path.substr(pos);
            }();
            if (iequals(ext, ".htm"))
                return "text/html";
            if (iequals(ext, ".html"))
                return "text/html";
            if (iequals(ext, ".php"))
                return "text/html";
            if (iequals(ext, ".css"))
                return "text/css";
            if (iequals(ext, ".txt"))
                return "text/plain";
            if (iequals(ext, ".js"))
                return "application/javascript";
            if (iequals(ext, ".json"))
                return "application/json";
            if (iequals(ext, ".xml"))
                return "application/xml";
            if (iequals(ext, ".swf"))
                return "application/x-shockwave-flash";
            if (iequals(ext, ".flv"))
                return "video/x-flv";
            if (iequals(ext, ".png"))
                return "image/png";
            if (iequals(ext, ".jpe"))
                return "image/jpeg";
            if (iequals(ext, ".jpeg"))
                return "image/jpeg";
            if (iequals(ext, ".jpg"))
                return "image/jpeg";
            if (iequals(ext, ".gif"))
                return "image/gif";
            if (iequals(ext, ".bmp"))
                return "image/bmp";
            if (iequals(ext, ".ico"))
                return "image/vnd.microsoft.icon";
            if (iequals(ext, ".tiff"))
                return "image/tiff";
            if (iequals(ext, ".tif"))
                return "image/tiff";
            if (iequals(ext, ".svg"))
                return "image/svg+xml";
            if (iequals(ext, ".svgz"))
                return "image/svg+xml";
            return "application/text";
        }

        template <class Derived>
        void http_session_base<Derived>::run_ws_session(
            boost::optional<boost::filesystem::path> doc_root,
            listener&                                lst,
            stream_type                              str,
            endpoint_type                            ep,
            websocket::request_type                  req)
        {
            //    shared::run_ws_session(srv, lst, std::move(str), ep, req);

            //       run_ws_session_plain_func(srv, lst, std::move(str), ep, req);

            // 2nd
            // auto sp = boost::make_shared<plain_ws_session_impl>(doc_root_, std::move(str), ep);
            // sp->run(std::move(req));

            // 3rd
            std::stringstream sstr;

            sstr << "run_ws_session(): " << req << std::endl;

            spdlog::debug(sstr.str());

            // auto sp = boost::make_shared<shared::plain_ws_session_impl>(srv.get_doc_root(), std::move(stream), ep);
            auto sp = boost::make_shared<shared::ws_session_t>(doc_root, std::move(str), ep);
            sp->set_dispatch_impl(get_dispatch_impl());
            sp->set_epjs_process_req_impl(get_epjs_process_req_impl());
            sp->set_wrapper(sp);
            sp->run(std::move(req));
        }

        template void http_session_base<ssl_http_session_impl>::operator()(
            beast::error_code ec,
            std::size_t       bytes_transferred,
            bool              need_eof);

        template void http_session_base<plain_http_session_impl>::operator()(
            beast::error_code ec,
            std::size_t       bytes_transferred,
            bool              need_eof);

    } // namespace shared
} // namespace service
} // namespace easyprospect
//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

#include "easyprospect-service-shared/listener.h"
#include "easyprospect-service-shared/server.h"
#include "easyprospect-service-shared/session.hpp"
#include "easyprospect-service-shared/utility.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <iostream>

#include <easyprospect-config/logging.h>

namespace easyprospect
{
namespace service
{
    namespace shared
    {

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

        // Append an HTTP rel-path to a local filesystem path.
        // The returned path is normalized for the platform.
        // std::string
        // path_cat(
        //    beast::string_view base,
        //    beast::string_view path)
        //{
        //    if(base.empty())
        //        return path.to_string();
        //    std::string result = base.to_string();
        //#if BOOST_MSVC
        //    char constexpr path_separator = '\\';
        //    if(result.back() == path_separator)
        //        result.resize(result.size() - 1);
        //    result.append(path.data(), path.size());
        //    for(auto& c : result)
        //        if(c == '/')
        //            c = path_separator;
        //#else
        //    char constexpr path_separator = '/';
        //    if(result.back() == path_separator)
        //        result.resize(result.size() - 1);
        //    result.append(path.data(), path.size());
        //#endif
        //    return result;
        //}

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

        // This function produces an HTTP response for the given
        // request. The type of the response object depends on the
        // contents of the request, so the interface requires the
        // caller to pass a generic lambda for receiving the response.
        template <class Body, class Allocator, class Send>
        void handle_request(
            boost::optional<boost::filesystem::path>             doc_root,
            std::vector<std::regex>                              epjs_extensions,
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
                if ( epjs_process_req_impl_ != nullptr )
                {
                    output = epjs_process_req_impl_(curr_path_str, doc_root->generic_string(), std::string(req.target()));
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

        using ep_session_ssl_function_type = std::function<void(
            application_impl_base&,
            shared::listener&,
            beast::ssl_stream<stream_type>,
            endpoint_type,
            websocket::request_type)>;

        using ep_session_plain_function_type = std::function<
            void(application_impl_base&, shared::listener&, stream_type, endpoint_type, websocket::request_type)>;

        //------------------------------------------------------------------------------

        //------------------------------------------------------------------------------

        void run_http_session(
            application_impl_base& srv,
            listener&              lst,
            stream_type            stream,
            endpoint_type          ep,
            flat_storage           storage)
        {
            auto sp = boost::make_shared<plain_http_session_impl>(srv, lst, std::move(stream), ep, std::move(storage));
            sp->run();
        }

        void run_https_session(
            application_impl_base& srv,
            listener&              lst,
            asio::ssl::context&    ctx,
            stream_type            stream,
            endpoint_type          ep,
            flat_storage           storage)
        {
            auto sp =
                boost::make_shared<ssl_http_session_impl>(srv, lst, ctx, std::move(stream), ep, std::move(storage));
            sp->run();
        }

    } // namespace shared
} // namespace service
} // namespace easyprospect
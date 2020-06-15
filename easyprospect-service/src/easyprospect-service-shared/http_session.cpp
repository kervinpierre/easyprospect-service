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


        // Append an HTTP rel-path to a local filesystem path.
        // The returned path is normalized for the platform.
        // std::string
        // path_cat(
        //    boost::beast::string_view base,
        //    boost::beast::string_view path)
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

        using ep_session_ssl_function_type = std::function<void(
            application_impl_base&,
            shared::listener&,
            boost::beast::ssl_stream<stream_type>,
            boost::asio::ip::tcp::endpoint,
            boost::beast::websocket::request_type)>;

        using ep_session_plain_function_type = std::function<
            void(application_impl_base&, shared::listener&, stream_type, boost::asio::ip::tcp::endpoint, boost::beast::websocket::request_type)>;

        //------------------------------------------------------------------------------

        //------------------------------------------------------------------------------

        void run_http_session(
            application_impl_base& srv,
            listener&              lst,
            stream_type            stream,
            boost::asio::ip::tcp::endpoint          ep,
            boost::beast::flat_buffer           storage)
        {
            auto sp = boost::make_shared<plain_http_session_impl>(srv, lst, std::move(stream), ep, std::move(storage));
            sp->run();
        }

        void run_https_session(
            application_impl_base& srv,
            listener&              lst,
            boost::asio::ssl::context&    ctx,
            stream_type            stream,
            boost::asio::ip::tcp::endpoint          ep,
            boost::beast::flat_buffer           storage)
        {
            auto sp =
                boost::make_shared<ssl_http_session_impl>(srv, lst, ctx, std::move(stream), ep, std::move(storage));
            sp->run();
        }

    } // namespace shared
} // namespace service
} // namespace easyprospect
//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

#pragma once

#include <easyprospect-service-shared/config.hpp>

#include <boost/beast/core/basic_stream.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/string_body.hpp>

#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/system_executor.hpp>
#include <cstdlib>

/*
    All the common types used by the server are
    together here so they may be easily changed.
*/

/// The type of executor agents and sessions will use
#ifdef LOUNGE_USE_SYSTEM_EXECUTOR
using executor_type = net::strand<
    net::system_executor>;
#else
using executor_type = net::strand<
    net::io_context::executor_type>;
#endif

/// The type of socket for agents to use
using socket_type =
    net::basic_stream_socket<tcp, executor_type>;

/// The type of plain stream for sessions to use
#if 1
using stream_type =
    beast::basic_stream<tcp, executor_type>;
#else
using stream_type = socket_type;
#endif

/// The type of timers
using timer_type =
    net::basic_waitable_timer<
        std::chrono::steady_clock,
        net::wait_traits<std::chrono::steady_clock>,
        executor_type>;

/// The type of flat storage to use
using flat_storage = beast::flat_buffer;

/// The type of network endpoint
using endpoint_type = tcp::endpoint;


using epjs_process_req_impl_type =
    std::function<std::string(std::string resolved_path, std::string doc_root, std::string target)>;

using run_proxy_session_impl_type =
    std::function<std::string(http::request_parser<http::string_body>&&, net::const_buffer, beast::error_code&)>;

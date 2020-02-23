//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

#pragma once

#include "config.hpp"
#include "utility.hpp"
#include <boost/beast/ssl/ssl_stream.hpp>
#include <easyprospect-web-worker/listener.hpp>
#include <easyprospect-service-shared/easyprospect-service-shared.h>

namespace easyprospect
{
    namespace service
    {
        namespace web_worker
        {
                void
                run_http_session(
                    server& srv,
                    listener& lst,
                    stream_type stream,
                    endpoint_type ep,
                    flat_storage storage);

                void
                run_https_session(
                    server& srv,
                    listener& lst,
                    asio::ssl::context& ctx,
                    stream_type stream,
                    endpoint_type ep,
                    flat_storage storage);
        }
    }
}
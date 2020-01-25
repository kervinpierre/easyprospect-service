//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

#include "easyprospect-web/message.hpp"
#include <boost/beast/core/flat_static_buffer.hpp>
#include <nlohmann/json.hpp>

//------------------------------------------------------------------------------

message
make_message(nlohmann::json const& jv)
{
    char buf[16384];
    auto jv_buf = jv.dump();
    std::strcpy(buf, jv_buf.c_str());

    return message(net::const_buffer(buf, jv_buf.length()));
}


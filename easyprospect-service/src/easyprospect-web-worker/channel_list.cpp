//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

#include "easyprospect-web-worker/channel.hpp"
#include "easyprospect-web-worker/channel_list.hpp"
#include "easyprospect-web-worker/message.hpp"
#include "easyprospect-web-worker/rpc.hpp"
#include "easyprospect-web-worker/server.hpp"
#include "easyprospect-web-worker/service.hpp"
#include "easyprospect-web-worker/user.hpp"
#include <boost/container/flat_set.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/shared_lock_guard.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/smart_ptr/make_unique.hpp>
#include <limits>
#include <mutex>
#include <vector>

namespace easyprospect
{
    namespace service
    {
        namespace web_worker
        {

            std::unique_ptr<channel_list>
                make_channel_list(server& srv)
            {
                return boost::make_unique<
                    channel_list_impl>(srv);
            }

        }
    }
}

//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//
#pragma once

#include "easyprospect-web-worker/channel.hpp"
#include "easyprospect-web-worker/rpc.hpp"
#include "easyprospect-web-worker/user.hpp"

namespace easyprospect
{
    namespace service
    {
        namespace web_worker
        {

            class room_impl : public channel
            {
            public:
                room_impl(
                    beast::string_view name,
                    channel_list& list)
                    : channel(
                        2,
                        name,
                        list)
                {
                }

                //--------------------------------------------------------------------------
                //
                // channel
                //
                //--------------------------------------------------------------------------

                void
                    on_insert(user&) override
                {
                }

                void
                    on_erase(user&) override
                {
                }

                void
                    on_dispatch(rpc_call& rpc) override
                {
                    if (rpc.method == "say")
                    {
                        do_say(rpc);
                    }
                    else if (rpc.method == "slash")
                    {
                        do_say(rpc);
                    }
                    else
                    {
                        rpc.fail(rpc_code::method_not_found);
                    }
                }

                //--------------------------------------------------------------------------
                //
                // room_impl
                //
                //--------------------------------------------------------------------------

                void
                    do_say(rpc_call& rpc)
                {
                    checked_user(rpc);
                    if (!is_joined(*rpc.u))
                        rpc.fail("not in channel");
                    auto const& text =
                        checked_string(rpc.params, "message");
                    {
                        // broadcast: say
                        nlohmann::json jv;
                        jv["verb"] = "say";
                        jv["cid"] = cid();
                        jv["name"] = name();
                        jv["user"] = rpc.u->name;
                        jv["message"] = text;
                        send(jv);
                    }
                    rpc.complete();
                }

                void
                    do_slash(rpc_call& rpc)
                {
                    checked_user(rpc);
                    rpc.fail("Unimplemented");
                }
            };


            void make_room(
                    channel_list& list,
                    beast::string_view name);
        }
    }
}
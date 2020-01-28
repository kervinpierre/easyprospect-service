#pragma once
#include "easyprospect-web-worker/rpc.hpp"
#include "easyprospect-web-worker/channel.hpp"
#include "easyprospect-web-worker/channel_list.hpp"
#include "easyprospect-web-worker/server.hpp"
#include "easyprospect-web-worker/user.hpp"
#include <boost/make_shared.hpp>

namespace easyprospect
{
    namespace service
    {
        namespace web_worker
        {

            class system_channel : public channel
            {
                server& srv_;

            public:
                explicit
                    system_channel(server& srv)
                    : channel(
                        1,
                        "System",
                        srv.channel_list())
                    , srv_(srv)
                {
                }

            protected:
                void
                    on_insert(user&) override
                {
                }

                void
                    on_erase(user&) override
                {
                }

                void
                    on_dispatch(
                        rpc_call& rpc) override
                {
                    if (rpc.method == "identify")
                    {
                        do_identify(rpc);
                    }
                    else if (rpc.method == "shutdown")
                    {
                        do_shutdown(rpc);
                    }
                    else if (rpc.method == "stop")
                    {
                        do_stop(rpc);
                    }
                    else
                    {
                        rpc.fail(rpc_code::method_not_found);
                    }
                }

                void
                    do_identify(rpc_call& rpc)
                {
                    auto const& name =
                        checked_string(rpc.params, "name");
                    if (name.size() > 20)
                        rpc.fail("Invalid \"name\": too long");
                    if (!rpc.u->name.empty())
                        rpc.fail("Identity is already set");
                    // VFALCO NOT THREAD SAFE!
                    rpc.u->name.assign(name.data(), name.size());
                    insert(*rpc.u);
                    rpc.complete();
                }

                void
                    do_shutdown(rpc_call& rpc)
                {
                    // TODO check user perms
                    boost::ignore_unused(rpc);
                    srv_.shutdown(
                        std::chrono::seconds(30));
                    rpc.complete();
                }

                void
                    do_stop(rpc_call& rpc)
                {
                    // TODO check user perms
                    boost::ignore_unused(rpc);
                    srv_.stop();
                    rpc.complete();
                }
            };

        }
    }
}
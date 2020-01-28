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
#include "uid.hpp"
#include <cstdlib>
#include <boost/asio/buffer.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <utility>
#include <boost/smart_ptr/make_unique.hpp>
#include "server.hpp"
#include "rpc.hpp"
#include "system.hpp"
#include "room.hpp"

namespace easyprospect
{
    namespace service
    {
        namespace web_worker
        {
            class channel;
            class rpc_call;
            class user;

            //------------------------------------------------------------------------------

            class channel_list
            {
            public:
                virtual
                    ~channel_list() = default;

                virtual
                    uid_type
                    next_uid() noexcept = 0;

                virtual
                    std::size_t
                    next_cid() noexcept = 0;

                /// Return the channel for a cid, or nullptr
                virtual
                    boost::shared_ptr<channel>
                    at(std::size_t cid) const = 0;

                /// Process a serialized message from a user
                virtual
                    void
                    dispatch(rpc_call& rpc) = 0;

                template<class T, class...  Args>
                friend
                    void
                    insert(
                        channel_list& list,
                        Args&&... args);

                virtual
                    void
                    erase(channel const& c) = 0;

            private:
                virtual
                    void
                    insert(boost::shared_ptr<channel> c) = 0;
            };

            class channel_list_impl
                : public channel_list
                , public service
            {
                struct element
                {
                    boost::shared_ptr<channel> c;
                    std::size_t next = 0;
                };

                using mutex = boost::shared_mutex;
                using lock_guard = boost::lock_guard<mutex>;
                using shared_lock_guard = boost::shared_lock_guard<mutex>;

                server& srv_;
                mutex mutable m_;
                std::vector<element> v_;
                // VFALCO look into https://github.com/greg7mdp/parallel-hashmap
                boost::container::flat_set<channel*> users_;
                std::atomic<uid_type> next_uid_;
                std::atomic<std::size_t> next_cid_;

            public:
                channel_list_impl(
                    server& srv)
                    : srv_(srv)
                    , next_uid_(1000)
                    , next_cid_(1000)
                {
                    // element 0 is unused
                    v_.resize(1);

                    easyprospect::service::web_worker::make_room(*this, "General");
                }

                //--------------------------------------------------------------------------
                //
                // service
                //
                //--------------------------------------------------------------------------

                void
                    on_start() override
                {
                }

                void
                    on_stop() override
                {
                }

                //--------------------------------------------------------------------------
                //
                // channel_list
                //
                //--------------------------------------------------------------------------

                boost::shared_ptr<channel>
                    at(std::size_t cid) const override
                {
                    shared_lock_guard lock(m_);
                    if (cid >= v_.size())
                        return nullptr;
                    return v_[cid].c;
                }

                void
                    dispatch(rpc_call& rpc) override
                {
                    // Validate and extract the channel id
                    auto const cid =
                        checked_value(rpc.params, "cid")
                        .get<size_t>();

                    spdlog::debug("channel_list::dispatch() called for cid {}", cid);

                    // Lookup cid
                    auto c = at(cid);
                    if (!c)
                        rpc.fail(
                            rpc_code::invalid_params,
                            "Unknown cid");

                    // Dispatch the request
                    c->dispatch(rpc);
                }

                uid_type
                    next_uid() noexcept override
                {
                    return ++next_uid_;
                }

                std::size_t
                    next_cid() noexcept override
                {
                    return ++next_cid_;
                }

                void
                    insert(boost::shared_ptr<channel> c) override
                {
                    auto const cid = c->cid();
                    lock_guard lock(m_);
                    v_.resize(std::max<std::size_t>(
                        cid + 1, v_.size()));
                    BOOST_ASSERT(v_[cid].c == nullptr);
                    v_[cid].c = std::move(c);
                }

                void
                    erase(channel const& c) override
                {
                    auto const cid = c.cid();
                    lock_guard lock(m_);
                    BOOST_ASSERT(cid < v_.size());
                    v_[cid].c = nullptr;
                }

                //--------------------------------------------------------------------------
                //
                // channel_list_impl
                //
                //--------------------------------------------------------------------------
            };

            template<class T, class...  Args>
            void
                insert(
                    channel_list& list,
                    Args&&... args)
            {
                list.insert(boost::make_shared<T>(
                    std::forward<Args>(args)...));
            }

            std::unique_ptr<channel_list>
                make_channel_list(server& srv);
        }
    }
}

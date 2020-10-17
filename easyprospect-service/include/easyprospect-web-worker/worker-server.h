#pragma once

#include <boost/container/flat_set.hpp>
#include <boost/smart_ptr/enable_shared_from.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/shared_lock_guard.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <easyprospect-service-shared/server.h>
//
#include <easyprospect-service-shared/uid.hpp>

#include <easyprospect-service-shared/user_base.hpp>


#include <easyprospect-config/easyprospect-config-worker.h>
#include <easyprospect-config/easyprospect-registry.h>
#include <easyprospect-web-worker/easyprospect-process-cntrl-client.h>

namespace easyprospect
{
namespace service
{
    namespace web_worker
    {
        class channel_list;
        class channel_list_impl;
        class application_impl;
        class channel;

        
        std::unique_ptr<channel_list_impl> make_channel_list(web_worker::application_impl& srv);
        void                               make_room(channel_list& list, boost::beast::string_view name);
        extern std::unique_ptr<shared::server> make_server(config::easyprospect_config_worker_core curr_config,
                                                            std::shared_ptr<config::easyprospect_registry> curr_reg);
        extern std::unique_ptr<process_cntrl_client> make_control_server(
            config::easyprospect_config_service_core curr_config,
            std::shared_ptr<config::easyprospect_registry> curr_reg,
            std::function<void()> shutdown_func);
        extern void                            make_system_channel(application_impl& srv);
        extern void                            make_blackjack_service(web_worker::application_impl& srv);

        /// Represents a connected user
        class user : public shared::user_base
        {
            std::mutex                           mutex_;
            boost::container::flat_set<channel*> channels_;

          public:
            ~user();
            void user::on_insert(channel& c);
            void user::on_erase(channel& c);
        };

        class ws_user : public user
        {
        };

        class channel_list;

        class channel : public boost::enable_shared_from
        {
            using mutex             = boost::shared_mutex;
            using lock_guard        = boost::lock_guard<mutex>;
            using shared_lock_guard = boost::shared_lock_guard<mutex>;

            channel_list& list_;
            boost::shared_mutex mutable mutex_;

            boost::container::flat_set<shared::ws_session_t*> sessions_;
            // boost::container::flat_set<shared::ws_session_base<plain_ws_session_impl>*> sessions_plain_;
            // boost::container::flat_set<shared::ws_session_base<ssl_ws_session_impl>*> sessions_ssl_;

            uid_type    uid_;
            std::size_t cid_;
            std::string name_;

            friend channel_list;

          public:
            /// Return the channel unique-id
            uid_type uid() const noexcept
            {
                return uid_;
            }

            /// Return the channel id
            std::size_t cid() const noexcept
            {
                return cid_;
            }

            /// Return the channel name
            boost::beast::string_view name() const noexcept
            {
                return name_;
            }

            /// Returns `true` if the user has joined the channel

            bool is_joined(shared::user& u, shared::ws_session_t& sess) const;

            /** Add a user to the channel.

                @returns `false` if the user was already in the channel.
            */

            bool insert(shared::user& u, shared::ws_session_t& sess);

            /** Remove the user from the channel.

                @param u Weak ownership of the user to remove.
            */

            bool erase(shared::user& u, shared::ws_session_t& sess);

            void send(nlohmann::json const& jv);
            void send(shared::message m);

            /// Process an RPC command for this channel
            void dispatch(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess);

          protected:
            channel(std::size_t reserved_cid, boost::beast::string_view name);
            channel(boost::beast::string_view name, channel_list& list);
            channel(std::size_t reserved_cid, boost::beast::string_view name, channel_list& list);
            ~channel();

            // template<typename T>
            // typename std::enable_if<std::is_same<T, plain_ws_session_impl>::value,
            // boost::container::flat_set<shared::ws_session_base<plain_ws_session_impl>*>>::type get_sessions()
            // {
            //     return sessions_plain_;
            // }

            // template<typename T>
            // typename std::enable_if<std::is_same<T, ssl_ws_session_impl>::value,
            // boost::container::flat_set<shared::ws_session_base<ssl_ws_session_impl>*>>::type get_sessions()
            // {
            //     return sessions_ssl_;
            // }

            // channel::
            //    channel(
            //        std::size_t reserved_cid,
            //        boost::beast::string_view name)
            //    : cid_(reserved_cid)
            //    , name_(name)
            //{
            //    list_ = channel_list();
            //}

            void checked_user(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess);

            /** Called when a user is inserted to the channel's list.

                @param u A strong reference to the user.
            */

            virtual void on_insert(shared::user& u, shared::ws_session_t& sess) = 0;

            /** Called when a user is erased from the channel's list.

                @param u A weak reference to the user.
            */

            virtual void on_erase(shared::user& u, shared::ws_session_t& sess) = 0;

            /// Called on an RPC command

            virtual void on_dispatch(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess) = 0;

          private:
            void do_join(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess);
            void do_leave(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess);
        };

        class channel_list
        {
          public:
            virtual ~channel_list() = default;

            virtual uid_type next_uid() noexcept = 0;

            virtual std::size_t next_cid() noexcept = 0;

            /// Return the channel for a cid, or nullptr
            virtual boost::shared_ptr<channel> at(std::size_t cid) = 0;

            /// Process a serialized message from a user
            //
            virtual void dispatch(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess) = 0;

            template <class T, class... Args>
            friend void insert(channel_list& list, Args&&... args);

            virtual void erase(channel const& c) = 0;

          private:
            virtual void insert(boost::shared_ptr<channel> c) = 0;
        };

        class room_impl : public channel
        {
          public:
            room_impl(boost::beast::string_view name, channel_list& list) : channel(2, name, list)
            {
            }

            //--------------------------------------------------------------------------
            //
            // channel
            //
            //--------------------------------------------------------------------------

            void on_insert(shared::user& u, shared::ws_session_t& sess)
            {
            }

            void on_erase(shared::user& u, shared::ws_session_t& sess)
            {
            }

            void on_dispatch(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& ses)
            {
                if (rpc.method == "say")
                {
                    do_say(rpc, u, ses);
                }
                else if (rpc.method == "slash")
                {
                    do_say(rpc, u, ses);
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

            void do_say(shared::rpc_call& rpc, shared::user& u,
                        shared::ws_session_t& ses);

            void do_slash(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& ses)
            {
                checked_user(rpc, u, ses);
                rpc.fail("Unimplemented");
            }
        };

        class channel_list_impl : public channel_list
        {
            struct element
            {
                boost::shared_ptr<channel> c;
                std::size_t                next = 0;
            };

            using mutex             = boost::shared_mutex;
            using lock_guard        = boost::lock_guard<mutex>;
            using shared_lock_guard = boost::shared_lock_guard<mutex>;

            application_impl& srv_;
            mutex mutable m_;
            std::vector<element> v_;
            // VFALCO look into https://github.com/greg7mdp/parallel-hashmap
            boost::container::flat_set<channel*> users_;
            std::atomic<uid_type>                next_uid_;
            std::atomic<std::size_t>             next_cid_;

          public:
            channel_list_impl(application_impl& srv) : srv_(srv), next_uid_(1000), next_cid_(1000)
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

            //--------------------------------------------------------------------------
            //
            // channel_list
            //
            //--------------------------------------------------------------------------

            boost::shared_ptr<channel> at(std::size_t cid)
            {
                shared_lock_guard lock(m_);
                if (cid >= v_.size())
                    return nullptr;
                return v_[cid].c;
            }

            void dispatch(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess) override;

            uid_type next_uid() noexcept override
            {
                return ++next_uid_;
            }

            std::size_t next_cid() noexcept override
            {
                return ++next_cid_;
            }

            void insert(boost::shared_ptr<channel> c) override
            {
                auto const cid = c->cid();
                lock_guard lock(m_);
                v_.resize(std::max<std::size_t>(cid + 1, v_.size()));
                BOOST_ASSERT(v_[cid].c == nullptr);
                v_[cid].c = std::move(c);
            }

            void erase(channel const& c) override
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

        
        class system_channel : public channel
        {
            shared::server& srv_;

          public:
            explicit system_channel(web_worker::application_impl& srv);

          protected:
            void on_insert(shared::user& u, shared::ws_session_t& sess)
            {
            }

            void on_erase(shared::user& u, shared::ws_session_t& sess)
            {
            }

            void on_dispatch(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess)
            {
                if (rpc.method == "identify")
                {
                    do_identify(rpc, u, sess);
                }
                else if (rpc.method == "shutdown")
                {
                    do_shutdown(rpc, u, sess);
                }
                else if (rpc.method == "stop")
                {
                    do_stop(rpc, u, sess);
                }
                else
                {
                    rpc.fail(rpc_code::method_not_found);
                }
            }

            void do_identify(shared::rpc_call& rpc, shared::user& u,
                             shared::ws_session_t& ses);

            void do_shutdown(shared::rpc_call& rpc, shared::user& u,
                             shared::ws_session_t& ses);

            void do_stop(shared::rpc_call& rpc, shared::user& u,
                         shared::ws_session_t& ses);
        };

        
        class blackjack_service : public shared::service
        {
            web_worker::application_impl& srv_;

          public:
            blackjack_service(web_worker::application_impl& srv) : srv_(srv)
            {
            }

            //--------------------------------------------------------------------------
            //
            // service
            //
            //--------------------------------------------------------------------------

            void on_stop() override
            {
            }

            void on_start(executor_type exe) override;
        };
    }
} // namespace service
} // namespace easyprospect

//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

#include <easyprospect-web-worker/worker-server.h>
#include <easyprospect-web-worker/worker-server-app.h>
#include <easyprospect-web-worker/app-error.h>

#include <easyprospect-service-shared/rpc.hpp>
#include <easyprospect-service-shared/server.h>
#include <boost/asio/basic_signal_set.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/thread.hpp>
#include <easyprospect-service-shared/externs.h>
#include <easyprospect-service-shared/listener.h>
#include <easyprospect-service-shared/message.hpp>
#include <easyprospect-service-shared/uid.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <vector>

namespace easyprospect
{
namespace service
{
    namespace web_server
    {
        class server_impl;
    }
} // namespace service
} // namespace easyprospect

//------------------------------------------------------------------------------

namespace easyprospect
{
namespace service
{
    namespace web_worker
    {
        // channel::channel(
        //    std::size_t reserved_cid,
        //    boost::beast::string_view name)
        //{
        //
        //}

        channel::channel(boost::beast::string_view name, channel_list& list) :
            list_(list), uid_(list.next_uid()), cid_(list.next_cid()), name_(name)
        {
        }

        channel::channel(std::size_t reserved_cid, boost::beast::string_view name, channel_list& list) :
            list_(list), uid_(list.next_uid()), cid_(reserved_cid), name_(name)
        {
        }

        // channel::
        //    channel(
        //        std::size_t reserved_cid,
        //        boost::beast::string_view name)
        //    : cid_(reserved_cid)
        //    , name_(name)
        //{
        //    list_ = channel_list();
        //}

        channel::~channel()
        {
            // The proper way to delete a channel is
            // to first remove all the users, so they
            // get the notification.
            BOOST_ASSERT(sessions_.empty());

            list_.erase(*this);
        }



        bool channel::is_joined(shared::user& u, shared::ws_session_t& sess) const
        {
            shared_lock_guard lock(mutex_);
            return sessions_.find(&sess) != sessions_.end();
        }

        bool channel::insert(shared::user& u, shared::ws_session_t& sess)
        {
            {
                auto const inserted = [&] {
                    lock_guard lock(mutex_);
                    return sessions_.insert(&sess).second;
                }();
                if (!inserted)
                    return false;
            }
            {
                // broadcast: join
                nlohmann::json jv;
                jv["cid"]  = cid();
                jv["verb"] = "join";
                jv["name"] = name();
                jv["user"] = u.name;
                send(jv);
            }
            // u.on_insert(*this);
            on_insert(u, sess);
            return true;
        }

        bool channel::erase(shared::user& u, shared::ws_session_t& sess)
        {
            // First remove the user from the list
            {
                lock_guard lock(mutex_);
                if (sessions_.erase(&sess) == 0)
                    return false;
            }

            // Notify channel participants
            {
                // broadcast: leave
                nlohmann::json jv;
                jv["cid"]  = cid();
                jv["verb"] = "leave";
                jv["name"] = name();
                jv["user"] = u.name;
                send(jv);

                // Also notify the user, if
                // they are still connected.

                if (auto sp = boost::weak_from(&sess).lock())
                    sp->send(jv);
            }
            // u.on_erase(*this);
            on_erase(u, sess);
            return true;
        }

        void channel::send(nlohmann::json const& jv)
        {
            send(shared::make_message(jv));
        }

        void channel::dispatch(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess)
        {
            if (rpc.method == "join")
            {
                do_join(rpc, u, sess);
            }
            else if (rpc.method == "leave")
            {
                do_leave(rpc, u, sess);
            }
            else
            {
                on_dispatch(rpc, u, sess);
            }
        }

        void channel::checked_user(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess)
        {
            if (u.name.empty())
                rpc.fail("1");
        }

        void channel::do_join(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess)
        {
            std::function<void(nlohmann::json const&)> f1 = [&sess](nlohmann::json const& j) { sess.send(j); };

            checked_user(rpc, u, sess);
            if (!insert(u, sess))
                rpc.fail("Already in channel");
            rpc.complete(f1);
        }

        void channel::do_leave(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess)
        {
            std::function<void(nlohmann::json)> f1 = [&sess](nlohmann::json j) { sess.send(j); };

            if (!erase(u, sess))
                rpc.fail("Not in channel");
            rpc.complete(f1);
        }

        void channel::send(shared::message m)
        {
            {
                // Make a local list of all the weak pointers
                // representing the users, so we can do the
                // actual sending without holding the mutex:
                std::vector<boost::weak_ptr<shared::ws_session_t>> v;
                {
                    shared_lock_guard lock(mutex_);
                    v.reserve(sessions_.size());
                    for (auto p : sessions_)
                        v.emplace_back(boost::weak_from(p));
                }

                // For each user in our local list, try to
                // acquire a strong pointer. If successful,
                // then send the message to that user.
                for (auto const& wp : v)
                    if (auto sp = wp.lock())
                        sp->send(m);
            }
        }

        void room_impl::do_say(shared::rpc_call& rpc, shared::user& u,
                               shared::ws_session_t& ses)
        {
            checked_user(rpc, u, ses);
            if (!is_joined(u, ses))
                rpc.fail("not in channel");
            auto const& text = shared::checked_string(rpc.params, "message");
            {
                // broadcast: say
                nlohmann::json jv;
                jv["verb"] = "say";
                jv["cid"] = cid();
                jv["name"] = name();
                jv["user"] = u.name;
                jv["message"] = text;
                send(jv);
            }

            std::function<void(nlohmann::json)> f1 = [&ses](nlohmann::json j)
            {
                ses.send(j);
            };

            rpc.complete(f1);
        }

        void channel_list_impl::dispatch(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess)
        {
            // Validate and extract the channel id
            auto const cid = shared::checked_value(rpc.params, "cid").get<size_t>();

            spdlog::debug("channel_list::dispatch() called for cid {}", cid);

            // Lookup cid
            auto c = at(cid);
            if (!c)
                rpc.fail(rpc_code::invalid_params, "Unknown cid");

            // Dispatch the request
            c->dispatch(rpc, u, sess);
        }

        system_channel::system_channel(web_worker::application_impl& srv):
            channel(1, "System", srv.channel_list()), srv_(srv)
        {
        }

        void system_channel::do_identify(shared::rpc_call& rpc, shared::user& u,
                                         shared::ws_session_t& ses)
        {
            auto const& name = shared::checked_string(rpc.params, "name");
            if (name.size() > 20)
                rpc.fail("Invalid \"name\": too long");
            if (!u.name.empty())
                rpc.fail("Identity is already set");
            // VFALCO NOT THREAD SAFE!
            u.name.assign(name.data(), name.size());
            insert(u, ses);

            std::function<void(nlohmann::json)> f1 = [&ses](nlohmann::json j)
            {
                ses.send(j);
            };

            rpc.complete(f1);
        }

        void system_channel::do_shutdown(shared::rpc_call& rpc, shared::user& u,
                                         shared::ws_session_t& ses)
        {
            // TODO check user perms
            boost::ignore_unused(rpc);
            srv_.shutdown(std::chrono::seconds(30));
            rpc.complete([&ses](nlohmann::json j) { ses.send(j); });
        }

        void system_channel::do_stop(shared::rpc_call& rpc, shared::user& u,
                                     shared::ws_session_t& ses)
        {
            // TODO check user perms
            boost::ignore_unused(rpc);
            srv_.stop();
            rpc.complete([&ses](nlohmann::json j) { ses.send(j); });
        }

        void make_blackjack_service(web_worker::application_impl& srv);

        class shoe
        {
            std::vector<char>           cards_;
            std::vector<char>::iterator pos_;

          public:
            explicit shoe(int decks)
            {
                cards_.resize(decks * 52);
                shuffle();
            }

            void shuffle()
            {
                for (std::size_t i = 0; i < cards_.size(); ++i)
                {
                    cards_[i] = 1 + (i % 52);
                }
                for (std::size_t i = 0; i < cards_.size() - 1; ++i)
                {
                    auto const j = i + (rand() % (cards_.size() - i));
                    std::swap(cards_[i], cards_[j]);
                }
                pos_ = cards_.begin();
            }

            char deal()
            {
                if (pos_ == cards_.end())
                    shuffle();
                return *pos_++;
            }
        };

        //------------------------------------------------------------------------------

        struct hand
        {
            // cards[0]==0 for hole card
            boost::beast::static_string<22> cards;
            int                      wager      = 0;
            bool                     busted     = false;
            bool                     twenty_one = false;
            bool                     blackjack  = false;

            void clear()
            {
                cards.clear();
                wager      = 0;
                busted     = false;
                twenty_one = false;
                blackjack  = false;
            }

            // Returns the value of a card,
            // always returns 1 for aces
            static int value(char c)
            {
                auto v = 1 + ((c - 1) % 13);
                if (v > 10)
                    v = 10;
                return v;
            }

            void eval()
            {
                int aces  = 0;
                int total = 0;
                for (auto c : cards)
                {
                    if (c != 1)
                    {
                        total += c;
                    }
                    else
                    {
                        ++aces;
                        total += 11;
                    }
                }
                while (total > 21 && aces--)
                    total -= 10;

                busted     = total > 21;
                twenty_one = total == 21;
                blackjack  = twenty_one && cards.size() == 2;
            }

            void deal(shoe& s)
            {
                cards.push_back(s.deal());
            }

            bool is_finished() const
            {
                return busted || twenty_one;
            }
        };

        void to_json(nlohmann::json& j, const hand& h);

        //------------------------------------------------------------------------------

        // Represents one of the five seating
        // positions of the blackjack playfield.
        //
        struct seat
        {
            enum state_t
            {
                dealer,
                waiting,
                playing,
                leaving,
                open
            };

            shared::user*     u     = nullptr;
            state_t           state = open;
            std::vector<hand> hands;
            int               chips;
            int               wager = 0;

            seat()
            {
                hands.resize(1);
            }

            void clear()
            {
                hands[0].clear();
                wager = 0;
                chips = 1000;
            }
        };

        void to_json(nlohmann::json& j, const seat& s);

        //------------------------------------------------------------------------------

        class game
        {
          public:
            enum class state
            {
                wait,
                play
            };

            std::vector<seat> get_seat() const
            {
                return seat_;
            }

          private:
            state s_ = state::wait;

          public:
            /// Receives notifications of game events
            struct callback
            {
                virtual ~callback() = default;

                // Wager is adjusted
                virtual void on_game_bet() = 0;

                // A card is dealt
                virtual void on_game_deal_card() = 0;
            };

            game(callback& cb, int decks) : cb_(cb), shoe_(decks)
            {
                BOOST_ASSERT(decks >= 1 && decks <= 6);
                seat_.resize(6);
                seat_[0].state = seat::state_t::dealer;
            }

            state get_state() const
            {
                return s_;
            }

            // Advance the game state, invoking
            // any appropriate callbacks.
            void tick()
            {
            }

            // Join the game as a player.
            // Returns seat assignment on success
            int64_t join(shared::user& u, boost::beast::error_code& ec)
            {
                if (find(u) != 0)
                {
                    ec = make_error_code(error::already_playing);
                    return 0;
                }
                for (auto& s : get_seat())
                {
                    if (s.state == seat::open)
                    {
                        s.u     = &u;
                        s.state = seat::playing;
                        ec.clear();
                        return &s - &get_seat().front();
                    }
                }
                ec = error::no_open_seat;
                return 0;
            }

            // Leave the game as a player.
            //  1 = now leaving, was playing
            //  2 = now open, was waiting
            int leave(shared::user& u, boost::beast::error_code& ec)
            {
                auto const i = find(u);
                if (!i)
                {
                    ec = error::not_playing;
                    return 0;
                }
                switch (get_seat()[i].state)
                {
                case seat::waiting:
                    get_seat()[i].state = seat::open;
                    return 2;

                case seat::playing:
                    get_seat()[i].state = seat::leaving;
                    // TEMPORARY
                    get_seat()[i].clear();
                    return 1;

                default:
                    BOOST_ASSERT(get_seat()[i].state == seat::leaving);
                    ec = error::already_leaving;
                    return 0;
                }
            }

            // Surrender the hand and leave
            //  1 = success
            // -1 = not playing
            int surrender(shared::user& u, shared::ws_session_t& sess)
            {
                auto const i = find(u);
                if (!i)
                    return -1;
                get_seat()[i].state = seat::open;
                return 1;
            }

            void bet(shared::user& u, boost::beast::error_code& ec)
            {
                auto const i = find(u);
                if (!i)
                {
                    ec = error::not_playing;
                    return;
                }
                switch (seat_[i].state)
                {
                default:
                    ec = error::no_more_bets;
                    return;

                case seat::playing:
                case seat::leaving:
                    break;
                }
                int const size = 5;
                seat_[i].wager += size;
                seat_[i].chips -= size;
                cb_.on_game_bet();
            }

            void start(boost::beast::error_code& ec)
            {
                deal_one();
                ec = {};
            }

          private:
            callback& cb_;
            shoe      shoe_;

            std::vector<seat> seat_; // 0 = dealer
            std::size_t       turn_ = 0;

            std::size_t find(shared::user const& u) const
            {
                for (auto const& s : seat_)
                    if (s.state != seat::open && s.u == &u)
                        return &s - &seat_.front();
                return 0;
            }

            std::size_t insert(shared::user& u)
            {
                for (auto& s : seat_)
                    if (s.state == seat::open)
                    {
                        s.state = seat::waiting;
                        s.u     = &u;
                        return &s - &seat_.front();
                    }
                return 0;
            }

            void deal_one()
            {
                seat_[turn_++].hands[0].deal(shoe_);
                if (turn_ >= seat_.size())
                    turn_ = 0;
                cb_.on_game_deal_card();
            }
        };

        void to_json(nlohmann::json& j, const game& g);

        //------------------------------------------------------------------------------

        class table : public channel, public game::callback
        {
            using lock_guard = std::lock_guard<std::mutex>;

            shared::server& srv_;
            timer_type      timer_;
            std::mutex mutable mutex_;
            game g_;

          public:
            explicit table(web_worker::application_impl& srv) :
                channel(3, "Blackjack", srv.channel_list()), srv_(srv),
                timer_(boost::asio::make_strand(srv.get_application_ioc().get_executor())), g_(*this, 1)
            {
                boost::ignore_unused(srv_);
            }

          private:
            // Post a call to the strand
            template <class... Args>
            void post(Args&&... args)
            {
                boost::asio::post(timer_.get_executor(), boost::beast::bind_front_handler(std::forward<Args>(args)...));
            }

            //--------------------------------------------------------------------------
            //
            // channel
            //
            //--------------------------------------------------------------------------

            void on_insert(shared::user& u, shared::ws_session_t& sess)
            {
                post(&table::do_insert, this, boost::shared_from(&sess));
            }

            void on_erase(shared::user& u, shared::ws_session_t& sess)
            {
                post(
                    &table::do_erase,
                    this,
                    std::reference_wrapper<shared::user>(u),
                    std::reference_wrapper<shared::ws_session_t>(sess));
            }

            void on_dispatch(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess)
            {
                if (rpc.method == "play")
                {
                    post(&table::do_play, this, std::move(rpc), std::move(u), std::move(sess));
                }
                else if (rpc.method == "watch")
                {
                    post(&table::do_watch, this, std::move(rpc), std::move(u), std::move(sess));
                }
                else if (rpc.method == "bet")
                {
                    post(&table::do_bet, this, std::move(rpc), std::move(u), std::move(sess));
                }
                else if (rpc.method == "start")
                {
                    post(&table::do_start, this, std::move(rpc), std::move(u), std::move(sess));
                }
                else if (rpc.method == "hit")
                {
                    post(&table::do_hit, this, std::move(rpc), std::move(u), std::move(sess));
                }
                else if (rpc.method == "stand")
                {
                    post(&table::do_stand, this, std::move(rpc), std::move(u), std::move(sess));
                }
                else
                {
                    rpc.fail(rpc_code::method_not_found);
                }
            }

            //--------------------------------------------------------------------------
            //
            // table
            //
            //--------------------------------------------------------------------------

            void update(boost::beast::string_view action)
            {
                nlohmann::json jv;

                jv["cid"]    = cid();
                jv["verb"]   = "update";
                jv["action"] = action;
                jv["game"]   = g_;

                send(jv);
            }

            void on_timer(boost::beast::error_code ec)
            {
                if (ec == boost::asio::error::operation_aborted)
                    return;

                if (ec)
                {
                    // log error
                    BOOST_ASSERT(!ec);
                }

                if (srv_.is_shutting_down())
                    return;

                // g_.tick();

                timer_.expires_after(std::chrono::seconds(1));
                timer_.async_wait(boost::beast::bind_front_handler(&table::on_timer, this));
            }

            //--------------------------------------------------------------------------

            void do_insert(boost::shared_ptr<shared::ws_session_t> sp)
            {
                nlohmann::json jv;

                jv["cid"]    = cid();
                jv["verb"]   = "update";
                jv["action"] = "init";
                jv["game"]   = g_;
                sp->send(jv);
            }

            void do_erase(shared::user& u, shared::ws_session_t& sess)
            {
                auto const result = g_.surrender(u, sess);
                if (result == 1)
                    update("surrender");
            }

            void do_play(shared::rpc_call&& rpc, shared::user&& u, shared::ws_session_t& sess)
            {
                std::function<void(nlohmann::json)> f1 = [&sess](nlohmann::json j) { sess.send(j); };

                try
                {
                    // TODO Optional seat choice
                    boost::beast::error_code ec;
                    g_.join(u, ec);
                    if (ec)
                        rpc.fail(ec.message());
                    update("play");
                    rpc.complete(f1);
                }
                catch (shared::rpc_error const& e)
                {
                    rpc.complete(e, f1);
                }
            }

            void do_watch(shared::rpc_call&& rpc, shared::user&& u, shared::ws_session_t& sess)
            {
                std::function<void(nlohmann::json)> f1 = [&sess](nlohmann::json j) { sess.send(j); };

                try
                {
                    boost::beast::error_code ec;
                    g_.leave(u, ec);
                    if (ec)
                        rpc.fail(ec.message());
                    update("watch");
                    rpc.complete(f1);
                }
                catch (shared::rpc_error const& e)
                {
                    rpc.complete(e, f1);
                }
            }

            void do_bet(shared::rpc_call&& rpc, shared::user&& u, shared::ws_session_t& sess)
            {
                std::function<void(nlohmann::json)> f1 = [&sess](nlohmann::json j) { sess.send(j); };

                try
                {
                    boost::beast::error_code ec;
                    g_.bet(u, ec);
                    if (ec)
                        rpc.fail(ec.message());
                    rpc.complete(f1);
                }
                catch (shared::rpc_error const& e)
                {
                    rpc.complete(e, f1);
                }
            }

            void do_start(shared::rpc_call&& rpc, shared::user&& u, shared::ws_session_t& sess)
            {
                std::function<void(nlohmann::json)> f1 = [&sess](nlohmann::json j) { sess.send(j); };

                try
                {
                    boost::beast::error_code ec;
                    g_.start(ec);
                    if (ec)
                        rpc.fail(ec.message());
                    rpc.complete(f1);
                }
                catch (shared::rpc_error const& e)
                {
                    rpc.complete(e, f1);
                }
            }

            void do_hit(shared::rpc_call&& rpc, shared::user&& u, shared::ws_session_t& sess)
            {
                std::function<void(nlohmann::json)> f1 = [&sess](nlohmann::json j) { sess.send(j); };

                try
                {
                    rpc.complete(f1);
                }
                catch (shared::rpc_error const& e)
                {
                    rpc.complete(e, f1);
                }
            }

            void do_stand(shared::rpc_call&& rpc, shared::user&& u, shared::ws_session_t& sess)
            {
                std::function<void(nlohmann::json)> f1 = [&sess](nlohmann::json j) { sess.send(j); };

                try
                {
                    rpc.complete(f1);
                }
                catch (shared::rpc_error const& e)
                {
                    rpc.complete(e, f1);
                }
            }

            //--------------------------------------------------------------------------
            //
            // game::callback
            //
            //--------------------------------------------------------------------------

            void on_game_bet() override
            {
                update("bet");
            }

            void on_game_deal_card() override
            {
                update("deal");
            }
        };

        void to_json(nlohmann::json& j, const seat& s)
        {
            j = nlohmann::json::object();

            auto arr_hands = nlohmann::json::array();
            for (hand h : s.hands)
            {
                nlohmann::json jh = h;

                arr_hands.emplace_back(h);
            }

            j.emplace("hands", arr_hands);

            switch (s.state)
            {
            case seat::state_t::dealer:
                j.emplace("state", "dealer");
                break;

            case seat::state_t::waiting:
                j.emplace("state", "waiting");
                j.emplace("user", s.u->name);
                j.emplace("chips", s.chips);
                break;

            case seat::state_t::playing:
                j.emplace("state", "playing");
                j.emplace("user", s.u->name);
                j.emplace("chips", s.chips);
                j.emplace("wager", s.wager);
                break;

            case seat::state_t::leaving:
                j.emplace("state", "leaving");
                j.emplace("user", s.u->name);
                j.emplace("chips", s.chips);
                j.emplace("wager", s.wager);
                break;

            case seat::state_t::open:
                j.emplace("state", "open");
                break;
            }
        }

        void to_json(nlohmann::json& j, const hand& h)
        {
            j = nlohmann::json::array();
            for (auto c : h.cards)
            {
                j.emplace_back(c);
            }
        }

        void to_json(nlohmann::json& j, const game& g)
        {
            j = nlohmann::json();

            switch (g.get_state())
            {
            case game::state::wait:
                j = {{"message", "Waiting for players"}};
                break;

            case game::state::play:
                j = {{"message", "Playing"}};
                break;
            }

            // nlohmann::basic_json<> obj = nlohmann::json::object();
            {
                auto arr_seats = nlohmann::json::array();
                for (std::size_t i = 0; i < g.get_seat().size(); ++i)
                    arr_seats.emplace_back(g.get_seat()[i]);

                j["seats"] = arr_seats;
            }
        }

        void blackjack_service::on_start(executor_type exe)
        {
           // insert<table>(srv_.channel_list(), srv_);

            srv_.channel_list().insert(boost::make_shared<table>(srv_));

        }

        user::~user()
        {
            // The loop is written this way because elements
            // are erased from the table during the loop.
            while (!channels_.empty())
            {
                auto it = channels_.end();
                --it;
                // TODO: KP. Fix this, clean up
                //  (*it)->erase(*this);
            }
        }

        void user::on_insert(channel& c)
        {
            // TODO: KP. Fix this, lock / synchronize

            // std::lock_guard<std::mutex> lock(mutex_);
            BOOST_VERIFY(channels_.insert(&c).second);
        }

        void user::on_erase(channel& c)
        {
            // TODO: KP. Fix this, lock / synchronize

            //  std::lock_guard<std::mutex> lock(mutex_);
            BOOST_VERIFY(channels_.erase(&c) == 1);
        }

        
        inline void application_impl::run()
        {
            if (running_)
                throw std::logic_error("server already running");

            running_ = true;

            // Start all agents
            for (auto const& sp : services_)
                sp->on_start(boost::asio::make_strand(network_ioc_.get_executor()));

            // Capture SIGINT and SIGTERM to perform a clean shutdown
            signals_.async_wait(
                boost::beast::bind_front_handler(&application_impl::on_signal, this));

#ifndef LOUNGE_USE_SYSTEM_EXECUTOR
            std::vector<std::thread> vt;
            while (vt.size() < cfg_.get_num_threads())
                vt.emplace_back([this] { this->network_ioc_.run(); });
#endif
            // Block the main thread until stop() is called
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] { return stop_.load(); });
            }

            // Notify all agents to stop
            auto agents = std::move(services_);
            for (auto const& sp : agents)
                sp->on_stop();

            // services must be kept alive until after
            // all executor threads are joined.

            // If we get here, then the server has
            // stopped, so join the threads before
            // destroying them.

#ifdef LOUNGE_USE_SYSTEM_EXECUTOR
                boost::asio::system_executor{}.context().join();
#else
            for (auto& t : vt)
                t.join();
#endif
        }
        /*

        ws_session is created
        userinfo has just the endpoint


        RPC commands

        method      params
        ------------------
        create      "user"
        login       "user"

        */

    } // namespace web_worker
} // namespace service
} // namespace easyprospect

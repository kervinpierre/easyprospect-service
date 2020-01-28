#include <easyprospect-web-worker/blackjack.hpp>

namespace easyprospect
{
    namespace service
    {
        namespace web_worker
        {
            void
                to_json(nlohmann::json& j, const seat& s)
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

            void
                to_json(nlohmann::json& j, const hand& h)
            {
                j = nlohmann::json::array();
                for (auto c : h.cards)
                {
                    j.emplace_back(c);
                }
            }

            void
                to_json(nlohmann::json& j, const game& g)
            {
                j = nlohmann::json();

                switch (g.get_state())
                {
                case game::state::wait:
                    j = {
                        { "message", "Waiting for players" }
                    };
                    break;

                case game::state::play:
                    j = {
                        { "message", "Playing" }
                    };
                    break;
                }

                // nlohmann::basic_json<> obj = nlohmann::json::object();
                {
                    auto arr_seats = nlohmann::json::array();
                    for (std::size_t i = 0;
                        i < g.get_seat().size(); ++i)
                        arr_seats.emplace_back(g.get_seat()[i]);

                    j["seats"] = arr_seats;
                }
            }
        }
    }
}

beast::error_code
make_error_code(error e)
{
    static error_codes const cat{};
    return { static_cast<std::underlying_type<
        error>::type>(e), cat };
}

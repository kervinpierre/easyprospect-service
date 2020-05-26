#pragma once
#include <boost/beast/core/error.hpp>

enum class error
{
    not_playing = 1,
    already_playing,
    already_leaving,
    no_open_seat,
    no_more_bets
};

namespace boost
{
namespace system
{
    template <>
    struct is_error_code_enum<error>
    {
        static bool constexpr value = true;
    };
} // namespace system
} // namespace boost

boost::beast::error_code make_error_code(error e);

class error_codes : public boost::beast::error_category
{
  public:
    const char* name() const noexcept override
    {
        return "beast-lounge.blackjack";
    }

    std::string message(int ev) const override
    {
        switch (static_cast<error>(ev))
        {
        default:
        case error::not_playing:
            return "Not playing";
        case error::already_playing:
            return "Already playing";
        case error::already_leaving:
            return "Already playing";
        case error::no_open_seat:
            return "No open seat";
        case error::no_more_bets:
            return "No more bets";
        }
    }

    boost::beast::error_condition default_error_condition(int ev) const noexcept override
    {
        return {ev, *this};
    }
};
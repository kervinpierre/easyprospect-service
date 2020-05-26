
#include <boost/beast/http/error.hpp>
#include <easyprospect-web-worker/app-error.h>

boost::beast::error_code make_error_code(error e)
{
    static boost::beast::detail::error_codes const cat{};
    return {static_cast<std::underlying_type<boost::beast::http::error>::type>(e), cat};
}
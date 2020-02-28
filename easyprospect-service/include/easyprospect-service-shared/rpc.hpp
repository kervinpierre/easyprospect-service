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
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/string.hpp>
#include <stdexcept>
#include <nlohmann/json.hpp>

/// Codes used in JSON-RPC error responses
enum class rpc_code
{
    parse_error = -32700,
    invalid_request = -32600,
    method_not_found = -32601,
    invalid_params = -32602,
    internal_error = -32603,

    /// Expected object in JSON-RPC request
    expected_object = 1,

    /// Expected string version in JSON-RPC request
    expected_string_version,

    /// Uknown version in JSON-RPC request
    unknown_version,

    /// Invalid null id in JSON-RPC request
    invalid_null_id,

    /// Expected string or number id in JSON-RPC request
    expected_strnum_id,

    /// Missing id in JSON-RPC request version 1
    expected_id,

    /// Missing method in JSON-RPC request
    missing_method,

    /// Expected string method in JSON-RPC request
    expected_string_method,

    /// Expected structured params in JSON-RPC request version 2
    expected_structured_params,

    /// Missing params in JSON-RPC request version 1
    missing_params,

    /// Expected array params in JSON-RPC request version 1
    expected_array_params
};

namespace boost
{
    namespace system
    {
        template<>
        struct is_error_code_enum<rpc_code>
        {
            static bool constexpr value = true;
        };
    } // system
} // boost

beast::error_code
make_error_code(rpc_code e);

namespace easyprospect
{
    namespace service
    {
        namespace shared
        {
            //------------------------------------------------------------------------------

            class rpc_error
                : public std::exception
            {
                int code_;
                std::string msg_;

            public:


                rpc_error()
                    : rpc_error(
                        rpc_code::internal_error)
                {
                }

                rpc_error(
                    rpc_code ev)
                    : rpc_error(ev,
                        beast::error_code(ev).message())
                {
                }

                rpc_error(
                    beast::string_view msg)
                    : rpc_error(
                        rpc_code::invalid_params,
                        msg)
                {
                }

                rpc_error(
                    rpc_code ev,
                    beast::string_view msg)
                    : code_(static_cast<int>(ev))
                    , msg_(msg)
                {
                }

                rpc_error(
                    beast::error_code const& ec)
                    : code_(static_cast<int>(ec.value()))
                    , msg_(ec.message())
                {
                }

                nlohmann::json
                    to_json(
                        int id) const;
            };

            //------------------------------------------------------------------------------

            /** Represents a JSON-RPC request
            */
            class rpc_call
            {
                /** The request id

                    If set, this will be string, number, or null
                */
                boost::optional<int> id_;

            public:
                /// The user submitting the request
                //boost::shared_ptr<user_base> u;

                /// Version of the request (1 or 2)
                int version = 2;

                /// The request method
                std::string method;

                /** The request parameters

                    This will be object, array, or null
                */
                nlohmann::json params;

                /** The response result

                    This value will be sent as the result of
                    a successful operation.
                */
                nlohmann::json result;

            public:
                rpc_call(rpc_call&&) = default;
                rpc_call& operator=(rpc_call&&) = delete;

                /** Construct an empty request using the specified storage.

                    The method, params, and id will be null,
                    and version will be 2.
                */
                rpc_call() = default;

                /** Extract a JSON-RPC request or return an error.
                */
                void
                    extract(
                        nlohmann::basic_json<>&& jv,
                        beast::error_code& ec);

                /** Complete the RPC request with a success.

                    This function sends the user originating the request
                    a JSON-RPC response object containing the result.
                */
                void complete(std::function<void(nlohmann::json)> f);

                /** Complete the RPC request with an error.

                    This function sends the user originating the request
                    a JSON-RPC response containing an error object.
                */
                void complete(rpc_error const& e, std::function<void(nlohmann::json)> f);

                /** Respond to a request with an error.

                    This function will throw an `rpc_error`
                    constructed from the argument list.
                */
                template<class... Args>
                [[noreturn]]
                void
                    fail(Args&&... args)
                {
                    throw rpc_error(
                        std::forward<Args>(args)...);
                }
            };

            extern
                nlohmann::json&
                checked_object(nlohmann::json& jv);

            extern
                nlohmann::json&
                checked_value(
                    nlohmann::json& jv,
                    beast::string_view key);

            extern
                std::string
                checked_string(nlohmann::json& jv);

            extern
                std::string
                checked_string(
                    nlohmann::json& jv,
                    beast::string_view key);
            //------------------------------------------------------------------------------

        }
    }
}

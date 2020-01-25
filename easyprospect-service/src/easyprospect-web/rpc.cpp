//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

#include "easyprospect-web/rpc.hpp"
#include "easyprospect-web/user.hpp"
#include <boost/beast/core/error.hpp>
#include <type_traits>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

//------------------------------------------------------------------------------

namespace {

class rpc_error_codes : public beast::error_category
{
public:
    const char*
    name() const noexcept override
    {
        return "beast-lounge";
    }

    std::string
    message(int ev) const override
    {
        switch(static_cast<rpc_code>(ev))
        {
        case rpc_code::parse_error: return
            "An error occurred on the server while parsing the JSON text.";
        case rpc_code::invalid_request: return
            "The JSON sent is not a valid Request object";
        case rpc_code::method_not_found: return
            "The method does not exist or is not available";
        case rpc_code::invalid_params: return
            "Invalid method parameters";
        case rpc_code::internal_error: return
            "Internal JSON-RPC error";

        case rpc_code::expected_object: return
            "Expected object in JSON-RPC request";
        case rpc_code::expected_string_version: return
            "Expected string version in JSON-RPC request";
        case rpc_code::unknown_version: return
            "Uknown version in JSON-RPC request";
        case rpc_code::invalid_null_id: return
            "Invalid null id in JSON-RPC request";
        case rpc_code::expected_strnum_id: return
            "Expected string or number id in JSON-RPC request";
        case rpc_code::expected_id: return
            "Missing id in JSON-RPC request version 1";
        case rpc_code::missing_method: return
            "Missing method in JSON-RPC request";
        case rpc_code::expected_string_method: return
            "Expected string method in JSON-RPC request";
        case rpc_code::expected_structured_params: return
            "Expected structured params in JSON-RPC request version 2";
        case rpc_code::missing_params: return
            "Missing params in JSON-RPC request version 1";
        case rpc_code::expected_array_params: return
            "Expected array params in JSON-RPC request version 1";
        }
        if( ev >= -32099 && ev <= -32000)
            return "An implementation defined server error was received";
        return "Unknown RPC error #" + std::to_string(ev);
    }

    beast::error_condition
    default_error_condition(int ev) const noexcept override
    {
        return {ev, *this};
    }
};

} // (anon)

beast::error_code
make_error_code(rpc_code e)
{
    static rpc_error_codes const cat{};
    return {static_cast<std::underlying_type<
        rpc_code>::type>(e), cat};
}

//------------------------------------------------------------------------------

nlohmann::json
rpc_error::
to_json(
    int id) const
{
    nlohmann::json jv;

    jv["jsonrpc"] = "2.0";
    jv["jsonrpc"]["error"]["code"] = code_;
    jv["jsonrpc"]["error"]["message"] = msg_;
    if(id >= 0)
        jv["id"] = id;

    return jv;
}

//------------------------------------------------------------------------------

rpc_call::
rpc_call(
    ::user& u_)
    : u(boost::shared_from(&u_))
{
}

void
rpc_call::
extract(
    nlohmann::basic_json<>&& jv,
    beast::error_code& ec)
{
    // must be object
    if(! jv.is_object())
    {
        ec = rpc_code::expected_object;
        return ;
    }

    // extract id first so on error,
    // the response can include it.
    {
        auto it = jv["id"].get<int>();
        id_ = it;
    }

    // now check the version
    {
        version = 1;
        auto it_str = jv["jsonrpc"].get<std::string>();
        float it = std::stof(it_str);
        if( it == 2 || it == 1)
        {
            version = it;
        }
        else
        {
            ec = rpc_code::unknown_version;
            return;
        }
    }

    // validate the extracted id
    {
        if(version == 2)
        {
            if(id_.has_value())
            {
                // The use of Null as a value for the
                // id member in a Request is discouraged.
                if(id_ < 0)
                {
                    ec = rpc_code::invalid_null_id;
                    return;
                }
            }
        }
        else
        {
            // id must be present in 1.0
            if(! id_.has_value())
            {
                ec = rpc_code::expected_id;
                return;
            }
        }
    }

    // extract method
    {
        std::string it;

        try
        {
            it = jv["method"].get<std::string>();
        }
        catch (nlohmann::json::out_of_range & ex)
        {
            spdlog::error("extract() : out of range exception.\n{}", ex.what());
            ec = rpc_code::missing_method;
            return;
        }

        method = it;
    }

    // extract params
    {
        auto it = jv.find("params");
        if (version == 2)
        {
            if (it != jv.end())
            {
                if (!it->is_object() &&
                    !it->is_array())
                {
                    ec = rpc_code::expected_structured_params;
                    return;
                }
                params = *it;
            }
        }
        else
        {
            if (it == jv.end())
            {
                ec = rpc_code::missing_params;
                return;
            }
            if (!it->is_array())
            {
                ec = rpc_code::expected_array_params;
                return;
            }
            params = *it;
        }
    }
}

void
rpc_call::
complete()
{
    if(! id_.has_value())
        return;

    nlohmann::json res;

    res["id"] = *id_;
    u->send(res);
}

void
rpc_call::
complete(rpc_error const& e)
{
    if(! id_.has_value())
        return;
    u->send(e.to_json(*id_));
}

nlohmann::json&
checked_object(nlohmann::json& jv)
{
    if (!jv.is_object())
        throw rpc_error{
            "expected object" };
    return jv;
}

nlohmann::json&
checked_value(
    nlohmann::json& jv,
    beast::string_view key)
{
    auto& obj =
        checked_object(jv);
    auto it = obj.find(key.to_string());
    if (it == obj.end())
        throw rpc_error{
            "key '" + key.to_string() + "' not found" };
    return *it;
}

std::string
checked_string(nlohmann::json& jv)
{
    if (!jv.is_string())
        throw rpc_error{
            "expected string" };
    std::string str = jv.get<std::string>();
    return str;
}

std::string
checked_string(
    nlohmann::json& jv,
    beast::string_view key)
{
    return checked_string(
        checked_value(jv, key));
}
//------------------------------------------------------------------------------

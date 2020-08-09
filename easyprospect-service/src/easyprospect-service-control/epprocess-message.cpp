#include <easyprospect-service-control/epprocess-message.h>

using namespace easyprospect::service::control;

std::unique_ptr<process_message_base> 
    process_message_base::process_input(std::vector<unsigned char>& in, size_t bytes)
{
    if (bytes < 1)
    {
        throw std::logic_error("parameter byte count < 1");
    }

    // Successful completion
    std::string o(in.begin(), in.end());
    if (bytes < in.size())
    {
        o.resize(bytes);
    }

    spdlog::debug("process_message_base::process_input() : '{}'", o);

    msgpack::object_handle oh = msgpack::unpack(o.data(), o.size());

    msgpack::object deserialized = oh.get();

    std::stringstream os;

   // os << deserialized << std::endl;

   // spdlog::debug("msgpack: '{}'", os.str());

    for (auto m : deserialized.via.map)
    {
        auto k = m.key.as<std::string>();
        spdlog::debug("key: '{}'", k);

        process_message_type curr_type = process_message_type::NONE;

        if (k == "type")
        {
            curr_type = m.val.as<process_message_type>();
        }
        else if (k == "process_message_base")
        {
            auto b    = m.val.as<process_message_base>();
            curr_type = b.type;
        }
        else
        {
            continue;
        }

        switch(curr_type)
        {
        case process_message_type::NONE: break;
        case process_message_type::BASE: break;
        case process_message_type::START:
            {
                auto st = std::make_unique<process_message_startup>();
                deserialized.convert<process_message_startup>(*st);

                return st;
            }
            break;

        case process_message_type::PING: break;
        case process_message_type::PONG: break;
        case process_message_type::CMD_STOP: break;
        case process_message_type::CMD_RESULT:
            {
                auto rs = std::make_unique<process_message_cmd_result>();
                deserialized.convert<process_message_cmd_result>(*rs);

                return rs;
            }
            break;

        default: ;
        }
    }

    return nullptr;
}

std::string process_message_base::to_string(const process_message_base& m)
{
    std::ostringstream os;

    {
        std::chrono::milliseconds ct(m.current_time);
        std::chrono::time_point<std::chrono::system_clock> dt(ct);
        std::time_t tt = std::chrono::system_clock::to_time_t(dt);
        std::tm ttm = *std::gmtime(&tt);
        os << "time: " << std::put_time(&ttm, "UTC %Y-%m-%d %H:%M:%S") << '.' <<
            std::setfill('0')
            << std::setw(3) << m.current_time % 1000 << std::endl;
    }

    os << "pid: " << m.pid << std::endl;
    os << "port: " << m.port << std::endl;
    os << "id1: " << m.id1 << std::endl;
    os << "id2: " << m.id2 << std::endl;

    switch (m.type)
    {
    case process_message_type::NONE:
        os << "type: NONE" << std::endl;
        break;

    case process_message_type::BASE:
        os << "type: BASE" << std::endl;
        break;

    case process_message_type::START:
        os << "type: START" << std::endl;
        break;

    case process_message_type::PING: break;
    case process_message_type::PONG: break;
    case process_message_type::CMD_STOP: break;
    case process_message_type::CMD_RESULT:
        os << "type: CMD_RESULT" << std::endl;
        os << to_string(static_cast<const process_message_cmd_result&>(m));
        break;

    default: ;
    }

    std::string res = os.str();

    return res;
}

std::string process_message_base::to_string(const process_message_cmd_result& m)
{
    std::ostringstream os;
    os << "result: " << m.result << std::endl;

    std::string res = os.str();

    return res;
}
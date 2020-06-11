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

    spdlog::debug("client read ended : '{}'", o);

    msgpack::object_handle oh = msgpack::unpack(o.data(), o.size());

    msgpack::object deserialized = oh.get();

    std::stringstream os;

   // os << deserialized << std::endl;

   // spdlog::debug("msgpack: '{}'", os.str());

    for (auto m : deserialized.via.map)
    {
        auto k = m.key.as<std::string>();
        spdlog::debug("key: '{}'", k);

        if (k != "type")
        {
            continue;
        }

        auto v = m.val.as<process_message_type>();
        switch(v)
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
        case process_message_type::CMD_RESULT: break;
        default: ;
        }
    }

    return nullptr;
}

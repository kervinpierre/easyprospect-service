#include <epworkercntl/epprocess-message.h>

using namespace easyprospect::service::control_worker;

// TODO: KP. There must be a better way to write this function
std::unique_ptr<msgpack::sbuffer> process_message_base::pack(
    process_message_base& o)
{
    auto sbuf = std::make_unique<msgpack::sbuffer>();

    switch (o.type)
    {
    case process_message_type::NONE:
    case process_message_type::BASE:
        msgpack::pack<msgpack::sbuffer, process_message_base>(*sbuf, o);
        break;

    case process_message_type::CMD_RESULT:
        process_message_cmd_result& p = static_cast<process_message_cmd_result&>(o); // UB at runtime
       msgpack::pack<msgpack::sbuffer, process_message_cmd_result>(*sbuf, p);
        break;
    }

    return sbuf;
}

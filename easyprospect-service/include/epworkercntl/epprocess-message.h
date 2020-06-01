#pragma once

#include <msgpack.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace easyprospect
{
namespace service
{
    namespace control_worker
    {
        enum class process_message_type
        {
            NONE = 0,
            BASE = 1,
            START = 2,
            PING = 3,
            PONG = 4,
            CMD_STOP = 5,
            CMD_RESULT= 6
        };

        struct process_message_base
        {
            process_message_type type; 
            int                  pid;
            int                  port;
            uint64_t             id1;
            uint64_t             id2;
            long long            current_time;
            MSGPACK_DEFINE(type, pid, port, id1, id2, current_time);

            process_message_base()
            {
                type = process_message_type::BASE;

                // https://stackoverflow.com/questions/31255486/c-how-do-i-convert-a-stdchronotime-point-to-long-and-back
                auto now    = std::chrono::system_clock::now();
                auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
                auto epoch  = now_ms.time_since_epoch();
                auto value  = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
                current_time  = value.count();
            }

        static std::unique_ptr<msgpack::sbuffer> pack(process_message_base& o);

        };

        struct process_message_startup final : process_message_base
        {
            process_message_startup()
            {
                type = process_message_type::START;
            }
        };

        struct process_message_ping final : process_message_base
        {
            process_message_ping()
            {
                type = process_message_type::PING;
            }

            void pong()
            {
                type = process_message_type::PONG;
            }
        };

        struct process_message_stop final : process_message_base
        {
            process_message_stop()
            {
                type = process_message_type::CMD_STOP;
            }
        };

        struct process_message_cmd_result final : public process_message_base
        {
            process_message_type type_cmd;
            int                  result;
            MSGPACK_DEFINE(MSGPACK_BASE(process_message_base), type_cmd, result);

            process_message_cmd_result()
            {
                type = process_message_type::CMD_RESULT;
            }
        };

        inline std::unique_ptr<msgpack::sbuffer> process_message_base::pack(process_message_base& o)
        // TODO: KP. There must be a better way to write this function
        {
            auto sbuf = std::make_unique<msgpack::sbuffer>();

            switch (o.type)
            {
            case process_message_type::NONE:
            case process_message_type::BASE:
                msgpack::pack<msgpack::sbuffer, process_message_base>(*sbuf, o);
                break;

            case process_message_type::CMD_RESULT:
                auto& p = static_cast<process_message_cmd_result&>(o); // UB at runtime
                msgpack::pack<msgpack::sbuffer, process_message_cmd_result>(*sbuf, p);
                break;
            }

            return sbuf;
        }
    } // namespace control_worker
} // namespace service
} // namespace easyprospect

MSGPACK_ADD_ENUM(easyprospect::service::control_worker::process_message_type);
#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <msgpack.hpp>
#include <sstream>
#include <spdlog/spdlog.h>

#include "easyprospect-os-utils/easyprospect-os-utils.h"

#define MSGPACK_USE_DEFINE_MAP

namespace easyprospect
{
namespace service
{
    namespace control
    {
        struct process_message_cmd_result;

        enum class process_message_type
        {
            NONE       = 0,
            BASE       = 1,
            START      = 2,
            PING       = 3,
            PONG       = 4,
            CMD_STOP   = 5,
            CMD_RESULT = 6
        };

        struct process_message_base
        {
            process_message_type type;
            long                 pid;
            int                  port;
            uint64_t             id1;
            uint64_t             id2;
            long long            current_time;
            MSGPACK_DEFINE_MAP(type, pid, port, id1, id2, current_time);

            process_message_base()
            {
                config(process_message_type::BASE, -1, 0, 0, 0, -1);
            }

            process_message_base(long pi, int po, uint64_t i1, uint64_t i2, long long ct)
            {
                config(process_message_type::BASE, pi, po, i1, i2, ct);
            }

            static std::unique_ptr<process_message_base> process_input(
                std::vector<unsigned char>& in, size_t bytes);

            static std::unique_ptr<msgpack::sbuffer> pack(const process_message_base& o);

            static std::string to_string(const process_message_cmd_result& m);
            static std::string to_string(const process_message_base& m);

        private:
            void config(process_message_type ty, long pi, int po, uint64_t i1, uint64_t i2, long long ct)
            {
                type = ty;
                port = po;
                id1  = i1;
                id2  = i2;

                if ( pid < 0 )
                {
                    pid = os_utils::ep_process_utils::getpid();
                }

                if (ct < 0)
                {
                    // https://stackoverflow.com/questions/31255486/c-how-do-i-convert-a-stdchronotime-point-to-long-and-back
                    auto now     = std::chrono::system_clock::now();
                    auto now_ms  = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
                    auto epoch   = now_ms.time_since_epoch();
                    auto value   = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
                    current_time = value.count();
                }
                else
                {
                    current_time = ct;
                }
            }
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
            MSGPACK_DEFINE_MAP(MSGPACK_BASE_MAP(process_message_base), type_cmd, result);

            process_message_cmd_result()
            {
                type = process_message_type::CMD_RESULT;
            }
        };

        inline std::unique_ptr<msgpack::sbuffer> process_message_base::pack(const process_message_base& o)
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
            {
                auto& p = static_cast<const process_message_cmd_result&>(o); // UB at runtime
                msgpack::pack<msgpack::sbuffer, process_message_cmd_result>(*sbuf, p);
            }
            break;

            case process_message_type::START:
            {
                auto& p = static_cast<const process_message_startup&>(o); // UB at runtime
                msgpack::pack<msgpack::sbuffer, process_message_startup>(*sbuf, p);
            }
            break;

            default:
                spdlog::error("Pack message type unknown: {}", o.type);
                throw std::logic_error("Message type unknown");
                break;
            }

            return sbuf;
        }
    } // namespace control_worker
} // namespace service
} // namespace easyprospect

MSGPACK_ADD_ENUM(easyprospect::service::control::process_message_type);

//using namespace easyprospect::service::control;
//
//namespace msgpack
//{
//MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
//{
//    namespace adaptor
//    {
//        template <>
//        struct as<process_message_base>
//        {
//           // template <typename U = T> 
//         //   typename std::enable_if<std::is_base_of<process_message_base, U>::value, U>::type
//           process_message_base operator()(msgpack::object const& o) const
//            {
//                if (o.type != msgpack::type::MAP)
//                    throw msgpack::type_error();
//
//                //if (o.via.array.size != 1)
//                //    throw msgpack::type_error();
//
//                auto m = o.via.map;
//
//                auto pi = 0, po = 0, i1 = 0, i2 = 0, ct = -1;
//                auto v = process_message_type::NONE;
//
//                for (auto i = 0; i < m.size; i++)
//                {
//                    if (m.ptr[i].key == "type")
//                    {
//                        v = m.ptr[i].val.as<process_message_type>();
//                    }
//                    else if (m.ptr[i].key == "pid")
//                    {
//                        pi = m.ptr[i].val.as<int>();
//                    }
//                    else if (m.ptr[i].key == "port")
//                    {
//                        po = m.ptr[i].val.as<int>();
//                    }
//                    else if (m.ptr[i].key == "id1")
//                    {
//                        i1 = m.ptr[i].val.as<int>();
//                    }
//                    else if (m.ptr[i].key == "id2")
//                    {
//                        i2 = m.ptr[i].val.as<int>();
//                    }
//                    else if (m.ptr[i].key == "current_time")
//                    {
//                        ct = m.ptr[i].val.as<int>();
//                    }
//                }
//
//                if (v == process_message_type::BASE)
//                {
//                    //return T(pi,po,i1,i2,ct);
//                   // return nullptr;
//                }
//
//                throw msgpack::type_error();
//            }
//        };
//    } // namespace adaptor
//} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
//} // namespace msgpack
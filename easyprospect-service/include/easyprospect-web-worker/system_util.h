#pragma once
#include <easyprospect-web-worker/server.hpp>
namespace easyprospect
{
    namespace service
    {
        namespace web_worker
        {
            class system_util
            {
            public:
                static void make_system_channel(server& srv);
            };
        }
    }
}
#pragma once
#include <easyprospect-service-shared/server.h>

namespace easyprospect
{
    namespace service
    {
        namespace web_worker
        {
            extern std::unique_ptr<shared::server> make_server(config::easyprospect_config_service_core curr_config);
        }
    }
}

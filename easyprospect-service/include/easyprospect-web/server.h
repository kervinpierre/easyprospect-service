#pragma once

#include <easyprospect-config/easyprospect-config-service.h>
#include <easyprospect-service-shared/server.h>

namespace easyprospect
{
    namespace service
    {
        namespace web_server
        {
            std::unique_ptr<shared::server>
                make_server(
                    easyprospect::service::config::easyprospect_config_service_core curr_config);
        }
    }
}

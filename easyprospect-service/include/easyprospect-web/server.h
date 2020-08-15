#pragma once

#include <easyprospect-config/easyprospect-config-service.h>
#include <easyprospect-service-shared/server.h>

#include "easyprospect-config/easyprospect-registry.h"

namespace easyprospect
{
    namespace service
    {
        namespace web_server
        {
            std::unique_ptr<shared::server>
                make_server(
                    config::easyprospect_config_service_core curr_config,
                    std::shared_ptr<config::easyprospect_registry> reg);
        }
    }
}

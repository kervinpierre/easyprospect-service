#pragma once

#include <easyprospect-config/easyprospect-config-service.h>
#include <easyprospect-service-shared/server.h>

#include <easyprospect-config/easyprospect-registry.h>
#include <easyprospect-config/easyprospect-config-server.h>

namespace easyprospect
{
    namespace service
    {
        namespace web_server
        {
            std::unique_ptr<shared::server>
                make_server(
                    config::easyprospect_config_server_core curr_config,
                    std::shared_ptr<config::easyprospect_registry> reg);
        }
    }
}

#pragma once

#include <easyprospect-config/easyprospect-config-wcntl.h>

#include "easyprospect-config/easyprospect-registry.h"

namespace easyprospect
{
namespace service
{
    namespace control_worker
    {
        void make_server(config::easyprospect_config_wcntl_core curr_config,
            std::shared_ptr<config::easyprospect_registry> curr_reg);
    } // namespace control_worker
} // namespace service
} // namespace easyprospect
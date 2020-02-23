#pragma once
#include <boost/asio/basic_signal_set.hpp>
#include <spdlog/spdlog.h>
#include "easyprospect-service-shared/server.h"

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

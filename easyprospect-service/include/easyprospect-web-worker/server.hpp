//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

#pragma once

#include "config.hpp"
#include "types.hpp"
#include <boost/beast/core/string.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <utility>
#include "easyprospect-config/easyprospect-config-service.h"
#include "service.hpp"

//------------------------------------------------------------------------------

namespace easyprospect
{
    namespace service
    {
        namespace web_worker
        {
            class channel_list;
            class rpc_handler;
            class service;
            class user;

            /** An instance of the lounge server.
            */
            class server
            {
            public:
                virtual ~server() = default;

                /** Return a new executor to use.
                */
                virtual
                    executor_type
                    make_executor() = 0;

                /** Add a service to the server.

                    Services may only be added before calling start().
                */
                virtual
                    void
                    insert(
                        std::unique_ptr<easyprospect::service::web_worker::service> sp) = 0;

                //--------------------------------------------------------------------------
                //
                // Services
                //
                //--------------------------------------------------------------------------

                virtual const boost::optional<boost::filesystem::path> doc_root() const = 0;

                virtual channel_list& channel_list() = 0;

                //--------------------------------------------------------------------------

                /** Run the server.

                    This call will block until the server is fully stopped.
                    After the server has stopped, the only valid operation
                    on the server is destruction.
                */
                virtual
                    void
                    run() = 0;

                /// Returns `true` if the server is shutting down gracefully
                virtual
                    bool
                    is_shutting_down() = 0;

                /** Shut down the server gracefully
                */
                virtual
                    void
                    shutdown(std::chrono::seconds cooldown) = 0;

                virtual
                    void
                    stop() = 0;
            };

            std::unique_ptr<server>
                make_server(
                    easyprospect::service::config::easyprospect_config_service_core curr_config);
        }
    }
}
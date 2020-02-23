#pragma once

#include <easyprospect-config/easyprospect-config-service.h>
#include <easyprospect-service-shared/types.hpp>
#include <easyprospect-service-shared/rpc.hpp>
#include "session.hpp"
#include "easyprospect-service-shared/user_base.hpp"
#include <spdlog/spdlog.h>

namespace easyprospect
{
    namespace service
    {
        namespace shared
        {
            class service;
            class rpc_call;
            class user;
            class ws_session_t;

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
                virtual void insert(std::unique_ptr<shared::service> sp) = 0;

                //--------------------------------------------------------------------------
                //
                // Services
                //
                //--------------------------------------------------------------------------

                virtual const boost::optional<boost::filesystem::path> doc_root() const = 0;


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


            class application_impl_base : public server
            {
            public:
                net::io_context ioc_;

                executor_type  make_executor() override
                {
#ifdef LOUNGE_USE_SYSTEM_EXECUTOR
                    return net::make_strand(
                        net::system_executor{});
#else
                    return net::make_strand(
                        ioc_.get_executor());
#endif
                }

             //   virtual void dispatch(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess) = 0;
            };


            std::unique_ptr<shared::server> make_server(config::easyprospect_config_service_core curr_config);

        }
    }
}

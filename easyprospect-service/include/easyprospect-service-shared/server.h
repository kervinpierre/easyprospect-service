#pragma once

#include <easyprospect-config/easyprospect-config-service.h>

//#include <easyprospect-service-shared/user_base.hpp>
//#include <easyprospect-service-shared/session.hpp>
#include <easyprospect-service-shared/rpc.hpp>
#include <easyprospect-service-shared/types.hpp>

#include <easyprospect-service-shared/easyprospect-http-request.h>
#include <easyprospect-service-shared/service.hpp>

namespace easyprospect
{
namespace service
{
    namespace shared
    {
        class rpc_call;
        class user;
        class ws_session_t;

        using dispatch_impl_type = std::function<void(rpc_call&, shared::user&, ws_session_t&)>;

        /** An instance of the lounge server.
         */
        class server
        {

          public:
            virtual ~server() = default;

            /** Return a new executor to use.
             */
           // virtual executor_type make_application_executor() = 0;

            /** Add a service to the server.

                Services may only be added before calling start().
            */
            virtual void insert(std::unique_ptr<shared::service> sp) = 0;

            //--------------------------------------------------------------------------
            //
            // Services
            //
            //--------------------------------------------------------------------------

            virtual const boost::optional<boost::filesystem::path> get_doc_root() const = 0;

            virtual std::vector<std::regex> get_epjs_url_path_regex() const = 0;

            //--------------------------------------------------------------------------

            /** Run the server.

                This call will block until the server is fully stopped.
                After the server has stopped, the only valid operation
                on the server is destruction.
            */
            virtual void run() = 0;

            /// Returns `true` if the server is shutting down gracefully
            virtual bool is_shutting_down() = 0;

            /** Shut down the server gracefully
             */
            virtual void shutdown(std::chrono::seconds cooldown) = 0;

            virtual void stop() = 0;
        };

        class application_impl_base : public server
        {
            dispatch_impl_type          dispatch_impl_;
            epjs_process_req_impl_type  epjs_process_req_impl_;
            send_worker_req_impl_type  send_worker_req_impl_;

          public:
            boost::asio::io_context application_ioc_;

            //executor_type make_application_executor() override
            //{
            //    return boost::asio::make_strand(application_ioc_.get_executor());
            //}

            //   virtual void dispatch(shared::rpc_call& rpc, shared::user& u, shared::ws_session_t& sess) = 0;
            dispatch_impl_type get_dispatch_impl() const
            {
                return dispatch_impl_;
            }

            void set_dispatch_impl(dispatch_impl_type val)
            {
                dispatch_impl_ = val;
            }

            epjs_process_req_impl_type get_epjs_process_req_impl() const
            {
                return epjs_process_req_impl_;
            }
            void set_epjs_process_req_impl(epjs_process_req_impl_type val)
            {
                epjs_process_req_impl_ = val;
            }

            send_worker_req_impl_type get_send_worker_req_impl() const
            {
                return send_worker_req_impl_;
            }
            void set_send_worker_req_impl(send_worker_req_impl_type val)
            {
                send_worker_req_impl_ = val;
            }
        };

        std::unique_ptr<shared::server> make_server(config::easyprospect_config_service_core curr_config);

    } // namespace shared
} // namespace service
} // namespace easyprospect

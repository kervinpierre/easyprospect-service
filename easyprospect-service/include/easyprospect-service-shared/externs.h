#pragma once
#include "easyprospect-web/server.h"
#include "listener.h"

namespace easyprospect
{
    namespace service
    {
        namespace shared
        {
            void
                run_http_session(
                    easyprospect::service::shared::application_impl_base& srv,
                    easyprospect::service::shared::listener& lst,
                    stream_type stream,
                    endpoint_type ep,
                    flat_storage storage);

            void
                run_https_session(
                    easyprospect::service::shared::application_impl_base& srv,
                    easyprospect::service::shared::listener& lst,
                    asio::ssl::context& ctx,
                    stream_type stream,
                    endpoint_type ep,
                    flat_storage storage);

            bool run_listener(application_impl_base& srv,
                config::easyprospect_config_service_listener_conf cfg);

            //extern void run_ws_session(
            //    application_impl_base& srv,
            //    shared::listener& lst,
            //    stream_type stream,
            //    endpoint_type ep,
            //    websocket::request_type req);

            //extern void run_ws_session(
            //    application_impl_base& srv,
            //    shared::listener& lst,
            //    beast::ssl_stream<
            //    stream_type> stream,
            //    endpoint_type ep,
            //    websocket::request_type req);
        }
    }
    }

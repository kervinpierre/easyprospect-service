#pragma once

#include <boost/asio/ssl/context.hpp>
#include <easyprospect-config/easyprospect-config-service.h>
#include <easyprospect-config/easyprospect-config.h>

#include "server.h"
#include "listener.h"

namespace easyprospect
{
namespace service
{
    namespace shared
    {
        void run_http_session(
            application_impl_base& srv,
            listener&              lst,
            stream_type                                           stream,
            boost::asio::ip::tcp::endpoint                                         ep,
            boost::beast::flat_buffer                                          storage);

        void run_https_session(
            application_impl_base& srv,
            listener&              lst,
            boost::asio::ssl::context&                                   ctx,
            stream_type                                           stream,
            boost::asio::ip::tcp::endpoint                                         ep,
            boost::beast::flat_buffer                                          storage);

        bool run_listener(application_impl_base& srv, 
                           config::easyprospect_config_service_listener_conf cfg,
            std::shared_ptr<config::easyprospect_registry>    reg,
            boost::asio::io_context&                           ioc);


        // extern void run_ws_session(
        //    application_impl_base& srv,
        //    shared::listener& lst,
        //    stream_type stream,
        //    boost::asio::ip::tcp::endpoint ep,
        //    boost::beast::websocket::request_type req);

        // extern void run_ws_session(
        //    application_impl_base& srv,
        //    shared::listener& lst,
        //    boost::beast::ssl_stream<
        //    stream_type> stream,
        //    boost::asio::ip::tcp::endpoint ep,
        //    boost::beast::websocket::request_type req);
    } // namespace shared
} // namespace service
} // namespace easyprospect

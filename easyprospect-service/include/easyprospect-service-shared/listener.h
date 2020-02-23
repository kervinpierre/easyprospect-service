#pragma once


//#include <easyprospect-service-shared/message.hpp>
#include <easyprospect-config/easyprospect-config-service.h>
#include <boost/asio/coroutine.hpp>
//#include <easyprospect-service-shared/session.hpp>
#include <easyprospect-service-shared/session.hpp>
#include "service.hpp"
#include <boost/asio/ip/address.hpp>
#include "types.hpp"
#include <boost/asio/ssl/context.hpp>

//#include <easyprospect-service-shared/types.hpp>
//#include <easyprospect-service-shared/server.h>
//#include <easyprospect-service-shared/service.hpp>
//#include <easyprospect-service-shared/session.hpp>
//#include <nlohmann/json.hpp>
//#include "server_certificate.hpp"

namespace easyprospect
{
    namespace service
    {
        namespace shared
        {
            class application_impl_base;
            class session;

            /** Configuration for a listening socket.
*/
            struct listener_config
            {
                explicit
                    listener_config(config::easyprospect_config_service_listener_conf& conf);

                // name of this port for logs
                std::string name;

                // endpoint to bind to
                net::ip::address address;

                // port number
                unsigned short port_num;

                enum
                {
                    no_tls,
                    allow_tls,
                    require_tls
                } kind = no_tls;
            };

            /// A listening socket
            class listener
            {
            public:
                virtual ~listener() = default;

                /// Add a session to the listener
                virtual
                    void
                    insert(session* p) = 0;

                /// Remove a session from the listener
                virtual
                    void
                    erase(session* p) = 0;
            };


            //------------------------------------------------------------------------------

            // Accepts incoming connections and launches the sessions
            class listener_impl
                : public boost::asio::coroutine
                , public shared::service
                , public shared::listener
            {
                // This hack works around a bug in basic_socket_acceptor
                // which uses the wrong socket type here:
                // https://github.com/boostorg/asio/blob/c7bbd30491c377ebe12f6e33a0992a3280d71fa4/include/boost/asio/detail/reactive_socket_accept_op.hpp#L198

                struct tcp_ex : net::ip::tcp
                {
                    tcp_ex(tcp const& t)
                        : tcp(t)
                    {
                    }

                    using socket =
                        net::basic_stream_socket<
                        tcp, executor_type>;
                };

                shared::application_impl_base& srv_;
                std::mutex mutex_;
                easyprospect::service::config::easyprospect_config_service_listener_conf cfg_;
                asio::ssl::context ctx_;
                net::basic_socket_acceptor<
                    tcp_ex, executor_type> acceptor_;
                boost::container::flat_set<
                    session*> sessions_;
                endpoint_type ep_;

                int current_port_;

            public:
                listener_impl(
                    application_impl_base& srv,
                    easyprospect::service::config::easyprospect_config_service_listener_conf cfg);

                ~listener_impl();

                bool open_port(asio::ip::address addr, int port, beast::error_code ec);

                bool open();

                void do_stop();

                void
                    listener_impl::operator()(
                        beast::error_code ec,
                        socket_type sock);

                // Report a failure
                void
                fail(beast::error_code ec, char const* what);

                //--------------------------------------------------------------------------
                //
                // listener
                //
                //--------------------------------------------------------------------------

                void
                insert(session* p) override;

                void
                erase(session* p) override;

                //--------------------------------------------------------------------------
                //
                // service
                //
                //--------------------------------------------------------------------------

                /// Called when the server starts
                void
                on_start() override;

                /// Called when the server stops
                void
                on_stop() override;
            };

        }
    }
}

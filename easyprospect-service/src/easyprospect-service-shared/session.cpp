
#include <easyprospect-service-shared/session.hpp>
#include <easyprospect-service-shared/message.hpp>
#include <spdlog/spdlog.h>
using namespace easyprospect::service::shared;

namespace easyprospect
{
    namespace service
    {
        namespace shared
        {
            ws_session_t::ws_session_t( application_impl_base& srv,
                                        listener& lst,
                                        stream_type stream,
                                        endpoint_type ep,
                                        flat_storage storage )
            {
                plain_ptr_ = std::make_unique<plain_http_session_impl>(
                        srv, lst,
                        std::move( stream ),
                        ep,
                        std::move( storage ) );

                ssl_ptr_ = nullptr;
            }

            ws_session_t::ws_session_t(
                application_impl_base& srv,
                listener& lst,
                asio::ssl::context& ctx,
                stream_type stream,
                endpoint_type ep,
                flat_storage storage )
            {
                ssl_ptr_ = std::make_unique<ssl_http_session_impl>( srv, lst, ctx,
                                               std::move( stream ),
                                               ep,
                                               std::move( storage ) );

                plain_ptr_ = nullptr;
            }

            void ws_session_t::send(message m)
            {

            }

            plain_http_session_impl::plain_http_session_impl(
                application_impl_base& srv, listener& lst, stream_type stream,
                endpoint_type ep, flat_storage storage): http_session_base(
                                                             srv, lst, ep,
                                                             std::move(storage))
                                                         , stream_(std::move(stream))
            {
                doc_root = srv.doc_root();
            }

            ssl_http_session_impl::ssl_http_session_impl(
                application_impl_base& srv, listener& lst,
                asio::ssl::context& ctx, stream_type stream, endpoint_type ep,
                flat_storage storage): http_session_base(
                                           srv, lst, ep, std::move(storage))
                                       , stream_(std::move(stream), ctx)
            {
                doc_root = srv.doc_root();
            }

            void ws_session_t::send( nlohmann::json const& jv )
            {
                // TODO: Complete send() call in session_t
            }

        }
    }
}
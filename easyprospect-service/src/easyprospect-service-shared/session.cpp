
#include <easyprospect-service-shared/message.hpp>
#include <easyprospect-service-shared/session.hpp>
#include <spdlog/spdlog.h>
using namespace easyprospect::service::shared;

namespace easyprospect
{
namespace service
{
    namespace shared
    {
        template <class Derived>
        void ws_session_base<Derived>::set_wrapper(boost::shared_ptr<ws_session_t> w)
        {
            wrapper_ = w;
        }

        template <class Derived>
        boost::shared_ptr<ws_session_t> ws_session_base<Derived>::get_wrapper()
        {
            return wrapper_;
        }

        boost::optional<boost::filesystem::path> session::doc_root() const
        {
            return doc_root_;
        }

        void ws_session_t::run(websocket::request_type req)
        {
            if (plain_ptr_)
            {
                plain_ptr_->run(req);
            }
        }

        void ws_session_t::set_dispatch_impl(std::function<void(rpc_call&, shared::user&, ws_session_t&)> val)
        {
            if (plain_ptr_)
            {
                plain_ptr_->set_dispatch_impl(val);
            }
        }

        void ws_session_t::set_wrapper(boost::shared_ptr<ws_session_t> w)
        {
            if (plain_ptr_)
            {
                plain_ptr_->set_wrapper(w);
            }
        }

        auto ws_session_t::get_primary_user() const
        {
            return primary_user_;
        }

        plain_http_session_impl::plain_http_session_impl(
            application_impl_base& srv,
            listener&              lst,
            stream_type            stream,
            endpoint_type          ep,
            flat_storage           storage) :
            http_session_base(srv, lst, ep, std::move(storage)),
            stream_(std::move(stream))
        {
            doc_root = srv.doc_root();
            set_dispatch_impl(srv.get_dispatch_impl());
        }

        ssl_http_session_impl::ssl_http_session_impl(
            application_impl_base& srv,
            listener&              lst,
            asio::ssl::context&    ctx,
            stream_type            stream,
            endpoint_type          ep,
            flat_storage           storage) :
            http_session_base(srv, lst, ep, std::move(storage)),
            stream_(std::move(stream), ctx)
        {
            doc_root = srv.doc_root();
        }

        void ws_session_t::send(nlohmann::json const& jv)
        {
            if (plain_ptr_)
            {
                plain_ptr_->send(jv);
            }
        }

        void ws_session_t::send(message m)
        {
            if (plain_ptr_)
            {
                plain_ptr_->send(m);
            }
        }

    } // namespace shared
} // namespace service
} // namespace easyprospect
#include <easyprospect-service-shared/listener.h>
#include <easyprospect-service-shared/server.h>

void run_ws_session(
    easyprospect::service::shared::application_impl_base& srv,
    easyprospect::service::shared::listener&              lst,
    stream_type                    stream,
    endpoint_type                  ep,
    websocket::request_type        req)
{
}

void run_ws_session(
    easyprospect::service::shared::application_impl_base& srv,
    easyprospect::service::shared::listener&              lst,
    beast::ssl_stream<stream_type> stream,
    endpoint_type                  ep,
    websocket::request_type        req)
{
    // auto sp = boost::make_shared<shared::ssl_ws_session_impl>(srv.get_doc_root(), std::move(stream), ep);
    auto sp = boost::make_shared<easyprospect::service::shared::ws_session_t>(srv.get_doc_root(), std::move(stream), ep);
    sp->set_dispatch_impl(srv.get_dispatch_impl());
    sp->set_epjs_process_req_impl(srv.get_epjs_process_req_impl());

    sp->set_wrapper(sp);
    sp->run(std::move(req));
}
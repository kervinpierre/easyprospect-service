

#include <boost/beast/http/error.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket/rfc6455.hpp>

#include <spdlog/spdlog.h>
#include <easyprospect-service-shared/session.hpp>

#include <easyprospect-service-shared/easyprospect-http-request.h>

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

        void ws_session_t::run(boost::beast::websocket::request_type req)
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

        void ws_session_t::set_epjs_process_req_impl(epjs_process_req_impl_type val)
        {
            if (plain_ptr_)
            {
                plain_ptr_->set_epjs_process_req_impl(val);
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
            boost::asio::ip::tcp::endpoint          ep,
            boost::beast::flat_buffer           storage) :
            http_session_base(srv, lst, ep, std::move(storage)),
            stream_(std::move(stream))
        {

            doc_root_ = srv.get_doc_root();
            epjs_exts_ = srv.get_epjs_url_path_regex();
            set_dispatch_impl(srv.get_dispatch_impl());
            set_epjs_process_req_impl( srv.get_epjs_process_req_impl());
            set_send_worker_req_impl( srv.get_send_worker_req_impl());
        }

        ssl_http_session_impl::ssl_http_session_impl(
            application_impl_base& srv,
            listener&              lst,
            boost::asio::ssl::context&    ctx,
            stream_type            stream,
            boost::asio::ip::tcp::endpoint          ep,
            boost::beast::flat_buffer           storage) :
            http_session_base(srv, lst, ep, std::move(storage)),
            stream_(std::move(stream), ctx)
        {
            doc_root_ = srv.get_doc_root();
        }

        void ep_make_req(boost::beast::http::request_parser<boost::beast::http::string_body>& pr)
        {
            std::stringstream out;

            out << pr.get().base();
            spdlog::debug(out.str());

            // std::stringstream rawStr;
            // rawStr << pr;

            boost::optional<uint64_t> cl;

            auto rq = pr.get();
            
            rq.content_length(cl);
            auto rt = rq[boost::beast::http::field::content_type];

            auto b = rq.body();

            auto u = rq.target();

            spdlog::debug(b);
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

        

        
        // Return a reasonable mime type based on the extension of a file.
        boost::beast::string_view mime_type(boost::beast::string_view path)
        {
            using boost::beast::iequals;
            auto const ext = [&path] {
                auto const pos = path.rfind(".");
                if (pos == boost::beast::string_view::npos)
                    return boost::beast::string_view{};
                return path.substr(pos);
            }();
            if (iequals(ext, ".htm"))
                return "text/html";
            if (iequals(ext, ".html"))
                return "text/html";
            if (iequals(ext, ".php"))
                return "text/html";
            if (iequals(ext, ".css"))
                return "text/css";
            if (iequals(ext, ".txt"))
                return "text/plain";
            if (iequals(ext, ".js"))
                return "application/javascript";
            if (iequals(ext, ".json"))
                return "application/json";
            if (iequals(ext, ".xml"))
                return "application/xml";
            if (iequals(ext, ".swf"))
                return "application/x-shockwave-flash";
            if (iequals(ext, ".flv"))
                return "video/x-flv";
            if (iequals(ext, ".png"))
                return "image/png";
            if (iequals(ext, ".jpe"))
                return "image/jpeg";
            if (iequals(ext, ".jpeg"))
                return "image/jpeg";
            if (iequals(ext, ".jpg"))
                return "image/jpeg";
            if (iequals(ext, ".gif"))
                return "image/gif";
            if (iequals(ext, ".bmp"))
                return "image/bmp";
            if (iequals(ext, ".ico"))
                return "image/vnd.microsoft.icon";
            if (iequals(ext, ".tiff"))
                return "image/tiff";
            if (iequals(ext, ".tif"))
                return "image/tiff";
            if (iequals(ext, ".svg"))
                return "image/svg+xml";
            if (iequals(ext, ".svgz"))
                return "image/svg+xml";
            return "application/text";
        }
    } // namespace shared
} // namespace service
} // namespace easyprospect
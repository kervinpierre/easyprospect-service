#include <easyprospect-service-shared/session.hpp>
#include <boost/asio/yield.hpp>

namespace easyprospect
{
namespace service
{
    namespace shared
    {
        template <class Derived>
        void http_session_base<Derived>::operator()(boost::beast::error_code ec, std::size_t bytes_transferred, bool need_eof)
        {
            boost::ignore_unused(bytes_transferred);
            reenter(*this)
            {
                // Set the expiration
                impl()->expires_after(std::chrono::seconds(30));

                // A new HTTP parser is required for each message
                pr_.emplace();

                // Set some limits to discourage attackers.
                pr_->body_limit(64 * 1024);
                pr_->header_limit(2048);

                //// 1. Read from impl()->stream()
                //// 2. Save in a local buffer and also send the the Beast request parser
                //// 3. Check the parser for an arbitrary header
                //// 4. Decide to if to stop parsing the stream
                //// 5. If parsing the stream is done send all data to a selected open socket
                //// 6. continue reading impl()->stream(), and passing data directly to that open socket
                //// bool header_found = false;
                // do
                //{
                //    // auto& strm = impl()->stream();
                //    // boost::beast::ssl_stream<stream_type> strm = impl()->stream();
                //
                //    yield impl()->stream().async_read_some(storage_.prepare(100), bind_front(this));
                //    if (ec)
                //    {
                //        spdlog::debug(ec.message());
                //    }
                //    storage_.commit(bytes_transferred);
                //
                //    spdlog::debug(storage_.size());
                //
                //    ec = boost::system::error_code();
                //
                //    // TODO: KP. check error code
                //    // auto pr_res = pr_->put(storage_, ec);
                //    pr_->put(storage_.cdata(), ec);
                //    if (ec)
                //    {
                //        spdlog::debug(ec.message());
                //    }
                //    spdlog::debug((char*)(storage_.data().data()));
                //    spdlog::debug(storage_.capacity());
                //    spdlog::debug(storage_.size());
                //    // ep_make_req(*pr_);
                //    // TODO: KP. Check for ec code, keep in mind header may be invalid
                //    // TODO: KP. Check header for arbitrary header, set "header_found" boolean variable or break
                //    ;
                //    // } while (!(pr_->is_header_done() && header_found));
                //} while (!(pr_->is_header_done())); // or is_done()

                // // Read the next HTTP request
                yield boost::beast::http::async_read(impl()->stream(), storage_, *pr_, bind_front(this));

                // This means they closed the connection
                if (ec == boost::beast::http::error::end_of_stream)
                {
                    return impl()->do_close();
                }

                // Handle the error, if any
                if (ec)
                    return impl()->fail(ec, "boost::beast::http::async_read");

                // TODO: KP. PROXY the HTTP request to a process here
                if (send_worker_req_impl_)
                {
                    send_worker_req_impl_(easyprospect_http_request_builder{*pr_}.to_request(), ec);
                }

                // if (1)
                //{
                //    // Write the existing read data
                //    // TODO: Kp. Pass the parser to get_proxy_stream for routing logic
                //  //  yield this->get_proxy_stream(impl()->stream()).async_write(storage_.cdata(), bind_front(this));
                //    if (ec)
                //    {
                //        spdlog::debug(ec.message());
                //    }

                //    while (!ec) // Stop when message is done
                //    {
                //        // Read then write loop?

                //        // TODO: Kp. How do we detect HTTP message boundaries?
                //        yield impl()->stream().async_read_some(storage_.prepare(100), bind_front(this));
                //        if (ec)
                //        {
                //            spdlog::debug(ec.message());
                //        }
                //        storage_.commit(bytes_transferred);

                //       // yield this->get_proxy_stream(impl()->stream())
                //        //    .async_write_some(storage_.cdata(), bind_front(this));
                //        if (ec)
                //        {
                //            spdlog::debug(ec.message());
                //        }
                //    }
                //}

                // From here and below is really about the worker

                // See if it is a WebSocket Upgrade
                if (boost::beast::websocket::is_upgrade(pr_->get()))
                {
                    // Turn off the expiration timer
                    impl()->expires_never();

                    // Convert the request type
                    boost::beast::websocket::request_type req(pr_->release());

                    // Create a WebSocket session by transferring the socket

                    // EPSRV run_ws_session simply forwards to workers
                    // WORKER run_ws_session does some actual work

                    return this->run_ws_session(doc_root_, lst_, std::move(impl()->stream()), ep_, std::move(req));

                    // return run_ws_session(
                    //    srv_, lst_,
                    //    std::move(impl()->stream()),
                    //    ep_,
                    //    std::move(req));
                }
                // Send the response
                // yield handle_request(doc_root_, epjs_exts_, epjs_process_req_impl_, pr_->release(),
                // send_lambda{*this});
                yield handle_request(doc_root_, epjs_exts_, epjs_process_req_impl_, pr_->release(), [this](auto&& msg) {
                    std::stringstream sstr;
                    sstr << "send_lambda() : " << msg.base() << std::endl;

                    spdlog::debug(sstr.str());

                    // The lifetime of the message has to extend
                    // for the duration of the async operation so
                    // we use a shared_ptr to manage it.
                    // auto sp = std::make_shared<boost::beast::http::message<isRequest, Body, Fields>>(std::move(msg));
                    auto sp =
                        std::make_shared<std::remove_reference_t<decltype(msg)>>(std::forward<decltype(msg)>(msg));

                    // Write the response
                    auto self = bind_front(this);
                    boost::beast::http::async_write(
                        this->impl()->stream(), *sp, [self, sp](boost::beast::error_code ec, std::size_t bytes_transferred) {
                            self(ec, bytes_transferred, sp->need_eof());
                        });
                });

                // Handle the error, if any
                if (ec)
                    return impl()->fail(ec, "boost::beast::http::async_write");

                if (need_eof)
                {
                    // This means we should close the connection, usually because
                    // the response indicated the "Connection: close" semantic.
                    return impl()->do_close();
                }
            }
        }

        template <class Derived>
        void http_session_base<Derived>::run_ws_session(
            boost::optional<boost::filesystem::path> doc_root,
            listener&                                lst,
            stream_type                              str,
            boost::asio::ip::tcp::endpoint                            ep,
            boost::beast::websocket::request_type                  req)
        {
            //    shared::run_ws_session(srv, lst, std::move(str), ep, req);

            //       run_ws_session_plain_func(srv, lst, std::move(str), ep, req);

            // 2nd
            // auto sp = boost::make_shared<plain_ws_session_impl>(doc_root_, std::move(str), ep);
            // sp->run(std::move(req));

            // 3rd
            std::stringstream sstr;

            sstr << "run_ws_session(): " << req << std::endl;

            spdlog::debug(sstr.str());

            // auto sp = boost::make_shared<shared::plain_ws_session_impl>(srv.get_doc_root(), std::move(stream), ep);
            auto sp = boost::make_shared<shared::ws_session_t>(doc_root, std::move(str), ep);
            sp->set_dispatch_impl(get_dispatch_impl());
            sp->set_epjs_process_req_impl(get_epjs_process_req_impl());
            sp->set_wrapper(sp);
            sp->run(std::move(req));
        }

        template void http_session_base<ssl_http_session_impl>::operator()(
            boost::beast::error_code ec,
            std::size_t       bytes_transferred,
            bool              need_eof);

        template void http_session_base<plain_http_session_impl>::operator()(
            boost::beast::error_code ec,
            std::size_t       bytes_transferred,
            bool              need_eof);
    } // namespace shared
} // namespace service
} // namespace easyprospect
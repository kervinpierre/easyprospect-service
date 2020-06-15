#include <easyprospect-service-shared/service.hpp>
#include <easyprospect-web-worker/worker-server-app.h>
#include <easyprospect-v8/easyprospect-v8.h>

namespace easyprospect
{
namespace service
{
    namespace web_worker
    {
        application_impl::application_impl(
            easyprospect::service::config::easyprospect_config_service_core
            cfg):
            cfg_(std::move(cfg)), timer_(this->make_executor()),
            signals_(timer_.get_executor(), SIGINT, SIGTERM),
            shutdown_time_(never()), stop_(false),
            channel_list_(make_channel_list(*this))
        {
            timer_.expires_at(never());

            set_dispatch_impl(
                [this](
                easyprospect::service::shared::rpc_call& r,
                easyprospect::service::shared::user& u,
                easyprospect::service::shared::ws_session_t& s)
                {
                    this->channel_list_->dispatch(r, u, s);
                });

            // process ebjs
            // TODO: KP. Pass in full URL including parameters, and all headers, including cookies.  Parsed or
            // unparsed.
            set_epjs_process_req_impl(
                [this](std::string resolved_path, std::string doc_root,
                       std::string target)
                {
                    // TODO: KP. Do things with the path

                    std::stringstream ss;

                    ss << "set_epjs_process_req_impl()" << std::endl
                        << "resolved_path\t:" << resolved_path << std::endl
                        << "doc_root\t:" << doc_root << std::endl
                        << "target\t:" << target << std::endl;

                    spdlog::debug(ss.str());

                    if (!boost::filesystem::exists(resolved_path))
                    {
                        spdlog::info("resolved_path '{}' does not exist",
                                     resolved_path);

                        // TODO: KP. Route the path to a real file or a buffer
                    }

                    std::ifstream t(resolved_path);
                    std::stringstream script_buff;
                    script_buff << t.rdbuf();

                    std::shared_ptr<Platform> platform =
                        platform::NewDefaultPlatform();
                    std::unique_ptr<easyprospect::ep_v8::api::easyprospect_v8>
                        ep =
                            easyprospect::ep_v8::api::easyprospect_v8::create<
                                easyprospect::ep_v8::api::easyprospect_v8>(
                                platform);

                    ep->init();

                    int r = 0;
                    unsigned int id = 0;
                    if ((r = ep->create_context(id)) != Success)
                        throw std::logic_error("Context error");

                    // Exception thrown on error
                    auto res = ep->run_javascript(id, script_buff.str());

                    ep->remove_context(id);

                    std::string res2 = res->ToString();

                    return res2;
                });

            // Create the control server
            try
            {
                control_client_ = easyprospect::service::web_worker::make_control_server(cfg);
            }
            catch (std::logic_error& ex)
            {
                spdlog::error("Control server failed. {}", ex.what());
                throw std::logic_error("control server failed.");
            }

            if (control_client_ == nullptr)
            {
                spdlog::error("Control server failed.");
                throw std::logic_error("control server failed.");
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));

            control::process_message_startup st;
            control_client_->send(st);

            // TODO: KP. Move URL parsing to some place needed.
            // Base
            // UriUriW baseUri;
            // int res = uriParseSingleUriW(&baseUri, L"example.com", NULL);
            // if (res != 0)
            //{
            //    ;
            //}

            web_worker::make_system_channel(*this);
        }

        void application_impl::insert(std::unique_ptr<shared::service> sp)
        {
            if (running_)
                throw std::logic_error("server already running");

            services_.emplace_back(std::move(sp));
        }

        void application_impl::run()
        {
            if (running_)
                throw std::logic_error("server already running");

            running_ = true;

            // Start all agents
            for (auto const& sp : services_)
                sp->on_start();

            // Capture SIGINT and SIGTERM to perform a clean shutdown
            signals_.async_wait(boost::beast::bind_front_handler(&application_impl::on_signal, this));

#ifndef LOUNGE_USE_SYSTEM_EXECUTOR
            std::vector<std::thread> vt;
            while (vt.size() < cfg_.get_num_threads())
                vt.emplace_back([this] { this->ioc_.run(); });
#endif
            // Block the main thread until stop() is called
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] { return stop_.load(); });
            }

            // Notify all agents to stop
            auto agents = std::move(services_);
            for (auto const& sp : agents)
                sp->on_stop();

                // services must be kept alive until after
                // all executor threads are joined.

                // If we get here, then the server has
                // stopped, so join the threads before
                // destroying them.

#ifdef LOUNGE_USE_SYSTEM_EXECUTOR
            boost::asio::system_executor{}.context().join();
#else
            for (auto& t : vt)
                t.join();
#endif
        }
    } // namespace web_worker
} // namespace service
} // namespace easyprospect
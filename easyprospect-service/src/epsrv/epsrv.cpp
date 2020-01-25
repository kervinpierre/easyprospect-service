#include <iostream>

#include <boost/config.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>

#include <easyprospect-web/server.hpp>
#include <easyprospect-web/listener.hpp>
#include <easyprospect-config/easyprospect-config-service.h>

#ifdef BOOST_MSVC
# ifndef WIN32_LEAN_AND_MEAN // VC_EXTRALEAN
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  undef WIN32_LEAN_AND_MEAN
# else
#  include <windows.h>
# endif
#endif

//------------------------------------------------------------------------------

/** Create a server.

    The configuration file is loaded,
    and all child objects are created.
*/
extern
std::unique_ptr<server>
make_server(
    easyprospect::service::config::easyprospect_config_service_core config_curr);

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    easyprospect::service::config
        ::easyprospect_config_service_shell shell;
    easyprospect::service::config::easyprospect_config_service_core_builder builder;

    try
    {
        builder = easyprospect::service::config
            ::easyprospect_config_service_shell
	            ::init_args(argc, argv);
    }
    catch (std::logic_error ex)
    {
	    return 1;
    }

    auto res = builder.to_config();

    if(res.get_display_help())
    {
        std::ostringstream disStr{};

        disStr << easyprospect::service::config
            ::easyprospect_config_service_shell::get_description();
        disStr << std::endl << builder.get_help_str() << std::endl;

        std::cout << disStr.str();
    }

#if BOOST_MSVC
    {
        int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
        flags |= _CRTDBG_LEAK_CHECK_DF;
        _CrtSetDbgFlag(flags);
    }
#endif

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto main_logger = std::make_shared<spdlog::logger>("", console_sink);

    spdlog::level::level_enum level;
    switch (res.get_debug_level())
    {
    case easyprospect::service::config::ep_debug_level_type::ep_off:
        level = spdlog::level::off;
        break;

    case easyprospect::service::config::ep_debug_level_type::ep_trace:
        level = spdlog::level::trace;
        break;

    case easyprospect::service::config::ep_debug_level_type::ep_debug:
        level = spdlog::level::debug;
        break;

    case easyprospect::service::config::ep_debug_level_type::ep_info:
        level = spdlog::level::info;
        break;

    case easyprospect::service::config::ep_debug_level_type::ep_warn:
        level = spdlog::level::warn;
        break;

    case easyprospect::service::config::ep_debug_level_type::ep_fatal:
        level = spdlog::level::critical;
        break;

    default:
        level = spdlog::level::err;
    }

    main_logger->set_level(level);
    spdlog::set_default_logger(main_logger);

    auto vb = res.get_verbosity();

    spdlog::debug("starting epsrv");

    spdlog::debug( res.str());

    std::stringstream sstr;

    sstr << "\nexe: '" << boost::filesystem::system_complete(argv[0]) << std::endl
         << "cwd: '" << boost::filesystem::current_path() << std::endl;

    spdlog::debug(sstr.str());

    // Create the server
     beast::error_code ec;
    auto srv = make_server(
        res);
    if (!srv)
        return EXIT_FAILURE;

    srv->run();

    return EXIT_SUCCESS;
    
    //try
    //{
    //  //  auto server = std::make_unique<easyprospect::web::server::EpHTTPServer>();

    // //   server->run_application();

    //        // Check command line arguments.
    //    if (argc != 5)
    //    {
    //        std::cerr <<
    //            "Usage: advanced-server-flex <address> <port> <doc_root> <threads>\n" <<
    //            "Example:\n" <<
    //            "    advanced-server-flex 0.0.0.0 8080 . 1\n";
    //        return EXIT_FAILURE;
    //    }
    //    auto const address = net::ip::make_address(argv[1]);
    //    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    //    auto const doc_root = std::make_shared<std::string>(argv[3]);
    //    auto const threads = std::max<int>(1, std::atoi(argv[4]));

    //    // The io_context is required for all I/O
    //    net::io_context ioc{ threads };

    //    // The SSL context is required, and holds certificates
    //    ssl::context ctx{ ssl::context::tlsv12 };

    //    // This holds the self-signed certificate used by the server
    //    load_server_certificate(ctx);

    //    // Create and launch a listening port
    //    std::make_shared<listener>(
    //        ioc,
    //        ctx,
    //        tcp::endpoint{ address, port },
    //        doc_root)->run();

    //    // Capture SIGINT and SIGTERM to perform a clean shutdown
    //    net::signal_set signals(ioc, SIGINT, SIGTERM);
    //    signals.async_wait(
    //        [&](beast::error_code const&, int)
    //        {
    //            // Stop the `io_context`. This will cause `run()`
    //            // to return immediately, eventually destroying the
    //            // `io_context` and all of the sockets in it.
    //            ioc.stop();
    //        });

    //    // Run the I/O service on the requested number of threads
    //    std::vector<std::thread> v;
    //    v.reserve(threads - 1);
    //    for (auto i = threads - 1; i > 0; --i)
    //        v.emplace_back(
    //            [&ioc]
    //            {
    //                ioc.run();
    //            });
    //    ioc.run();

    //    // (If we get here, it means we got a SIGINT or SIGTERM)

    //    // Block until all the threads exit
    //    for (auto& t : v)
    //        t.join();
    //}
    //catch (const std::exception & x)
    //{
    //    std::cerr << "Exception caught: " << x.what() << std::endl;
    //    return 2;
    //}
    //catch (...)
    //{
    //    std::cerr << "Unknown exception" << std::endl;
    //    return 3;
    //}

    //return 0;
}

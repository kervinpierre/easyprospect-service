#include <iostream>

#include <boost/config.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <easyprospect-web-worker/server.h>
#include <easyprospect-config/easyprospect-config-service.h>

#include <easyprospect-config/logging.h>
#include <easyprospect-config/logging.h>

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

/** Create a web worker.

    The configuration file is loaded,
    and all child objects are created.
*/

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
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

    if (res.get_display_help())
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

    std::shared_ptr<spdlog::logger> main_logger;

    auto logfile = res.get_log_file();

    if (logfile)
    {
        std::string logFileName = logfile->generic_string();

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFileName, true);
        //file_sink->set_level()

        spdlog::sinks_init_list sink_list = { file_sink, console_sink };

        main_logger = std::make_shared<spdlog::logger>("", sink_list.begin(), sink_list.end());
    }
    else
    {
        main_logger = std::make_shared<spdlog::logger>("", console_sink);
    }

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

    spdlog::debug("starting ep-web-worker");

    spdlog::debug(res.str());

    std::stringstream sstr;

    sstr << "\nexe: '" << boost::filesystem::system_complete(argv[0]) << std::endl
        << "cwd: '" << boost::filesystem::current_path() << std::endl;

    spdlog::debug(sstr.str());

    // Create the server
    beast::error_code ec;
    auto srv = easyprospect::service::web_worker::make_server(
        res);
    if (!srv)
        return EXIT_FAILURE;

    srv->run();

    return EXIT_SUCCESS;
}

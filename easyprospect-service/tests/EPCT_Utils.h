#pragma once

//#ifndef BOOST_ALL_DYN_LINK
//#   define BOOST_ALL_DYN_LINK
//#endif

#define EPCT_JS_SOURCE_REGEX "^TC(\\d{4})_T(\\d{4})_([^_]+)_(.*)$"

#include <string>
#include <easyprospect-v8/easyprospect-v8.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/msvc_sink.h>
#include <boost/test/unit_test.hpp>

using namespace easyprospect::ep_v8::api;

struct setup_fixture
{
    setup_fixture()
    {
        // Get current flag
        int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

        // Turn on leak-checking bit.
       // tmpFlag |= _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_EVERY_16_DF |
       //            _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF;

        // Set flag to the new value.
        _CrtSetDbgFlag(tmpFlag);


#ifdef BOOST_MSVC
        auto console_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
#endif

        std::shared_ptr<spdlog::logger> main_logger;
        main_logger = std::make_shared<spdlog::logger>("", console_sink);

       // auto main_logger = spdlog::stdout_logger_mt("console");

        main_logger->set_level(spdlog::level::trace);
        spdlog::set_default_logger(main_logger);

        spdlog::debug("setup fixture");

        std::shared_ptr<Platform> platform = platform::NewDefaultPlatform();
        ep = easyprospect_v8::create<easyprospect_v8>(platform);

        ep->init(boost::unit_test::framework::master_test_suite().argv[0]);
        ep->create_context(id);


        spdlog::debug("starting test suite");
    }

    void setup()
    {
        spdlog::debug("setup");

    }
    void teardown()
    {
        spdlog::debug("teardown");
    }

    ~setup_fixture()
    {
        spdlog::debug("teardown fixture");

        // Allow the heap-based platform to be deleted after destroy() is called.
        std::shared_ptr<Platform> platform = ep->get_platform();

        ep.reset();
        easyprospect_v8::destroy();
    }

    static std::unique_ptr<easyprospect_v8> ep;
    static unsigned int id;
};

namespace EPCT
{
    struct EPCT_FILE_TEST
    {
        std::string testCase;
        std::string test;
        std::string ns;
        std::string desc;
        std::string path;
    };

    class EPCT_Utils
    {

    public:
        //Create a context, run a script and remove the context
         static std::string ContextSingleUseToString(easyprospect_v8& ep, std::string script);
         static std::unique_ptr<EpJsResult> ContextSingleUse(easyprospect_v8& ep, std::string script);

         static bool ContextSingleUseFromFile(easyprospect_v8& ep, std::string scriptPath);

         //Run a script in a context
         static std::string Context(easyprospect_v8& ep, unsigned id, std::string script);
         static bool ContextFromFile(unsigned int id, std::string scriptPath);
         
         static std::vector<EPCT::EPCT_FILE_TEST> 
             ProcessJSDir(std::string testDirStr, int testCaseId = 0);

         static std::string readFile(std::string path);
    };
}


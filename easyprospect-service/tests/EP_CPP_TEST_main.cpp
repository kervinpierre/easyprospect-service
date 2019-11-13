#define CATCH_CONFIG_EXTERNAL_INTERFACES
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <easyprospect-config/easyprospect-config.h>
#include <easyprospect-config/easyprospect-config-v8.h>

using namespace easyprospect::service::config;

//struct CatchListener : Catch::TestEventListenerBase
//{
//    using TestEventListenerBase::TestEventListenerBase;
//
//    void testCaseStarting(Catch::TestCaseInfo const&) override
//    {
//    }
//
//    void testCaseEnded(Catch::TestCaseStats const&) override
//    {
//    }
//};
//
//CATCH_REGISTER_LISTENER(CatchListener)

int main(int argc, char* argv[])
{
    spdlog::info("EP_CPP_TEST_main");

    //EPCT_Utils::ep.Init(argv[0]);
    return Catch::Session().run(argc, argv);
}

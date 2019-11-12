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

TEST_CASE("CmdLine.V8. Simple Display help")
{
    char* test_argv[] = { "EP_CPP_TEST_main", "--help", NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    //std::vector<std::string> arguments = { "EP_CPP_TEST_main", "--help" };

    //std::vector<char*> argv;
    //for (const auto& arg : arguments)
    //    argv.push_back((char*)arg.data());
    //argv.push_back(nullptr);

    auto cnf = easyprospect_config_v8_shell::get_config(ep_config_type::none);
    auto opts = easyprospect_config_v8_shell::get_options(cnf);
    auto res = easyprospect_config_v8_shell
                ::parse_options(cnf, opts, test_argc, test_argv);
   
    REQUIRE(res.get_display_help());
    REQUIRE_FALSE(res.get_display_version());
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_none);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);

}

int main(int argc, char* argv[])
{
    spdlog::info("EP_CPP_TEST_main");

    //EPCT_Utils::ep.Init(argv[0]);
    return Catch::Session().run(argc, argv);
}

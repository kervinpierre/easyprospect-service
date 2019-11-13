#include <catch.hpp>
#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <easyprospect-config/easyprospect-config.h>
#include <easyprospect-config/easyprospect-config-v8.h>

using namespace easyprospect::service::config;

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

TEST_CASE("CmdLine.V8. Simple Display version")
{
    char* test_argv[] = { "EP_CPP_TEST_main", "--version", NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    auto cnf = easyprospect_config_v8_shell::get_config(ep_config_type::none);
    auto opts = easyprospect_config_v8_shell::get_options(cnf);
    auto res = easyprospect_config_v8_shell
        ::parse_options(cnf, opts, test_argc, test_argv);

    REQUIRE_FALSE(res.get_display_help());
    REQUIRE(res.get_display_version());
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_none);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}

TEST_CASE("CmdLine.V8. Remainder")
{
    std::vector<std::string> testArg = { "--help" };
    char* test_argv[] = { "EP_CPP_TEST_main", "--",
                          const_cast<char*>(testArg[0].data()), NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    auto cnf = easyprospect_config_v8_shell::get_config(ep_config_type::none);
    auto opts = easyprospect_config_v8_shell::get_options(cnf);
    auto res = easyprospect_config_v8_shell
        ::parse_options(cnf, opts, test_argc, test_argv);

    REQUIRE_FALSE(res.get_display_help());
    REQUIRE_FALSE(res.get_display_version());
    REQUIRE(res.get_remainder_args().get() == testArg);
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_none);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}

TEST_CASE("CmdLine.V8. Remainder x 2")
{
    std::vector<std::string> testArg = { "--help", "--version" };
    char* test_argv[] = { "EP_CPP_TEST_main", "--",
                          const_cast<char*>(testArg[0].data()),
                          const_cast<char*>(testArg[1].data()),
                            NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    auto cnf = easyprospect_config_v8_shell::get_config(ep_config_type::none);
    auto opts = easyprospect_config_v8_shell::get_options(cnf);
    auto res = easyprospect_config_v8_shell
        ::parse_options(cnf, opts, test_argc, test_argv);

    REQUIRE_FALSE(res.get_display_help());
    REQUIRE_FALSE(res.get_display_version());
    REQUIRE(res.get_remainder_args().get() == testArg);
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_none);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}

TEST_CASE("CmdLine.V8. Remainder x 2 - plus help")
{
    std::vector<std::string> testArg = { "--help", "--version" };
    char* test_argv[] = { "EP_CPP_TEST_main", "--help", "--",
                          const_cast<char*>(testArg[0].data()),
                          const_cast<char*>(testArg[1].data()),
                            NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    auto cnf = easyprospect_config_v8_shell::get_config(ep_config_type::none);
    auto opts = easyprospect_config_v8_shell::get_options(cnf);
    auto res = easyprospect_config_v8_shell
        ::parse_options(cnf, opts, test_argc, test_argv);

    REQUIRE(res.get_display_help());
    REQUIRE_FALSE(res.get_display_version());
    REQUIRE(res.get_remainder_args().get() == testArg);
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_none);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}
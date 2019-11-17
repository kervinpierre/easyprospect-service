#include <catch.hpp>
#include <iostream>
#include <cstdio>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <easyprospect-config/easyprospect-config.h>
#include <easyprospect-config/easyprospect-config-v8.h>

using namespace easyprospect::service::config;

TEST_CASE("CmdLine.V8. Unknown Option handling")
{
    char* test_argv[] = { "EP_CPP_TEST_main",
                            "--mistake-option", NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    easyprospect_config_v8_core_builder builder;

    SECTION("Unknowned option parse")
    {
        REQUIRE_THROWS_AS(builder = easyprospect_config_v8_shell
            ::init_args(test_argc, test_argv), boost::program_options::unknown_option);
    }

    auto res = builder.to_config();

    REQUIRE_FALSE(res.get_display_help());
    REQUIRE_FALSE(res.get_display_version());
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_none);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}

TEST_CASE("CmdLine.V8. Simple Display help")
{
    char* test_argv[] = { "EP_CPP_TEST_main", "--help", NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    //std::vector<std::string> arguments = { "EP_CPP_TEST_main", "--help" };

    //std::vector<char*> argv;
    //for (const auto& arg : arguments)
    //    argv.push_back((char*)arg.data());
    //argv.push_back(nullptr);

    auto builder = easyprospect_config_v8_shell
                ::init_args(test_argc, test_argv);
    auto res = builder.to_config();

    REQUIRE(res.get_display_help());
    REQUIRE_FALSE(res.get_display_version());
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_none);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}

TEST_CASE("CmdLine.V8. Simple Display version")
{
    char* test_argv[] = { "EP_CPP_TEST_main", "--version", NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    auto builder = easyprospect_config_v8_shell
        ::init_args(test_argc, test_argv);
    auto res = builder.to_config();

    REQUIRE_FALSE(res.get_display_help());
    REQUIRE(res.get_display_version());
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_none);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}

TEST_CASE("CmdLine.V8. Simple Out File")
{
   // auto p1 = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();

    auto p1 = std::tmpnam(nullptr);

    char* test_argv[] = { "EP_CPP_TEST_main", "--output-file",
        p1, NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    auto builder = easyprospect_config_v8_shell
        ::init_args(test_argc, test_argv);
    auto res = builder.to_config();

    auto f1 = res.get_out_file().get();
    auto f2 = boost::filesystem::path(p1);

    auto eres = false;

    {
        boost::filesystem::ofstream f3(f1);
        boost::system::error_code& ec = boost::system::error_code();
        eres = boost::filesystem::equivalent(
            res.get_out_file().get(),
            boost::filesystem::path(p1),
            ec);
        boost::filesystem::remove(f1, ec);
    }

    REQUIRE(eres);

    REQUIRE_FALSE(res.get_display_help());
    REQUIRE_FALSE(res.get_display_version());
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_none);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}

TEST_CASE("CmdLine.V8. Simple Log File")
{
    auto p1 = std::tmpnam(nullptr);

    char* test_argv[] = { "EP_CPP_TEST_main", "--log-file",
        p1, NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    auto builder = easyprospect_config_v8_shell
        ::init_args(test_argc, test_argv);
    auto res = builder.to_config();

    auto f1 = res.get_log_file().get();
    auto f2 = boost::filesystem::path(p1);

    boost::filesystem::ofstream f3(f1);

    auto eres = false;

    {
        boost::filesystem::ofstream f3(f1);
        boost::system::error_code& ec = boost::system::error_code();
        eres = boost::filesystem::equivalent(
            res.get_log_file().get(),
            boost::filesystem::path(p1),
            ec);
        boost::filesystem::remove(f1, ec);
    }

    REQUIRE(eres);

    REQUIRE_FALSE(res.get_display_help());
    REQUIRE_FALSE(res.get_display_version());
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_none);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}

TEST_CASE("CmdLine.V8. Simple Argument File")
{
    auto s1 = std::tmpnam(nullptr);

    char* test_argv[] = { "EP_CPP_TEST_main", "--arg-file",
        s1, NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    auto p1 = boost::filesystem::path(s1);
    boost::filesystem::ofstream f1(p1);

    f1 << "--version --help --debug-level ep_fatal" << std::endl;

    auto builder = easyprospect_config_v8_shell
        ::init_args(test_argc, test_argv);
    auto res = builder.to_config();

    auto eres = false;

    {
        boost::system::error_code ec;
        eres = boost::filesystem::equivalent(
            res.get_arg_file().get(),
            boost::filesystem::path(p1),
            ec);
        boost::filesystem::remove(p1, ec);
    }

    REQUIRE(eres);

    REQUIRE(res.get_display_help());
    REQUIRE(res.get_display_version());
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_fatal);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}

TEST_CASE("CmdLine.V8. Simple Config File")
{
    auto s1 = std::tmpnam(nullptr);

    char* test_argv[] = { "EP_CPP_TEST_main", "--config-file",
        s1, NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    auto p1 = boost::filesystem::path(s1);
    boost::filesystem::ofstream f1(p1);

    f1 << 
R"({
    "debug_level": "ep_fatal"
}
)" << std::endl;

    auto builder = easyprospect_config_v8_shell
        ::init_args(test_argc, test_argv);
    auto res = builder.to_config();

    auto eres = false;

    {
        boost::system::error_code ec;
        eres = boost::filesystem::equivalent(
            res.get_cnf_file().get(),
            boost::filesystem::path(p1),
            ec);
        boost::filesystem::remove(p1, ec);
    }

    REQUIRE(eres);

    REQUIRE_FALSE(res.get_display_help());
    REQUIRE_FALSE(res.get_display_version());
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_fatal);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}

TEST_CASE("CmdLine.V8. Simple PID File")
{
    auto p1 = std::tmpnam(nullptr);

    char* test_argv[] = { "EP_CPP_TEST_main", "--pid-file",
        p1, NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    auto builder = easyprospect_config_v8_shell
        ::init_args(test_argc, test_argv);
    auto res = builder.to_config();

    auto f1 = res.get_pid_file().get();
    auto f2 = boost::filesystem::path(p1);

    boost::filesystem::ofstream f3(f1);

    auto eres = false;

    {
        boost::filesystem::ofstream f3(f1);
        boost::system::error_code& ec = boost::system::error_code();
        eres = boost::filesystem::equivalent(
            res.get_pid_file().get(),
            boost::filesystem::path(p1),
            ec);
        boost::filesystem::remove(f1, ec);
    }

    REQUIRE(eres);

    REQUIRE_FALSE(res.get_display_help());
    REQUIRE_FALSE(res.get_display_version());
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_none);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}

TEST_CASE("CmdLine.V8. Verbosity")
{
    char* test_argv[] = { "EP_CPP_TEST_main", "--verbosity",
                          "maximum",
                            NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    auto builder = easyprospect_config_v8_shell
        ::init_args(test_argc, test_argv);
    auto res = builder.to_config();

    REQUIRE_FALSE(res.get_display_help());
    REQUIRE_FALSE(res.get_display_version());
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_none);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::maximum);
}

TEST_CASE("CmdLine.V8. Debug Level")
{
    char* test_argv[] = { "EP_CPP_TEST_main", "--debug-level",
                          "ep_fatal",
                            NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    auto builder = easyprospect_config_v8_shell
        ::init_args(test_argc, test_argv);
    auto res = builder.to_config();

    REQUIRE_FALSE(res.get_display_help());
    REQUIRE_FALSE(res.get_display_version());
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_fatal);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}

TEST_CASE("CmdLine.V8. Remainder")
{
    std::vector<std::string> testArg = { "--help" };
    char* test_argv[] = { "EP_CPP_TEST_main", "--",
                          const_cast<char*>(testArg[0].data()), NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    auto builder = easyprospect_config_v8_shell
        ::init_args(test_argc, test_argv);
    auto res = builder.to_config();

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

    auto builder = easyprospect_config_v8_shell
        ::init_args(test_argc, test_argv);
    auto res = builder.to_config();

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

    auto builder = easyprospect_config_v8_shell
        ::init_args(test_argc, test_argv);
    auto res = builder.to_config();

    REQUIRE(res.get_display_help());
    REQUIRE_FALSE(res.get_display_version());
    REQUIRE(res.get_remainder_args().get() == testArg);
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_none);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}

TEST_CASE("CmdLine.V8. Source Files")
{
    auto s1 = std::tmpnam(nullptr);
    auto s2 = std::tmpnam(nullptr);
    auto s3 = std::tmpnam(nullptr);

    char* test_argv[] = { "EP_CPP_TEST_main",
        "--source-files", s1,  s2,  s3,
        NULL };
    int test_argc = sizeof(test_argv) / sizeof(char*) - 1;

    auto builder = easyprospect_config_v8_shell
        ::init_args(test_argc, test_argv);
    auto res = builder.to_config();

    auto p1 = boost::filesystem::path(s1);

    auto eres = false;

    {
        boost::filesystem::ofstream f1(p1);
        boost::system::error_code& ec = boost::system::error_code();
        eres = boost::filesystem::equivalent(
            res.get_source_files()->at(0),
            boost::filesystem::path(s1),
            ec);
        boost::filesystem::remove(p1, ec);
    }

    REQUIRE(eres);

    REQUIRE_FALSE(res.get_display_help());
    REQUIRE_FALSE(res.get_display_version());
    REQUIRE(res.get_debug_level() == ep_debug_level_type::ep_none);
    REQUIRE(res.get_verbosity() == ep_verbosity_type::none);
}


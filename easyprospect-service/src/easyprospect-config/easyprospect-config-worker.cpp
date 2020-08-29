#include "easyprospect-config/easyprospect-config-worker.h"

#include "easyprospect-config/easyprospect-config-server.h"

#include <easyprospect-config/easyprospect-config-service.h>
#include <string>
#include <vector>

using namespace easyprospect::service::config;

boost::program_options::options_description easyprospect_config_worker_shell::get_options(
    easyprospect_config_worker_shell& config)
{
    auto desc = easyprospect_config_service_shell::get_options(config);

    desc.add_options()(
        "control-protocol",
        boost::program_options::value<bool>(),
        "Whether to setup the local client/server protocol. Boolean.");

    return desc;
}

void easyprospect_config_worker_shell::parse_options(
    easyprospect_config_worker_core_builder&   builder,
    boost::program_options::variables_map       vm,
    boost::program_options::options_description desc)
{
    if (vm.count("control-protocol"))
    {
        builder.set_control_protocol(vm["control-protocol"].as<bool>());
    }

    easyprospect_config_service_shell::parse_options(builder, vm, desc);
}

std::string easyprospect_config_worker_shell::get_description()
{
    std::ostringstream res("EasyProspect Worker Configuration.");

    return res.str();
}

easyprospect_config_worker_core_builder easyprospect_config_worker_shell::init_args(int test_argc, char* test_argv[])
{
    easyprospect_config_worker_shell           cnf;
    boost::program_options::options_description opts = easyprospect_config_worker_shell::get_options(cnf);
    boost::program_options::variables_map vm = easyprospect_config_worker_shell ::get_map(opts, test_argc, test_argv);

    auto builder = easyprospect_config_worker_core_builder{};

    // Get the config file first, so it can set the builder.
    // Using config first gives it the lowest precedence.
    if (vm.count("config-file"))
    {
        auto cf = vm["config-file"].as<std::string>();
        builder.set_cnf_file(cf);
        builder.read_from_file(cf);
    }

    cnf.parse_options(builder, vm, opts);

    return builder;
}

std::string easyprospect_config_worker_core::str()
{
    std::stringstream sstr;

    sstr << easyprospect_config_service_core::str() << std::endl;

    sstr << "control protocol\t: " << get_control_protocol() << std::endl;

    return sstr.str();
}

const easyprospect_config_worker_core easyprospect_config_worker_core_builder::to_config()
{
    easyprospect_config_worker_core_builder builder;

    easyprospect_config_worker_core res(
        easyprospect_config_core::make_shared_enabler{0},
        display_help_,
        display_version_,
        control_protocol_,
        num_threads_,
        verbosity_,
        debug_level_,
        remainder_args_,
        epjs_url_path_regex_,
        epjs_url_path_regex_str_,
        mime_types_,
        out_file_,
        log_file_,
        arg_file_,
        cnf_file_,
        pid_file_,
        pid_dir_path_,
        webroot_dir_,
        listen_file_,
        listen_dir_path_,
        listeners_);

    return res;
}

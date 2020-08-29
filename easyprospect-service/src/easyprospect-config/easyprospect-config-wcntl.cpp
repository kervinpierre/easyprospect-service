#include <easyprospect-config/easyprospect-config-wcntl.h>
#include <string>
#include <vector>

using namespace easyprospect::service::config;

boost::program_options::options_description easyprospect_config_wcntl_shell::get_options(
    easyprospect_config_wcntl_shell& config)
{
    auto desc = easyprospect_config_service_shell::get_options(config);

    desc.add_options()(
        "worker-exe-use-path",
        boost::program_options::value<bool>(),
        "Whether to use the path for finding launched workers. Boolean.");

    desc.add_options()(
        "num-workers", boost::program_options::value<std::string>(), "Number of workers in our process pool.");

    desc.add_options()(
        "worker-args",
        boost::program_options::value<std::string>(),
        "Quited string of arguments to pass to launched workers");

    desc.add_options()(
        "worker-conf", boost::program_options::value<std::string>(), "A args file for launching workers.");

    desc.add_options()(
        "worker-args-file", boost::program_options::value<std::string>(), "A args file for launching workers.");

    desc.add_options()("worker-exe", boost::program_options::value<std::string>(), "Executable used for the worker.");

    return desc;
}

void easyprospect_config_wcntl_shell::parse_options(
    easyprospect_config_wcntl_core_builder&     builder,
    boost::program_options::variables_map       vm,
    boost::program_options::options_description desc)
{
    if (vm.count("worker-exe-use-path"))
    {
        builder.set_worker_exe_use_path(vm["worker-exe-use-path"].as<std::string>());
    }

    if (vm.count("num-workers"))
    {
        builder.set_num_workers(vm["num-workers"].as<std::string>());
    }

    if (vm.count("worker-exe"))
    {
        builder.set_worker_exe(vm["worker-exe"].as<std::string>());
    }

    if (vm.count("worker-args"))
    {
        builder.set_worker_args(vm["worker-args"].as<std::string>());
    }

    if (vm.count("worker-conf"))
    {
        builder.set_worker_conf(vm["worker-conf"].as<std::string>());
    }

    if (vm.count("worker-args-file"))
    {
        builder.set_worker_args_file(vm["worker-args-file"].as<std::string>());
    }

    easyprospect_config_service_shell::parse_options(builder, vm, desc);
}

std::string easyprospect_config_wcntl_shell::get_description()
{
    std::ostringstream res("EasyProspect Service Configuration.");

    return res.str();
}

easyprospect_config_wcntl_core_builder easyprospect_config_wcntl_shell::init_args(int test_argc, char* test_argv[])
{
    easyprospect_config_wcntl_shell             cnf;
    boost::program_options::options_description opts = easyprospect_config_wcntl_shell::get_options(cnf);
    boost::program_options::variables_map vm = easyprospect_config_wcntl_shell ::get_map(opts, test_argc, test_argv);

    easyprospect_config_wcntl_core_builder builder = easyprospect_config_wcntl_core_builder{};

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

std::string easyprospect_config_wcntl_core::str()
{
    std::stringstream sstr;

    sstr << easyprospect_config_core::str() << std::endl;

    sstr << "worker exe\t: " << (get_worker_exe() ? get_worker_exe()->generic_string() : "") << std::endl;

    return sstr.str();
}

const easyprospect_config_wcntl_core easyprospect_config_wcntl_core_builder::to_config()
{
    easyprospect_config_wcntl_core_builder builder;

    easyprospect_config_wcntl_core res(
        easyprospect_config_core::make_shared_enabler{0},
        display_help_,
        display_version_,
        worker_exe_use_path_,
        num_threads_,
        num_workers_,
        worker_args_,
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
        webroot_dir_, log_file_, listen_dir_path_,
        worker_conf_,
        worker_args_file_,
        worker_exe_, listeners_);

    return res;
}

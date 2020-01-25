#include <string>
#include <vector>
#include <easyprospect-config/easyprospect-config-service.h>

using namespace easyprospect::service::config;

boost::program_options::options_description
easyprospect_config_service_shell::get_options(
    easyprospect_config_service_shell& config)
{
    auto desc = easyprospect_config_cmd::get_options(config);

    desc.add_options()("webroot-dir",
        boost::program_options::value<std::string>(),
        "Webroot directory");

    desc.add_options()("num-threads",
        boost::program_options::value<std::string>(),
        "Number of threads");

    desc.add_options()("listener",
        boost::program_options::value< std::vector<std::string> >(),
        "Listener configurations in a comma-delimited list 'name,address,port' ( without quotes )");

    return desc;
}

void
easyprospect_config_service_shell::parse_options(
    easyprospect_config_service_core_builder& builder,
    boost::program_options::variables_map vm,
    boost::program_options::options_description desc)
{
    if (vm.count("webroot-dir"))
    {
        builder.set_webroot_dir(vm["webroot-dir"].as<std::string>());
    }

    if (vm.count("num-threads"))
    {
        builder.set_num_threads(vm["num-threads"].as<std::string>());
    }

    if (vm.count("listener"))
    {
        auto sListeners = vm["listener"].as<std::vector<std::string>>();
        if (sListeners.empty())
        {
            throw std::logic_error("Missing Listener configuration 'name,address,port'");
        }

        builder.set_listeners(sListeners);
    }

    easyprospect_config_cmd::parse_options(builder,vm,desc);
}

std::string 
easyprospect_config_service_shell::get_description()
{
    std::ostringstream res( "EasyProspect Service Configuration.");

    return res.str();
}

easyprospect_config_service_core_builder
easyprospect_config_service_shell::init_args(int test_argc, char* test_argv[])
{
    easyprospect_config_service_shell cnf;
    boost::program_options::options_description opts
        = easyprospect_config_service_shell::get_options(cnf);
    boost::program_options::variables_map vm
        = easyprospect_config_service_shell
        ::get_map(opts, test_argc, test_argv);

    easyprospect_config_service_core_builder builder
        = easyprospect_config_service_core_builder{};

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

std::string
easyprospect_config_service_listener_conf::str()
{
    std::stringstream sstr;

    sstr << "name\t:" << get_name() << std::endl
         << "port\t:" << get_port() << std::endl
         << "address\t:" << get_address() << std::endl
         << "kind\t:" << get_kind() << std::endl;

    return sstr.str();
}

std::string easyprospect_config_service_core::str()
{
    std::stringstream sstr;

    sstr << easyprospect_config_core::str() << std::endl;

    sstr << "num threads\t: " << get_num_threads() << std::endl
         << "web root\t: " << (get_webroot_dir() ? get_webroot_dir()->generic_string() : "") << std::endl
         << "listener\t: ";

    auto rargs = get_listeners();
    if (rargs)
    {
        for (auto arg : *rargs)
        {
            sstr << "'" << arg.str() << "', ";
        }
    }

    return sstr.str();
}

const easyprospect_config_service_core 
easyprospect_config_service_core_builder::to_config()
{
    easyprospect_config_service_core_builder builder;
     
    easyprospect_config_service_core res(easyprospect_config_core::make_shared_enabler{ 0 }, display_help_,
        display_version_, num_threads_, verbosity_,debug_level_, remainder_args_,out_file_,
        log_file_,arg_file_,cnf_file_,pid_file_,webroot_dir_, listeners_);

    return res;
}


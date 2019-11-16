#include <string>
#include <vector>
#include <easyprospect-config/easyprospect-config-v8.h>

using namespace easyprospect::service::config;

boost::program_options::options_description
easyprospect_config_v8_shell::get_options(
    easyprospect_config_v8_shell& config)
{
    auto desc = easyprospect_config_cmd::get_options(config);

    desc.add_options()("source-files",
        boost::program_options::value< std::vector<std::string> >(),
        "Javascript source file");

    return desc;
}

void
easyprospect_config_v8_shell::parse_options(
    easyprospect_config_v8_core_builder& builder,
    boost::program_options::variables_map vm,
    boost::program_options::options_description desc) const
{
    if (vm.count("source-files"))
    {
        auto sFiles = vm["source-files"].as<std::vector<std::string>>();
        if (sFiles.empty())
        {
            throw std::logic_error("Missing Javascript source file");
        }

        builder.set_source_files(sFiles);
    }

    easyprospect_config_cmd::parse_options(builder,vm,desc);
}

std::string 
easyprospect_config_v8_shell::get_description() const
{
    std::string res = "EasyProspect V8 Shell Configuration.";

    return res;
}

const easyprospect_config_v8_core 
easyprospect_config_v8_core_builder::to_config()
{
    easyprospect_config_v8_core_builder builder;
     
    easyprospect_config_v8_core res(easyprospect_config_core::make_shared_enabler{ 0 }, display_help_,
        display_version_, verbosity_,debug_level_, remainder_args_,out_file_,
        log_file_,arg_file_,cnf_file_,pid_file_,source_files_);

    return res;
}

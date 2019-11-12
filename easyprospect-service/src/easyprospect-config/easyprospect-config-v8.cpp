#include <string>
#include <vector>
#include <easyprospect-config/easyprospect-config-v8.h>

using namespace easyprospect::service::config;

boost::program_options::options_description 
easyprospect::service::config::easyprospect_config_v8_shell::add_options(boost::program_options::options_description desc) const
{
    desc.add_options()("source-file", 
        boost::program_options::value< std::vector<std::string> >(), 
        "Javascript source file");

    return desc;
}

void 
easyprospect::service::config::easyprospect_config_v8_shell::validate_options(boost::program_options::variables_map vm) const
{
    if (vm.count("source-file"))
    {
        std::string sFile = vm["source-file"].as<std::string>();
        if (sFile.empty())
        {
            throw new std::logic_error("Missing Javascript source file");
        }
    }
}

std::string 
easyprospect::service::config::easyprospect_config_v8_shell::get_description() const
{
    std::string res = "EasyProspect V8 Shell Configuration.";

    return res;
}

const easyprospect_config_v8_core 
easyprospect::service::config::easyprospect_config_v8_core_builder::to_config()
{
    easyprospect_config_v8_core_builder builder;
     
    easyprospect_config_v8_core res(easyprospect_config_core::make_shared_enabler{ 0 }, display_help_,
        display_version_, verbosity_,debug_level_, remainder_args_,out_file_,
        log_file_,arg_file_,cnf_file_,pid_file_,source_files_);

    return res;
}

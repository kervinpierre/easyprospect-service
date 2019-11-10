#include <string>
#include <vector>
#include <easyprospect-config-v8.h>

using namespace easyprospect::service::config;

boost::program_options::options_description 
easyprospect::service::config::EasyProspectConfigV8Shell::AddOptions(boost::program_options::options_description desc)
{
    desc.add_options()("source-file", 
        boost::program_options::value< std::vector<std::string> >(), 
        "Javascript source file");

    return desc;
}

void 
easyprospect::service::config::EasyProspectConfigV8Shell::ValidateOptions(boost::program_options::variables_map vm)
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
easyprospect::service::config::EasyProspectConfigV8Shell::GetDescription()
{
    std::string res = "EasyProspect V8 Shell Configuration.";

    return res;
}

const EasyProspectConfigV8Core 
easyprospect::service::config::EasyProspectConfigV8CoreBuilder::toConfig()
{
    EasyProspectConfigV8CoreBuilder builder;
     
    EasyProspectConfigV8Core res(EasyProspectConfigCore::make_shared_enabler{ 0 }, displayHelp,
        displayVersion, verbosity,debugLevel, remainderArgs,outFile,
        logFile,argFile,cnfFile,pidFile,sourceFiles);

    return res;
}

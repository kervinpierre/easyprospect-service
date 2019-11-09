#include <easyprospect-config.h>
#include <easyprospect-config-v8.h>

#include <boost/convert.hpp>
#include <boost/convert/stream.hpp>

#include <boost/algorithm/string/predicate.hpp>

//#include <boost/type_index.hpp>

using namespace easyprospect::service::config;

EasyProspectConfigCmd
EasyProspectConfigCmd::GetConfig(EpConfigType type)
{
    //auto currName = boost::typeindex::type_id<T>().pretty_name();

    EasyProspectConfigCmd config;

    switch (type)
    {
    case EpConfigType::EPV8:
        config = EasyProspectConfigV8Shell();
        break;

    default:
        break;
    }

    return config;
}

boost::program_options::options_description 
EasyProspectConfigCmd::GetOptions(EasyProspectConfigCmd config)
{
    boost::program_options::options_description desc(config.GetDescription());
    
    // https://stackoverflow.com/questions/32822076/boost-program-options-forward-parameters-after-to-another-program
    
    // Global options
    desc.add_options()("help", "produce help message");

    // Component options
    config.AddOptions(desc);

    return desc;
}

const EasyProspectConfigCore
easyprospect::service::config::EasyProspectConfigCmd::ParseOptions(EasyProspectConfigCmd config,
    boost::program_options::options_description desc, int argc , char* argv[] )
{
    boost::program_options::variables_map vm;

    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    return vm;
}

void 
easyprospect::service::config::EasyProspectConfigCmd::ValidateOptions(EasyProspectConfigCmd config,
    boost::program_options::variables_map vm)
{
    config.ValidateOptions(vm);
}

const EasyProspectConfigCore 
easyprospect::service::config::EasyProspectConfigCoreBuilder::toConfigCore()
{
    EasyProspectConfigCore res(EasyProspectConfigCore::make_shared_enabler{ 0 }, displayHelp, displayVersion,
        verbosity, debugLevel, remainderArgs, outFile, logFile, argFile, cnfFile,
        pidFile);

    return res;
}

void
easyprospect::service::config::EasyProspectConfigCoreBuilder::SetVerbosity(std::string verbosity)
{
    auto v = EasyProspectConfigCore::verbosityFrom(verbosity);

    SetVerbosity(v);
}

void 
easyprospect::service::config::EasyProspectConfigCoreBuilder::SetDebugLevel(std::string debugLevel)
{
    auto d = EasyProspectConfigCore::debugLevelFrom(debugLevel);

    SetDebugLevel(d);
}

void 
easyprospect::service::config::EasyProspectConfigCoreBuilder::SetLogFile(std::string logFile)
{
    auto l = boost::filesystem::path(logFile);
    SetLogFile(l);
}

void 
easyprospect::service::config::EasyProspectConfigCoreBuilder::SetOutFile(std::string outFile)
{
    auto o = boost::filesystem::path(outFile);

    SetOutFile(o);
}

void 
easyprospect::service::config::EasyProspectConfigCoreBuilder::SetArgFile(std::string argFile)
{
    auto a = boost::filesystem::path(argFile);

    SetArgFile(a);
}

void 
easyprospect::service::config::EasyProspectConfigCoreBuilder::SetCnfFile(std::string cnfFile)
{
    auto c = boost::filesystem::path(cnfFile);

    SetCnfFile(c);
}

void 
easyprospect::service::config::EasyProspectConfigCoreBuilder::SetPidFile(std::string pidFile)
{
    auto p = boost::filesystem::path(pidFile);

    SetPidFile(p);
}

void 
easyprospect::service::config::EasyProspectConfigCoreBuilder::SetDisplayHelp(std::string displayHelp)
{
    auto d = boost::convert<bool>(displayHelp, boost::cnv::cstream());
    if (d.has_value())
    {
        SetDisplayHelp(d.get());
    }
    else
    {
        throw new std::logic_error("Invalid display help option");
    }
}

void 
easyprospect::service::config::EasyProspectConfigCoreBuilder::SetDisplayVersion(std::string displayVersion)
{
    auto v = boost::convert<bool>(displayVersion, boost::cnv::cstream());
    if (v.has_value())
    {
        SetDisplayHelp(v.get());
    }
    else
    {
        throw new std::logic_error("Invalid display version option");
    }
}

const std::string 
easyprospect::service::config::EasyProspectConfigCore::toString(const EpVerbosityType v)
{
    std::string res;

    switch (v)
    {
    case EpVerbosityType::NONE:
        res = "NONE";
        break;

    case EpVerbosityType::QUIET:
        res = "QUIET";
        break;

    case EpVerbosityType::NORMAL:
        res = "NORMAL";
        break;

    case EpVerbosityType::MINIMUM:
        res = "MINIMUM";
        break;

    case EpVerbosityType::MAXIMUM:
        res = "MAXIMUM";
        break;

    default:
        res = "";
        break;
    }

    return res;
}

const std::string 
easyprospect::service::config::EasyProspectConfigCore::toString(const EpDebugLevelType d)
{
    std::string res;

    switch (d)
    {
    case EpDebugLevelType::NONE:
        res = "NONE";
        break;

    case EpDebugLevelType::ALL:
        res = "ALL";
        break;

    case EpDebugLevelType::DEBUG:
        res = "DEBUG";
        break;

    case EpDebugLevelType::INFO:
        res = "INFO";
        break;

    case EpDebugLevelType::WARN:
        res = "WARN";
        break;

    case EpDebugLevelType::ERROR:
        res = "ERROR";
        break;

    case EpDebugLevelType::FATAL:
        res = "FATAL";
        break;

    case EpDebugLevelType::OFF:
        res = "OFF";
        break;

    default:
        res = "";
        break;
    }

    return res;
}

const EpVerbosityType 
easyprospect::service::config::EasyProspectConfigCore::verbosityFrom(std::string v)
{
    EpVerbosityType res;

    if (boost::iequals(v, "NONE"))
    {
        res = EpVerbosityType::NONE;
    }
    else if (boost::iequals(v, "QUIET"))
    {
        res = EpVerbosityType::QUIET;
    }
    else if (boost::iequals(v, "MINIMUM"))
    {
        res = EpVerbosityType::MINIMUM;
    }
    else if (boost::iequals(v, "NORMAL"))
    {
        res = EpVerbosityType::NORMAL;
    }
    else if (boost::iequals(v, "MAXIMUM"))
    {
        res = EpVerbosityType::MAXIMUM;
    }
    else if (boost::iequals(v, "DEBUG"))
    {
        res = EpVerbosityType::DEBUG;
    }
    else
    {
        throw new std::logic_error("Invalid verbosity");
    }

    return res;
}

const EpDebugLevelType
easyprospect::service::config::EasyProspectConfigCore::debugLevelFrom(std::string d)
{
    EpDebugLevelType res;

    if (boost::iequals(d, "NONE"))
    {
        res = EpDebugLevelType::NONE;
    }
    else if (boost::iequals(d, "ALL"))
    {
        res = EpDebugLevelType::ALL;
    }
    else if (boost::iequals(d, "DEBUG"))
    {
        res = EpDebugLevelType::DEBUG;
    }
    else if (boost::iequals(d, "INFO"))
    {
        res = EpDebugLevelType::INFO;
    }
    else if (boost::iequals(d, "WARN"))
    {
        res = EpDebugLevelType::WARN;
    }
    else if (boost::iequals(d, "ERROR"))
    {
        res = EpDebugLevelType::ERROR;
    }
    else if (boost::iequals(d, "FATAL"))
    {
        res = EpDebugLevelType::FATAL;
    }
    else if (boost::iequals(d, "OFF"))
    {
        res = EpDebugLevelType::OFF;
    }
    else
    {
        throw new std::logic_error("Invalid debug level");
    }

    return res;
}

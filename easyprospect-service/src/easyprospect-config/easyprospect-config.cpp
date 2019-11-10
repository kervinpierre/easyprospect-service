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
    desc.add_options()("version", "Display the program version");
    desc.add_options()("verbosity", boost::program_options::value<std::string>(), "Verbosity level");
    desc.add_options()("debug-level", boost::program_options::value<std::string>(), "Debug level");
    desc.add_options()("log-file", boost::program_options::value<std::string>(), "Output logs to this file");
    desc.add_options()("output-file", boost::program_options::value<std::string>(), "Regular output goes into this file");
    desc.add_options()("arg-file", boost::program_options::value<std::string>(), "Contains regular command line arguments");
    desc.add_options()("config-file", boost::program_options::value<std::string>(), "Contains confuration options");
    desc.add_options()("pid-file", boost::program_options::value<std::string>(), "File keeps the PID of the running instance");
    desc.add_options()("--", "Options after this are sent to the called application or script");

    // Component options
    config.AddOptions(desc);

    return desc;
}

const EasyProspectConfigCore
easyprospect::service::config::EasyProspectConfigCmd::ParseOptions(EasyProspectConfigCmd config,
    boost::program_options::options_description desc, int argc , char* argv[] )
{
    spdlog::trace("ParseOptions() called\n");

    boost::program_options::variables_map vm;

    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    EasyProspectConfigCoreBuilder builder;
    
    if (vm.count("help"))
    {
        builder.SetDisplayHelp(true);

        //spdlog::trace("Compression level was set to '{}'", vm["compression"].as<int>());
    }

    if (vm.count("version"))
    {
        builder.SetDisplayVersion(true);
    }

    if (vm.count("verbosity"))
    {
        builder.SetVerbosity(vm["verbosity"].as<std::string>());
    }

    if (vm.count("debug-level"))
    {
        builder.SetVerbosity(vm["debug-level"].as<std::string>());
    }

    if (vm.count("log-file"))
    {
        builder.SetLogFile(vm["log-file"].as<std::string>());
    }

    if (vm.count("output-file"))
    {
        builder.SetOutFile(vm["output-file"].as<std::string>());
    }

    if (vm.count("arg-file"))
    {
        builder.SetArgFile(vm["arg-file"].as<std::string>());
    }

    if (vm.count("config-file"))
    {
        builder.SetCnfFile(vm["config-file"].as<std::string>());
    }

    if (vm.count("pid-file"))
    {
        builder.SetPidFile(vm["pid-file"].as<std::string>());
    }

    auto res = builder.toConfigCore();

    return res;
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
    case EpDebugLevelType::EPNONE:
        res = "NONE";
        break;

    case EpDebugLevelType::EPALL:
        res = "ALL";
        break;

    case EpDebugLevelType::EPDEBUG:
        res = "DEBUG";
        break;

    case EpDebugLevelType::EPINFO:
        res = "INFO";
        break;

    case EpDebugLevelType::EPWARN:
        res = "WARN";
        break;

    case EpDebugLevelType::EPERROR:
        res = "ERROR";
        break;

    case EpDebugLevelType::EPFATAL:
        res = "FATAL";
        break;

    case EpDebugLevelType::EPOFF:
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
        res = EpDebugLevelType::EPNONE;
    }
    else if (boost::iequals(d, "ALL"))
    {
        res = EpDebugLevelType::EPALL;
    }
    else if (boost::iequals(d, "DEBUG"))
    {
        res = EpDebugLevelType::EPDEBUG;
    }
    else if (boost::iequals(d, "INFO"))
    {
        res = EpDebugLevelType::EPINFO;
    }
    else if (boost::iequals(d, "WARN"))
    {
        res = EpDebugLevelType::EPWARN;
    }
    else if (boost::iequals(d, "ERROR"))
    {
        res = EpDebugLevelType::EPERROR;
    }
    else if (boost::iequals(d, "FATAL"))
    {
        res = EpDebugLevelType::EPFATAL;
    }
    else if (boost::iequals(d, "OFF"))
    {
        res = EpDebugLevelType::EPOFF;
    }
    else
    {
        throw new std::logic_error("Invalid debug level");
    }

    return res;
}

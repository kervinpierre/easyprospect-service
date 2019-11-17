#include <easyprospect-config/easyprospect-config.h>
#include <easyprospect-config/easyprospect-config-v8.h>

#include <boost/tokenizer.hpp>
#include <boost/convert.hpp>
#include <boost/convert/stream.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/algorithm/string.hpp>

//#include <boost/type_index.hpp>

using namespace easyprospect::service::config;

boost::program_options::options_description 
easyprospect_config_cmd::get_options(easyprospect_config_cmd& config)
{
    boost::program_options::options_description desc(config.get_description());
    
    // https://stackoverflow.com/questions/32822076/boost-program-options-forward-parameters-after-to-another-program
    
    // Global options
    desc.add_options()("help", "produce help message");
    desc.add_options()("version", "Display the program version");
    desc.add_options()("verbosity", boost::program_options::value<std::string>(), "Verbosity level");
    desc.add_options()("debug-level", boost::program_options::value<std::string>(), "Debug level");
    desc.add_options()("log-file", boost::program_options::value<std::string>(), "Output logs to this file");
    desc.add_options()("output-file", boost::program_options::value<std::string>(), "Regular output goes into this file");
    desc.add_options()("arg-file", boost::program_options::value<std::string>(), "Contains regular command line arguments");
    desc.add_options()("config-file", boost::program_options::value<std::string>(), "Contains configuration options");
    desc.add_options()("pid-file", boost::program_options::value<std::string>(), "File keeps the PID of the running instance");
    desc.add_options()("--", boost::program_options::value<std::vector<std::string>>(), "Options after this are sent to the called application or script");

    return desc;
}

boost::program_options::variables_map
easyprospect_config_cmd::get_map(
    boost::program_options::options_description desc,
    int argc, char* argv[])
{
    boost::program_options::positional_options_description p;
    p.add("--", -1);

    boost::program_options::variables_map vm;

    boost::program_options::parsed_options parsed
        = boost::program_options::command_line_parser(argc, argv)
        .options(desc).positional(p).run();

    boost::program_options::store(parsed, vm);

    if (vm.count("arg-file"))
    {
        std::string argFile
                    = vm["arg-file"].as<std::string>();

        std::ifstream ifs(argFile);
        if (!ifs)
        {
            std::string errMsg
                = fmt::format("Could not open the response file '{}'",
                argFile);

            spdlog::trace(errMsg);

            throw std::logic_error(errMsg);
        }

        // Read the whole file into a string
        std::stringstream ss;
        ss << ifs.rdbuf();
        // Split the file content
        boost::char_separator<char> sep(" \n\r");
        std::string sstr = ss.str();
        boost::tokenizer<boost::char_separator<char> > tok(sstr, sep);
        std::vector<std::string> args;
        copy(tok.begin(), tok.end(), back_inserter(args));
        // Parse the file and store the options
        boost::program_options::store(
            boost::program_options::command_line_parser(args)
                .options(desc).run(), vm);
    }

    boost::program_options::notify(vm);

    return vm;
}

void
easyprospect_config_cmd::parse_options(
    easyprospect_config_core_builder& builder,
    boost::program_options::variables_map vm,
    boost::program_options::options_description desc) const
{
    spdlog::trace("parse_options() called\n");

    if (vm.count("help"))
    {
        builder.set_display_help(true);

        //spdlog::trace("Compression level was set to '{}'", vm["compression"].as<int>());
    }

    if (vm.count("version"))
    {
        builder.set_display_version(true);
    }

    if (vm.count("verbosity"))
    {
        builder.set_verbosity(vm["verbosity"].as<std::string>());
    }

    if (vm.count("debug-level"))
    {
        builder.set_debug_level(vm["debug-level"].as<std::string>());
    }

    if (vm.count("log-file"))
    {
        builder.set_log_file(vm["log-file"].as<std::string>());
    }

    if (vm.count("output-file"))
    {
        builder.set_out_file(vm["output-file"].as<std::string>());
    }

    if (vm.count("arg-file"))
    {
        builder.set_arg_file(vm["arg-file"].as<std::string>());
    }

    if (vm.count("config-file"))
    {
        builder.set_cnf_file(vm["config-file"].as<std::string>());
    }

    if (vm.count("pid-file"))
    {
        builder.set_pid_file(vm["pid-file"].as<std::string>());
    }

    if (vm.count("--"))
    {
        builder.set_remainder_args(
            vm["--"].as<std::vector<std::string>>());
    }
}

std::unique_ptr<easyprospect_config_core>
easyprospect_config_core_builder::to_config_core()
{
    auto res = std::make_unique<easyprospect_config_core>(easyprospect_config_core::make_shared_enabler{ 0 }, display_help_, display_version_,
        verbosity_, debug_level_, remainder_args_, out_file_, log_file_, arg_file_, cnf_file_,
        pid_file_);

    return res;
}

void
easyprospect_config_core_builder::set_verbosity(std::string verbosity)
{
    auto v = easyprospect_config_core::verbosity_from(verbosity);

    set_verbosity(v);
}

void 
easyprospect_config_core_builder::set_debug_level(std::string debug_level)
{
    auto d = easyprospect_config_core::debug_level_from(debug_level);

    set_debug_level(d);
}

void 
easyprospect_config_core_builder::set_log_file(std::string log_file)
{
    auto l = boost::filesystem::path(log_file);
    set_log_file(l);
}

void 
easyprospect::service::config::easyprospect_config_core_builder::set_out_file(std::string out_file)
{
    auto o = boost::filesystem::path(out_file);

    set_out_file(o);
}

void 
easyprospect::service::config::easyprospect_config_core_builder::set_arg_file(std::string argFile)
{
    auto a = boost::filesystem::path(argFile);

    set_arg_file(a);
}

void 
easyprospect::service::config::easyprospect_config_core_builder::set_cnf_file(std::string cnf_file)
{
    auto c = boost::filesystem::path(cnf_file);

    set_cnf_file(c);
}

void 
easyprospect::service::config::easyprospect_config_core_builder::set_pid_file(std::string pid_file)
{
    auto p = boost::filesystem::path(pid_file);

    set_pid_file(p);
}

void 
easyprospect::service::config::easyprospect_config_core_builder::set_display_help(std::string display_help)
{
    auto d = boost::convert<bool>(display_help, boost::cnv::cstream());
    if (d.has_value())
    {
        set_display_help(d.get());
    }
    else
    {
        throw new std::logic_error("Invalid display help option");
    }
}

void 
easyprospect::service::config::easyprospect_config_core_builder::set_display_version(std::string display_version)
{
    auto v = boost::convert<bool>(display_version, boost::cnv::cstream());
    if (v.has_value())
    {
        set_display_help(v.get());
    }
    else
    {
        throw new std::logic_error("Invalid display version option");
    }
}

void easyprospect_config_core_builder::set_remainder_args(std::vector<std::string> remainder_args)
{
    this->remainder_args_ = remainder_args;
}

void easyprospect_config_core_builder::read_from_file(std::string filePath)
{
    boost::filesystem::path fp(filePath);

    if ( !boost::filesystem::exists(fp) )
    {
        // TODO: Throw exception?
        spdlog::error("Config file {} does not exist", filePath);

        return;
    }

    boost::property_tree::ptree root;
    boost::property_tree::read_json(filePath,root);

    auto dl = root.get<std::string>("debug_level");
    boost::trim(dl);

    if( !dl.empty() )
    {
        set_debug_level(dl);
    }
}

const std::string 
easyprospect_config_core::to_string(const ep_verbosity_type v)
{
    std::string res;

    switch (v)
    {
    case ep_verbosity_type::none:
        res = "none";
        break;

    case ep_verbosity_type::quiet:
        res = "quiet";
        break;

    case ep_verbosity_type::normal:
        res = "normal";
        break;

    case ep_verbosity_type::minimum:
        res = "minimum";
        break;

    case ep_verbosity_type::maximum:
        res = "maximum";
        break;

    default:
        res = "";
        break;
    }

    return res;
}

const std::string 
easyprospect::service::config::easyprospect_config_core::to_string(const ep_debug_level_type d)
{
    std::string res;

    switch (d)
    {
    case ep_debug_level_type::ep_none:
        res = "ep_none";
        break;

    case ep_debug_level_type::ep_all:
        res = "ep_all";
        break;

    case ep_debug_level_type::ep_debug:
        res = "ep_debug";
        break;

    case ep_debug_level_type::ep_info:
        res = "ep_info";
        break;

    case ep_debug_level_type::ep_warn:
        res = "ep_warn";
        break;

    case ep_debug_level_type::ep_error:
        res = "ep_error";
        break;

    case ep_debug_level_type::ep_fatal:
        res = "ep_fatal";
        break;

    case ep_debug_level_type::ep_off:
        res = "ep_off";
        break;

    default:
        res = "";
        break;
    }

    return res;
}

const ep_verbosity_type 
easyprospect::service::config::easyprospect_config_core::verbosity_from(std::string v)
{
    ep_verbosity_type res;

    if (boost::iequals(v, "none"))
    {
        res = ep_verbosity_type::none;
    }
    else if (boost::iequals(v, "quiet"))
    {
        res = ep_verbosity_type::quiet;
    }
    else if (boost::iequals(v, "minimum"))
    {
        res = ep_verbosity_type::minimum;
    }
    else if (boost::iequals(v, "normal"))
    {
        res = ep_verbosity_type::normal;
    }
    else if (boost::iequals(v, "maximum"))
    {
        res = ep_verbosity_type::maximum;
    }
    else if (boost::iequals(v, "debug"))
    {
        res = ep_verbosity_type::debug;
    }
    else
    {
        throw new std::logic_error("Invalid verbosity");
    }

    return res;
}

const ep_debug_level_type
easyprospect::service::config::easyprospect_config_core::debug_level_from(std::string d)
{
    ep_debug_level_type res;

    if (boost::iequals(d, "ep_none"))
    {
        res = ep_debug_level_type::ep_none;
    }
    else if (boost::iequals(d, "ep_all"))
    {
        res = ep_debug_level_type::ep_all;
    }
    else if (boost::iequals(d, "ep_debug"))
    {
        res = ep_debug_level_type::ep_debug;
    }
    else if (boost::iequals(d, "ep_info"))
    {
        res = ep_debug_level_type::ep_info;
    }
    else if (boost::iequals(d, "ep_warn"))
    {
        res = ep_debug_level_type::ep_warn;
    }
    else if (boost::iequals(d, "ep_error"))
    {
        res = ep_debug_level_type::ep_error;
    }
    else if (boost::iequals(d, "ep_fatal"))
    {
        res = ep_debug_level_type::ep_fatal;
    }
    else if (boost::iequals(d, "ep_off"))
    {
        res = ep_debug_level_type::ep_off;
    }
    else
    {
        throw new std::logic_error("Invalid debug level");
    }

    return res;
}

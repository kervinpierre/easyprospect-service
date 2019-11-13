#pragma once

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace easyprospect
{
    namespace service
    {
        namespace config
        {
            enum class ep_verbosity_type : int
            {
                none    = 0,
                quiet   = 1,
                minimum = 2,
                normal  = 3,
                debug   = 4,
                maximum = 4,
            };

            enum class ep_debug_level_type : int
            {
                ep_none = 0,
                ep_off = 1,
                ep_fatal = 2,
                ep_error = 3,
                ep_warn   = 4,
                ep_info = 5,
                ep_debug = 6,
                ep_all = 7
            };

            enum class ep_config_type : int
            {
                none = 0,
                ep_v8 = 1
            };

            class easyprospect_config_core_builder;

            /************************************************************************/
            /* Main service config class                                                                     */
            /************************************************************************/
            class easyprospect_config_core
            {
            private:
                const bool                display_help_;
                const bool                display_version_;
                const ep_verbosity_type   verbosity_;
                const ep_debug_level_type debug_level_;
                const boost::optional<std::vector<std::string>>             remainder_args_;
                const boost::optional<boost::filesystem::path> out_file_;
                const boost::optional<boost::filesystem::path> log_file_;
                const boost::optional<boost::filesystem::path> arg_file_;
                const boost::optional<boost::filesystem::path> cnf_file_;
                const boost::optional<boost::filesystem::path> pid_file_;

                friend class easyprospect_config_core_builder;

            protected:
                struct make_shared_enabler
                {
                    explicit make_shared_enabler(int) {};
                };

            public:
                easyprospect_config_core(const make_shared_enabler &,
                    bool dh, bool dv, ep_verbosity_type verb, ep_debug_level_type db,
                    boost::optional<std::vector<std::string>> rem_args,
                    boost::optional <boost::filesystem::path> of,
                    boost::optional <boost::filesystem::path> lf, 
                    boost::optional <boost::filesystem::path> af,
                    boost::optional <boost::filesystem::path> cf, 
                    boost::optional <boost::filesystem::path> pf):
                    display_help_(dh), display_version_(dv), verbosity_(verb),
                    debug_level_(db), remainder_args_(rem_args),
                    out_file_(of), log_file_(lf), arg_file_(af), cnf_file_(cf),
                    pid_file_(pf){};

                /**********************************************************************************************//**
                 * @fn  template <typename C, typename... T> static ::std::shared_ptr<C> easyprospect_config_core::create(T&&... args)
                 *
                 * @brief   Static constructor.  https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const
                 *
                 * @author  Kervin
                 * @date    2019-11-09
                 *
                 * @tparam  C   Type of the c.
                 * @tparam  T   Generic type parameter.
                 * @param   args    Variable arguments providing [in,out] The arguments.
                 **************************************************************************************************/

                template <typename C, typename... T>
                static ::std::shared_ptr<C> create(T&&... args)
                {
                    return ::std::make_shared<C>(make_shared_enabler{ 0 },
                        ::std::forward<T>(args)...);
                }


                const ep_verbosity_type  get_verbosity()  const { return verbosity_; }
                const ep_debug_level_type get_debug_level() const { return debug_level_; }
                const boost::optional<boost::filesystem::path> get_log_file() const { return log_file_; }
                const boost::optional<boost::filesystem::path> get_out_file() const { return out_file_; }
                const boost::optional<boost::filesystem::path> get_arg_file() const { return arg_file_; }
                const boost::optional<boost::filesystem::path> get_cnf_file() const { return cnf_file_; }
                const boost::optional<boost::filesystem::path> get_pid_file() const { return pid_file_; }
                const bool get_display_help() const { return display_help_; }
                const bool get_display_version() const { return display_version_; }
                const boost::optional<std::vector<std::string>> get_remainder_args() const { return remainder_args_; }

                static const std::string to_string(const ep_verbosity_type v);
                static const std::string to_string(const ep_debug_level_type d);
                static const ep_verbosity_type verbosity_from(std::string v);
                static const ep_debug_level_type debug_level_from(std::string d);
            };

            /************************************************************************/
            /* Builder class for config objects                                     */
            /************************************************************************/
            class easyprospect_config_core_builder
            {
            protected:
                bool                display_help_;
                bool                display_version_;
                ep_verbosity_type   verbosity_;
                ep_debug_level_type debug_level_;
                boost::optional<std::vector<std::string>>      remainder_args_;
                boost::optional<boost::filesystem::path> out_file_;
                boost::optional<boost::filesystem::path> log_file_;
                boost::optional<boost::filesystem::path> arg_file_;
                boost::optional<boost::filesystem::path> cnf_file_;
                boost::optional<boost::filesystem::path> pid_file_;

            public:
                easyprospect_config_core_builder()
                {
                    display_help_    = false;
                    display_version_ = false;
                    verbosity_      = ep_verbosity_type::none;
                    debug_level_     = ep_debug_level_type::ep_none;
                    remainder_args_  = boost::none;

                    out_file_ = boost::none;
                    log_file_ = boost::none;
                    arg_file_ = boost::none;
                    cnf_file_ = boost::none;
                    pid_file_ = boost::none;
                }

                const easyprospect_config_core to_config_core();

                void set_verbosity(const ep_verbosity_type verbosity) { this->verbosity_ = verbosity; }
                void set_verbosity(std::string verbosity);
                void set_debug_level(const ep_debug_level_type debug_level) { this->debug_level_ = debug_level; }
                void set_debug_level(std::string debug_level);
                void set_log_file(const boost::optional<boost::filesystem::path> log_file) { this->log_file_ = log_file; }
                void set_log_file(std::string log_file);
                void set_out_file(const boost::optional <boost::filesystem::path> out_file) { this->out_file_ = out_file; }
                void set_out_file(std::string out_file);
                void set_arg_file(const boost::optional <boost::filesystem::path> arg_file) { this->arg_file_ = arg_file; }
                void set_arg_file(std::string argFile);
                void set_cnf_file(const boost::optional <boost::filesystem::path> cnf) { this->cnf_file_ = cnf; }
                void set_cnf_file(std::string cnf_file);
                void set_pid_file(boost::optional <boost::filesystem::path> pid_file) { this->pid_file_ = pid_file; }
                void set_pid_file(std::string pid_file);
                void set_display_help(bool display_help) { this->display_help_ = display_help; }
                void set_display_help(std::string display_help);
                void set_display_version(bool display_version) { this->display_version_ = display_version; }
                void set_display_version(std::string display_version);
                void set_remainder_args(std::vector<std::string> remainder_args);
            };

            class easyprospect_config_cmd
            {
            public:
               virtual std::string 
                   get_description() const { return "";  };
                virtual boost::program_options::options_description 
                    add_options(boost::program_options::options_description desc) const
                {
                    return boost::program_options::options_description();
                };
                virtual void validate_options(boost::program_options::variables_map vm) const
                {
                    
                };

                static easyprospect_config_cmd
                    get_config(ep_config_type type);

                static boost::program_options::options_description
                    get_options(easyprospect_config_cmd config);

                static const easyprospect_config_core
                    parse_options(easyprospect_config_cmd config,
                        boost::program_options::options_description desc,
                        int argc, char* argv[]);

                static void validate_options(easyprospect_config_cmd config,
                    boost::program_options::variables_map vm);
            };
        }
    }
}
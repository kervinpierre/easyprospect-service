#pragma once

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <regex>
#include <spdlog/spdlog.h>

#include <easyprospect-config/easyprospect-defaults.h>

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
            maximum = 5,
        };

        enum class ep_debug_level_type : int
        {
            ep_none  = 0,
            ep_off   = 1,
            ep_fatal = 2,
            ep_error = 3,
            ep_warn  = 4,
            ep_info  = 5,
            ep_debug = 6,
            ep_trace = 7,
            ep_all   = 8
        };

        enum class ep_config_type : int
        {
            none  = 0,
            ep_v8 = 1
        };

        class ep_mime_type
        {
            std::string mext_;
            std::string mtype_;

          public:
            ep_mime_type(std::string e, std::string t) : mext_(e), mtype_(t)
            {
            }

            const std::string get_mext() const
            {
                return mext_;
            }

            const std::string get_mtype() const
            {
                return mtype_;
            }
        };

        class easyprospect_config_core_builder;

        /************************************************************************/
        /* Main service config class                                                                     */
        /************************************************************************/
        class easyprospect_config_core
        {
          private:
            const bool                                      display_help_;
            const bool                                      display_version_;
            const ep_verbosity_type                         verbosity_;
            const ep_debug_level_type                       debug_level_;
            std::vector<std::regex>                         epjs_url_path_regex_;
            std::vector<std::string>                        epjs_url_path_regex_str_;
            std::vector<ep_mime_type>                       mime_types_;
            const boost::optional<std::vector<std::string>> remainder_args_;
            const boost::optional<boost::filesystem::path>  out_file_;
            const boost::optional<boost::filesystem::path>  log_file_;
            const boost::optional<boost::filesystem::path>  arg_file_;
            const boost::optional<boost::filesystem::path>  cnf_file_;
            const boost::optional<boost::filesystem::path>  pid_file_;

            const boost::optional<boost::filesystem::path> pid_dir_path_;

            friend class easyprospect_config_core_builder;

          protected:
            struct make_shared_enabler
            {
                explicit make_shared_enabler(int){};
            };

          public:
            easyprospect_config_core(
                const make_shared_enabler&,
                bool                                      dh,
                bool                                      dv,
                ep_verbosity_type                         verb,
                ep_debug_level_type                       db,
                boost::optional<std::vector<std::string>> rem_args,
                std::vector<std::regex>                   epjs_upr,
                std::vector<std::string>                   epjs_upr_str,
                std::vector<ep_mime_type>                 mtypes,
                boost::optional<boost::filesystem::path>  of,
                boost::optional<boost::filesystem::path>  lf,
                boost::optional<boost::filesystem::path>  af,
                boost::optional<boost::filesystem::path>  cf,
                boost::optional<boost::filesystem::path>  pf,
                boost::optional<boost::filesystem::path>  pd) :
                display_help_(dh),
                display_version_(dv), verbosity_(verb), debug_level_(db), remainder_args_(rem_args),
                epjs_url_path_regex_(epjs_upr), epjs_url_path_regex_str_(epjs_upr_str), mime_types_(mtypes), out_file_(of),
                log_file_(lf), arg_file_(af),
                cnf_file_(cf), pid_file_(pf), pid_dir_path_(pd){};

            easyprospect_config_core(
                const make_shared_enabler&,
                bool                                      dh,
                bool                                      dv,
                ep_verbosity_type                         verb,
                ep_debug_level_type                       db,
                boost::optional<std::vector<std::string>> rem_args,
                boost::optional<boost::filesystem::path>  of,
                boost::optional<boost::filesystem::path>  lf,
                boost::optional<boost::filesystem::path>  af,
                boost::optional<boost::filesystem::path>  cf,
                boost::optional<boost::filesystem::path>  pf,
                boost::optional<boost::filesystem::path>  pd) :
                display_help_(dh),
                display_version_(dv), verbosity_(verb), debug_level_(db), remainder_args_(rem_args),
                epjs_url_path_regex_({}), mime_types_({}), out_file_(of), log_file_(lf), arg_file_(af), cnf_file_(cf),
                pid_file_(pf), pid_dir_path_(pd){};
            /**********************************************************************************************/

            /**
             * @fn  template <typename C, typename... T> static ::std::shared_ptr<C>
             *easyprospect_config_core::create(T&&... args)
             *
             * @brief   Static constructor.
             *https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const
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
                return ::std::make_shared<C>(make_shared_enabler{0}, ::std::forward<T>(args)...);
            }

            const ep_verbosity_type get_verbosity() const
            {
                return verbosity_;
            }
            const ep_debug_level_type get_debug_level() const
            {
                return debug_level_;
            }
            const boost::optional<boost::filesystem::path> get_log_file() const
            {
                return log_file_;
            }
            const boost::optional<boost::filesystem::path> get_out_file() const
            {
                return out_file_;
            }
            const boost::optional<boost::filesystem::path> get_arg_file() const
            {
                return arg_file_;
            }
            const boost::optional<boost::filesystem::path> get_cnf_file() const
            {
                return cnf_file_;
            }
            const boost::optional<boost::filesystem::path> get_pid_file() const
            {
                return pid_file_;
            }
            const boost::optional<boost::filesystem::path> get_pid_dir() const
            {
                return pid_dir_path_;
            }
            const bool get_display_help() const
            {
                return display_help_;
            }
            const bool get_display_version() const
            {
                return display_version_;
            }
            const boost::optional<std::vector<std::string>> get_remainder_args() const
            {
                return remainder_args_;
            }

            std::vector<std::regex> get_epjs_url_path_regex() const
            {
                return epjs_url_path_regex_;
            }

            std::vector<std::string> get_epjs_url_path_regex_str() const
            {
                return epjs_url_path_regex_str_;
            }

            std::vector<ep_mime_type> get_mime_types() const
            {
                return mime_types_;
            }

            static const std::string         to_string(const ep_verbosity_type v);
            static const std::string         to_string(const ep_debug_level_type d);
            static const ep_verbosity_type   verbosity_from(std::string v);
            static const ep_debug_level_type debug_level_from(std::string d);

            std::string str();
        };

        /************************************************************************/
        /* Builder class for config objects                                     */
        /************************************************************************/
        class easyprospect_config_core_builder
        {
          protected:
            bool                                      display_help_;
            bool                                      display_version_;
            ep_verbosity_type                         verbosity_;
            ep_debug_level_type                       debug_level_;
            std::vector<std::regex>                   epjs_url_path_regex_;
            std::vector<std::string>                   epjs_url_path_regex_str_;
            std::vector<ep_mime_type>                 mime_types_;
            boost::optional<std::vector<std::string>> remainder_args_;
            boost::optional<boost::filesystem::path>  out_file_;
            boost::optional<boost::filesystem::path>  log_file_;
            boost::optional<boost::filesystem::path>  arg_file_;
            boost::optional<boost::filesystem::path>  cnf_file_;
            boost::optional<boost::filesystem::path>  pid_file_;
            boost::optional<boost::filesystem::path>  pid_dir_path_;

            std::string help_str_;

          public:
            easyprospect_config_core_builder()
            {
                display_help_    = false;
                display_version_ = false;
                verbosity_       = ep_verbosity_type::none;
                debug_level_     = ep_debug_level_type::ep_none;
                remainder_args_  = boost::none;

                std::regex reg1(EPCONFIG_DEFAULT_EPJS_URL_PATH_REGEX_01);
                std::regex reg2(EPCONFIG_DEFAULT_EPJS_URL_PATH_REGEX_02);
                epjs_url_path_regex_ = std::vector<std::regex>{reg1, reg2};
                epjs_url_path_regex_str_ = {EPCONFIG_DEFAULT_EPJS_URL_PATH_REGEX_01,
                                            EPCONFIG_DEFAULT_EPJS_URL_PATH_REGEX_02};

                out_file_     = boost::none;
                log_file_     = boost::none;
                arg_file_     = boost::none;
                cnf_file_     = boost::none;
                pid_file_     = boost::none;
                pid_dir_path_ = boost::none;
            }

            std::unique_ptr<easyprospect_config_core> to_config_core();

            void set_verbosity(const ep_verbosity_type verbosity)
            {
                this->verbosity_ = verbosity;
            }
            void set_verbosity(std::string verbosity);
            void set_debug_level(const ep_debug_level_type debug_level)
            {
                this->debug_level_ = debug_level;
            }
            void set_debug_level(std::string debug_level);
            void set_log_file(const boost::optional<boost::filesystem::path> log_file)
            {
                this->log_file_ = log_file;
            }
            void set_log_file(std::string log_file);
            void set_out_file(const boost::optional<boost::filesystem::path> out_file)
            {
                this->out_file_ = out_file;
            }
            void set_out_file(std::string out_file);
            void set_arg_file(const boost::optional<boost::filesystem::path> arg_file)
            {
                this->arg_file_ = arg_file;
            }
            void set_arg_file(std::string argFile);
            void set_cnf_file(const boost::optional<boost::filesystem::path> cnf)
            {
                this->cnf_file_ = cnf;
            }
            void set_cnf_file(std::string cnf_file);
            void set_pid_file(boost::optional<boost::filesystem::path> pid_file)
            {
                this->pid_file_ = pid_file;
            }
            void set_pid_file(std::string pid_file);
            void set_pid_dir_path(boost::optional<boost::filesystem::path> pid_dir_path)
            {
                this->pid_dir_path_ = pid_dir_path;
            }
            void set_pid_dir_path(std::string pid_dir_path_);
            void set_display_help(bool display_help)
            {
                this->display_help_ = display_help;
            }
            void set_display_help(std::string display_help);
            void set_display_version(bool display_version)
            {
                this->display_version_ = display_version;
            }
            void set_display_version(std::string display_version);
            void set_remainder_args(std::vector<std::string> remainder_args);

            void set_help_str(std::string s)
            {
                help_str_ = s;
            }
            const std::string get_help_str() const
            {
                return help_str_;
            }

            void set_epjs_url_path_regex(std::vector<std::string> cses, bool clear = false)
            {
                if (clear)
                {
                    epjs_url_path_regex_.clear();
                    epjs_url_path_regex_str_.clear();
                }

                for (auto s : cses)
                {
                    try
                    {
                        epjs_url_path_regex_.emplace_back(s);
                        epjs_url_path_regex_str_.push_back(s);
                    }
                    catch (std::regex_error& ex)
                    {
                        std::string errMsg = fmt::format("Error parsing regex '{}'", ex.what());

                        spdlog::error(errMsg);

                        throw std::logic_error(errMsg);
                    }
                }
            }

            void set_mime_types(std::vector<ep_mime_type> val)
            {
                mime_types_ = val;
            }

            void read_from_file(std::string filePath);
        };

        class easyprospect_config_cmd
        {
          public:
            virtual std::string get_description() const
            {
                return "";
            };

            void parse_options(
                easyprospect_config_core_builder&           builder,
                boost::program_options::variables_map       vm,
                boost::program_options::options_description desc);

            static boost::program_options::options_description get_options(easyprospect_config_cmd& config);

            static boost::program_options::variables_map get_map(
                boost::program_options::options_description desc,
                int                                         argc,
                char*                                       argv[]);
        };
    } // namespace config
} // namespace service
} // namespace easyprospect
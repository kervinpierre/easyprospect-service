#pragma once

#include <easyprospect-config/easyprospect-config.h>
#include <string>

#include <boost/program_options.hpp>

#include <easyprospect-config/ep-listener-config.h>

#include "easyprospect-config-service.h"

namespace easyprospect
{
namespace service
{
    namespace config
    {
        class easyprospect_config_wcntl_core_builder;

        /************************************************************************/
        /* EpService core configuration object                                  */
        /************************************************************************/
        class easyprospect_config_wcntl_core final : public easyprospect_config_service_core
        {
          private:
            const bool                                     worker_exe_use_path_;
            const int                                      num_workers_;
            const std::string                              worker_args_;
            const boost::optional<boost::filesystem::path> worker_conf_;
            const boost::optional<boost::filesystem::path> worker_args_file_;
            const boost::optional<boost::filesystem::path> worker_exe_;

            friend class easyprospect_config_wcntl_core_builder;

          protected:
            easyprospect_config_wcntl_core(
                const make_shared_enabler&                mse,
                bool                                      dh,
                bool                                      dv,
                bool                                      wkup,
                int                                       nt,
                int                                       np,
                std::string                               wargs,
                ep_verbosity_type                         verb,
                ep_debug_level_type                       db,
                boost::optional<std::vector<std::string>> remArgs,
                std::vector<std::regex>                   epjs_upr,
                std::vector<std::string>                  epjs_upr_str,
                std::vector<ep_mime_type>                 mtypes,
                boost::optional<boost::filesystem::path>  of,
                boost::optional<boost::filesystem::path>  lf,
                boost::optional<boost::filesystem::path>  af,
                boost::optional<boost::filesystem::path>  cf,
                boost::optional<boost::filesystem::path>  pf,
                boost::optional<boost::filesystem::path>  pd,
                boost::optional<boost::filesystem::path>  sf,
                boost::optional<boost::filesystem::path>  lstfile,
                boost::optional<boost::filesystem::path>  lstdir,
                boost::optional<boost::filesystem::path>  wkconf,
                boost::optional<boost::filesystem::path>  wkargs,
                boost::optional<boost::filesystem::path>  wkexe,
                std::vector<easyprospect_config_service_listener_conf> ls) :
                easyprospect_config_service_core(
                    mse,
                    dh,
                    dv,
                    nt,
                    verb,
                    db,
                    remArgs,
                    epjs_upr,
                    epjs_upr_str,
                    mtypes,
                    of,
                    lf,
                    af,
                    cf,
                    pf,
                    pd, sf, lstfile, lstdir, ls),
                num_workers_(np), worker_exe_(wkexe), worker_exe_use_path_(wkup), worker_args_(wargs),
                worker_conf_(wkconf), worker_args_file_(wkargs){};

          public:
            std::string str();

            const boost::optional<boost::filesystem::path> get_worker_exe()
            {
                return worker_exe_;
            }

            const bool get_worker_exe_use_path() const
            {
                return worker_exe_use_path_;
            }

            const std::string get_worker_args() const
            {
                return worker_args_;
            }

            const int get_num_workers() const
            {
                return num_workers_;
            }

            const boost::optional<boost::filesystem::path> get_worker_args_file() const
            {
                return worker_args_file_;
            }

            const boost::optional<boost::filesystem::path> get_worker_conf() const
            {
                return worker_conf_;
            }
        };

        /************************************************************************/
        /* EpService core configuration object                                  */
        /************************************************************************/
        class easyprospect_config_wcntl_core_builder final : public easyprospect_config_service_core_builder
        {
            bool                                     worker_exe_use_path_;
            int                                      num_workers_;
            std::string                              worker_args_;
            boost::optional<boost::filesystem::path> worker_conf_;
            boost::optional<boost::filesystem::path> worker_args_file_;
            boost::optional<boost::filesystem::path> worker_exe_;

          public:
            easyprospect_config_wcntl_core_builder() :
                easyprospect_config_service_core_builder(){

                };
            void set_worker_exe_use_path(bool u)
            {
                worker_exe_use_path_ = u;
            }

            void set_worker_exe_use_path(std::string u)
            {
                worker_exe_use_path_ = boost::lexical_cast<bool>(u);
            }

            void set_worker_args(std::string u)
            {
                worker_args_ = u;
            }

            void set_num_workers(int nw)
            {
                num_workers_ = nw;
            }

            void set_num_workers(std::string nw)
            {
                int res = std::stoi(nw);

                set_num_workers(res);
            }

            void set_worker_exe(boost::optional<boost::filesystem::path> sf)
            {
                worker_exe_ = sf;
            }

            void set_worker_exe(std::string sf)
            {
                boost::optional<boost::filesystem::path> res = boost::filesystem::path(sf);

                set_worker_exe(res);
            }

            void set_worker_conf(boost::optional<boost::filesystem::path> sf)
            {
                worker_conf_ = sf;
            }

            void set_worker_conf(std::string sf)
            {
                boost::optional<boost::filesystem::path> res = boost::filesystem::path(sf);

                set_worker_conf(res);
            }

            void set_worker_args_file(boost::optional<boost::filesystem::path> sf)
            {
                worker_args_file_ = sf;
            }

            void set_worker_args_file(std::string sf)
            {
                boost::optional<boost::filesystem::path> res = boost::filesystem::path(sf);

                set_worker_args_file(res);
            }

            const easyprospect_config_wcntl_core to_config();
        };

        /************************************************************************/
        /* EpServiceShell configuration                                         */
        /************************************************************************/
        class easyprospect_config_wcntl_shell final : public easyprospect_config_service_shell
        {
          public:
            static std::string get_description();

            static boost::program_options::options_description get_options(easyprospect_config_wcntl_shell& config);

            static easyprospect_config_wcntl_core_builder init_args(int test_argc, char* test_argv[]);

            void parse_options(
                easyprospect_config_wcntl_core_builder&     builder,
                boost::program_options::variables_map       vm,
                boost::program_options::options_description desc);
        };
    } // namespace config
} // namespace service
} // namespace easyprospect
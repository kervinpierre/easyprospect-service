#pragma once

#include <easyprospect-config/easyprospect-config.h>
#include <easyprospect-config/ep-listener-config.h>

#include <easyprospect-config/easyprospect-config-service.h>

#include "ep-backend-config.h"

namespace easyprospect
{
namespace service
{
    namespace config
    {
        class easyprospect_config_server_core_builder;

        /************************************************************************/
        /* EpService core configuration object                                  */
        /************************************************************************/
        class easyprospect_config_server_core final : public easyprospect_config_service_core
        {
          private:
            const std::vector<easyprospect_config_service_backend_conf> backends_;

            friend class easyprospect_config_server_core_builder;

          protected:
            easyprospect_config_server_core(
                const make_shared_enabler& mse,
                bool                                                   dh,
                bool                                                   dv,
                int                                                    nt,
                ep_verbosity_type                                      verb,
                ep_debug_level_type                                    db,
                boost::optional<std::vector<std::string>>              remArgs,
                std::vector<std::regex>                                epjs_upr,
                std::vector<std::string>                                epjs_upr_str,
                std::vector<ep_mime_type>                              mtypes,
                boost::optional<boost::filesystem::path>               of,
                boost::optional<boost::filesystem::path>               lf,
                boost::optional<boost::filesystem::path>               af,
                boost::optional<boost::filesystem::path>               cf,
                boost::optional<boost::filesystem::path>               pf,
                boost::optional<boost::filesystem::path>               pd,
                boost::optional<boost::filesystem::path>               sf,
                boost::optional<boost::filesystem::path>               lstfile,
                boost::optional<boost::filesystem::path>               lstdir,
                std::vector<easyprospect_config_service_listener_conf> ls,
                std::vector<easyprospect_config_service_backend_conf> be) :
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
                    pd,
                    sf,
                    lstfile,
                    lstdir,
                    ls),
                 backends_(be){};

          public:

            const boost::optional<std::vector<easyprospect_config_service_backend_conf>> get_backends() const
            {
                return backends_;
            }

            std::string str();
        };

        /************************************************************************/
        /* EpService core configuration object                                  */
        /************************************************************************/
        class easyprospect_config_server_core_builder final : public easyprospect_config_service_core_builder
        {
            std::vector<easyprospect_config_service_backend_conf>  backends_;

          public:
            easyprospect_config_server_core_builder() : easyprospect_config_service_core_builder()
            {

            };

            void set_backends(std::string l)
            {
                auto c = easyprospect_config_service_backend_conf::parse(l);
                backends_.push_back(c);
            }

            void set_backends(std::vector<std::string> l)
            {
                auto la = easyprospect_config_service_backend_conf::parse(l);
                backends_.insert(backends_.end(), la.begin(), la.end());
            }

            const easyprospect_config_server_core to_config();
        };

        /************************************************************************/
        /* EpServiceShell configuration                                         */
        /************************************************************************/
        class easyprospect_config_server_shell final : public easyprospect_config_service_shell
        {
          public:
            static std::string get_description();

            static boost::program_options::options_description get_options(easyprospect_config_server_shell& config);

            static easyprospect_config_server_core_builder init_args(int test_argc, char* test_argv[]);

            void parse_options(
                easyprospect_config_server_core_builder&   builder,
                boost::program_options::variables_map       vm,
                boost::program_options::options_description desc);
        };
    } // namespace config
} // namespace service
} // namespace easyprospect
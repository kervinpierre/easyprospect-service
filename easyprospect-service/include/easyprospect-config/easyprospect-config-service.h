#pragma once

#include <easyprospect-config/easyprospect-config.h>
#include <easyprospect-config/ep-listener-config.h>

namespace easyprospect
{
namespace service
{
    namespace config
    {
        class easyprospect_config_service_core_builder;

        /************************************************************************/
        /* EpService core configuration object                                  */
        /************************************************************************/
        class easyprospect_config_service_core : public easyprospect_config_core
        {
          private:
            const int                                                    num_threads_;
            const boost::optional<boost::filesystem::path>               webroot_dir_;
            const std::vector<easyprospect_config_service_listener_conf> listeners_;

            const boost::optional<boost::filesystem::path> listen_file_;
            const boost::optional<boost::filesystem::path> listen_dir_path_;

            friend class easyprospect_config_service_core_builder;

          protected:
            easyprospect_config_service_core(
                const make_shared_enabler&                             mse,
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
                std::vector<easyprospect_config_service_listener_conf> ls) :
                easyprospect_config_core(mse, dh, dv, verb, db, remArgs, epjs_upr, epjs_upr_str, mtypes, of, lf, af, cf, pf, pd),
                num_threads_(nt), listen_file_(lstfile), listen_dir_path_(lstdir),
                webroot_dir_(sf), listeners_(ls){};

          public:

            const boost::optional<boost::filesystem::path> get_webroot_dir() const
            {
                return webroot_dir_;
            }

            const int get_num_threads() const
            {
                return num_threads_;
            }

            const boost::optional<std::vector<easyprospect_config_service_listener_conf>> get_listeners() const
            {
                return listeners_;
            }

            std::string str();
        };

        /************************************************************************/
        /* EpService core configuration object                                  */
        /************************************************************************/
        class easyprospect_config_service_core_builder : public easyprospect_config_core_builder
        {
        protected:
            int                                                    num_threads_;
            boost::optional<boost::filesystem::path>               webroot_dir_;
            std::vector<easyprospect_config_service_listener_conf> listeners_;
            boost::optional<boost::filesystem::path>               listen_file_;
            boost::optional<boost::filesystem::path>               listen_dir_path_;

          public:
            easyprospect_config_service_core_builder() : easyprospect_config_core_builder()
            {
                webroot_dir_ = "wwwroot";
                num_threads_ = 1;
            };

            void set_webroot_dir(boost::optional<boost::filesystem::path> sf)
            {
                webroot_dir_ = sf;
            }

            void set_webroot_dir(std::string sf)
            {
                boost::optional<boost::filesystem::path> res = boost::filesystem::path(sf);

                set_webroot_dir(res);
            }

            void set_num_threads(int nt)
            {
                num_threads_ = nt;
            }

            void set_num_threads(std::string nt)
            {
                int res = std::stoi(nt);

                set_num_threads(res);
            }

            void set_listeners(std::vector<easyprospect_config_service_listener_conf> l)
            {
                listeners_ = l;
            }

            void set_listeners(std::string l)
            {
                auto c = easyprospect_config_service_listener_conf::parse(l);
                listeners_.push_back(c);
            }

            void set_listeners(std::vector<std::string> l)
            {
                auto la = easyprospect_config_service_listener_conf::parse(l);
                listeners_.insert(listeners_.end(), la.begin(), la.end());
            }

            void set_listen_file(boost::optional<boost::filesystem::path> sf)
            {
                listen_file_ = sf;
            }

            void set_listen_file(std::string sf)
            {
                boost::optional<boost::filesystem::path> res = boost::filesystem::path(sf);

                set_listen_file(res);
            }

            void set_listen_dir(boost::optional<boost::filesystem::path> sf)
            {
                listen_dir_path_ = sf;
            }

            void set_listen_dir(std::string sf)
            {
                boost::optional<boost::filesystem::path> res = boost::filesystem::path(sf);

                set_listen_dir(res);
            }

            const easyprospect_config_service_core to_config();
        };

        /************************************************************************/
        /* EpServiceShell configuration                                         */
        /************************************************************************/
        class easyprospect_config_service_shell : public easyprospect_config_cmd
        {
          public:
            static std::string get_description();

            static boost::program_options::options_description get_options(easyprospect_config_service_shell& config);

            static easyprospect_config_service_core_builder init_args(int test_argc, char* test_argv[]);

            void parse_options(
                easyprospect_config_service_core_builder&   builder,
                boost::program_options::variables_map       vm,
                boost::program_options::options_description desc);
        };
    } // namespace config
} // namespace service
} // namespace easyprospect
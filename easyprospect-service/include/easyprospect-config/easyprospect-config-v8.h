#pragma once

#include <string>
#include <easyprospect-config/easyprospect-config.h>

#include <boost/program_options.hpp>

namespace easyprospect
{
    namespace service
    {
        namespace config
        {
            class easyprospect_config_v8_core_builder;

            /************************************************************************/
            /* EpV8 core configuration object                                  */
            /************************************************************************/
            class easyprospect_config_v8_core final : public easyprospect_config_core
            {
            private:
                const boost::optional<std::vector<boost::filesystem::path>>
                    source_files_;

                friend class easyprospect_config_v8_core_builder;

            protected:
                easyprospect_config_v8_core(const make_shared_enabler &mse, bool dh, bool dv, ep_verbosity_type verb, ep_debug_level_type db,
                    boost::optional<std::vector<std::string>> remArgs,
                    boost::optional<boost::filesystem::path> of,
                    boost::optional<boost::filesystem::path> lf,
                    boost::optional<boost::filesystem::path> af,
                    boost::optional<boost::filesystem::path> cf,
                    boost::optional<boost::filesystem::path> pf,
                    boost::optional<std::vector<boost::filesystem::path>> sf) 
                        :easyprospect_config_core( mse, dh, dv, verb, db, remArgs, of, 
                            lf, af, cf, pf), source_files_(sf)
                { };

            public:
                const boost::optional<std::vector<boost::filesystem::path>> 
                    get_source_files() const { return source_files_; }
            };

            /************************************************************************/
            /* EpV8 core configuration object                                  */
            /************************************************************************/
            class easyprospect_config_v8_core_builder final : public easyprospect_config_core_builder
            {
                boost::optional<std::vector<boost::filesystem::path>>
                    source_files_;

            public:
                easyprospect_config_v8_core_builder()
                    : easyprospect_config_core_builder()
                {
                    source_files_ = boost::none;
                };

                void set_source_files(boost::optional<std::vector<boost::filesystem::path>> sf)
                {
                    source_files_ = sf;
                }

                void set_source_files(std::vector<std::string> sf)
                {
                    boost::optional<std::vector<boost::filesystem::path>>
                        res = std::vector<boost::filesystem::path>();

                    for(auto i : sf)
                    {
                        res->push_back(boost::filesystem::path(i));
                    }

                    set_source_files(res);
                }

                const easyprospect_config_v8_core to_config();
            };

            /************************************************************************/
            /* EpV8Shell configuration                                         */
            /************************************************************************/
            class easyprospect_config_v8_shell final : public easyprospect_config_cmd
            {
            public:
                std::string get_description() const override;
                
                static boost::program_options::options_description
                    get_options(easyprospect_config_v8_shell& config);

                static easyprospect_config_v8_core_builder
                    init_args(int test_argc, char* test_argv[]);
                   
                void  parse_options(
                    easyprospect_config_v8_core_builder& builder,
                    boost::program_options::variables_map vm,
                    boost::program_options::options_description desc);
            };
        }
    }
}
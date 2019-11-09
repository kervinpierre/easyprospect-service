#pragma once

#include <string>
#include <easyprospect-config.h>

#include <boost/program_options.hpp>

namespace easyprospect
{
    namespace service
    {
        namespace config
        {
            class EasyProspectConfigV8CoreBuilder;

            /************************************************************************/
            /* EpV8 core configuration object                                  */
            /************************************************************************/
            class EasyProspectConfigV8Core final : public EasyProspectConfigCore
            {
            private:
                const boost::optional<std::vector<boost::filesystem::path>>
                    sourceFiles;

                friend class EasyProspectConfigV8CoreBuilder;

            protected:
                EasyProspectConfigV8Core(const make_shared_enabler &mse, bool dh, bool dv, EpVerbosityType verb, EpDebugLevelType db,
                    boost::optional<std::string> remArgs, 
                    boost::optional<boost::filesystem::path> of,
                    boost::optional<boost::filesystem::path> lf,
                    boost::optional<boost::filesystem::path> af,
                    boost::optional<boost::filesystem::path> cf,
                    boost::optional<boost::filesystem::path> pf,
                    boost::optional<std::vector<boost::filesystem::path>> sf) 
                        :EasyProspectConfigCore( mse, dh, dv, verb, db, remArgs, of, 
                            lf, af, cf, pf), sourceFiles(sf)
                { };

            public:
                const boost::optional<std::vector<boost::filesystem::path>> 
                    GetSourceFiles() const { return sourceFiles; }
            };

            /************************************************************************/
            /* EpV8 core configuration object                                  */
            /************************************************************************/
            class EasyProspectConfigV8CoreBuilder final : public EasyProspectConfigCoreBuilder
            {
                boost::optional<std::vector<boost::filesystem::path>>
                    sourceFiles;

            public:
                EasyProspectConfigV8CoreBuilder()
                    : EasyProspectConfigCoreBuilder()
                {
                    sourceFiles = boost::none;
                };

                void setSourceFiles(boost::optional<std::vector<boost::filesystem::path>> sf)
                {
                    sourceFiles = sf;
                }

                const EasyProspectConfigV8Core toConfig();
            };

            /************************************************************************/
            /* EpV8Shell configuration                                         */
            /************************************************************************/
            class EasyProspectConfigV8Shell : public EasyProspectConfigCmd
            {
            public:
                virtual std::string GetDescription() override;
                
                virtual boost::program_options::options_description 
                    AddOptions(boost::program_options::options_description desc) override;

                virtual void ValidateOptions(boost::program_options::variables_map vm) override;
            };
        }
    }
}
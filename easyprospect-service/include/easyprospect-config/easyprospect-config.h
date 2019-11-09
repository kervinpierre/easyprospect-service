#pragma once

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

namespace easyprospect
{
    namespace service
    {
        namespace config
        {
            enum class EpVerbosityType : int
            {
                NONE    = 0,
                QUIET   = 1,
                MINIMUM = 2,
                NORMAL  = 3,
                DEBUG   = 4,
                MAXIMUM = 4,
            };

            enum class EpDebugLevelType : int
            {
                NONE = 0,
                OFF = 1,
                FATAL = 2,
                ERROR = 3,
                WARN   = 4,
                INFO = 5,
                DEBUG = 6,
                ALL = 7
            };

            enum class EpConfigType : int
            {
                NONE = 0,
                EPV8 = 1
            };

            class EasyProspectConfigCoreBuilder;

            /************************************************************************/
            /* Main service config class                                                                     */
            /************************************************************************/
            class EasyProspectConfigCore
            {
            private:
                const bool             displayHelp;
                const bool             displayVersion;
                const EpVerbosityType  verbosity;
                const EpDebugLevelType debugLevel;
                const boost::optional<std::string>             remainderArgs;
                const boost::optional<boost::filesystem::path> outFile;
                const boost::optional<boost::filesystem::path> logFile;
                const boost::optional<boost::filesystem::path> argFile;
                const boost::optional<boost::filesystem::path> cnfFile;
                const boost::optional<boost::filesystem::path> pidFile;

                friend class EasyProspectConfigCoreBuilder;

            protected:
                struct make_shared_enabler
                {
                    explicit make_shared_enabler(int) {};
                };

            public:
                EasyProspectConfigCore(const make_shared_enabler &,
                    bool dh, bool dv, EpVerbosityType verb, EpDebugLevelType db,
                    boost::optional<std::string> remArgs,
                    boost::optional <boost::filesystem::path> of,
                    boost::optional <boost::filesystem::path> lf, 
                    boost::optional <boost::filesystem::path> af,
                    boost::optional <boost::filesystem::path> cf, 
                    boost::optional <boost::filesystem::path> pf):
                    displayHelp(dh), displayVersion(dv), verbosity(verb),
                    debugLevel(db), remainderArgs(remArgs),
                    outFile(of), logFile(lf), argFile(af), cnfFile(cf),
                    pidFile(pf){};

                template <typename C, typename... T>
                static ::std::shared_ptr<C> Create(T&&... args)
                {
                    return ::std::make_shared<C>(make_shared_enabler{ 0 },
                        ::std::forward<T>(args)...);
                }

                const EpVerbosityType  GetVerbosity()  const { return verbosity; }
                const EpDebugLevelType GetDebugLevel() const { return debugLevel; }
                const boost::optional<boost::filesystem::path> GetLogFile() const { return logFile; }
                const boost::optional<boost::filesystem::path> GetOutFile() const { return outFile; }
                const boost::optional<boost::filesystem::path> GetArgFile() const { return argFile; }
                const boost::optional<boost::filesystem::path> GetCnfFile() const { return cnfFile; }
                const boost::optional<boost::filesystem::path> GetPidFile() const { return pidFile; }
                const bool GetDisplayHelp() const { return GetDisplayHelp(); }
                const bool GetDisplayVersion() const { return displayVersion; }
                const boost::optional<std::string> GetRemainderArgs() const { return remainderArgs; }

                static const std::string toString(const EpVerbosityType v);
                static const std::string toString(const EpDebugLevelType d);
                static const EpVerbosityType verbosityFrom(std::string v);
                static const EpDebugLevelType debugLevelFrom(std::string d);
            };

            /************************************************************************/
            /* Builder class for config objects                                     */
            /************************************************************************/
            class EasyProspectConfigCoreBuilder
            {
            protected:
                bool             displayHelp;
                bool             displayVersion;
                EpVerbosityType  verbosity;
                EpDebugLevelType debugLevel;
                boost::optional<std::string>      remainderArgs;
                boost::optional<boost::filesystem::path> outFile;
                boost::optional<boost::filesystem::path> logFile;
                boost::optional<boost::filesystem::path> argFile;
                boost::optional<boost::filesystem::path> cnfFile;
                boost::optional<boost::filesystem::path> pidFile;

            public:
                EasyProspectConfigCoreBuilder()
                {
                    displayHelp    = false;
                    displayVersion = false;
                    verbosity      = EpVerbosityType::NONE;
                    debugLevel     = EpDebugLevelType::NONE;
                    remainderArgs  = boost::none;

                    outFile = boost::none;
                    logFile = boost::none;
                    argFile = boost::none;
                    cnfFile = boost::none;
                    pidFile = boost::none;
                }

                const EasyProspectConfigCore toConfigCore();

                void SetVerbosity(EpVerbosityType verbosity) { this->verbosity = verbosity; }
                void SetVerbosity(std::string verbosity);
                void SetDebugLevel(EpDebugLevelType debugLevel) { this->debugLevel = debugLevel; }
                void SetDebugLevel(std::string debugLevel);
                void SetLogFile(boost::optional <boost::filesystem::path> logFile) { this->logFile = logFile; }
                void SetLogFile(std::string logFile);
                void SetOutFile(boost::optional <boost::filesystem::path> outFile) { this->outFile = outFile; }
                void SetOutFile(std::string outFile);
                void SetArgFile(boost::optional <boost::filesystem::path> argFile) { this->argFile = argFile; }
                void SetArgFile(std::string argFile);
                void SetCnfFile(boost::optional <boost::filesystem::path> cnf) { this->cnfFile = cnf; }
                void SetCnfFile(std::string cnfFile);
                void SetPidFile(boost::optional <boost::filesystem::path> pidFile) { this->pidFile = pidFile; }
                void SetPidFile(std::string pidFile);
                void SetDisplayHelp(bool displayHelp) { this->displayHelp = displayHelp; }
                void SetDisplayHelp(std::string displayHelp);
                void SetDisplayVersion(bool displayVersion) { this->displayVersion = displayVersion; }
                void SetDisplayVersion(std::string displayVersion);
                void SetRemainderArgs(std::string remainderArgs) { this->remainderArgs = remainderArgs; }
            };

            class EasyProspectConfigCmd
            {
            public:
                virtual std::string 
                    GetDescription();
                virtual boost::program_options::options_description 
                    AddOptions(boost::program_options::options_description desc);
                virtual void ValidateOptions(boost::program_options::variables_map vm);

                static EasyProspectConfigCmd
                    GetConfig(EpConfigType type);

                static boost::program_options::options_description
                    GetOptions(EasyProspectConfigCmd config);

                static const EasyProspectConfigCore
                    ParseOptions(EasyProspectConfigCmd config,
                        boost::program_options::options_description desc,
                        int argc, char* argv[]);

                static void ValidateOptions(EasyProspectConfigCmd config,
                    boost::program_options::variables_map vm);
            };
        }
    }
}
#pragma once

#include <string>
#include <easyprospect-config/easyprospect-config.h>

#include <boost/tokenizer.hpp>
#include <boost/program_options.hpp>

namespace easyprospect
{
    namespace service
    {
        namespace config
        {

            enum class listener_kind
            {
                none,
                no_tls,
                allow_tls,
                require_tls
            };

            class easyprospect_config_service_core_builder;

            class easyprospect_config_service_listener_conf final
            {
            private:
                std::string name_;
                std::string address_;
                std::string port_;
                std::string kind_;

            public:
                easyprospect_config_service_listener_conf(std::string n, std::string a, std::string p, std::string k)
                    : name_(n), address_(a), port_(p), kind_(k) {};

                easyprospect_config_service_listener_conf() {};

                std::string   get_name() const { return name_; }
                std::string   get_port() const { return port_; }
                std::string   get_address() const { return address_; }
                std::string   get_kind() const { return kind_; }

                std::string str();

                static std::string to_string(listener_kind k)
                {
                    switch(k)
                    {
                    case listener_kind::allow_tls:
                        return "allow_tls";
                    case listener_kind::no_tls:
                        return "no_tls";
                    case listener_kind::require_tls:
                        return "require_tls";
                    case listener_kind::none:
                        return "none";
                    default:
                        throw std::logic_error("unknown kind");
                    }
                }

                static listener_kind str_to_listener_kind(std::string k)
                {
                    if( k == "allow_tls")
                    {
                        return listener_kind::allow_tls;
                    }
                    else if (k == "no_tls")
                    {
                        return listener_kind::no_tls;
                    }
                    else if (k == "require_tls")
                    {
                        return listener_kind::require_tls;
                    }
                    else
                    {
                        return listener_kind::none;
                    }
                }

                static std::vector<easyprospect_config_service_listener_conf> parse(std::vector<std::string> s)
                {
                    std::vector<easyprospect_config_service_listener_conf> res;

                    for(auto i : s)
                    {
                        auto l = parse(i);
                        res.push_back(parse(i));
                    }

                    return res;
                }

                static easyprospect_config_service_listener_conf parse(std::string s)
                {
                    boost::escaped_list_separator<char> sep;
                    boost::tokenizer<boost::escaped_list_separator<char> > tok(s, sep);

                    std::vector<std::string> args(tok.begin(), tok.end());

                    if( args.empty() )
                    {
                        // TODO: KP. Log this
                        throw std::logic_error("listener config is empty.");
                    }
                    else if( args.size() < 3)
                    {
                        throw std::logic_error("listener config should be provided in the format 'name,address,port' ( without the quote ).");
                    }

                    std::string kind = "no_tls";
                    if( args.size() > 3)
                    {
                        kind = args[3];
                    }

                    easyprospect_config_service_listener_conf res(args[0], args[1], args[2], kind);

                    return res;
                }
            };

            /************************************************************************/
            /* EpService core configuration object                                  */
            /************************************************************************/
            class easyprospect_config_service_core final : public easyprospect_config_core
            {
            private:
                const int num_threads_;
                const boost::optional<boost::filesystem::path>
                    webroot_dir_;
                const std::vector<easyprospect_config_service_listener_conf>
                    listeners_;

                friend class easyprospect_config_service_core_builder;

            protected:
                easyprospect_config_service_core(const make_shared_enabler &mse, bool dh, bool dv, int nt, ep_verbosity_type verb, ep_debug_level_type db,
                    boost::optional<std::vector<std::string>> remArgs,
                    boost::optional<boost::filesystem::path> of,
                    boost::optional<boost::filesystem::path> lf,
                    boost::optional<boost::filesystem::path> af,
                    boost::optional<boost::filesystem::path> cf,
                    boost::optional<boost::filesystem::path> pf,
                    boost::optional<boost::filesystem::path> sf,
                    std::vector<easyprospect_config_service_listener_conf> ls)
                        :easyprospect_config_core( mse, dh, dv, verb, db, remArgs, of, 
                            lf, af, cf, pf), num_threads_(nt), webroot_dir_(sf), listeners_(ls)
                { };

            public:
                const boost::optional<boost::filesystem::path> 
                    get_webroot_dir() const { return webroot_dir_; }

                const int get_num_threads() const { return num_threads_; }

                const boost::optional<std::vector<easyprospect_config_service_listener_conf>>
                    get_listeners() const { return listeners_; }

                std::string str();
            };

            /************************************************************************/
            /* EpService core configuration object                                  */
            /************************************************************************/
            class easyprospect_config_service_core_builder final : public easyprospect_config_core_builder
            {
                boost::optional<boost::filesystem::path>
                    webroot_dir_;
                int num_threads_;
                std::vector<easyprospect_config_service_listener_conf>
                    listeners_;

            public:
                easyprospect_config_service_core_builder()
                    : easyprospect_config_core_builder()
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
                    boost::optional<boost::filesystem::path>
                        res = boost::filesystem::path(sf);

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
                    listeners_.insert(listeners_.end(),la.begin(), la.end());
                }

                const easyprospect_config_service_core to_config();
            };

            /************************************************************************/
            /* EpServiceShell configuration                                         */
            /************************************************************************/
            class easyprospect_config_service_shell final : public easyprospect_config_cmd
            {
            public:
                static std::string get_description();
                
                static boost::program_options::options_description
                    get_options(easyprospect_config_service_shell& config);

                static easyprospect_config_service_core_builder
                    init_args(int test_argc, char* test_argv[]);
                   
                void  parse_options(
                    easyprospect_config_service_core_builder& builder,
                    boost::program_options::variables_map vm,
                    boost::program_options::options_description desc);
            };
        }
    }
}
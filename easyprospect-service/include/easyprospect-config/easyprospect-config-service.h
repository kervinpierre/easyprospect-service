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
                std::string min_port_;
                std::string max_port_;
                std::string kind_;

            public:
                easyprospect_config_service_listener_conf(std::string n, std::string a, std::string p, std::string minp, std::string maxp, std::string k)
                    : name_(n), address_(a), port_(p), min_port_(minp), max_port_(maxp), kind_(k) {};

                easyprospect_config_service_listener_conf() {};

                std::string   get_name() const { return name_; }
                std::string   get_port() const { return port_; }
                std::string   get_min_port() const { return min_port_; }
                std::string   get_max_port() const { return max_port_; }
                std::string   get_address()  const { return address_; }
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
                        throw std::logic_error("listener config should be provided in the format 'name=<name>,address=<address>,port=<port>[,min-port=<min port>,max-port=<max port>],kind=<kind>' ( without the quote ).");
                    }

                    std::string name;
                    std::string address;
                    std::string port;
                    std::string min_port;
                    std::string max_port;
                    std::string kind;

                    for(auto a : args)
                    {
                        std::string::size_type pos = a.find('=');
                        if (pos != std::string::npos)
                        {
                            auto n = a.substr(0, pos);
                            auto v = a.substr(pos + 1, a.length());

                            if( n == "name" )
                            {
                                name = v;
                            }
                            else if(n == "address")
                            {
                                address = v;
                            }
                            else if (n == "port")
                            {
                                port = v;
                            }
                            else if (n == "min-port")
                            {
                                min_port = v;
                            }
                            else if (n == "max-port")
                            {
                                max_port = v;
                            }
                            else if (n == "kind")
                            {
                                kind = v;
                            }
                        }
                    }

                    easyprospect_config_service_listener_conf res(name, address, port, min_port, max_port, kind);

                    return res;
                }
            };

            /************************************************************************/
            /* EpService core configuration object                                  */
            /************************************************************************/
            class easyprospect_config_service_core final : public easyprospect_config_core
            {
            private:
                const bool worker_;
                const bool worker_exe_use_path_;
                const int num_threads_;
                const int num_workers_;
                const std::string worker_args_;
                const boost::optional<boost::filesystem::path>
                    worker_conf_;
                const boost::optional<boost::filesystem::path>
                    worker_exe_;
                const boost::optional<boost::filesystem::path>
                    webroot_dir_;
                const std::vector<easyprospect_config_service_listener_conf>
                    listeners_;

                const boost::optional<boost::filesystem::path> listen_file_;
                const boost::optional<boost::filesystem::path> listen_dir_path_;

                friend class easyprospect_config_service_core_builder;

            protected:
                easyprospect_config_service_core(const make_shared_enabler &mse, bool dh, bool dv, bool wk, bool wkup, 
                    int nt, int np, std::string wargs, ep_verbosity_type verb, ep_debug_level_type db,
                    boost::optional<std::vector<std::string>> remArgs,
                    boost::optional<boost::filesystem::path> of,
                    boost::optional<boost::filesystem::path> lf,
                    boost::optional<boost::filesystem::path> af,
                    boost::optional<boost::filesystem::path> cf,
                    boost::optional<boost::filesystem::path> pf,
                    boost::optional<boost::filesystem::path> pd,
                    boost::optional<boost::filesystem::path> sf,
                    boost::optional<boost::filesystem::path> wkconf,
                    boost::optional<boost::filesystem::path> wkexe,
                    boost::optional<boost::filesystem::path> lstfile,
                    boost::optional<boost::filesystem::path> lstdir,
                    std::vector<easyprospect_config_service_listener_conf> ls)
                        :easyprospect_config_core( mse, dh, dv, verb, db, remArgs, of, 
                            lf, af, cf, pf, pd), num_threads_(nt), num_workers_(np), worker_(wk),
                            worker_exe_(wkexe), worker_exe_use_path_(wkup), worker_args_(wargs),
                            worker_conf_(wkconf), listen_file_(lstfile), listen_dir_path_(lstdir),
                            webroot_dir_(sf), listeners_(ls)
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
                bool worker_;
                bool worker_exe_use_path_;
                int num_threads_;
                int num_workers_;
                std::string worker_args_;
                boost::optional<boost::filesystem::path>
                    worker_conf_;
                boost::optional<boost::filesystem::path>
                    worker_exe_;
                boost::optional<boost::filesystem::path>
                    webroot_dir_;
                std::vector<easyprospect_config_service_listener_conf>
                    listeners_;
                boost::optional<boost::filesystem::path> listen_file_;
                boost::optional<boost::filesystem::path> listen_dir_path_;
            public:
                easyprospect_config_service_core_builder()
                    : easyprospect_config_core_builder()
                {
                    webroot_dir_ = "wwwroot";
                    num_threads_ = 1;
                };

                void set_worker(bool w)
                {
                    worker_ = w;
                }

                void set_worker(std::string w)
                {
                    worker_ = boost::lexical_cast<bool>(w);
                }

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

                void set_num_workers(int nw)
                {
                    num_workers_ = nw;
                }

                void set_num_workers(std::string nw)
                {
                    int res = std::stoi(nw);

                    set_num_workers(res);
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

                void set_worker_exe(boost::optional<boost::filesystem::path> sf)
                {
                    worker_exe_ = sf;
                }

                void set_worker_exe(std::string sf)
                {
                    boost::optional<boost::filesystem::path>
                        res = boost::filesystem::path(sf);

                    set_worker_exe(res);
                }

                void set_worker_conf(boost::optional<boost::filesystem::path> sf)
                {
                    worker_conf_ = sf;
                }

                void set_worker_conf(std::string sf)
                {
                    boost::optional<boost::filesystem::path>
                        res = boost::filesystem::path(sf);

                    set_worker_conf(res);
                }

                void set_listen_file(boost::optional<boost::filesystem::path> sf)
                {
                    listen_file_ = sf;
                }

                void set_listen_file(std::string sf)
                {
                    boost::optional<boost::filesystem::path>
                        res = boost::filesystem::path(sf);

                    set_listen_file(res);
                }

                void set_listen_dir(boost::optional<boost::filesystem::path> sf)
                {
                    listen_dir_path_ = sf;
                }

                void set_listen_dir(std::string sf)
                {
                    boost::optional<boost::filesystem::path>
                        res = boost::filesystem::path(sf);

                    set_listen_dir(res);
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
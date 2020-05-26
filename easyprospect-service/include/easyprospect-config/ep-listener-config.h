#pragma once
#include <string>
#include <vector>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>

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
            easyprospect_config_service_listener_conf(
                std::string n,
                std::string a,
                std::string p,
                std::string minp,
                std::string maxp,
                std::string k) :
                name_(n),
                address_(a), port_(p), min_port_(minp), max_port_(maxp), kind_(k){};

            easyprospect_config_service_listener_conf(){};

            std::string get_name() const
            {
                return name_;
            }
            std::string get_port() const
            {
                return port_;
            }
            std::string get_min_port() const
            {
                return min_port_;
            }
            std::string get_max_port() const
            {
                return max_port_;
            }
            std::string get_address() const
            {
                return address_;
            }
            std::string get_kind() const
            {
                return kind_;
            }

            std::string str();

            static std::string to_string(listener_kind k)
            {
                switch (k)
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
                if (k == "allow_tls")
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

                for (auto i : s)
                {
                    auto l = parse(i);
                    res.push_back(parse(i));
                }

                return res;
            }

            static easyprospect_config_service_listener_conf parse(std::string s)
            {
                boost::escaped_list_separator<char>                   sep;
                boost::tokenizer<boost::escaped_list_separator<char>> tok(s, sep);

                std::vector<std::string> args(tok.begin(), tok.end());

                if (args.empty())
                {
                    // TODO: KP. Log this
                    throw std::logic_error("listener config is empty.");
                }
                else if (args.size() < 3)
                {
                    throw std::logic_error("listener config should be provided in the format "
                                           "'name=<name>,address=<address>,port=<port>[,min-port=<min "
                                           "port>,max-port=<max port>],kind=<kind>' ( without the quote ).");
                }

                std::string name;
                std::string address;
                std::string port;
                std::string min_port;
                std::string max_port;
                std::string kind;

                for (auto a : args)
                {
                    std::string::size_type pos = a.find('=');
                    if (pos != std::string::npos)
                    {
                        auto n = a.substr(0, pos);
                        auto v = a.substr(pos + 1, a.length());

                        if (n == "name")
                        {
                            name = v;
                        }
                        else if (n == "address")
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
    } // namespace config
} // namespace service
} // namespace easyprospect
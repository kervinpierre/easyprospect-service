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
        enum class backend_kind
        {
            none,
            standard
        };

        class easyprospect_config_service_backend_conf final
        {
          private:
            std::string name_;
            std::string address_;
            std::string port_;
            std::string min_port_;
            std::string max_port_;
            std::string kind_;

          public:
            easyprospect_config_service_backend_conf(
                std::string n,
                std::string a,
                std::string p) :
                name_(n),
                address_(a), port_(p){};

            easyprospect_config_service_backend_conf(){};

            std::string get_name() const
            {
                return name_;
            }
            std::string get_port() const
            {
                return port_;
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

            static std::string to_string(backend_kind k)
            {
                switch (k)
                {
                case backend_kind::standard:
                    return "standard";
                case backend_kind::none:
                    return "none";
                default:
                    throw std::logic_error("unknown kind");
                }
            }

            static backend_kind str_to_backend_kind(std::string k)
            {
                if (k == "standard")
                {
                    return backend_kind::standard;
                }
                else
                {
                    return backend_kind::none;
                }
            }

            static std::vector<easyprospect_config_service_backend_conf> parse(std::vector<std::string> s)
            {
                std::vector<easyprospect_config_service_backend_conf> res;

                for (auto i : s)
                {
                    auto l = parse(i);
                    res.push_back(parse(i));
                }

                return res;
            }

            static easyprospect_config_service_backend_conf parse(std::string s)
            {
                boost::escaped_list_separator<char>                   sep;
                boost::tokenizer<boost::escaped_list_separator<char>> tok(s, sep);

                std::vector<std::string> args(tok.begin(), tok.end());

                if (args.empty())
                {
                    // TODO: KP. Log this
                    throw std::logic_error("backend config is empty.");
                }
                else if (args.size() < 3)
                {
                    throw std::logic_error("backend config should be provided in the format "
                                           "be1=127.0.0.1:9101,be2=127.0.0.1:9102" );
                }

                std::string name;
                std::string address;
                std::string port;

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
                    }
                }

                easyprospect_config_service_backend_conf res(name, address, port);

                return res;
            }
        };

        inline std::string easyprospect_config_service_backend_conf::str()
        {
            std::stringstream sstr;

            sstr << "name\t\t:" << get_name() << std::endl
                 << "port\t\t:" << get_port() << std::endl
                 << "address\t:" << get_address() << std::endl
                 << "kind\t:" << get_kind() << std::endl;

            return sstr.str();
        }
    } // namespace config
} // namespace service
} // namespace easyprospect
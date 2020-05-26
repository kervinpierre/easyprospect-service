#include <sstream>
#include <easyprospect-config/ep-listener-config.h>

std::string easyprospect::service::config::easyprospect_config_service_listener_conf::str()
{
    std::stringstream sstr;

    sstr << "name\t\t:" << get_name() << std::endl
         << "port\t\t:" << get_port() << std::endl
         << "min-port\t:" << get_min_port() << std::endl
         << "max-port\t:" << get_max_port() << std::endl
         << "address\t:" << get_address() << std::endl
         << "kind\t:" << get_kind() << std::endl;

    return sstr.str();
}

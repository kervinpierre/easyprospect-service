#pragma once

#include <string>
#include <functional>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

#include <easyprospect-common/easyprospect-plugin.h>

namespace easyprospect
{
    namespace service
    {
        namespace plugin
        {
            class easyprospect_service_plugin : public common::plugin::easyprospect_plugin
            {

            protected:
                easyprospect_service_plugin(
                  const std::string        name,
                  const common::plugin::ep_plugin_category type,
                  const common::plugin::ep_plugin_type_id type_id, 
                    const boost::uuids::uuid id) :
                    easyprospect_plugin{name, type, type_id, id}
                {}

            public:
                easyprospect_service_plugin() = delete;
            };
        }
    }
}

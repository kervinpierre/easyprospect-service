#pragma once

#include <string>
#include <functional>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

#include "easyprospect-common/easyprospect-plugin.h"

namespace easyprospect
{
    namespace ep_v8
    {
        namespace plugin
        {
            class easyprospect_v8_plugin : public common::plugin::easyprospect_plugin
            {

            protected:
                easyprospect_v8_plugin(
                  const std::string        name,
                  const common::plugin::ep_plugin_category type,
                  const common::plugin::ep_plugin_type_id type_id, 
                    const boost::uuids::uuid id) :
                    easyprospect_plugin{name, type, type_id, id}
                {}

            public:
                easyprospect_v8_plugin() = delete;
            };
        }
    }
}

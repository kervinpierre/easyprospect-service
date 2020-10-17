#pragma once

#include <string>
#include <functional>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

namespace easyprospect
{
    namespace ep_v8
    {
        namespace plugin
        {
            enum class ep_plugin_type_id : int
            {
                PLUGIN_NONE = 0,
                PLUGIN_PRINT_CONSOLE = 1
            };

            enum class ep_plugin_category : int
            {
                TYPE_NONE = 0,
                TYPE_PRINT = 1
            };

            class easyprospect_plugin
            {
            private:
                const std::string  plugin_name;
                const ep_plugin_category plugin_category;
                const ep_plugin_type_id   plugin_typeId;
                const boost::uuids::uuid   plugin_id;

            protected:
                easyprospect_plugin() = delete;
                easyprospect_plugin(std::string name, ep_plugin_category type,
                    ep_plugin_type_id type_id, boost::uuids::uuid id) :
                    plugin_name(name), plugin_category(type),
                    plugin_typeId(type_id), 
                    plugin_id(id){};
            };
        }
    }
}
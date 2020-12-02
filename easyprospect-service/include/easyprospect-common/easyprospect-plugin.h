#pragma once

#include <string>
#include <functional>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

namespace easyprospect
{
    namespace common
    {
        namespace plugin
        {
            enum class ep_plugin_type_id : int
            {
                PLUGIN_NONE = 0,
                PLUGIN_V8_PRINT_CONSOLE = 1,
                PLUGIN_SRV_SALESFORCE = 2
            };

            enum class ep_plugin_category : int
            {
                TYPE_NONE = 0,
                TYPE_PRINT = 1,
                TYPE_SERVICE = 2
            };

            class easyprospect_plugin
            {
            protected:
                const std::string  plugin_name_;
                const ep_plugin_category plugin_category_;
                const ep_plugin_type_id   plugin_type_id_;
                const boost::uuids::uuid   plugin_id_;

                easyprospect_plugin(const std::string name, 
                                    const ep_plugin_category type,
                                    const ep_plugin_type_id type_id, 
                                    const boost::uuids::uuid id) :
                    plugin_name_(name), plugin_category_(type),
                    plugin_type_id_(type_id), 
                    plugin_id_(id)
                {}

            public:
                easyprospect_plugin() = delete;
            };
        }
    }
}
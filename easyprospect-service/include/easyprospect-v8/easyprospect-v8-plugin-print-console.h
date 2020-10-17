#pragma once

#include <vector>

#include <v8.h>
#include <libplatform/libplatform.h>
#include <v8pp/class.hpp>
#include <v8pp/module.hpp>
#include <v8pp/context.hpp>

#include<easyprospect-v8/easyprospect-v8-plugin.h>

namespace easyprospect
{
    namespace ep_v8
    {
        namespace plugin
        {
            class easyprospect_plugin_print_console final : public easyprospect_plugin
            {
            protected:
                struct make_shared_enabler
                {
                    explicit make_shared_enabler(int) {};
                };

            private:
                // TODO: Private object which contains a print call

            public:
                easyprospect_plugin_print_console(const make_shared_enabler&)
                    : easyprospect_plugin("Console Print Plugin",
                        ep_plugin_category::TYPE_PRINT,
                        ep_plugin_type_id::PLUGIN_PRINT_CONSOLE,
                        boost::uuids::random_generator()())
                {

                }
                // Global Javascript functions

                static std::unique_ptr<easyprospect_plugin_print_console> get_instance();

                /**********************************************************************************************//**
                 * @fn  template <typename C, typename... T> static ::std::shared_ptr<C> easyprospect_config_core::create(T&&... args)
                 *
                 * @brief   Static constructor.  https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const
                 *
                 * @author  Kervin
                 * @date    2019-11-09
                 *
                 * @tparam  C   Type of the c.
                 * @tparam  T   Generic type parameter.
                 * @param   args    Variable arguments providing [in,out] The arguments.
                 **************************************************************************************************/

                template <typename C, typename... T>
                static ::std::unique_ptr<C> create(T&&... args)
                {
                    return ::std::make_unique<C>(make_shared_enabler{ 0 },
                        ::std::forward<T>(args)...);
                }

                // TODO: register with a module
                static void register_plugin(std::shared_ptr<v8pp::context> context);

                static void print(std::string str);
            };
        }
    }
}
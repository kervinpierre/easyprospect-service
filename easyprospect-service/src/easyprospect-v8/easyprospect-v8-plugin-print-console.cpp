#include <boost/uuid/uuid_generators.hpp>
#include <easyprospect-v8/easyprospect-v8-plugin-print-console.h>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace easyprospect::ep_v8::plugin;



std::unique_ptr<easyprospect_plugin_print_console>
easyprospect_plugin_print_console::get_instance()
{
    auto res = create<easyprospect_plugin_print_console>();

    return res;
}

void easyprospect::ep_v8::plugin::easyprospect_plugin_print_console::register_plugin(std::shared_ptr<v8pp::context> context)
{
    auto iso = context->isolate();

    v8pp::module printlib(iso);

    v8pp::class_<easyprospect_plugin_print_console> print_console(iso);
    print_console.set("print", &easyprospect_plugin_print_console::print);

    printlib.set("out", print_console);
    auto con2 = iso->GetCurrentContext();

    con2->Global()->Set(con2, v8::String::NewFromUtf8(iso, "console").ToLocalChecked(), printlib.new_instance());
}

void easyprospect::ep_v8::plugin::easyprospect_plugin_print_console::print(std::string str)
{
    spdlog::debug(str);
}


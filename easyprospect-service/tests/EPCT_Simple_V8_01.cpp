#define BOOST_TEST_MODULE epct_simple_v8

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <boost/test/unit_test.hpp>

#include "EPCT_Utils.h"
#include "EPCT_TestMethodsSimple.h"

#include <easyprospect-v8/easyprospect-v8-plugin-print-console.h>

#include <v8pp/class.hpp>
#include <v8pp/module.hpp>
using namespace EPCT;



std::unique_ptr<easyprospect_v8> setup_fixture::ep;
unsigned int setup_fixture::id;

BOOST_TEST_GLOBAL_FIXTURE(setup_fixture);

BOOST_AUTO_TEST_CASE(Main_Simple_Math_Adds_integers_and_returns_a_value)
{
    BOOST_TEST(EPCT_Utils::ContextSingleUseToString(*setup_fixture::ep, "var s = 1; s = 2 + 3; s;") == "5");
    BOOST_TEST(EPCT_Utils::ContextSingleUseToString(*setup_fixture::ep, "var d = 3; d = 4 + 4; d;") == "8");
}

BOOST_AUTO_TEST_CASE(Main_Simple_Math_Runs_a_circle_with_adding_integers)
{
    BOOST_TEST(EPCT_Utils::ContextSingleUseToString(*setup_fixture::ep, "var s = 0; for(var i = 1; i < 100; i++) s += i; s;") == "4950");
}

BOOST_AUTO_TEST_CASE(Main_Simple_String_Concatanes_strings)
{
    BOOST_TEST(EPCT_Utils::ContextSingleUseToString(*setup_fixture::ep, "var t = \"text\"; t += \" add string\";") == "text add string");
}

BOOST_AUTO_TEST_CASE(Main_Simple_Math_Runs_a_function_which_returns_an_integer)
{
    BOOST_TEST(EPCT_Utils::ContextSingleUseToString(*setup_fixture::ep, "function f(a){return a + 2}; s=f(3);") == "5");
    BOOST_TEST(EPCT_Utils::ContextSingleUseToString(*setup_fixture::ep, "var f1 = function(a, b){return Math.pow(a, b)}; s=f1(3, 2); s;") == "9");
}

BOOST_AUTO_TEST_CASE(Main_Simple_Array_Using_an_array)
{
    BOOST_TEST(EPCT_Utils::ContextSingleUseToString(*setup_fixture::ep, "var index = [12, 5, 8, 130, 44].indexOf(8); index;") == "2");
}

BOOST_AUTO_TEST_CASE(Main_Simple_Continuation_Run_scripts_by_chunks_with_different_operations)
{
    int r = 0;
    unsigned int id = 0;
    if ((r = setup_fixture::ep->create_context(id)) != Success)
        BOOST_FAIL(setup_fixture::ep->result_code_to_string(r));

    BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id, "var s = 1; s;") == "1");
    BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id, "s += 10; s;") == "11");
    BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id, "var p = 5; p += s; p;") == "16");
    BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id, "var index = [12, 5, 8, 130, 44, 5]") == "undefined");
    BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id, "var i = index.lastIndexOf(5); i;") == "5");

    EPCT_Utils::Context(*setup_fixture::ep, id, "var arr = new Array(\"First\",\"Second\",\"Third\");");
    BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id, "var str = arr.join(\",\");str;") == "First,Second,Third");

    if ((r = setup_fixture::ep->remove_context(id)) != Success)
        BOOST_FAIL(setup_fixture::ep->result_code_to_string(r));
}

BOOST_AUTO_TEST_CASE(Main_Simple_Contexts_Run_scripts_in_different_contexts)
{
    {
        BOOST_TEST_CHECKPOINT("Contexts Run scripts in different contexts");

        int r = 0;
        unsigned int id1 = 0, id2 = 0;
        if ((r = setup_fixture::ep->create_context(id1)) != Success)
            BOOST_FAIL(setup_fixture::ep->result_code_to_string(r));
        if ((r = setup_fixture::ep->create_context(id2)) != Success)
            BOOST_FAIL(setup_fixture::ep->result_code_to_string(r));

        BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id1, "var s = 10; s;") == "10");
        BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id2, "var s = 100; s;") == "100");
        BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id1, "s += 10; s;") == "20");
        BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id2, "s += 10; s;") == "110");

        // Careful, you must remove the current isolate, or DCHECK complains
        if ((r = setup_fixture::ep->remove_context(id2)) != Success)
            BOOST_FAIL(setup_fixture::ep->result_code_to_string(r));
        if ((r = setup_fixture::ep->remove_context(id1)) != Success)
            BOOST_FAIL(setup_fixture::ep->result_code_to_string(r));

    }
}

BOOST_AUTO_TEST_CASE(_Main_Simple_CPP_Using_external_CPP_functions)
{
    int r = 0;
    unsigned int id = 0;

    if ((r = setup_fixture::ep->create_context(id)) != Success)
        BOOST_FAIL(setup_fixture::ep->result_code_to_string(r));

    auto con = setup_fixture::ep->get_context(id);
    v8::HandleScope handle_scope(con->isolate());

    v8pp::module epctlib(con->isolate());

    v8pp::class_<EPCT_TestMethodsSimple> TMS(con->isolate());
    TMS.set("f1", &EPCT_TestMethodsSimple::f1);
    TMS.set("f2", &EPCT_TestMethodsSimple::f2);
    TMS.set("f3", &EPCT_TestMethodsSimple::f3);
    TMS.set("f4", &EPCT_TestMethodsSimple::f4);
    TMS.set("str_func", &EPCT_TestMethodsSimple::str_func);
    
    epctlib.set("TMS", TMS);
    auto con2 = con->isolate()->GetCurrentContext();

    con2->Global()->Set(con2, v8::String::NewFromUtf8(con->isolate(), "epct").ToLocalChecked(), epctlib.new_instance());

    //ep.set_function(id, "f1", &EPCT_TestMethodsSimple::f1);
    BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id, "var s = epct.TMS.f1(1); s;") == "1");

   // ep.set_function(id, "f2", &EPCT_TestMethodsSimple::f2);
    BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id, "var s = epct.TMS.f2(5, epct.TMS.f1(10)); s;") == "15");

    //ep.set_function(id, "str_func", &EPCT_TestMethodsSimple::str_func);
    BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id, "var s = epct.TMS.str_func(\"word\", \"one\"); s;") == "wordone");

   // ep.set_function(id, "f3", &EPCT_TestMethodsSimple::f3);
    BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id, "var s = epct.TMS.f1(5) + Math.round(epct.TMS.f3(epct.TMS.f2(10, 3), 2.3)); s;") == "35");

    //ep.set_function(id, "f4", &EPCT_TestMethodsSimple::f4);
    BOOST_TEST(EPCT_Utils::Context(*setup_fixture::ep, id, "var i = epct.TMS.f4(2, 3, 4); i;") == "16");

    if ((r = setup_fixture::ep->remove_context(id)) != Success)
       BOOST_FAIL(setup_fixture::ep->result_code_to_string(r));
}

BOOST_AUTO_TEST_CASE(_Main_Simple_CPP_Using_external_print_functions)
{
    int r = 0;
    unsigned int id = 0;

    if ((r = setup_fixture::ep->create_context(id)) != Success)
        BOOST_FAIL(setup_fixture::ep->result_code_to_string(r));

    auto con = setup_fixture::ep->get_context(id);
    v8::HandleScope handle_scope(con->isolate());

    v8pp::module epctlib(con->isolate());

    v8pp::class_<easyprospect::ep_v8::plugin::easyprospect_plugin_print_console> print_console(con->isolate());
    print_console.set("print", &easyprospect::ep_v8::plugin::easyprospect_plugin_print_console::print);

    epctlib.set("out2", print_console);
    auto con2 = con->isolate()->GetCurrentContext();

    con2->Global()->Set(con2, v8::String::NewFromUtf8(con->isolate(), "con2").ToLocalChecked(), epctlib.new_instance());

    EPCT_Utils::Context(*setup_fixture::ep, id, "con2.out2.print(\"hi\");");

    if ((r = setup_fixture::ep->remove_context(id)) != Success)
        BOOST_FAIL(setup_fixture::ep->result_code_to_string(r));
}

BOOST_AUTO_TEST_CASE(_Main_Simple_CPP_Console_Print)
{
    int r = 0;
    unsigned int id = 0;

    if ((r = setup_fixture::ep->create_context(id)) != Success)
        BOOST_FAIL(setup_fixture::ep->result_code_to_string(r));

    auto con = setup_fixture::ep->get_context(id);
    v8::HandleScope handle_scope(con->isolate());

    easyprospect::ep_v8::plugin::easyprospect_plugin_print_console::register_plugin(con);

    EPCT_Utils::Context(*setup_fixture::ep, id, "console.out.print(\"hi\");");

    if ((r = setup_fixture::ep->remove_context(id)) != Success)
        BOOST_FAIL(setup_fixture::ep->result_code_to_string(r));
}

BOOST_AUTO_TEST_CASE(Main_Simple_JS_Error)
{
    BOOST_CHECK_THROW(EPCT_Utils::ContextSingleUse(*setup_fixture::ep, "var t = ;"), std::logic_error);
}

BOOST_AUTO_TEST_CASE(Main_Simple_JS_Using_JS_classes)
{
    BOOST_TEST(EPCT_Utils::ContextSingleUseToString(*setup_fixture::ep, "\
        class A {\
            constructor() {}\
            func()\
            {\
                return 5;\
            }\
        };\
        class B extends A\
        {\
            constructor() {super() }\
        };\
        var a = new A();\
        var b = a.func();\
        b;") == "5");
}

BOOST_AUTO_TEST_CASE(Main_Simple_JS_Using_with_keyword)
{
    BOOST_TEST(EPCT_Utils::ContextSingleUseToString(*setup_fixture::ep, "\
        function f1(o, str)\
        {\
            with (o)\
            {\
                return function () { return eval(str); }\
            }\
        }\
        var f = f1({}, \"true\")(); f;") == "true");
}


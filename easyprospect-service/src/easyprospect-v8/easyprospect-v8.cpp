#include <sstream>

#include "easyprospect-v8/easyprospect-v8.h"

#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// EasyProspect

using namespace easyprospect::ep_v8::api;

easyprospect_v8::~easyprospect_v8()
{
    contexts.clear();
}

std::shared_ptr<Platform> easyprospect_v8::get_platform()
{
    return platform;
}

int easyprospect_v8::init(const char* exec_path)
{
    if (exec_path)
    {
        v8::V8::InitializeICUDefaultLocation(exec_path);
    }
    else
    {
        auto cwd = boost::filesystem::current_path();
        v8::V8::InitializeICUDefaultLocation(cwd.generic_string().c_str());
    }

    // v8::V8::InitializeExternalStartupData(exec_path);
    V8::InitializePlatform(platform.get());
    V8::Initialize();

    return Success;
}

void easyprospect_v8::destroy()
{
    V8::Dispose();
    V8::ShutdownPlatform();
}

int easyprospect_v8::create_context(unsigned int& id)
{
    // Local<Context> local_context = Context::New(isolate);
    // id = get_next_context_id();
    // contexts[id] = new Persistent<Context>(isolate, local_context);
    // local_context->Enter();

    auto context = std::make_shared<v8pp::context>();
    id           = get_next_context_id();
    contexts.insert({id, context});

    return Success;
}

std::shared_ptr<v8pp::context> easyprospect_v8::get_context(unsigned int id)
{
    return contexts.at(id);
}

int easyprospect_v8::remove_context(const unsigned int id)
{
    auto it = contexts.find(id);
    if (it == contexts.end())
        return ContextNotFound;
    it->second.reset();
    // it->second->Reset();
    // delete it->second;
    contexts.erase(it);
    return Success;
}

void easyprospect_v8::report_exception(v8::Isolate* isolate, v8::TryCatch* try_catch)
{
    v8::HandleScope        handle_scope(isolate);
    v8::String::Utf8Value  exception(isolate, try_catch->Exception());
    const char*            exception_string = to_cstring(exception);
    v8::Local<v8::Message> message          = try_catch->Message();

    if (message.IsEmpty())
    {
        // V8 didn't provide any extra information about this error; just
        // print the exception.

        // fprintf(stderr, "%s\n", exception_string);
        spdlog::error("{}\n", exception_string);
    }
    else
    {
        std::ostringstream oss;

        oss << std::endl;

        // Print (filename):(line number): (message).
        v8::String::Utf8Value  filename(isolate, message->GetScriptOrigin().ResourceName());
        v8::Local<v8::Context> context(isolate->GetCurrentContext());
        const char*            filename_string = to_cstring(filename);
        int                    linenum         = message->GetLineNumber(context).FromJust();
        // fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
        oss << filename_string << ":" << linenum << ":" << exception_string << std::endl;

        // Print line of source code.
        v8::String::Utf8Value sourceline(isolate, message->GetSourceLine(context).ToLocalChecked());
        const char*           sourceline_string = to_cstring(sourceline);
        // fprintf(stderr, "%s\n", sourceline_string);
        oss << sourceline_string << std::endl;

        // Print wavy underline (GetUnderline is deprecated).
        int start = message->GetStartColumn(context).FromJust();
        for (int i = 0; i < start; i++)
        {
            oss << " ";
            // fprintf(stderr, " ");
        }
        int end = message->GetEndColumn(context).FromJust();
        for (int i = start; i < end; i++)
        {
            oss << "^";
            // fprintf(stderr, "^");
        }
        oss << std::endl;
        // fprintf(stderr, "\n");

        v8::Local<v8::Value> stack_trace_string;
        if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) && stack_trace_string->IsString() &&
            v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0)
        {
            v8::String::Utf8Value stack_trace(isolate, stack_trace_string);
            const char*           stack_trace_string = to_cstring(stack_trace);
            // fprintf(stderr, "%s\n", stack_trace_string);
            oss << stack_trace_string << std::endl;
        }

        spdlog::error(oss.str());
    }
}

std::unique_ptr<EpJsResult> easyprospect_v8::run_javascript(const unsigned int id, const std::string& script)
{
    if (script.empty() || std::all_of(script.begin(), script.end(), isspace))
    {
        spdlog::warn("Called to run an empty script.");
    }

    auto it = contexts.find(id);
    if (it == contexts.end())
        throw std::logic_error("context not found");

    auto context = it->second;

    HandleScope handle_scope(context->isolate());

    TryCatch try_catch(context->isolate());

    // FIXME: We're compiling twice.  First to check the validity, then in the v8pp library.
    //        This is ok for now until we optimize.
    MaybeLocal<String> source =
        String::NewFromUtf8(context->isolate(), script.c_str(), NewStringType::kNormal, script.size());
    Local<Script> s;
    if (!Script::Compile(context->isolate()->GetCurrentContext(), source.ToLocalChecked()).ToLocal(&s))
    {
        report_exception(context->isolate(), &try_catch);

        throw std::logic_error("compile error");
    }
    std::cout << "testing...";
    // Now call using v8pp
    Local<Value> resV8 = context->run_script(script);

    if (try_catch.HasCaught())
    {
        report_exception(context->isolate(), &try_catch);

        throw std::logic_error("run error");
    }

    auto res = EpJsResult::CreateResult(resV8, context->isolate());

    return res;
}

std::string easyprospect_v8::result_code_to_string(int result_code)
{
    static std::map<int, std::string> result_code_str = {{Success, "Success"},
                                                         {ContextNotFound, "Context not found"},
                                                         {CompileError, "Compile error"},
                                                         {RuntimeError, "Runtime error"}};
    return result_code_str[result_code];
}

unsigned int easyprospect_v8::get_next_context_id()
{
    return next_id++;
}

std::string easyprospect_v8::value_to_string(Local<Value> val, Isolate* isolate)
{
    String::Utf8Value data(isolate, val);
    const char*       p = *data;
    if (!p)
        return std::string();
    return std::string(p);
}

// Extracts a C string from a V8 Utf8Value.
const char* easyprospect_v8::to_cstring(const v8::String::Utf8Value& value)
{
    return *value ? *value : "<string conversion failed>";
}

std::double_t easyprospect_v8::value_to_number(Local<Value> val, Isolate* isolate)
{
    Number* currNum = Number::Cast(*val);

    std::double_t res = currNum->Value();

    return res;
}

std::int32_t easyprospect_v8::value_to_int32(Local<Value> val, Isolate* isolate)
{
    Int32* currInt = Int32::Cast(*val);

    std::int32_t res = currInt->Value();

    return res;
}

bool easyprospect_v8::value_to_boolean(Local<Value> val, Isolate* isolate)
{
    bool res = val->ToBoolean(isolate)->IsTrue();

    return res;
}

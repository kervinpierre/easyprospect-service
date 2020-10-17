#pragma once

#include <string>
#include <map>
#include <v8.h>
#include <libplatform/libplatform.h>
#include "function.h"
#include "convert.h"
#include "result_code.h"

#include "EpJsResult.h"

using namespace v8;

namespace easyprospect
{
    namespace ep_v8
    {
        namespace api
        {
            /**
              Main class to run JavaScripts in the V8 engine.
            */
            class easyprospect_v8_2
            {
            public:
                /**
                 * @brief Constructor
                 */
                easyprospect_v8_2();
                /**
                 * @brief Destructor
                 */
                ~easyprospect_v8_2();

                /**
                 * @brief Init V8
                 * @param[in] exec_path path to the current application
                 * @return error code
                 */
                int init(const char* exec_path);

                /**
                 * @brief Create JavaScript context
                 * @param[out] id Context id
                 * @return error code
                 * @see remove_context
                 */
                int create_context(unsigned int& id);

                /**
                 * @brief Remove JavaScript context
                 * @param[in] id Context id
                 * @return error code
                 * @see CreateContext
                 */
                int remove_context(const unsigned int id);

                /**
                 * @brief Run JavaScript code on a context
                 * @param[in] id Context id
                 * @param[in] script JavaScript code
                 * @param[out] result output of executed JavaScript code
                 * @return error code
                 */
                std::unique_ptr<EpJsResult> run_javascript(const unsigned int id, const std::string& script);

                /**
                 * @brief Convert a result code to a string
                 * @param[in] result code
                 * @return corresponding string
                 */
                std::string result_code_to_string(int result_code);

                /**
                 * @brief Set an external C/C++ function to be run from the JavaScript code
                 * @param[in] id Context id
                 * @param[in] name function's name
                 * @param[out] func external function
                 * @return error code
                 */
                template<typename F, typename Traits = raw_ptr_traits>
                int set_function(const unsigned int id, const std::string& name, F&& func)
                {
                    auto it = contexts.find(id);
                    if (it == contexts.end())
                        return ContextNotFound;

                    Isolate::Scope isolate_scope(isolate);
                    HandleScope handle_scope(isolate);

                    TryCatch try_catch(isolate);

                    Persistent<Context>* context = it->second;

                    Local<Context> local_context(context->Get(isolate));

                    using F_type = typename std::decay<F>::type;
                    Local<Function> fn = Function::New(local_context, &ForwardFunction<Traits, F_type>,
                        set_external_data(isolate, std::forward<F_type>(func))).ToLocalChecked();

                    MaybeLocal<String> s = String::NewFromUtf8(isolate, name.c_str(), NewStringType::kNormal, name.size()).ToLocalChecked();
                    local_context->Global()->Set(isolate->GetCurrentContext(), s.ToLocalChecked(), fn);

                    return Success;
                }


                static std::string  value_to_string(Local<Value> val, Isolate* isolate);
                static std::double_t value_to_number(Local<Value> val, Isolate* isolate);
                static std::int32_t value_to_int32(Local<Value> val, Isolate* isolate);
                static bool         value_to_boolean(Local<Value> val, Isolate* isolate);

                static const char* to_cstring(const v8::String::Utf8Value& value);

                static void report_exception(v8::Isolate* isolate, v8::TryCatch* try_catch);

            private:
                unsigned int get_next_context_id();

            protected:
                std::unique_ptr<Platform> platform;
                Isolate::CreateParams create_params;

                std::map<unsigned int, Persistent<Context>*> contexts;
                Isolate* isolate = nullptr;

            private:
                unsigned int next_id = 1;
            };
        }
    }
}

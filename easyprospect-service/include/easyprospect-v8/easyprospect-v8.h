#pragma once

#include "convert.h"
#include "function.h"
#include "result_code.h"
#include <libplatform/libplatform.h>
#include <map>
#include <string>
#include <v8.h>

#include "EpJsResult.h"

#include <v8pp/context.hpp>

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
        class easyprospect_v8 final
        {
          protected:
            struct make_shared_enabler
            {
                explicit make_shared_enabler(int){};
            };

          public:
            /**
             * @brief Constructor
             */
            easyprospect_v8(const make_shared_enabler&, std::shared_ptr<Platform> pf) : platform(pf){};

            /**
             * @brief Destructor
             */
            ~easyprospect_v8();

            /**********************************************************************************************
             * @fn  template <typename C, typename... T> static ::std::shared_ptr<C>
             *easyprospect_config_core::create(T&&... args)
             *
             * @brief   Static constructor.
             *https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const
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
                return ::std::make_unique<C>(make_shared_enabler{0}, ::std::forward<T>(args)...);
            }

            std::shared_ptr<Platform> get_platform();

            /**
             * @brief Init V8
             * @param[in] exec_path path to the current application
             * @return error code
             */
            int         init(const char* exec_path = nullptr);
            static void destroy();

            /**
             * @brief Create JavaScript context
             * @param[out] id Context id
             * @return error code
             * @see remove_context
             */
            int create_context(unsigned int& id);

            std::shared_ptr<v8pp::context> get_context(unsigned int id);

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

            static std::string   value_to_string(Local<Value> val, Isolate* isolate);
            static std::double_t value_to_number(Local<Value> val, Isolate* isolate);
            static std::int32_t  value_to_int32(Local<Value> val, Isolate* isolate);
            static bool          value_to_boolean(Local<Value> val, Isolate* isolate);

            static const char* to_cstring(const v8::String::Utf8Value& value);

            static void report_exception(v8::Isolate* isolate, v8::TryCatch* try_catch);

          private:
            unsigned int get_next_context_id();

          protected:
            std::shared_ptr<Platform> platform;
            Isolate::CreateParams     create_params;

            std::map<unsigned int, std::shared_ptr<v8pp::context>> contexts;
            // Isolate* isolate = nullptr;

          private:
            unsigned int next_id = 1;
        };
    } // namespace api
} // namespace ep_v8
} // namespace easyprospect

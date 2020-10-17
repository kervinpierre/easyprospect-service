#pragma once

#include <v8.h>
#include "call.h"

template<typename T>
using is_pointer_cast_allowed = std::integral_constant<bool,
    sizeof(T) <= sizeof(void*) && std::is_trivial<T>::value>;

template<typename T>
union PointerCast
{
private:
    void* ptr;
    T value;

public:
    static_assert(is_pointer_cast_allowed<T>::value, "PointerCast is not allowed");

    explicit PointerCast(void* ptr) : ptr(ptr)
    {
    }

    explicit PointerCast(T value) : value(value)
    {
    }

    operator void*() const
    {
        return ptr;
    }

    operator T() const
    {
        return value;
    }
};

template<typename T>
class ExternalData
{
public:
    static v8::Local<v8::External> set(v8::Isolate* isolate, T&& data)
    {
        ExternalData* value = new ExternalData;
        try
        {
            new (value->storage()) T(std::forward<T>(data));
        }
        catch (...)
        {
            delete value;
            throw;
        }

        v8::Local<v8::External> ext = v8::External::New(isolate, value);
        value->pext_.Reset(isolate, ext);
        value->pext_.SetWeak(value,
            [](v8::WeakCallbackInfo<ExternalData> const& data)
        {
            delete data.GetParameter();
        }, v8::WeakCallbackType::kParameter);
        return ext;
    }

    static T& get(v8::Local<v8::External> ext)
    {
        ExternalData* value = static_cast<ExternalData*>(ext->Value());
        return *static_cast<T*>(value->storage());
    }

private:
    void* storage()
    {
        return &storage_;
    }

    ~ExternalData()
    {
        if (!pext_.IsEmpty())
        {
            static_cast<T*>(storage())->~T();
            pext_.Reset();
        }
    }

    using data_storage = typename std::aligned_storage<sizeof(T)>::type;
    data_storage storage_;
    v8::Global<v8::External> pext_;
};

template<typename T>
typename std::enable_if<is_pointer_cast_allowed<T>::value, v8::Local<v8::Value>>::type
set_external_data(v8::Isolate* isolate, T value)
{
    return v8::External::New(isolate, PointerCast<T>(value));
}

template<typename T>
typename std::enable_if<!is_pointer_cast_allowed<T>::value, v8::Local<v8::Value>>::type
set_external_data(v8::Isolate* isolate, T&& value)
{
    return ExternalData<T>::set(isolate, std::forward<T>(value));
}

template<typename T>
typename std::enable_if<is_pointer_cast_allowed<T>::value, T>::type
get_external_data(v8::Local<v8::Value> value)
{
    return PointerCast<T>(value.As<v8::External>()->Value());
}

template<typename T>
typename std::enable_if<!is_pointer_cast_allowed<T>::value, T&>::type
get_external_data(v8::Local<v8::Value> value)
{
    return ExternalData<T>::get(value.As<v8::External>());
}

template<typename Traits, typename F>
typename function_traits<F>::return_type
invoke(v8::FunctionCallbackInfo<v8::Value> const& args, std::false_type /*is_member_function_pointer*/)
{
    return call_from_v8<Traits, F>(std::forward<F>(get_external_data<F>(args.Data())), args);
}

template<typename Traits, typename F>
void ForwardRet(v8::FunctionCallbackInfo<v8::Value> const& args, std::true_type /*is_void_return*/)
{
    invoke<Traits, F>(args, std::is_member_function_pointer<F>());
}

template<typename Traits, typename F>
void ForwardRet(v8::FunctionCallbackInfo<v8::Value> const& args, std::false_type /*is_void_return*/)
{
    using return_type = typename function_traits<F>::return_type;
    using converter = typename CallFromTraits<F>::template arg_converter<return_type, Traits>;
    args.GetReturnValue().Set(converter::to_v8(args.GetIsolate(),
        invoke<Traits, F>(args, std::is_member_function_pointer<F>())));
}

extern Local<Value> throw_ex(Isolate* isolate, char const* str);

extern Local<Value> throw_ex(Isolate* isolate, char const* str, Local<Value>(*exception_ctor)(Local<String>));

template<typename Traits, typename F>
void ForwardFunction(v8::FunctionCallbackInfo<v8::Value> const& args)
{
    static_assert(is_callable<F>::value || std::is_member_function_pointer<F>::value, "required callable F");

    v8::Isolate* isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    try
    {
        ForwardRet<Traits, F>(args, is_void_return<F>());
    }
    catch (std::exception const& ex)
    {
        args.GetReturnValue().Set(throw_ex(isolate, ex.what()));
    }
}

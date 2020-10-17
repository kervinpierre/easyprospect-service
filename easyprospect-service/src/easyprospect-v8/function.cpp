#include <easyprospect-v8/function.h>

Local<Value> throw_ex(Isolate* isolate, char const* str)
{
#ifdef _WIN32
	//return isolate->ThrowException(String::NewFromUtf8(isolate, str));
    return isolate->ThrowException(String::NewFromUtf8(isolate, str).ToLocalChecked());

    #else
	return isolate->ThrowException(String::NewFromUtf8(isolate, str).ToLocalChecked());
#endif
}

Local<Value> throw_ex(Isolate* isolate, char const* str, Local<Value>(*exception_ctor)(Local<String>))
{
#ifdef _WIN32
	//return isolate->ThrowException(exception_ctor(String::NewFromUtf8(isolate, str)));
    return isolate->ThrowException(exception_ctor(String::NewFromUtf8(isolate, str).ToLocalChecked()));
#else
	return isolate->ThrowException(exception_ctor(String::NewFromUtf8(isolate, str).ToLocalChecked()));
#endif
}

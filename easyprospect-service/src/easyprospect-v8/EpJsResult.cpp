#include <easyprospect-v8/EpJsResult.h>
#include <easyprospect-v8/easyprospect-v8-2.h>

using namespace easyprospect::ep_v8::api;

constexpr EpJsResultValueType easyprospect::ep_v8::api::operator|(EpJsResultValueType lhs, EpJsResultValueType rhs)
{
    return static_cast<EpJsResultValueType> (
        static_cast<std::underlying_type<EpJsResultValueType>::type>(lhs) |
        static_cast<std::underlying_type<EpJsResultValueType>::type>(rhs)
        );
}


constexpr EpJsResultValueType easyprospect::ep_v8::api::operator&(EpJsResultValueType lhs, EpJsResultValueType rhs)
{
    return static_cast<EpJsResultValueType> (
        static_cast<std::underlying_type<EpJsResultValueType>::type>(lhs) &
        static_cast<std::underlying_type<EpJsResultValueType>::type>(rhs)
        );
}

EpJsResult& easyprospect::ep_v8::api::EpJsResult::operator=(const EpJsResult& other)
{
    return *this;
}

easyprospect::ep_v8::api::EpJsResult::~EpJsResult()
{
}

std::unique_ptr<EpJsResult>
easyprospect::ep_v8::api::EpJsResult::CreateResult(v8::Local<v8::Value> val, v8::Isolate* isolate)
{
    bool isNull = val->IsNull();
    bool IsUndefined = val->IsUndefined();
    std::string valStr, typeStr;

    std::unique_ptr<EpJsResult> res;

    // Save the string cast
    valStr  = easyprospect_v8_2::value_to_string(val, isolate);
    typeStr = easyprospect_v8_2::value_to_string(val->TypeOf(isolate), isolate);

    if (val->IsStringObject())
    {
        std::string temp = easyprospect_v8_2::value_to_string(val, isolate);
        res = std::make_unique<EpJsResult>(temp, isNull, IsUndefined, valStr, typeStr);
    }
    else if (val->IsString())
    {
        std::string temp = easyprospect_v8_2::value_to_string(val, isolate);
        res = std::make_unique<EpJsResult>(temp, isNull, IsUndefined, valStr, typeStr);
    }
    else if (val->IsBoolean())
    {
        bool temp = easyprospect_v8_2::value_to_boolean(val, isolate);
        res = std::make_unique<EpJsResult>(temp, isNull, IsUndefined, valStr, typeStr);
    }
    else if (val->IsInt32())
    {
        int64_t temp = easyprospect_v8_2::value_to_int32(val, isolate);
        res = std::make_unique<EpJsResult>(temp, isNull, IsUndefined, valStr, typeStr);
    }
    else if (val->IsNumber())
    {
        double_t temp = easyprospect_v8_2::value_to_number(val, isolate);
        res = std::make_unique<EpJsResult>(temp, isNull, IsUndefined, valStr, typeStr);
    }
    else
    {
        res = std::make_unique<EpJsResult>(isNull, IsUndefined, valStr, typeStr);
    }

    return res;
}

bool easyprospect::ep_v8::api::EpJsResult::IsUndefined() const
{
    return m_isUndefined;
}

bool easyprospect::ep_v8::api::EpJsResult::IsNull() const
{
    return m_isNull;
}

bool easyprospect::ep_v8::api::EpJsResult::IsNullOrUndefined() const
{
    if (IsNull() || IsUndefined())
    {
        return true;
    }

    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsTrue() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsFalse() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsName() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsString() const
{
    return (m_type & EpJsResultValueType::STRING) != EpJsResultValueType::NONE;
}

bool easyprospect::ep_v8::api::EpJsResult::IsSymbol() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsFunction() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsArray() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsObject() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsBigInt() const
{
    return false;
}

constexpr bool easyprospect::ep_v8::api::EpJsResult::IsBoolean() const
{
    return (m_type&EpJsResultValueType::BOOLEAN)!=EpJsResultValueType::NONE;
}

bool easyprospect::ep_v8::api::EpJsResult::IsNumber() const
{
    return (m_type & EpJsResultValueType::NUMBER) != EpJsResultValueType::NONE;
}

bool easyprospect::ep_v8::api::EpJsResult::IsExternal() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsInt32() const
{
    return (m_type & EpJsResultValueType::INTEGER) != EpJsResultValueType::NONE;
}

bool easyprospect::ep_v8::api::EpJsResult::IsUint32() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsDate() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsArgumentsObject() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsBigIntObject() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsBooleanObject() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsNumberObject() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsStringObject() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsSymbolObject() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsNativeError() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsRegExp() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsAsyncFunction() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsGeneratorFunction() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsGeneratorObject() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsPromise() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsMap() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsSet() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsMapIterator() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsSetIterator() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsWeakMap() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsWeakSet() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsArrayBuffer() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsArrayBufferView() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsTypedArray() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsUint8Array() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsUint8ClampedArray() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsInt8Array() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsUint16Array() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsInt16Array() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsUint32Array() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsInt32Array() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsFloat32Array() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsFloat64Array() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsBigInt64Array() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsBigUint64Array() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsDataView() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsSharedArrayBuffer() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsProxy() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsWebAssemblyCompiledModule() const
{
    return false;
}

bool easyprospect::ep_v8::api::EpJsResult::IsModuleNamespaceObject() const
{
    return false;
}

const bool easyprospect::ep_v8::api::EpJsResult::ToBool()
{
    if (IsUndefined())
       throw std::logic_error("result is undefined.");

    if (!IsBoolean())
        throw std::logic_error("result is not boolean.");

    if (IsNull())
        throw std::logic_error("result is null.");

    bool res = boost::get<bool>(m_simpleValue);

    return res;
}

const std::string easyprospect::ep_v8::api::EpJsResult::ToString()
{
    return m_valueString;
}

const std::string easyprospect::ep_v8::api::EpJsResult::GetJSType()
{
    return m_typeString;
}

constexpr EpJsResultValueType easyprospect::ep_v8::api::EpJsResult::GetEpType()
{
    return m_type;
}


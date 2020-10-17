#pragma once
#include <string>
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>
#include <v8.h>
#include <libplatform/libplatform.h>

namespace easyprospect
{
    namespace ep_v8
    {
        namespace api
        {
            // Does not represent all values, just the common simple values
            typedef boost::variant<nullptr_t, bool, int64_t, uint64_t, double, std::string> EpV8VariantType;
            
            enum class EpJsResultValueType : int
            {
                NONE      = 0,
                VALUE     = 1 << 0,
                OBJECT    = 1 << 1,
                DATE      = 1 << 2,
                FUNCTION  = 1 << 3,
                ARRAY     = 1 << 4,
                PRIMATIVE = 1 << 5,
                BOOLEAN   = 1 << 6,
                NUMBER    = 1 << 7,
                STRING    = 1 << 8,
                INTEGER   = 1 << 9,
            };

            constexpr EpJsResultValueType operator|(EpJsResultValueType lhs, EpJsResultValueType rhs);
            constexpr EpJsResultValueType operator&(EpJsResultValueType lhs, EpJsResultValueType rhs);

            class EpJsResult final
            {
            private:
                const bool m_isUndefined;
                const bool m_isNull;
                const std::string m_valueString;
                const std::string m_typeString;
                const EpJsResultValueType m_type;
                const EpV8VariantType              m_simpleValue;
                //const std::vector<EpV8VariantType> m_simpleValueArray;

            public:
                EpJsResult() = delete;
                explicit EpJsResult(bool isNull, bool isUndefined, std::string strVal, std::string strType) 
                    : m_simpleValue(nullptr), m_isNull(isNull), m_isUndefined(isUndefined), 
                      m_valueString(strVal), m_typeString(strType), m_type( EpJsResultValueType::NONE )
                { };

                explicit EpJsResult(int64_t val, bool isNull, bool isUndefined, std::string strVal, std::string strType) 
                    : m_simpleValue(val), m_isNull(isNull), m_isUndefined(isUndefined), 
                    m_valueString(strVal), m_typeString(strType), m_type(EpJsResultValueType::NUMBER|EpJsResultValueType::INTEGER)
                { };

                explicit EpJsResult(uint64_t val, bool isNull, bool isUndefined, std::string strVal, std::string strType) 
                    : m_simpleValue(val), m_isNull(false), m_isUndefined(false),
                    m_valueString(""), m_typeString(strType), m_type(EpJsResultValueType::NUMBER|EpJsResultValueType::INTEGER) 
                { };

                explicit EpJsResult(double val, bool isNull, bool isUndefined, std::string strVal, std::string strType) 
                    : m_simpleValue(val), m_isNull(isNull), m_isUndefined(isUndefined), 
                    m_valueString(strVal), m_typeString(strType), m_type(EpJsResultValueType::NUMBER)
                { };

                explicit EpJsResult(std::string val, bool isNull, bool isUndefined, std::string strVal, std::string strType) 
                    : m_simpleValue(val), m_isNull(isNull), m_isUndefined(isUndefined), 
                    m_valueString(strVal), m_typeString(strType), m_type(EpJsResultValueType::STRING)
                { };
                
                explicit EpJsResult(bool val, bool isNull, bool isUndefined, std::string strVal, std::string strType) 
                    : m_simpleValue(val), m_isNull(isNull), m_isUndefined(isUndefined), 
                    m_valueString(strVal), m_typeString(strType), m_type(EpJsResultValueType::BOOLEAN)
                { };

                /**
                 * Creates a new std::unique_ptr&lt;EpJsResult&gt;
                 *
                 * @author Kervin
                 * @date 2019-10-27
                 *
                 * @param val The value.
                 *
                 * @returns A std::unique_ptr&lt;EpJsResult&gt;
                 */
                static std::unique_ptr<EpJsResult>
                    CreateResult(v8::Local<v8::Value> val, v8::Isolate* isolate);

                EpJsResult& EpJsResult::operator=(const EpJsResult& other);
                ~EpJsResult();

                /**
                 * No Setter methods or modification allowed
                 */

                bool IsUndefined() const;
                bool IsNull() const;
                bool IsNullOrUndefined() const;
                bool IsTrue() const;
                bool IsFalse() const;
                bool IsName() const;
                bool IsString() const;
                bool IsSymbol() const;
                bool IsFunction() const;
                bool IsArray() const;
                bool IsObject() const;
                bool IsBigInt() const;
                constexpr bool IsBoolean() const;
                bool IsNumber() const;
                bool IsExternal() const;
                bool IsInt32() const;
                bool IsUint32() const;
                bool IsDate() const;
                bool IsArgumentsObject() const;
                bool IsBigIntObject() const;
                bool IsBooleanObject() const;
                bool IsNumberObject() const;
                bool IsStringObject() const;
                bool IsSymbolObject() const;
                bool IsNativeError() const;
                bool IsRegExp() const;
                bool IsAsyncFunction() const;
                bool IsGeneratorFunction() const;
                bool IsGeneratorObject() const;
                bool IsPromise() const;
                bool IsMap() const;
                bool IsSet() const;
                bool IsMapIterator() const;
                bool IsSetIterator() const;
                bool IsWeakMap() const;
                bool IsWeakSet() const;
                bool IsArrayBuffer() const;
                bool IsArrayBufferView() const;
                bool IsTypedArray() const;
                bool IsUint8Array() const;
                bool IsUint8ClampedArray() const;
                bool IsInt8Array() const;
                bool IsUint16Array() const;
                bool IsInt16Array() const;
                bool IsUint32Array() const;
                bool IsInt32Array() const;
                bool IsFloat32Array() const;
                bool IsFloat64Array() const;
                bool IsBigInt64Array() const;
                bool IsBigUint64Array() const;
                bool IsDataView() const;
                bool IsSharedArrayBuffer() const;
                bool IsProxy() const;
                bool IsWebAssemblyCompiledModule() const;
                bool IsModuleNamespaceObject() const;

                const bool ToBool();
                const std::string ToString();

                const     std::string         GetJSType();
                constexpr EpJsResultValueType GetEpType();
            };
        }
    }
}
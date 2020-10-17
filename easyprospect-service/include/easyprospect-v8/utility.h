#pragma once

#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <v8.h>

using namespace v8;

template<typename T>
struct tuple_tail;

template<typename Head, typename... Tail>
struct tuple_tail<std::tuple<Head, Tail...>>
{
    using type = std::tuple<Tail...>;
};

/////////////////////////////////////////////////////////////////////////////
//
// Function traits
//
template<typename F>
struct function_traits;

template<typename R, typename ...Args>
struct function_traits<R (Args...)>
{
    using return_type = R;
    using arguments = std::tuple<Args...>;
};

// function pointer
template<typename R, typename ...Args>
struct function_traits<R (*)(Args...)>
    : function_traits<R (Args...)>
{
    using pointer_type = R (*)(Args...);
};

// member function pointer
template<typename C, typename R, typename ...Args>
struct function_traits<R (C::*)(Args...)>
    : function_traits<R (C&, Args...)>
{
    template<typename D = C>
    using pointer_type = R (D::*)(Args...);
};

// const member function pointer
template<typename C, typename R, typename ...Args>
struct function_traits<R (C::*)(Args...) const>
    : function_traits<R (C const&, Args...)>
{
    template<typename D = C>
    using pointer_type = R (D::*)(Args...) const;
};

// volatile member function pointer
template<typename C, typename R, typename ...Args>
struct function_traits<R (C::*)(Args...) volatile>
    : function_traits<R (C volatile&, Args...)>
{
    template<typename D = C>
    using pointer_type = R (D::*)(Args...) volatile;
};

// const volatile member function pointer
template<typename C, typename R, typename ...Args>
struct function_traits<R (C::*)(Args...) const volatile>
    : function_traits<R (C const volatile&, Args...)>
{
    template<typename D = C>
    using pointer_type = R (D::*)(Args...) const volatile;
};

// member object pointer
template<typename C, typename R>
struct function_traits<R (C::*)>
    : function_traits<R (C&)>
{
    template<typename D = C>
    using pointer_type = R (D::*);
};

// const member object pointer
template<typename C, typename R>
struct function_traits<const R (C::*)>
    : function_traits<R (C const&)>
{
    template<typename D = C>
    using pointer_type = const R (D::*);
};

// volatile member object pointer
template<typename C, typename R>
struct function_traits<volatile R (C::*)>
    : function_traits<R (C volatile&)>
{
    template<typename D = C>
    using pointer_type = volatile R (D::*);
};

// const volatile member object pointer
template<typename C, typename R>
struct function_traits<const volatile R (C::*)>
    : function_traits<R (C const volatile&)>
{
    template<typename D = C>
    using pointer_type = const volatile R (D::*);
};

// function object, std::function, lambda
template<typename F>
struct function_traits
{
    static_assert(!std::is_bind_expression<F>::value,
        "std::bind result is not supported yet");
private:
    using callable_traits = function_traits<decltype(&F::operator())>;
public:
    using return_type = typename callable_traits::return_type;
    using arguments = typename tuple_tail<typename callable_traits::arguments>::type;
};

template<typename F>
struct function_traits<F&> : function_traits<F> {};

template<typename F>
struct function_traits<F&&> : function_traits<F> {};

template<typename F>
using is_void_return = std::is_same<void,
    typename function_traits<F>::return_type>;

template<typename F, bool is_class>
struct is_callable_impl
    : std::is_function<typename std::remove_pointer<F>::type>
{
};

template<typename F>
struct is_callable_impl<F, true>
{
private:
    struct fallback { void operator()(); };
    struct derived : F, fallback {};

    template<typename U, U> struct check;

    template<typename>
    static std::true_type test(...);

    template<typename C>
    static std::false_type test(check<void(fallback::*)(), &C::operator()>*);

    using type = decltype(test<derived>(0));
public:
    static const bool value = type::value;
};

template<typename F>
using is_callable = std::integral_constant<bool,
    is_callable_impl<F, std::is_class<F>::value>::value>;

#if (__cplusplus > 201402L) || (defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023918)
using std::index_sequence;
using std::make_index_sequence;
#else
/////////////////////////////////////////////////////////////////////////////
//
// integer_sequence
//
template<typename T, T... I>
struct integer_sequence
{
    using type = T;
    static size_t size() { return sizeof...(I); }

    template<T N>
    using append = integer_sequence<T, I..., N>;

    using next = append<sizeof...(I)>;
};

template<typename T, T Index, size_t N>
struct sequence_generator
{
    using type = typename sequence_generator<T, Index - 1, N - 1>::type::next;
};

template<typename T, T Index>
struct sequence_generator<T, Index, 0ul>
{
    using type = integer_sequence<T>;
};

template<size_t... I>
using index_sequence = integer_sequence<size_t, I...>;

template<typename T, T N>
using make_integer_sequence = typename sequence_generator<T, N, N>::type;

template<size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;
#endif

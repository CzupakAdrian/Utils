#pragma once

#include "Header.h"

#if defined _USE_MODULES

import <functional>;
import <memory>;
import <type_traits>;

#else

#include <functional>
#include <memory>
#include <type_traits>

#endif

_EXPORT_UTILS namespace Utils
{
    template < template <class> typename Test, typename... List >
    struct any_fulfills : std::true_type {};

    template < template <class> typename Test, typename Head, typename... List >
    struct any_fulfills<Test, Head, List ...>
        : std::conditional< Test<Head>::value,
        std::true_type,
        Utils::any_fulfills<Test, List...>
        >::type {};

    template < template <class> typename Test >
    struct any_fulfills<Test> : std::false_type {};

    template < template <class> typename Test, typename... List >
    _INLINE_VAR constexpr bool any_fulfills_v = any_fulfills<Test, List...>::value;

    template < template <class> typename Test, typename... List >
    struct all_fulfill : std::false_type {};

    template < template <class> typename Test, typename Head, typename... List >
    struct all_fulfill<Test, Head, List ...>
        : std::conditional< Test<Head>::value,
        Utils::all_fulfill<Test, List...>,
        std::false_type
        >::type {};

    template < template <class> typename Test >
    struct all_fulfill<Test> : std::true_type {};

    template < template <class> typename Test, typename... List >
    _INLINE_VAR constexpr bool all_fulfill_v = all_fulfill<Test, List...>::value;

    template <class T> struct is_same_as
    {
        template <class Arg>
        struct test : std::is_same<T, Arg> {};

        template <class Arg>
        _INLINE_VAR constexpr static bool test_v = test<Arg>::value;
    };

    template < typename Tp, typename... List >
    struct contains : std::true_type {};

    template < typename Tp, typename Head, typename... Rest >
    struct contains<Tp, Head, Rest...>
        : std::conditional< std::is_same<Tp, Head>::value,
        std::true_type,
        Utils::contains<Tp, Rest...>
        >::type {};

    template < typename Tp >
    struct contains<Tp> : std::false_type {};

    template < typename Tp, typename... List >
    _INLINE_VAR constexpr bool contains_v = contains<Tp, List ...>::value;

    template <class ... Args>
    struct any_of
    {
        //template <class ... Types>
        //struct is_in : Utils::any_fulfills<typename Utils::is_in<Types ...>::test, Args ...> {};

        //template <class ... Types>
        //UTILS_CONSTEXPR_CXX17 static bool is_in_v = is_in<Types ...>::value;

        template <class Type>
        struct is_same_as : Utils::any_fulfills<typename Utils::is_same_as<Type>::test, Args ...> {};

        template <class Type>
        _INLINE_VAR constexpr static bool is_same_as_v = is_same_as<Type>::value;

        template <template <class> class _Test>
        struct fulfills : Utils::any_fulfills<_Test, Args ...> {};

        template <template <class> class _Test>
        _INLINE_VAR constexpr static bool fulfills_v = fulfills<_Test>::value;
    };

    template <class ... Types> struct is_in
    {
        template <class Arg>
        struct test : Utils::any_of<Types ...>::template is_same_as<Arg> {};

        template <class Arg>
        _INLINE_VAR constexpr static bool test_v = test<Arg>::value;
    };

    template <class ... Args>
    struct all_of
    {
        template <class ... Types>
        struct are_in : Utils::all_fulfill<typename Utils::is_in<Types ...>::test, Args ...> {};

        template <class ... Types>
        _INLINE_VAR constexpr static bool are_in_v = are_in<Types ...>::value;

        template <class Type>
        struct are : Utils::all_fulfill<typename Utils::is_same_as<Type>::test, Args ...> {};

        template <class Type>
        _INLINE_VAR constexpr static bool are_v = are<Type>::value;

        template <template <class> class _Test>
        struct fulfill : Utils::all_fulfill<_Test, Args ...> {};

        template <template <class> class _Test>
        _INLINE_VAR constexpr static bool fulfill_v = fulfill<_Test>::value;
    };

    template <class ... Calls>
    struct can_call : std::is_void<std::void_t<Calls ...>> {};

    template <class ... Calls>
    _INLINE_VAR constexpr bool can_call_v = can_call<Calls ...>::value;

    template <class Pointer>
    struct is_shared_pointer : std::bool_constant<
        can_call_v<typename Pointer::element_type> &&
        std::is_same_v<std::shared_ptr<typename Pointer::element_type>, Pointer>> {};

    template <class Pointer>
    _INLINE_VAR constexpr bool is_shared_pointer_v = is_shared_pointer<Pointer>::value;

    template <class Pointer>
    struct is_unique_pointer : std::bool_constant<
        can_call_v<typename Pointer::element_type, typename Pointer::deleter_type> &&
        std::is_same_v<std::unique_ptr<typename Pointer::element_type, typename Pointer::deleter_type>, Pointer>> {};

    template <class Pointer>
    _INLINE_VAR constexpr bool is_unique_pointer_v = is_unique_pointer<Pointer>::value;

    template <class Pointer>
    struct is_owning_pointer : std::bool_constant<
        is_unique_pointer_v<Pointer> || is_shared_pointer_v<Pointer> > {};

    template <class Pointer>
    _INLINE_VAR constexpr bool is_owning_pointer_v = is_owning_pointer<Pointer>::value;

#if _HAS_CXX20
    namespace Concepts
    {
        template <class Pointer>
        concept SharedPointer = is_shared_pointer_v<Pointer>;
        template <class Pointer>
        concept UniquePointer = is_shared_pointer_v<Pointer>;
        template <class Pointer>
        concept OwningPointer = is_owning_pointer_v<Pointer>;
        template<typename BaseType>
        concept DerivedFrom = requires (class DerivedType) {
            requires std::derived_from<DerivedType, BaseType>;
        };
    }
#endif // _HAS_CXX17
}
//
// Created by kiva on 2018/3/17.
//
#pragma once

#include <utility>
#include <type_traits>

namespace v9 {
    template<typename ...>
    using void_t = void;

    struct true_type {
        constexpr static bool value = true;
    };

    struct false_type {
        constexpr static bool value = false;
    };

    template<typename T>
    struct remove_reference {
        using type = T;
    };

    template<typename T>
    struct remove_reference<T &> {
        using type = T;
    };

    template<typename T>
    struct remove_reference<T &&> {
        using type = T;
    };

    template<typename T>
    using remove_reference_t = typename remove_reference<T>::type;

    template<typename T>
    struct remove_volatile {
        using type = T;
    };

    template<typename T>
    struct remove_volatile<volatile T> {
        using type = T;
    };

    template<typename T>
    using remove_volatile_t = typename remove_volatile<T>::type;

    template<typename From, typename To, typename = void_t<>>
    struct InstanceOf : public false_type {
    };

    template<typename From, typename To>
    struct InstanceOf
        <From, To,
            void_t<decltype(static_cast<To &&>(std::declval<From>()))>
        > : public true_type {
    };

    template<typename From, typename To>
    static inline constexpr bool InstanceOf_v = InstanceOf<From, To>::value;

    template<typename T, typename = void_t<>>
    struct OverloadsGT : public std::false_type {
    };

    template<typename T>
    struct OverloadsGT
        <T,
            void_t<decltype(std::declval<T>() > std::declval<T>())>
        > : public std::true_type {
    };

    template<typename T, typename = void_t<>>
    struct OverloadsGE : public std::false_type {
    };

    template<typename T>
    struct OverloadsGE
        <T,
            void_t<decltype(std::declval<T>() >= std::declval<T>())>
        > : public std::true_type {
    };

    template<typename T, typename = void_t<>>
    struct OverloadsLT : public std::false_type {
    };

    template<typename T>
    struct OverloadsLT
        <T,
            void_t<decltype(std::declval<T>() < std::declval<T>())>
        > : public std::true_type {
    };

    template<typename T, typename = void_t<>>
    struct OverloadsLE : public std::false_type {
    };

    template<typename T>
    struct OverloadsLE
        <T,
            void_t<decltype(std::declval<T>() <= std::declval<T>())>
        > : public std::true_type {
    };

    template<typename T, typename = void_t<>>
    struct OverloadsEQ : public std::false_type {
    };

    template<typename T>
    struct OverloadsEQ
        <T,
            void_t<decltype(std::declval<T>() == std::declval<T>())>
        > : public std::true_type {
    };

    template<typename T, typename = void_t<>>
    struct OverloadsNE : public std::false_type {
    };

    template<typename T>
    struct OverloadsNE
        <T,
            void_t<decltype(std::declval<T>() != std::declval<T>())>
        > : public std::true_type {
    };

    template<typename T>
    inline v9::remove_reference_t<T> &&move(T &&t) noexcept {
        typedef v9::remove_reference_t<T> U;
        return static_cast<U &&>(t);
    }

    template<typename T>
    void swap(T &lhs, T &rhs) {
        T t(v9::move(lhs));
        lhs = v9::move(rhs);
        rhs = v9::move(t);
    }
}

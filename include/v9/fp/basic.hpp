//
// Created by kiva on 2019/10/17.
//
#pragma once

#include <v9/bits/traits.hpp>

namespace v9::fp {
    namespace legacy {
        template<typename T, typename U, typename = void_t<>>
        struct CanAdd : public std::false_type {
            using type = void;
        };

        template<typename T, typename U>
        struct CanAdd
            <T, U,
                void_t<decltype(std::declval<T>() + std::declval<U>())>
            > : public std::true_type {
            using type = decltype(std::declval<T>() + std::declval<U>());
        };

        template<typename T, typename U, typename = void_t<>>
        struct CanSub : public std::false_type {
            using type = void;
        };

        template<typename T, typename U>
        struct CanSub
            <T, U,
                void_t<decltype(std::declval<T>() - std::declval<U>())>
            > : public std::true_type {
            using type = decltype(std::declval<T>() - std::declval<U>());
        };

        template<typename T, typename U, typename = void_t<>>
        struct CanMul : public std::false_type {
            using type = void;
        };

        template<typename T, typename U>
        struct CanMul
            <T, U,
                void_t<decltype(std::declval<T>() * std::declval<U>())>
            > : public std::true_type {
            using type = decltype(std::declval<T>() * std::declval<U>());
        };

        template<typename T, typename U, typename = void_t<>>
        struct CanDiv : public std::false_type {
            using type = void;
        };

        template<typename T, typename U>
        struct CanDiv
            <T, U,
                void_t<decltype(std::declval<T>() / std::declval<U>())>
            > : public std::true_type {
            using type = decltype(std::declval<T>() / std::declval<U>());
        };

        template<typename T, typename U>
        std::enable_if_t<CanAdd<T, U>::value, typename CanAdd<T, U>::type> add(T &&lhs, U &&rhs) {
            return lhs + rhs;
        }

        template<typename T, typename U>
        std::enable_if_t<CanSub<T, U>::value, typename CanSub<T, U>::type> sub(T &&lhs, U &&rhs) {
            return lhs - rhs;
        }

        template<typename T, typename U>
        std::enable_if_t<CanMul<T, U>::value, typename CanMul<T, U>::type> mul(T &&lhs, U &&rhs) {
            return lhs * rhs;
        }

        template<typename T, typename U>
        std::enable_if_t<CanDiv<T, U>::value, typename CanDiv<T, U>::type> div(T &&lhs, U &&rhs) {
            return lhs / rhs;
        }
    }

    using namespace legacy;
}

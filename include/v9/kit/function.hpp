#pragma once

#include <type_traits>
#include <v9/kit/typelist.hpp>

namespace v9::kit {
    /**
     * An alias for std::function. In case that we need to
     * use our own function implementation in the future.
     */
    template <typename T>
    using FunctionAlias = std::function<T>;

    /**
     * Remove all qualifiers from typenames.
     * e.g. `const std::string &` -> `std::string`
     *
     * @tparam Args typenames
     */
    template <typename ... Args>
    struct TypePurifier {
        template <typename T>
        using Purify = std::remove_const_t<std::remove_reference_t<std::remove_const_t<T>>>;

        template <typename ...>
        struct QualifierRemover;

        template <typename A, typename ... As>
        struct QualifierRemover<A, As...> {
            using RestList = typename QualifierRemover<As...>::Type;
            using Type = TypeList::cons<Purify<A>, RestList>;
        };

        template <>
        struct QualifierRemover<> {
            using Type = TypeList::Empty;
        };

        using Type = typename QualifierRemover<Args...>::Type;
    };

    /**
     * Parse function typename to return type and argument type(s).
     * @tparam F function typename
     */
    template <typename F>
    struct FunctionParser : public FunctionParser<decltype(&F::operator())> {
    };

    /**
     * Base condition of recursion, extracting all information from functions.
     *
     * @tparam Class function owner
     * @tparam R function return type
     * @tparam Args function argument type(s)
     */
    template <typename Class, typename R, typename... Args>
    struct FunctionParser<R(Class::*)(Args...) const> {
        using FunctionType = FunctionAlias<R(Args...)>;
        using ReturnType = R;
        using ClassType = Class;

        /**
         * Raw argument types, unmodified.
         */
        using ArgTypes = TypeList::List<Args...>;

        /**
         * Purified argument types, with all qualifiers removed.
         * @see TypePurifier
         */
        using PureArgsTypes = typename TypePurifier<Args...>::Type;
    };

    /**
     * Parse all function pointers, including global functions,
     * static class functions.
     * Note that, instance methods are not supported.
     *
     * @tparam P function pointer type
     */
    template <typename P>
    struct FunctionParser<P *> : public FunctionParser<FunctionAlias<P>> {
    };

    /**
     * Short for {@code typename FunctionParser<F>::FunctionType}
     */
    template <typename F>
    using FunctionType = typename FunctionParser<F>::FunctionType;

    /**
     * Convert callable things to function type (aka FunctionAlias).
     *
     * @see FunctionAlias
     * @tparam F Function typename
     * @param f function itself
     * @return FunctionAlias object
     */
    template <typename F>
    static FunctionType<F> makeFunction(F &f) {
        return static_cast<FunctionType<F>>(f);
    }
}


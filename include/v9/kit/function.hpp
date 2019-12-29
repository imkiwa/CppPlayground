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

    template <typename ... Args>
    struct ArgTypeInfo {
        /**
         * Raw argument types, unmodified.
         */
        using ArgTypes = TypeList::List<Args...>;

        /**
         * Purified argument types, with all qualifiers removed.
         * @see TypePurifier
         */
        using PureArgTypes = TypeList::List<std::decay_t<Args>...>;
    };

    /**
     * Parse callable object and lambdas.
     * @tparam F functor
     */
    template <typename F>
    struct FunctorParser : public FunctorParser<decltype(&F::operator())> {
    };

    /**
     * Base condition of recursive, extracting all information from a function.
     */
    template <typename Class, typename R, typename... Args>
    struct FunctorParser<R(Class::*)(Args...) const> : public ArgTypeInfo<Args...> {
        using FunctionType = FunctionAlias<R(Args...)>;
        using ReturnType = R;
        using ClassType = Class;
    };

    /**
     * Parse function typename to return type and argument type(s).
     * @tparam F function typename
     */
    template <typename F>
    struct FunctionParser : public FunctorParser<F> {
    };

    /**
     * Parse all function pointers, including
     * global functions, static class functions.
     */
    template <typename P>
    struct FunctionParser<P *> : public FunctionParser<P> {
    };

    /**
     * Parse function (pointer) lvalue references.
     */
    template <typename F>
    struct FunctionParser<F &> : public FunctionParser<F> {
    };

    /**
     * Parse function (pointer) rvalue references.
     */
    template <typename F>
    struct FunctionParser<F &&> : public FunctionParser<F> {
    };

    /**
     * Parse instance methods by converting signature to normal form.
     */
    template <typename Class, typename R, typename... Args>
    struct FunctionParser<R(Class::*)(Args...)> : public FunctionParser<R(Class &, Args...)> {
        using ClassType = Class;
    };

    /**
     * Parse const instance methods by converting signature to normal form.
     */
    template <typename Class, typename R, typename... Args>
    struct FunctionParser<R(Class::*)(Args...) const> : public FunctionParser<R(const Class &, Args...) const> {
        using ClassType = Class;
    };

    /**
     * Parse a normal function by converting signature to const normal form.
     */
    template <typename R, typename... Args>
    struct FunctionParser<R(Args...)> : public FunctionParser<R(Args...) const> {
    };

    /**
     * Base condition of recursion, extracting all information
     * from a normal function signature.
     */
    template <typename R, typename... Args>
    struct FunctionParser<R(Args...) const> : public ArgTypeInfo<Args...> {
        using FunctionType = FunctionAlias<R(Args...)>;
        using ReturnType = R;
    };

    /**
     * Short for {@code typename FunctionParser<F>::FunctionType}
     */
    template <typename F>
    using FunctionType = typename FunctionParser<F>::FunctionType;

    /**
     * Convert callable things to function type (aka FunctionAlias).
     * Sample usage: {@code auto f = makeFunction(sth); }
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


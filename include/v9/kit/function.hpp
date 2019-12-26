#pragma once

#include <type_traits>
#include <v9/kit/typelist.hpp>

namespace v9::kit {
    /**
     * An alias for function implementation.
     * In case that we need to use our own function
     * in the future.
     */
    template <typename T>
    using FunctionAlias = std::function<T>;

    template <typename ... Args>
    struct ArgTypePurifier {
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

    template <typename Handler>
    struct FunctionParser : public FunctionParser<decltype(&Handler::operator())> {
    };

    template <typename ClassType, typename R, typename... Args>
    struct FunctionParser<R(ClassType::*)(Args...) const> {
        using FunctionType = FunctionAlias<R(Args...)>;
        using ArgTypes = TypeList::List<Args...>;
        using PureArgsTypes = typename ArgTypePurifier<Args...>::Type;
    };

    template <typename Handler>
    using FunctionType = typename FunctionParser<Handler>::FunctionType;

    template <typename Handler>
    static FunctionType<Handler> makeFunction(Handler &cb) {
        return static_cast<FunctionType<Handler>>(cb);
    }
}


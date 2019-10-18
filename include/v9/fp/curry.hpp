//
// Created by kiva on 2019/10/13.
//
#pragma once

#include <v9/bits/traits.hpp>
#include <functional>

namespace v9::fp {
    namespace superior {
        template<typename F, typename = void_t<>>
        struct IsFullCurried : public false_type {
        };

        template<typename F>
        struct IsFullCurried
            <F,
                void_t<decltype(std::declval<F>()())>
            > : public true_type {
        };

        template<typename F>
        static auto curry(F &&f);

        template<bool>
        struct CurryImpl;

        template<>
        struct CurryImpl<false> {
            template<typename F>
            static decltype(auto) _curry(F &&f) {
                // given the code:
                // curry(f)(1)(2)(3)

                // return a function that takes the (1)
                return [=](auto &&arg) {
                    // bind arg to the first argument of f
                    // and return a new function that takes
                    // the rest of the arguments (2)(3)
                    return curry([=](auto &&...rest) {
                        // make the call
                        return f(arg, rest...);
                    });
                };
            }
        };

        template<>
        struct CurryImpl<true> {
            template<typename F>
            static decltype(auto) _curry(F &&f) {
                return f;
            }
        };

        template<typename F>
        static auto curry(F &&f) {
            return CurryImpl<IsFullCurried<F>::value>::_curry(f);
        }
    }

    namespace legacy {
        template<typename F, typename = void_t<>>
        struct IsFullCurried : public false_type {
        };

        template<typename F>
        struct IsFullCurried
            <F,
                void_t<decltype(std::declval<F>()())>
            > : public true_type {
        };

        template<typename F>
        struct FunctionParser;

        template<typename C, typename R, typename ...Args>
        struct FunctionParser<R(C::*)(Args...) const> {
            using type = std::function<R(Args...)>;
            using return_type = R;
        };

        template<typename F>
        using FunctionType = typename FunctionParser<decltype(&F::operator())>::type;

        template<typename F>
        struct CurriedFunction;

        template<typename R, typename Arg>
        struct CurriedFunction<R(Arg)> {
            using type = std::function<R(Arg)>;
        };

        template<typename R, typename Arg1, typename ...Args>
        struct CurriedFunction<R(Arg1, Args...)> {
            using reduced_type = typename CurriedFunction<R(Args...)>::type;
            using type = std::function<reduced_type(Arg1)>;
        };

        template<typename F>
        static auto curry(F &&f) -> decltype(curry(FunctionType<F>(std::forward<F>(f))));

        template<typename R>
        static std::function<R()> curry(std::function<R()> &&f);

        template<typename R, typename Arg>
        static std::function<R(Arg)> curry(std::function<R(Arg)> &&f);

        template<typename R, typename Arg, typename ...ArgsT>
        static typename CurriedFunction<R(Arg, ArgsT...)>::type curry(std::function<R(Arg, ArgsT...)> &&f) {
            // given the code:
            // curry(f)(1)(2)(3)
            // which is equal to f(1, 2, 3)

            // return a function that takes the (1)
            return [=](Arg &&arg) {
                // bind arg to the first argument of f
                // and return a new function that takes
                // the rest of the arguments (2)(3)
                return curry<R, ArgsT...>(std::function<R(ArgsT...)>(
                    [=](ArgsT &&...rest) {
                        // make the call
                        return f(arg, std::forward<ArgsT>(rest)...);
                    }));
            };
        }

        template<typename R>
        static std::function<R()> curry(std::function<R()> &&f) {
            return f;
        }

        template<typename R, typename Arg>
        static std::function<R(Arg)> curry(std::function<R(Arg)> &&f) {
            return f;
        }

        template<typename F>
        static decltype(auto) curry(F &&f) {
            return curry(FunctionType<F>(std::forward<F>(f)));
        }
    }

    using namespace legacy;
}

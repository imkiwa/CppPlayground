//
// Created by kiva on 2019/12/29.
//

#pragma once

#include <cstdio>
#include <v9/kit/function.hpp>
#include <v9/kit/tuple.hpp>

namespace v9::kit {
    struct Placeholders {
        template <size_t N>
        struct Index {
            static constexpr size_t value = N;
        };

        static constexpr auto _1 = Index<1>();
        static constexpr auto _2 = Index<2>();
        static constexpr auto _3 = Index<3>();
        static constexpr auto _4 = Index<4>();
        static constexpr auto _5 = Index<5>();
    };

    template <typename, typename, typename, typename ...>
    class Bind;

    template <typename F, typename R, typename ... Args, typename ... Ps>
    class Bind<F, R, TypeList::List<Args...>, TypeList::List<Ps...>> {
        F _f;
        Tuple<Ps...> _withPlaceholders;

    private:
        template <typename, typename>
        struct Binder;

        template <typename T, typename ...Ts, typename ... As>
        struct Binder<Tuple<T, Ts...>, Tuple<As...>> {
            static Tuple<T, Ts...> doit(const Tuple<T, Ts...> &placeholders,
                                        const Tuple<As...> &args) {
                auto rest = Binder<Tuple<Ts ...>, Tuple<As ...>>::doit(
                    placeholders.tail(), args);
                return rest.cons(placeholders.head());
            }
        };

        template <typename A, typename ...Ts, typename ... As, size_t N>
        struct Binder<Tuple<Placeholders::Index<N>, Ts...>, Tuple<A, As...>> {
            static Tuple<A, Ts...> doit(const Tuple<const Placeholders::Index<N> &, Ts...> &placeholders,
                                        const Tuple<A, As...> &args) {
                auto rest = Binder<Tuple<Ts ...>, Tuple<As ...>>::doit(
                    placeholders.tail(), args.tail());
                return rest.cons(args.template get<N - 1>());
            }
        };

        template <>
        struct Binder<Tuple<>, Tuple<>> {
            static Tuple<> doit(const Tuple<> &, const Tuple<> &) {
                return makeTuple();
            }
        };

        template <typename, typename, typename>
        struct Invoker;

        template <typename ... Us>
        struct Invoker<F, R, Tuple<Us...>> {
            template <typename>
            struct Impl;

            template <size_t ... Is>
            struct Impl<StaticList::List<size_t, Is...>> {
                static R doit(F &&f, const Tuple<Us...> &tuple) {
                    return f(tuple.template get<Is>()...);
                }
            };

            static R doit(F &&f, const Tuple<Us...> &tuple) {
                return Impl<makeTupleIndices<Us...>>::doit(std::forward<F>(f), tuple);
            }
        };

    public:
        explicit Bind(F &&f, Ps &&...ps)
            : _f(f), _withPlaceholders(std::forward<Ps>(ps)...) {
        }

        template <typename ... As>
        R operator()(As &&...as) {
            auto placement = makeTuple(std::forward<As>(as)...);
            auto full = Binder<Tuple<std::decay_t<Ps>...>, Tuple<As ...>>::doit(_withPlaceholders, placement);
            return Invoker<F, R, decltype(full)>::doit(_f, full);
        }

        R operator()() {
            return Invoker<F, R, decltype(_withPlaceholders)>::doit(_f, _withPlaceholders);
        }
    };

    template <typename F, typename ... Ps>
    auto bind(F &&f, Ps &&... ps) {
        using R = typename FunctionParser<F>::ReturnType;
        using A = typename FunctionParser<F>::PureArgTypes;
        return Bind<F, R, A, TypeList::List<Ps...>>(f, std::forward<Ps>(ps)...);
    }
}

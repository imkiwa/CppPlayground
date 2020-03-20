//
// Created by kiva on 2019/12/27.
//

#pragma once

#include <cstdio>
#include <type_traits>

namespace v9::kit {
    namespace cmp {
        template <typename T, T pivot>
        struct Lt {
            template <T v>
            static constexpr bool value = v < pivot;
        };

        template <typename T, T pivot>
        struct Eq {
            template <T v>
            static constexpr bool value = v == pivot;
        };

        template <typename T, T pivot>
        struct Gt {
            template <T v>
            static constexpr bool value = v > pivot;
        };

        template <typename T, T pivot>
        struct Le {
            template <T v>
            static constexpr bool value = v <= pivot;
        };

        template <typename T, T pivot>
        struct Ge {
            template <T v>
            static constexpr bool value = v >= pivot;
        };
    }

    struct StaticList {
    private:
        template <typename Seq>
        struct Size;

        template <typename Seq>
        struct ElementType;

        template <typename Seq>
        struct Head;

        template <typename Seq>
        struct Tail;

        template <typename SeqL, typename SeqR>
        struct Concat;

        template <typename Const, typename Seq>
        struct Cons;

        template <typename Seq, typename Const>
        struct Snoc;

        template <typename Seq, typename F>
        struct Filter;

        template <typename Seq>
        struct Qsort;

        template <typename Seq, size_t Index>
        struct Visit {
            using type = typename Visit<typename Tail<Seq>::type, Index - 1>::type;
        };

        template <typename Seq>
        struct Visit<Seq, 0> {
            using type = typename Head<Seq>::type;
        };

    public:
        template <typename T, T ...>
        struct List {};

        template <typename T>
        using Empty = List<T>;

        template <typename T, T Const>
        struct Constant {
            static constexpr T value = Const;
        };

        template <typename Seq>
        using size = typename Size<Seq>::type;

        template <typename Seq>
        using elementType = typename ElementType<Seq>::type;

        template <typename Seq>
        using head = typename Head<Seq>::type;

        template <typename Seq>
        using tail = typename Tail<Seq>::type;

        template <typename SeqL, typename SeqR>
        using concat = typename Concat<SeqL, SeqR>::type;

        template <typename Const, typename Seq>
        using cons = typename Cons<Const, Seq>::type;

        template <typename Seq, typename Const>
        using snoc = typename Snoc<Seq, Const>::type;

        template <typename Seq, size_t Index>
        using visit = typename Visit<Seq, Index>::type;

        template <typename Seq>
        using empty = std::is_same<size<Seq>, std::integral_constant<size_t, 0>>;

        template <typename SeqL, typename SeqR>
        using equals = std::is_same<SeqL, SeqR>;

        template <typename Seq, typename F>
        using filter = typename Filter<Seq, F>::type;

        template <typename Seq>
        using qsort = typename Qsort<Seq>::type;

        template <typename Seq>
        static constexpr size_t size_v = size<Seq>::value;

        template <typename Seq>
        static constexpr bool empty_v = empty<Seq>::value;

        template <typename SeqL, typename SeqR>
        static constexpr bool equals_v = equals<SeqL, SeqR>::value;

    private:
        template <typename T, template <typename, T...> typename Seq, T...ts>
        struct Size<Seq<T, ts...>> {
            using type = std::integral_constant<size_t, sizeof...(ts)>;
        };

        template <typename T, template <typename, T...> typename Seq, T...ts>
        struct ElementType<Seq<T, ts...>> {
            using type = T;
        };

        template <typename T, template <typename, T...> typename Seq, T t, T...ts>
        struct Head<Seq<T, t, ts...>> {
            using type = Constant<T, t>;
        };

        template <typename T, template <typename, T...> typename Seq, T t, T...ts>
        struct Tail<Seq<T, t, ts...>> {
            using type = Seq<T, ts...>;
        };

        template <typename T, template <typename, T...> typename Seq, T...ts1, T ...ts2>
        struct Concat<Seq<T, ts1...>, Seq<T, ts2...>> {
            using type = Seq<T, ts1..., ts2...>;
        };

        template <typename T,
            template <typename, T...> typename Seq,
            template <typename, T> typename Const,
            T t, T ...ts>
        struct Cons<Const<T, t>, Seq<T, ts...>> {
            using type = Seq<T, t, ts...>;
        };

        template <typename T,
            template <typename, T...> typename Seq,
            template <typename, T> typename Const,
            T t, T ...ts>
        struct Snoc<Seq<T, ts...>, Const<T, t>> {
            using type = Seq<T, ts..., t>;
        };

        template <typename T,
            template <typename, T...> typename Seq,
            typename F,
            T t, T...ts>
        struct Filter<Seq<T, t, ts...>, F> {
            using rest = filter<Seq<T, ts...>, F>;

            template <bool>
            struct FilterImpl;

            template <>
            struct FilterImpl<true> {
                using type = cons<Constant<T, t>, rest>;
            };

            template <>
            struct FilterImpl<false> {
                using type = rest;
            };

            using type = typename FilterImpl<F::template value<t>>::type;
        };

        template <typename T,
            template <typename, T...> typename Seq,
            typename F>
        struct Filter<Seq<T>, F> {
            using type = Seq<T>;
        };

        template <typename T, template <typename, T...> typename Seq, T pivot, T...ts>
        struct Qsort<Seq<T, pivot, ts...>> {
            using data = Seq<T, ts...>;
            using filterL = cmp::Le<T, pivot>;
            using filterG = cmp::Gt<T, pivot>;

            using smallers = filter<data, filterL>;
            using greaters = filter<data, filterG>;

            using type = concat<qsort<smallers>,
                cons<Constant<T, pivot>, qsort<greaters>>>;
        };

        template <typename T, template <typename, T...> typename Seq, T t>
        struct Qsort<Seq<T, t>> {
            using type = Seq<T, t>;
        };

        template <typename T, template <typename, T...> typename Seq>
        struct Qsort<Seq<T>> {
            using type = Seq<T>;
        };
    };
}

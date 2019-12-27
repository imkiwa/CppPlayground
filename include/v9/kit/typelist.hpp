//
// Created by kiva on 2019/12/25.
//

#pragma once

#include <type_traits>

namespace v9::kit {
    struct TypeList {
    private:
        template <typename Seq, typename T>
        struct Cons;

        template <typename Seq>
        struct Head;

        template <typename Seq>
        struct Tail;

        template <typename SeqL, typename SeqR>
        struct Concat;

        template <typename Seq>
        struct Size;

        template <typename T, template <typename...> class Seq, typename... Ts>
        struct Cons<T, Seq<Ts...>> {
            using type = Seq<T, Ts...>;
        };

        template <template <typename...> class Seq, typename T, typename... Ts>
        struct Head<Seq<T, Ts...>> {
            using type = T;
        };

        template <template <typename...> class Seq, typename T, typename... Ts>
        struct Tail<Seq<T, Ts...>> {
            using type = Seq<Ts...>;
        };

        template <template <typename...> class Seq, typename... Ts, typename ... Us>
        struct Concat<Seq<Ts...>, Seq<Us ...>> {
            using type = Seq<Ts..., Us...>;
        };

        template <template <typename...> typename Seq, typename... Ts>
        struct Size<Seq<Ts...>> {
            using type = std::integral_constant<std::size_t, sizeof...(Ts)>;
        };

    public:
        template <typename ...>
        struct List {};

        using Empty = List<>;

        template <typename T, typename Seq>
        using cons = typename Cons<T, Seq>::type;

        template <typename Seq>
        using head = typename Head<Seq>::type;

        template <typename Seq>
        using tail = typename Tail<Seq>::type;

        template <typename SeqL, typename SeqR>
        using concat = typename Concat<SeqL, SeqR>::type;

        template <typename Seq>
        using size = typename Size<Seq>::type;

        template <typename Seq>
        static constexpr size_t size_v = size<Seq>::value;

        template <typename Seq>
        using empty = std::is_same<size<Seq>, std::integral_constant<size_t, 0>>;

        template <typename Seq>
        static constexpr bool empty_v = empty<Seq>::value;

        template <typename SeqL, typename SeqR>
        using equals = std::is_same<SeqL, SeqR>;

        template <typename SeqL, typename SeqR>
        static constexpr bool equals_v = equals<SeqL, SeqR>::value;
    };
}

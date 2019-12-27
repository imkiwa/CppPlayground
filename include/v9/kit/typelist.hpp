//
// Created by kiva on 2019/12/25.
//

#pragma once

#include <type_traits>

namespace v9::kit {
    struct TypeList {
    private:
        template <typename Seq, typename T>
        struct TypeListCons;

        template <typename Seq>
        struct TypeListHead;

        template <typename Seq>
        struct TypeListTail;

        template <typename SeqL, typename SeqR>
        struct TypeListConcat;

        template <typename Seq>
        struct TypeListSize;

        template <typename T, template <typename...> class Seq, typename... Ts>
        struct TypeListCons<T, Seq<Ts...>> {
            using type = Seq<T, Ts...>;
        };

        template <template <typename...> class Seq, typename T, typename... Ts>
        struct TypeListHead<Seq<T, Ts...>> {
            using type = T;
        };

        template <template <typename...> class Seq, typename T, typename... Ts>
        struct TypeListTail<Seq<T, Ts...>> {
            using type = Seq<Ts...>;
        };

        template <template <typename...> class Seq, typename... Ts, typename ... Us>
        struct TypeListConcat<Seq<Ts...>, Seq<Us ...>> {
            using type = Seq<Ts..., Us...>;
        };

        template <template <typename...> typename Seq, typename... Ts>
        struct TypeListSize<Seq<Ts...>> {
            using type = std::integral_constant<std::size_t, sizeof...(Ts)>;
        };

    public:
        template <typename ...>
        struct List {};

        using Empty = List<>;

        template <typename T, typename Seq>
        using cons = typename TypeListCons<T, Seq>::type;

        template <typename Seq>
        using head = typename TypeListHead<Seq>::type;

        template <typename Seq>
        using tail = typename TypeListTail<Seq>::type;

        template <typename SeqL, typename SeqR>
        using concat = typename TypeListConcat<SeqL, SeqR>::type;

        template <typename Seq>
        using size = typename TypeListSize<Seq>::type;

        template <typename SeqL, typename SeqR>
        using equals = std::is_same<SeqL, SeqR>;
    };
}

//
// Created by kiva on 2019/12/29.
//

#pragma once

#include <v9/kit/typelist.hpp>
#include <v9/kit/staticlist.hpp>

namespace v9::kit {
    template <typename ... Ts>
    class Tuple {
    private:
        using ElementTypes = TypeList::List<Ts...>;

        template <typename Tp>
        using TupleSize = TypeList::size<Tp>;

        template <typename Tp, size_t Index>
        using ElementType = TypeList::visit<Tp, Index>;

        template <typename ... Us>
        struct Indexer;

        template <typename U, typename ... Us>
        struct Indexer<U, Us...> {
            using rest = typename Indexer<Us...>::type;
            using currentIndex = StaticList::size<rest>;

            using type = StaticList::snoc<rest,
                StaticList::Constant<size_t, currentIndex::value>>;
        };

        template <>
        struct Indexer<> {
            using type = StaticList::Empty<size_t>;
        };

        template <typename ... Seq>
        using makeIndices = typename Indexer<Seq...>::type;

    private:
        template <size_t Index, typename T>
        struct TupleSlot {
            T _value;

            explicit TupleSlot(T &&t)
                : _value(t) {
            }

            T getValue() {
                return _value;
            }
        };

        template <typename Seq>
        struct TupleData;

        template <size_t ... Is>
        struct TupleData<StaticList::List<size_t, Is...>> : public TupleSlot<Is, Ts> ... {
            explicit TupleData(Ts &&...ts)
                : TupleSlot<Is, Ts>{std::forward<Ts>(ts)}... {
            }

            template <size_t Index>
            ElementType<ElementTypes, Index> get() {
                return TupleSlot<Index, ElementType<ElementTypes, Index>>::getValue();
            }
        };

    private:
        TupleData<makeIndices<Ts...>> _data;

    public:
        explicit Tuple(Ts &&... ts)
            : _data(std::forward<Ts>(ts)...) {
        }

        template <size_t Index>
        ElementType<ElementTypes, Index> get() {
            return _data.template get<Index>();
        }
    };

    template <typename ... Ts>
    constexpr Tuple<std::decay_t<Ts>...> makeTuple(Ts &&...ts) {
        return Tuple<std::decay_t<Ts>...>{std::forward<Ts>(ts)...};
    }
}

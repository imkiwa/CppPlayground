//
// Created by kiva on 2019/12/29.
//

#pragma once

#include <v9/kit/typelist.hpp>
#include <v9/kit/staticlist.hpp>
#include <utility>

namespace v9::kit {
    template <typename>
    struct TupleTail;

    template <typename, typename>
    struct TupleCons;

    template <typename ... Us>
    struct TupleIndexer;

    template <typename U, typename ... Us>
    struct TupleIndexer<U, Us...> {
        using rest = typename TupleIndexer<Us...>::type;
        using currentIndex = StaticList::size<rest>;

        using type = StaticList::snoc<rest,
            StaticList::Constant<size_t, currentIndex::value>>;
    };

    template <>
    struct TupleIndexer<> {
        using type = StaticList::Empty<size_t>;
    };

    template <typename ... Seq>
    using makeTupleIndices = typename TupleIndexer<Seq...>::type;

    template <typename ... Ts>
    class Tuple {
    private:
        using ElementTypes = TypeList::List<Ts...>;

        template <typename Tp>
        using TupleSize = TypeList::size<Tp>;

        template <typename Tp, size_t Index>
        using ElementType = TypeList::visit<Tp, Index>;

    private:
        template <size_t Index, typename T>
        struct TupleSlot {
            T _value;

            explicit TupleSlot(T &&t)
                : _value(t) {
            }

            T &getValue() {
                return _value;
            }

            const T &getValue() const {
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
            ElementType<ElementTypes, Index> get() const {
                return TupleSlot<Index, ElementType<ElementTypes, Index>>::getValue();
            }
        };

    private:
        TupleData<makeTupleIndices<Ts...>> _data;

    public:
        explicit Tuple(Ts &&... ts)
            : _data(std::forward<Ts>(ts)...) {
        }

        template <size_t Index>
        ElementType<ElementTypes, Index> get() const {
            return _data.template get<Index>();
        }

        auto tail() const {
            return TupleTail<ElementTypes>::doit(this);
        }

        auto head() const {
            return get<0>();
        }

        template <typename N>
        auto cons(N &&n) const {
            return TupleCons<std::decay_t<N>, ElementTypes>::doit(this, std::forward<N>(n));
        }
    };

    template <typename ... Ts>
    constexpr Tuple<std::decay_t<Ts>...> makeTuple(Ts &&...ts) {
        return Tuple<std::decay_t<Ts>...>{std::forward<Ts>(ts)...};
    }

    template <typename U, typename ... Us>
    struct TupleTail<TypeList::List<U, Us...>> {
        template <typename>
        struct Impl;

        template <size_t ... Is>
        struct Impl<StaticList::List<size_t, Is...>> {
            static Tuple<Us...> doit(const Tuple<U, Us...> *tuple) {
                return makeTuple(tuple->template get<Is + 1>()...);
            }
        };

        static Tuple<Us...> doit(const Tuple<U, Us...> *tuple) {
            return Impl<makeTupleIndices<Us...>>::doit(tuple);
        }
    };

    template <typename N, typename ... Us>
    struct TupleCons<N, TypeList::List<Us...>> {
        template <typename>
        struct Impl;

        template <size_t ... Is>
        struct Impl<StaticList::List<size_t, Is...>> {
            static Tuple<N, Us...> doit(const Tuple<Us...> *tuple, N &&n) {
                return makeTuple(std::forward<N>(n), tuple->template get<Is>()...);
            }
        };

        static Tuple<N, Us...> doit(const Tuple<Us...> *tuple, N &&n) {
            return Impl<makeTupleIndices<Us...>>::doit(tuple, std::forward<N>(n));
        }
    };
}

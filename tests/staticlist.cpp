//
// Created by kiva on 2019/12/27.
//
#include <cstdio>
#include <v9/kit/staticlist.hpp>

int main() {
    using namespace v9::kit;

    using t1 = StaticList::List<int, 1, 2, 3>;
    using t2 = StaticList::List<char, 'a', 'b', 'c', 'd'>;
    static_assert(StaticList::size_v<t1> == 3, "You wrote a bug");
    static_assert(StaticList::size_v<t2> == 4, "You wrote a bug");

    static_assert(StaticList::head<t1>::value == 1, "You wrote a bug");
    static_assert(StaticList::visit<t1, 2>::value == 3, "You wrote a bug");

    using d0 = StaticList::List<int, 2, 3, 9, 1, 8, 8>;
    using d1 = StaticList::qsort<d0>;

    using expected = StaticList::List<int, 1, 2, 3, 8, 8, 9>;
    static_assert(StaticList::equals<d1, expected>::value,
        "You wrote a bug");
}

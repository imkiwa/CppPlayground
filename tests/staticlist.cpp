//
// Created by kiva on 2019/12/27.
//
#include <cstdio>
#include <v9/kit/staticlist.hpp>

int main() {
    using namespace v9::kit;

    using t1 = StaticList::List<int, 1, 2, 3>;
    using t2 = StaticList::List<char, 'a', 'b', 'c', 'd'>;
    printf("%zd\n", StaticList::size_v<t1>);
    printf("%zd\n", StaticList::size_v<t2>);

    printf("t1[0]: %d\n", StaticList::head<t1>::value);
    printf("t1[2]: %d\n", StaticList::visit<t1, 2>::value);

    using d0 = StaticList::List<int, 2, 3, 9, 1, 8, 8>;
    using d1 = StaticList::qsort<d0>;

    using expected = StaticList::List<int, 1, 2, 3, 8, 8, 9>;

    static_assert(StaticList::equals<d1, expected>::value,
        "You wrote a bug");
}

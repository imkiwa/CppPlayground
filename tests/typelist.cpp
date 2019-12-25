//
// Created by kiva on 2019/12/25.
//
#include <cstdio>
#include <string>
#include <v9/kit/typelist.hpp>

int main() {
    using namespace v9::kit;

    using t1 = TypeList::List<int, double, float, char>;
    using size1 = TypeList::size<t1>;
    using size2 = TypeList::size<TypeList::Empty>;

    static_assert(size1::value == 4, "You wrote a bug");
    static_assert(size2::value == 0, "You wrote a bug");

    using e1 = TypeList::Empty;
    using e2 = TypeList::cons<char, e1>;
    using e3 = TypeList::cons<t1, e2>;

    static_assert(TypeList::size<e1>::value == 0, "You wrote a bug");
    static_assert(TypeList::size<e2>::value == 1, "You wrote a bug");
    static_assert(TypeList::size<e3>::value == 2, "You wrote a bug");

    using e3t = TypeList::tail<e3>;
    using e3h = TypeList::head<e3>;

    static_assert(TypeList::equals<e3t, e2>::value, "You wrote a bug");
    static_assert(TypeList::equals<e3h, t1>::value, "You wrote a bug");
}

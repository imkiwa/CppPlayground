//
// Created by kiva on 2019/12/25.
//
#include <cstdio>
#include <cxxabi.h>
#include <cstdlib>
#include <typeinfo>

#include <v9/kit/typelist.hpp>

using namespace v9::kit;

template <int From, int To>
struct Move {
};

template <int N, int From, int To, int Via>
struct Hanoi {
    using type = TypeList::concat<TypeList::concat<
        typename Hanoi<N - 1, From, Via, To>::type,
        TypeList::List<Move<From, To>>>,
        typename Hanoi<N - 1, Via, To, From>::type>;
};

template <int From, int To, int Via>
struct Hanoi<1, From, To, Via> {
    using type = TypeList::List<Move<From, To>>;
};

template <typename T>
struct show {
    show() {
        char *s = abi::__cxa_demangle(typeid(T).name(),
            nullptr, nullptr, nullptr);
        printf("T = %s\n", s);
        std::free(s);
    }
};

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

    using c1 = TypeList::List<int, char>;
    using c2 = TypeList::List<double, float>;
    using c3 = TypeList::List<int, char, double, float>;

    static_assert(TypeList::equals<c3, TypeList::concat<c1, c2>>::value,
        "You wrote a bug");
    static_assert(TypeList::equals<c1, TypeList::concat<c1, TypeList::Empty>>::value,
        "You wrote a bug");
    static_assert(TypeList::equals<c2, TypeList::concat<TypeList::Empty, c2>>::value,
        "You wrote a bug");

    show<Hanoi<3, 1, 2, 3>::type>();
}

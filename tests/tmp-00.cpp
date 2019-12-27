//
// Created by kiva on 2019/12/27.
//

#include <cstdio>

template <size_t A, size_t B>
struct Add {
    static constexpr size_t result = Add<A + 1, B - 1>::result;
};

//template <size_t A>
//struct Add<A, 0> {
//    static constexpr size_t result = A;
//};

int main() {
    static_assert(Add<1, 2>::result == 3, "You wrote a bug");
    static_assert(Add<50, 99>::result == 149, "You wrote a bug");
}

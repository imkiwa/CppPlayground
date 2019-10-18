//
// Created by kiva on 2019/10/13.
//

#include <cstdio>
#include <v9/fp/curry.hpp>

int main(int argc, const char **argv) {
    using namespace v9::fp;

    auto fn1 = curry([]() { return 10086; });
    auto fn2 = curry([](int a, int b, int c) { return a + b + c; });
    static_assert(IsFullCurried<decltype(fn1)>::value, "You wrote a bug");

    printf("%d\n", fn1());
    printf("%d\n", fn2(1)(2)(3));

    auto add10 = fn2(2)(8);
    static_assert(!IsFullCurried<decltype(add10)>::value, "You wrote a bug");

    printf("%d\n", add10(6));
}

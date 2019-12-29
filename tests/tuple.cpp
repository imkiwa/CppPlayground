//
// Created by kiva on 2019/12/25.
//
#include <cstdio>
#include <v9/kit/tuple.hpp>

#include <cxxabi.h>
#include <cstdlib>
#include <typeinfo>

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

    auto t = makeTuple(1, 2, 3, "fuck");

    show<decltype(t)>();

    printf("%d\n", t.get<0>());
    printf("%d\n", t.get<1>());
    printf("%d\n", t.get<2>());
    printf("%s\n", t.get<3>());
}

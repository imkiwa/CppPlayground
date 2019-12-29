//
// Created by kiva on 2019/12/25.
//
#include <cstdio>
#include <v9/kit/bind.hpp>

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

    auto L = [](int x, int y) { return x + y; };

    // f == [](int x) { return L(x, 10); }
    auto f = bind(L, Placeholders::_1, 10);
    printf("%d\n", f(99));
}

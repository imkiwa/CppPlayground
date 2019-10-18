//
// Created by kiva on 2019/10/13.
//

#include <cstdio>
#include <v9/bits/traits.hpp>

using namespace v9;

struct Base {};
struct Derived : public Base {};
struct Nothing {};

int main(int argc, const char **argv) {
    static_assert(InstanceOf<Derived, Base>::value, "You wrote a bug");
    static_assert(InstanceOf<Derived, Derived>::value, "You wrote a bug");
    static_assert(!InstanceOf<Derived, Nothing>::value, "You wrote a bug");
    static_assert(!InstanceOf<Base, Nothing>::value, "You wrote a bug");
}

//
// Created by kiva on 2019/9/13.
//
#include <v9/memory/delegate.h>
#include <cstdio>
#include <cassert>

int main() {
    struct X {
        int x;
        int y;
        char s[32];
        double d;
    };

    using namespace v9::memory;

    assert(offsetOf(&X::x) == 0);
    assert(offsetOf(&X::y) == sizeof(int));
    assert(offsetOf(&X::d) == sizeof(int) + sizeof(int) + sizeof(char[32]));

    free(malloc(22));
    auto mx = static_cast<unsigned char *>(malloc(sizeof(int)));
    free(malloc(200));
    auto my = static_cast<unsigned char *>(malloc(sizeof(int)));
    free(malloc(20));
    auto ms = static_cast<unsigned char *>(malloc(sizeof(char[32])));
    free(malloc(112));
    auto md = static_cast<unsigned char *>(malloc(sizeof(double)));

    printf("mx: %p\n", mx);
    printf("my: %p\n", my);
    printf("ms: %p\n", ms);
    printf("md: %p\n", md);

    Block blocks[] = {{mx, sizeof(int)},
                      {my, sizeof(int)},
                      {ms, sizeof(char[32])},
                      {md, sizeof(double)}};
    Delegate<X> delegate(blocks, 4);

    {
        using TypeOfY = decltype(delegate.F(&X::y));
        TypeOfY s1 = delegate.F(&X::y);
        printf("addr  y: %p\n", s1);

        *s1 = 10086;

        TypeOfY s2 = delegate.F(&X::y);
        printf("addr  y: %p\n", s2);

        assert(s1 == s2);
        assert(*s1 == *s2);
    }

    {
        using TypeOfD = decltype(delegate.F(&X::d));
        TypeOfD s1 = delegate.F(&X::d);
        printf("addr  d: %p\n", s1);

        *s1 = 3.14;

        TypeOfD s2 = delegate.F(&X::d);
        printf("addr  d: %p\n", s2);

        assert(s1 == s2);
        assert(*s1 == *s2);
    }



//
//    *s = 10086;
//
//    printf("mx: %d\n", *reinterpret_cast<int *>(mx));
//    printf("my: %d\n", *reinterpret_cast<int *>(my));

    // TODO: fix
//    assert(offsetOf(&X::s) == sizeof(int) + sizeof(int));
//    using T = decltype(fieldType(&X::s));
}


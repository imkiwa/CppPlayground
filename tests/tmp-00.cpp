//
// Created by kiva on 2019/12/27.
//

#include <cstdio>
#include <typeinfo>
#include <functional>
#include <cxxabi.h>
#include <type_traits>

template <typename T>
struct show {
    show() {
        char *s = abi::__cxa_demangle(typeid(T).name(),
            nullptr, nullptr, nullptr);
        printf("T = %s\n", s);
        std::free(s);
    }
};

template <typename T>
using FunctionAlias = std::function<T>;

namespace details {
    template <typename F>
    struct FunctorParser : public FunctorParser<decltype(&F::operator())> {
    };

    template <typename Class, typename R, typename... Args>
    struct FunctorParser<R(Class::*)(Args...) const> {
        using FunctionType = FunctionAlias<R(Args...)>;
        using ReturnType = R;
        using ClassType = Class;
    };

    template <typename F>
    struct FunctionParser : FunctorParser<F> {
    };

    template <typename P>
    struct FunctionParser<P *> : public FunctionParser<P> {
    };

    template <typename P>
    struct FunctionParser<P &> : public FunctionParser<P> {
    };

    template <typename P>
    struct FunctionParser<P &&> : public FunctionParser<P> {
    };

    template <typename Class, typename R, typename... Args>
    struct FunctionParser<R(Class::*)(Args...)> : public FunctionParser<R(Class::*)(Args...) const> {
    };

    template <typename R, typename... Args>
    struct FunctionParser<R(Args...)> : public FunctionParser<R(Args...) const> {
    };

    template <typename Class, typename R, typename... Args>
    struct FunctionParser<R(Class::*)(Args...) const> : public FunctionParser<R(Class *, Args...) const> {
        using ClassType = Class;
    };

    template <typename R, typename... Args>
    struct FunctionParser<R(Args...) const> {
        using FunctionType = FunctionAlias<R(Args...)>;
        using ReturnType = R;
    };
}

template <typename F>
using FunctionType = typename details::FunctionParser<F>::FunctionType;

template <typename F>
static FunctionType<F> makeFunction(F f) {
    return static_cast<FunctionType<F>>(f);
}

struct Fun {
    Fun() = default;

    ~Fun() = default;

    void hi(int, short, char) { printf("Support member methods\n"); }

    void hi_const(char, short, int) const { printf("Support const member methods\n"); }

    static void say() { printf("Support static class methods\n"); }

    void operator()() { printf("Support operator()\n"); }
};

void fptr(Fun, Fun) { printf("Support function pointers\n"); }

int main() {
    Fun fun;
    auto fm = makeFunction(&Fun::hi);
    fm(&fun, 0, 0, '0');

    auto fc = makeFunction(&Fun::hi_const);
    fc(&fun, '0', 0, 0);

    auto fs = makeFunction(&Fun::say);
    fs();

    auto fp = makeFunction(fptr);
    fp(fun, fun);

    auto ff = makeFunction(&Fun::operator());
    ff(&fun);

    auto fl = makeFunction([]() { printf("Support lambda"); });
    fl();
}

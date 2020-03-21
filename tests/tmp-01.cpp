//
// Created by kiva on 2019/12/27.
//

#include <cstdio>
#include <cxxabi.h>
#include <typeindex>
#include <memory>

template <typename T>
struct show {
    show() {
        char *s = abi::__cxa_demangle(typeid(T).name(),
            nullptr, nullptr, nullptr);
        printf("%s\n", s);
        std::free(s);
    }
};

template <char ...>
struct chars {};

template <typename T, char c>
struct chars_push;

template <char c, char ...cs>
struct chars_push<chars<cs...>, c> {
    using type = chars<c, cs...>;
};

template <typename T>
struct chars_to_string {};

template <char ...cs>
struct chars_to_string<chars<cs...>> {
    static constexpr char value[sizeof...(cs)] = {cs..., '\0'};
};

template <int N, int Base = 10>
struct itoax {

};

struct Zero {};

template <typename Nat>
struct Succ {};

template <typename L, typename R>
struct Add;

template <typename L, typename R>
using add = typename Add<L, R>::type;

template <typename R>
struct Add<Zero, R> {
    using type = R;
};

template <typename L, typename R>
struct Add<Succ<L>, R> {
    using type = Succ<add<L, R>>;
};

// prove that: A + B == C
template <typename A, typename B, typename C>
struct Equality {
    Equality() = delete;
};

template <typename A, typename B>
struct Equality<A, B, add<A, B>> {
    Equality() = default;
};

template <typename N, typename T>
struct Vec {
    T operator[](int i) {
        // blahblah...
        return T{};
    }

    Vec<Succ<N>, T> push(T x) {
        // blahblah...
        return Vec<Succ<N>, T>{};
    }
};

template <typename T>
struct Vec<Zero, T> {
    Vec<Succ<Zero>, T> push(T x) {
        // blahblah...
        return Vec<Succ<Zero>, T>{};
    }
};

template <typename T>
Vec<Zero, T> make_empty_vector() {
    return Vec<Zero, T>{};
}

int main() {
    auto a = make_empty_vector<int>();
    a[0];
    auto b = a.push(10);
    b[0];
}

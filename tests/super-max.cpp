//
// Created by kiva on 2019/10/11.
//
#include <type_traits>
#include <utility>

template<typename ...>
using void_t = void;

template<typename T, typename = void_t<>>
struct OverloadsGT : public std::false_type {
};

template<typename T>
struct OverloadsGT
    <T,
        void_t<decltype(std::declval<T>() > std::declval<T>())>
    > : public std::true_type {
};

template <typename T, bool = OverloadsGT<T>::value>
struct Max {
    static T superMax(T &&lhs, T &&rhs) {
        return lhs > rhs ? lhs : rhs;
    }
};

template <typename T>
struct Max<T, false> {
    static T superMax(T &&lhs, T &&rhs) = delete;
};

struct Test1 {
};

struct Test2 {
    bool operator>(Test2 &&o) const { return true; }
    bool operator>(const Test2 &o) const { return true; }
};


template <typename T, typename = void_t<decltype(std::declval<T>() > std::declval<T>())>>
constexpr bool hasGT(int) { return true; }

template <typename T>
constexpr bool hasGT(char) { return false; }

int main() {
//    static_assert(hasGT<int>(0), "FUCK");
//    static_assert(hasGT<Test1>(99999), "FUCK");
//
    Max<int>::superMax(1, 2);
    Max<Test2>::superMax(Test2(), Test2());
//    Max<Test1>::superMax(Test1(), Test1());
}

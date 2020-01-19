//
// Created by kiva on 2019/12/27.
//

#include <cstdio>
#include <type_traits>
#include <utility>
#include <typeindex>
#include <cxxabi.h>
#include <string>

template <typename T>
std::string demangle() {
    char *s = abi::__cxa_demangle(typeid(T).name(),
        nullptr, nullptr, nullptr);
    std::string r = s;
    std::free(s);
    return std::move(r);
}

template <typename T, typename ... Ts>
struct is_one_of {
    static constexpr bool value = false;
};

template <typename U, typename T, typename ... Ts>
struct is_one_of<U, T, Ts...> {
    static constexpr bool value = std::is_same_v<U, T>
                                  || is_one_of<U, Ts...>::value;
};

template <typename T>
struct is_primitive_type {
    static constexpr bool value =
        is_one_of<std::remove_cv_t<T>, bool, char, unsigned char,
            short, unsigned short,
            int, unsigned int,
            long, unsigned long,
            long long, unsigned long long,
            float, double,
            long double>::value;
};

template <typename T>
static constexpr bool is_primitive_type_v = is_primitive_type<T>::value;

template <typename T, typename = void>
struct Do {
    static void doit(const T &t) {
        printf("%s: pass-by-reference\n", demangle<T>().c_str());
    }
};

template <typename T>
struct Do<T, std::enable_if_t<is_primitive_type_v<T>>> {
    static void doit(T t) {
        printf("%s: pass-by-value\n", demangle<T>().c_str());
    }
};

template <typename T>
void fuck(T &&t) {
    Do<T>::doit(std::forward<T>(t));
}

struct App {
    App() = default;
    App(const App &) = delete;
    App(App &&) noexcept = delete;
    App &operator=(App &&) = delete;
    App &operator=(const App &) = delete;
};

int main() {
    const char *ss = "abcd";
    fuck(1);
    fuck(1.0f);
    fuck("fuck");
    fuck(ss);
    fuck(App{});
}

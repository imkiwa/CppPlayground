//
// Created by kiva on 2019/12/27.
//

#include <cstdio>

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



int main() {

}

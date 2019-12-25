//
// Created by kiva on 2019/12/25.
//

#include <iostream>
#include <v9/kit/string.hpp>

using namespace v9::kit;

bool verify(StringRef str) {
    int index = 0;
    int x = str.intStream()
        .take(17)
        .map([&index](int c) {
            int code = (1 << (18 - index++ - 1)) % 11 * (c - '0');
            return code;
        })
        .reduce<int>(0, [](int acc, int c) {
            return acc + c;
        });

    x = (12 - (x % 11)) % 11;
    return (x + '0') == str[17];
}

int main() {
    std::string s;
    std::getline(std::cin, s);

    if (verify(s)) {
        printf("OK\n");
    } else {
        printf("FUCK\n");
    }
}

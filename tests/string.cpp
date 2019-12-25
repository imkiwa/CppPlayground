//
// Created by kiva on 2019/12/25.
//

#include <iostream>
#include <v9/kit/string.hpp>

using namespace v9::kit;

bool verify(StringRef str) {
    int x = str.indexedStream()
        .take(17)
        .map([](auto pair) {
            int code = (1 << (18 - pair.first - 1)) % 11 * (pair.second - '0');
            return std::make_pair(pair.first, code);
        })
        .reduce<int>(0, [](int acc, auto pair) {
            return acc + pair.second;
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

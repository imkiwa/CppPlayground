//
// Created by kiva on 2019/12/7.
//
#include <v9/kit/optional.hpp>
#include <string>
#include <vector>
#include <cstdio>

using namespace v9::kit;

void sayHi(Optional<std::string> name) {
    if (name.hasValue()) {
        printf("hi: %s\n", name.get().c_str());
    } else {
        printf("you didn't tell me who you are) QwQ\n");
    }
}

int sum(const Optional<std::vector<int>> &nums) {
    return nums.applyOr<int>(0, [](auto &&v) -> int {
        int s = 0;
        for (auto &&e : v) {
            s += e;
        }
        return s;
    });
}

int main(int argc, const char **argv) {
    sayHi(Optional<std::string>::none());
    sayHi(Optional<std::string>::from("imkiva"));

    std::vector<int> v{1, 2, 3, 4};
    printf("sum1 = %d\n", sum(Optional<decltype(v)>::from(v)));
    printf("sum2 = %d\n", sum(Optional<decltype(v)>::none()));
    printf("sum3 = %d\n", sum(Optional<decltype(v)>::emplace(1, 2, 3, 4, 5)));
}

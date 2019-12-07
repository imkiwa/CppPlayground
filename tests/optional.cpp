//
// Created by kiva on 2019/12/7.
//
#include <v9/memory/optional.hpp>
#include <string>
#include <vector>
#include <cstdio>

using namespace v9::memory;

void sayHi(Optional<std::string> name) {
    if (name.hasValue()) {
        printf("hi: %s\n", name.get().c_str());
    } else {
        printf("you didn't tell me who you are) QwQ\n");
    }
}

int sum(Optional<std::vector<int>> nums) {
    int s = 0;
    if (nums.hasValue()) {
        for (auto e : nums.get()) {
            s += e;
        }
    }
    return s;
}

int main(int argc, const char **argv) {
    sayHi(Optional<std::string>::nothing());
    sayHi(Optional<std::string>::just("imkiva"));

    std::vector<int> v{1, 2, 3, 4};
    printf("sum1 = %d\n", sum(Optional<decltype(v)>::just(v)));
    printf("sum2 = %d\n", sum(Optional<decltype(v)>::nothing()));
    printf("sum3 = %d\n", sum(Optional<decltype(v)>::emplace(1, 2, 3, 4, 5)));
}

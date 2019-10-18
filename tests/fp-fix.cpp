//
// Created by kiva on 2019/10/17.
//
#include <v9/fp/curry.hpp>
#include <vector>

template <typename T>
std::vector<T> fmap(std::vector<T> &&elems, std::function<T(T)> &&mapper) {
    std::vector<T> result(elems);
    for (auto &&e : result) {
        e = mapper(e);
    }
    return std::move(result);
}

int main(int argc, const char **argv) {
    using namespace v9::fp;

    auto add = [](int a, int b) { return a + b; };

    std::vector<int> v{1, 2, 3, 4, 5, 6, 7};
    std::vector<int> r = fmap(v, curry(add)(1));
    for (auto &&e : r) {
        printf("%d, ", e);
    }
    printf("\n");
}

//
// Created by kiva on 2019/12/27.
//

#include <iostream>
#include <vector>
#include <utility>
#include <unordered_map>
#include <cassert>

template <typename T, T t>
struct identity_value {
    static constexpr T value = t;
};

std::pair<int, int> try_buy_all(int money, int price) {
    return std::make_pair(money / price, money % price);
}

std::tuple<int, int, int> fuck(int n) {
    if (n < 3 || n == 5) {
        // buy nmlgb
        return std::make_tuple(-1, -1, -1);
    }
    // condition 2: buy a pair first
    auto[pairs, remaining] = try_buy_all(n, 14);

    // condition 3: buy as much items as possible
    auto[all2n, all2r] = try_buy_all(remaining, 4);
    auto[all3n, all3r] = try_buy_all(remaining, 3);

    // if we can use the remaining money to buy all item3
    if (all3r == 0) {
        // the amount is sure the max.
        return std::make_tuple(pairs, pairs, pairs + all3n);
    }

    // if we can use the remaining money to buy all item2
    if (all2r == 0) {
        // the amount is sure the max.
        return std::make_tuple(pairs, pairs + all2n, pairs);
    }

    // condition 1: spend all money
    for (int more_2 = 1; more_2 <= all2n; ++more_2) {
        if (all3r + 3 * more_2 - 4 * more_2 == 0) {
            return std::make_tuple(pairs, pairs + more_2, pairs + all3n - more_2);
        }
    }

    // buy nmlgb
    return std::make_tuple(-1, -1, -1);

//    switch (all3r) {
//        case 1:
//            // we have 1 money left
//            // buy (all3n - 1) item2 and 1 item3 will spend all money.
//            return std::make_tuple(pairs, pairs + 1, pairs + all3n - 1);
//        case 2:
//            // we have 2 money left,
//            // buy (all3n - 2) items and 2 item3 will spend all money.
//            return std::make_tuple(pairs, pairs + 2, pairs + all3n - 2);
//        default:
//            assert(!"should not reach here");
//    }
}

int main() {
//    int n;
//    std::cin >> n;
    constexpr size_t mikecovlee = ~(size_t) 0;
    constexpr size_t imkiva = - ~(size_t) 0;
    static_assert(mikecovlee > imkiva, "impossible");
}

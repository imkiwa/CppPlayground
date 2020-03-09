//
// Created by kiva on 2019/12/27.
//

#include <iostream>
#include <vector>
#include <utility>
#include <unordered_map>

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};

struct location {
    bool _source = false;
    int _around = 0;
    int _score = 0;
};

int main() {
    using map_t = std::unordered_map<std::pair<int, int>, location, pair_hash>;
    map_t map;

    int n = 0;
    std::cin >> n;

    int x = 0;
    int y = 0;

    for (int i = 0; i < n; ++i) {
        std::cin >> x >> y;
        map[{x, y}]._source = true;
        ++map[{x + 1, y + 1}]._score;
        ++map[{x + 1, y - 1}]._score;
        ++map[{x - 1, y + 1}]._score;
        ++map[{x - 1, y - 1}]._score;
        ++map[{x, y + 1}]._around;
        ++map[{x, y - 1}]._around;
        ++map[{x + 1, y}]._around;
        ++map[{x - 1, y}]._around;
    }

    for (auto &&e : map) {
        if (e.second._source && e.second._around == 4) {
            printf("(%d, %d): %d\n",
                e.first.first, e.first.second,
                e.second._score);
        }
    }
}

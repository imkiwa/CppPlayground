//
// Created by kiva on 2019/12/25.
//
#include <cstdio>
#include <string>
#include <algorithm>
#include <vector>

std::string longest(const std::string &src, const std::vector<std::string> &dict) {
    int current = 0;
    int longest = -1;
    int longest_match = -1;

    for (auto &&elem : dict) {
        auto l = src.c_str();
        auto r = elem.c_str();
        while (*l && *r) {
            if (*l == *r) {
                ++l;
                ++r;
            } else {
                ++l;
            }
        }

        // matched the whole src using elem
        if (!*r) {
            int match = r - elem.c_str();
            if (match > longest_match) {
                longest_match = match;
                longest = current;
            }
        }
        ++current;
    }

    return longest >= 0 ? dict[longest] : "";
}

int main() {
    auto str = longest("abpcplea", {"monkey", "plea", "ale", "apple"});
    printf("%s\n", str.c_str());
}

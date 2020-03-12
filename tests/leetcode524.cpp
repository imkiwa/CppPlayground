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

        // iterate src and dict
        while (*l && *r) {
            if (*l == *r) {
                // if current char are the same,
                // we match this char
                ++l;
                ++r;
            } else {
                // or we just delete this char from src,
                // try to match elem with the next char in src
                ++l;
            }
        }

        // successfully matched the elem after deleting some chars in src,
        // the elem has been matched, r now points to the '\0',
        if (!*r) {
            // if this match is longer than previous one,
            // replace it with current one.
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

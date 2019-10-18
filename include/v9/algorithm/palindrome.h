//
// Created by kiva on 2018/4/22.
//
#pragma once

#include <string>

namespace v9 {
    namespace solve {
        bool isPalindrome(const std::string &s) {
            for (int i = 0, j = static_cast<int>(s.size() - 1); i < j; ++i, --j) {
                if (s[i] != s[j]) {
                    return false;
                }
            }
            return true;
        }
    }
}

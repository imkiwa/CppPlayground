//
// Created by kiva on 2019/12/13.
//

#pragma once

#include <cstdio>
#include <cwchar>
#include <cstring>
#include <cstdint>
#include <v9/kit/char.hpp>

namespace v9::kit {
    class String {
    private:
        Char *_data = nullptr;
        size_t _length = 0;
        size_t _capacity = 0;

    public:
        String() = default;

        ~String() {
        }


    };
}

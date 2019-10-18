//
// Created by kiva on 2019/10/17.
//
#pragma once

#include <v9/fp/curry.hpp>

namespace v9::fp {
    namespace legacy {
        template<typename F>
        auto Y(F &&f) {
            return std::bind(f, std::bind(Y, f), std::placeholders::_1);
        }
    }

    using namespace legacy;
}

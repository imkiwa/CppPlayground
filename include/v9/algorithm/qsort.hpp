//
// Created by kiva on 2017/10/21.
//
#pragma once

#include <v9/bits/types.hpp>
#include <v9/bits/traits.hpp>

namespace v9 {
    namespace sorts {
        template<typename T>
        void qsort(T *elements, size_t length) {
            if (length < 2) return;

            size_t left = 0;
            size_t right = length;
            const T &base = elements[0];

            for (;;) {
                while (--right > left && elements[right] > base);
                if (right > left) {
                    elements[left] = elements[right];
                } else {
                    break;
                }

                while (++left < right && elements[left] <= base);
                if (left < right) {
                    elements[right] = elements[left];
                } else {
                    break;
                }
            }

            elements[left] = base;
            qsort(elements, left);
            qsort(elements + left + 1, length - left - 1);
        }
    }
}

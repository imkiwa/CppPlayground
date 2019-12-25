//
// Created by kiva on 2017/10/21.
//
#pragma once

#include <v9/bits/types.hpp>
#include <v9/bits/traits.hpp>

namespace v9 {
    namespace sorts {
        template<typename T>
        void qsort(T *a, size_t low, size_t high) {
            while (low < high) {
                size_t md = low + (high - low) / 2;
                size_t l = low - 1;
                size_t h = high + 1;
                T p = a[md];

                while (true) {
                    while (a[++l] < p);
                    while (a[--h] > p);
                    if (l >= h) {
                        break;
                    }
                    std::swap(a[l], a[h]);
                }

                l = h++;
                if ((l - low) <= (high - h)) {
                    qsort(a, low, l);
                    low = h;
                } else {
                    qsort(a, h, high);
                    high = l;
                }
            }
        }
    }
}

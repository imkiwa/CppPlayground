//
// Created by kiva on 2017/10/21.
//

#include <v9/v9.hpp>
#include <cstdio>

int main() {
    int a[] = {6, 8, 7, 6, 5, 4, 1, 2, 5, 6, 3, 2, 1, 2, 3, 4, 5, 7,
               5, 6, 4, 12, 312, 3, 412, 4, 13, 21, 4, 12, 31, 4, 12,
               4, 1, 3, 6, 4, 99, 12312, 41231, 523, 1234, 4, 123, 413, 13213, 1,};
    v9::sorts::qsort(a, sizeof(a) / sizeof(a[0]));
    for (int i : a) {
        printf("%d, ", i);
    }
    printf("\n");
}

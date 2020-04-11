//
// Created by kiva on 2020/4/8.
//

#include <cstdio>
#include <cstring>
#include <cstdint>

int main() {
    int a = 1999999999;
    int b = 1999999999;
    if ((a > 0) && (b > 2147483647 - a)) {
        printf("overflow\n");
    }
}


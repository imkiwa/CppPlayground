//
// Created by kiva on 2020/2/27.
//
#include <stdio.h>

int main() {
    int a = 10, y = 0;
    for (; a < 14;) {
        a += 2;
        y += a;
        if (y > 50) break;
    }
    printf("%d\n", y);
}

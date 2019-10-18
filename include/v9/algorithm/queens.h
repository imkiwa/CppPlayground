//
// Created by kiva on 2018/4/22.
//
#pragma once

#include <cmath>
#include <cstdio>

namespace v9 {
    namespace solve {
        int queenAbs(int x) {
            return x >= 0 ? x : -x;
        }

        bool queenCheck(int n, const int *queen) {
            for (int i = 0; i < n; ++i) {
                int x = n - i;
                int y = queen[n] - queen[i];

                if (y == 0 || queenAbs(x) == queenAbs(y)) {
                    return false;
                }
            }
            return true;
        }

        void queenShowResult(int n, const int *queen, int counter) {
            printf("No. %d\n", counter);
            for (int y = 0; y < n; ++y) {
                for (int x = 0; x < n; ++x) {
                    if (y == queen[x]) {
                        printf("1 ");
                    } else {
                        printf("0 ");
                    }
                }
                printf("\n");
            }
        }

        void queenDfs(int step, int n, int *queen, int &counter) {
            if (step == n) {
                ++counter;
                queenShowResult(n, queen, counter);
                return;
            }

            for (int i = 0; i < n; ++i) {
                queen[step] = i;
                if (queenCheck(step, queen)) {
                    queenDfs(step + 1, n, queen, counter);
                }
            }
        }

        int solveQueens(int n) {
            int array[n];
            int counter = 0;
            queenDfs(0, n, array, counter);
            return counter;
        }
    }
}

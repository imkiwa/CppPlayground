//
// Created by kiva on 2019/10/17.
//

#include <v9/fp/basic.hpp>

int main(int argc, const char **argv) {
    using namespace v9::fp;
    add(1, 2);
    add(1, 4);
    add(1, 4.0);
    sub(0.1, 2);
    sub(0, 1);
    mul(1, 2);
    mul(0.0, 0.0);
    div(1, 2);
    div(1.0, 2);
    int x = 1;
    add(x, 1);
    sub(3, x);
}

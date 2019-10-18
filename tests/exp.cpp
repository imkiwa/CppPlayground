//
// Created by kiva on 2018/4/20.
//

#include <v9/expression/evaluator.h>

int main() {
    using namespace v9::expression;
    Number r = evaluate("9 2 3 + 6 * +");
    printf("%f\n", r);
}

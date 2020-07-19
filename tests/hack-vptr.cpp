#include <cstdio>
#include <exception>
#include <string>

class Base {
public:
    virtual void test() = 0;
};

class Child : public Base {
public:
    void test() override { std::terminate(); }
};

void test_hack(Child *_this) {
    printf("女装真香\n");
}

void call_test(Base *base) {
    base->test();
}

int main() {
    Base *mikecovlee = new Child;

    void *new_vptr[] = {reinterpret_cast<void *>(test_hack)};
    *((void ***) mikecovlee) = new_vptr;

    call_test(mikecovlee);
}

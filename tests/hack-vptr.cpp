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
    printf("Hacked\n");
}

int main() {
    Base *obj = new Child;

    void *new_vptr[] = {reinterpret_cast<void *>(test_hack)};
    *((void ***) obj) = new_vptr;

    obj->test();
}

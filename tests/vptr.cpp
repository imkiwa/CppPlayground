//
// Created by kiva on 2019/12/16.
//

class Base {
public:
    virtual ~Base() = default;
    virtual void pureVirtual1() = 0;
    virtual void pureVirtual2() = 0;
    virtual void pureVirtual3() = 0;
    virtual void pureVirtual4() = 0;
};

class Derived : public Base {
public:
    ~Derived() override = default;
    void pureVirtual1() override {}
    void pureVirtual2() override {}
    void pureVirtual3() override {}
    void pureVirtual4() override {}
};

int main() {
    Derived d;
    d.pureVirtual1();
}

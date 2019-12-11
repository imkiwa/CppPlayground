#include <cstdio>
#include <exception>
#include <string>

class 女装会长 {
public:
    virtual void 女装() = 0;
};

class 老李 : public 女装会长 {
public:
    void 女装() override { std::terminate(); }
};

void 真香(老李 *_this) {
    printf("女装真香\n");
}

void 淦(女装会长 *老李) {
    老李->女装();
}

int main() {
    女装会长 *mikecovlee = new 老李;

    void *new_vptr[] = {reinterpret_cast<void *>(真香)};
    *((void ***) mikecovlee) = new_vptr;

    淦(mikecovlee);
}

void hi() {
    女装会长 *mikecovlee = new 老李;

    void **vptr = *((void ***) mikecovlee);
    printf(":: Before hack, vptr: %p\n", vptr);

    void *new_vptr[] = {reinterpret_cast<void *>(真香)};
    *((void ***) mikecovlee) = new_vptr;
    vptr = *((void ***) mikecovlee);
    printf(":: After hack,  vptr: %p\n", vptr);

    淦(mikecovlee);
}

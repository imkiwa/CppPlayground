//
// Created by kiva on 2020/2/27.
//

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    const char *name;
    int size;

    void (*constructor)(void *);

    void (*destructor)(void *);
} ClassInfo;

struct People {
    ClassInfo *__class;
    const char *name;
    int age;
};

void People_ctor(struct People *this) {
    printf("People constructor\n");
}

void People_dtor(struct People *this) {
    printf("People destructor\n");
}

ClassInfo People = {
    .name = "People",
    .size = sizeof(struct People),
    .constructor = People_ctor,
    .destructor = People_dtor,
};

#define new(class) new_(&class)

void *new_(ClassInfo *class) {
    void *obj = malloc(class->size);
    *((ClassInfo **) obj) = class;
    class->constructor(obj);
    return obj;
}

void delete(void *obj) {
    ClassInfo *class = *((ClassInfo **) obj);
    class->destructor(obj);
    free(obj);
}

int main() {
    struct People *p = new(People);
    p->name = "LoverFucker";
    p->age = 10;
    delete(p);
}

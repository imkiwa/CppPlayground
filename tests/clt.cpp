#include <stdio.h>
#include <assert.h>

#define MEM_SIZE 128
static char mem[MEM_SIZE];

#define MINBLK 16
struct block {
    struct block *next;
    size_t length;
    char data[0];//data指向地址空间的起始
};

static struct block *free_list = NULL;
static struct block *start;

void malloc_init(void) {
    free_list = (struct block *) mem;
    start = free_list;
    free_list->next = NULL;
    free_list->length = MEM_SIZE - sizeof(struct block);
}

//void *malloc(size_t size) {
//    size = (size + 7) & ~7;
//
//    struct block *prev = NULL;
//    struct block *cur = free_list;
//
//    while (cur != NULL) {
//        if (cur->length >= size) break;///剩余空间大于请求空间
//        prev = cur;
//        cur = cur->next;
//    }
//
//    if (cur == NULL)//找不到符合要求的空间
//        return NULL;
//
//    if (cur->length - size >= MINBLK) {
//        assert(sizeof(struct block) <= MINBLK);
//        struct block *rest_after_alloc = (struct block *) (cur->data + size);
//        rest_after_alloc->next = cur->next;
//        rest_after_alloc->length = cur->length - size - sizeof(struct block);// 新分配的空间在原空间之前
//        if (prev) {
//            prev->next = rest_after_alloc;
//        } else {
//            free_list = rest_after_alloc;
//        }
//        cur->length = size;
//        cur->next = rest_after_alloc;
//    } else {
//        if (prev) {
//            prev->next = cur->next;
//        } else {
//            free_list = cur->next;
//        }
//    }
//
//    return cur->data;
//}

void *malloc(size_t size) {
    size = (size + 7) & ~7;

    struct block **indirect = &free_list;

    while (*indirect) {
        if ((*indirect)->length >= size) {
            break;
        }
        indirect = &((*indirect)->next);
    }

    if (*indirect == NULL)//找不到符合要求的空间
        return NULL;

    char *data = (*indirect)->data;

    if ((*indirect)->length - size >= MINBLK) {
        assert(sizeof(struct block) <= MINBLK);
        struct block *rest_after_alloc = (struct block *) ((*indirect)->data + size);
        rest_after_alloc->next = (*indirect)->next;
        rest_after_alloc->length = (*indirect)->length - size - sizeof(struct block);// 新分配的空间在原空间之前
        (*indirect)->length = size;
        (*indirect)->next = rest_after_alloc;
        *indirect = rest_after_alloc;
    } else {
        *indirect = (*indirect)->next;
    }

    return data;
}

void free(void *ptr) {
    if (ptr == NULL)
        return;

    struct block *owner_block = (struct block *) ((char *) ptr - sizeof(struct block));
    struct block *prev = NULL;
    struct block *cur = start;

    while (cur) {
        if (cur == owner_block) {
            break;
        }

        prev = cur;
        cur = cur->next;
    }

    if (cur == NULL) {
        printf("attempt to free a memory that was not allocated by us\n");
        return;
    }

    // case2
    if (prev) {
        if ((char *) cur == prev->data + prev->length) {
            prev->length += cur->length + sizeof(struct block);
            prev->next = cur->next;
            return;
        }
    }

    // case1
    if (cur->data + cur->length == (char *) cur->next) {
        assert(cur->next != NULL);
        cur->length += cur->next->length + sizeof(struct block);
        cur->next = cur->next->next;
        if (prev) {
            prev->next = cur;
        } else {
            free_list = cur;
        }
        return;
    }

    // case3
    // handled in case1 and case2
}

void malloc_state(void) {
    printf("free blocks:\n");
    struct block *ptr;
    for (ptr = free_list; ptr != NULL; ptr = ptr->next) {
        //int i;
        printf("%p:", ptr);
        /*for (i = 0; i < ptr->length; i++) {
            printf("=");
        }*/
        printf("left free block size: %zd\n", ptr->length);
        printf(" ");
    }
    printf("\n");
}

void malloc_test(void) {
    printf("test start\n");
    printf("size of a single block %zd\n", sizeof(struct block));
    char *a = (char *) malloc(5);
    malloc_state();
    char *b = (char *) malloc(10);
    malloc_state();
    char *c = (char *) malloc(20);
    malloc_state();
    free((void *) b);
    malloc_state();
    free((void *) c);
    malloc_state();
    free((void *) a);
    malloc_state();
    assert(free_list == start);
}

int main(int argc, const char *argv[]) {
    malloc_init();
    malloc_state();
    malloc_test();
    return 0;
}

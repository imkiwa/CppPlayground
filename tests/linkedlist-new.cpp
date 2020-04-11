//
// Created by kiva on 2020/4/8.
//
#include <cstdio>
#include <cstdlib>
#include <new>
#include <list>

template <typename T>
struct default_allocator {
    using pointer = T *;
    using reference = T &;
    using const_pointer = const T *;
    using const_reference = const T &;

    T *allocate(size_t n) {
        auto *ptr = static_cast<T *>(::malloc(n * sizeof(T)));
        if (!ptr) {
            throw std::bad_alloc();
        }
        return ptr;
    }

    void deallocate(T *ptr) {
        if (ptr) {
            ::free(ptr);
        }
    }
};

template <typename T, typename Allocator = default_allocator<T>>
class list {
public:
    using pointer = typename Allocator::pointer;
    using reference = typename Allocator::reference;
    using const_pointer = typename Allocator::const_pointer;
    using const_reference = typename Allocator::const_reference;
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using allocator_type = Allocator;
};

//
// Created by kiva on 2019/12/27.
//

#include <cstdio>
#include <typeinfo>
#include <functional>
#include <type_traits>


//class A {
//public:
//    A(int);
//
//    friend void fuck(A) {}
//};
//
//int main() {
//    A a = 0;
//    fuck(a);
//    fuck(0);
//}


//class A {
//public:
//    A(int) {}
//
//    friend void f(A);
//};
//
//void g(A);
//
//class B {
//    friend void f(A) {
//        printf("hello world!\n");
//    }
//
//    class C {
//        friend void g(A) {
//            printf("!dlrow olleh\n");
//        }
//    };
//};
//
//int main() {
//    A a(0);
//    f(a);
//    g(1);
//}

//constexpr int lifetime_expired();
//
//template <bool available = !noexcept(lifetime_expired())>
//void test1() {
//    static_assert(available, "should available");
//    printf("lifetime available: %d\n", available);
//}
//
//void indirection();
//
//int main() {
//    test1();
//    indirection();
//}
//
//constexpr int lifetime_expired() {
//    return 10086;
//}
//
//template <bool available = !noexcept(lifetime_expired())>
//void test2() {
//    static_assert(!available, "should ! available");
//    printf("lifetime available: %d\n", available);
//}
//
//void indirection() {
//    test2();
//}

//
//constexpr int lifetime_expired();
//
//template <bool available = !noexcept(lifetime_expired())>
//void test1() {
//    static_assert(available, "should available");
//    printf("lifetime available: %d\n", available);
//}
//
//void indirection();
//
//int main() {
//    test1();
//    indirection();
//}
//
//template <typename>
//struct lifetime_struct {
//    friend constexpr int lifetime_expired() {
//        return 10086;
//    }
//};
//
//template <bool available = !noexcept(lifetime_expired())>
//void test2() {
//    static_assert(!available, "should ! available");
//    printf("lifetime available: %d\n", available);
//}
//
//void indirection() {
//    test2();
//}

//#define LIFETIME_NAME __lifetime_expired
//#define NEW_LIFETIME(L) extern constexpr int LIFETIME_NAME();
//#define DELETE_LIFETIME(L) { (void) lifetime_deleter<L>(); }
//
//constexpr int LIFETIME_NAME();
//
//template <size_t I>
//struct lifetime_deleter {
//    friend constexpr int LIFETIME_NAME() {
//        return 10086;
//    }
//};
//
//template <size_t I, bool available = !noexcept(LIFETIME_NAME())>
//constexpr bool lifetime_available() {
//    return available;
//}
//
//int main() {
//    NEW_LIFETIME(1)
//
//    printf("lifetime available: %d\n", lifetime_available<1>());
//    static_assert(lifetime_available<1>(), "should available");
//
//    DELETE_LIFETIME(1)
//
//    printf("lifetime available: %d\n", lifetime_available<1>());
//    static_assert(!lifetime_available<1>(), "should ! available");
//}

//constexpr int lifetime_available(int);
//
//template <typename T>
//struct writer {
//    friend constexpr int lifetime_available(T) {
//        return 0;
//    }
//};
//
//template <bool B, typename T = int>
//struct dependent_writer : writer<T> {};
//
//template <
//    bool B = noexcept(lifetime_available(0)),
//    int    = sizeof(dependent_writer<B>)
//>
//constexpr int f() {
//    return B;
//}
//
//int main() {
//    constexpr int a = f();
//    constexpr int b = f();
//    printf("%d\n", a);
//    printf("%d\n", b);
//
//    static_assert(a != b, "should be true");
//}

namespace borrow {
    /*
     * Something to simulate the requires keyword introduced in C++20.
     */

    template <typename ...>
    using requires_all = void;

    template <bool>
    struct requires_true_ {
    };

    template <>
    struct requires_true_<true> {
        using type = void;
    };

    template <bool b>
    using requires_true = typename requires_true_<b>::type;

    template <bool b>
    using requires_false = typename requires_true_<!b>::type;

    template <typename T, typename R>
    using requires_same = requires_true<std::is_same<T, R>::value>;

    template <typename LT, int>
    struct counter {
        friend constexpr auto prev_lifetime(counter);
    };

    template <typename LT, int N, typename T>
    struct counter_inc {
        friend constexpr auto prev_lifetime(counter<LT, N>) {
            return T{};
        }
    };

    template <typename LT, int N = 0>
    constexpr int current_count(float, counter<LT, N> = {}) {
        return N;
    }

    template <typename LT, int N = 0,
        typename = decltype(prev_lifetime(counter<LT, N>{}))
    >
    constexpr int current_count(int, counter<LT, N> = {}, int value = current_count(0, counter<LT, N + 1>{})) {
        return value;
    }

    // the static lifetime
    struct Static {
    };

    template <typename, typename, typename = void>
    struct const_borrow;

    template <typename Super, typename Sub>
    struct const_borrow<Super, Sub, std::void_t<typename Sub::lifetime_info>> {
    };

    template <typename, typename, typename = void>
    struct mut_borrow;

    template <typename Super, typename Sub>
    struct mut_borrow<Super, Sub, ::std::void_t<typename Sub::lifetime_info>> {
    };

    template <typename>
    constexpr bool is_const_borrow = false;

    template <typename Super, typename Sub>
    constexpr bool is_const_borrow<const_borrow<Super, Sub>> = true;

    template <typename T>
    constexpr bool is_mut_borrow = !is_const_borrow<T>;

    template <typename, typename>
    struct borrowed;

    template <typename T, typename Super, typename Sub>
    struct borrowed<T, const_borrow<Super, Sub>> {
        const T &ref;
    };

    template <typename T, typename Super, typename Sub>
    struct borrowed<T, mut_borrow<Super, Sub>> {
        T &ref;
    };

    template <typename T>
    constexpr bool is_borrowed = false;

    template <typename T, typename BorrowType>
    constexpr bool is_borrowed<borrowed<T, BorrowType>> = true;

    template <typename LT, typename = typename LT::lifetime_info::destroyed>
    constexpr bool lifetime_alive(LT, int) {
        // if the destroyed tag was set, the lifetime has died.
        return false;
    }

    template <typename LT>
    constexpr bool lifetime_alive(LT, long) {
        // otherwise, the lifetime is still alive.
        return true;
    }

    constexpr bool all_alive(Static) {
        // the static lifetime is always alive
        return true;
    }

    template <typename Super, typename Sub,
        bool SUB_ALIVE = lifetime_alive(Sub{}, 0),
        bool SUPER_ALIVE = all_alive(Super{})
    >
    constexpr bool all_alive(const_borrow<Super, Sub>) {
        // we say a lifetime is alive if
        // 1) this lifetime itself is alive
        // 2) the super lifetime is alive
        return SUB_ALIVE && SUPER_ALIVE;
    }

    template <typename Super, typename Sub,
        bool SUB_ALIVE = lifetime_alive(Sub{}, 0),
        bool SUPER_ALIVE = all_alive(Super{})
    >
    constexpr bool all_alive(mut_borrow<Super, Sub>) {
        // we say a lifetime is alive if
        // 1) this lifetime itself is alive
        // 2) the super lifetime is alive
        return SUB_ALIVE && SUPER_ALIVE;
    }

    constexpr bool any_alive(Static) {
        // if there's no living lifetime,
        // we simply say all lifetimes are died.
        return false;
    }

    template <typename Super, typename Sub,
        bool SUB_ALIVE = lifetime_alive(Sub{}, 0),
        bool SUPER_ALIVE = any_alive(Super{})
    >
    constexpr bool any_alive(const_borrow<Super, Sub>) {
        // we say a lifetime is any alive if either
        // 1) this lifetime itself is alive
        // 2) the super lifetime is alive
        return SUB_ALIVE || SUPER_ALIVE;
    }

    template <typename Super, typename Sub,
        bool SUB_ALIVE = lifetime_alive(Sub{}, 0),
        bool SUPER_ALIVE = any_alive(Super{})
    >
    constexpr bool any_alive(mut_borrow<Super, Sub>) {
        // we say a lifetime is any alive if either
        // 1) this lifetime itself is alive
        // 2) the super lifetime is alive
        return SUB_ALIVE || SUPER_ALIVE;
    }

    /////////////////////////////////////////////////////////////////////////////////
    // dereference
    /////////////////////////////////////////////////////////////////////////////////

    template <typename T, typename Super, typename Sub,
        // borrowed must be alive
        // error: `x` does not live long enough
        typename = requires_true<all_alive(const_borrow<Super, Sub>{})>
    >
    const T &operator*(const borrowed<T, const_borrow<Super, Sub>> &ptr) {
        return ptr.ref;
    }

    template <typename T, typename Super, typename Sub,
        // borrowed must be alive
        // error: `x` does not live long enough
        typename = requires_true<all_alive(mut_borrow<Super, Sub>{})>,
        // sanity check
        int N = current_count<Sub>(0),
        typename = requires_true<(N > 0)>,
        typename State = decltype(prev_lifetime(counter<Sub, N - 1>{})),
        typename = requires_true<is_const_borrow<State>>
    >
    const T &operator*(const borrowed<T, mut_borrow<Super, Sub>> &ptr) {
        return ptr.ref;
    }

    template <typename T, typename Super, typename Sub,
        // borrowed must be alive
        // error: `x` does not live long enough
        typename = requires_true<all_alive(mut_borrow<Super, Sub>{})>,
        // sanity check
        int N = current_count<Sub>(0),
        typename = requires_true<(N == 0)>
    >
    T &operator*(borrowed<T, mut_borrow<Super, Sub>> &ptr) {
        return ptr.ref;
    }

    template <typename T, typename Super, typename Sub,
        // borrowed must be alive
        // error: `x` does not live long enough
        typename = requires_true<all_alive(mut_borrow<Super, Sub>{})>,
        // sanity check
        int N = current_count<Sub>(0),
        typename = requires_true<(N > 0)>,
        typename State = decltype(prev_lifetime(counter<Sub, N - 1>{})),
        typename = requires_false<any_alive(State{})>
    >
    T &operator*(borrowed<T, mut_borrow<Super, Sub>> &ptr) {
        return ptr.ref;
    }

    /////////////////////////////////////////////////////////////////////////////////
    // borrow
    /////////////////////////////////////////////////////////////////////////////////

    template <typename LT, typename T, typename = requires_all<
        // this function handles borrowing from an unborrowed object
        requires_false<is_borrowed<T>>,
        // should borrow from a living lifetime
        requires_true<all_alive(const_borrow<Static, LT>{})>
    >>
    borrowed<T, const_borrow<Static, LT>>
    borrow(const T &ref) {
        return {ref};
    }

    // borrow as immutable from immutable borrows
    template <typename LT, typename T, typename Super, typename Sub, typename = requires_all<
        // should borrow from a living lifetime
        requires_true<all_alive(const_borrow<const_borrow<Super, Sub>, LT>{})>
    >>
    borrowed<T, const_borrow<const_borrow<Super, Sub>, LT>>
    borrow(const borrowed<T, const_borrow<Super, Sub>> &ptr) {
        return {ptr.ref};
    }

    template <typename LT, typename T, typename Super, typename Sub,
        // should borrow from a living lifetime
        typename = requires_true<all_alive(const_borrow<mut_borrow<Super, Sub>, LT>{})>,
        // obtain the immutable borrow count
        int N = current_count<Sub>(0),
        // if this value has not been borrowed as immutable
        typename = requires_true<N == 0>,
        // increase immutable borrow count
        size_t = sizeof(counter_inc<Sub, N, const_borrow<Static, LT>>)
    >
    borrowed<T, const_borrow<mut_borrow<Super, Sub>, LT>>
    borrow(const borrowed<T, mut_borrow<Super, Sub>> &ptr) {
        return {ptr.ref};
    }

    template <typename LT, typename T, typename Super, typename Sub,
        // should borrow from a living lifetime
        typename = requires_true<all_alive(const_borrow<mut_borrow<Super, Sub>, LT>{})>,
        // obtain immutable borrow count
        int N = current_count<Sub>(0),
        // if this value has been borrowed as immutable before
        typename = requires_true<(N > 0)>,
        // and previous borrows are all dead,
        typename FromLifetime = decltype(prev_lifetime(counter<Sub, N - 1>{})),
        typename = requires_false<any_alive(FromLifetime{})>,
        // increase immutable borrow count
        size_t = sizeof(counter_inc<Sub, N, const_borrow<Static, LT>>)
    >
    borrowed<T, const_borrow<mut_borrow<Super, Sub>, LT>>
    borrow(const borrowed<T, mut_borrow<Super, Sub>> &ptr) {
        return {ptr.ref};
    }

    template <typename LT, typename T, typename Super, typename Sub,
        // should borrow from a living lifetime
        typename = requires_true<all_alive(const_borrow<mut_borrow<Super, Sub>, LT>{})>,
        // obtain immutable borrow count
        int N = current_count<Sub>(0),
        // if this value has been borrowed as immutable before
        typename = requires_true<(N > 0)>,
        // and previous immutable borrow is alive
        typename State = decltype(prev_lifetime(counter<Sub, N - 1>{})),
        typename = requires_true<any_alive(State{})>,
        // previous borrow must be immutable
        // error: cannot borrow `x` as immutable because it also borrowed as mutable
        typename = requires_true<is_const_borrow<State>>,
        // increase immutable borrow count
        size_t = sizeof(counter_inc<Sub, N, const_borrow<State, LT>>)
    >
    borrowed<T, const_borrow<mut_borrow<Super, Sub>, LT>>
    borrow(const borrowed<T, mut_borrow<Super, Sub>> &ptr) {
        return {ptr.ref};
    }

    /////////////////////////////////////////////////////////////////////////////////
    // borrow_mut
    /////////////////////////////////////////////////////////////////////////////////

    template <typename LT, typename T, typename = requires_all<
        // this function handles borrowing from an unborrowed object
        requires_false<is_borrowed<T>>,
        // should borrow from a living lifetime
        requires_true<all_alive(mut_borrow<Static, LT>{})>
    >>
    borrowed<T, mut_borrow<Static, LT>>
    borrow_mut(T &ref) {
        return {ref};
    }

    template <typename LT, typename T, typename Super, typename Sub,
        // should borrow from a living lifetime
        typename = requires_true<all_alive(mut_borrow<mut_borrow<Super, Sub>, LT>{})>,
        // obtain mutable borrow count
        int N = current_count<Sub>(0),
        // if this value has not been borrowed as mutable
        typename = requires_true<N == 0>,
        // increase mutable borrow count
        size_t = sizeof(counter_inc<Sub, N, mut_borrow<Static, LT>>)
    >
    borrowed<T, mut_borrow<mut_borrow<Super, Sub>, LT>>
    borrow_mut(borrowed<T, mut_borrow<Super, Sub>> &ptr) {
        return {ptr.ref};
    }

    template <typename LT, typename T, typename Super, typename Sub,
        // should borrow from a living lifetime
        typename = requires_true<all_alive(mut_borrow<mut_borrow<Super, Sub>, LT>{})>,
        // obtain mutable borrow count
        int N = current_count<Sub>(0),
        // if this value has been borrowed as mutable before
        typename = requires_true<(N > 0)>,
        // the previous mutable borrows must be dead
        // only one mutable borrow at one time
        // error: cannot borrow `x` as mutable because it is already borrowed as mutable
        typename State = decltype(prev_lifetime(counter<Sub, N - 1>{})),
        typename = requires_false<any_alive(State{})>,
        // increase mutable borrow count
        size_t = sizeof(counter_inc<Sub, N, mut_borrow<Static, LT>>)
    >
    borrowed<T, mut_borrow<mut_borrow<Super, Sub>, LT>>
    borrow_mut(borrowed<T, mut_borrow<Super, Sub>> &ptr) {
        return {ptr.ref};
    }
}

#define NEW_LIFETIME(LT) struct LT { struct lifetime_info; };
#define DELETE_LIFETIME(LT) struct LT::lifetime_info { struct destroyed; };

int main() {
    using borrow::borrow;
    using borrow::borrow_mut;

    int value = 0;

    NEW_LIFETIME(L1);
    NEW_LIFETIME(L2);
    NEW_LIFETIME(L3);
    NEW_LIFETIME(L4);
    NEW_LIFETIME(L5);

    auto p1 = borrow_mut<L1>(value);
    auto p2_im = borrow<L1>(value);

    auto p2 = borrow_mut<L2>(p1);
    // cannot borrow `p1` as immutable because it also borrowed as mutable
//    auto p3_ = borrow<L2>(p1);

    // cannot borrow `p1` as mutable because it is already borrowed as mutable
//    auto p3__ = borrow_mut<L2>(p1); // will compile error

//    *p2_im = 1; // p2_im is borrowed as immutable
    DELETE_LIFETIME(L2);

    *p1 = 2;
//    *p2; // p2 outlives L2

    auto p4 = borrow_mut<L4>(p1);
    // can! borrow `p1` as immutable because it is also borrowed as mutable
//    auto p5 = borrow<L5>(p1); // will compile error
    *p4 += 1;

    DELETE_LIFETIME(L3);
    DELETE_LIFETIME(L4);
    DELETE_LIFETIME(L5);

    DELETE_LIFETIME(L1);
//    *p1 = 2; // p1 outlives L1
}

//
// Created by kiva on 2019/12/26.
//

#pragma once

#include <system_error>
#include <v9/kit/optional.hpp>
#include <v9/kit/function.hpp>

namespace v9::kit {
    template <typename A, typename B>
    class Either {
    private:
        Optional<A> _left;
        Optional<B> _right;

        explicit Either(Optional<A> &&data, std::true_type)
            : _left(std::forward<Optional<A>>(data)) {
        }

        explicit Either(Optional<B> &&data, std::false_type)
            : _right(std::forward<Optional<B>>(data)) {
        }

    public:
        static Either<A, B> left(A &&a) {
            return Either(Optional<A>(std::forward<A>(a)), std::true_type{});
        }

        static Either<A, B> right(B &&b) {
            return Either(Optional<B>(std::forward<B>(b)), std::false_type{});
        }

    public:
        Either() = delete;
        Either(const A &) = delete;
        Either(A &&) = delete;
        Either(const B &) = delete;
        Either(B &&) = delete;
        ~Either() = default;

        bool isLeft() const { return _left.hasValue(); }
        bool isRight() const { return _right.hasValue(); }

        Optional<A> &left() { return _left; }
        Optional<B> &right() { return _right; }

        const Optional<A> &left() const { return _left; }
        const Optional<B> &right() const { return _right; }
    };

    template <typename T>
    using ErrorOr = Either<std::error_code, T>;

    template <typename T>
    class Errors {
    public:
        static ErrorOr<T> error(std::error_code ec) {
            return Either<std::error_code, T>::left(ec);
        }

        static ErrorOr<T> error(int ec) {
            return Either<std::error_code, T>::left(
                std::error_code(ec, std::generic_category())
            );
        }

        static ErrorOr<T> ok(T &&t) {
            return Either<std::error_code, T>::right(std::forward<T>(t));
        }
    };
}

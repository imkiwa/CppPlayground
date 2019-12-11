//
// Created by kiva on 2019/12/7.
//

#pragma once

#include <array>
#include <utility>
#include <functional>

namespace v9::kit {
    template <typename T>
    class Optional {
    private:
        /**
         * One extra byte to store the status of this memory
         * memory[0] == true: this optional has a value
         * memory[0] == false: this optional has no value
         */
        std::array<unsigned char, 1 + sizeof(T)> _memory{0};

    public:
        static Optional<T> from(const T &t) {
            return Optional<T>(t);
        }

        static Optional<T> from(T &&t) {
            return Optional<T>(std::forward<T>(t));
        }

        template <typename ...Args>
        static Optional<T> emplace(Args &&...args) {
            return Optional<T>(T{std::forward<Args>(args)...});
        }

        static Optional<T> none() {
            return Optional<T>();
        }

    public:
        Optional() = default;

        explicit Optional(const T &t) {
            new(_memory.data() + 1) T(t);
            _memory[0] = static_cast<unsigned char>(true);
        }

        explicit Optional(T &&t) {
            new(_memory.data() + 1) T(std::forward<T>(t));
            _memory[0] = static_cast<unsigned char>(true);
        }

        Optional(const Optional<T> &other) {
            if (other.hasValue()) {
                new(_memory.data() + 1) T(other.get());
                _memory[0] = static_cast<unsigned char>(true);
            }
        }

        Optional(Optional<T> &&other) noexcept {
            swap(other);
        }

        ~Optional() {
            if (hasValue()) {
                ptr()->~T();
                _memory[0] = static_cast<unsigned char>(false);
            }
        }

        Optional &operator=(const Optional<T> &other) {
            if (this == &other) {
                return *this;
            }

            Optional<T> t(other);
            swap(t);
            return *this;
        }

        Optional &operator=(Optional<T> &&other) noexcept {
            if (this == &other) {
                return *this;
            }

            swap(other);
            return *this;
        }

        void swap(Optional<T> &&other) {
            std::swap(this->_memory, other._memory);
        }

        void swap(Optional<T> &other) {
            std::swap(this->_memory, other._memory);
        }

        T *ptr() {
            return hasValue() ? reinterpret_cast<T *>(_memory.data() + 1) : nullptr;
        }

        const T *ptr() const {
            return hasValue() ? reinterpret_cast<const T *>(_memory.data() + 1) : nullptr;
        }

        T &get() {
            return *ptr();
        }

        const T &get() const {
            return *ptr();
        }

        T &getOr(T &o) {
            if (ptr() == nullptr) {
                return o;
            }
            return *ptr();
        }

        const T &getOr(T &o) const {
            if (ptr() == nullptr) {
                return o;
            }
            return *ptr();
        }

        void apply(const std::function<void(T &)> &consumer) {
            if (ptr() != nullptr) {
                consumer(get());
            }
        }

        void apply(const std::function<void(const T &)> &consumer) const {
            if (ptr() != nullptr) {
                consumer(get());
            }
        }

        template <typename R>
        R applyOr(const R &r, const std::function<R(T &)> &consumer) {
            if (ptr() != nullptr) {
                return consumer(get());
            }
            return r;
        }

        template <typename R>
        R applyOr(const R &r, const std::function<R(const T &)> &consumer) const {
            if (ptr() != nullptr) {
                return consumer(get());
            }
            return r;
        }

        bool hasValue() const {
            return static_cast<bool>(_memory[0]);
        }
    };
}


//
// Created by kiva on 2019/12/8.
//

#pragma once

#include <cstdlib>
#include <string>
#include <functional>
#include <list>
#include <unordered_map>

namespace v9::kit {
    class EventEmitter {
    private:
        /**
         * An alias for function implementation.
         * In case that we need to use our own function
         * in the future.
         */
        template <typename T>
        using FunctionAlias = std::function<T>;

        template <typename ...Args>
        struct ArgsChecker {
            static constexpr size_t count = ArgsChecker<Args...>::count;
        };

        template <typename A, typename ...Args>
        struct ArgsChecker<A, Args...> {
            static constexpr size_t count = 1 + ArgsChecker<Args...>::count;
        };

        template <typename A>
        struct ArgsChecker<A> {
            static constexpr size_t count = 1;
        };

        template <typename Handler>
        struct FunctionParser : public FunctionParser<decltype(&Handler::operator())> {
        };

        template <typename ClassType, typename R, typename... Args>
        struct FunctionParser<R(ClassType::*)(Args...) const> {
            using FunctionType = FunctionAlias<R(Args...)>;
            static constexpr size_t argsCount = ArgsChecker<Args...>::count;
        };

        template <typename Handler>
        using FunctionType = typename FunctionParser<Handler>::FunctionType;

        template <typename Handler>
        static FunctionType<Handler> makeFunction(Handler &cb) {
            return static_cast<FunctionType<Handler>>(cb);
        }

        class HandlerContainer {
        private:
            size_t _argsCount = 0;
            std::shared_ptr<char> _handler;

        public:
            template <typename Handler>
            explicit HandlerContainer(Handler &&handler) {
                using WrapperType = decltype(makeFunction(handler));

                auto *m = std::malloc(sizeof(WrapperType));
                if (m == nullptr) {
                    // should panic oom
                    return;
                }

                // generate the handler wrapper dynamically according to
                // the callback type, so we can pass varied and arbitrary
                // count of arguments to trigger the event handler.
                auto fn = new(m) WrapperType(makeFunction(handler));

                // store argument count for call-time type check.
                _argsCount = FunctionParser<WrapperType>::argsCount;

                // use std::shared_ptr to manage the allocated memory
                // (char *) and (void *) are known as universal pointers.
                _handler = std::shared_ptr<char>(
                    // wrapper function itself
                    reinterpret_cast<char *>(fn),

                    // wrapper function deleter, responsible to call destructor
                    [](char *ptr) {
                        if (ptr != nullptr) {
                            reinterpret_cast<WrapperType *>(ptr)->~WrapperType();
                            std::free(ptr);
                        }
                    }
                );
            }

            template <typename T>
            FunctionAlias<T> *callablePtr() {
                if (_argsCount == FunctionParser<FunctionAlias<T>>::argsCount) {
                    return reinterpret_cast<FunctionAlias<T> *>(_handler.get());
                }
                return nullptr;
            }

            template <>
            FunctionAlias<void()> *callablePtr() {
                if (_argsCount == 0) {
                    return reinterpret_cast<FunctionAlias<void()> *>(_handler.get());
                }
                return nullptr;
            }
        };

    private:
        std::unordered_map<std::string, std::list<HandlerContainer>> _event;

    public:
        EventEmitter() = default;

        ~EventEmitter() = default;

        template <typename Handler>
        void on(const std::string &name, Handler handler) {
            _event[name].push_back(HandlerContainer(handler));
        }

        void clearAllHandlers(const std::string &name) {
            auto it = _event.find(name);
            if (it != _event.end()) {
                _event.erase(it);
            }
        }

        template <typename ...Args>
        void emit(const std::string &name, Args ...args) {
            auto it = _event.find(name);
            if (it == _event.end()) {
                return;
            }

            for (auto &&fn : it->second) {
                auto handler = fn.callablePtr<void(Args...)>();
                assert(handler && "Invalid call to event handler: mismatched argument count");
                (*handler)(args...);
            }
        }

        void emit(const std::string &name) {
            auto it = _event.find(name);
            if (it == _event.end()) {
                return;
            }

            for (auto &&fn : it->second) {
                auto handler = fn.callablePtr<void()>();
                assert(handler && "Invalid call to event handler: mismatched argument count");
                (*handler)();
            }
        }
    };

}

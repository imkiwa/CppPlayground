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
        std::unordered_map<std::string, std::list<std::shared_ptr<char>>> _event;

        template <typename Handler>
        struct FunctionParser : public FunctionParser<decltype(&Handler::operator())> {
        };

        template <typename ClassType, typename R, typename... Args>
        struct FunctionParser<R(ClassType::*)(Args...) const> {
            using FunctionType = std::function<R(Args...)>;
        };

        template <typename Handler>
        using FunctionType = typename FunctionParser<Handler>::FunctionType;

        template <typename Handler>
        FunctionType<Handler> makeFunction(Handler &cb) {
            return static_cast<FunctionType<Handler>>(cb);
        }

    public:
        EventEmitter() = default;

        ~EventEmitter() = default;

        template <typename Handler>
        void on(const std::string &name, Handler handler) {
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

            // use std::shared_ptr to manage the allocated memory
            // (char *) and (void *) are known as universal pointers.
            _event[name].push_back(std::shared_ptr<char>(
                // wrapper function itself
                reinterpret_cast<char *>(fn),

                // wrapper function deleter, responsible to call destructor
                [](char *ptr) {
                    if (ptr != nullptr) {
                        reinterpret_cast<WrapperType *>(ptr)->~WrapperType();
                        std::free(ptr);
                    }
                }
            ));
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
            if (it != _event.end()) {
                auto &&callbacks = it->second;
                for (auto &&fn : callbacks) {
                    auto handler = reinterpret_cast<std::function<void(Args...)> *>(fn.get());
                    (*handler)(args...);
                }
            }
        }

        void emit(const std::string &name) {
            auto it = _event.find(name);
            if (it != _event.end()) {
                auto &&callbacks = it->second;
                for (auto &&fn : callbacks) {
                    auto handler = reinterpret_cast<std::function<void(void)> *>(fn.get());
                    (*handler)();
                }
            }
        }
    };

}

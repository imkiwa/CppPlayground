//
// Created by kiva on 2019/12/8.
//

#pragma once

#include <cstdlib>
#include <string>
#include <list>
#include <typeindex>
#include <functional>
#include <unordered_map>

#include <v9/kit/function.hpp>
#include <v9/kit/typelist.hpp>

namespace v9::kit {
    class HandlerContainer {
    private:
        size_t _argsCount = 0;
        std::type_index _argsInfo;
        std::shared_ptr<char> _handler;

    public:
        template <typename Handler>
        explicit HandlerContainer(Handler &&handler)
            :_argsInfo(typeid(void)) {
            // handler-dependent types
            using WrapperType = decltype(makeFunction(handler));
            using ArgTypes = typename FunctionParser<WrapperType>::PureArgsTypes;

            auto *m = std::malloc(sizeof(WrapperType));
            if (m == nullptr) {
                // should panic oom
                return;
            }

            // generate the handler wrapper dynamically according to
            // the callback type, so we can pass varied and arbitrary
            // count of arguments to trigger the event handler.
            auto fn = new(m) WrapperType(makeFunction(handler));

            // store argument info for call-time type check.
            _argsCount = TypeList::size<ArgTypes>::value;
            _argsInfo = typeid(ArgTypes);

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
            using ArgTypes = typename FunctionParser<FunctionAlias<T>>::PureArgsTypes;
            if (_argsInfo == typeid(ArgTypes)) {
                return reinterpret_cast<FunctionAlias<T> *>(_handler.get());
            }
            return nullptr;
        }

        template <>
        FunctionAlias<void()> *callablePtr() {
            // Directly check argument count
            // because _argsInfo == typeid(TypeList::Empty) is much slower here.
            if (_argsCount == 0) {
                return reinterpret_cast<FunctionAlias<void()> *>(_handler.get());
            }
            return nullptr;
        }
    };

    class EventEmitter {
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
                assert(handler && "Invalid call to event handler: mismatched argument list");
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
                assert(handler && "Invalid call to event handler: mismatched argument list");
                (*handler)();
            }
        }
    };

}

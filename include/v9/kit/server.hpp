//
// Created by kiva on 2019/12/8.
//

#pragma once

#include <string>
#include <unordered_map>
#include <v9/bits/traits.hpp>
#include <v9/kit/event.hpp>
#include <v9/kit/optional.hpp>

namespace v9::kit {
    /**
     * Epoll based high performance IO server.
     *
     * @tparam E should derive from EventEmitter and have a default constructor
     */
    template <typename E,
        typename = typename std::enable_if<std::is_default_constructible_v<E>, E>::type,
        typename = typename std::enable_if<InstanceOf_v<E, EventEmitter>, E>::type
    >
    class IOServer {
    private:
        std::unordered_map<std::string, Optional<E>> _connections;

    public:
        IOServer() = default;

        ~IOServer() = default;

        IOServer(const IOServer &&) = delete;

        IOServer(IOServer &&other) noexcept
            : _connections(std::move(other._connections)) {
        }

        IOServer &operator=(const IOServer &) = delete;

        IOServer &operator=(IOServer &&other) noexcept {
            if (this != &other) {
                this->_connections = std::move(other._connections);
            }
            return *this;
        }

    public:
    };
}

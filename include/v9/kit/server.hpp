//
// Created by kiva on 2019/12/8.
//

#pragma once

#include <string>
#include <unordered_map>
#include <v9/kit/event.hpp>
#include <v9/kit/optional.hpp>

namespace v9::kit {
    /**
     * Epoll based high performance IO server.
     *
     * @tparam K Key of the connection pool.
     * @tparam E should derive from EventEmitter and have a default constructor
     */
    template <typename K, typename E,
        // E should have a default constructor
        typename = typename std::enable_if<std::is_default_constructible_v<E>, E>::type,

        // E should derive from EventEmitter
        typename = typename std::enable_if<std::is_base_of_v<EventEmitter, E>, E>::type
    >
    class IOServer {
    private:
        std::unordered_map<K, Optional<E>> _connections;

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
        Optional<E> &openNew(const K &k) {
            _connections[k] = Optional<E>::from(E());
            return _connections[k];
        }

        Optional<E> &open(const K &k) {
            auto it = _connections.find(k);
            if (it != _connections.end()) {
                return it->second;
            }

            return openNew(k);
        }
    };
}

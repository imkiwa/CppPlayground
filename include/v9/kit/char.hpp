//
// Created by kiva on 2019/12/13.
//

#pragma once

#include <cstdint>

namespace v9::kit {
    class Char {
    private:
        uint32_t _char{0};

    public:
        /*explicit*/ Char(char c) : _char(c) {}

        /*explicit*/ Char(char32_t c) : _char(c) {}

        /*explicit*/ Char(int c) : _char(c) {}

        uint32_t toUnicode() const { return _char; }

        Char &operator=(char c) {
            this->_char = c;
            return *this;
        }

        Char &operator=(char32_t c) {
            this->_char = c;
            return *this;
        }

        Char &operator=(int c) {
            this->_char = c;
            return *this;
        }

        Char &operator=(const Char &c) {
            if (this == &c) { return *this; }
            this->_char = c._char;
            return *this;
        }

        bool operator==(const Char &other) const { return this->_char == other._char; }

        bool operator!=(const Char &other) const { return this->_char != other._char; }

        bool operator>=(const Char &other) const { return this->_char >= other._char; }

        bool operator<=(const Char &other) const { return this->_char <= other._char; }

        bool operator<(const Char &other) const { return this->_char < other._char; }

        bool operator>(const Char &other) const { return this->_char > other._char; }
    };
}

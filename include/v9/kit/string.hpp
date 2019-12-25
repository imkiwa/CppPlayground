#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <bitset>
#include <functional>
#include <v9/kit/stream.hpp>

namespace v9::kit {
    /**
     * Represent a constant reference to a string, i.e. a character
     * array and a length, which need not be null terminated.
     *
     * This class does not own the string data, it is expected to be used in
     * situations where the character data resides in some other buffer, whose
     * lifetime extends past that of the StringRef. For this reason, it is not in
     * general safe to store a StringRef.
     */
    class StringRef {
    public:
        static constexpr size_t npos = ~size_t(0);

        using Iterator = const char *;
        using ConstIterator = const char *;
        using SizeType = size_t;

    private:
        /**
         * The start of the string, in an external buffer.
         */
        const char *_data = nullptr;

        /**
         * The length of the string.
         */
        size_t _length = 0;

        /**
         * Workaround memcmp issue with null pointers (undefined behavior)
         * by providing a specialized version.
         *
         * @param lhs
         * @param rhs
         * @param length
         * @return
         */
        static int safeMemcmp(const char *lhs, const char *rhs, size_t length) {
            if (length == 0) { return 0; }
            return ::memcmp(lhs, rhs, length);
        }

        /**
         * Constexpr version of std::strlen.
         */
        static size_t stringLength(const char *str) {
            return std::char_traits<char>::length(str);
        }

        static int ascii_strncasecmp(const char *lhs, const char *rhs, size_t length) {
            for (size_t index = 0; index < length; ++index) {
                unsigned char lw = std::tolower(lhs[index]);
                unsigned char rw = std::tolower(rhs[index]);
                if (lw != rw) {
                    return lw < rw ? -1 : 1;
                }
            }
            return 0;
        }

    public:
        /**
         * Wrap a string.
         * If data is nullptr, this function will wrap an empty string.
         *
         * @param data string
         * @return StringRef
         */
        static StringRef with(const char *data) {
            return StringRef{data ? data : ""};
        }

    public:
        char operator[](size_t index) const {
            // TODO: replace with exception handling system.
            assert(index < _length && "Invalid index!");
            return _data[index];
        }

        /**
         * Disallow accidental assignment from a temporary std::string.
         * The declaration here is extra complicated so that `StringRef = {}`
         * and `StringRef = "abc"` continue to select the move assignment operator.
         *
         * @tparam T
         * @return
         */
        template <typename T>
        std::enable_if_t<std::is_same<T, std::string>::value, StringRef> &
        operator=(T &&) = delete;

        /*implicit*/ operator std::string() const {
            return str();
        }

    public:
        /*implicit*/ StringRef() = default;

        StringRef(std::nullptr_t) = delete;

        /*implicit*/ constexpr StringRef(const char *str)
            : _data(str), _length(str ? stringLength(str) : 0) {}

        /*implicit*/ constexpr StringRef(const char *data, size_t length)
            : _data(data), _length(length) {}

        /*implicit*/ StringRef(const std::string &str)
            : _data(str.data()), _length(str.length()) {}

        Iterator begin() const { return _data; }

        Iterator end() const { return _data + _length; }

        const unsigned char *bytesBegin() const {
            return reinterpret_cast<const unsigned char *>(begin());
        }

        const unsigned char *bytesEnd() const {
            return reinterpret_cast<const unsigned char *>(end());
        }

        /**
         * Get a pointer to the start of the string (which may not be null
         * terminated).
         *
         * @return char array
         */
        const char *data() const { return _data; }

        /**
         * Check if the string is empty.
         *
         * @return is empty?
         */
        bool empty() const { return _length == 0; }

        /**
         * Get the length of the string.
         *
         * @return length
         */
        size_t size() const { return _length; }

        /**
         * Get the first character in the string.
         *
         * @return the first char
         */
        char front() const {
            // TODO: replace with exception handling system.
            assert(!empty());
            return _data[0];
        }

        /**
         * Get the last character in the string.
         *
         * @return the last char
         */
        char back() const {
            // TODO: replace with exception handling system.
            assert(!empty());
            return _data[_length - 1];
        }

        /**
         * Allocate copy in Allocator and return StringRef to it.
         * @tparam Allocator Allocator Type
         * @param allocator allocator
         * @return copied StringRef(with data)
         */
        template <typename Allocator>
        StringRef copy(Allocator &allocator) const {
            if (empty()) {
                return StringRef{};
            }
            char *data = allocator.template allocate<char>(_length);
            std::copy(begin(), end(), data);
            return StringRef{data, _length};
        }

        /**
         * Check for string equality.
         *
         * @param rhs the other string
         * @return is equal?
         */
        bool equals(StringRef rhs) const {
            return (_length == rhs._length &&
                    safeMemcmp(_data, rhs._data, rhs._length) == 0);
        }

        /**
         * Check for string equality, case insensitively.
         *
         * @param rhs
         * @return is equal case insensitively?
         */
        bool equalsIgnoreCase(StringRef rhs) const {
            return _length == rhs._length && compareIgnoreCase(rhs) == 0;
        }

        /**
         * Compare two strings.
         * The result is -1, 0, or 1 if this string is lexicographically
         * less than, equal to, or greater than the rhs.
         *
         * @param rhs
         * @return -1, 0 or 1
         */
        int compare(StringRef rhs) const {
            // Check the prefix for a mismatch.
            int r = safeMemcmp(_data, rhs._data, std::min(_length, rhs._length));
            if (r) {
                return r < 0 ? -1 : 1;
            }

            // Otherwise the prefixes match, so we only need to check the lengths.
            if (_length == rhs._length) {
                return 0;
            }
            return _length < rhs._length ? -1 : 1;
        }

        /**
         * Compare two strings, case insensitively.
         * @param rhs
         * @return {@see StringRef::compare(StringRef)}
         */
        int compareIgnoreCase(StringRef rhs) const {
            int r = ascii_strncasecmp(_data, rhs._data, std::min(_length, rhs._length));
            if (r) {
                return r;
            }

            if (_length == rhs._length) {
                return 0;
            }
            return _length < rhs._length ? -1 : 1;
        }

        /**
         * Compare two strings, treating sequences of digits as numbers.
         * @param rhs
         * @return
         */
        int compareNumeric(StringRef rhs) const {
            for (size_t i = 0, end = std::min(_length, rhs._length); i != end; ++i) {
                // Check for sequences of digits.
                if (std::isdigit(_data[i]) && std::isdigit(rhs._data[i])) {
                    // The longer sequence of numbers is considered larger.
                    // This doesn't really handle prefixed zeros well.
                    size_t j = 0;
                    for (j = i + 1; j != end + 1; ++j) {
                        bool ld = j < _length && std::isdigit(_data[j]);
                        bool rd = j < rhs._length && std::isdigit(rhs._data[j]);
                        if (ld != rd) {
                            return rd ? -1 : 1;
                        }
                        if (!rd) {
                            break;
                        }
                    }

                    // The two number sequences have the same length (j-i), just memcmp them.
                    int r = safeMemcmp(_data + i, rhs._data + i, j - i);
                    if (r) {
                        return r < 0 ? -1 : 1;
                    }
                    // Identical number sequences, continue search after the numbers.
                    i = j - 1;
                    continue;
                }

                if (_data[i] != rhs._data[i]) {
                    return (unsigned char) _data[i] < (unsigned char) rhs._data[i] ? -1 : 1;
                }
            }

            if (_length == rhs._length) {
                return 0;
            }
            return _length < rhs._length ? -1 : 1;
        }

        /**
         * Get the contents as an std::string.
         * @return
         */
        std::string str() const {
            if (!_data) { return std::string(); }
            return std::string(_data, _length);
        }

        /**
         * Check if this string starts with the given prefix.
         * @param prefix
         * @return
         */
        bool startsWith(StringRef prefix) const {
            return _length >= prefix._length &&
                   safeMemcmp(_data, prefix._data, prefix._length) == 0;
        }

        /**
         * Check if this string starts with the given prefix, case insensitively.
         * @param prefix
         * @return
         */
        bool startsWithIgnoreCase(StringRef prefix) const {
            return _length >= prefix._length &&
                   ascii_strncasecmp(_data, prefix._data, prefix._length) == 0;
        }

        /**
         * Check if this string ends with the given suffix.
         * @param suffix
         * @return
         */
        bool endsWith(StringRef suffix) const {
            return _length >= suffix._length &&
                   safeMemcmp(end() - suffix._length, suffix._data, suffix._length) == 0;
        }

        /**
         * Check if this string ends with the given prefix, case insensitively.
         * @param prefix
         * @return
         */
        bool endsWithIgnoreCase(StringRef suffix) const {
            return _length >= suffix._length &&
                   ascii_strncasecmp(end() - suffix._length, suffix._data, suffix._length) == 0;
        }

        /**
         * Search for the first character c in the string.
         * @param c
         * @param start_index
         * @return index of the first c, or npos if not found
         */
        size_t find(char c, size_t start_index = 0) const {
            size_t pos = std::min(start_index, _length);
            if (pos < _length) {
                // Avoid calling memchr with nullptr.
                // Just forward to memchr, which is faster than a hand-rolled loop.
                if (const void *s = ::memchr(_data + pos, c, _length - pos)) {
                    return static_cast<const char *>(s) - _data;
                }
            }
            return npos;
        }

        /**
         * Search for the first character c in the string, case insensitively.
         * @param c
         * @param start_index
         * @return index of the first c, or npos if not found
         */
        size_t findIgnoreCase(char c, size_t start_index = 0) const {
            int lc = std::tolower(c);
            return findIf(
                [lc](char d) { return std::tolower(d) == lc; },
                start_index
            );
        }

        /**
         * Search for the first character satisfying the predicate f
         * @param f
         * @param start_index
         * @return the position or npos
         */
        size_t findIf(const std::function<bool(char)> &f, size_t start_index = 0) const {
            StringRef s = dropFront(start_index);
            while (!s.empty()) {
                if (f(s.front())) {
                    return size() - s.size();
                }
                s = s.dropFront();
            }
            return npos;
        }

        /**
         * Search for the first character not satisfying the predicate f
         * @param f
         * @param start_index
         * @return
         */
        size_t findIfNot(const std::function<bool(char)> &f, size_t start_index = 0) const {
            return findIf(
                [&f](char c) { return !f(c); },
                start_index
            );
        }

        size_t find(StringRef str, size_t start_index = 0) const {
            if (start_index > _length) {
                return npos;
            }

            const char *start = _data + start_index;
            size_t size = _length - start_index;

            const char *needle = str.data();
            size_t N = str.size();
            if (N == 0) {
                return start_index;
            }
            if (size < N) {
                return npos;
            }
            if (N == 1) {
                const char *p = (const char *) ::memchr(start, needle[0], size);
                return p == nullptr ? npos : p - _data;
            }

            const char *end = start + (size - N + 1);

            // For short haystacks or unsupported needles fall back to the naive algorithm
            if (size < 16 || N > 255) {
                do {
                    if (std::memcmp(start, needle, N) == 0)
                        return start - _data;
                    ++start;
                } while (start < end);
                return npos;
            }

            // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
            uint8_t skipped[256];
            std::memset(skipped, N, 256);
            for (unsigned i = 0; i != N - 1; ++i) {
                skipped[(uint8_t) str[i]] = N - 1 - i;
            }

            do {
                uint8_t last = start[N - 1];
                if (last == (uint8_t) needle[N - 1]
                    && std::memcmp(start, needle, N - 1) == 0) {
                    return start - _data;
                }
                // Otherwise skip the appropriate number of bytes.
                start += skipped[last];
            } while (start < end);

            return npos;
        }

        size_t findIgnoreCase(StringRef str, size_t start_index = 0) const {
            StringRef _this = substr(start_index);
            while (_this.size() >= str.size()) {
                if (_this.startsWithIgnoreCase(str)) {
                    return start_index;
                }
                _this = _this.dropFront();
                ++start_index;
            }
            return npos;
        }

        size_t rfind(char c, size_t start_index = npos) const {
            start_index = std::min(start_index, _length);
            size_t i = start_index;
            while (i != 0) {
                --i;
                if (_data[i] == c) {
                    return i;
                }
            }
            return npos;
        }

        size_t rfindIgnoreCase(char c, size_t start_index = npos) const {
            start_index = std::min(start_index, _length);
            size_t i = start_index;
            while (i != 0) {
                --i;
                if (std::tolower(_data[i]) == std::tolower(c)) {
                    return i;
                }
            }
            return npos;
        }

        size_t rfind(StringRef str) const {
            size_t N = str.size();
            if (N > _length) {
                return npos;
            }
            for (size_t i = _length - N + 1; i != 0;) {
                --i;
                if (substr(i, N).equals(str)) {
                    return i;
                }
            }
            return npos;
        }

        size_t rfindIgnoreCase(StringRef str) const {
            size_t N = str.size();
            if (N > _length) {
                return npos;
            }
            for (size_t i = _length - N + 1; i != 0;) {
                --i;
                if (substr(i, N).equalsIgnoreCase(str)) {
                    return i;
                }
            }
            return npos;
        }

        size_t findFirstOf(char C, size_t From = 0) const {
            return find(C, From);
        }

        size_t findFirstOf(StringRef chars, size_t start_index = 0) const {
            std::bitset<1 << CHAR_BIT> char_bits;
            for (SizeType i = 0; i != chars.size(); ++i) {
                char_bits.set((unsigned char) chars[i]);
            }

            for (SizeType i = std::min(start_index, _length); i != _length; ++i) {
                if (char_bits.test((unsigned char) _data[i])) {
                    return i;
                }
            }
            return npos;
        }

        size_t findFirstNotOf(char c, size_t start_index = 0) const {
            for (SizeType i = std::min(start_index, _length); i != _length; ++i) {
                if (_data[i] != c) {
                    return i;
                }
            }
            return npos;
        }

        size_t findFirstNotOf(StringRef chars, size_t start_index = 0) const {
            std::bitset<1 << CHAR_BIT> char_bits;
            for (SizeType i = 0; i != chars.size(); ++i) {
                char_bits.set((unsigned char) chars[i]);
            }

            for (SizeType i = std::min(start_index, _length); i != _length; ++i) {
                if (!char_bits.test((unsigned char) _data[i])) {
                    return i;
                }
            }
            return npos;
        }

        size_t findLastOf(char c, size_t start_index = npos) const {
            return rfind(c, start_index);
        }

        size_t findLastOf(StringRef chars, size_t start_index = npos) const {
            std::bitset<1 << CHAR_BIT> char_bits;
            for (SizeType i = 0; i != chars.size(); ++i) {
                char_bits.set((unsigned char) chars[i]);
            }

            for (SizeType i = std::min(start_index, _length) - 1; i != -1; --i) {
                if (char_bits.test((unsigned char) _data[i])) {
                    return i;
                }
            }
            return npos;
        }

        size_t findLastNotOf(char c, size_t start_index = npos) const {
            for (SizeType i = std::min(start_index, _length) - 1; i != -1; --i) {
                if (_data[i] != c) {
                    return i;
                }
            }
            return npos;
        }

        size_t findLastNotOf(StringRef chars, size_t start_index = npos) const {
            std::bitset<1 << CHAR_BIT> char_bits;
            for (SizeType i = 0, e = chars.size(); i != e; ++i) {
                char_bits.set((unsigned char) chars[i]);
            }

            for (SizeType i = std::min(start_index, _length) - 1; i != -1; --i) {
                if (!char_bits.test((unsigned char) _data[i])) {
                    return i;
                }
            }
            return npos;
        }

        bool contains(StringRef other) const { return find(other) != npos; }

        bool contains(char c) const { return find_first_of(c) != npos; }

        bool containsIgnoreCase(StringRef other) const {
            return findIgnoreCase(other) != npos;
        }

        bool containsIgnoreCase(char c) const { return findIgnoreCase(c) != npos; }

        size_t count(char c) const {
            size_t count = 0;
            for (size_t i = 0; i != _length; ++i) {
                if (_data[i] == c) {
                    ++count;
                }
            }
            return count;
        }

        size_t count(StringRef str) const {
            size_t count = 0;
            size_t N = str.size();
            if (N > _length) {
                return 0;
            }
            for (size_t i = 0, end = _length - N + 1; i != end; ++i) {
                if (substr(i, N).equals(str)) {
                    ++count;
                }
            }
            return count;
        }

        // Convert the given ASCII string to lowercase.
        std::string lower() const {
            std::string result(size(), char());
            for (SizeType i = 0, e = size(); i != e; ++i) {
                result[i] = std::tolower(_data[i]);
            }
            return result;
        }

        /// Convert the given ASCII string to uppercase.
        std::string upper() const {
            std::string result(size(), char());
            for (SizeType i = 0, e = size(); i != e; ++i) {
                result[i] = std::toupper(_data[i]);
            }
            return result;
        }

        /**
         * Return a reference to the substring from [start_index, start_index + N).
         *
         * @param start_index The index of the starting character in the substring; if
         * the index is npos or greater than the length of the string then the
         * empty substring will be returned.
         * @param N The number of characters to included in the substring. If N
         * exceeds the number of characters remaining in the string, the string
         * suffix (starting with start_index) will be returned.
         *
         * @return
         */
        StringRef substr(size_t start_index, size_t N = npos) const {
            start_index = std::min(start_index, _length);
            return StringRef{_data + start_index,
                             std::min(N, _length - start_index)};
        }

        /**
         * Return a StringRef equal to 'this' but with only the first N
         * elements remaining.  If N is greater than the length of the
         * string, the entire string is returned.
         *
         * @param N
         * @return
         */
        StringRef takeFront(size_t N = 1) const {
            if (N >= size())
                return *this;
            return dropBack(size() - N);
        }

        /**
         * Return a StringRef equal to 'this' but with only the last \p N
         * elements remaining.  If N is greater than the length of the
         * string, the entire string is returned.
         *
         * @param N
         * @return
         */
        StringRef takeBack(size_t N = 1) const {
            if (N >= size())
                return *this;
            return dropFront(size() - N);
        }

        /**
         * Return the longest prefix of 'this' such that every character
         * in the prefix satisfies the given predicate.
         *
         * @param f
         * @return
         */
        StringRef takeWhile(const std::function<bool(char)> &f) const {
            return substr(0, findIfNot(f));
        }

        /**
         * Return the longest prefix of 'this' such that no character in
         * the prefix satisfies the given predicate.
         *
         * @param f
         * @return
         */
        StringRef takeUntil(const std::function<bool(char)> &f) const {
            return substr(0, findIf(f));
        }

        /**
         * Return a StringRef equal to 'this' but with the first N elements
         * dropped.
         *
         * @param N
         * @return
         */
        StringRef dropFront(size_t N = 1) const {
            // TODO: replace with exception handling system.
            assert(size() >= N && "Dropping more elements than exist");
            return substr(N);
        }

        /**
         * Return a StringRef equal to 'this' but with the last N elements
         * dropped.
         *
         * @param N
         * @return
         */
        StringRef dropBack(size_t N = 1) const {
            // TODO: replace with exception handling system.
            assert(size() >= N && "Dropping more elements than exist");
            return substr(0, size() - N);
        }

        /**
         * Return a StringRef equal to 'this', but with all characters satisfying
         * the given predicate dropped from the beginning of the string.
         *
         * @param f
         * @return
         */
        StringRef dropWhile(const std::function<bool(char)> &f) const {
            return substr(findIfNot(f));
        }

        /**
         * Return a StringRef equal to 'this', but with all characters not
         * satisfying the given predicate dropped from the beginning of the string.
         *
         * @param f
         * @return
         */
        StringRef dropUntil(const std::function<bool(char)> &f) const {
            return substr(findIf(f));
        }

        /**
         * Return a reference to the substring from [start, end).
         * @param start The index of the starting character in the substring; if
         * the index is npos or greater than the length of the string then the
         * empty substring will be returned.
         *
         * @param end The index following the last character to include in the
         * substring. If this is npos or exceeds the number of characters
         * remaining in the string, the string suffix (starting with start)
         * will be returned. If this is less than start, an empty string will
         * be returned.
         *
         * @return
         */
        StringRef slice(size_t start, size_t end) const {
            start = std::min(start, _length);
            end = std::min(std::max(start, end), _length);
            return StringRef{_data + start, end - start};
        }

        /**
         * Split into two substrings around the first occurrence of a separator
         * character.
         *
         * If separator is in the string, then the result is a pair (LHS, RHS)
         * such that (*this == LHS + separator + RHS) is true and RHS is
         * maximal. If separator is not in the string, then the result is a
         * pair (LHS, RHS) where (*this == LHS) and (RHS == "").
         *
         * @param separator
         * @return
         */
        std::pair<StringRef, StringRef> split(char separator) const {
            return split(StringRef(&separator, 1));
        }

        std::pair<StringRef, StringRef> split(StringRef separator) const {
            size_t index = find(separator);
            if (index == npos) {
                return std::make_pair(*this, StringRef());
            }
            return std::make_pair(slice(0, index),
                slice(index + separator.size(), npos));
        }

        /**
         * Split into two substrings around the last occurrence of a separator
         * character.
         *
         * If separator is in the string, then the result is a pair (LHS, RHS)
         * such that (*this == LHS + separator + RHS) is true and RHS is
         * minimal. If separator is not in the string, then the result is a
         * pair (LHS, RHS) where (*this == LHS) and (RHS == "").
         *
         * @param separator
         * @return
         */
        std::pair<StringRef, StringRef> rsplit(char separator) const {
            return rsplit(StringRef(&separator, 1));
        }

        std::pair<StringRef, StringRef> rsplit(StringRef separator) const {
            size_t index = rfind(separator);
            if (index == npos) {
                return std::make_pair(*this, StringRef());
            }
            return std::make_pair(slice(0, index),
                slice(index + separator.size(), npos));
        }

        /**
         * Split into substrings around the occurrences of a separator string.
         *
         * Each substring is stored in result. If max_split is >= 0, at most
         * max_split splits are done and consequently <= max_split + 1
         * elements are added to result.
         * If keep_empty is false, empty strings are not added to result. They
         * still count when considering max_split
         * An useful invariant is that
         * separator.join(result) == *this if max_split == -1 and keep_empty == true
         *
         * @param result Where to put the substrings.
         * @param separator The string to split on.
         * @param max_split  The maximum number of times the string is split.
         * @param keep_empty True if empty substring should be added.
         */
        void split(std::vector<StringRef> &result,
                   StringRef separator, int max_split = -1,
                   bool keep_empty = true) const {
            StringRef str = *this;

            while (max_split-- != 0) {
                size_t index = str.find(separator);
                if (index == npos) {
                    break;
                }

                // Push this split.
                if (keep_empty || index > 0)
                    result.push_back(str.slice(0, index));

                // Jump forward.
                str = str.slice(index + separator.size(), npos);
            }

            // Push the tail.
            if (keep_empty || !str.empty()) {
                result.push_back(str);
            }
        }

        void split(std::vector<StringRef> &result, char separator, int max_split = -1,
                   bool keep_empty = true) const {
            StringRef str = *this;

            while (max_split-- != 0) {
                size_t index = str.find(separator);
                if (index == npos) {
                    break;
                }

                // Push this split.
                if (keep_empty || index > 0) {
                    result.push_back(str.slice(0, index));
                }

                // Jump forward.
                str = str.slice(index + 1, npos);
            }

            // Push the tail.
            if (keep_empty || !str.empty()) {
                result.push_back(str);
            }
        }

        StringRef ltrim(char chars) const {
            return dropFront(std::min(_length, findFirstNotOf(chars)));
        }

        StringRef ltrim(StringRef chars = " \t\n\v\f\r") const {
            return dropFront(std::min(_length, findFirstNotOf(chars)));
        }

        StringRef rtrim(char chars) const {
            return dropBack(_length - std::min(_length, findLastNotOf(chars) + 1));
        }

        StringRef rtrim(StringRef chars = " \t\n\v\f\r") const {
            return dropBack(_length - std::min(_length, findLastNotOf(chars) + 1));
        }

        StringRef trim(char chars) const {
            return ltrim(chars).rtrim(chars);
        }

        StringRef trim(StringRef chars = " \t\n\v\f\r") const {
            return ltrim(chars).rtrim(chars);
        }

        Stream<char> stream() {
            std::deque<char> d;
            std::copy(begin(), end(), std::back_inserter(d));
            return Stream<char>::of(std::move(d));
        }
    };
}

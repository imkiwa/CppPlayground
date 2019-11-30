//
// Created by kiva on 2019/11/29.
//

#include <set>
#include <list>
#include <array>
#include <cerrno>
#include <vector>
#include <string>
#include <sstream>
#include <utility>
#include <unordered_map>

#include <sys/stat.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#pragma ide diagnostic ignored "modernize-use-nodiscard"

namespace cs::compiler {
    enum ByteOrder {
        BO_LITTLE_ENDIAN, BO_BIG_ENDIAN
    };

    struct SizedBuffer {
        size_t _size;
        size_t _used;
        uint8_t *_bytes;
    };

    class ByteBuffer {
    public:
        using byte = uint8_t;

        static constexpr float BUFFER_GROWTH_FACTOR = 1.5f;
        static constexpr size_t BUFFER_DEFAULT_LENGTH = 128;

    private:
        byte *_buffer;
        size_t _bufferLength;
        size_t _position;
        ByteOrder _byteOrder;

    private:
        void growBuffer();

        void growIfNeeded(size_t least);

        static void copy(const byte *from, byte *to, size_t count);

    public:
        ByteBuffer();

        explicit ByteBuffer(size_t initialLength);

        ~ByteBuffer();

        SizedBuffer getBuffer() {
            return {
                ._size = getLength(),
                ._used = getPosition(),
                ._bytes = _buffer,
            };
        }

        void setOrder(ByteOrder order) {
            this->_byteOrder = order;
        }

        ByteOrder getByteOrder() const {
            return _byteOrder;
        }

        size_t getLength() const {
            return _bufferLength;
        }

        size_t getPosition() const {
            return _position;
        }

        void rewind() {
            _position = 0;
        }

    private:
        int16_t readInt16AtLE(size_t index);

        int16_t readInt16AtBE(size_t index);

        int32_t readInt32AtLE(size_t index);

        int32_t readInt32AtBE(size_t index);

        int64_t readInt64AtLE(size_t index);

        int64_t readInt64AtBE(size_t index);

        float readFloatAtLE(size_t index);

        float readFloatAtBE(size_t index);

        double readDoubleAtLE(size_t index);

        double readDoubleAtBE(size_t index);

        void writeInt16AtLE(size_t index, int16_t value);

        void writeInt16AtBE(size_t index, int16_t value);

        void writeInt32AtLE(size_t index, int32_t value);

        void writeInt32AtBE(size_t index, int32_t value);

        void writeInt64AtLE(size_t index, int64_t value);

        void writeInt64AtBE(size_t index, int64_t value);

        void writeFloatAtLE(size_t index, float value);

        void writeFloatAtBE(size_t index, float value);

        void writeDoubleAtLE(size_t index, double value);

        void writeDoubleAtBE(size_t index, double value);

    public:
        size_t placeholderInt8() {
            int p = this->_position;
            writeInt8(0);
            return p;
        }

        size_t placeholderInt16() {
            int p = this->_position;
            writeInt16(0);
            return p;
        }

        size_t placeholderInt32() {
            int p = this->_position;
            writeInt32(0);
            return p;
        }

        size_t placeholderInt64() {
            int p = this->_position;
            writeInt64(0);
            return p;
        }

        size_t placeholderFloat() {
            int p = this->_position;
            writeFloat(0);
            return p;
        }

        size_t placeholderDouble() {
            int p = this->_position;
            writeDouble(0);
            return p;
        }

        size_t placeholderSkip(size_t count) {
            int p = this->_position;
            _position += count;
            return p;
        }

        int8_t readInt8() {
            return readInt8At(_position++);
        }

        int16_t readInt16() {
            auto r = readInt16At(_position);
            _position += 2;
            return r;
        }

        int32_t readInt32() {
            auto r = readInt32At(_position);
            _position += 4;
            return r;
        }

        int64_t readInt64() {
            auto r = readInt64At(_position);
            _position += 8;
            return r;
        }

        float readFloat() {
            auto r = readFloatAt(_position);
            _position += 4;
            return r;
        }

        double readDouble() {
            auto r = readDoubleAt(_position);
            _position += 8;
            return r;
        }

        std::string readString(size_t length) {
            const auto &r = readStringAt(_position, length);
            _position += length;
            return r;
        }

        int8_t readInt8At(size_t index);

        int16_t readInt16At(size_t index);

        int32_t readInt32At(size_t index);

        int64_t readInt64At(size_t index);

        float readFloatAt(size_t index);

        double readDoubleAt(size_t index);

        std::string readStringAt(size_t index, size_t length);

    public:
        void writeInt8(int8_t value) {
            writeInt8At(_position++, value);
        }

        void writeInt16(int16_t value) {
            writeInt16At(_position, value);
            _position += 2;
        }

        void writeInt32(int32_t value) {
            writeInt32At(_position, value);
            _position += 4;
        }

        void writeInt64(int64_t value) {
            writeInt64At(_position, value);
            _position += 8;
        }

        void writeFloat(float value) {
            writeFloatAt(_position, value);
            _position += 4;
        }

        void writeDouble(double value) {
            writeDoubleAt(_position, value);
            _position += 8;
        }

        void writeString(const std::string &value) {
            writeStringAt(_position, value);
            _position += value.length();
        }

        void writeInt8At(size_t index, int8_t value);

        void writeInt16At(size_t index, int16_t value);

        void writeInt32At(size_t index, int32_t value);

        void writeInt64At(size_t index, int64_t value);

        void writeFloatAt(size_t index, float value);

        void writeDoubleAt(size_t index, double value);

        void writeStringAt(size_t index, const std::string &value);
    };

    ByteBuffer::ByteBuffer()
        : ByteBuffer(BUFFER_DEFAULT_LENGTH) {
    }

    ByteBuffer::ByteBuffer(size_t initialLength)
        : _buffer(nullptr), _bufferLength(initialLength), _position(0),
          _byteOrder(ByteOrder::BO_BIG_ENDIAN) {
        growBuffer();
    }

    ByteBuffer::~ByteBuffer() {
        if (_buffer != nullptr) {
            free(_buffer);
        }
    }

    void ByteBuffer::copy(const ByteBuffer::byte *from, ByteBuffer::byte *to, size_t count) {
        memcpy(to, from, count);
    }

    void ByteBuffer::growBuffer() {
        if (_buffer == nullptr) {
            // The buffer has not been allocated
            _buffer = static_cast<byte *>(malloc(sizeof(_buffer[0]) * _bufferLength));

            if (!_buffer) {
                throw std::runtime_error("growBuffer(): malloc failed");
            }
            return;
        }

        // The buffer needs to be expanded
        size_t currentLength = _bufferLength == 1 ? 2 : _bufferLength;
        size_t fitSize = currentLength * BUFFER_GROWTH_FACTOR;
        byte *fitBuffer = static_cast<byte *>(realloc(_buffer, sizeof(_buffer[0]) * fitSize));

        if (fitBuffer != nullptr) {
            // New buffer successfully allocated, the original buffer
            // was freed by realloc()
            _buffer = fitBuffer;
            _bufferLength = fitSize;
        } else {
            // Nothing changed when allocation failed
            throw std::runtime_error("growBuffer(): realloc failed");
        }
    }

    void ByteBuffer::growIfNeeded(size_t least) {
        // Keep growing until current buffer can afford the required length.
        while (_position + least >= _bufferLength) {
            growBuffer();
        }
    }

    int8_t ByteBuffer::readInt8At(size_t index) {
        return _buffer[index];
    }

    int16_t ByteBuffer::readInt16At(size_t index) {
        return (_byteOrder == BO_BIG_ENDIAN ? readInt16AtBE(index) : readInt16AtLE(index));
    }

    int32_t ByteBuffer::readInt32At(size_t index) {
        return (_byteOrder == BO_BIG_ENDIAN ? readInt32AtBE(index) : readInt32AtLE(index));
    }

    int64_t ByteBuffer::readInt64At(size_t index) {
        return (_byteOrder == BO_BIG_ENDIAN ? readInt64AtBE(index) : readInt64AtLE(index));
    }

    float ByteBuffer::readFloatAt(size_t index) {
        return (_byteOrder == BO_BIG_ENDIAN ? readFloatAtBE(index) : readFloatAtLE(index));
    }

    double ByteBuffer::readDoubleAt(size_t index) {
        return (_byteOrder == BO_BIG_ENDIAN ? readDoubleAtBE(index) : readDoubleAtLE(index));
    }

    std::string ByteBuffer::readStringAt(size_t index, size_t length) {
        std::stringstream ss;
        for (size_t i = 0; i < length; ++i) {
            ss << _buffer[index++];
        }
        return ss.str();
    }

    void ByteBuffer::writeInt8At(size_t index, int8_t value) {
        growIfNeeded(1);
        _buffer[index] = value;
    }

    void ByteBuffer::writeInt16At(size_t index, int16_t value) {
        growIfNeeded(2);
        (_byteOrder == BO_BIG_ENDIAN) ? writeInt16AtBE(index, value) : writeInt16AtLE(index, value);
    }

    void ByteBuffer::writeInt32At(size_t index, int32_t value) {
        growIfNeeded(4);
        (_byteOrder == BO_BIG_ENDIAN) ? writeInt32AtBE(index, value) : writeInt32AtLE(index, value);
    }

    void ByteBuffer::writeInt64At(size_t index, int64_t value) {
        growIfNeeded(8);
        (_byteOrder == BO_BIG_ENDIAN) ? writeInt64AtBE(index, value) : writeInt64AtLE(index, value);
    }

    void ByteBuffer::writeFloatAt(size_t index, float value) {
        growIfNeeded(4);
        (_byteOrder == BO_BIG_ENDIAN) ? writeFloatAtBE(index, value) : writeFloatAtLE(index, value);
    }

    void ByteBuffer::writeDoubleAt(size_t index, double value) {
        growIfNeeded(8);
        (_byteOrder == BO_BIG_ENDIAN) ? writeDoubleAtBE(index, value) : writeDoubleAtLE(index, value);
    }

    void ByteBuffer::writeStringAt(size_t index, const std::string &value) {
        growIfNeeded(value.length());
        copy(reinterpret_cast<const byte *>(value.c_str()), _buffer + index, value.length());
    }

    ///////////////////////////////////////////////////////////////////////////////
    int16_t ByteBuffer::readInt16AtLE(size_t index) {
        byte b1 = _buffer[index + 1];
        byte b0 = _buffer[index];

        return ((b1 & 0xFF) << 8) | (b0 & 0xFF);
    }

    int16_t ByteBuffer::readInt16AtBE(size_t index) {
        byte b0 = _buffer[index + 1];
        byte b1 = _buffer[index];

        return ((b1 & 0xFF) << 8) | (b0 & 0xFF);
    }

    int32_t ByteBuffer::readInt32AtLE(size_t index) {
        byte b3 = _buffer[index + 3];
        byte b2 = _buffer[index + 2];
        byte b1 = _buffer[index + 1];
        byte b0 = _buffer[index];

        return (b3 << 24) | ((b2 & 0xFF) << 16) | ((b1 & 0xFF) << 8) | (b0 & 0xFF);
    }

    int32_t ByteBuffer::readInt32AtBE(size_t index) {
        byte b0 = _buffer[index + 3];
        byte b1 = _buffer[index + 2];
        byte b2 = _buffer[index + 1];
        byte b3 = _buffer[index];

        return (b3 << 24) | ((b2 & 0xFF) << 16) | ((b1 & 0xFF) << 8) | (b0 & 0xFF);
    }

    int64_t ByteBuffer::readInt64AtLE(size_t index) {
        byte b7 = _buffer[index + 7];
        byte b6 = _buffer[index + 6];
        byte b5 = _buffer[index + 5];
        byte b4 = _buffer[index + 4];
        byte b3 = _buffer[index + 3];
        byte b2 = _buffer[index + 2];
        byte b1 = _buffer[index + 1];
        byte b0 = _buffer[index];

        return (((int64_t) b7 & 0xFF) << 56) | (((int64_t) b6 & 0xFF) << 48) |
               (((int64_t) b5 & 0xFF) << 40) | (((int64_t) b4 & 0xFF) << 32) |
               (((int64_t) b3 & 0xFF) << 24) | (((int64_t) b2 & 0xFF) << 16) |
               (((int64_t) b1 & 0xFF) << 8) | ((int64_t) b0 & 0xFF);
    }

    int64_t ByteBuffer::readInt64AtBE(size_t index) {
        byte b0 = _buffer[index + 7];
        byte b1 = _buffer[index + 6];
        byte b2 = _buffer[index + 5];
        byte b3 = _buffer[index + 4];
        byte b4 = _buffer[index + 3];
        byte b5 = _buffer[index + 2];
        byte b6 = _buffer[index + 1];
        byte b7 = _buffer[index];

        return (((int64_t) b7 & 0xFF) << 56) | (((int64_t) b6 & 0xFF) << 48) |
               (((int64_t) b5 & 0xFF) << 40) | (((int64_t) b4 & 0xFF) << 32) |
               (((int64_t) b3 & 0xFF) << 24) | (((int64_t) b2 & 0xFF) << 16) |
               (((int64_t) b1 & 0xFF) << 8) | ((int64_t) b0 & 0xFF);
    }

    float ByteBuffer::readFloatAtLE(size_t index) {
        union {
            byte bytes[4];
            float value;
        } bits{};

        for (int i = 0; i < 4; i++) {
            bits.bytes[i] = _buffer[index++];
        }

        return bits.value;
    }

    float ByteBuffer::readFloatAtBE(size_t index) {
        union {
            byte bytes[4];
            float value;
        } bits{};

        for (int i = 3; i >= 0; i--) {
            bits.bytes[i] = _buffer[index++];
        }

        return bits.value;
    }

    double ByteBuffer::readDoubleAtLE(size_t index) {
        union {
            byte bytes[8];
            double value;
        } bits{};

        for (int i = 0; i < 8; i++) {
            bits.bytes[i] = _buffer[index++];
        }

        return bits.value;
    }

    double ByteBuffer::readDoubleAtBE(size_t index) {
        union {
            byte bytes[8];
            double value;
        } bits{};

        for (int i = 7; i >= 0; i--) {
            bits.bytes[i] = _buffer[index++];
        }

        return bits.value;
    }

    ///////////////////////////////////////////////////////////////////////////////

    void ByteBuffer::writeInt16AtLE(size_t index, int16_t value) {
        _buffer[index++] = value & 0xFF;
        _buffer[index] = (value >> 8) & 0xFF;
    }

    void ByteBuffer::writeInt16AtBE(size_t index, int16_t value) {
        _buffer[index++] = (value >> 8) & 0xFF;
        _buffer[index] = value & 0xFF;
    }

    void ByteBuffer::writeInt32AtLE(size_t index, int32_t value) {
        _buffer[index++] = value & 0xFF;
        _buffer[index++] = (value >> 8) & 0xFF;
        _buffer[index++] = (value >> 16) & 0xFF;
        _buffer[index] = (value >> 24) & 0xFF;
    }

    void ByteBuffer::writeInt32AtBE(size_t index, int32_t value) {
        _buffer[index++] = (value >> 24) & 0xFF;
        _buffer[index++] = (value >> 16) & 0xFF;
        _buffer[index++] = (value >> 8) & 0xFF;
        _buffer[index] = value & 0xFF;
    }

    void ByteBuffer::writeInt64AtLE(size_t index, int64_t value) {
        _buffer[index++] = value & 0xFF;
        _buffer[index++] = (value >> 8) & 0xFF;
        _buffer[index++] = (value >> 16) & 0xFF;
        _buffer[index++] = (value >> 24) & 0xFF;
        _buffer[index++] = (value >> 32) & 0xFF;
        _buffer[index++] = (value >> 40) & 0xFF;
        _buffer[index++] = (value >> 48) & 0xFF;
        _buffer[index] = (value >> 56) & 0xFF;
    }

    void ByteBuffer::writeInt64AtBE(size_t index, int64_t value) {
        _buffer[index++] = (value >> 56) & 0xFF;
        _buffer[index++] = (value >> 48) & 0xFF;
        _buffer[index++] = (value >> 40) & 0xFF;
        _buffer[index++] = (value >> 32) & 0xFF;
        _buffer[index++] = (value >> 24) & 0xFF;
        _buffer[index++] = (value >> 16) & 0xFF;
        _buffer[index++] = (value >> 8) & 0xFF;
        _buffer[index] = value & 0xFF;
    }

    void ByteBuffer::writeFloatAtLE(size_t index, float value) {
        union {
            byte bytes[4];
            float value;
        } bits{};

        bits.value = value;
        for (int i = 0; i < 4; i++) {
            _buffer[index++] = bits.bytes[i];
        }
    }

    void ByteBuffer::writeFloatAtBE(size_t index, float value) {
        union {
            byte bytes[4];
            float value;
        } bits{};

        bits.value = value;
        for (int i = 3; i >= 0; i--) {
            _buffer[index++] = bits.bytes[i];
        }
    }

    void ByteBuffer::writeDoubleAtLE(size_t index, double value) {
        union {
            byte bytes[8];
            double value;
        } bits{};

        bits.value = value;
        for (int i = 0; i < 8; i++) {
            _buffer[index++] = bits.bytes[i];
        }
    }

    void ByteBuffer::writeDoubleAtBE(size_t index, double value) {
        union {
            byte bytes[8];
            double value;
        } bits{};

        bits.value = value;
        for (int i = 7; i >= 0; i--) {
            _buffer[index++] = bits.bytes[i];
        }
    }
}

namespace kiva::huffman {
    using namespace cs::compiler;

    constexpr size_t HFZ_MAGIC_SIZE = 4;
    constexpr size_t TABLE_SIZE = UINT8_MAX + 1;
    constexpr unsigned char HFZ_MAGIC[HFZ_MAGIC_SIZE] = {0xde, 0xad, 0xfa, 0xce};

    template <typename T, size_t S>
    using Array = std::array<T, S>;
    template <typename T, typename U>
    using Pair = std::pair<T, U>;
    using String = std::string;
    using CodePoint = int;

    /**
     * The minimal heap whose root element is always the minimal value
     * @tparam T Heap object type
     * @tparam S Heap size
     * @tparam Comparator Comparator used in sort
     */
    template <typename T, size_t S, typename Comparator>
    class MinHeap {
    private:
        Array<T, S> _data = {0};
        size_t _position = 0;
        Comparator _comparator;

        void sortHeap(int currentNode) {
            int left = 2 * currentNode + 1;
            int right = 2 * currentNode + 2;
            int tmp;

            if (left >= size()) {
                return;
            }

            if (left == size() - 1) {
                if (_comparator(_data[currentNode], _data[left]) > 0) {
                    std::swap(_data[currentNode], _data[left]);
                }
                return;
            }

            if (_comparator(_data[currentNode], _data[left]) > 0
                || _comparator(_data[currentNode], _data[right]) > 0) {
                // swap the min parent node with current one
                if (_comparator(_data[left], _data[right]) < 0) {
                    std::swap(_data[currentNode], _data[left]);
                    sortHeap(left);
                } else {
                    std::swap(_data[currentNode], _data[right]);
                    sortHeap(right);
                }
            }
        }

        void sortHeap() {
            for (int i = size() - 1; i >= 0; i--) {
                int left = 2 * i + 1;
                if (left < size()) {
                    sortHeap(i);
                }
            }
        }

    public:
        MinHeap() = default;

        ~MinHeap() = default;

        void push(const T &t) {
            if (_position >= S) {
                throw std::runtime_error("Heap overflow");
            }

            _data.at(_position++) = std::move(t);
            sortHeap();
        }

        bool empty() {
            return _position == 0;
        }

        size_t size() {
            return _position;
        }

        T pop() {
            if (empty()) {
                throw std::runtime_error("Empty heap");
            }

            T elem = _data[0];
            _data[0] = _data[_position - 1];
            --_position;

            sortHeap();
            return std::move(elem);
        }
    };

    /**
     * The huffman tree
     */
    class HuffmanTree {
    public:
        struct Comparator {
            int operator()(HuffmanTree *lhs, HuffmanTree *rhs) {
                if (*lhs < *rhs) {
                    return -1;
                }
                if (*lhs > *rhs) {
                    return 1;
                }
                return 0;
            }
        };

    private:
        CodePoint _codePoint = '\0';
        bool _codePointNode = false;
        int _freq = 0;

        HuffmanTree *_left = nullptr;
        HuffmanTree *_right = nullptr;

    public:
        HuffmanTree() = default;

        HuffmanTree(CodePoint codePoint, int freq)
            : _codePoint(codePoint), _freq(freq), _codePointNode(true) {
        }

        HuffmanTree(HuffmanTree *left, HuffmanTree *right) {
            setLeft(left);
            setRight(right);
        }

        ~HuffmanTree() = default;

        bool isCodePoint() const {
            return _codePointNode;
        }

        void setLeft(HuffmanTree *left) {
            this->_left = left;
            if (left != nullptr) {
                this->_freq += left->getFreq();
            }
        }

        void setRight(HuffmanTree *right) {
            this->_right = right;
            if (right != nullptr) {
                this->_freq += right->getFreq();
            }
        }

        int getFreq() const {
            return _freq;
        }

        CodePoint getCodePoint() const {
            return _codePoint;
        }

        HuffmanTree *getLeft() {
            return _left;
        }

        HuffmanTree *getRight() {
            return _right;
        }

        bool operator>(const HuffmanTree &other) {
            return this->getFreq() > other.getFreq();
        }

        bool operator<(const HuffmanTree &other) {
            return this->getFreq() < other.getFreq();
        }
    };

    /**
     * The minimal unit of memory operation in C/C++
     * is byte (unsigned char), but we want to write
     * contents bit-by-bit.
     *
     * This is kind of a bit buffer that write a byte
     * (namely 8 bits) to memory at one time.
     */
    class BitWriter {
    private:
        unsigned char _buffer = 0;
        size_t _count = 0;

    public:
        void writeAll(FILE *fp) {
            if (_count != 0) {
                fwrite(&_buffer, sizeof(_buffer), 1, fp);
                _buffer = 0;
                _count = 0;
            }
        }

        void writeBit(FILE *fp, bool bit) {
            _buffer <<= 1U;
            _buffer |= bit ? 1U : 0U;
            if (++_count == 8) {
                writeAll(fp);
            }
        }
    };

    class HfzUtils {
    public:
        static bool checkTable(const int *table, size_t size) {
            std::unordered_map<std::string, int> reversed;
            int expectedSize = 0;

            for (size_t i = 0; i < size; ++i) {
                int comb = table[i];
                if (comb == 0) {
                    continue;
                }

                auto bitCount = static_cast<short>(comb >> 16);
                auto bits = static_cast<short>(comb & 0xffff);

                std::stringstream ss;

                for (int p = bitCount - 1; p >= 0; p--) {
                    bool b = (bits & (1 << p)) != 0;
                    ss << static_cast<char>('0' + b);
                }

                auto code = ss.str();
                reversed.emplace(code, i);
                ++expectedSize;
            }

            return reversed.size() == expectedSize;
        }

        /**
         * Recursivly create a directory, including its parent directory
         * @param cpath Directory to create
         * @param mode Directory permission
         * @return true if success
         */
        static bool mkdirR(const String &cpath, mode_t mode) {
            char *path = strdup(cpath.c_str());
            if (!path) {
                return false;
            }

            char currentPath[PATH_MAX];
            char *pathPiece;

            struct stat st{};
            strcpy(currentPath, "");
            pathPiece = strtok(path, "/");

            if (path[0] == '/') {
                strcat(currentPath, "/");
            }

            while (pathPiece != nullptr) {
                if (strlen(currentPath) + strlen(pathPiece) + 2 > PATH_MAX) {
                    free(path);
                    return false;
                }

                strcat(currentPath, pathPiece);
                strcat(currentPath, "/");

                if (stat(currentPath, &st) != 0) {
                    if (mkdir(currentPath, mode) < 0) {
                        free(path);
                        return false;
                    }
                }

                pathPiece = strtok(nullptr, "/");
            }

            free(path);
            return true;
        }
    };

    namespace debug {
        void dumpTree(HuffmanTree *tree, const String &code, int depth) {
            if (tree == nullptr) {
                return;
            }

            if (tree->isCodePoint()) {
                printf("%*sCodePointNode: { %d -> %s }\n", depth * 2, " ",
                    tree->getCodePoint(), code.c_str());
                return;
            }

            printf("%*sLeft {\n", depth * 2, " ");
            dumpTree(tree->getLeft(), code + "0", depth + 1);
            printf("%*s}\n", depth * 2, " ");

            printf("%*sRight {\n", depth * 2, " ");
            dumpTree(tree->getRight(), code + "1", depth + 1);
            printf("%*s}\n", depth * 2, " ");
        }

        void dumpTree(HuffmanTree *tree) {
            printf("RootLeft {\n");
            dumpTree(tree->getLeft(), "0", 1);
            printf("}\n");

            printf("RootRight {\n");
            dumpTree(tree->getRight(), "1", 1);
            printf("}\n");
        }
    }

    /**
     * Compressed entry header
     */
    struct EntryHeader {
        int totalBytes = 0;
        char filePath[PATH_MAX] = {'\0'};
        int huffmanTable[TABLE_SIZE] = {0};
    };

    class HfzCompressor {
    private:
        // Type alias to save typing time
        using TreeHeap = MinHeap<HuffmanTree *, TABLE_SIZE, HuffmanTree::Comparator>;
        using CodeDict = Array<CodePoint, TABLE_SIZE>;
        using HuffmanTable = Array<int, TABLE_SIZE>;
        using ArchiveFile = Pair<String, HuffmanTable>;
        using ArchiveFileStream = std::list<ArchiveFile>;

    private:
        /**
         * Check whether the huffman table is valid to decompress
         * @param table Huffman table
         * @return true if valid
         */
        static bool checkTable(const HuffmanTable &table) {
            return HfzUtils::checkTable(table.data(), table.size());
        }

        static void resetCodeDict(CodeDict &dictionary) {
            memset(dictionary.data(), '\0',
                sizeof(CodePoint) * dictionary.size());
        }

        static void deleteTree(HuffmanTree *tree) {
            if (tree == nullptr) {
                return;
            }

            deleteTree(tree->getLeft());
            deleteTree(tree->getRight());
            delete tree;
        }

        /**
         * Calculate code point frequency of a file
         * @param filePath File to calculate
         * @param dictionary code point frequency
         * @return true if success
         */
        static bool loadDictionary(const String &filePath, CodeDict &dictionary) {
            FILE *fp = fopen(filePath.c_str(), "rb");

            if (fp == nullptr) {
                return false;
            }

            int ch = 0;
            while (!feof(fp) && (ch = fgetc(fp)) != EOF) {
                ++dictionary[ch];
            }

            fclose(fp);
            return true;
        }

        static void generateHuffmanTable(HuffmanTable &table, HuffmanTree *tree,
                                         short code, int depth) {
            if (tree == nullptr) {
                return;
            }

            if (tree->isCodePoint()) {
                auto item = static_cast<int>((depth << 16) | (code & 0xffff));
                table[tree->getCodePoint()] = item;
                return;
            }

            short left = (code << 1) | 0;
            short right = (code << 1) | 1;
            generateHuffmanTable(table, tree->getLeft(), left, depth + 1);
            generateHuffmanTable(table, tree->getRight(), right, depth + 1);
        }

        /**
         * Generate a map from code point to huffman encoding
         * @param tree Huffman tree
         * @return Huffman table
         */
        static HuffmanTable generateHuffmanTable(HuffmanTree *tree) {
            HuffmanTable table{0};
            generateHuffmanTable(table, tree->getLeft(), 0, 1);
            generateHuffmanTable(table, tree->getRight(), 1, 1);
            return table;
        }

        static HuffmanTable encodeDictionary(const CodeDict &dictionary) {
            assert(dictionary.size() < UINT8_MAX + 1);

            TreeHeap heap;
            for (CodePoint ch = 0; ch < dictionary.size(); ++ch) {
                if (dictionary[ch] == 0) {
                    continue;
                }
                auto tree = new HuffmanTree(ch, dictionary[ch]);
                heap.push(tree);
            }

            while (heap.size() > 1) {
                auto one = heap.pop();
                auto two = heap.pop();
                auto node = new HuffmanTree(one, two);
                heap.push(node);
            }

            auto tree = heap.pop();
            auto &&table = generateHuffmanTable(tree);
            deleteTree(tree);
            return table;
        }

        static ArchiveFileStream encodeFiles(const std::vector<String> &files) {
            ArchiveFileStream archive;
            CodeDict dictionary{0};

            for (auto &&f : files) {
                resetCodeDict(dictionary);
                if (!loadDictionary(f, dictionary)) {
                    fprintf(stderr, "compressor: %s: no such file or directory\n", f.c_str());
                    continue;
                }
                archive.emplace_back(f, encodeDictionary(dictionary));
            }

            return std::move(archive);
        }

    private:
        ByteBuffer _byteBuffer;

    private:
        void writeHeader(const EntryHeader &header) {
            fwrite(&header, sizeof(EntryHeader), 1, fp);
        }

        void writeFile(FILE *fp, const HuffmanTable &table, FILE *fileIn) {
            int ch = 0;

            BitWriter writer;
            while (!feof(fileIn) && (ch = fgetc(fileIn)) != EOF) {
                int comb = table[ch];
                auto bitCount = static_cast<short>(comb >> 16);
                auto bits = static_cast<short>(comb & 0xffff);

                for (int i = bitCount - 1; i >= 0; i--) {
                    bool b = (bits & (1 << i)) != 0;
                    writer.writeBit(fp, b);
                }
            }

            writer.writeAll(fp);
        }

    public:
    };

//    bool compress(const String &outputFile, const std::vector<String> &files) {
//        FILE *fp = fopen(outputFile.c_str(), "wb");
//        if (fp == nullptr) {
//            fprintf(stderr, "encode: failed to open file for write %s: %s\n",
//                outputFile.c_str(), strerror(errno));
//            return false;
//        }
//
//        // global file header
//        fwrite(EntryHeader::MAGIC, sizeof(EntryHeader::MAGIC), 1, fp);
//
//        int errors = 0;
//        Archive archive = encodeFiles(files);
//        EntryHeader header;
//
//        for (auto &&item : archive) {
//            auto &&f = item.first;
//            auto &&table = item.second;
//
//            if (!checkTable(table)) {
//                fprintf(stderr, "encode: table corrupted: %s\n", f.c_str());
//                ++errors;
//                continue;
//            }
//
//            printf("Adding %s", f.c_str());
//
//            FILE *fileIn = fopen(f.c_str(), "rb");
//            if (fileIn == nullptr) {
//                fprintf(stderr, "encode: failed to open file for read %s: %s\n",
//                    f.c_str(), strerror(errno));
//                ++errors;
//                continue;
//            }
//
//            // record header position and write an placeholder
//            long headerPosition = ftell(fp);
//            memset(&header, '\0', sizeof(EntryHeader));
//            writeHeader(fp, header);
//
//            // record the start position of compressed data
//            long startPosition = ftell(fp);
//            writeFile(fp, table, fileIn);
//            fclose(fileIn);
//
//            // calculate the compressed size and write the real header
//            long endPosition = ftell(fp);
//
//            header.totalBytes = static_cast<int>(endPosition - startPosition);
//            strncpy(header.filePath, f.c_str(), sizeof(header.filePath));
//            memcpy(header.huffmanTable, table.data(), table.size());
//
//            // go to the saved header position
//            fseek(fp, headerPosition, SEEK_SET);
//            writeHeader(fp, header);
//
//            // go back to the end of the stream
//            fseek(fp, 0, SEEK_END);
//
//            printf(", compressed size: %d (bytes)\n", header.totalBytes);
//        }
//
//        fclose(fp);
//        return !errors;
//    }
//
//    bool decompress(const String &compressedFile, const String &outputDir) {
//        FILE *fp = fopen(compressedFile.c_str(), "rb");
//        if (fp == nullptr) {
//            fprintf(stderr, "decompress: failed to open file %s for read: %s\n",
//                compressedFile.c_str(), strerror(errno));
//            return false;
//        }
//
//        if (!mkdirR(outputDir, 0755)) {
//            fprintf(stderr, "decompress: failed to mkdir %s for read: %s\n",
//                outputDir.c_str(), strerror(errno));
//            fclose(fp);
//            return false;
//        }
//
//        // check magic
//        unsigned char magic[MAGIC_SIZE] = {0};
//        fread(magic, MAGIC_SIZE, 1, fp);
//        if (memcmp(magic, EntryHeader::MAGIC, MAGIC_SIZE) != 0) {
//            fprintf(stderr, "decompress: .hfz file magic not found\n");
//            fclose(fp);
//            return false;
//        }
//
//        EntryHeader header;
//        while (!feof(fp)) {
//            memset(&header, '\0', sizeof(EntryHeader));
//            if (fread(&header, sizeof(EntryHeader), 1, fp) != 1) {
//                break;
//            }
//
//            printf("Extracting %s, compressed size: %d\n",
//                header.filePath, header.totalBytes);
//
//            fseek(fp, header.totalBytes, SEEK_CUR);
//        }
//
//        fclose(fp);
//        return true;
//    }

}

//int main(int argc, const char **argv) {
//    if (argc < 2) {
//        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
//        fprintf(stderr, "  where command are one of the followings:\n");
//        fprintf(stderr, "    c <out.hfz> <file [, file...]>\n");
//        fprintf(stderr, "    d <file.hfz> <out-dir>\n");
//        return 1;
//    }
//
//    --argc;
//    ++argv;
//
//    if (strcmp(argv[0], "c") == 0) {
//        ++argv;
//        --argc;
//        if (argc == 0) {
//            fprintf(stderr, "compress: No output file name specified\n");
//            return 1;
//        }
//
//        const char *outputFile = *argv++;
//        std::vector<String> files;
//        while (*argv) {
//            files.emplace_back(*argv++);
//        }
//
//        if (!compress(outputFile, files)) {
//            fprintf(stderr, "compress: error encountered\n");
//            return 1;
//        }
//
//    } else if (strcmp(argv[0], "d") == 0) {
//        ++argv;
//        --argc;
//        if (argc == 0) {
//            fprintf(stderr, "decompress: No .hfz file specified\n");
//            return 1;
//        }
//
//        const char *compressed = argv[0];
//        const char *outDir = argc == 2 ? argv[1] : ".";
//
//        if (!decompress(compressed, outDir)) {
//            fprintf(stderr, "decompress: error encountered\n");
//            return 1;
//        }
//    }
//
//    return 0;
//}

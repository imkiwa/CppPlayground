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
    struct SizedBuffer {
        size_t _size;
        size_t _used;
        uint8_t *_bytes;
    };

    class ByteBuffer {
    public:
        using byte = uint8_t;

        static constexpr float BUFFER_GROWTH_FACTOR = 1.5f;
        static constexpr size_t BUFFER_DEFAULT_LENGTH = 1024;

    private:
        byte *_buffer;
        size_t _bufferLength;
        size_t _position;

    private:
        void growBuffer() {
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

        void growIfNeeded(size_t least) {
            // Keep growing until current buffer can afford the required length.
            while (_position + least >= _bufferLength) {
                growBuffer();
            }
        }

        static void copy(const byte *from, byte *to, size_t count) {
            memcpy(to, from, count);
        }

    public:
        ByteBuffer()
            : ByteBuffer(BUFFER_DEFAULT_LENGTH) {
        }

        explicit ByteBuffer(size_t initialLength)
            : _buffer(nullptr), _bufferLength(initialLength), _position(0) {
            growBuffer();
        }

        ~ByteBuffer() {
            if (_buffer != nullptr) {
                free(_buffer);
            }
        }

        SizedBuffer getBuffer() {
            return {
                ._size = getLength(),
                ._used = getPosition(),
                ._bytes = _buffer,
            };
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

    public:
        size_t reserve(size_t count) {
            size_t p = this->_position;
            _position += count;
            return p;
        }

        void writeU8(byte u) {
            writeU8At(_position++, u);
        }

        void write(const byte *data, size_t size) {
            writeAt(_position, data, size);
            _position += size;
        }

        bool read(byte *to, size_t size) {
            bool r = readAt(_position, to, size);
            _position += size;
            return r;
        }

        void writeU8At(size_t index, byte u) {
            growIfNeeded(1);
            _buffer[index] = u;
        }

        void writeAt(size_t index, const byte *data, size_t size) {
            if (index + size >= _bufferLength) {
                growIfNeeded(size);
            }
            copy(data, _buffer + index, size);
        }

        bool readAt(size_t index, byte *to, size_t size) {
            if (index + size >= _bufferLength) {
                return false;
            }
            copy(_buffer + index, to, size);
            return true;
        }
    };
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
    using HuffmanTable = Array<int, TABLE_SIZE>;
    using HuffmanInvTable = std::unordered_map<short, unsigned char>;

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
        void writeAll(ByteBuffer &byteBuffer) {
            if (_count != 0) {
                byteBuffer.writeU8(_buffer);
                _buffer = 0;
                _count = 0;
            }
        }

        void writeBit(ByteBuffer &byteBuffer, bool bit) {
            _buffer <<= 1U;
            _buffer |= bit ? 1U : 0U;
            if (++_count == 8) {
                writeAll(byteBuffer);
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
    struct HfzEntryHeader {
        int compressedSize = 0;
        char filePath[PATH_MAX] = {0};
        int huffmanTable[TABLE_SIZE] = {0};
    };

    class HfzCommand {
    private:
        // Type alias to save typing time
        using TreeHeap = MinHeap<HuffmanTree *, TABLE_SIZE, HuffmanTree::Comparator>;
        using CodeDict = Array<CodePoint, TABLE_SIZE>;

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
        static bool loadDictionary(const ByteBuffer::byte *bytes, size_t size, CodeDict &dictionary) {
            if (bytes == nullptr) {
                return false;
            }

            auto v = bytes;
            for (size_t i = 0; i < size; ++i) {
                ++dictionary[*v++];
            }

            return true;
        }

        static void genHuffmanTable(HuffmanTable &table, HuffmanTree *tree,
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
            genHuffmanTable(table, tree->getLeft(), left, depth + 1);
            genHuffmanTable(table, tree->getRight(), right, depth + 1);
        }

        /**
         * Generate a map from code point to huffman encoding
         * @param tree Huffman tree
         * @return Huffman table
         */
        static HuffmanTable genHuffmanTable(HuffmanTree *tree) {
            HuffmanTable table{0};
            genHuffmanTable(table, tree->getLeft(), 0, 1);
            genHuffmanTable(table, tree->getRight(), 1, 1);
            return table;
        }

        static HuffmanTable genHuffmanTable(const CodeDict &dictionary) {
            assert(dictionary.size() == UINT8_MAX + 1);

            TreeHeap heap;
            for (CodePoint ch = 0; ch < dictionary.size(); ++ch) {
                if (dictionary[ch] == 0) {
                    continue;
                }
                auto tree = new HuffmanTree(ch, dictionary[ch]);
                heap.push(tree);
            }

            // the dictionary contains nothing
            // so the huffman table is all-zero
            if (heap.empty()) {
                return HuffmanTable();
            }

            while (heap.size() > 1) {
                auto one = heap.pop();
                auto two = heap.pop();
                auto node = new HuffmanTree(one, two);
                heap.push(node);
            }

            auto tree = heap.pop();
            auto &&table = genHuffmanTable(tree);
            deleteTree(tree);
            return table;
        }

        static void writeEncoded(ByteBuffer &byteBuffer, const HuffmanTable &table,
                                 const ByteBuffer::byte *bytes, size_t size) {
            BitWriter writer;

            auto v = bytes;
            for (size_t j = 0; j < size; ++j) {
                int comb = table[*v++];

                auto bitCount = static_cast<short>(comb >> 16);
                auto bits = static_cast<short>(comb & 0xffff);

                for (int i = bitCount - 1; i >= 0; i--) {
                    bool b = (bits & (1 << i)) != 0;
                    writer.writeBit(byteBuffer, b);
                }
            }

            writer.writeAll(byteBuffer);
        }

        static HuffmanInvTable genHuffmanInvTable(const HuffmanTable &table) {
            HuffmanInvTable invTable;
            for (size_t i = 0; i < table.size(); ++i) {
                int comb = table[i];
                if (comb == 0) {
                    continue;
                }

//                auto bitCount = static_cast<short>(comb >> 16);
                auto bits = static_cast<short>(comb & 0xffff);

                invTable[bits] = i;
//                for (int p = bitCount - 1; p >= 0; p--) {
//                    bool b = (bits & (1 << p)) != 0;
//                }
            }
            return invTable;
        }

    public:
        static bool compressContent(const ByteBuffer::byte *bytes, size_t size, ByteBuffer &result) {
            CodeDict dict{0};
            if (!loadDictionary(bytes, size, dict)) {
                return false;
            }

            auto &&table = genHuffmanTable(dict);
            if (!checkTable(table)) {
                return false;
            }

            // let's encode the buffer

            // currently we don't know the size of compressed data
            // so we cannot write header here
            // we record the header position and write it later
            size_t headerPosition = result.reserve(sizeof(HfzEntryHeader));

            // write the compressed data
            size_t compressedStart = result.getPosition();
            writeEncoded(result, table, bytes, size);

            // fill header fields
            // note that: we won't fill the filePath field
            size_t compressedEnd = result.getPosition();
            HfzEntryHeader header{};
            header.compressedSize = static_cast<int>(compressedEnd - compressedStart);
            memcpy(header.huffmanTable, table.data(), sizeof(int) * table.size());

            // write the real header
            result.writeAt(headerPosition,
                reinterpret_cast<const ByteBuffer::byte *>(&header),
                sizeof(HfzEntryHeader));

            return true;
        }

        static bool decompressContent(const ByteBuffer::byte *bytes, size_t size,
                                      const HuffmanTable &table, ByteBuffer &result) {
            auto &&inv = genHuffmanInvTable(table);

            auto v = bytes;
            short bits = 0;

            for (size_t i = 0; i < size; ++i) {
                auto ch = *v++;
                for (int p = 7; p >= 0; p--) {
                    bool b = (ch & (1 << p)) != 0;
                    bits = (bits << 1) | (b ? 1 : 0);

                    if (inv.find(bits) != inv.end()) {
                        result.writeU8(inv[bits]);
                        bits = 0;
                    }
                }
            }

            return true;
        }
    };

    class HfzCompressor {
    private:
        std::vector<String> _files;
        String _outputFile;

    public:
        HfzCompressor() = default;

        explicit HfzCompressor(String outputFile)
            : _outputFile(std::move(outputFile)) {
        }

        ~HfzCompressor() = default;

        const String &getOutputFile() const {
            return _outputFile;
        }

        void setOutputFile(const String &outputFile) {
            _outputFile = outputFile;
        }

        void addFile(const String &file) {
            _files.push_back(file);
        }

        const std::vector<String> &getFiles() const {
            return _files;
        }

        void compress() {
            ByteBuffer outputBuffer;
            ByteBuffer inputBuffer;

            // file magic
            outputBuffer.write(HFZ_MAGIC, HFZ_MAGIC_SIZE);

            printf("Creating %s\n", _outputFile.c_str());
            for (auto &&f : _files) {
                FILE *fileIn = fopen(f.c_str(), "rb");
                if (fileIn == nullptr) {
                    throw std::runtime_error("failed to open " + f + ": " + strerror(errno));
                }

                fseek(fileIn, 0, SEEK_END);
                long fileSize = ftell(fileIn);
                fseek(fileIn, 0, SEEK_SET);

                auto bytes = new ByteBuffer::byte[fileSize];
                fread(bytes, fileSize, 1, fileIn);

                inputBuffer.rewind();
                if (!HfzCommand::compressContent(bytes, fileSize, inputBuffer)) {
                    delete[] bytes;
                    fclose(fileIn);
                    throw std::runtime_error("failed to compress file " + f);
                }

                delete[] bytes;

                // write file name to header
                size_t offset = offsetof(HfzEntryHeader, filePath);
                inputBuffer.writeAt(offset,
                    reinterpret_cast<const ByteBuffer::byte *>(f.c_str()),
                    f.size());

                // write compressed entry to output buffer
                auto &&compressed = inputBuffer.getBuffer();
                outputBuffer.write(compressed._bytes, compressed._used);

                if (fileSize > 0) {
                    size_t compressedSize = inputBuffer.getPosition();
                    double rate = 1.0 * compressedSize / fileSize;
                    printf("  adding: %s (deflated %.0lf%%)\n", f.c_str(), rate * 100);
                } else {
                    printf("  adding: %s (empty file)\n", f.c_str());
                }
            }

            // all entries compressed
            // create the output hfz file
            FILE *fp = fopen(_outputFile.c_str(), "wb");

            if (fp == nullptr) {
                throw std::runtime_error("failed to create hfz file "
                                         + _outputFile + ": "
                                         + strerror(errno));
            }

            auto &&sizedBuffer = outputBuffer.getBuffer();
            fwrite(sizedBuffer._bytes, sizedBuffer._used, 1, fp);

            fclose(fp);
        }

        void operator()() {
            compress();
        }
    };

    class HfzEntry {
        friend class HfzIterator;

    private:
        HfzEntryHeader _entryHeader{};
        ByteBuffer _inflateBuffer;
        bool _inflated = false;
        FILE *_stream = nullptr;

    private:
        bool doInflate() {
            auto bytes = new ByteBuffer::byte[getCompressedSize()];
            if (fread(bytes, getCompressedSize(), 1, _stream) != 1) {
                delete[] bytes;
                return false;
            }

            _inflateBuffer.rewind();
            HuffmanTable table{0};
            memcpy(table.data(), _entryHeader.huffmanTable, sizeof(int) * table.size());
            bool r = HfzCommand::decompressContent(bytes, getCompressedSize(),
                table, _inflateBuffer);
            delete[] bytes;
            return r;
        }

        void discard() {
            if (_inflated || _stream == nullptr) {
                return;
            }
            fseek(_stream, getCompressedSize(), SEEK_CUR);
        }

    public:
        HfzEntry() = default;

        ~HfzEntry() = default;

        HfzEntry(const HfzEntry &) = delete;

        HfzEntry(HfzEntry &&) = delete;

        HfzEntry &operator=(const HfzEntry &) = delete;

        HfzEntry &&operator=(HfzEntry &&) = delete;

        int getCompressedSize() const {
            return _entryHeader.compressedSize;
        }

        String getEntryFilePath() const {
            return _entryHeader.filePath;
        }

        SizedBuffer inflate() {
            if (!_inflated) {
                if (!doInflate()) {
                    return SizedBuffer{
                        ._bytes = nullptr,
                        ._size = 0,
                        ._used = 0,
                    };
                }
                _inflated = true;
            }
            return _inflateBuffer.getBuffer();
        }
    };

    class HfzIterator : public std::iterator<std::input_iterator_tag, std::shared_ptr<HfzEntry>> {
    private:
        FILE *_stream;
        std::shared_ptr<HfzEntry> _currentEntry;

    private:
        bool next() {
            HfzEntry *entry = _currentEntry.get();
            if (_stream == nullptr || feof(_stream) || entry == nullptr) {
                return false;
            }

            if (!entry->_inflated) {
                entry->discard();
            }

            memset(&entry->_entryHeader, '\0', sizeof(HfzEntryHeader));
            if (fread(&entry->_entryHeader, sizeof(HfzEntryHeader), 1, _stream) != 1) {
                return false;
            }

            entry->_stream = _stream;
            return true;
        }

    public:
        explicit HfzIterator(FILE *stream, HfzEntry *entry)
            : _stream(stream), _currentEntry(entry) {
            next();
        }

        ~HfzIterator() = default;

        HfzIterator &operator=(const HfzIterator &other) {
            this->_stream = other._stream;
            this->_currentEntry = other._currentEntry;
            return *this;
        }

        bool operator==(const HfzIterator &other) {
            return this->_stream == other._stream
                   && this->_currentEntry == other._currentEntry;
        }

        bool operator!=(const HfzIterator &other) {
            return !(*this == other);
        }

        HfzIterator &operator++() {
            if (!next()) {
                this->_stream = nullptr;
                this->_currentEntry = nullptr;
            }
            return *this;
        }

        std::shared_ptr<HfzEntry> operator*() {
            return _currentEntry;
        }

        std::shared_ptr<HfzEntry> operator->() {
            return _currentEntry;
        }
    };

    class HfzArchive {
    private:
        String _hfzFile;
        std::shared_ptr<FILE> _stream;

    public:
        explicit HfzArchive(String hfzFile)
            : _hfzFile(std::move(hfzFile)) {
        }

        const String &getHfzFile() const {
            return _hfzFile;
        }

        void open() {
            FILE *fp = fopen(_hfzFile.c_str(), "rb");
            if (fp == nullptr) {
                throw std::runtime_error("failed to open file "
                                         + _hfzFile + " for read: "
                                         + strerror(errno));
                return;
            }

            _stream = std::shared_ptr<FILE>(fp, std::fclose);

            // check magic
            unsigned char magic[HFZ_MAGIC_SIZE] = {0};
            fread(magic, HFZ_MAGIC_SIZE, 1, fp);
            if (memcmp(magic, HFZ_MAGIC, HFZ_MAGIC_SIZE) != 0) {
                throw std::runtime_error(".hfz file magic not found in " + _hfzFile);
            }
        }

        HfzIterator begin() {
            if (_stream == nullptr) {
                return end();
            }
            return HfzIterator(_stream.get(), new HfzEntry);
        }

        HfzIterator end() {
            return HfzIterator(nullptr, nullptr);
        }
    };

    class HfzDecompressor {
    private:
        String _hfzFile;
        String _outputDir;

    public:
        explicit HfzDecompressor(String compressedFile)
            : _hfzFile(std::move(compressedFile)),
              _outputDir(".") {
        }

        HfzDecompressor(String compressedFile, String outputDir)
            : _hfzFile(std::move(compressedFile)),
              _outputDir(std::move(outputDir)) {
        }

        ~HfzDecompressor() = default;

        const String &getOutputDir() const {
            return _outputDir;
        }

        void setOutputDir(const String &outputDir) {
            _outputDir = outputDir;
        }

        void decompress() {
            HfzArchive archive(_hfzFile);
            archive.open();

            printf("Extracting %s into %s\n", _hfzFile.c_str(), _outputDir.c_str());
            if (!HfzUtils::mkdirR(_outputDir, 0755)) {
                throw std::runtime_error("failed to mkdir " + _outputDir
                                         + ": " + strerror(errno));
            }

            for (auto &&item : archive) {
                String outputFile = _outputDir + "/" + item->getEntryFilePath();
                printf("  inflating: %s\n", outputFile.c_str());

                FILE *fp = fopen(outputFile.c_str(), "wb");
                if (fp == nullptr) {
                    throw std::runtime_error("failed to create file " + outputFile
                                             + ": " + strerror(errno));
                }

                auto buffer = item->inflate();
                fwrite(buffer._bytes, buffer._used, 1, fp);
                fclose(fp);
            }
        }

        void operator()() {
            decompress();
        }
    };
}

int main(int argc, const char **argv) {
    using namespace kiva::huffman;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        fprintf(stderr, "  where command are one of the followings:\n");
        fprintf(stderr, "    c <out.hfz> <file [, file...]>\n");
        fprintf(stderr, "    d <file.hfz> <out-dir>\n");
        return 1;
    }

    --argc;
    ++argv;

    if (strcmp(argv[0], "c") == 0) {
        ++argv;
        --argc;
        if (argc == 0) {
            fprintf(stderr, "compress: No output file name specified\n");
            return 1;
        }

        HfzCompressor compressor(*argv++);

        while (*argv) {
            compressor.addFile(*argv++);
        }

        try {
            compressor();
        } catch (std::runtime_error &e) {
            fprintf(stderr, "compress: error encountered: %s\n", e.what());
            return 1;
        }

    } else if (strcmp(argv[0], "d") == 0) {
        ++argv;
        --argc;
        if (argc == 0) {
            fprintf(stderr, "decompress: No .hfz file specified\n");
            return 1;
        }

        HfzDecompressor decompressor(argv[0]);
        if (argc == 2) {
            decompressor.setOutputDir(argv[1]);
        }

        try {
            decompressor();
        } catch (std::runtime_error &e) {
            fprintf(stderr, "decompress: error encountered: %s\n", e.what());
            return 1;
        }
    }

    return 0;
}

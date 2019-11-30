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

constexpr size_t MAGIC_SIZE = 4;
constexpr size_t TABLE_SIZE = UINT8_MAX + 1;

template <typename T, size_t S>
using Array = std::array<T, S>;

template <typename T, typename U>
using Pair = std::pair<T, U>;

using String = std::string;
using CodePoint = int;
using CodeDict = Array<CodePoint, TABLE_SIZE>;
using HuffmanTable = Array<int, TABLE_SIZE>;
using ArchiveFile = Pair<String, HuffmanTable>;
using Archive = std::list<ArchiveFile>;

/**
 * The minimal heap whose root element is always the minimal value
 * @tparam T Heap object type
 * @tparam S Heap size
 * @tparam Comparator Comparator used in sort
 */
template <typename T, size_t S, typename Comparator>
class MinHeap {
private:
    Array<T, S> _data;
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

// Type alias to save typing time
using TreeHeap = MinHeap<HuffmanTree *, TABLE_SIZE, HuffmanTree::Comparator>;

/**
 * Compressed entry header
 */
struct Header {
    static constexpr unsigned char MAGIC[MAGIC_SIZE] = {0xde, 0xad, 0xfa, 0xce};
    int totalBytes = 0;
    char filePath[PATH_MAX] = {'\0'};
    int huffmanTable[TABLE_SIZE] = {0};
};

/**
 * Recursivly create a directory, including its parent directory
 * @param cpath Directory to create
 * @param mode Directory permission
 * @return true if success
 */
bool mkdirR(const String &cpath, mode_t mode) {
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

/**
 * Check whether the huffman table is valid to decompress
 * @param table Huffman table
 * @return true if valid
 */
bool checkTable(const HuffmanTable &table) {
    std::unordered_map<std::string, int> reversed;
    int expectedSize = 0;

    for (int i = 0; i < table.size(); ++i) {
        int comb = table[i];
        if (comb == 0) {
            continue;
        }

        auto bitCount = static_cast<short>(comb >> 16);
        auto bits = static_cast<short>(comb & 0xffff);

        std::stringstream ss;

        for (int i = bitCount - 1; i >= 0; i--) {
            bool b = (bits & (1 << i)) != 0;
            ss << static_cast<char>('0' + b);
        }

        auto code = ss.str();
        reversed.emplace(code, i);
        ++expectedSize;
    }

    return reversed.size() == expectedSize;
}

void resetCodeDict(CodeDict &dictionary) {
    memset(dictionary.data(), '\0', sizeof(CodePoint) * dictionary.size());
}

void deleteTree(HuffmanTree *tree) {
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
bool loadDictionary(const String &filePath, CodeDict &dictionary) {
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

void generateHuffmanTable(HuffmanTable &table, HuffmanTree *tree, short code, int depth) {
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
HuffmanTable generateHuffmanTable(HuffmanTree *tree) {
    HuffmanTable table{0};
    generateHuffmanTable(table, tree->getLeft(), 0, 1);
    generateHuffmanTable(table, tree->getRight(), 1, 1);
    return table;
}

HuffmanTable encodeDictionary(const CodeDict &dictionary) {
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

Archive encodeFiles(const std::vector<String> &files) {
    Archive archive;
    CodeDict dictionary{0};

    for (auto &&f : files) {
        resetCodeDict(dictionary);
        if (!loadDictionary(f, dictionary)) {
            fprintf(stderr, "encode: %s: no such file or directory\n", f.c_str());
            continue;
        }
        archive.emplace_back(f, encodeDictionary(dictionary));
    }

    return std::move(archive);
}

void writeHeader(FILE *fp, const Header &header) {
    fwrite(&header, sizeof(Header), 1, fp);
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

bool compress(const String &outputFile, const std::vector<String> &files) {
    FILE *fp = fopen(outputFile.c_str(), "wb");
    if (fp == nullptr) {
        fprintf(stderr, "encode: failed to open file for write %s: %s\n",
            outputFile.c_str(), strerror(errno));
        return false;
    }

    // global file header
    fwrite(Header::MAGIC, sizeof(Header::MAGIC), 1, fp);

    int errors = 0;
    Archive archive = encodeFiles(files);
    Header header;

    for (auto &&item : archive) {
        auto &&f = item.first;
        auto &&table = item.second;

        if (!checkTable(table)) {
            fprintf(stderr, "encode: table corrupted: %s\n", f.c_str());
            ++errors;
            continue;
        }

        printf("Adding %s", f.c_str());

        FILE *fileIn = fopen(f.c_str(), "rb");
        if (fileIn == nullptr) {
            fprintf(stderr, "encode: failed to open file for read %s: %s\n",
                f.c_str(), strerror(errno));
            ++errors;
            continue;
        }

        // record header position and write an placeholder
        long headerPosition = ftell(fp);
        memset(&header, '\0', sizeof(Header));
        writeHeader(fp, header);

        // record the start position of compressed data
        long startPosition = ftell(fp);
        writeFile(fp, table, fileIn);
        fclose(fileIn);

        // calculate the compressed size and write the real header
        long endPosition = ftell(fp);

        header.totalBytes = static_cast<int>(endPosition - startPosition);
        strncpy(header.filePath, f.c_str(), sizeof(header.filePath));
        memcpy(header.huffmanTable, table.data(), table.size());

        // go to the saved header position
        fseek(fp, headerPosition, SEEK_SET);
        writeHeader(fp, header);

        // go back to the end of the stream
        fseek(fp, 0, SEEK_END);

        printf(", compressed size: %d (bytes)\n", header.totalBytes);
    }

    fclose(fp);
    return !errors;
}

bool decompress(const String &compressedFile, const String &outputDir) {
    FILE *fp = fopen(compressedFile.c_str(), "rb");
    if (fp == nullptr) {
        fprintf(stderr, "decompress: failed to open file %s for read: %s\n",
            compressedFile.c_str(), strerror(errno));
        return false;
    }

    if (!mkdirR(outputDir, 0755)) {
        fprintf(stderr, "decompress: failed to mkdir %s for read: %s\n",
            outputDir.c_str(), strerror(errno));
        fclose(fp);
        return false;
    }

    // check magic
    unsigned char magic[MAGIC_SIZE] = {0};
    fread(magic, MAGIC_SIZE, 1, fp);
    if (memcmp(magic, Header::MAGIC, MAGIC_SIZE) != 0) {
        fprintf(stderr, "decompress: .hfz file magic not found\n");
        fclose(fp);
        return false;
    }

    Header header;
    while (!feof(fp)) {
        memset(&header, '\0', sizeof(Header));
        if (fread(&header, sizeof(Header), 1, fp) != 1) {
            break;
        }

        printf("Extracting %s, compressed size: %d\n",
            header.filePath, header.totalBytes);

        fseek(fp, header.totalBytes, SEEK_CUR);
//        auto buf = new unsigned char[header.totalBytes];
//        fread(buf, header.totalBytes, 1, fp);
    }

    fclose(fp);
    return true;
}

int main(int argc, const char **argv) {
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

        const char *outputFile = *argv++;
        std::vector<String> files;
        while (*argv) {
            files.emplace_back(*argv++);
        }

        if (!compress(outputFile, files)) {
            fprintf(stderr, "compress: error encountered\n");
            return 1;
        }

    } else if (strcmp(argv[0], "d") == 0) {
        ++argv;
        --argc;
        if (argc == 0) {
            fprintf(stderr, "decompress: No .hfz file specified\n");
            return 1;
        }

        const char *compressed = argv[0];
        const char *outDir = argc == 2 ? argv[1] : ".";

        if (!decompress(compressed, outDir)) {
            fprintf(stderr, "decompress: error encountered\n");
            return 1;
        }
    }

    return 0;
}

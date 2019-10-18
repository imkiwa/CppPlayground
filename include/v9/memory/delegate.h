//
// Created by kiva on 2019/9/13.
//
#pragma once

#include <cstdlib>
#include <utility>
#include <cstdio>
#include <unordered_map>

namespace v9 {
    namespace memory {
        template<typename T>
        struct AddPointer {
            using type = T *;
        };

        template<typename T>
        struct FieldTypeFix {
            using type = T;
        };

        template<typename T, std::size_t N>
        struct FieldTypeFix<T[N]> {
            using type = typename AddPointer<typename FieldTypeFix<T>::type>::type;
        };

        template<typename T, typename FieldType>
        typename FieldTypeFix<FieldType>::type fieldType(FieldType T::*);

        template<typename T, typename FieldType>
        T classType(FieldType T::*);

        template<typename T, typename FieldType, FieldType T::*F>
        constexpr size_t __offset_of_helper() {
            return reinterpret_cast<size_t>(&(((T *) 0)->*F));
        }

#define offsetOf(field) \
    v9::memory::__offset_of_helper<decltype(v9::memory::classType(field)), \
            decltype(v9::memory::fieldType(field)), \
            field>()

        struct Block {
            unsigned char *memory;
            size_t size;
            size_t usedSize;

            Block(unsigned char *memory, size_t size) : memory(memory), size(size), usedSize(0) {}

            void use(size_t size) {
                usedSize += size;
            }

            void unuse(size_t size) {
                usedSize = usedSize < size ? 0 : usedSize - size;
            }

            size_t availableSize() const {
                return size - usedSize;
            }

            bool operator<(const Block &other) const {
                return memory < other.memory;
            }

            bool operator>(const Block &other) const {
                return memory > other.memory;
            }

            bool operator<=(const Block &other) const {
                return memory <= other.memory;
            }

            bool operator>=(const Block &other) const {
                return memory >= other.memory;
            }

            bool operator==(const Block &other) const {
                return memory == other.memory;
            }
        };

        template<typename T>
        class Delegate {
        private:
            Block *blocks;
            size_t blockCount;
            std::unordered_map<size_t, unsigned char *> fields;

        public:
            Delegate(Block *blocks, size_t blockCount) : blocks(blocks), blockCount(blockCount) {
                for (int i = 0; i < blockCount; ++i) {
                    printf("block %d: %p\n", i, blocks[i].memory);
                }
            }

#define F(field) \
    fieldGet<decltype(v9::memory::fieldType(field))>(                                   \
            v9::memory::__offset_of_helper<decltype(v9::memory::classType(field)),      \
                decltype(v9::memory::fieldType(field)),                                 \
                field>()                                                                \
            )

            Block *chooseBlock(Block *current, Block *candidate, size_t requiredSize) {
                if (current == nullptr) {
                    return candidate;
                }

                return ((current->availableSize() - requiredSize) < (candidate->availableSize() - requiredSize))
                       ? current : candidate;
            }

            template<typename FieldType>
            FieldType *fieldGet(size_t offset) {
                if (fields.find(offset) != fields.end()) {
                    return reinterpret_cast<FieldType *>(fields[offset]);
                }

                size_t size = sizeof(FieldType);

                Block *found = nullptr;
                for (int i = 0; i < blockCount; ++i) {
                    auto *block = blocks + i;
                    if (block->availableSize() == size) {
                        found = block;
                        break;
                    }

                    if (block->availableSize() > size) {
                        found = chooseBlock(found, block, size);
                    }
                }

                if (found != nullptr) {
                    auto memory = found->memory + found->usedSize;
                    fields[offset] = memory;
                    found->use(size);
                    return reinterpret_cast<FieldType *>(memory);
                }

                return nullptr;
            }
        };
    }
}

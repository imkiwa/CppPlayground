//
// Created by kiva on 2020/3/1.
//

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <cstdint>
#include <cstddef>
#include <cstring>

#include <sys/mman.h>

enum Opcode { SUB, MUL, PUSH, STORE, LOAD, JNZ, HALT };

class VM {
private:
    using CompiledCode = int64_t(void *);
    using ManagedCompiledCode = std::unique_ptr<void, std::function<void(void *)>>;

    std::vector<int8_t> _bytecodes;
    std::vector<uint8_t> _buffer;
    ManagedCompiledCode _managedCode;

private:
    void pushCode(std::initializer_list<uint8_t> code) {
        _buffer.insert(_buffer.end(), code);
    }

    void genPrologue() {
        pushCode({
            0x53,             // push rbx
            0x41, 0x54,       // push r12
            0x49, 0x89, 0xfc, // mov  r12, rdi
        });
    }

    void genHalt() {
        pushCode({
            0x49, 0x8b, 0x04, 0x24, // mov  rax, qword ptr [r12]
            0x41, 0x5c,             // pop  r12
            0x5b,                   // pop  rbx
            0xc3,                   // ret
        });
    }

    void genJnz() {
        pushCode({
            0x49, 0x8b, 0x04, 0x24, // mov  rax, qword ptr [r12]
            0x49, 0x83, 0xec, 0x08, // sub  r12, 8
            0x48, 0x83, 0xf8, 0x00, // cmp  rax, 0
            0x0f, 0x85,             // jnz  label
        });
    }

    void genLoad() {
        pushCode({
            0x49, 0x83, 0xc4, 0x08, // add  r12, 8
            0x49, 0x89, 0x1c, 0x24, // mov  qword ptr [r12], rbx
        });
    }

    void genStore() {
        pushCode({
            0x49, 0x8b, 0x1c, 0x24, // mov  rbx, qword ptr [r12]
            0x49, 0x83, 0xec, 0x08, // sub  r12, 8
        });
    }

    void genPush(int64_t imm) {
        pushCode({
            0x48, 0xb8, // mov  rax, imm
        });
        for (int i = 0; i < 8; ++i) {
            _buffer.push_back(reinterpret_cast<uint8_t *>(&imm)[i]);
        }
        pushCode({
            0x49, 0x83, 0xc4, 0x08, // add  r12, 8
            0x49, 0x89, 0x04, 0x24, // mov  qword ptr [r12], rax
        });
    }

    void genMul() {
        pushCode({
            0x49, 0x8b, 0x04, 0x24,       // mov  rax, qword ptr [r12]
            0x49, 0x83, 0xec, 0x08,       // sub  r12, 8
            0x49, 0x0f, 0xaf, 0x04, 0x24, // imul rax, qword ptr [r12]
            0x49, 0x89, 0x04, 0x24,       // mov  qword ptr [r12], rax
        });
    }

    void genSub() {
        pushCode({
            0x49, 0x8b, 0x04, 0x24, // mov  rax, qword ptr [r12]
            0x49, 0x83, 0xec, 0x08, // sub  r12, 8
            0x49, 0x29, 0x04, 0x24, // sub  qword ptr [r12], rax
        });
    }

    CompiledCode *createExecutableBuffer() {
        auto buf_size = _buffer.size();
        auto buf = mmap(nullptr, buf_size, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        memcpy(buf, _buffer.data(), buf_size);
        mprotect(buf, buf_size, PROT_READ | PROT_EXEC);
        _managedCode = ManagedCompiledCode(buf, [buf_size](void *buf) { munmap(buf, buf_size); });
        return reinterpret_cast<CompiledCode *>(buf);
    }

public:
    VM(std::initializer_list<int8_t> code) : _bytecodes(code) {}

    int64_t run() {
        char stack[64 * 8] = {0};
        return compile()(stack);
    }

    CompiledCode *compile() {
        std::unordered_map<int, std::size_t> labels;
        std::unordered_map<std::size_t, int> backfill;
        _buffer.clear();
        // generate prologue
        genPrologue();

        // generate code
        for (int pc = 0; pc < _bytecodes.size(); ++pc) {
            labels[pc] = _buffer.size();
            switch (_bytecodes[pc]) {
                case SUB: {
                    genSub();
                    break;
                }
                case MUL: {
                    genMul();
                    break;
                }
                case PUSH: {
                    int64_t imm = _bytecodes[++pc];
                    genPush(imm);
                    break;
                }
                case STORE: {
                    genStore();
                    break;
                }
                case LOAD: {
                    genLoad();
                    break;
                }
                case JNZ: {
                    genJnz();
                    backfill[_buffer.size()] = pc + _bytecodes[pc + 1];
                    _buffer.resize(_buffer.size() + 4);
                    ++pc;
                    break;
                }
                case HALT: {
                    genHalt();
                    break;
                }
            }
        }
        // backfill labels
        for (const auto &it : backfill) {
            int32_t offset = labels[it.second];
            offset -= it.first + 4;
            for (int i = 0; i < 4; ++i) {
                _buffer[it.first + i] = reinterpret_cast<uint8_t *>(&offset)[i];
            }
        }
        // create executable buffer
        return createExecutableBuffer();
    }
};

int64_t fact(int8_t n) {
    VM vm = {
        PUSH, n, STORE, PUSH, 1,
        LOAD, MUL,
        LOAD, PUSH, 1, SUB, STORE,
        LOAD, JNZ, -8,
        HALT,
    };
    return vm.run();
}

int main(int argc, const char *argv[]) {
    printf("%d! = %ld\n", 10, fact(10));

    return 0;
}

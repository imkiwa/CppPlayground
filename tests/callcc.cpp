//
// Created by kiva on 2020/3/20.
//

#include <cstdio>
#include <utility>
#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>
#include <stack>

enum Opcode {
    LOAD /* n */,       // load local[n] into stack
    STORE /* n */,      // store stack top to local[n]
    BIPUSH /* i */,     // push i onto stack
    INC,                // increase stack top
    PRINTLN,            // dump VM info
    CALL_CC /* n */,    // call/cc function named name[n]
    YIELD /* n */,      // resume continuation (local[n]) with stack top
    RESUME /* n */,     // resume continuation (local[n])
//    CALL /* i */,       // call function named name[i]
//    RET,                // return from function with stack top
};

struct Frame;

struct Continuation {
    std::shared_ptr<Frame> _frame;
};

union Object {
    int i32;
    Continuation *cont = nullptr;
};

struct Function {
    int _argc = 0;
    int _locals = 0;
    bool _return = false;
    std::string _name;
    std::vector<uint8_t> _body;
};

struct Frame {
    std::vector<Object> _local;
    std::stack<Object> _stack;
    std::shared_ptr<Function> _func;
    int _resumed_from = -1;
    int _pc = 0;

    explicit Frame(const std::shared_ptr<Function> &func)
        : _func(func), _local(func->_argc + func->_locals) {
    }
};

struct VM {
    std::shared_ptr<Frame> _current;
    std::vector<std::shared_ptr<Frame>> _frames;
    std::unordered_map<std::string, std::shared_ptr<Function>> _table;
    std::vector<std::string> _strings;

    explicit VM(std::unordered_map<std::string, std::shared_ptr<Function>> table)
        : _table(std::move(table)) {
    }

    Object current_cont() {
        auto *cont = new Continuation;
        cont->_frame = _current;
        return Object{.cont = cont};
    }

    void run() {
        while (_current->_pc < _current->_func->_body.size()) {
            if (_current->_resumed_from != -1) {
                _current->_local[_current->_resumed_from] = std::move(_current->_stack.top());
                _current->_stack.pop();
                _current->_resumed_from = -1;
            }

            switch (_current->_func->_body[_current->_pc++]) {
                case Opcode::LOAD: {
                    int index = _current->_func->_body[_current->_pc++];
                    _current->_stack.push(_current->_local[index]);
                    break;
                }
                case Opcode::STORE: {
                    int index = _current->_func->_body[_current->_pc++];
                    _current->_local[index] = std::move(_current->_stack.top());
                    _current->_stack.pop();
                    break;
                }
                case Opcode::BIPUSH: {
                    int value = _current->_func->_body[_current->_pc++];
                    _current->_stack.push(Object{.i32 = value});
                    break;
                }
                case Opcode::INC: {
                    ++_current->_stack.top().i32;
                    break;
                }
                case Opcode::PRINTLN: {
                    printf("%d\n", _current->_stack.top().i32);
                    _current->_stack.pop();
                    break;
                }
                case Opcode::CALL_CC: {
                    int index = _current->_func->_body[_current->_pc++];
                    auto func = _table.find(_strings[index]);
                    if (func == _table.end()) {
                        fprintf(stderr, "unsatisfied link: %s\n", func->first.c_str());
                        std::terminate();
                    }

                    auto frame = std::make_shared<Frame>(func->second);
                    for (int i = 0; i < func->second->_argc - 1; ++i) {
                        frame->_local[i] = std::move(_current->_stack.top());
                        _current->_stack.pop();
                    }
                    frame->_local[func->second->_argc - 1] = current_cont();
                    _frames.push_back(frame);
                    // switch context
                    _current = frame;
                    break;
                }
                case Opcode::YIELD: {
                    int index = _current->_func->_body[_current->_pc++];
                    auto *caller = _current->_local[index].cont;

                    if (_current->_func->_return) {
                        caller->_frame->_stack.push(std::move(_current->_stack.top()));
                        _current->_stack.pop();
                    }
                    caller->_frame->_stack.push(current_cont());
                    // switch context
                    _current = caller->_frame;
                    break;
                }
                case Opcode::RESUME: {
                    int index = _current->_func->_body[_current->_pc++];
                    auto *cont = _current->_local[index].cont;
                    // for auto-update cont in this scope
                    _current->_resumed_from = index;
                    // switch context
                    _current = cont->_frame;
                    break;
                }
                default:
                    fprintf(stderr, "Illegal instruction\n");
                    std::terminate();
            }
        }
    }

    void start() {
        auto main = _table.find("main");
        if (main == _table.end()) {
            fprintf(stderr, "no main() function found\n");
            return;
        }

        _current = std::make_shared<Frame>(main->second);
        _frames.push_back(_current);
        run();
    }
};

std::shared_ptr<Function> make_main() {
    auto f = new Function;
    /**
     * void main() {
     *     var (cont, x) = generator(100);
     *     println(x);
     *     println(resume cont);
     *     println(resume cont);
     * }
     */
    f->_argc = 0;
    f->_locals = 2;
    f->_return = false;
    f->_name = "main";
    f->_body = {
        // var (cont, x) = generator(100)
        BIPUSH, 100,
        CALL_CC, 1,
        STORE, 0, // cont
        STORE, 1, // x
        // println(x)
        LOAD, 1,
        PRINTLN,
        // show(resume cont)
        RESUME, 0,
        PRINTLN,
        // show(resume cont)
        RESUME, 0,
        PRINTLN,
    };
    return std::shared_ptr<Function>{f};
}

std::shared_ptr<Function> make_generator() {
    auto f = new Function;
    /*
     * int generator(int x) {
     *     yield x;
     *     yield x + 1;
     *     yield x + 2;
     * }
     */

    f->_argc = 2; // one more for continuation
    f->_locals = 0;
    f->_return = true;
    f->_name = "generator";
    f->_body = {
        // yield x
        LOAD, 0,
        YIELD, 1,
        // yield x + 1
        LOAD, 0,
        INC,
        YIELD, 1,
        // yield x + 2
        LOAD, 0,
        INC,
        INC,
        YIELD, 1,
    };
    return std::shared_ptr<Function>{f};
}

int main() {
    VM vm({
        {"main",      make_main()},
        {"generator", make_generator()},
    });

    vm._strings = {"main", "generator"};

    vm.start();
}

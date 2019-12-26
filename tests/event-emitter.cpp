//
// Created by kiva on 2019/12/8.
//

#include <v9/kit/event.hpp>
#include <string>
#include <utility>

class REPL : public v9::kit::EventEmitter {
};

class BaseDispatcher : public v9::kit::EventEmitter {
public:
    BaseDispatcher() {
        on("int", [](int i) {
            printf("BaseDispatcher: got an %d\n", i);
        });
    }
};

class DerivedDispatcher : public BaseDispatcher {
public:
    DerivedDispatcher() {
        // TODO: support override handlers
        this->clearAllHandlers("int");
        on("int", [this](int i) {
            printf("DerivedDispatcher: got an %d\n", i);
            this->love();
        });
    }

    void love() {
        printf("Tell my father: I love him\n");
    }
};

void love() {
    printf("I love u\n");
}

int main(int argc, const char **argv) {
    REPL repl;

    // register event handlers
    repl.on("SIGINT", []() {
        printf("Keyboard Interrupt (Ctrl-C pressed)\n");
    });

    repl.on("expr",[] (const std::string &expr) {
        printf("evaluating: %s\n", expr.c_str());
    });

    repl.on("command", [](const std::string& opt) {
        printf("applying command: %s\n", opt.c_str());
    });

    repl.on("sum", [](int a, int b, int c) {
        printf("sum: %d\n", a + b + c);
    });

    repl.on("love", love);

    // simulate real-world situation
    repl.emit("command", std::string("b main"));
    repl.emit("expr", std::string("system.run(\"rm -rf --no-preserve-root /\")"));
    repl.emit("SIGINT");
    repl.emit("sum", 2, 2, 7);
    repl.emit("love");

    DerivedDispatcher dispatcher;
    dispatcher.emit("int", 100);
}

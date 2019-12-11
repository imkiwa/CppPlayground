//
// Created by kiva on 2019/12/8.
//

#include <v9/kit/event.hpp>
#include <string>
#include <utility>

class REPL : public v9::kit::EventEmitter {
};

int main(int argc, const char **argv) {
    REPL repl;

    // register event handlers
    repl.on("SIGINT", []() {
        printf("Keyboard Interrupt (Ctrl-C pressed)\n");
    });

    repl.on("expr",[] (const std::string &expr) {
        printf("evaluating: %s\n", expr.c_str());
    });

    repl.on("command", [](const std::string &opt) {
        printf("applying command: %s\n", opt.c_str());
    });

    // simulate real-world situation
    repl.emit("command", std::string("b main"));
    repl.emit("expr", std::string("system.run(\"rm -rf --no-preserve-root /\")"));
    repl.emit("SIGINT");
}
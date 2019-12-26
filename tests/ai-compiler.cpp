//
// Created by kiva on 2019/12/26.
//

#include <random>
#include <fstream>
#include <v9/kit/string.hpp>
#include <v9/kit/error.hpp>

using namespace v9::kit;
using namespace std::literals;

/**
 * 存储编译结果
 */
class CompileResult {
private:
    std::string _output;

public:
    explicit CompileResult(StringRef output)
        : _output(output) {
    }

    const std::string &output() const {
        return _output;
    }
};

/**
 * 人工智能编译器核心代码
 */
class AICompiler {
private:
    static auto clang(StringRef file) {
        using ReturnType = Errors<std::string>;

        int code = system(("clang++ "s + file.str()).c_str());
        return !code ? ReturnType::ok("./a.out") : ReturnType::error(code);
    }

public:
    static auto compile(StringRef file) {
        using ReturnType = Either<std::string, CompileResult>;

        std::default_random_engine e(time(nullptr));
        std::uniform_int_distribution<int> d(0, 100);

        int willing = d(e);
        if (willing >= 0 && willing <= 50) {
            return ReturnType::left("I don't want to compile this shit.");
        }

        auto r = clang(file);
        if (r.isRight()) {
            return ReturnType::right(CompileResult(r.right().get()));
        }

        return ReturnType::left(
            "Your code is shit, did you just repeat\n"
            "converting C++ code to Java and convert them back\n"
            "until they looks disgusting...fuck me,\n"
            "what's wrong with you people?");
    }
};

int main() {
    constexpr const char *code = "#include <cstdio>\n"
                                 "int main() {\n"
                                 "    printf(\"hello world\\n\");\n"
                                 "    return 0;\n"
                                 "}\n";

    std::ofstream out("main.cpp", std::ios::out);
    out << code;
    out.flush();
    out.close();

    auto res = AICompiler::compile("main.cpp");
    if (res.isRight()) {
        system(res.right()->output().c_str());
    } else {
        printf("\nCompilation aborted: %s\n", res.left()->c_str());
    }
}

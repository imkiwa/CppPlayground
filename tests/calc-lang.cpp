#include <iostream>
#include <string>
#include <deque>
#include <utility>

int gcd(int a, int b) {
    return b == 0 ? a : gcd(b, a % b);
}

struct Num {
    int son;
    int mon;

    Num() : Num(0, 1) {}

    void simplify() {
        int g = gcd(son, mon);
        son /= g;
        mon /= g;
    }

    Num(int son, int mon) : son(son), mon(mon) {
        simplify();
    }

    Num operator+(const Num &rhs) const {
        return Num(
            this->son * rhs.mon + this->mon * rhs.son,
            this->mon * rhs.mon
        );
    }

    Num operator-(const Num &rhs) const {
        return Num(
            this->son * rhs.mon - this->mon * rhs.son,
            this->mon * rhs.mon
        );
    }

    Num operator*(const Num &rhs) const {
        return Num(
            this->son * rhs.son,
            this->mon * rhs.mon
        );
    }

    Num operator/(const Num &rhs) const {
        if (rhs.son == 0) {
            throw std::runtime_error("Divide by zero");
        }
        return this->operator*(Num(rhs.mon, rhs.son));
    }
};

std::ostream &operator<<(std::ostream &out, const Num &num) {
    if (num.mon == 1) {
        out << num.son;
    } else {
        out << num.son << " % " << num.mon;
    }
    return out;
}

// Lexer : String -> List<Token>

enum struct TokenType {
    ADD, SUB, MUL, DIV,
    LEFT_B, RIGHT_B,
    NUM, ID,
};

struct Token {
    TokenType type;
    Num num;
    std::string id;

    explicit Token(TokenType type) : type(type) {}

    explicit Token(Num num) : type(TokenType::NUM), num(num) {}

    explicit Token(std::string id) : type(TokenType::ID), id(std::move(id)) {}
};

using CharP = const char *;

void skip_ws(CharP &p) {
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n')) {
        p++;
    }
}

std::deque<Token> lex(const std::string &src) {
    std::deque<Token> result;
    const char *p = src.c_str();
    while (*p) {
        skip_ws(p);

        if (*p == '+') {
            result.emplace_back(TokenType::ADD);
            ++p;
            continue;
        }

        if (*p == '-') {
            result.emplace_back(TokenType::SUB);
            ++p;
            continue;
        }

        if (*p == '*') {
            result.emplace_back(TokenType::MUL);
            ++p;
            continue;
        }

        if (*p == '/') {
            result.emplace_back(TokenType::DIV);
            ++p;
            continue;
        }

        if (*p == '(') {
            result.emplace_back(TokenType::LEFT_B);
            ++p;
            continue;
        }

        if (*p == ')') {
            result.emplace_back(TokenType::RIGHT_B);
            ++p;
            continue;
        }

        // num
        if (*p >= '0' && *p <= '9') {
            int son = 0;
            int mon = 0;
            while (*p && *p >= '0' && *p <= '9') {
                son = son * 10 + (*p - '0');
                ++p;
            }

            skip_ws(p);

            if (*p == '%') {
                // consume '%'
                ++p;

                skip_ws(p);

                while (*p && *p >= '0' && *p <= '9') {
                    mon = mon * 10 + (*p - '0');
                    ++p;
                }
            } else {
                mon = 1;
            }

            result.emplace_back(Num(son, mon));
            continue;
        }

        if (std::isalpha(*p)) {
            std::string id;
            while (std::isalpha(*p) || std::isdigit(*p)) {
                id.push_back(*p++);
            }

            result.emplace_back(id);
            continue;
        }

        throw std::runtime_error(std::string(
            "Syntax Error: unexpected'") + *p + "', expected operators or digits");
    }

    return result;
}

// Parser : List<Token> -> Unit
struct Term;

struct Node {
    virtual Num eval() = 0;
};

struct Expr : public Node {
    Node *lhs{};
    Node *rhs{};
    TokenType op;

    Num eval() override;

    ~Expr();
};

struct Factor : public Node {
    Node *expr{};
    Num *num{};
    std::string id;

    Num eval() override {
        if (expr != nullptr) {
            return expr->eval();
        }
        if (num != nullptr) {
            return *num;
        }
        throw std::runtime_error("// TODO: support id");
    }

    ~Factor() {
        delete expr;
        delete num;
    }
};

struct Term : public Node {
    Node *lhs{};
    Node *rhs{};
    TokenType op;

    Num eval() override {
        Num l = lhs->eval();
        if (rhs) {
            Num r = rhs->eval();
            switch (op) {
                case TokenType::MUL:
                    return l * r;
                case TokenType::DIV:
                    return l / r;
            }
        }
        return l;
    }

    ~Term() {
        delete lhs;
        delete rhs;
    }
};

struct Unit : public Node {
    Expr *expr{};

    Num eval() override {
        return expr->eval();
    }

    ~Unit() {
        delete expr;
    }
};

Num Expr::eval() {
    Num l = lhs->eval();
    if (rhs) {
        Num r = rhs->eval();
        switch (op) {
            case TokenType::ADD:
                return l + r;
            case TokenType::SUB:
                return l - r;
        }
    }
    return l;
}

Expr::~Expr() {
    delete lhs;
    delete rhs;
}

struct Parser {
    std::deque<Token> tokens;

    void ensure() {
        if (isEOF()) {
            throw std::runtime_error("Syntax Error: Unexpected EOF");
        }
    }

    TokenType consumeOp() {
        ensure();
        switch (tokens.front().type) {
            case TokenType::ADD:
            case TokenType::SUB:
            case TokenType::MUL:
            case TokenType::DIV: {
                TokenType type = tokens.front().type;
                tokens.pop_front();
                return type;
            }
            default: {
                throw std::runtime_error("Syntax Error: expected operator");
            }
        }
    }

    void consume() {
        ensure();
        tokens.pop_front();
    }

    void consume_type(TokenType type) {
        if (peek() != type) {
            throw std::runtime_error(std::string("Syntax Error: unexpected token"));
        }
        consume();
    }

    Num consume_num() {
        if (peek() != TokenType::NUM) {
            throw std::runtime_error(std::string("Syntax Error: expected num"));
        }
        Num num = tokens.front().num;
        consume();
        return num;
    }

    std::string consume_id() {
        if (peek() != TokenType::ID) {
            throw std::runtime_error(std::string("Syntax Error: expected id"));
        }
        auto id = tokens.front().id;
        consume();
        return id;
    }

    bool isEOF() {
        return tokens.empty();
    }

    TokenType peek() {
        ensure();
        return tokens.front().type;
    }

    explicit Parser(std::deque<Token> tokens) : tokens(std::move(tokens)) {}

    Factor *parseFactor() {
        Factor *factor = new Factor;
        // lookahead
        switch (peek()) {
            case TokenType::LEFT_B: {
                consume();
                factor->expr = parseExpr();
                consume_type(TokenType::RIGHT_B);
                break;
            }
            case TokenType::NUM: {
                factor->num = new Num(consume_num());
                break;
            }
            case TokenType::ID: {
                factor->id = consume_id();
                break;
            }
            default: {
                throw std::runtime_error(std::string("Syntax Error: expected '(' or num"));
            }
        }
        return factor;
    }

    Term *parseTerm() {
        Term *term = new Term;
        term->lhs = parseFactor();
        // lookahead
        while (!isEOF()) {
            switch (peek()) {
                case TokenType::MUL:
                case TokenType::DIV: {
                    term->op = consumeOp();
                    term->rhs = parseFactor();
                    Term *next = new Term;
                    next->lhs = term;
                    term = next;
                    break;
                }
                default:
                    goto break_while;
            }
        }
        break_while:
        return term;
    }

    Expr *parseExpr() {
        Expr *expr = new Expr;
        expr->lhs = parseTerm();
        // lookahead
        while (!isEOF()) {
            switch (peek()) {
                case TokenType::ADD:
                case TokenType::SUB: {
                    expr->op = consumeOp();
                    expr->rhs = parseTerm();
                    Expr *next = new Expr;
                    next->lhs = expr;
                    expr = next;
                    break;
                }
                default:
                    goto break_while;
            }
        }
        break_while:
        return expr;
    }

    Unit *parseUnit() {
        Unit *unit = new Unit;
        unit->expr = parseExpr();
        return unit;
    }
};

struct T {
    int *p = nullptr;

    T() = default;

    T(T &&rhs) noexcept {
        delete p;
        p = nullptr;
        this->p = rhs.p;
        rhs.p = nullptr;
    }

    T(const T &rhs) {
        delete p;
        p = nullptr;
        this->set(*rhs.p);
    }

    T &operator=(const T &r) {
        this->p = r.p;
        return *this;
    }

    void set(int i) {
        p = new int(i);
    }

    ~T() {
        printf("p = %p\n", p);
        delete p;
        p = nullptr;
    }
};

int main() {
    std::string line;

    while (true) {
        std::cout << "> ";
        std::cout.flush();
        if (!std::getline(std::cin, line)) {
            break;
        }

        try {
            Parser parser(lex(line));
            Unit *unit = parser.parseUnit();
            std::cout << unit->eval() << std::endl;
            delete unit;
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }

    return 0;
}

//
// Created by kiva on 2020/4/8.
//

#include <stack>
#include <deque>
#include <memory>
#include <unordered_map>
#include <utility>
#include <string>
#include <vector>
#include <cstring>

namespace mpp {
    template <typename T, typename... ArgsT>
    __attribute__((noreturn))
    void throw_ex(ArgsT &&... args) {
        static_assert(std::is_base_of<std::exception, T>::value,
            "Only std::exception and its derived classes can be thrown");
        T exception{std::forward<ArgsT>(args)...};
        throw exception;
    }
}

namespace shared {
    template <typename ST, ST GLOBAL>
    struct state_manager {
    private:
        ST _state = GLOBAL;
        std::stack<ST> _previous;

        ST last_state() {
            if (_previous.empty()) {
                return GLOBAL;
            }
            ST state = _previous.top();
            _previous.pop();
            return state;
        }

    public:
        void new_state(ST state) {
            _previous.push(_state);
            _state = state;
        }

        void end(ST expected) {
            if (_state == expected) {
                _state = last_state();
            }
        }

        void replace(ST state) {
            _state = state;
        }

        ST pop() {
            ST state = current();
            end(state);
            return state;
        }

        ST current() const {
            return _state;
        }
    };
}

namespace lexer {
    enum class token_type {
        ID_OR_KW,
        INT_LITERAL,
        OPERATOR,
    };

    enum class operator_type {
        UNDEFINED,             //
        OPERATOR_ADD,          // +
        OPERATOR_SUB,          // -
        OPERATOR_MUL,          // *
        OPERATOR_DIV,          // /
        OPERATOR_MOD,          // *
        OPERATOR_ASSIGN,       // =
        OPERATOR_TO,           // ..
        OPERATOR_EQ,           // ==
        OPERATOR_NE,           // !=
        OPERATOR_GT,           // >
        OPERATOR_GE,           // >=
        OPERATOR_LT,           // <
        OPERATOR_LE,           // <=
        OPERATOR_COMMA,        // ,
        OPERATOR_COLON,        // :
        OPERATOR_LPAREN,       // (
        OPERATOR_RPAREN,       // )
        OPERATOR_LBRACKET,     // [
        OPERATOR_RBRACKET,     // ]
        OPERATOR_LBRACE,       // {
        OPERATOR_RBRACE,       // }

        OPERATOR_SEMI,
    };

    ////////////////////////////////////////////////////////////////////////////////
    // tokens
    ////////////////////////////////////////////////////////////////////////////////

    struct token {
        std::size_t _line;
        std::size_t _column;
        std::string _token_text;
        token_type _type;

        explicit token(std::size_t line, std::size_t column,
                       std::string text, token_type type)
            : _line(line), _column(column),
              _token_text(std::move(text)), _type(type) {}

        virtual ~token() = default;
    };

    struct token_operator : public token {
        std::string _value;
        operator_type _op_type;

        explicit token_operator(std::size_t line, std::size_t column,
                                std::string text, std::string value,
                                operator_type type)
            : token(line, column, std::move(text), token_type::OPERATOR),
              _value(std::move(value)), _op_type(type) {}

        ~token_operator() override = default;
    };

    struct token_id_or_kw : public token {
        std::string _value;

        explicit token_id_or_kw(std::size_t line, std::size_t column,
                                std::string text, std::string value)
            : token(line, column, std::move(text), token_type::ID_OR_KW),
              _value(std::move(value)) {}

        ~token_id_or_kw() override = default;
    };

    struct token_int_literal : public token {
        int32_t _value;

        explicit token_int_literal(std::size_t line, std::size_t column,
                                   std::string text, int32_t value)
            : token(line, column, std::move(text), token_type::INT_LITERAL),
              _value(value) {}

        ~token_int_literal() override = default;
    };

    ////////////////////////////////////////////////////////////////////////////////
    // lexer state / lexer input
    ////////////////////////////////////////////////////////////////////////////////

    enum class lexer_state {
        GLOBAL,
        INT_LIT,
        OPERATOR,

        ERROR_OPERATOR,
    };

    using state_manager = shared::state_manager<lexer_state, lexer_state::GLOBAL>;

    struct lexer_input {
        using CharT = char;
    private:
        std::string _source;
        const CharT *_begin = nullptr;
        std::size_t _length = 0;

    public:
        void source(std::string data) {
            std::swap(_source, data);
            this->_begin = _source.c_str();
            this->_length = _source.length();
        }

        const CharT *begin() const {
            return _begin;
        }

        const CharT *end() const {
            return _begin + _length;
        }
    };

    struct lexer_error : public std::runtime_error {
        std::size_t _line;
        std::size_t _start_column;
        std::size_t _end_column;
        std::string _error_text;

        explicit lexer_error(std::size_t line, std::size_t start_column, std::size_t end_column,
                             std::string error_text, const std::string &message)
            : std::runtime_error(message), _line(line),
              _start_column(start_column), _end_column(end_column),
              _error_text(std::move(error_text)) {
        }

        ~lexer_error() override = default;
    };

    ////////////////////////////////////////////////////////////////////////////////
    // lexer
    ////////////////////////////////////////////////////////////////////////////////

    struct lexer {
        using CharT = char;
        using iter_t = const CharT *;
    private:
        state_manager _state;
        lexer_input _input;
        std::unordered_map<std::string, operator_type> _op_maps;

        template <typename T, typename ...Args>
        std::unique_ptr<token> make_token(std::size_t line, iter_t line_start,
                                          iter_t token_start, iter_t token_end,
                                          Args &&...args) {
            return std::unique_ptr<token>(
                new T{line, static_cast<std::size_t>(token_start - line_start),
                      {token_start, static_cast<std::size_t>(token_end - token_start)},
                      std::forward<Args>(args)...}
            );
        }

        __attribute__((noreturn))
        void error(std::size_t line, iter_t line_start,
                   iter_t token_start, iter_t token_end,
                   const std::string &message) {
            mpp::throw_ex<lexer_error>(
                line, static_cast<std::size_t>(token_start - line_start),
                static_cast<std::size_t>(token_end - line_start),
                std::string{token_start, static_cast<std::size_t>(token_end - token_start)},
                message
            );
            std::terminate();
        }

        // return true if it's id or keyword
        bool is_id_or_kw(CharT c, bool first) const {
            return (c >= 'a' && c <= 'z')
                   || (c >= 'A' && c <= 'Z')
                   || c == '$'
                   || c == '_'
                   || (!first && is_digit_char(c));
        }

        bool is_separator_char(CharT c) const {
            return c == ' '
                   || c == '\n'
                   || c == '\r'
                   || c == '\t'
                   || c == '\f'
                   || c == '\v'
                   || c == ';';
        }

        bool is_digit_char(CharT c) const {
            return (c >= '0' && c <= '9');
        }

        void consume_preprocessor(iter_t &current, iter_t end) {
            iter_t left = current;
            auto length = static_cast<std::size_t>(end - current);
            auto *s = std::char_traits<CharT>::find(current, length, '\n');

            current = s ? s : end;
        }

        std::pair<int32_t, double> consume_number(iter_t &current, iter_t end) {
            int32_t integer_part = *current++ - '0';
            if (current == end) {
                _state.new_state(lexer_state::INT_LIT);
                return std::make_pair(integer_part, 0);
            }

            // parsing dec number
            while (current < end) {
                if (is_digit_char(*current)) {
                    integer_part = integer_part * 10 + *current++ - '0';
                } else {
                    // consumed the number
                    break;
                }
            }

            _state.new_state(lexer_state::INT_LIT);
            return std::make_pair(integer_part, 0);
        }

        std::string consume_id_or_kw(iter_t &current, iter_t end) {
            // start part
            iter_t left = current++;
            while (current < end && is_id_or_kw(*current, false)) {
                ++current;
            }
            return {left, static_cast<std::size_t>(current - left)};
        }

        std::pair<std::string, operator_type> consume_operator(iter_t &current, iter_t end) {
            iter_t left = current;

            // be greedy, be lookahead
            while (current < end
                   && !is_separator_char(*current)
                   && !is_id_or_kw(*current, false)) {
                ++current;
            }

            iter_t most = current;
            while (current != left) {
                std::string op = {left, static_cast<std::size_t>(current - left)};
                auto iter = _op_maps.find(op);
                if (iter != _op_maps.end()) {
                    _state.new_state(lexer_state::OPERATOR);
                    return std::make_pair(op, iter->second);
                }
                // lookahead failed, try previous one
                --current;
            }

            _state.new_state(lexer_state::ERROR_OPERATOR);
            return std::make_pair(
                std::string{left, static_cast<std::size_t>(most - left)},
                operator_type::UNDEFINED);
        }

    public:
        void source(const std::string &str) {
            _input.source(str);
        }

        void add_operators(const std::unordered_map<std::string, operator_type> &ops) {
            _op_maps.insert(ops.begin(), ops.end());
        }

        void lex(std::deque<std::unique_ptr<token>> &tokens) {
            iter_t p = _input.begin();
            iter_t end = _input.end();

            // current line start position
            std::size_t line_no = 1;
            iter_t line_start = p;

            while (p < end) {
                /////////////////////////////////////////////////////////////////
                // special position
                /////////////////////////////////////////////////////////////////
                // tokens only available in the beginning of a line
                if (line_start == p) {
                    if (*p == '#') {
                        // comment and preprocessor tag in CovScript 3
                        consume_preprocessor(p, end);
                        continue;
                    }
                }

                /////////////////////////////////////////////////////////////////
                // global state
                /////////////////////////////////////////////////////////////////

                // if we meet \n
                if (*p == '\n') {
                    ++line_no;
                    line_start = ++p;
                    continue;
                }

                // skip separators
                if (is_separator_char(*p)) {
                    ++p;
                    continue;
                }

                // digit
                if (is_digit_char(*p)) {
                    iter_t token_start = p;
                    auto result = consume_number(p, end);
                    switch (_state.pop()) {
                        case lexer_state::INT_LIT:
                            tokens.push_back(make_token<token_int_literal>(
                                line_no, line_start, token_start, p, result.first));
                            break;
                        default:
                            error(line_no, line_start, token_start, p,
                                "<internal error>: illegal state in number literal");
                    }
                    continue;
                }

                // id or kw
                if (is_id_or_kw(*p, true)) {
                    iter_t token_start = p;
                    auto value = consume_id_or_kw(p, end);
                    tokens.push_back(make_token<token_id_or_kw>(
                        line_no, line_start, token_start, p, value));
                    continue;
                }

                iter_t token_start = p;
                // we have operators
                auto value = consume_operator(p, end);
                switch (_state.pop()) {
                    case lexer_state::OPERATOR:
                        tokens.push_back(make_token<token_operator>(
                            line_no, line_start, token_start, p, value.first, value.second));
                        break;
                    case lexer_state::ERROR_OPERATOR:
                        error(line_no, line_start, token_start, p,
                            "unexpected token");
                    default:
                        error(line_no, line_start, token_start, p,
                            "<internal error>: illegal state in post-done lex");
                }
            }
        }
    };
}

namespace rt {
    enum class type_type {
        INT,
        ARRAY,
    };

    struct type {
        type_type _type_type;

        explicit type(type_type tt) : _type_type(tt) {}

        type_type get_type_type() const {
            return _type_type;
        }
    };

    struct int_type : public type {
        int_type() : type(type_type::INT) {}
    };

    struct array_type : public type {
        size_t _start_index;
        size_t _length;

        array_type(size_t start, size_t length)
            : type(type_type::ARRAY),
              _start_index(start),
              _length(length) {}
    };
}

namespace tree {
    using namespace lexer;
    using namespace rt;

    enum class node_type {
        PROGRAM,
        DECL,
        EXPR,
        STMT,
    };

    enum class expr_type {
        ATOM_ID,
        ATOM_VISIT,
        ATOM_LIT,
        BINARY_RELATION,
        BINARY_MATH,
    };

    enum class stmt_type {
        FOR,
        WHILE,
        IF,
        SET,
        PRINT,
    };

    struct node {
        node_type _node_type;

        explicit node(node_type type) : _node_type(type) {}

        node_type get_type() const {
            return _node_type;
        }
    };

    struct decl : public node {
        std::unordered_map<std::string, std::unique_ptr<rt::type>> _vars;

        decl() : node(node_type::DECL) {}
    };

    struct expr : public node {
        expr_type _expr_type;

        explicit expr(expr_type type) : node(node_type::EXPR), _expr_type(type) {}
    };

    struct expr_atom : public expr {
        std::string _str;
        int _i32;
        std::unique_ptr<expr> _index;

        expr_atom(expr_type type)
            : expr(type) {}
    };

    struct expr_binary : public expr {
        operator_type op_type;
        std::unique_ptr<expr> lhs;
        std::unique_ptr<expr> rhs;

        explicit expr_binary(expr_type type) : expr(type) {}
    };

    struct stmt : public node {
        stmt_type _stmt_type;

        explicit stmt(stmt_type type) : node(node_type::STMT), _stmt_type(type) {}
    };

    struct stmt_for : public stmt {
        std::string _var;
        std::unique_ptr<expr> _start;
        std::unique_ptr<expr> _end;
        std::vector<std::unique_ptr<stmt>> _body;

        stmt_for() : stmt(stmt_type::FOR) {}
    };

    struct stmt_while : public stmt {
        std::unique_ptr<expr> _cond;
        std::vector<std::unique_ptr<stmt>> _body;

        stmt_while() : stmt(stmt_type::WHILE) {}
    };

    struct stmt_if : public stmt {
        std::unique_ptr<expr> _cond;
        std::vector<std::unique_ptr<stmt>> _body;

        stmt_if() : stmt(stmt_type::IF) {}
    };

    struct stmt_set : public stmt {
        std::unique_ptr<expr> _var;
        std::unique_ptr<expr> _value;

        stmt_set() : stmt(stmt_type::SET) {}
    };

    struct stmt_print : public stmt {
        std::unique_ptr<expr> _var;

        stmt_print() : stmt(stmt_type::PRINT) {}
    };

    struct program : public node {
        std::unique_ptr<decl> _decl;
        std::vector<std::unique_ptr<stmt>> _stmts;

        program() : node(node_type::PROGRAM) {}
    };
}

namespace parser {
    using namespace lexer;
    using namespace tree;
    using namespace rt;

    enum parser_state {
        GLOBAL,
    };

    using state_manager = shared::state_manager<parser_state, parser_state::GLOBAL>;

    struct parser_error : public std::runtime_error {
        std::unique_ptr<token> _offending;

        parser_error(const std::string &message)
            : std::runtime_error(message) {
        }

        parser_error(const std::string &message, std::unique_ptr<token> offending)
            : std::runtime_error(message), _offending(std::move(offending)) {
        }
    };

    struct parser {
    private:
        std::deque<std::unique_ptr<token>> _tokens;
        state_manager _state;

    public:
        explicit parser(std::deque<std::unique_ptr<token>> tokens) : _tokens(std::move(tokens)) {}

    private:
        bool has_token() {
            return !_tokens.empty();
        }

        void ensure_token() {
            if (!has_token()) {
                mpp::throw_ex<parser_error>("unexpected EOF");
            }
        }

        void consume(token_type type, const std::string &token_text) {
            ensure_token();

            auto top = std::move(_tokens.front());
            _tokens.pop_front();
            if (top->_type != type || top->_token_text != token_text) {
                mpp::throw_ex<parser_error>("unexpected token", std::move(top));
            }
        }

        auto consume(token_type type) {
            ensure_token();

            auto top = std::move(_tokens.front());
            _tokens.pop_front();
            if (top->_type != type) {
                mpp::throw_ex<parser_error>("unexpected token", std::move(top));
            }
            return std::move(top);
        }

        auto consume(operator_type type) {
            ensure_token();

            auto top = std::move(_tokens.front());
            _tokens.pop_front();
            if (top->_type != token_type::OPERATOR) {
                mpp::throw_ex<parser_error>("unexpected token", std::move(top));
            }

            auto *opt = static_cast<token_operator *>(top.get());
            if (opt->_op_type != type) {
                mpp::throw_ex<parser_error>("unexpected token", std::move(top));
            }
            return std::move(top);
        }

        bool coming(operator_type type) {
            if (!has_token()) {
                return false;
            }

            auto &top = _tokens.front();
            if (top->_type != token_type::OPERATOR) {
                return false;
            }

            auto *opt = static_cast<token_operator *>(top.get());
            return opt->_op_type == type;
        }

        bool coming(token_type type) {
            if (!has_token()) {
                return false;
            }

            auto &top = _tokens.front();
            return top->_type == type;
        }

        int consume_int() {
            ensure_token();

            auto lit = consume(token_type::INT_LITERAL);
            auto *p = static_cast<token_int_literal *>(lit.get());
            return p->_value;
        }

    private:
        bool is_operator(const std::string &text) {
            return text == "lt"
                   || text == "le"
                   || text == "gt"
                   || text == "ge"
                   || text == "eq"
                   || text == "ne";
        }

        operator_type to_operator(const std::string &text) {
            if (text == "lt") {
                return operator_type::OPERATOR_LT;
            } else if (text == "le") {
                return operator_type::OPERATOR_LE;
            } else if (text == "gt") {
                return operator_type::OPERATOR_GT;
            } else if (text == "ge") {
                return operator_type::OPERATOR_GE;
            } else if (text == "ne") {
                return operator_type::OPERATOR_NE;
            } else if (text == "eq") {
                return operator_type::OPERATOR_EQ;
            } else if (text == "+") {
                return operator_type::OPERATOR_ADD;
            } else if (text == "-") {
                return operator_type::OPERATOR_SUB;
            }
            mpp::throw_ex<parser_error>("unsupported operator");
        }

        std::unique_ptr<rt::type> parse_type() {
            auto name = consume(token_type::ID_OR_KW);
            if (name->_token_text == "int") {
                return std::make_unique<rt::int_type>();
            }
            if (name->_token_text == "array") {
                consume(operator_type::OPERATOR_LBRACKET);
                consume(token_type::ID_OR_KW, "int");
                consume(operator_type::OPERATOR_COMMA);
                auto start = consume_int();
                consume(operator_type::OPERATOR_TO);
                auto end = consume_int();
                consume(operator_type::OPERATOR_RBRACKET);
                return std::make_unique<rt::array_type>(start, end - start + 1);
            }

            mpp::throw_ex<parser_error>("unsupported type name", std::move(name));
        }

        std::string parse_id() {
            auto id = consume(token_type::ID_OR_KW);
            return id->_token_text;
        }

        std::unique_ptr<decl> parse_decl() {
            consume(operator_type::OPERATOR_LBRACE);
            consume(token_type::ID_OR_KW, "vars");

            auto d = std::make_unique<decl>();

            while (!coming(operator_type::OPERATOR_RBRACE)) {
                auto name = consume(token_type::ID_OR_KW);
                consume(operator_type::OPERATOR_COLON);
                auto type = parse_type();
                d->_vars.insert(std::make_pair(name->_token_text, std::move(type)));
            }

            consume(operator_type::OPERATOR_RBRACE);
            return d;
        }

        std::unique_ptr<stmt> parse_block_stmt() {
            consume(operator_type::OPERATOR_LBRACE);
            auto kw = consume(token_type::ID_OR_KW);
            if (kw->_token_text == "while") {
                auto loop = std::make_unique<stmt_while>();
                loop->_cond = parse_expr();
                loop->_body = parse_stmts();
                consume(operator_type::OPERATOR_RBRACE);
                return loop;

            } else if (kw->_token_text == "hor") {
                auto loop = std::make_unique<stmt_for>();
                loop->_var = parse_id();
                consume(operator_type::OPERATOR_COMMA);
                loop->_start = parse_expr();
                consume(operator_type::OPERATOR_COMMA);
                loop->_end = parse_expr();
                loop->_body = parse_stmts();
                consume(operator_type::OPERATOR_RBRACE);
                return loop;

            } else if (kw->_token_text == "ihu") {
                auto cond = std::make_unique<stmt_if>();
                cond->_cond = parse_expr();
                cond->_body = parse_stmts();
                consume(operator_type::OPERATOR_RBRACE);
                return cond;
            }

            mpp::throw_ex<parser_error>("unsupported loop statement");
        }

        std::unique_ptr<stmt> parse_stmt() {
            if (coming(operator_type::OPERATOR_LBRACE)) {
                return parse_block_stmt();
            }

            consume(operator_type::OPERATOR_COLON);
            auto kw = consume(token_type::ID_OR_KW);
            if (kw->_token_text == "yosoro") {
                auto print = std::make_unique<stmt_print>();
                print->_var = parse_expr();
                return print;
            } else if (kw->_token_text == "set") {
                auto set = std::make_unique<stmt_set>();
                set->_var = parse_expr();
                consume(operator_type::OPERATOR_COMMA);
                set->_value = parse_expr();
                return set;
            }

            mpp::throw_ex<parser_error>("unsupported statement");
        }

        std::vector<std::unique_ptr<stmt>> parse_stmts() {
            std::vector<std::unique_ptr<stmt>> v{};
            while (coming(operator_type::OPERATOR_COLON)
                   || coming(operator_type::OPERATOR_LBRACE)) {
                v.push_back(parse_stmt());
            }
            return v;
        }

        std::unique_ptr<expr> parse_expr_relation(const std::string &op) {
            auto expr = std::make_unique<expr_binary>(expr_type::BINARY_RELATION);
            expr->op_type = to_operator(op);
            consume(operator_type::OPERATOR_COMMA);
            expr->lhs = parse_expr();
            consume(operator_type::OPERATOR_COMMA);
            expr->rhs = parse_expr();
            return expr;
        }

        std::unique_ptr<expr> parse_atom_lit() {
            auto lit = consume(token_type::INT_LITERAL);
            auto *ptr = static_cast<token_int_literal *>(lit.get());
            auto expr = std::make_unique<expr_atom>(expr_type::ATOM_LIT);
            expr->_i32 = ptr->_value;
            return expr;
        }

        std::unique_ptr<expr> lookahead_parse_visit(const std::string &id) {
            auto expr = std::make_unique<expr_atom>(expr_type::ATOM_ID);
            expr->_str = id;
            if (coming(operator_type::OPERATOR_LBRACKET)) {
                consume(operator_type::OPERATOR_LBRACKET);
                auto index = parse_expr();
                expr->_expr_type = expr_type::ATOM_VISIT;
                expr->_index = std::move(index);
                consume(operator_type::OPERATOR_RBRACKET);
            }
            return expr;
        }

        std::unique_ptr<expr> lookahead_parse_binary(std::unique_ptr<expr> lhs) {
            if (coming(operator_type::OPERATOR_ADD)
                || coming(operator_type::OPERATOR_SUB)) {
                auto op = consume(token_type::OPERATOR);
                auto rhs = parse_expr();
                auto expr = std::make_unique<expr_binary>(expr_type::BINARY_MATH);
                expr->op_type = to_operator(op->_token_text);
                expr->lhs = std::move(lhs);
                expr->rhs = std::move(rhs);
                return expr;
            }
            return lhs;
        }

        std::unique_ptr<expr> parse_expr() {
            if (coming(token_type::ID_OR_KW)) {
                auto kw = consume(token_type::ID_OR_KW);
                if (is_operator(kw->_token_text)) {
                    return parse_expr_relation(kw->_token_text);
                } else {
                    auto lhs = lookahead_parse_visit(kw->_token_text);
                    return lookahead_parse_binary(std::move(lhs));
                }
            } else if (coming(token_type::INT_LITERAL)) {
                auto lit = parse_atom_lit();
                return lookahead_parse_binary(std::move(lit));
            }

            mpp::throw_ex<parser_error>("Unsupported expression");
        }

    public:
        std::unique_ptr<program> parse_program() {
            auto prog = std::make_unique<program>();
            prog->_decl = parse_decl();
            prog->_stmts = parse_stmts();
            return prog;
        }
    };
}

namespace rt {
    using namespace tree;

    struct value {
        type_type _type;
        int _start;
        int *_value;
    };

    struct interpreter {
    private:
        std::unordered_map<std::string, std::shared_ptr<value>> _vars;

    private:
        std::shared_ptr<value> create_var(std::unique_ptr<rt::type> &type) {
            auto val = std::make_shared<value>();
            val->_type = type->get_type_type();

            switch (type->get_type_type()) {
                case rt::type_type::INT:
                    val->_value = new int;
                    val->_start = 0;
                    break;
                case rt::type_type::ARRAY:
                    auto t = static_cast<array_type *>(type.get());
                    val->_value = new int[t->_length];
                    val->_start = t->_start_index;
                    break;
            }
            return val;
        }

        void run_decls(std::unique_ptr<decl> &decls) {
            for (auto &&decl : decls->_vars) {
                _vars.insert(std::make_pair(decl.first, create_var(decl.second)));
            }
        }

        std::shared_ptr<value> wrap_value(int *ptr) {
            auto result = std::make_shared<value>();
            result->_start = 0;
            result->_type = type_type::INT;
            result->_value = ptr;
            return result;
        }

        std::shared_ptr<value> wrap_value(int val) {
            auto result = std::make_shared<value>();
            result->_start = 0;
            result->_type = type_type::INT;
            result->_value = new int(val);
            return result;
        }

        std::shared_ptr<value> eval_expr_atom_id(expr_atom *expr) {
            auto iter = _vars.find(expr->_str);
            if (iter == _vars.end()) {
                mpp::throw_ex<std::runtime_error>("Variable not found");
            }
            return iter->second;
        }

        std::shared_ptr<value> eval_expr_atom_visit(expr_atom *expr) {
            auto arr = eval_expr_atom_id(expr);
            int index = *eval_expr(expr->_index)->_value;
            int offset = index - arr->_start;
            return wrap_value(arr->_value + offset);
        }

        std::shared_ptr<value> eval_expr_atom_lit(expr_atom *expr) {
            return wrap_value(expr->_i32);
        }

        std::shared_ptr<value> eval_expr_binary_relation(expr_binary *expr) {
            auto lhs = eval_expr(expr->lhs);
            auto rhs = eval_expr(expr->rhs);
            switch (expr->op_type) {
                case operator_type::OPERATOR_LT:
                    return wrap_value(*(lhs->_value) < *(rhs->_value));
                case operator_type::OPERATOR_LE:
                    return wrap_value(*(lhs->_value) <= *(rhs->_value));
                case operator_type::OPERATOR_GT:
                    return wrap_value(*(lhs->_value) > *(rhs->_value));
                case operator_type::OPERATOR_GE:
                    return wrap_value(*(lhs->_value) >= *(rhs->_value));
                case operator_type::OPERATOR_NE:
                    return wrap_value(*(lhs->_value) != *(rhs->_value));
                case operator_type::OPERATOR_EQ:
                    return wrap_value(*(lhs->_value) == *(rhs->_value));
            }
            mpp::throw_ex<std::runtime_error>("unsupported relation operator");
        }

        std::shared_ptr<value> eval_expr_binary_math(expr_binary *expr) {
            auto lhs = eval_expr(expr->lhs);
            auto rhs = eval_expr(expr->rhs);
            switch (expr->op_type) {
                case operator_type::OPERATOR_ADD:
                    return wrap_value(*(lhs->_value) + *(rhs->_value));
                case operator_type::OPERATOR_SUB:
                    return wrap_value(*(lhs->_value) - *(rhs->_value));
            }
            mpp::throw_ex<std::runtime_error>("unsupported math operator");
        }

        std::shared_ptr<value> eval_expr(std::unique_ptr<expr> &expr) {
            switch (expr->_expr_type) {
                case expr_type::ATOM_ID:
                    return eval_expr_atom_id(static_cast<expr_atom *>(expr.get()));
                case expr_type::ATOM_VISIT:
                    return eval_expr_atom_visit(static_cast<expr_atom *>(expr.get()));
                case expr_type::ATOM_LIT:
                    return eval_expr_atom_lit(static_cast<expr_atom *>(expr.get()));
                case expr_type::BINARY_RELATION:
                    return eval_expr_binary_relation(static_cast<expr_binary *>(expr.get()));
                case expr_type::BINARY_MATH:
                    return eval_expr_binary_math(static_cast<expr_binary *>(expr.get()));
            }
        }

        void run_stmt_for(stmt_for *stmt) {
            auto start = *eval_expr(stmt->_start)->_value;
            auto end = *eval_expr(stmt->_end)->_value;
            _vars.insert(std::make_pair(stmt->_var, wrap_value(start)));

            for (int i = start; i <= end; ++i) {
                *(_vars[stmt->_var])->_value = i;
                run_stmts(stmt->_body);
            }
        }

        void run_stmt_while(stmt_while *stmt) {
            while (true) {
                auto cond = *eval_expr(stmt->_cond)->_value;
                if (!cond) {
                    break;
                }
                run_stmts(stmt->_body);
            }
        }

        void run_stmt_if(stmt_if *stmt) {
            auto cond = *eval_expr(stmt->_cond)->_value;
            if (cond) {
                run_stmts(stmt->_body);
            }
        }

        void run_stmt_set(stmt_set *stmt) {
            auto var = eval_expr(stmt->_var);
            auto value = eval_expr(stmt->_value);
            if (var->_type != value->_type) {
                mpp::throw_ex<std::runtime_error>("unsupported assign");
            }

            *(var->_value) = *(value->_value);
        }

        void run_stmt_print(stmt_print *stmt) {
            auto val = eval_expr(stmt->_var);
            switch (val->_type) {
                case type_type::INT:
                    printf("%d ", *val->_value);
                    break;
                case type_type::ARRAY:
                    mpp::throw_ex<std::runtime_error>("unsupported print array");
            }
        }

        void run_stmts(const std::vector<std::unique_ptr<stmt>> &stmts) {
            for (auto &&stmt : stmts) {
                switch (stmt->_stmt_type) {
                    case stmt_type::FOR:
                        run_stmt_for(static_cast<stmt_for *>(stmt.get()));
                        break;
                    case stmt_type::WHILE:
                        run_stmt_while(static_cast<stmt_while *>(stmt.get()));
                        break;
                    case stmt_type::IF:
                        run_stmt_if(static_cast<stmt_if *>(stmt.get()));
                        break;
                    case stmt_type::SET:
                        run_stmt_set(static_cast<stmt_set *>(stmt.get()));
                        break;
                    case stmt_type::PRINT:
                        run_stmt_print(static_cast<stmt_print *>(stmt.get()));
                        break;
                }
            }
        }

    public:
        void run(std::unique_ptr<program> prog) {
            run_decls(prog->_decl);
            run_stmts(prog->_stmts);
        }
    };
}

int main() {
    using lexer::operator_type;
    using lexer::token_type;
    using lexer::token;

    lexer::lexer lex;
    lex.add_operators({
        {"+",  operator_type::OPERATOR_ADD},
        {"-",  operator_type::OPERATOR_SUB},
        {"*",  operator_type::OPERATOR_MUL},
        {"/",  operator_type::OPERATOR_DIV},
        {"%",  operator_type::OPERATOR_MOD},
        {"=",  operator_type::OPERATOR_ASSIGN},
        {"==", operator_type::OPERATOR_EQ},
        {"!=", operator_type::OPERATOR_NE},
        {">",  operator_type::OPERATOR_GT},
        {">=", operator_type::OPERATOR_GE},
        {"<",  operator_type::OPERATOR_LT},
        {"<=", operator_type::OPERATOR_LE},
        {":",  operator_type::OPERATOR_COLON},
        {",",  operator_type::OPERATOR_COMMA},
        {"(",  operator_type::OPERATOR_LPAREN},
        {")",  operator_type::OPERATOR_RPAREN},
        {"[",  operator_type::OPERATOR_LBRACKET},
        {"]",  operator_type::OPERATOR_RBRACKET},
        {"{",  operator_type::OPERATOR_LBRACE},
        {"}",  operator_type::OPERATOR_RBRACE},
        {";",  operator_type::OPERATOR_SEMI},
        {"..", operator_type::OPERATOR_TO},
    });
    lex.source("{ vars\n"
               "    chika:int\n"
               "    you:int\n"
               "    ruby:array[int, 1..2]\n"
               "    i:int\n"
               "}\n"
               "# 以上变量默认值均为0\n"
               "# 变量名只可是英文字母。\n"
               "\n"
               "# yosoro语句可以输出一个数字，随后跟一个空格。\n"
               ":yosoro 2\n"
               "# 输出2和一个空格(以下不再提到空格)。\n"
               "\n"
               "# set语句可以为变量赋值。\n"
               "# 运算符只支持加减号即可。\n"
               ":set chika, 1\n"
               ":set you, 2\n"
               ":yosoro chika + you\n"
               "# 上一条语句将输出3\n"
               "\n"
               "# 以下的判断语句均使用以下的格式：\n"
               "# 操作符，表达式，表达式\n"
               "# 例如eq, a, 1即C语言中 a==1\n"
               "# 所有操作符包括: lt: < gt: > le: <= ge: >= eq: == neq: !=\n"
               "\n"
               "# 日本来的CYaRon三人没法正确地发出if这个音，因此她们就改成了ihu。\n"
               "{ ihu eq, chika, 1\n"
               "    :set you, 3\n"
               "    :yosoro 1\n"
               "}\n"
               "# 输出1\n"
               "# 以上是ihu语句，无需支持else。\n"
               "\n"
               "# hor语句同理，\n"
               "# for i=1 to you如下\n"
               "{ hor i, 1, you\n"
               "    :yosoro i\n"
               "}\n"
               "# 输出1 2 3\n"
               "\n"
               "# 如下是while和数组的使用方法。\n"
               ":set i, 1\n"
               "{ while le, i, 2\n"
               "    :yosoro i\n"
               "    :set ruby[i], i+1\n"
               "    :yosoro ruby[i]\n"
               "    :set i, i+1\n"
               "}\n"
               "# 输出1 2 2 3\n"
               "\n"
               "# 数组不会出现嵌套，即只会有a[i]、a[i+2]而不会有类似于a[i+b[i]]这样的。\n"
               "\n"
               "# CYaRon语的最后一行，一定是一个换行。");

    std::deque<std::unique_ptr<token>> tokens;
    lex.lex(tokens);

    parser::parser par{std::move(tokens)};
    auto p = par.parse_program();

    rt::interpreter interp;
    interp.run(std::move(p));
}

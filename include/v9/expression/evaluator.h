//
// Created by kiva on 2018/4/20.
//
#include <cstdio>
#include <string>
#include <stack>
#include <cmath>

namespace v9 {
    namespace expression {
        enum TokenType {
            NUMBER, OPERATOR, WHITESPACE
        };

        enum Operator {
            ADD = '+',
            SUB = '-',
            MUL = '*',
            DIV = '/',
        };

        using Number = float;

        struct Token {
            TokenType tokenType;
            union {
                Number num;
                Operator op;
            };
        };

        class Lexer {
        private:
            const char *stream;

        private:
            static inline bool isNumber(char ch) {
                return ch >= '0' && ch <= '9';
            }

        private:
            bool operatorToken(Token *t, char op) {
                t->op = static_cast<Operator>(op);
                t->tokenType = OPERATOR;
                return true;
            }

            bool numberToken(Token *t, char start) {
                t->tokenType = NUMBER;
                Number integer = start - '0';
                Number point = 0;
                int pointCount = 0;
                bool meetDot = false;

                char ch;
                while (*stream) {
                    ch = *stream++;
                    if (ch == '.') {
                        meetDot = true;

                    } else if (isNumber(ch)) {
                        if (meetDot) {
                            ++pointCount;
                            point = point * 10 + ch - '0';
                        } else {
                            integer = integer * 10 + ch - '0';
                        }
                    } else {
                        break;
                    }
                }
                Number f = integer + point * powf(10, -pointCount);
                t->num = f;
                return true;
            }

            bool whiteSpaceToken(Token *t) {
                t->tokenType = WHITESPACE;
                return true;
            }

        public:
            static int priorityFor(Operator op) {
                switch (op) {
                    case ADD:
                    case SUB:
                        return 1;
                    case MUL:
                    case DIV:
                        return 2;
                }
                return 0;
            }

        public:
            explicit Lexer(const char *stream) : stream(stream) {}

            inline bool nextToken(Token *t) {
                char ch;
                while ((ch = *stream++)) {
                    switch (ch) {
                        case '+':
                        case '-':
                        case '*':
                        case '/':
                            return operatorToken(t, ch);

                        case '0'...'9':
                            return numberToken(t, ch);

                        case ' ':
                            return whiteSpaceToken(t);

                        default:
                            return false;
                    }
                }
                return false;
            }
        };

        Number calculate(std::stack<Number> &nums, Operator op) {
            Number rhs = nums.top();
            nums.pop();
            Number lhs = nums.top();
            nums.pop();

            printf("%f %c %f\n", lhs, op, rhs);
            switch (op) {
                case ADD:
                    return lhs + rhs;
                case SUB:
                    return lhs - rhs;
                case MUL:
                    return lhs * rhs;
                case DIV:
                    return lhs / rhs;
            }
            return 0;
        }

        Number evaluate(const std::string &exp) {
            Lexer lexer(exp.c_str());
            Token t{};
            std::stack<Number> nums;

            while (lexer.nextToken(&t)) {
                switch (t.tokenType) {
                    case TokenType::NUMBER:
                        printf("number: %f\n", t.num);
                        nums.push(t.num);
                        break;

                    case TokenType::OPERATOR: {
                        printf("op: %c\n", t.op);
                        nums.push(calculate(nums, t.op));
                        break;
                    }

                    case TokenType::WHITESPACE:
                        break;

                    default:
                        printf("Unknown token: %d\n", t.tokenType);
                }
            }
            return nums.top();
        }
    }
}

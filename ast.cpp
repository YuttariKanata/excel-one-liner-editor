#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <variant>

enum class ExcelError { Div0, Value, Ref, Name, Num, NA, NU_LL };
using Value = std::variant<double, std::string, bool, ExcelError>;

// 演算子の型安全な管理
enum class OpType {
    Add, Sub, Mul, Div, Mod, Pow,       // 四則演算 + べき乗
    Eq, Neq, Lt, Le, Gt, Ge,            // 比較
    Concat                              // 文字列結合 '&'
};

class ExprAST {
public:
    virtual ~ExprAST() = default;
    // virtual Value eval() const = 0; 
};

// 数値 (123.45)
class NumberExprAST : public ExprAST {
    double val;
public:
    NumberExprAST(double val) : val(val) {}
};

// 文字列 ("Hello")
class StringExprAST : public ExprAST {
    std::string val;
public:
    StringExprAST(std::string val) : val(std::move(val)) {}
};

// 二項演算 (lhs + rhs)
class BinaryExprAST : public ExprAST {
    std::string op; // "<=" などに対応できるよう string もしくは enum
    std::unique_ptr<ExprAST> lhs, rhs;
public:
    BinaryExprAST(std::string op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : op(std::move(op)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
};

// 単項演算 (-rhs)
class UnaryExprAST : public ExprAST {
    char op;
    std::unique_ptr<ExprAST> rhs;
public:
    UnaryExprAST(char op, std::unique_ptr<ExprAST> rhs)
        : op(op), rhs(std::move(rhs)) {}
};

// 関数呼び出し (SUM(a, b, c))
class CallExprAST : public ExprAST {
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;
public:
    CallExprAST(std::string callee, std::vector<std::unique_ptr<ExprAST>> args)
        : callee(std::move(callee)), args(std::move(args)) {}
};
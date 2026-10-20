
#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace compiler {

struct SourceLocation {
    int line = 1;
    int column = 1;
};

struct Node {
    SourceLocation location;

    explicit Node(SourceLocation loc = {}) : location(loc) {}
    virtual ~Node() = default;
};

struct Expr : Node {
    using Node::Node;
};

using ExprPtr = std::unique_ptr<Expr>;

struct NumberExpr final : Expr {
    long value;

    NumberExpr(long v, SourceLocation loc = {})
        : Expr(loc), value(v) {}
};

struct IdentifierExpr final : Expr {
    std::string name;

    IdentifierExpr(std::string n, SourceLocation loc = {})
        : Expr(loc), name(std::move(n)) {}
};

struct BinaryExpr final : Expr {
    char op;
    ExprPtr left;
    ExprPtr right;

    BinaryExpr(char operation, ExprPtr lhs, ExprPtr rhs,
               SourceLocation loc = {})
        : Expr(loc),
          op(operation),
          left(std::move(lhs)),
          right(std::move(rhs)) {}
};

struct Stmt : Node {
    using Node::Node;
};

using StmtPtr = std::unique_ptr<Stmt>;

struct DeclarationStmt final : Stmt {
    std::string name;
    ExprPtr initializer;

    DeclarationStmt(std::string n, ExprPtr init,
                    SourceLocation loc = {})
        : Stmt(loc),
          name(std::move(n)),
          initializer(std::move(init)) {}
};

struct AssignmentStmt final : Stmt {
    std::string name;
    ExprPtr value;

    AssignmentStmt(std::string n, ExprPtr expr,
                   SourceLocation loc = {})
        : Stmt(loc),
          name(std::move(n)),
          value(std::move(expr)) {}
};

struct PrintStmt final : Stmt {
    ExprPtr expression;

    explicit PrintStmt(ExprPtr expr, SourceLocation loc = {})
        : Stmt(loc), expression(std::move(expr)) {}
};

struct Program {
    std::vector<StmtPtr> statements;
};

} // namespace compiler

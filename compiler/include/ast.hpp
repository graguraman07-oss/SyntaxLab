
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

struct FloatExpr final : Expr {
    double value;

    FloatExpr(double v, SourceLocation loc = {})
        : Expr(loc), value(v) {}
};

struct CharacterExpr final : Expr {
    std::string value;

    CharacterExpr(std::string v, SourceLocation loc = {})
        : Expr(loc), value(std::move(v)) {}
};

struct StringExpr final : Expr {
    std::string value;

    StringExpr(std::string v, SourceLocation loc = {})
        : Expr(loc), value(std::move(v)) {}
};

struct IdentifierExpr final : Expr {
    std::string name;

    IdentifierExpr(std::string n, SourceLocation loc = {})
        : Expr(loc), name(std::move(n)) {}
};

struct BinaryExpr final : Expr {
    std::string op;
    ExprPtr left;
    ExprPtr right;

    BinaryExpr(std::string operation, ExprPtr lhs, ExprPtr rhs,
               SourceLocation loc = {})
        : Expr(loc),
          op(std::move(operation)),
          left(std::move(lhs)),
          right(std::move(rhs)) {}
};

struct UnaryExpr final : Expr {
    std::string op;
    ExprPtr operand;

    UnaryExpr(std::string operation, ExprPtr expr,
              SourceLocation loc = {})
        : Expr(loc),
          op(std::move(operation)),
          operand(std::move(expr)) {}
};

struct Stmt : Node {
    using Node::Node;
};

using StmtPtr = std::unique_ptr<Stmt>;

struct DeclarationStmt final : Stmt {
    std::string name;
    ExprPtr initializer;
    bool isPointer;
    std::string dataType;

    DeclarationStmt(std::string n, ExprPtr init,
                    SourceLocation loc = {}, bool pointer = false,
                    std::string type = "int")
        : Stmt(loc),
          name(std::move(n)),
          initializer(std::move(init)),
          isPointer(pointer),
          dataType(std::move(type)) {}
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

struct IfStmt final : Stmt {
    ExprPtr condition;
    std::vector<StmtPtr> thenBody;
    std::vector<StmtPtr> elseBody;

    IfStmt(ExprPtr cond,
           std::vector<StmtPtr> thenStatements,
           std::vector<StmtPtr> elseStatements = {},
           SourceLocation loc = {})
        : Stmt(loc),
          condition(std::move(cond)),
          thenBody(std::move(thenStatements)),
          elseBody(std::move(elseStatements)) {}
};

struct WhileStmt final : Stmt {
    ExprPtr condition;
    std::vector<StmtPtr> body;

    WhileStmt(ExprPtr cond, std::vector<StmtPtr> statements,
              SourceLocation loc = {})
        : Stmt(loc),
          condition(std::move(cond)),
          body(std::move(statements)) {}
};

struct ForStmt final : Stmt {
    StmtPtr initializer;
    ExprPtr condition;
    StmtPtr increment;
    std::vector<StmtPtr> body;

    ForStmt(StmtPtr init, ExprPtr cond, StmtPtr step,
            std::vector<StmtPtr> statements,
            SourceLocation loc = {})
        : Stmt(loc),
          initializer(std::move(init)),
          condition(std::move(cond)),
          increment(std::move(step)),
          body(std::move(statements)) {}
};

struct Program {
    std::vector<StmtPtr> statements;
};

}

#include "ast.hpp"
#include "semantic.hpp"
#include "parser.hpp"

#include <iostream>
#include <string>
#include <vector>
struct TokenInfo {
    std::string type;
    std::string value;
    int line;
    int column;
};

std::vector<TokenInfo> collectedTokens;
static std::string jsonEscape(const std::string& s) {
    std::string out;

    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:   out += c; break;
        }
    }

    return out;
}

static void printExpr(const compiler::Expr& expr) {
    using namespace compiler;

    if (auto* n = dynamic_cast<const NumberExpr*>(&expr)) {
        std::cout << "{\"type\":\"Number\",\"value\":"
                  << n->value << "}";
    } else if (auto* id =
                   dynamic_cast<const IdentifierExpr*>(&expr)) {
        std::cout << "{\"type\":\"Identifier\",\"name\":\""
                  << jsonEscape(id->name) << "\"}";
    } else if (auto* b = dynamic_cast<const BinaryExpr*>(&expr)) {
        std::cout << "{\"type\":\"BinaryExpression\",\"operator\":\""
                  << jsonEscape(b->op) << "\",\"left\":";

        if (b->left) printExpr(*b->left);
        else std::cout << "null";

        std::cout << ",\"right\":";

        if (b->right) printExpr(*b->right);
        else std::cout << "null";

        std::cout << "}";
    } else if (auto* u = dynamic_cast<const UnaryExpr*>(&expr)) {
        std::cout << "{\"type\":\"UnaryExpression\",\"operator\":\""
                  << jsonEscape(u->op) << "\",\"operand\":";

        if (u->operand) printExpr(*u->operand);
        else std::cout << "null";

        std::cout << "}";
    } else {
        std::cout << "null";
    }
}

static void printStmt(const compiler::Stmt& stmt) {
    using namespace compiler;

    if (auto* d = dynamic_cast<const DeclarationStmt*>(&stmt)) {
        std::cout << "{\"type\":\"Declaration\",\"name\":\""
                  << jsonEscape(d->name)
                  << "\",\"isPointer\":"
                  << (d->isPointer ? "true" : "false")
                  << ",\"initializer\":";

        if (d->initializer) printExpr(*d->initializer);
        else std::cout << "null";

        std::cout << "}";
    } else if (auto* a =
                   dynamic_cast<const AssignmentStmt*>(&stmt)) {
        std::cout << "{\"type\":\"Assignment\",\"name\":\""
                  << jsonEscape(a->name) << "\",\"value\":";

        if (a->value) printExpr(*a->value);
        else std::cout << "null";

        std::cout << "}";
    } else if (auto* p = dynamic_cast<const PrintStmt*>(&stmt)) {
        std::cout << "{\"type\":\"Print\",\"expression\":";

        if (p->expression) printExpr(*p->expression);
        else std::cout << "null";

        std::cout << "}";
    } else if (auto* node = dynamic_cast<const IfStmt*>(&stmt)) {
        std::cout << "{\"type\":\"If\",\"condition\":";

        if (node->condition) printExpr(*node->condition);
        else std::cout << "null";

        std::cout << ",\"thenBody\":[";
        for (std::size_t i = 0; i < node->thenBody.size(); ++i) {
            if (i) std::cout << ",";
            if (node->thenBody[i]) printStmt(*node->thenBody[i]);
            else std::cout << "null";
        }

        std::cout << "],\"elseBody\":[";
        for (std::size_t i = 0; i < node->elseBody.size(); ++i) {
            if (i) std::cout << ",";
            if (node->elseBody[i]) printStmt(*node->elseBody[i]);
            else std::cout << "null";
        }

        std::cout << "]}";
    } else if (auto* node = dynamic_cast<const WhileStmt*>(&stmt)) {
        std::cout << "{\"type\":\"While\",\"condition\":";

        if (node->condition) printExpr(*node->condition);
        else std::cout << "null";

        std::cout << ",\"body\":[";
        for (std::size_t i = 0; i < node->body.size(); ++i) {
            if (i) std::cout << ",";
            if (node->body[i]) printStmt(*node->body[i]);
            else std::cout << "null";
        }

        std::cout << "]}";
    } else if (auto* node = dynamic_cast<const ForStmt*>(&stmt)) {
        std::cout << "{\"type\":\"For\",\"initializer\":";

        if (node->initializer) printStmt(*node->initializer);
        else std::cout << "null";

        std::cout << ",\"condition\":";

        if (node->condition) printExpr(*node->condition);
        else std::cout << "null";

        std::cout << ",\"increment\":";

        if (node->increment) printStmt(*node->increment);
        else std::cout << "null";

        std::cout << ",\"body\":[";
        for (std::size_t i = 0; i < node->body.size(); ++i) {
            if (i) std::cout << ",";
            if (node->body[i]) printStmt(*node->body[i]);
            else std::cout << "null";
        }

        std::cout << "]}";
    } else {
        std::cout << "null";
    }
}

int main() {
    compiler::Program program;
    std::vector<compiler::Diagnostic> diagnostics;

    yy::parser parser(program, diagnostics);

    if (parser.parse() != 0) {
        std::cout << "{\"success\":false,\"diagnostics\":[";
        for (std::size_t i = 0; i < diagnostics.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << "{\"phase\":\""
                      << jsonEscape(diagnostics[i].phase)
                      << "\",\"line\":" << diagnostics[i].line
                      << ",\"column\":" << diagnostics[i].column
                      << ",\"message\":\""
                      << jsonEscape(diagnostics[i].message)
                      << "\"}";
        }
        std::cout << "]}\n";
        return 1;
    }

    compiler::SemanticAnalyzer analyzer;
    diagnostics = analyzer.analyze(program);

    std::cout << "{\"success\":"
              << (diagnostics.empty() ? "true" : "false")
              << ",\"ast\":{\"type\":\"Program\",\"statements\":[";

    for (std::size_t i = 0; i < program.statements.size(); ++i) {
        if (i) std::cout << ",";
        if (program.statements[i]) printStmt(*program.statements[i]);
        else std::cout << "null";
    }

    std::cout << "]},\"diagnostics\":[";

    for (std::size_t i = 0; i < diagnostics.size(); ++i) {
        if (i) std::cout << ",";
        std::cout << "{\"phase\":\""
                  << jsonEscape(diagnostics[i].phase)
                  << "\",\"line\":" << diagnostics[i].line
                  << ",\"column\":" << diagnostics[i].column
                  << ",\"message\":\""
                  << jsonEscape(diagnostics[i].message)
                  << "\"}";
    }

    std::cout << "]}\n";
    return diagnostics.empty() ? 0 : 1;
}
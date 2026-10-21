#include "ast.hpp"
#include "semantic.hpp"
#include "parser.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>

struct TokenInfo {
    std::string type;
    std::string value;
    int line;
    int column;
};

std::vector<TokenInfo> collectedTokens;

static std::string jsonEscape(const std::string& s) {
    std::ostringstream out;

    for (unsigned char c : s) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (c < 0x20) {
                    out << "\\u"
                        << std::hex << std::setw(4)
                        << std::setfill('0')
                        << static_cast<int>(c)
                        << std::dec;
                } else {
                    out << static_cast<char>(c);
                }
        }
    }

    return out.str();
}

static void printTokens() {
    std::cout << "\"tokens\":[";

    for (std::size_t i = 0; i < collectedTokens.size(); ++i) {
        if (i) std::cout << ",";

        const auto& token = collectedTokens[i];

        std::cout << "{\"type\":\"" << jsonEscape(token.type)
                  << "\",\"value\":\"" << jsonEscape(token.value)
                  << "\",\"line\":" << token.line
                  << ",\"column\":" << token.column << "}";
    }

    std::cout << "]";
}

static void printDiagnostics(
    const std::vector<compiler::Diagnostic>& diagnostics
) {
    std::cout << "\"diagnostics\":[";

    for (std::size_t i = 0; i < diagnostics.size(); ++i) {
        if (i) std::cout << ",";

        const auto& d = diagnostics[i];

        std::cout << "{\"phase\":\"" << jsonEscape(d.phase)
                  << "\",\"line\":" << d.line
                  << ",\"column\":" << d.column
                  << ",\"message\":\"" << jsonEscape(d.message)
                  << "\"}";
    }

    std::cout << "]";
}

static void printExpr(const compiler::Expr& expr) {
    using namespace compiler;

    if (const auto* n = dynamic_cast<const NumberExpr*>(&expr)) {
        std::cout << "{\"type\":\"Number\",\"value\":"
                  << n->value << "}";

    } else if (const auto* f = dynamic_cast<const FloatExpr*>(&expr)) {
        std::cout << "{\"type\":\"Float\",\"value\":"
                  << std::setprecision(15) << f->value << "}";

    } else if (const auto* c = dynamic_cast<const CharacterExpr*>(&expr)) {
        std::cout << "{\"type\":\"Character\",\"value\":\""
                  << jsonEscape(c->value) << "\"}";

    } else if (const auto* s = dynamic_cast<const StringExpr*>(&expr)) {
        std::cout << "{\"type\":\"String\",\"value\":\""
                  << jsonEscape(s->value) << "\"}";

    } else if (const auto* id = dynamic_cast<const IdentifierExpr*>(&expr)) {
        std::cout << "{\"type\":\"Identifier\",\"name\":\""
                  << jsonEscape(id->name) << "\"}";

    } else if (const auto* b = dynamic_cast<const BinaryExpr*>(&expr)) {
        std::cout << "{\"type\":\"BinaryExpression\",\"operator\":\""
                  << jsonEscape(b->op) << "\",\"left\":";

        if (b->left) printExpr(*b->left);
        else std::cout << "null";

        std::cout << ",\"right\":";

        if (b->right) printExpr(*b->right);
        else std::cout << "null";

        std::cout << "}";

    } else if (const auto* u = dynamic_cast<const UnaryExpr*>(&expr)) {
        std::cout << "{\"type\":\"UnaryExpression\",\"operator\":\""
                  << jsonEscape(u->op) << "\",\"operand\":";

        if (u->operand) printExpr(*u->operand);
        else std::cout << "null";

        std::cout << "}";

    } else {
        std::cout << "null";
    }
}

static void printStmt(const compiler::Stmt& stmt);

static void printStatementArray(
    const std::vector<compiler::StmtPtr>& statements
) {
    std::cout << "[";

    for (std::size_t i = 0; i < statements.size(); ++i) {
        if (i) std::cout << ",";

        if (statements[i]) printStmt(*statements[i]);
        else std::cout << "null";
    }

    std::cout << "]";
}

static void printStmt(const compiler::Stmt& stmt) {
    using namespace compiler;

    if (const auto* d = dynamic_cast<const DeclarationStmt*>(&stmt)) {
        std::cout << "{\"type\":\"Declaration\",\"name\":\""
                  << jsonEscape(d->name)
                  << "\",\"dataType\":\""
                  << jsonEscape(d->dataType)
                  << "\",\"isPointer\":"
                  << (d->isPointer ? "true" : "false")
                  << ",\"initializer\":";

        if (d->initializer) printExpr(*d->initializer);
        else std::cout << "null";

        std::cout << "}";

    } else if (const auto* a = dynamic_cast<const AssignmentStmt*>(&stmt)) {
        std::cout << "{\"type\":\"Assignment\",\"name\":\""
                  << jsonEscape(a->name) << "\",\"value\":";

        if (a->value) printExpr(*a->value);
        else std::cout << "null";

        std::cout << "}";

    } else if (const auto* p = dynamic_cast<const PrintStmt*>(&stmt)) {
        std::cout << "{\"type\":\"Print\",\"expression\":";

        if (p->expression) printExpr(*p->expression);
        else std::cout << "null";

        std::cout << "}";

    } else if (const auto* node = dynamic_cast<const IfStmt*>(&stmt)) {
        std::cout << "{\"type\":\"If\",\"condition\":";

        if (node->condition) printExpr(*node->condition);
        else std::cout << "null";

        std::cout << ",\"thenBody\":";
        printStatementArray(node->thenBody);

        std::cout << ",\"elseBody\":";
        printStatementArray(node->elseBody);

        std::cout << "}";

    } else if (const auto* node = dynamic_cast<const WhileStmt*>(&stmt)) {
        std::cout << "{\"type\":\"While\",\"condition\":";

        if (node->condition) printExpr(*node->condition);
        else std::cout << "null";

        std::cout << ",\"body\":";
        printStatementArray(node->body);

        std::cout << "}";

    } else if (const auto* node = dynamic_cast<const ForStmt*>(&stmt)) {
        std::cout << "{\"type\":\"For\",\"initializer\":";

        if (node->initializer) printStmt(*node->initializer);
        else std::cout << "null";

        std::cout << ",\"condition\":";

        if (node->condition) printExpr(*node->condition);
        else std::cout << "null";

        std::cout << ",\"increment\":";

        if (node->increment) printStmt(*node->increment);
        else std::cout << "null";

        std::cout << ",\"body\":";
        printStatementArray(node->body);

        std::cout << "}";

    } else {
        std::cout << "null";
    }
}

int main() {
    compiler::Program program;
    std::vector<compiler::Diagnostic> diagnostics;

    yy::parser parser(program, diagnostics);
    const int parseResult = parser.parse();

    if (parseResult != 0) {
        std::cout << "{\"success\":false,";
        printTokens();
        std::cout << ",";
        printDiagnostics(diagnostics);
        std::cout << "}\n";
        return 1;
    }

    compiler::SemanticAnalyzer analyzer;
    auto semanticDiagnostics = analyzer.analyze(program);

    diagnostics.insert(
        diagnostics.end(),
        semanticDiagnostics.begin(),
        semanticDiagnostics.end()
    );

    std::cout << "{\"success\":"
              << (diagnostics.empty() ? "true" : "false")
              << ",";

    printTokens();

    std::cout << ",\"ast\":{\"type\":\"Program\",\"statements\":[";

    for (std::size_t i = 0; i < program.statements.size(); ++i) {
        if (i) std::cout << ",";

        if (program.statements[i]) printStmt(*program.statements[i]);
        else std::cout << "null";
    }

    std::cout << "]},";
    printDiagnostics(diagnostics);
    std::cout << "}\n";

    return diagnostics.empty() ? 0 : 1;
}

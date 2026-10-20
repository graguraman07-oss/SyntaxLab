
#include "ast.hpp"
#include "parser.hpp"
#include "semantic.hpp"

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

static std::string jsonEscape(const std::string& text) {
    std::string result;

    for (unsigned char ch : text) {
        switch (ch) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (ch < 0x20) {
                    const char hex[] = "0123456789abcdef";
                    result += "\\u00";
                    result += hex[ch >> 4];
                    result += hex[ch & 15];
                } else {
                    result += static_cast<char>(ch);
                }
        }
    }
    return result;
}

static void printExpr(const compiler::Expr& expr) {
    using namespace compiler;

    if (auto* n = dynamic_cast<const NumberExpr*>(&expr)) {
        std::cout << "{\"type\":\"Number\",\"value\":" << n->value << "}";
    } else if (auto* id = dynamic_cast<const IdentifierExpr*>(&expr)) {
        std::cout << "{\"type\":\"Identifier\",\"name\":\""
                  << jsonEscape(id->name) << "\"}";
    } else if (auto* b = dynamic_cast<const BinaryExpr*>(&expr)) {
        std::cout << "{\"type\":\"BinaryExpression\",\"operator\":\""
                  << b->op << "\",\"left\":";
        printExpr(*b->left);
        std::cout << ",\"right\":";
        printExpr(*b->right);
        std::cout << "}";
    } else {
        std::cout << "null";
    }
}

static void printStmt(const compiler::Stmt& stmt) {
    using namespace compiler;

    if (auto* d = dynamic_cast<const DeclarationStmt*>(&stmt)) {
        std::cout << "{\"type\":\"Declaration\",\"name\":\""
                  << jsonEscape(d->name) << "\",\"initializer\":";
        if (d->initializer) printExpr(*d->initializer);
        else std::cout << "null";
        std::cout << "}";
    } else if (auto* a = dynamic_cast<const AssignmentStmt*>(&stmt)) {
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
    } else {
        std::cout << "null";
    }
}

int main() {
    compiler::Program program;
    std::vector<compiler::Diagnostic> diagnostics;

    yy::parser parser(program, diagnostics);
    const int parseStatus = parser.parse();

    if (parseStatus == 0) {
        compiler::SemanticAnalyzer analyzer;
        auto semanticDiagnostics = analyzer.analyze(program);
        diagnostics.insert(
            diagnostics.end(),
            semanticDiagnostics.begin(),
            semanticDiagnostics.end()
        );
    }

    std::cout << "{\n  \"success\": "
              << ((parseStatus == 0 && diagnostics.empty()) ? "true" : "false")
              << ",\n  \"tokens\": [";

    for (std::size_t i = 0; i < collectedTokens.size(); ++i) {
        const auto& t = collectedTokens[i];
        if (i) std::cout << ",";
        std::cout << "\n    {\"type\":\"" << jsonEscape(t.type)
                  << "\",\"value\":\"" << jsonEscape(t.value)
                  << "\",\"line\":" << t.line
                  << ",\"column\":" << t.column << "}";
    }

    if (!collectedTokens.empty()) std::cout << "\n  ";
    std::cout << "],\n  \"ast\": ";

    if (parseStatus == 0) {
        std::cout << "{\"type\":\"Program\",\"statements\":[";
        for (std::size_t i = 0; i < program.statements.size(); ++i) {
            if (i) std::cout << ",";
            printStmt(*program.statements[i]);
        }
        std::cout << "]}";
    } else {
        std::cout << "null";
    }

    std::cout << ",\n  \"diagnostics\": [";
    for (std::size_t i = 0; i < diagnostics.size(); ++i) {
        const auto& d = diagnostics[i];
        if (i) std::cout << ",";
        std::cout << "\n    {\"phase\":\"" << jsonEscape(d.phase)
                  << "\",\"line\":" << d.line
                  << ",\"column\":" << d.column
                  << ",\"message\":\"" << jsonEscape(d.message) << "\"}";
    }

    if (!diagnostics.empty()) std::cout << "\n  ";
    std::cout << "],\n  \"recovered\": "
              << ((parseStatus == 0 && !diagnostics.empty()) ? "true" : "false")
              << "\n}\n";

    return 0;
}

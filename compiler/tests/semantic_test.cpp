
#include "ast.hpp"
#include "semantic.hpp"

#include <iostream>
#include <memory>

int main() {
    compiler::Program program;

    // Declare x = 10
    program.statements.push_back(
        std::make_unique<compiler::DeclarationStmt>(
            "x",
            std::make_unique<compiler::NumberExpr>(10)
        )
    );

    // Test: x = 10 / 0
    auto division = std::make_unique<compiler::BinaryExpr>(
        '/',
        std::make_unique<compiler::NumberExpr>(10),
        std::make_unique<compiler::NumberExpr>(0)
    );

    program.statements.push_back(
        std::make_unique<compiler::AssignmentStmt>(
            "x", std::move(division)
        )
    );

    compiler::SemanticAnalyzer analyzer;
    auto diagnostics = analyzer.analyze(program);

    if (diagnostics.empty()) {
        std::cout << "FAIL: Division by zero was not detected.\n";
        return 1;
    }

    std::cout << "PASS: An error was detected.\n";

    for (const auto& diagnostic : diagnostics) {
        std::cout << diagnostic.phase
                  << " error at "
                  << diagnostic.line << ":"
                  << diagnostic.column << ": "
                  << diagnostic.message << '\n';
    }

    return 0;
}

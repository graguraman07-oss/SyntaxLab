
#include "semantic.hpp"

#include <string>

namespace compiler {

std::vector<Diagnostic>
SemanticAnalyzer::analyze(const Program& program) {
    declared_.clear();
    diagnostics_.clear();

    for (const auto& statement : program.statements) {
        if (statement) {
            analyzeStatement(*statement);
        }
    }

    return diagnostics_;
}

void SemanticAnalyzer::analyzeStatement(const Stmt& statement) {
    if (const auto* declaration =
            dynamic_cast<const DeclarationStmt*>(&statement)) {

        if (declared_.count(declaration->name)) {
            diagnostics_.push_back({
                "semantic",
                declaration->location.line,
                declaration->location.column,
                "Variable '" + declaration->name +
                    "' is already declared."
            });
        } else {
            // Register before checking the initializer so that
            // self-references are treated as declared.
            declared_.insert(declaration->name);
        }

        if (declaration->initializer) {
            analyzeExpression(*declaration->initializer);
        }

        return;
    }

    if (const auto* assignment =
            dynamic_cast<const AssignmentStmt*>(&statement)) {

        if (!declared_.count(assignment->name)) {
            diagnostics_.push_back({
                "semantic",
                assignment->location.line,
                assignment->location.column,
                "Assignment to undeclared variable '" +
                    assignment->name + "'."
            });
        }

        if (assignment->value) {
            analyzeExpression(*assignment->value);
        }

        return;
    }

    if (const auto* printStatement =
            dynamic_cast<const PrintStmt*>(&statement)) {

        if (printStatement->expression) {
            analyzeExpression(*printStatement->expression);
        }
    }
}

void SemanticAnalyzer::analyzeExpression(const Expr& expression) {
    if (const auto* identifier =
            dynamic_cast<const IdentifierExpr*>(&expression)) {

        if (!declared_.count(identifier->name)) {
            diagnostics_.push_back({
                "semantic",
                identifier->location.line,
                identifier->location.column,
                "Use of undeclared variable '" +
                    identifier->name + "'."
            });
        }

        return;
    }

    if (const auto* binary =
            dynamic_cast<const BinaryExpr*>(&expression)) {

        if (binary->left) {
            analyzeExpression(*binary->left);
        }

        if (binary->right) {
            analyzeExpression(*binary->right);
        }

        if (binary->op == '/' && binary->right) {
            if (const auto* number =
                    dynamic_cast<const NumberExpr*>(binary->right.get())) {

                if (number->value == 0) {
                    diagnostics_.push_back({
                        "semantic",
                        binary->right->location.line,
                        binary->right->location.column,
                        "Division by literal zero."
                    });
                }
            }
        }
    }
}

} // namespace compiler

#include "semantic.hpp"

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

        return;
    }

    if (const auto* ifStatement =
            dynamic_cast<const IfStmt*>(&statement)) {

        if (ifStatement->condition) {
            analyzeExpression(*ifStatement->condition);
        }

        for (const auto& child : ifStatement->thenBody) {
            if (child) {
                analyzeStatement(*child);
            }
        }

        for (const auto& child : ifStatement->elseBody) {
            if (child) {
                analyzeStatement(*child);
            }
        }

        return;
    }

    if (const auto* whileStatement =
            dynamic_cast<const WhileStmt*>(&statement)) {

        if (whileStatement->condition) {
            analyzeExpression(*whileStatement->condition);
        }

        for (const auto& child : whileStatement->body) {
            if (child) {
                analyzeStatement(*child);
            }
        }

        return;
    }

    if (const auto* forStatement =
            dynamic_cast<const ForStmt*>(&statement)) {

        if (forStatement->initializer) {
            analyzeStatement(*forStatement->initializer);
        }

        if (forStatement->condition) {
            analyzeExpression(*forStatement->condition);
        }

        if (forStatement->increment) {
            analyzeStatement(*forStatement->increment);
        }

        for (const auto& child : forStatement->body) {
            if (child) {
                analyzeStatement(*child);
            }
        }
    }
}

void SemanticAnalyzer::analyzeExpression(const Expr& expression) {
    if (dynamic_cast<const NumberExpr*>(&expression) ||
        dynamic_cast<const FloatExpr*>(&expression) ||
        dynamic_cast<const CharacterExpr*>(&expression) ||
        dynamic_cast<const StringExpr*>(&expression)) {
        return;
    }

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

        if (binary->op == "/" && binary->right) {
            if (const auto* number =
                    dynamic_cast<const NumberExpr*>(
                        binary->right.get())) {

                if (number->value == 0) {
                    diagnostics_.push_back({
                        "semantic",
                        binary->right->location.line,
                        binary->right->location.column,
                        "Division by literal zero."
                    });
                }
            }

            if (const auto* number =
                    dynamic_cast<const FloatExpr*>(
                        binary->right.get())) {

                if (number->value == 0.0) {
                    diagnostics_.push_back({
                        "semantic",
                        binary->right->location.line,
                        binary->right->location.column,
                        "Division by literal zero."
                    });
                }
            }
        }

        return;
    }

    if (const auto* unary =
            dynamic_cast<const UnaryExpr*>(&expression)) {

        if (unary->operand) {
            analyzeExpression(*unary->operand);
        }
    }
}

}

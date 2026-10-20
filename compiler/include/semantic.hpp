
#pragma once

#include "ast.hpp"

#include <string>
#include <unordered_set>
#include <vector>

namespace compiler {

struct Diagnostic {
    std::string phase;
    int line = 1;
    int column = 1;
    std::string message;
};

class SemanticAnalyzer {
public:
    std::vector<Diagnostic> analyze(const Program& program);

private:
    std::unordered_set<std::string> declared_;
    std::vector<Diagnostic> diagnostics_;

    void analyzeStatement(const Stmt& statement);
    void analyzeExpression(const Expr& expression);
};

} // namespace compiler


%skeleton "lalr1.cc"
%require "3.8"

%defines
%define api.namespace {yy}
%define api.parser.class {parser}
%define api.value.type variant
%define api.token.constructor
%define parse.error detailed
%define parse.lac full
%locations

%code requires {
    #include "ast.hpp"
    #include "semantic.hpp"
    #include <memory>
    #include <string>
    #include <vector>
}

%code {
    static compiler::SourceLocation toSourceLocation(
        const yy::location& loc
    ) {
        return {
            static_cast<int>(loc.begin.line),
            static_cast<int>(loc.begin.column)
        };
    }

    yy::parser::symbol_type yylex();
}

%parse-param { compiler::Program& program }
%parse-param { std::vector<compiler::Diagnostic>& diagnostics }

%token INT "int"
%token PRINT "print"
%token IF "if"
%token ELSE "else"
%token WHILE "while"
%token FOR "for"

%token <std::string> IDENTIFIER "identifier"
%token <long> NUMBER "integer"

%token PLUS "+"
%token MINUS "-"
%token STAR "*"
%token SLASH "/"
%token ASSIGN "="
%token EQUAL "=="
%token NOT_EQUAL "!="
%token LESS "<"
%token GREATER ">"
%token LESS_EQUAL "<="
%token GREATER_EQUAL ">="
%token AMPERSAND "&"
%token INCREMENT "++"
%token DECREMENT "--"

%token LPAREN "("
%token RPAREN ")"
%token LBRACE "{"
%token RBRACE "}"
%token SEMICOLON ";"

%token INVALID "invalid token"

%type <compiler::ExprPtr> expression equality relational
%type <compiler::ExprPtr> additive term unary factor
%type <compiler::StmtPtr> statement declaration assignment
%type <compiler::StmtPtr> print_statement if_statement
%type <compiler::StmtPtr> while_statement for_statement
%type <compiler::StmtPtr> for_initializer for_increment
%type <std::vector<compiler::StmtPtr>> block statements

%left EQUAL NOT_EQUAL
%left LESS GREATER LESS_EQUAL GREATER_EQUAL
%left PLUS MINUS
%left STAR SLASH
%right UMINUS UADDRESS UDEREFERENCE

%start input

%%

input:
      %empty
    | input statement
      {
          if ($2) {
              program.statements.push_back(std::move($2));
          }
      }
    | input error SEMICOLON
      {
          diagnostics.push_back({
              "syntax",
              static_cast<int>(@2.begin.line),
              static_cast<int>(@2.begin.column),
              "Invalid statement; skipped to the next semicolon."
          });
          yyerrok;
      }
    ;

statement:
      declaration
      { $$ = std::move($1); }
    | assignment
      { $$ = std::move($1); }
    | print_statement
      { $$ = std::move($1); }
    | if_statement
      { $$ = std::move($1); }
    | while_statement
      { $$ = std::move($1); }
    | for_statement
      { $$ = std::move($1); }
    ;

declaration:
      INT IDENTIFIER ASSIGN expression SEMICOLON
      {
          $$ = std::make_unique<compiler::DeclarationStmt>(
              std::move($2), std::move($4),
              toSourceLocation(@1), false
          );
      }
    | INT STAR IDENTIFIER ASSIGN expression SEMICOLON
      {
          $$ = std::make_unique<compiler::DeclarationStmt>(
              std::move($3), std::move($5),
              toSourceLocation(@1), true
          );
      }
    | INT IDENTIFIER SEMICOLON
      {
          $$ = std::make_unique<compiler::DeclarationStmt>(
              std::move($2), nullptr,
              toSourceLocation(@1), false
          );
      }
    | INT STAR IDENTIFIER SEMICOLON
      {
          $$ = std::make_unique<compiler::DeclarationStmt>(
              std::move($3), nullptr,
              toSourceLocation(@1), true
          );
      }
    ;

assignment:
      IDENTIFIER ASSIGN expression SEMICOLON
      {
          $$ = std::make_unique<compiler::AssignmentStmt>(
              std::move($1), std::move($3),
              toSourceLocation(@1)
          );
      }
    | IDENTIFIER INCREMENT SEMICOLON
      {
          auto oldValue = std::make_unique<compiler::IdentifierExpr>(
              $1, toSourceLocation(@1)
          );
          auto one = std::make_unique<compiler::NumberExpr>(
              1, toSourceLocation(@2)
          );
          auto sum = std::make_unique<compiler::BinaryExpr>(
              "+", std::move(oldValue), std::move(one),
              toSourceLocation(@2)
          );
          $$ = std::make_unique<compiler::AssignmentStmt>(
              std::move($1), std::move(sum),
              toSourceLocation(@1)
          );
      }
    | IDENTIFIER DECREMENT SEMICOLON
      {
          auto oldValue = std::make_unique<compiler::IdentifierExpr>(
              $1, toSourceLocation(@1)
          );
          auto one = std::make_unique<compiler::NumberExpr>(
              1, toSourceLocation(@2)
          );
          auto difference = std::make_unique<compiler::BinaryExpr>(
              "-", std::move(oldValue), std::move(one),
              toSourceLocation(@2)
          );
          $$ = std::make_unique<compiler::AssignmentStmt>(
              std::move($1), std::move(difference),
              toSourceLocation(@1)
          );
      }
    ;

print_statement:
      PRINT LPAREN expression RPAREN SEMICOLON
      {
          $$ = std::make_unique<compiler::PrintStmt>(
              std::move($3), toSourceLocation(@1)
          );
      }
    ;

if_statement:
      IF LPAREN expression RPAREN block
      {
          $$ = std::make_unique<compiler::IfStmt>(
              std::move($3), std::move($5),
              std::vector<compiler::StmtPtr>{},
              toSourceLocation(@1)
          );
      }
    | IF LPAREN expression RPAREN block ELSE block
      {
          $$ = std::make_unique<compiler::IfStmt>(
              std::move($3), std::move($5),
              std::move($7), toSourceLocation(@1)
          );
      }
    ;

while_statement:
      WHILE LPAREN expression RPAREN block
      {
          $$ = std::make_unique<compiler::WhileStmt>(
              std::move($3), std::move($5),
              toSourceLocation(@1)
          );
      }
    ;

for_statement:
      FOR LPAREN for_initializer expression SEMICOLON for_increment RPAREN block
      {
          $$ = std::make_unique<compiler::ForStmt>(
              std::move($3), std::move($4), std::move($6),
              std::move($8), toSourceLocation(@1)
          );
      }
    ;

for_initializer:
      INT IDENTIFIER ASSIGN expression SEMICOLON
      {
          $$ = std::make_unique<compiler::DeclarationStmt>(
              std::move($2), std::move($4),
              toSourceLocation(@1), false
          );
      }
    | IDENTIFIER ASSIGN expression SEMICOLON
      {
          $$ = std::make_unique<compiler::AssignmentStmt>(
              std::move($1), std::move($3), toSourceLocation(@1)
          );
      }
    | SEMICOLON
      {
          $$ = nullptr;
      }
    ;

for_increment:
      IDENTIFIER INCREMENT
      {
          auto oldValue = std::make_unique<compiler::IdentifierExpr>(
              $1, toSourceLocation(@1)
          );
          auto one = std::make_unique<compiler::NumberExpr>(
              1, toSourceLocation(@2)
          );
          auto sum = std::make_unique<compiler::BinaryExpr>(
              "+", std::move(oldValue), std::move(one),
              toSourceLocation(@2)
          );
          $$ = std::make_unique<compiler::AssignmentStmt>(
              std::move($1), std::move(sum), toSourceLocation(@1)
          );
      }
    | IDENTIFIER DECREMENT
      {
          auto oldValue = std::make_unique<compiler::IdentifierExpr>(
              $1, toSourceLocation(@1)
          );
          auto one = std::make_unique<compiler::NumberExpr>(
              1, toSourceLocation(@2)
          );
          auto difference = std::make_unique<compiler::BinaryExpr>(
              "-", std::move(oldValue), std::move(one),
              toSourceLocation(@2)
          );
          $$ = std::make_unique<compiler::AssignmentStmt>(
              std::move($1), std::move(difference),
              toSourceLocation(@1)
          );
      }
    | IDENTIFIER ASSIGN expression
      {
          $$ = std::make_unique<compiler::AssignmentStmt>(
              std::move($1), std::move($3), toSourceLocation(@1)
          );
      }
    | %empty
      {
          $$ = nullptr;
      }
    ;

block:
      LBRACE statements RBRACE
      {
          $$ = std::move($2);
      }
    ;

statements:
      %empty
      {
          $$ = std::vector<compiler::StmtPtr>{};
      }
    | statements statement
      {
          $$ = std::move($1);
          if ($2) {
              $$.push_back(std::move($2));
          }
      }
    ;

expression:
      equality
      { $$ = std::move($1); }
    ;

equality:
      relational
      { $$ = std::move($1); }
    | equality EQUAL relational
      {
          $$ = std::make_unique<compiler::BinaryExpr>(
              "==", std::move($1), std::move($3),
              toSourceLocation(@2)
          );
      }
    | equality NOT_EQUAL relational
      {
          $$ = std::make_unique<compiler::BinaryExpr>(
              "!=", std::move($1), std::move($3),
              toSourceLocation(@2)
          );
      }
    ;

relational:
      additive
      { $$ = std::move($1); }
    | relational LESS additive
      {
          $$ = std::make_unique<compiler::BinaryExpr>(
              "<", std::move($1), std::move($3),
              toSourceLocation(@2)
          );
      }
    | relational GREATER additive
      {
          $$ = std::make_unique<compiler::BinaryExpr>(
              ">", std::move($1), std::move($3),
              toSourceLocation(@2)
          );
      }
    | relational LESS_EQUAL additive
      {
          $$ = std::make_unique<compiler::BinaryExpr>(
              "<=", std::move($1), std::move($3),
              toSourceLocation(@2)
          );
      }
    | relational GREATER_EQUAL additive
      {
          $$ = std::make_unique<compiler::BinaryExpr>(
              ">=", std::move($1), std::move($3),
              toSourceLocation(@2)
          );
      }
    ;

additive:
      additive PLUS term
      {
          $$ = std::make_unique<compiler::BinaryExpr>(
              "+", std::move($1), std::move($3),
              toSourceLocation(@2)
          );
      }
    | additive MINUS term
      {
          $$ = std::make_unique<compiler::BinaryExpr>(
              "-", std::move($1), std::move($3),
              toSourceLocation(@2)
          );
      }
    | term
      { $$ = std::move($1); }
    ;

term:
      term STAR unary
      {
          $$ = std::make_unique<compiler::BinaryExpr>(
              "*", std::move($1), std::move($3),
              toSourceLocation(@2)
          );
      }
    | term SLASH unary
      {
          $$ = std::make_unique<compiler::BinaryExpr>(
              "/", std::move($1), std::move($3),
              toSourceLocation(@2)
          );
      }
    | unary
      { $$ = std::move($1); }
    ;

unary:
      MINUS unary %prec UMINUS
      {
          $$ = std::make_unique<compiler::UnaryExpr>(
              "-", std::move($2), toSourceLocation(@1)
          );
      }
    | AMPERSAND unary %prec UADDRESS
      {
          $$ = std::make_unique<compiler::UnaryExpr>(
              "&", std::move($2), toSourceLocation(@1)
          );
      }
    | STAR unary %prec UDEREFERENCE
      {
          $$ = std::make_unique<compiler::UnaryExpr>(
              "*", std::move($2), toSourceLocation(@1)
          );
      }
    | factor
      { $$ = std::move($1); }
    ;

factor:
      NUMBER
      {
          $$ = std::make_unique<compiler::NumberExpr>(
              $1, toSourceLocation(@1)
          );
      }
    | IDENTIFIER
      {
          $$ = std::make_unique<compiler::IdentifierExpr>(
              std::move($1), toSourceLocation(@1)
          );
      }
    | LPAREN expression RPAREN
      {
          $$ = std::move($2);
      }
    ;

%%

void yy::parser::error(
    const location_type& loc,
    const std::string& message
) {
    diagnostics.push_back({
        "syntax",
        static_cast<int>(loc.begin.line),
        static_cast<int>(loc.begin.column),
        message
    });
}

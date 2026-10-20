
%option noyywrap nounput noinput

%{
#include "parser.hpp"

#include <cerrno>
#include <cstdlib>
#include <string>
#include <vector>

#define YY_DECL yy::parser::symbol_type yylex()

struct TokenInfo {
    std::string type;
    std::string value;
    int line;
    int column;
};

extern std::vector<TokenInfo> collectedTokens;

static int currentLine = 1;
static int currentColumn = 1;

static yy::location makeLocation(int length) {
    yy::location result;
    result.begin.line = currentLine;
    result.begin.column = currentColumn;

    for (int i = 0; i < length; ++i) {
        if (yytext[i] == '\n') {
            ++currentLine;
            currentColumn = 1;
        } else {
            ++currentColumn;
        }
    }

    result.end.line = currentLine;
    result.end.column = currentColumn;
    return result;
}

static void recordToken(
    const std::string& type,
    const std::string& value,
    int line,
    int column
) {
    collectedTokens.push_back({type, value, line, column});
}
%}

DIGIT       [0-9]
LETTER      [a-zA-Z_]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT})*
INTEGER     {DIGIT}+

%%

"int" {
    int line = currentLine, column = currentColumn;
    auto tokenLoc = makeLocation(yyleng);
    recordToken("INT", yytext, line, column);
    return yy::parser::make_INT(tokenLoc);
}

"print" {
    int line = currentLine, column = currentColumn;
    auto tokenLoc = makeLocation(yyleng);
    recordToken("PRINT", yytext, line, column);
    return yy::parser::make_PRINT(tokenLoc);
}

{IDENTIFIER} {
    int line = currentLine, column = currentColumn;
    auto tokenLoc = makeLocation(yyleng);
    recordToken("IDENTIFIER", yytext, line, column);
    return yy::parser::make_IDENTIFIER(
        std::string(yytext, yyleng), tokenLoc
    );
}

{INTEGER} {
    int line = currentLine, column = currentColumn;
    auto tokenLoc = makeLocation(yyleng);

    errno = 0;
    char* end = nullptr;
    long value = std::strtol(yytext, &end, 10);

    if (errno == ERANGE || end != yytext + yyleng) {
        recordToken("INVALID", yytext, line, column);
        return yy::parser::make_INVALID(tokenLoc);
    }

    recordToken("NUMBER", yytext, line, column);
    return yy::parser::make_NUMBER(value, tokenLoc);
}

"+" {
    int line = currentLine, column = currentColumn;
    auto tokenLoc = makeLocation(yyleng);
    recordToken("PLUS", yytext, line, column);
    return yy::parser::make_PLUS(tokenLoc);
}

"-" {
    int line = currentLine, column = currentColumn;
    auto tokenLoc = makeLocation(yyleng);
    recordToken("MINUS", yytext, line, column);
    return yy::parser::make_MINUS(tokenLoc);
}

"*" {
    int line = currentLine, column = currentColumn;
    auto tokenLoc = makeLocation(yyleng);
    recordToken("STAR", yytext, line, column);
    return yy::parser::make_STAR(tokenLoc);
}

"/" {
    int line = currentLine, column = currentColumn;
    auto tokenLoc = makeLocation(yyleng);
    recordToken("SLASH", yytext, line, column);
    return yy::parser::make_SLASH(tokenLoc);
}

"=" {
    int line = currentLine, column = currentColumn;
    auto tokenLoc = makeLocation(yyleng);
    recordToken("ASSIGN", yytext, line, column);
    return yy::parser::make_ASSIGN(tokenLoc);
}

"(" {
    int line = currentLine, column = currentColumn;
    auto tokenLoc = makeLocation(yyleng);
    recordToken("LPAREN", yytext, line, column);
    return yy::parser::make_LPAREN(tokenLoc);
}

")" {
    int line = currentLine, column = currentColumn;
    auto tokenLoc = makeLocation(yyleng);
    recordToken("RPAREN", yytext, line, column);
    return yy::parser::make_RPAREN(tokenLoc);
}

";" {
    int line = currentLine, column = currentColumn;
    auto tokenLoc = makeLocation(yyleng);
    recordToken("SEMICOLON", yytext, line, column);
    return yy::parser::make_SEMICOLON(tokenLoc);
}

[ \t\r]+ {
    makeLocation(yyleng);
}

\n {
    makeLocation(yyleng);
}

. {
    int line = currentLine, column = currentColumn;
    auto tokenLoc = makeLocation(yyleng);
    recordToken("INVALID", yytext, line, column);
    return yy::parser::make_INVALID(tokenLoc);
}

<<EOF>> {
    yy::location endLoc;
    endLoc.begin.line = currentLine;
    endLoc.begin.column = currentColumn;
    endLoc.end = endLoc.begin;
    return yy::parser::make_YYEOF(endLoc);
}

%%

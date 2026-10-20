
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
    yy::location loc;
    loc.begin.line = currentLine;
    loc.begin.column = currentColumn;

    for (int i = 0; i < length; ++i) {
        if (yytext[i] == '\n') {
            ++currentLine;
            currentColumn = 1;
        } else {
            ++currentColumn;
        }
    }

    loc.end.line = currentLine;
    loc.end.column = currentColumn;
    return loc;
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
    auto loc = makeLocation(yyleng);
    recordToken("INT", yytext, line, column);
    return yy::parser::make_INT(loc);
}

"print" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("PRINT", yytext, line, column);
    return yy::parser::make_PRINT(loc);
}

"if" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("IF", yytext, line, column);
    return yy::parser::make_IF(loc);
}

"else" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("ELSE", yytext, line, column);
    return yy::parser::make_ELSE(loc);
}

"while" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("WHILE", yytext, line, column);
    return yy::parser::make_WHILE(loc);
}

"for" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("FOR", yytext, line, column);
    return yy::parser::make_FOR(loc);
}

    /* Longest operators first. */
"==" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("EQUAL", yytext, line, column);
    return yy::parser::make_EQUAL(loc);
}

"!=" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("NOT_EQUAL", yytext, line, column);
    return yy::parser::make_NOT_EQUAL(loc);
}

"<=" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("LESS_EQUAL", yytext, line, column);
    return yy::parser::make_LESS_EQUAL(loc);
}

">=" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("GREATER_EQUAL", yytext, line, column);
    return yy::parser::make_GREATER_EQUAL(loc);
}

"++" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("INCREMENT", yytext, line, column);
    return yy::parser::make_INCREMENT(loc);
}

"--" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("DECREMENT", yytext, line, column);
    return yy::parser::make_DECREMENT(loc);
}

{IDENTIFIER} {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("IDENTIFIER", yytext, line, column);
    return yy::parser::make_IDENTIFIER(
        std::string(yytext, yyleng), loc
    );
}

{INTEGER} {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);

    errno = 0;
    char* end = nullptr;
    long value = std::strtol(yytext, &end, 10);

    if (errno == ERANGE || end != yytext + yyleng) {
        recordToken("INVALID", yytext, line, column);
        return yy::parser::make_INVALID(loc);
    }

    recordToken("NUMBER", yytext, line, column);
    return yy::parser::make_NUMBER(value, loc);
}

"+" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("PLUS", yytext, line, column);
    return yy::parser::make_PLUS(loc);
}

"-" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("MINUS", yytext, line, column);
    return yy::parser::make_MINUS(loc);
}

"*" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("STAR", yytext, line, column);
    return yy::parser::make_STAR(loc);
}

"/" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("SLASH", yytext, line, column);
    return yy::parser::make_SLASH(loc);
}

"=" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("ASSIGN", yytext, line, column);
    return yy::parser::make_ASSIGN(loc);
}

"<" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("LESS", yytext, line, column);
    return yy::parser::make_LESS(loc);
}

">" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("GREATER", yytext, line, column);
    return yy::parser::make_GREATER(loc);
}

"&" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("AMPERSAND", yytext, line, column);
    return yy::parser::make_AMPERSAND(loc);
}

"{" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("LBRACE", yytext, line, column);
    return yy::parser::make_LBRACE(loc);
}

"}" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("RBRACE", yytext, line, column);
    return yy::parser::make_RBRACE(loc);
}

"(" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("LPAREN", yytext, line, column);
    return yy::parser::make_LPAREN(loc);
}

")" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("RPAREN", yytext, line, column);
    return yy::parser::make_RPAREN(loc);
}

";" {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("SEMICOLON", yytext, line, column);
    return yy::parser::make_SEMICOLON(loc);
}

[ \t\r]+ {
    makeLocation(yyleng);
}

\n {
    makeLocation(yyleng);
}

. {
    int line = currentLine, column = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("INVALID", yytext, line, column);
    return yy::parser::make_INVALID(loc);
}

<<EOF>> {
    yy::location loc;
    loc.begin.line = currentLine;
    loc.begin.column = currentColumn;
    loc.end = loc.begin;
    return yy::parser::make_YYEOF(loc);
}

%%


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

static void recordToken(const std::string& type,
                        const std::string& value,
                        int line, int column) {
    collectedTokens.push_back({type, value, line, column});
}
%}

DIGIT       [0-9]
LETTER      [a-zA-Z_]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT})*
INTEGER     {DIGIT}+
FLOATING    {DIGIT}+"."{DIGIT}+
STRINGLIT   \"([^\\\"\n]|\\.)*\"
CHARLIT     \'([^\\\'\n]|\\.)\'

%%

"int" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("INT", yytext, l, c);
    return yy::parser::make_INT(loc);
}

"float" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("FLOAT", yytext, l, c);
    return yy::parser::make_FLOAT(loc);
}

"char" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("CHAR", yytext, l, c);
    return yy::parser::make_CHAR(loc);
}

"print" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("PRINT", yytext, l, c);
    return yy::parser::make_PRINT(loc);
}

"if" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("IF", yytext, l, c);
    return yy::parser::make_IF(loc);
}

"else" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("ELSE", yytext, l, c);
    return yy::parser::make_ELSE(loc);
}

"while" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("WHILE", yytext, l, c);
    return yy::parser::make_WHILE(loc);
}

"for" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("FOR", yytext, l, c);
    return yy::parser::make_FOR(loc);
}

"==" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("EQUAL", yytext, l, c);
    return yy::parser::make_EQUAL(loc);
}

"!=" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("NOT_EQUAL", yytext, l, c);
    return yy::parser::make_NOT_EQUAL(loc);
}

"<=" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("LESS_EQUAL", yytext, l, c);
    return yy::parser::make_LESS_EQUAL(loc);
}

">=" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("GREATER_EQUAL", yytext, l, c);
    return yy::parser::make_GREATER_EQUAL(loc);
}

"++" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("INCREMENT", yytext, l, c);
    return yy::parser::make_INCREMENT(loc);
}

"--" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("DECREMENT", yytext, l, c);
    return yy::parser::make_DECREMENT(loc);
}

{STRINGLIT} {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    std::string value(yytext + 1, yyleng - 2);
    recordToken("STRING", value, l, c);
    return yy::parser::make_STRING_LITERAL(value, loc);
}

{CHARLIT} {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    std::string value(yytext + 1, yyleng - 2);
    recordToken("CHARACTER", value, l, c);
    return yy::parser::make_CHARACTER_LITERAL(value, loc);
}

{FLOATING} {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    errno = 0;
    char* end = nullptr;
    double value = std::strtod(yytext, &end);

    if (errno == ERANGE || end != yytext + yyleng) {
        recordToken("INVALID", yytext, l, c);
        return yy::parser::make_INVALID(loc);
    }

    recordToken("FLOAT_NUMBER", yytext, l, c);
    return yy::parser::make_FLOAT_NUMBER(value, loc);
}

{IDENTIFIER} {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("IDENTIFIER", yytext, l, c);
    return yy::parser::make_IDENTIFIER(std::string(yytext, yyleng), loc);
}

{INTEGER} {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    errno = 0;
    char* end = nullptr;
    long value = std::strtol(yytext, &end, 10);

    if (errno == ERANGE || end != yytext + yyleng) {
        recordToken("INVALID", yytext, l, c);
        return yy::parser::make_INVALID(loc);
    }

    recordToken("NUMBER", yytext, l, c);
    return yy::parser::make_NUMBER(value, loc);
}

"+" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("PLUS", yytext, l, c);
    return yy::parser::make_PLUS(loc);
}

"-" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("MINUS", yytext, l, c);
    return yy::parser::make_MINUS(loc);
}

"*" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("STAR", yytext, l, c);
    return yy::parser::make_STAR(loc);
}

"/" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("SLASH", yytext, l, c);
    return yy::parser::make_SLASH(loc);
}

"=" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("ASSIGN", yytext, l, c);
    return yy::parser::make_ASSIGN(loc);
}

"<" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("LESS", yytext, l, c);
    return yy::parser::make_LESS(loc);
}

">" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("GREATER", yytext, l, c);
    return yy::parser::make_GREATER(loc);
}

"&" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("AMPERSAND", yytext, l, c);
    return yy::parser::make_AMPERSAND(loc);
}

"{" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("LBRACE", yytext, l, c);
    return yy::parser::make_LBRACE(loc);
}

"}" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("RBRACE", yytext, l, c);
    return yy::parser::make_RBRACE(loc);
}

"(" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("LPAREN", yytext, l, c);
    return yy::parser::make_LPAREN(loc);
}

")" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("RPAREN", yytext, l, c);
    return yy::parser::make_RPAREN(loc);
}

";" {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("SEMICOLON", yytext, l, c);
    return yy::parser::make_SEMICOLON(loc);
}

[ \t\r]+ {
    makeLocation(yyleng);
}

\n {
    makeLocation(yyleng);
}

. {
    int l = currentLine, c = currentColumn;
    auto loc = makeLocation(yyleng);
    recordToken("INVALID", yytext, l, c);
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

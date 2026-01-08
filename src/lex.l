%option noyywrap
%option nounput noinput

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"

// 自己维护行列
int lineno = 1;
int column = 1;

#define YY_DECL struct token lex_tok()

struct token make_token(enum tok type) {
    struct token t;
    t.range.start.line = lineno;
    t.range.start.pos = column;
    t.range.end.line = lineno;
    t.range.end.pos = column + yyleng - 1;
    t.type = type;

    if (type == tok_id) {
        t.name = strndup(yytext, yyleng);
    } else if (type == tok_num) {
        char buf[64];
        strncpy(buf, yytext, yyleng);
        buf[yyleng] = '\0';
        t.num = atoi(buf);
    } else if (type == tok_unknown) {
        asprintf(&t.lex_error, "非法单词：%.*s", yyleng, yytext);
    }

    column += yyleng;
    return t;
}

%}

%%

"//".*       { column += yyleng; }

"/*"([^*]|\*+[^*/])*\*+"/"  {
    for (int i = 0; i < yyleng; ++i) {
        if (yytext[i] == '\n') {
            lineno++;
            column = 1;
        } else {
            column++;
        }
    }
}

\n              { lineno++; column = 1; }
[ \t]+          { column += yyleng; }

(?i:"if")       { return make_token(tok_kw_if); }
(?i:"else")     { return make_token(tok_kw_else); }
(?i:"for")      { return make_token(tok_kw_for); }
(?i:"do")       { return make_token(tok_kw_do); }
(?i:"while")    { return make_token(tok_kw_while); }
(?i:"int")      { return make_token(tok_kw_int); }
(?i:"void")     { return make_token(tok_kw_void); }
(?i:"write")    { return make_token(tok_kw_write); }
(?i:"read")     { return make_token(tok_kw_read); }
(?i:"switch")   { return make_token(tok_kw_switch); }
(?i:"case")     { return make_token(tok_kw_case); }
(?i:"break")    { return make_token(tok_kw_break); }
(?i:"continue") { return make_token(tok_kw_continue); }
(?i:"default")  { return make_token(tok_kw_default); }
(?i:"return")   { return make_token(tok_kw_return); }

"<="            { return make_token(tok_lessequal); }
"<"             { return make_token(tok_less); }
">="            { return make_token(tok_greaterequal); }
">"             { return make_token(tok_greater); }
"=="            { return make_token(tok_equalequal); }
"!="            { return make_token(tok_exclaimequal); }

"!"             { return make_token(tok_exclaim); }
"&&"            { return make_token(tok_ampamp); }
"||"            { return make_token(tok_pipepipe); }

"+"             { return make_token(tok_plus); }
"-"             { return make_token(tok_minus); }
"*"             { return make_token(tok_star); }
"/"             { return make_token(tok_slash); }

"&"             { return make_token(tok_amp); }
"|"             { return make_token(tok_pipe); }

","             { return make_token(tok_comma); }

"{"             { return make_token(tok_l_brace); }
"}"             { return make_token(tok_r_brace); }

"("             { return make_token(tok_l_paren); }
")"             { return make_token(tok_r_paren); }

"["             { return make_token(tok_l_square); }
"]"             { return make_token(tok_r_square); }

":"             { return make_token(tok_colon); }

";"             { return make_token(tok_semi); }

"="             { return make_token(tok_equal); }

[a-zA-Z_][a-zA-Z0-9_]* { return make_token(tok_id); }

[0-9]+          { return make_token(tok_num); }

.               { return make_token(tok_unknown); }

<<EOF>>         { struct token t = {0}; t.type = tok_eof; return t; }
%%



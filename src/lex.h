#pragma once
#include "source_position.h"

enum tok {
    /// 未知 token
    tok_unknown,

    /// 保留字
    tok_kw_if,
    tok_kw_else,
    tok_kw_for,
    tok_kw_do,
    tok_kw_while,
    tok_kw_int,
    tok_kw_void,
    tok_kw_write,
    tok_kw_read,
    tok_kw_switch,
    tok_kw_case,
    tok_kw_break,
    tok_kw_continue,
    tok_kw_default,
    tok_kw_return,

    /// 比较、逻辑运算符
    tok_less,         // <
    tok_lessequal,    // <=
    tok_greater,      // >
    tok_greaterequal, // >=
    tok_equalequal,   // ==
    tok_exclaimequal, // !=

    tok_exclaim,  // !
    tok_ampamp,   // &&
    tok_pipepipe, // ||

    /// 算数运算符
    tok_plus,  // +
    tok_minus, // -
    tok_star,  // *
    tok_slash, // /

    /// 位运算符
    tok_amp,  // &
    tok_pipe, // |

    /// 逗号运算符
    tok_comma, // ,

    /// 大括号
    tok_l_brace, // {
    tok_r_brace, // }

    /// 圆括号
    tok_l_paren, // (
    tok_r_paren, // )

    /// 中括号
    tok_l_square, // [
    tok_r_square, // ]

    /// 冒号
    tok_colon, // :

    /// 分号
    tok_semi, // ;

    /// 赋值运算符
    tok_equal, // =

    /// 标识符
    tok_id,

    /// 无符号整数
    tok_num,

    /// EOF
    tok_eof,
};

struct token {
    struct source_range range;
    enum tok type;

    union {
        char *name;
        int num;
        char *lex_error;
    };
};


struct token lex_tok();
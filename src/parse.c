#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ast.h"
#include "lex.h"
#include "source_position.h"
#include "util.h"

// #define IN printf("enter %s\n", __FUNCTION__);
// #define OUT printf("leave %s\n", __FUNCTION__);

#define IN
#define OUT

// match 实现
const char *tok_to_str(enum tok t);



struct token tk, lh, lh2, lh3;

void print_tok(const struct token *t) {

    switch (t->type) {
    case tok_id:
        printf("ID\t%s\t\n", t->name);
        break;
    case tok_num:
        printf("NUM\t%d\t\n", t->num);
        break;
    case tok_unknown:
        printf("UNKNOWN\t%s\t\n", t->lex_error);
        break;
    case tok_eof:
        break;
    default:
        printf("%s\t%s\t\n",tok_to_str(t->type),tok_to_str(t->type));
    }
    // printf("\n");
}

struct token get_tok() {
    struct token t = lex_tok();
    print_tok(&t);
    if (t.type == tok_unknown) {
        error(t.range, "词法错误：%s", t.lex_error);
    }
    return t;
}


void tok_init() {
    lh = get_tok();
    lh2 = get_tok();
    lh3 = get_tok();
}

void next_tok() {
    tk = lh;
    lh = lh2;
    lh2 = lh3;
    lh3 = get_tok();
}

void match(enum tok expected) {
    if (lh.type == expected) {
        next_tok();
    } else {
        struct source_range range;
        range.start.line = tk.range.end.line;
        range.start.pos = tk.range.end.pos + 1;
        error(range, "缺少 “%s”。", tok_to_str(expected));
        // next_tok();
    }
}

const char *tok_to_str(enum tok t) {
    switch (t) {
    case tok_kw_if:
        return "if";
    case tok_kw_else:
        return "else";
    case tok_kw_switch:
        return "switch";
    case tok_kw_break:
        return "break";
    case tok_kw_continue:
        return "continue";
    case tok_kw_default:
        return "default";
    case tok_ampamp:
        return "&&";
    case tok_colon:
        return ":";
    case tok_exclaim:
        return "!";
    case tok_pipepipe:
        return "||";
    case tok_pipe:
        return "|";
    case tok_amp:
        return "&";
    case tok_kw_for:
        return "for";
    case tok_kw_do:
        return "do";
    case tok_kw_while:
        return "while";
    case tok_kw_int:
        return "int";
    case tok_kw_write:
        return "write";
    case tok_kw_read:
        return "read";
    case tok_kw_case:
        return "case";
    case tok_l_square:
        return "[";
    case tok_r_square:
        return "]";
    case tok_less:
        return "<";
    case tok_lessequal:
        return "<=";
    case tok_greater:
        return ">";
    case tok_greaterequal:
        return ">=";
    case tok_equalequal:
        return "==";
    case tok_exclaimequal:
        return "!=";
    case tok_plus:
        return "+";
    case tok_minus:
        return "-";
    case tok_star:
        return "*";
    case tok_slash:
        return "/";
    case tok_equal:
        return "=";
    case tok_l_brace:
        return "{";
    case tok_r_brace:
        return "}";
    case tok_l_paren:
        return "(";
    case tok_r_paren:
        return ")";
    case tok_comma:
        return ",";
    case tok_semi:
        return ";";
    case tok_id:
        return "identifier";
    case tok_num:
        return "number";
    case tok_eof:
        return "EOF";
    case tok_kw_void:
        return "void";
    case tok_kw_return:
        return "return";
    case tok_unknown:
        return "unknown";
    default:
        return "unknown_tok";
    }
}

// forward declarations
struct program *parse_program();

void parse_declarations(struct vector *decls);
void parse_var_decl(struct vector *decls);
struct function_decl *parse_function();
struct var_decl *parse_declarator(struct source_range range);
struct param_decl *parse_param();

struct statement *parse_statement();

struct if_stmt *parse_if_stmt();
struct switch_stmt *parse_switch_stmt();
struct case_clause_stmt *parse_case_stmt();

struct while_stmt *parse_while_stmt();
struct do_while_stmt *parse_do_while_stmt();
struct for_stmt *parse_for_stmt();

struct read_stmt *parse_read_stmt();
struct write_stmt *parse_write_stmt();

struct return_stmt *parse_return_stmt();
struct compound_stmt *parse_compound_stmt();
struct expr_stmt *parse_expr_stmt();

struct expr *parse_expr();

struct expr *parse_bool_expr();
struct expr *parse_additive_expr();
struct expr *parse_term();
struct expr *parse_factor();
struct subscript_expr *parse_subscript_expr();
struct declref *parse_declref();
struct literal *parse_lit();

#define new(type, var) type *var = (type *)malloc(sizeof(type))

struct trie *current_frame;

// (1) <program> ::={fun_declaration }<main_declaration>
struct program *parse_program() {
    IN;
    tok_init();
    error_buf = 0;

    new (struct program, prog);
    vec_init(&prog->decls);
    trie_init(&prog->frame);
    current_frame = &prog->frame;

    while (lh.type != tok_eof) {
        parse_declarations(&prog->decls);
    }

    // printf("PROGRAM: \n");
    // trie_print(current_frame);

    int size = prog->decls.size;
    if (!size) {
        error(prog->range, "空程序");
    }

    struct declaration *decl = (struct declaration *)prog->decls.data[size - 1];
    if (decl->kind == decl_function) {
        struct function_decl *main = (struct function_decl *)decl;
        if (strcmp(main->name, "main") != 0) {
            error(decl->range, "程序中最后的函数定义名字必须为main。");
        }
    } else {
        error(decl->range, "程序中最后的声明必须是一个函数定义，名字为main。");
    }

    OUT;
    return prog;
}

#define BASE(x) &(x->base)
#define move(x) x

void parse_declarations(struct vector *decls) {
    if (lh.type == tok_kw_int || lh.type == tok_kw_void || lh.type == tok_id) {
        if (lh.type == tok_id || lh.type == tok_kw_void) {
            vec_pb(decls, (ll)BASE(parse_function()));
        } else if (lh2.type == tok_id) {
            if (lh3.type == tok_l_paren) {
                vec_pb(decls, (ll)BASE(parse_function()));
            } else {
                parse_var_decl(decls);
            }
        } else {
            error(lh2.range, "expect ID");
        }
    } else {
        error(lh.range, "未知的符号 %s 出现在顶层声明", tok_to_str(lh.type));
        match(lh.type);
    }
}

// int ID
struct param_decl *parse_param() {
    IN;

    new (struct param_decl, param);
    param->base.kind = decl_param;
    param->base.range.start = lh.range.start;

    new (struct type, ty);
    ty->range = lh.range;

    // int param[]
    // ^^^
    match(tok_kw_int);
    ty->kind = ty_int;
    param->ty = ty;

    // int param[]
    //     ^^^^^
    match(tok_id);
    param->name = move(tk.name);

    {
        struct declaration *existed_decl = (struct declaration *)
            trie_add(current_frame, param->name, (ll)param);

        if (existed_decl != (struct declaration *)-1) {
            // trie_print(current_frame);
            error(tk.range, "命名冲突，%s已经定义于位置 第%d行，第%d列。",
                  param->name, existed_decl->range.start,
                  existed_decl->range.end);
            return NULL;
        }
    }

    // int param[]
    //          ^^
    if (lh.type == tok_l_square) {
        match(tok_l_square);
        param->ty->kind = ty_array;
        if (lh.type == tok_num) {
            match(tok_num);
            param->ty->array_type.length = tk.num;
        }
        match(tok_r_square);
    }

    OUT;

    param->base.range.end = tk.range.end;
    return param;
}

// (2) <fun_declaration>::= <type> ID ‘(’ ‘)’ <function_body>
struct function_decl *parse_function() {
    IN;

    new (struct function_decl, func);
    func->base.range.start = lh.range.start;
    func->base.kind = decl_function;
    vec_init(&func->params);
    vec_init(&func->decls);
    vec_init(&func->stmts);
    trie_init(&func->frame);
    func->frame.parent = current_frame;

    // int func() { ... }
    // ^^^
    {
        new (struct type, ty);
        ty->range.start = ty->range.end = lh.range.start;
        ty->kind = ty_void;

        // int func() { ... }
        // ^^^
        if (lh.type == tok_kw_int) {
            match(tok_kw_int);
            ty->range = tk.range;
            ty->kind = ty_int;
        }

        // void func() { ... }
        // ^^^^
        else if (lh.type == tok_kw_void) {
            match(tok_kw_void);
            ty->range = tk.range;
            ty->kind = ty_void;
        }

        func->ret_ty = ty;
    }

    // func() { ... }
    // ^^^^
    match(tok_id);
    func->name = move(tk.name);

    {
        struct declaration *decl = (struct declaration *)
            trie_add(current_frame, func->name, (ll)func);

        if (decl != (struct declaration *)-1) {
            error(tk.range, "命名冲突，%s已经定义于位置 第%d行，第%d列。",
                  func->name, decl->range.start.line, decl->range.start.pos);
            return NULL;
        }
    }

    // 进入作用域
    struct trie *old_frame = current_frame;
    current_frame = &func->frame;

    // func(int a, int b[]) { ... }
    //     ^^^^^^^^^^^^^^^^
    {
        match(tok_l_paren);

        while (lh.type != tok_r_paren) {
            vec_pb(&func->params, (ll)BASE(parse_param()));

            if (lh.type == tok_comma) {
                match(tok_comma);
            } else if (lh.type != tok_r_paren) {
                error(lh.range, "函数参数声明以非预期的符号 %s 结尾，应为“,”或“)”", tok_to_str(lh.type));
                break;
            }
        }

        match(tok_r_paren);
    }

    // func() { ... }
    //        ^^^^^^^
    {
        match(tok_l_brace);
        func->body_range.start = tk.range.start;

        func->decl_list_range.start = lh.range.start;
        // func() { int a, b; a = a + 1; }
        //          ^^^^^^^^^
        while (lh.type == tok_kw_int) {
            parse_declarations(&func->decls);
        }
        func->decl_list_range.end = tk.range.end;

        func->stat_list_range.start = lh.range.start;
        // func() { int a, b; a = a + 1; }
        //                    ^^^^^^^^^^
        while (lh.type != tok_r_brace) {
            vec_pb(&func->stmts, (ll)parse_statement());
        }
        func->stat_list_range.end = tk.range.end;

        match(tok_r_brace);
        func->body_range.end = tk.range.end;
    }

    // printf("FUNCTION %s: \n", func->name);
    // trie_print(current_frame);

    // 离开作用域
    current_frame = old_frame;

    OUT;

    func->base.range.end = tk.range.end;
    return func;
}

// (6) <declaration_stmt>::= int ID[ '[' <expr> ']'] [= <expr>], {ID [=
// <expr>]};
void parse_var_decl(struct vector *decls) {
    IN;

    struct source_range range = lh.range;

    // int a, b[10];
    // ^^^
    match(tok_kw_int);

    // int a, b[10];
    //     ^^^^^^^^
    do {
        vec_pb(decls, (ll)BASE(parse_declarator(range)));
        if (lh.type == tok_comma) {
            match(tok_comma);
        } else {
            break;
        }
    } while (1);

    match(tok_semi);
    OUT;
}

struct var_decl *parse_declarator(struct source_range range) {
    IN;

    new (struct var_decl, decl);
    decl->base.range.start = lh.range.start;
    decl->base.kind = decl_var;

    new (struct type, ty);
    ty->range = range;
    ty->kind = ty_int;

    // (int a,) b[10] = 1 (, c ...;)
    //          ^
    match(tok_id);
    decl->name = move(tk.name);

    {
        ll t = trie_add(current_frame, decl->name, (ll)decl);
        struct declaration *existed_decl = (struct declaration *)t;
            
        if (t != -1) {
            error(tk.range, "命名冲突，%s已经定义于位置 第%d行，第%d列。",
                  decl->name, existed_decl->range.start.line,
                  existed_decl->range.start.pos);
            return NULL;
        }
    }

    // (int a,) b[10] = 1 (, c ...;)
    //           ^^^^
    if (lh.type == tok_l_square) {
        match(tok_l_square);
        ty->kind = ty_array;
        if (lh.type == tok_num) {
            match(tok_num);
            ty->array_type.length = tk.num;
        }
        match(tok_r_square);
    }

    decl->ty = ty;

    // (int a,) b[10] = 1 (, c ...;)
    //                ^^^
    if (lh.type == tok_equal) {
        match(tok_equal);
        decl->init = parse_expr();
    }

    decl->base.range.end = tk.range.end;
    return decl;
}

// (8) <statement> ::= <if_stmt> | <while_stmt> | <for_stmt> | <read_stmt>
//       | <write_stmt> | <compound_stmt> | <expression_stmt> | <call_stmt>
struct statement *parse_statement() {
    IN;

    switch (lh.type) {
    case tok_kw_if:
        return BASE(parse_if_stmt());
    case tok_kw_while:
        return BASE(parse_while_stmt());
    case tok_kw_do:
        return BASE(parse_do_while_stmt());
    case tok_kw_for:
        return BASE(parse_for_stmt());
    case tok_kw_read:
        return BASE(parse_read_stmt());
    case tok_kw_write:
        return BASE(parse_write_stmt());
    case tok_kw_switch:
        return BASE(parse_switch_stmt());
    case tok_l_brace:
        return BASE(parse_compound_stmt());
    case tok_kw_return:
        return BASE(parse_return_stmt());
    case tok_kw_case:
        return BASE(parse_case_stmt());
    case tok_semi: {
        new (struct statement, stmt);
        stmt->kind = stmt_empty;
        match(tok_semi);
        return stmt;
    }
    case tok_kw_break: {
        new (struct statement, stmt);
        stmt->kind = stmt_break;
        match(tok_kw_break);
        match(tok_semi);
        return stmt;
    }
    case tok_kw_continue: {
        new (struct statement, stmt);
        stmt->kind = stmt_continue;
        match(tok_kw_continue);
        match(tok_semi);
        return stmt;
    }
    case tok_kw_default: {
        new (struct statement, stmt);
        stmt->kind = stmt_default_clause;
        match(tok_kw_default);
        match(tok_colon);
        return stmt;
    }
    case tok_id:
    case tok_num:
    case tok_l_paren:
    case tok_exclaim:
        return BASE(parse_expr_stmt());
    default:
        error(lh.range, "未知的语句开始符号 “%s”", tok_to_str(lh.type));
        match(lh.type);
        return 0;
    }

    OUT;
}

struct expr_stmt *parse_expr_stmt() {
    IN;

    new (struct expr_stmt, stmt);
    stmt->base.range.start = lh.range.start;
    stmt->base.kind = stmt_expr;

    // a = a + 1;
    // ^^^^^^^^^
    stmt->exp = parse_expr();

    match(tok_semi);

    OUT;

    stmt->base.range.end = tk.range.end;
    return stmt;
}

struct case_clause_stmt *parse_case_stmt() {
    IN;

    new (struct case_clause_stmt, stmt);
    stmt->base.range.start = lh.range.start;
    stmt->base.kind = stmt_case_clause;

    // case 'a':
    // ^^^^
    match(tok_kw_case);

    // case 'a':
    //      ^^^
    stmt->val = parse_expr();

    // case 'a':
    //         ^
    match(tok_colon);

    OUT;

    stmt->base.range.end = tk.range.end;
    return stmt;
}

struct return_stmt *parse_return_stmt() {
    IN;

    new (struct return_stmt, stmt);
    stmt->base.range.start = lh.range.start;
    stmt->base.kind = stmt_return;

    // return 12;
    // ^^^^^^
    match(tok_kw_return);

    if (lh.type != tok_semi) {
        // return 12;
        //        ^^
        stmt->val = parse_expr();
    } else {
        // return;
        //       ^
        stmt->val = 0;
    }

    // return 12;
    //          ^
    match(tok_semi);

    OUT;

    stmt->base.range.end = tk.range.end;
    return stmt;
}

struct switch_stmt *parse_switch_stmt() {
    IN;

    new (struct switch_stmt, stmt);
    stmt->base.range.start = lh.range.start;
    stmt->base.kind = stmt_switch;
    vec_init(&stmt->stmts);

    // switch(c) { ... }
    // ^^^^^^
    match(tok_kw_switch);

    // switch(c) { ... }
    //       ^^^
    match(tok_l_paren);
    stmt->val = parse_expr();
    match(tok_r_paren);

    // switch(c) { ... }
    //           ^^^^^^^
    {
        match(tok_l_brace);

        while (lh.type != tok_r_brace) {
            vec_pb(&stmt->stmts, (ll)parse_statement());
        }

        match(tok_r_brace);
    }

    OUT;

    stmt->base.range.end = tk.range.end;
    return stmt;
}

// (9) <if_stmt>::= if ‘(’ <expr> ‘)’ <statement> [else <statement>]
struct if_stmt *parse_if_stmt() {
    IN;

    new (struct if_stmt, stmt);
    stmt->base.range.start = lh.range.start;
    stmt->base.kind = stmt_if;

    // if (123) { ... } else { ... }
    // ^^
    match(tok_kw_if);

    // if (123) { ... } else { ... }
    //    ^^^^^
    match(tok_l_paren);
    stmt->cond = parse_expr();
    match(tok_r_paren);

    // if (123) { ... } else { ... }
    //          ^^^^^^^
    stmt->then_branch = parse_statement();

    // if (123) { ... } else { ... }
    //                  ^^^^^^^^^^^^
    stmt->else_branch = NULL;
    if (lh.type == tok_kw_else) {
        match(tok_kw_else);
        stmt->else_branch = parse_statement();
    }

    OUT;

    stmt->base.range.end = tk.range.end;
    return stmt;
}

// (10) <while_stmt>::= while ‘(’ <bool_expr> ‘)’ <statement>
struct while_stmt *parse_while_stmt() {
    IN;

    new (struct while_stmt, stmt);
    stmt->base.range.start = lh.range.start;
    stmt->base.kind = stmt_while;

    // while (123) { ... }
    // ^^^^^
    match(tok_kw_while);

    // while (123) { ... }
    //       ^^^^^
    match(tok_l_paren);
    stmt->cond = parse_expr();
    match(tok_r_paren);

    // while (123) { ... }
    //             ^^^^^^^
    stmt->body = parse_statement();

    OUT;

    stmt->base.range.end = tk.range.end;
    return stmt;
}

// (10) <while_stmt>::= while ‘(’ <bool_expr> ‘)’ <statement>
struct do_while_stmt *parse_do_while_stmt() {
    IN;

    new (struct do_while_stmt, stmt);
    stmt->base.range.start = lh.range.start;
    stmt->base.kind = stmt_do_while;

    // do { ... } while (123);
    // ^^^^^^^^^^^^^^^^
    match(tok_kw_do);
    stmt->body = parse_statement();
    match(tok_kw_while);

    // do { ... } while (123);
    //                  ^^^^^
    match(tok_l_paren);
    stmt->cond = parse_expr();
    match(tok_r_paren);

    // do { ... } while (123);
    //                       ^
    match(tok_semi);

    OUT;

    stmt->base.range.end = tk.range.end;
    return stmt;
}

// (11) <for_stmt>::= for ‘(’ <expression> ; <bool_expr> ; <expression> ‘)’
// <statement>
struct for_stmt *parse_for_stmt() {
    IN;

    new (struct for_stmt, stmt);
    stmt->base.range.start = lh.range.start;
    stmt->base.kind = stmt_for;

    // for (i = 0; i < n; i = i + 1) { ... }
    // ^^^
    match(tok_kw_for);

    // for (i = 0; i < n; i = i + 1) { ... }
    //     ^^^^^^^^^^^^^^^^^^^^^^^^^
    {
        match(tok_l_paren);
        stmt->init = parse_expr();
        match(tok_semi);
        stmt->cond = parse_expr();
        match(tok_semi);
        stmt->update = parse_expr();
        match(tok_r_paren);
    }

    // for (i = 0; i < n; i = i + 1) { ... }
    //                               ^^^^^^^
    stmt->body = parse_statement();

    OUT;

    stmt->base.range.end = tk.range.end;
    return stmt;
}

// (12) <write_stmt>::= write <expression> ;
struct write_stmt *parse_write_stmt() {
    IN;

    new (struct write_stmt, stmt);
    stmt->base.range.start = lh.range.start;
    stmt->base.kind = stmt_write;

    // write a + 1;
    // ^^^^^^^^^^^^
    match(tok_kw_write);
    stmt->exp = parse_expr();
    match(tok_semi);

    OUT;

    stmt->base.range.end = tk.range.end;
    return stmt;
}

// (13) <read_stmt>::= read ID;
struct read_stmt *parse_read_stmt() {
    IN;

    new (struct read_stmt, stmt);
    stmt->base.range.start = lh.range.start;
    stmt->base.kind = stmt_read;

    // read a;
    // ^^^^^^^
    match(tok_kw_read);
    stmt->var = parse_declref();
    match(tok_semi);

    OUT;

    stmt->base.range.end = tk.range.end;
    return stmt;
}

// (14) <compound_stmt>::= ‘{’ <statement_list> ‘}’
struct compound_stmt *parse_compound_stmt() {
    IN;

    new (struct compound_stmt, stmt);
    stmt->base.range.start = lh.range.start;
    stmt->base.kind = stmt_compound;
    vec_init(&stmt->stmts);

    // { ... }
    // ^
    match(tok_l_brace);

    // { ... }
    //   ^^^
    while (lh.type != tok_r_brace) {
        vec_pb(&stmt->stmts, (ll)parse_statement());
    }

    // { ... }
    //       ^
    match(tok_r_brace);

    OUT;

    stmt->base.range.end = tk.range.end;
    return stmt;
}

// (16) <call_stmt>::= ID '(' ')'
struct call_expr *parse_call_expr() {
    IN;

    new (struct call_expr, expr);
    expr->base.range.start = lh.range.start;
    expr->base.kind = expr_call;
    vec_init(&expr->params);

    // func(1, a, a + 1)
    // ^^^^
    expr->func = parse_declref();

    // func(1, a, a + 1)
    //     ^^^^^^^^^^^^^
    {
        match(tok_l_paren);

        while (lh.type != tok_r_paren) {
            vec_pb(&expr->params, (ll)parse_expr());
            if (lh.type == tok_comma) {
                match(tok_comma);
            } else if (lh.type != tok_r_paren) {
                error(lh.range, "函数调用参数以非预期的符号结尾，应为“,”或“)”");
            }
        }

        match(tok_r_paren);
    }

    OUT;

    expr->base.range.end = tk.range.end;
    return expr;
}

int get_op_precedence(enum tok type) {
    switch (type) {
    case tok_star:
    case tok_slash:
        return 5;
    case tok_plus:
    case tok_minus:
        return 4;
    case tok_less:
    case tok_lessequal:
    case tok_greater:
    case tok_greaterequal:
        return 3;
    case tok_equalequal:
    case tok_exclaimequal:
        return 2;
    case tok_equal:
        return 1;
    default:
        return 0;
    }
}

struct expr *parse_binary_expr(int min_prec) {
    IN;

    struct expr *lhs = parse_factor();

    while (1) {
        int prec = get_op_precedence(lh.type);
        if (prec < min_prec)
            break;

        enum binary_expr_kind kind;
        switch (lh.type) {
        case tok_greater:
            kind = bin_greater;
            break;
        case tok_less:
            kind = bin_less;
            break;
        case tok_greaterequal:
            kind = bin_greater_equal;
            break;
        case tok_lessequal:
            kind = bin_less_equal;
            break;
        case tok_equalequal:
            kind = bin_equal;
            break;
        case tok_exclaimequal:
            kind = bin_not_equal;
            break;
        case tok_star:
            kind = bin_mul;
            break;
        case tok_slash:
            kind = bin_div;
            break;
        case tok_plus:
            kind = bin_add;
            break;
        case tok_minus:
            kind = bin_minus;
            break;
        case tok_equal:
            kind = bin_assign;
            break;
        default:

            return lhs;
        }

        new (struct binary_expr, expr);
        if (lhs) {
            expr->base.range.start = lhs->range.start;
        }
        expr->base.kind = expr_binary;

        expr->lhs = lhs;

        match(lh.type);

        expr->kind = kind;

        int next_min_prec = prec + (tk.type == tok_equal ? 0 : 1);
        expr->rhs = parse_binary_expr(next_min_prec);

        expr->base.range.end = tk.range.end;
        lhs = BASE(expr);
    }

    OUT;

    return lhs;
}

// (17) <expression>::= ID = <bool_expr> | <bool_expr>
struct expr *parse_expr() {
    IN;

    return parse_binary_expr(0);

    OUT;
}

// (21) <factor>::= ‘(’ <additive_expr> ‘)’ | ID | NUM | CALL | SUBSCRIPT | !expr | -expr
struct expr *parse_factor() {
    IN;

    // (a + b)
    // ^^^^^^^
    if (lh.type == tok_l_paren) {
        new (struct paren_expr, expr);
        expr->base.range.start = lh.range.start;
        expr->base.kind = expr_paren;

        match(tok_l_paren);
        expr->expr = parse_expr();
        match(tok_r_paren);

        expr->base.range.end = tk.range.end;
        return BASE(expr);
    }

    // - k - u
    // ^^^
    else if (lh.type == tok_minus) {
        new (struct unary_expr, expr);
        expr->base.range.start = lh.range.start;
        expr->base.kind = expr_unary;

        match(tok_minus);
        expr->kind = una_neg;

        expr->operand = parse_factor();

        expr->base.range.end = tk.range.end;
        return BASE(expr);
    }

    // !Sk1ua
    // ^^^^^^
    else if (lh.type == tok_exclaim) {
        new (struct unary_expr, expr);
        expr->base.range.start = lh.range.start;
        expr->base.kind = expr_unary;

        match(tok_exclaim);
        expr->kind = una_not;

        expr->operand = parse_factor();

        expr->base.range.end = tk.range.end;
        return BASE(expr);
    }

    // Sk() | Sk[1] | Sk
    // ^^     ^^      ^^
    else if (lh.type == tok_id) {

        // Sk()
        //   ^^
        if (lh2.type == tok_l_paren) {
            return BASE(parse_call_expr());
        }

        // Sk[i]
        //   ^^^
        else if (lh2.type == tok_l_square) {
            return BASE(parse_subscript_expr());
        }

        // Sk
        // ^^
        else {
            return BASE(parse_declref());
        }
    }

    // 53145
    // ^^^^^
    else if (lh.type == tok_num) {
        return BASE(parse_lit());
    }

    // error
    else {
        error(lh.range, "<factor> 未知的开始符号 “%s”", tok_to_str(lh.type));
        match(lh.type);
        return NULL;
    }
}

// <declref>:= ID
struct declref *parse_declref() {
    IN;

    new (struct declref, expr);
    expr->base.range.start = lh.range.start;
    expr->base.kind = expr_declref;

    // Sk1ua
    // ^^^^^
    match(tok_id);
    expr->name = move(tk.name);

    struct declaration *decl = (struct declaration *)trie_find(current_frame, expr->name);
    if (!decl) {
        // trie_print(current_frame);
        error(tk.range, "未能找到标识符定义：%s", tk.name);
    }

    OUT;

    expr->base.range.end = tk.range.end;
    return expr;
}

// <lit>:= num
struct literal *parse_lit() {
    IN;

    new (struct literal, expr);
    expr->base.range.start = lh.range.start;
    expr->base.kind = expr_literal;

    // 12345
    // ^^^^^
    match(tok_num);
    expr->num = tk.num;

    OUT;

    expr->base.range.end = tk.range.end;
    return expr;
}

struct subscript_expr *parse_subscript_expr() {
    IN;

    new (struct subscript_expr, expr);
    expr->base.range.start = lh.range.start;
    expr->base.kind = expr_subscript;

    // Sk[1]
    // ^^
    expr->var = parse_declref();

    // Sk[1]
    //   ^^^
    match(tok_l_square);
    expr->exp = parse_expr();
    assert(expr->exp);
    match(tok_r_square);

    OUT;

    expr->base.range.end = tk.range.end;
    return expr;
}
#pragma once

#include "source_position.h"
#include "util.h"

enum type_kind { ty_int, ty_array, ty_void, ty_ptr };

struct type {
    struct source_range range;
    enum type_kind kind;
    union {
        struct {
            int length;
        } array_type;
    };
};

enum expr_kind {
    expr_binary,
    expr_unary,
    expr_literal,
    expr_declref,
    expr_call,
    expr_subscript,
    expr_paren,
};

struct expr {
    struct source_range range;
    enum expr_kind kind;
};

enum unary_expr_kind {
    una_not,
    una_neg,
};

struct unary_expr {
    struct expr base;
    
    enum unary_expr_kind kind;
    struct expr *operand;
};

enum binary_expr_kind {
    bin_add,
    bin_minus,
    bin_mul,
    bin_div,
    bin_greater,
    bin_less,
    bin_greater_equal,
    bin_less_equal,
    bin_equal,
    bin_not_equal,
    bin_assign,
};

struct binary_expr {
    struct expr base;

    enum binary_expr_kind kind;
    struct expr *lhs, *rhs;
};

struct declref {
    struct expr base;
    char *name;
    int var_id;
};

struct literal {
    struct expr base;
    int num;
};

struct call_expr {
    struct expr base;
    struct declref *func;
    struct vector params;
};

struct subscript_expr {
    struct expr base;
    struct declref *var;
    struct expr *exp;
};

struct paren_expr {
    struct expr base;

    struct expr *expr;
};

enum stmt_kind {
    stmt_if,
    stmt_switch,
    stmt_case_clause,
    stmt_default_clause,

    stmt_while,
    stmt_do_while,
    stmt_for,

    stmt_break,
    stmt_continue,
    stmt_return,

    stmt_read,
    stmt_write,

    stmt_compound,
    stmt_expr,
    stmt_empty,
};

struct statement {
    struct source_range range;
    enum stmt_kind kind;
};

struct if_stmt {
    struct statement base;

    struct expr *cond;
    struct statement *then_branch;
    struct statement *else_branch;
};

struct switch_stmt {
    struct statement base;

    struct expr *val;
    struct vector stmts;
};

struct case_clause_stmt {
    struct statement base;

    struct expr *val;
};

struct while_stmt {
    struct statement base;

    struct expr *cond;
    struct statement *body;
};

struct do_while_stmt {
    struct statement base;

    struct expr *cond;
    struct statement *body;
};

struct for_stmt {
    struct statement base;

    struct expr *init, *cond, *update;
    struct statement *body;
};

struct return_stmt {
    struct statement base;

    struct expr *val;
};

struct read_stmt {
    struct statement base;

    struct declref *var;
};

struct write_stmt {
    struct statement base;

    struct expr *exp;
};

struct compound_stmt {
    struct statement base;

    struct vector stmts;
};

struct expr_stmt {
    struct statement base;

    struct expr *exp;
};

/*
 *  声明节点 declaration
 *  变量声明 var_decl
 *  函数参数声明 param_decl
 *  函数声明 function_decl
 */

enum decl_kind {
    decl_var,
    decl_param,
    decl_function,
};

struct declaration {
    struct source_range range;
    enum decl_kind kind;
};

struct var_decl {
    struct declaration base;

    struct type *ty;
    char *name;
    struct expr *init;
};

struct param_decl {
    struct declaration base;

    struct type *ty;
    char *name;
};

struct function_decl {
    struct declaration base;

    char *name;
    struct type *ret_ty;
    struct vector params;

    struct source_range body_range;
    struct source_range decl_list_range;
    struct source_range stat_list_range;

    struct vector decls;
    struct vector stmts;

    struct trie frame;
};

struct program {
    struct source_range range;

    struct trie frame;

    struct vector decls;
};
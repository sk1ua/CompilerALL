#pragma once
#include "ast.h"

// 缩进打印
void print_indent();


// 打印语法符号
void print_syntax(const char *name);

// 打印表达式节点
void print_expr(struct expr *e);

// 打印语句节点
void print_stmt(struct statement *stmt);

// 打印整个程序语法树
void print_program(struct program *prog);

void print_declaration(struct declaration *d, int indent);
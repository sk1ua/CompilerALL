#include "ast.h"
#include "source_position.h"
#include "stdio.h"
#include <assert.h>

int indent;

void print_indent() {
    for (int i = 0; i < indent; ++i)
        printf(" ");
}

void print_token(const char *token) {
    print_indent();
    printf("%s\n", token);
}

void print_unit(const char *name, struct source_range range) {
    print_indent();
    printf("%s (%d)\n", name, range.start.line);
}

#define UPPER(...) { indent += 2; __VA_ARGS__ indent -= 2; }

void print_expr(struct expr *e) {
    if (!e) {
        assert(0);
    }

    fflush(stdout);
    switch (e->kind) {
    case expr_binary: {
        struct binary_expr *b = (struct binary_expr *)e;
        print_unit("binary_expr", e->range);

        UPPER(
            print_expr(b->lhs);
    
            switch (b->kind) {
            case bin_add:
                print_token("PLUS");
                break;
            case bin_minus:
                print_token("MINUS");
                break;
            case bin_mul:
                print_token("STAR");
                break;
            case bin_div:
                print_token("DIV");
                break;
            case bin_greater:
                print_token("GT");
                break;
            case bin_less:
                print_token("LT");
                break;
            case bin_greater_equal:
                print_token("GE");
                break;
            case bin_less_equal:
                print_token("LE");
                break;
            case bin_equal:
                print_token("EQ");
                break;
            case bin_not_equal:
                print_token("NE");
                break;
            case bin_assign:
                print_token("ASSIGN");
                break;
            default:
                print_token("UNKNOWN_BIN");
                break;
            }
    
            print_expr(b->rhs);
        )
        break;
    }

    case expr_unary: {
        struct unary_expr *u = (struct unary_expr *)e;
        print_unit("unary_expr", e->range);
        UPPER(
            if (u->kind == una_not) {
                print_token("NOT");
            } else {
                print_token("NEG");
            }
            print_expr(u->operand);
        )
        break;
    }

    case expr_literal: {
        struct literal *lit = (struct literal *)e;
        print_indent();
        printf("%s: %d\n", "INT", lit->num);
        break;
    }

    case expr_declref: {
        struct declref *d = (struct declref *)e;
        print_indent();
        printf("%s %s\n", "ID", d->name);
        break;
    }

    case expr_call: {
        struct call_expr *c = (struct call_expr *)e;
        print_unit("call_expr", e->range);
        UPPER(
            print_expr((struct expr *)c->func);
            print_token("LP");
            for (int i = 0; i < c->params.size; ++i) {
                print_expr((struct expr *)c->params.data[i]);
                if (i < c->params.size - 1)
                    print_token("COMMA");
            }
            print_token("RP");
        )
        break;
    }

    case expr_subscript: {
        struct subscript_expr *s = (struct subscript_expr *)e;
        print_unit("subscript_expr", e->range);
        UPPER(
            print_expr((struct expr *)s->var);
            print_token("LB");
            print_expr((struct expr *)s->exp);
            print_token("RB");
        )
        break;
    }

    case expr_paren: {
        struct paren_expr *p = (struct paren_expr *)e;
        print_unit("paren_expr", e->range);
        UPPER(
            print_token("LP");
            print_expr(p->expr);
            print_token("RP");
        )
        break;
    }

    default:
        print_token("UNKNOWN_EXPR");
    }
}

void print_statement(struct statement *s) {
    switch (s->kind) {
    case stmt_if: {
        struct if_stmt *i = (struct if_stmt *)s;
        print_unit("if_stat", i->base.range);
        UPPER(
            print_token("IF");
    
            print_token("LP");
            print_expr(i->cond);
            print_token("RP");
    
            print_statement(i->then_branch);
    
            if (i->else_branch) {
                print_token("ELSE");
                print_statement(i->else_branch);
            }
        )
        break;
    }
    case stmt_while: {
        struct while_stmt *w = (struct while_stmt *)s;
        print_unit("while_stat", w->base.range);

        UPPER(
            print_token("WHILE");
    
            print_token("LP");
            print_expr(w->cond);
            print_token("RP");
    
            print_statement(w->body);
        )
        break;
    }
    case stmt_do_while: {
        struct do_while_stmt *dw = (struct do_while_stmt *)s;
        print_unit("do_while_stat", dw->base.range);

        UPPER(
            print_token("DO");
            
            print_statement(dw->body);
            
            print_token("WHILE");
            print_token("LP");
            print_expr(dw->cond);
            print_token("RP");
            print_token("SEMI");
        )
        break;
    }
    case stmt_for: {
        struct for_stmt *f = (struct for_stmt *)s;
        print_unit("for_stat", f->base.range);

        UPPER(
            print_token("FOR");
    
            print_token("LP");
            if (f->init)
                print_expr(f->init);
            print_token("SEMI");
            if (f->cond)
                print_expr(f->cond);
            print_token("SEMI");
            if (f->update)
                print_expr(f->update);
            print_token("RP");
    
            print_statement(f->body);
        )
        break;
    }
    case stmt_switch: {
        struct switch_stmt *sw = (struct switch_stmt *)s;
        print_unit("switch_stat", sw->base.range);

        UPPER(
            print_token("SWITCH");
            print_token("LP");
            print_expr(sw->val);
            print_token("RP");
    
            print_token("LC");
            for (int i = 0; i < sw->stmts.size; ++i)
                print_statement((struct statement *)sw->stmts.data[i]);
            print_token("RC");
        )
        break;
    }
    case stmt_case_clause: {
        struct case_clause_stmt *cs = (struct case_clause_stmt *)s;
        print_unit("case_clause", cs->base.range);

        UPPER(
            print_token("CASE");
            print_expr(cs->val);
            print_token("COLON");
        )
        break;
    }
    case stmt_default_clause: {
        print_unit("default_clause", s->range);

        UPPER(
            print_token("DEFAULT");
            print_token("COLON");
        )
        break;
    }
    case stmt_return: {
        struct return_stmt *r = (struct return_stmt *)s;
        print_unit("return_stat", r->base.range);
        UPPER(
            print_token("RETURN");
            if (r->val)
                print_expr(r->val);
            print_token("SEMI");
        )
        break;
    }
    case stmt_compound: {
        struct compound_stmt *cs = (struct compound_stmt *)s;
        print_unit("compund_stat", cs->base.range);

        UPPER(
            print_token("LC");
    
            for (int i = 0; i < cs->stmts.size; ++i) {
                print_statement((struct statement *)cs->stmts.data[i]);
            }
    
            print_token("RC");
        )
        break;
    }
    case stmt_expr: {
        struct expr_stmt *e = (struct expr_stmt *)s;
        print_expr(e->exp);
        UPPER(
            print_token("SEMI");
        )
        break;
    }
    case stmt_break: {
        UPPER(
            print_token("BREAK");
            print_token("SEMI");
        )
        break;
    }
    case stmt_continue: {
        UPPER(
            print_token("CONTINUE");
            print_token("SEMI");
        )
        break;
    }
    case stmt_empty: {
        print_token("SEMI");
        break;
    }
    case stmt_read: {
        struct read_stmt *r = (struct read_stmt *)s;
        print_unit("read_stat", s->range);
        UPPER(
            print_token("READ");

            print_indent();
            printf("%s %s\n", "ID", r->var->name);

            print_token("SEMI");
        )
        break;
    }

    case stmt_write: {
        struct write_stmt *w = (struct write_stmt *)s;
        print_unit("write_stat", s->range);
        UPPER(
            print_token("WRITE");
            print_expr(w->exp);
            print_token("SEMI");
        )
        break;
    }
    default:
        print_indent();
        assert(0);
        printf("UNKNOWN_STATEMENT\n");
    }
}

void print_declaration(struct declaration *d) {
    if (d->kind == decl_function) {
        struct function_decl *f = (struct function_decl *)d;

        print_unit("function_declaration", f->base.range);

        UPPER(
            print_indent();
            printf("ID: %s\n", f->name);
    
            print_token("LC");
    
            for (int i = 0; i < f->params.size; ++i) {
                if (i > 0) {
                    print_token("COMMA");
                }
                UPPER(
                    print_declaration((struct declaration *)f->params.data[i]);
                )
            }
    
            print_token("RP");
    
            print_unit("function_body", f->body_range);
    
            print_token("LC");
    
            UPPER(
                for (int i = 0; i < f->decls.size; ++i) {
                    print_declaration((struct declaration *)f->decls.data[i]);
                }
            )
    
            UPPER(
                for (int i = 0; i < f->stmts.size; ++i) {
                    print_statement((struct statement *)f->stmts.data[i]);
                }
            )
    
            print_token("RC");
        )


    } else if (d->kind == decl_var) {
        struct var_decl *v = (struct var_decl *)d;

        print_unit("declaration_stat", v->base.range);

        UPPER(
            print_indent();
            if (v->ty->kind == ty_int) {
                printf("TYPE: int\n");
            } else if (v->ty->kind == ty_array) {
                printf("TYPE: int[%d]\n", v->ty->array_type.length);
            } else {
                printf("TYPE: unknown\n");
            }
    
            print_indent();
            printf("ID: %s\n", v->name);
    
            if (v->init) {
                print_token("INIT");
                UPPER(
                    print_expr(v->init);
                )
            }
    
            print_token("SEMI");
        )
    } else if (d->kind == decl_param) {
        struct param_decl *v = (struct param_decl *)d;

        print_unit("param_declaration_stat", v->base.range);

        UPPER(
            print_indent();
            if (v->ty->kind == ty_int) {
                printf("TYPE: int\n");
            } else if (v->ty->kind == ty_array) {
                printf("TYPE: int[%d]\n", v->ty->array_type.length);
            } else {
                printf("TYPE: unknown\n");
            }
    
            print_indent();
            printf("ID: %s\n", v->name);
        )
    }
}

void print_program(struct program *p) {
    print_indent();
    printf("Program (1)\n");
    UPPER(
        for (int i = 0; i < p->decls.size; ++i) {
            print_declaration((struct declaration *)p->decls.data[i]);
        }
    )
}
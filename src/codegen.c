#include "ast.h"
#include "util.h"
#include <stdarg.h>
#include <stdio.h>
// #define DBG(...) __VA_ARGS__
#define DBG(...)

struct trie * emitter_current_frame;

#define new(type, name) type * name = (type *) malloc(sizeof(type));

#define BASE(x) &(x->base)

enum instruction_kind {
    // load offset: PUSH(MEM[base + offset])
    load_inst,

    // load: PUSH(MEM[base + POP()])
    loada_inst,

    // loadi imm: PUSH(imm)
    loadi_inst,

    // sto offset: MEM[base + offset] = POP()
    sto_inst,
    stoa_inst,

    // operators: a = POP(), b = POP(), PUSH(a op b) or PUSH(op POP())
    add_inst,
    sub_inst,
    mult_inst,
    div_inst,
    eq_inst,
    noteq_inst,
    gt_inst,
    les_inst,
    ge_inst,
    le_inst,
    and_inst,
    or_inst,
    not_inst,
    
    // br ip_addr: ip = ip_addr 
    br_inst,
    // brf ip_addr: if (POP() == 0) ip = ip_addr 
    brf_inst,
    // in: PUSH(input())
    in_inst,
    // out: print(POP())
    out_inst,
    // ???? br 0
    stop_inst,
    // cal ip_addr: PUSH(base), PUSH(ip), base = top, ip = ip_addr
    cal_inst,
    // enter space: top += space
    enter_inst,
    // return: top = base + 2, ip = POP(), base = POP()
    return_inst,

    // pop: --top
    pop_inst,
};

const char * instruction_name[] =  {
    "LOAD",
    "LOADA",
    "LOADI",
    "STO",
    "STOA",
    "ADD",
    "SUB",
    "MULT",
    "DIV",
    "EQ",
    "NOTEQ",
    "GT",
    "LES",
    "GE",
    "LE",
    "AND",
    "OR",
    "NOT",
    "BR",
    "BRF",
    "IN",
    "OUT",
    // STOP
    "BR 0",
    "CAL",
    "ENTER",
    "RETURN",
    // POP,
    "STO",
};

struct instruction {
    enum instruction_kind kind;
};

struct arg_instruction {
    struct instruction base;
    int operand;
    const char * extra_info;
};

static int lineno;



int label_count = 0;
struct vector break_labels;
struct vector continue_labels;
struct vector label_name;
struct vector label_lineno;
int new_label(const char * name) {
    char *buf;
    asprintf(&buf, "L%d_%s", label_count, name);
    vec_pb(&label_name, (ll)buf);
    vec_pb(&label_lineno, 0);
    int ret = label_count;
    label_count ++;
    return ret;
}

struct vector machine_codes;
void print_inst(int kind, ...) {
    DBG(printf("%s", instruction_name[kind]);)
    if (kind == loadi_inst || kind == enter_inst || kind == br_inst || kind == brf_inst || kind == cal_inst) {
        new(struct arg_instruction, inst);
        inst->base.kind = (enum instruction_kind)kind;
        inst->extra_info = 0;

        va_list args;
        va_start(args, kind);
        int imm = va_arg(args, int);
        
        inst->operand = imm;
        vec_pb(&machine_codes, (ll)BASE(inst));

        DBG(
            printf(" %d", imm);
            if (kind == br_inst || kind == brf_inst || kind == cal_inst) {
                printf (" (%s)", (char *)label_name.data[imm]);
            }
        )
    } else if (kind == load_inst || kind == sto_inst)  {
        new(struct arg_instruction, inst);
        inst->base.kind = (enum instruction_kind)kind;
        inst->extra_info = 0;

        va_list args;
        va_start(args, kind);
        int imm = va_arg(args, int);
        const char * name = va_arg(args, const char *);

        inst->operand = imm;
        inst->extra_info = name;
        vec_pb(&machine_codes, (ll)BASE(inst));

        DBG(printf (" %d (%s)", imm, inst->extra_info);)
    } else if (kind == pop_inst) {
        new(struct arg_instruction, inst);
        inst->base.kind = sto_inst;
        inst->operand = -1;
        inst->extra_info = 0;
        vec_pb(&machine_codes, (ll)BASE(inst));
    } else {
        new(struct instruction, inst);
        inst->kind = (enum instruction_kind)kind;
        vec_pb(&machine_codes, (ll)inst);
    }
    DBG(printf("\n");)
    lineno += 1;
}

int get_base = 0;
void set_label(int label) {
    label_lineno.data[label] = (lineno);
    DBG(printf("set label %s = %d\n", (char *)label_name.data[label], (int)label_lineno.data[label]);)
}

void print_all() {
    for (int i = 0; i < machine_codes.size; ++i) {
        struct instruction * inst = (struct instruction *)machine_codes.data[i];
        enum instruction_kind kind = inst->kind;
        DBG(printf("%d: ", i + 1);)
        printf("%s", instruction_name[kind]);
        if (kind == loadi_inst || kind == enter_inst) {
            printf(" %d", ((struct arg_instruction *)inst)->operand);
        } else if (kind == br_inst || kind == brf_inst || kind == cal_inst) {
            int label = ((struct arg_instruction *)inst)->operand;
            printf(" %d", (int)label_lineno.data[label]);
            DBG(printf(" (%s)", (const char *)label_name.data[label]);)
        } else if (kind == load_inst || kind == sto_inst)  {
            int addr = ((struct arg_instruction *)inst)->operand;
            printf(" %d", addr);
            // const char * name = ((struct arg_instruction *)inst)->extra_info;
            // if (name) {
            //     printf(" (%s)", name);
            // }
        } else {
            if (kind != stop_inst) {
                printf(" 0");
            }
        }
        printf("\n");
    }
}

void load_glob_addr(const char *name) {
    print_inst(loadi_inst, 0);
    print_inst(enter_inst, 1);
    print_inst(cal_inst, get_base);
    print_inst(sub_inst);
    int offset = (long)trie_find(emitter_current_frame, name);
    print_inst(loadi_inst, offset);
    print_inst(add_inst);
}

void emit_expr(struct expr *e);
void emit_stmt(struct statement *s);

void emit_expr(struct expr *e) {
    switch (e->kind) {
        case expr_literal: {
            struct literal *lit = (struct literal *)e;
            print_inst(loadi_inst, lit->num);
            break;
        }
        case expr_declref: {
            struct declref *ref = (struct declref *)e;
            if (trie_is_local(emitter_current_frame, ref->name)) {
                int offset = (long)trie_find(emitter_current_frame, ref->name);
                print_inst(load_inst, offset, ref->name);
            } else {
                load_glob_addr(ref->name);
                print_inst(loada_inst);
            }
            break;
        }
        case expr_binary: {
            struct binary_expr *bin = (struct binary_expr *)e;
            if (bin->kind == bin_assign) {
                emit_expr(bin->rhs);
                if (bin->lhs->kind == expr_declref) {
                    struct declref *lhs = (struct declref *)bin->lhs;

                    if (trie_is_local(emitter_current_frame, lhs->name)) {
                        int offset = (long)trie_find(emitter_current_frame, lhs->name);
                        print_inst(sto_inst, offset, lhs->name);
                        print_inst(load_inst, offset, lhs->name);
                    } else {
                        load_glob_addr(lhs->name);
                        print_inst(stoa_inst);
                        load_glob_addr(lhs->name);
                        print_inst(loada_inst);
                    }
                } else if (bin->lhs->kind == expr_subscript) {
                    struct subscript_expr *lhs = (struct subscript_expr *)bin->lhs;

                    if (trie_is_local(emitter_current_frame, lhs->var->name)) {
                        int offset = (long)trie_find(emitter_current_frame, lhs->var->name);
    
                        emit_expr(lhs->exp);
    
                        print_inst(loadi_inst, offset);
                        print_inst(add_inst);
                        print_inst(stoa_inst);
    
                        emit_expr(lhs->exp);
    
                        print_inst(loadi_inst, offset);
                        print_inst(add_inst);
                        print_inst(loada_inst);
                    } else {
                        emit_expr(lhs->exp);

                        load_glob_addr(lhs->var->name);
                        print_inst(add_inst);
                        print_inst(stoa_inst);


                        load_glob_addr(lhs->var->name);
                        print_inst(add_inst);
                        print_inst(loada_inst);
                    }
                }
                break;
            }

            emit_expr(bin->lhs);
            emit_expr(bin->rhs);
            switch (bin->kind) {
                case bin_add: print_inst(add_inst); break;
                case bin_minus: print_inst(sub_inst); break;
                case bin_mul: print_inst(mult_inst); break;
                case bin_div: print_inst(div_inst); break;
                case bin_equal: print_inst(eq_inst); break;
                case bin_not_equal: print_inst(noteq_inst); break;
                case bin_greater: print_inst(gt_inst); break;
                case bin_less: print_inst(les_inst); break;
                case bin_greater_equal: print_inst(ge_inst); break;
                case bin_less_equal: print_inst(le_inst); break;
                default:
                break;
            }
            break;
        }
        case expr_unary: {
            struct unary_expr *u = (struct unary_expr *)e;
            if (u->kind == una_not) {
                emit_expr(u->operand);
                print_inst(not_inst);
            } else if (u->kind == una_neg) {
                print_inst(loadi_inst, 0);
                emit_expr(u->operand);
                print_inst(sub_inst);
            }
            break;
        }
        case expr_call: {
            struct call_expr *call = (struct call_expr *)e;

            // retval, base, ip
            print_inst(enter_inst, 3);

            // | current data... | - | - | - | ... | 
            // ^ base                        ^ top
            for (int i = 0; i < call->params.size; ++i) {
                emit_expr((struct expr *)call->params.data[i]);
            }

            // | current data... | - | - | - | params... | 
            // ^ base                                    ^ top

            // back
            print_inst(enter_inst, -(2 + call->params.size));
            
            // | current data... | - | - | - | params... | 
            // ^ base                ^ top
            int label = trie_find(emitter_current_frame, call->func->name);

            DBG(printf("emit call %s : %d\n", call->func->name, label);)
            print_inst(cal_inst, label, call->func->name);


            // | current data... | - | - | - | params... | 
            //                               ^ base, top
            break;
        }
        case expr_paren: {
            struct paren_expr *p = (struct paren_expr *)e;
            emit_expr(p->expr);
            break;
        }
        case expr_subscript: {
            struct subscript_expr *sub = (struct subscript_expr *)e;

            if (trie_is_local(emitter_current_frame, sub->var->name)) {
                int offset = (long)trie_find(emitter_current_frame, sub->var->name);

                emit_expr(sub->exp);

                print_inst(loadi_inst, offset);
                print_inst(add_inst);
                print_inst(loada_inst);
            } else {
                emit_expr(sub->exp);

                load_glob_addr(sub->var->name);
                print_inst(add_inst);
                print_inst(loada_inst);
            }
            break;
        }
    }
}

void emit_decl(struct declaration *d) {
    switch (d->kind) {
        case decl_var:
            struct var_decl *var = (struct var_decl *) d;
            if (var->init) {
                emit_expr(var->init);
                int offset = (long)trie_find(emitter_current_frame, var->name);
                print_inst(sto_inst, offset, var->name);
            }
            break;
        default:
    }
}

void emit_stmt(struct statement *s) {
    DBG(printf("EMIT STMT\n");)
    switch (s->kind) {
        case stmt_expr: {
            DBG(printf("EMIT EXPR\n");)
            struct expr_stmt *es = (struct expr_stmt *)s;
            emit_expr(es->exp);
            print_inst(pop_inst);
            break;
        }
        case stmt_return: {
            DBG(printf("EMIT RETURN\n");)
            struct return_stmt *rs = (struct return_stmt *)s;
            if (rs->val) {
                emit_expr(rs->val);
                print_inst(sto_inst, -1);
            }
            print_inst(return_inst);
            break;
        }
        case stmt_write: {
            DBG(printf("EMIT WRITE\n");)
            struct write_stmt *ws = (struct write_stmt *)s;
            emit_expr(ws->exp);
            print_inst(out_inst);
            break;
        }
        case stmt_read: {
            DBG(printf("EMIT READ\n");)
            struct read_stmt *rs = (struct read_stmt *)s;
            print_inst(in_inst);
            int offset = (long)trie_find(emitter_current_frame, rs->var->name);
            print_inst(sto_inst, offset, rs->var->name);
            break;
        }
        case stmt_if: {

            DBG(printf("EMIT IF\n");)
            struct if_stmt *ifs = (struct if_stmt *)s;
            int label_else = new_label("else_br");
            int label_end = new_label("if_end");
            emit_expr(ifs->cond);
            print_inst(brf_inst, label_else, label_name.data[label_else]);
            emit_stmt(ifs->then_branch);
            if (ifs->else_branch) {
                print_inst(br_inst, label_end);
                set_label(label_else);
                emit_stmt(ifs->else_branch);
                set_label(label_end);
            } else {
                set_label(label_else);
            }
            break;
        }
        case stmt_while: {
            DBG(printf("EMIT WHILE\n");)
            struct while_stmt *ws = (struct while_stmt *)s;
            int start = new_label("while_start");
            int end = new_label("while_end");
            vec_pb(&continue_labels, (ll)start);
            vec_pb(&break_labels, (ll)end);
            set_label(start);
            emit_expr(ws->cond);
            print_inst(brf_inst, end);
            emit_stmt(ws->body);
            print_inst(br_inst, start);
            set_label(end);
            vec_pop(&continue_labels);
            vec_pop(&break_labels);
            break;
        }
        case stmt_do_while: {
            DBG(printf("EMIT DO WHILE\n");)
            struct do_while_stmt *ds = (struct do_while_stmt *)s;
            int start = new_label("do_while_start");
            int end = new_label("do_while_end");
            vec_pb(&continue_labels, (ll)start);
            vec_pb(&break_labels, (ll)end);
            set_label(start);
            emit_stmt(ds->body);
            emit_expr(ds->cond);
            print_inst(brf_inst, end);
            print_inst(br_inst, start);
            set_label(end);
            vec_pop(&continue_labels);
            vec_pop(&break_labels);
            break;
        }
        case stmt_for: {
            DBG(printf("EMIT FOR\n");)
            struct for_stmt *fs = (struct for_stmt *)s;
            int start = new_label("for_start");
            int end = new_label("for_end");
            int update = new_label("for_update");
            if (fs->init) emit_expr(fs->init);

            vec_pb(&continue_labels, (ll)update);
            vec_pb(&break_labels, (ll)end);

            set_label(start);
            
            if (fs->cond) {
                emit_expr(fs->cond);
                print_inst(brf_inst, end);
            }

            emit_stmt(fs->body);

            set_label(update);
            
            if (fs->update) emit_expr(fs->update);

            print_inst(br_inst, start);
            set_label(end);


            vec_pop(&continue_labels);
            vec_pop(&break_labels);
            break;
        }
        case stmt_switch: {
            DBG(printf("EMIT SWITCH\n");)
            struct switch_stmt *ss = (struct switch_stmt *)s;
            int end = new_label("switch_end");

            vec_pb(&break_labels, (ll)end);

            new(struct vector, labels);
            vec_init(labels);
            for (int i = 0; i < ss->stmts.size; ++i) {
                struct statement *case_stmt = (struct statement *)ss->stmts.data[i];
                if (case_stmt->kind == stmt_case_clause) {
                    struct case_clause_stmt *cs = (struct case_clause_stmt *)case_stmt;
                    emit_expr(ss->val);
                    emit_expr(cs->val);
                    print_inst(eq_inst);
                    int label = new_label("case");
                    vec_pb(labels, (ll)label);
                    print_inst(loadi_inst, 0);
                    print_inst(eq_inst);
                    print_inst(brf_inst, label);
                } else if (case_stmt->kind == stmt_default_clause) {
                    int default_label = new_label("default");
                    vec_pb(labels, (ll)default_label);
                    print_inst(br_inst, default_label);
                }
            }
            print_inst(br_inst, end);
            
            int case_count = 0;
            for (int i = 0; i < ss->stmts.size; ++i) {
                struct statement *case_stmt = (struct statement *)ss->stmts.data[i];
                if (case_stmt->kind == stmt_case_clause || case_stmt->kind == stmt_default_clause) {
                    set_label((int)(long)labels->data[case_count]);
                    case_count += 1;
                } else {
                    emit_stmt(case_stmt);
                }
            }
            set_label(end);
            vec_pop(&break_labels);
            break;
        }
        case stmt_break: {
            DBG(printf("EMIT BREAK\n");)
            if (break_labels.size >= 0) {
                print_inst(br_inst, break_labels.data[break_labels.size - 1]);
            } else {
                printf("// error: break outside of loop/switch\n");
            }
            break;
        }
        case stmt_continue: {
            DBG(printf("EMIT CONTINUE\n");)
            if (continue_labels.size >= 0) {
                print_inst(br_inst, continue_labels.data[break_labels.size - 1]);
            } else {
                printf("// error: continue outside of loop\n");
            }
            break;
        }
        case stmt_compound: {
            DBG(printf("EMIT COMPOUND\n");)
            struct compound_stmt *cs = (struct compound_stmt *)s;
            for (int i = 0; i < cs->stmts.size; ++i) {
                emit_stmt((struct statement *)cs->stmts.data[i]);
            }
            break;
        }
        case stmt_empty: {
            break;
        }
        default:
            printf("// unsupported statement\n");
            break;
    }
}

long alloc_space(int base, struct vector * decls) {
    int offset = base;
    for (int i = 0; i < decls->size; ++i) {
        struct declaration * decl = (struct declaration *)decls->data[i];
        if (decl->kind == decl_var) {
            struct var_decl * var = (struct var_decl *)decl;
            if (var->ty->kind == ty_int) {
                DBG(printf("var %s offset = %d\n", var->name, offset);)
                trie_add(emitter_current_frame, var->name, offset);
                offset += 1;
            } else {
                DBG(printf("var %s[] offset = %d\n", var->name, offset);)
                trie_add(emitter_current_frame, var->name, offset);
                offset += var->ty->array_type.length;
            }
        } else if (decl->kind == decl_param) {
            struct param_decl * var = (struct param_decl *)decl;
            if (var->ty->kind == ty_int) {
                DBG(printf("param var %s offset = %d\n", var->name, offset);)
                trie_add(emitter_current_frame, var->name, offset);
                offset += 1;
            } else {
                DBG(printf("param var %s[] offset = %d\n", var->name, offset);)
                trie_add(emitter_current_frame, var->name, offset);
                offset += var->ty->array_type.length;
            }
        }
    }

    return offset;
}


void emit_program(struct program* prog) {
    freopen("../tests/code.txt", "w", stdout);
    new(struct trie, frame);
    trie_init(frame);
    vec_init(&label_name);
    vec_init(&label_lineno);
    vec_init(&break_labels);
    vec_init(&continue_labels);
    vec_init(&machine_codes);
    emitter_current_frame = frame;

    int static_count = alloc_space(0, &prog->decls);    

    get_base = new_label("func_get_base");
    int main_label = new_label("func_main");
    print_inst(enter_inst, static_count + 2 + 1);

    for (int j = 0; j < prog->decls.size; ++j) {
        emit_decl((struct declaration *)prog->decls.data[j]);
    }

    print_inst(cal_inst, main_label);
    print_inst(stop_inst);

    for (int i = 0; i < prog->decls.size; ++i) {
        if (((struct declaration *)prog->decls.data[i])->kind != decl_function) continue;
        struct function_decl* func = (struct function_decl *)prog->decls.data[i];
        char *buf;
        
        DBG(printf("EMIT FUNC %s START\n", func->name);)

        int label;
        if (strcmp(func->name, "main") == 0) {
            set_label(main_label);
            label = main_label;
        } else {
            asprintf(&buf, "func_%s", func->name);
            label = new_label(buf);
            set_label(label);
        }

        DBG(printf("decl %s : %d\n", func->name, label);)
        trie_add(emitter_current_frame, func->name, label);
    
        new(struct trie, frame);
        trie_init(frame);
        frame->parent = emitter_current_frame;
        struct trie * old_frame = emitter_current_frame;
        emitter_current_frame = frame;


        int function_data = 2;
        function_data = alloc_space(function_data, &func->params);
        function_data = alloc_space( function_data, &func->decls);
        print_inst(enter_inst, function_data);
    
        for (int j = 0; j < func->decls.size; ++j) {
            emit_decl((struct declaration *)func->decls.data[j]);
        }

        for (int j = 0; j < func->stmts.size; ++j) {
            emit_stmt((struct statement *)func->stmts.data[j]);
        }

        print_inst(return_inst);


        DBG(printf("EMIT FUNC %s END\n", func->name);)

        emitter_current_frame = old_frame;
    }
    
    set_label(get_base);
    print_inst(enter_inst, 2);
    print_inst(load_inst, 0);
    print_inst(sto_inst, -1);
    print_inst(return_inst);

    print_all();
}
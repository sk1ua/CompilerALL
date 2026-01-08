#include "util.h"
#include "ast.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void vec_init(struct vector *vec) {
    vec->data = (long long *)malloc(sizeof(long long) * 4);
    vec->size = 0;
    vec->capacity = 4;
}

void vec_pb(struct vector *vec, long long ptr) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (long long *)realloc(vec->data, sizeof(long long ) * vec->capacity);
    }
    vec->data[vec->size] = ptr;
    vec->size += 1;
}

void vec_pop(struct vector *vec) {
    --vec->size;
}

struct trie_node *new_node() {
    struct trie_node *ret =
        (struct trie_node *)malloc(sizeof(struct trie_node));
    ret->data = -1;
    memset(ret->tran, 0, ALPHABET * sizeof(struct trie_node *));
    return ret;
}

int encode(char c) {
    int code;
    if (islower(c)) {
        code = c - 'a';
    } else if (isupper(c)) {
        code = c - 'A' + 26;
    } else if (isdigit(c)) {
        code = c - '0' + 26 * 2;
    } else if (c == '_') {
        code = 26 * 2 + 10;
    } else {
        printf("err: %d\n", (int)c);
        assert(0);
    }

    return code;
}

ll trie_add(struct trie *trie, const char *str, ll data) {
    struct trie_node **ptr = &trie->root, * p = *ptr;

    while (*str) {
        char c = *str;

        if (!p) {
            *ptr = new_node();
            p = *ptr;
            p->data = -1;
        }

        ptr = &p->tran[encode(c)];
        p = *ptr;

        ++str;
    }

    if (!p) {
        *ptr = new_node();
        p = *ptr;
        p->data = -1;
    }

    if (p->data != -1) {
        return p->data;
    } 

    p->data = data;

    return -1;
}

ll trie_find(struct trie *trie, const char *str) {
    struct trie_node *p = trie->root;

    struct trie_node *p1 = NULL;
    if (trie->parent) {
        p1 = trie->parent->root;
    }

    while (*str) {
        char c = *str;

        if (!p) {
            if (p1) {
                p = p1;
                p1 = 0;
            } else {
                return 0;
            }
        }

        p = p->tran[encode(c)];
        if (p1) {
            p1 = p1->tran[encode(c)];
        }

        ++str;
    }

    if (!p) {
        if (p1) {
            p = p1;
            p1 = 0;
        } else {
            return 0;
        }
    }    

    if (p->data == -1) {
        if (p1) {
            return p1->data;
        } else {
            return 0;
        }
    }

    return p->data;
}

ll trie_is_local(struct trie *trie, const char *str) {
    struct trie_node *p = trie->root;

    while (*str) {
        char c = *str;

        if (!p) {
            return 0;
        }

        p = p->tran[encode(c)];

        ++str;
    }

    if (!p) {
        return 0;
    }    

    if (p->data == -1) {
        return 0;
    }

    return 1;
}

void trie_init(struct trie *trie) {
    trie->root = 0;
    trie->parent = 0;
}

void print_decl(struct declaration *d) {
    if (d->kind == decl_param) {
        struct param_decl * decl = (struct param_decl * ) d;
        printf("| PARAM %s (%d:%d to %d:%d)\n", decl->name, decl->base.range.start.line, decl->base.range.start.pos, decl->base.range.end.line, decl->base.range.end.pos);
    } else if (d->kind == decl_function) {
        struct function_decl * decl = (struct function_decl * ) d;
        printf("| FUNC %s (%d:%d to %d:%d)\n", decl->name, decl->base.range.start.line, decl->base.range.start.pos, decl->base.range.end.line, decl->base.range.end.pos);
    } else {
        struct var_decl * decl = (struct var_decl * ) d;
        printf("| VAR %s (%d:%d to %d:%d)\n", decl->name, decl->base.range.start.line, decl->base.range.start.pos, decl->base.range.end.line, decl->base.range.end.pos);
    }
}

void trie_node_print(struct trie_node * p, struct trie_node * p2) {
    if (!p) {
        if (p2) {
            trie_node_print(p2, 0);
        }
        return;
    }

    if (p->data != -1) {
        print_decl((struct declaration *)p->data);
    } else if (p2 && p2->data != -1) {
        print_decl((struct declaration *)p2->data);
    }

    for (int i = 0; i < ALPHABET; ++i) {
        struct trie_node * pp = p->tran[i];
        struct trie_node * pp2 = NULL;
        if (p2) {
            pp2 = p2->tran[i];
        }

        if (pp) {
            trie_node_print(pp, pp2);
        } else {
            trie_node_print(pp2, 0);
        }
    }
}

void trie_print(struct trie *trie) {
    struct trie_node * p2 = 0;
    if (trie->parent) {
        p2 = trie->parent->root;
    }
    trie_node_print(trie->root, p2);
}

struct vector * error_buf;
void error(struct source_range range, const char *format, ...) {
    if (!error_buf) {
        error_buf = (struct vector *) malloc(sizeof(struct vector));
        vec_init(error_buf);
    }

    char *err;
    asprintf(&err, "第 %d 行，第 %d 列：", range.start.line, range.start.pos);
    vec_pb(error_buf, (ll)err);

    va_list args;
    va_start(args, format);
    vasprintf(&err, format, args);
    vec_pb(error_buf, (ll)err);
    
    asprintf(&err, "\n");
    vec_pb(error_buf, (ll)err);
    va_end(args);
    fflush(stdout);
    // assert(0);
    // exit(-1);

}
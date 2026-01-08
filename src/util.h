#pragma once

#include <stdlib.h>
#include <string.h>
struct vector {
    long long *data;
    int size;
    int capacity;
};

typedef long long ll;

void vec_init(struct vector *vec);
void vec_pb(struct vector *vec, long long ptr);
void vec_pop(struct vector *vec);

#define ALPHABET (2 * 26 + 10 + 1)

struct trie_node {
    struct trie_node * tran[ALPHABET];
    ll data;
};

struct trie_node * new_node();

struct trie {
    struct trie_node * root;
    struct trie * parent;
};

ll trie_add(struct trie * trie, const char * string, ll data);
ll trie_find(struct trie * trie, const char * string);
void trie_init(struct trie *trie);
void trie_print(struct trie *trie);
ll trie_is_local(struct trie *trie, const char *str);

#include "ast.h"

extern struct vector * error_buf;
void error(struct source_range range, const char *format, ...);
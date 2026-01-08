#include "util.h"
#include <stdio.h>

extern struct program * parse_program();
extern void print_program(struct program *prog, int indent);
extern void emit_program(struct program *prog);

int main() {
    freopen("../tests/input.txt", "r", stdin);
    freopen("../tests/output.txt", "w", stdout);

    struct program * prog = parse_program();
    
    if (error_buf) {
        for (int i = 0; i < error_buf->size; ++i) {
            printf("%s", (char *)error_buf->data[i]);
        }
        printf("解析失败\n");
    } else {
        print_program(prog, 0);
        emit_program(prog);
    }
    return 0;
}
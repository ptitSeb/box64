#include <stdint.h>

typedef struct rbtree rbtree;

rbtree* init_rbtree();
void delete_rbtree(rbtree *tree);

uint8_t rb_get(rbtree *tree, uintptr_t addr);
int rb_get_end(rbtree* tree, uintptr_t addr, uint8_t* val, uintptr_t* end);
int rb_set(rbtree *tree, uintptr_t start, uintptr_t end, uint8_t data);
int rb_unset(rbtree *tree, uintptr_t start, uintptr_t end);

void print_rbtree(const rbtree *tree);
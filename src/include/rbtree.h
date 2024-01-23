#include <stdint.h>

#ifndef RBTREE_H
#define RBTREE_H

typedef struct rbtree rbtree;

rbtree* init_rbtree();
void delete_rbtree(rbtree *tree);

uint32_t rb_get(rbtree *tree, uintptr_t addr);
int rb_get_end(rbtree* tree, uintptr_t addr, uint32_t* val, uintptr_t* end);
int rb_set(rbtree *tree, uintptr_t start, uintptr_t end, uint32_t data);
int rb_unset(rbtree *tree, uintptr_t start, uintptr_t end);
uintptr_t rb_get_righter(rbtree *tree);

void print_rbtree(const rbtree *tree);

#endif // RBTREE_H

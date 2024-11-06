#ifndef RBTREE_H
#define RBTREE_H

#include <stdint.h>

typedef struct rbtree rbtree_t;

rbtree_t* rbtree_init(const char* name);
void rbtree_delete(rbtree_t* tree);

uint32_t rb_get(rbtree_t* tree, uintptr_t addr);
int rb_get_end(rbtree_t* tree, uintptr_t addr, uint32_t* val, uintptr_t* end);
int rb_set(rbtree_t* tree, uintptr_t start, uintptr_t end, uint32_t data);
int rb_unset(rbtree_t* tree, uintptr_t start, uintptr_t end);
uintptr_t rb_get_righter(rbtree_t* tree);

#endif // RBTREE_H

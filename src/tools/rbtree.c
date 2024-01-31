#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef RBTREE_TEST
#define rbtreeMalloc malloc
#define rbtreeFree free
#else
#include "custommem.h"
#include "debug.h"
#include "rbtree.h"
#if 0
#define rbtreeMalloc box_malloc
#define rbtreeFree box_free
#else
#define rbtreeMalloc customMalloc
#define rbtreeFree customFree
#endif
#endif

typedef struct rbnode {
    struct rbnode *left, *right, *parent;
    uintptr_t start, end;
    uint32_t data;
    uint8_t meta;
} rbnode;

typedef struct rbtree {
    rbnode *root;
    int is_unstable;
} rbtree;

rbtree* init_rbtree() {
    rbtree* tree = rbtreeMalloc(sizeof(rbtree));
    tree->root = NULL;
    tree->is_unstable = 0;
    return tree;
}

void delete_rbnode(rbnode *root) {
    if (!root) return;
    delete_rbnode(root->left);
    delete_rbnode(root->right);
    rbtreeFree(root);
}
void delete_rbtree(rbtree *tree) {
    delete_rbnode(tree->root);
    rbtreeFree(tree);
}

#define IS_LEFT  0x1
#define IS_BLACK 0x2

// Make sure prev is either the rightmost node before start or the leftmost range after start
int add_range_next_to(rbtree *tree, rbnode *prev, uintptr_t start, uintptr_t end, uint32_t data) {
// printf("Adding %lX-%lX:%hhX next to %p\n", start, end, data, prev);
    rbnode *node = rbtreeMalloc(sizeof(*node));
    if (!node) return -1;
    node->start = start;
    node->end = end;
    node->data = data;
    node->left = NULL;
    node->right = NULL;

    if (tree->is_unstable) {
        printf_log(LOG_NONE, "Warning, unstable Red-Black tree; trying to add a node anyways\n");
    }
    tree->is_unstable = 1;

    if (!tree->root) {
        node->parent = NULL;
        node->meta = IS_BLACK;
        tree->root = node;
        tree->is_unstable = 0;
        return 0;
    }

    node->parent = prev;
    if (prev->start < start) {
        prev->right = node;
        node->meta = 0;
    } else {
        prev->left = node;
        node->meta = IS_LEFT;
    }
    
    while (!(node->meta & IS_BLACK)) {
        if (!node->parent) {
            node->meta = IS_BLACK;
            tree->root = node;
            tree->is_unstable = 0;
            return 0;
        }
        if (node->parent->meta & IS_BLACK) {
            tree->is_unstable = 0;
            return 0;
        }
        if (!node->parent->parent) {
            tree->is_unstable = 0;
            return 0; // Cannot happen as the root is black, unless the tree is unstable
        }
        if (node->parent->meta & IS_LEFT) {
            if (node->parent->parent->right && !(node->parent->parent->right->meta & IS_BLACK)) {
                node->parent->meta |= IS_BLACK;
                node = node->parent->parent;
                node->meta &= ~IS_BLACK;
                node->right->meta |= IS_BLACK;
            } else {
                if (!(node->meta & IS_LEFT)) {
                    rbnode *y, *z;
                    y = node;
                    z = y->parent;
                    // ((Bz->left), Rz, ((By->left), Ry, (By->right)))
                    // y = RED, rchild of z
                    // z = RED, child of z->parent
                    // target = (((Bz->left), Rz, (By->left)), Ry, (By->right))
                    if (z->meta & IS_LEFT) {
                        z->parent->left = y;
                    } else {
                        z->parent->right = y;
                    }
                    y->meta = z->meta; // red + same side as z
                    z->meta = IS_LEFT; // red + left
                    y->parent = z->parent;
                    z->parent = y;
                    z->right = y->left;
                    y->left = z;
                    if (z->right) {
                        z->right->meta &= ~IS_LEFT;
                        z->right->parent = z;
                    }
                    node = z;
                }
                rbnode *y, *z;
                y = node->parent;
                z = y->parent;
                // (((Rnode), Ry, (By->right)), Bz, (Bz->right))
                // node = RED, lchild of y
                // y = RED, lchild of z
                // z = BLACK, child of z->parent OR ROOT
                // target = ((Rnode), By, ((By->right), Rz, (Bz->right)))
                if (z->parent) {
                    if (z->meta & IS_LEFT) {
                        z->parent->left = y;
                    } else {
                        z->parent->right = y;
                    }
                }
                y->meta = z->meta; // black + same side as z
                z->meta = 0; // red + right
                y->parent = z->parent;
                z->parent = y;
                z->left = y->right;
                y->right = z;
                if (z->left) {
                    z->left->meta |= IS_LEFT;
                    z->left->parent = z;
                }
                if (!y->parent) tree->root = y;
                tree->is_unstable = 0;
                return 0;
            }
        } else {
            if (node->parent->parent->left && !(node->parent->parent->left->meta & IS_BLACK)) {
                node->parent->meta |= IS_BLACK;
                node = node->parent->parent;
                node->meta &= ~IS_BLACK;
                node->left->meta |= IS_BLACK;
            } else {
                if (node->meta & IS_LEFT) {
                    rbnode *y, *z;
                    y = node;
                    z = y->parent;
                    // (((By->left), Ry, (By->right)), Rz, (Bz->right))
                    // y = RED, lchild of z
                    // z = RED, child of z->parent
                    // target = ((By->left), Ry, ((By->right), Rz, (Bz->right)))
                    if (z->meta & IS_LEFT) {
                        z->parent->left = y;
                    } else {
                        z->parent->right = y;
                    }
                    y->meta = z->meta; // red + same side as z
                    z->meta = 0; // red + right
                    y->parent = z->parent;
                    z->parent = y;
                    z->left = y->right;
                    y->right = z;
                    if (z->left) {
                        z->left->meta |= IS_LEFT;
                        z->left->parent = z;
                    }
                    node = z;
                }
                rbnode *y, *z;
                y = node->parent;
                z = y->parent;
                // ((Bz->left), Bz, ((By->left), Ry, (Rnode)))
                // node = RED, rchild of y
                // y = RED, rchild of z
                // z = BLACK, child of z->parent OR ROOT
                // target = (((Bz->left), Rz, (By->left)), By, (Rnode))
                if (z->parent) {
                    if (z->meta & IS_LEFT) {
                        z->parent->left = y;
                    } else {
                        z->parent->right = y;
                    }
                }
                y->meta = z->meta; // black + same side as z
                z->meta = IS_LEFT; // red + left
                y->parent = z->parent;
                z->parent = y;
                z->right = y->left;
                y->left = z;
                if (z->right) {
                    z->right->meta &= ~IS_LEFT;
                    z->right->parent = z;
                }
                if (!y->parent) tree->root = y;
                tree->is_unstable = 0;
                return 0;
            }
        }
    }
    tree->is_unstable = 0;
    return -1; // unreachable
}
int add_range(rbtree *tree, uintptr_t start, uintptr_t end, uint32_t data) {
// printf("add_range\n");
    rbnode *cur = tree->root, *prev = NULL;
    while (cur) {
        prev = cur;
        if (cur->start < start) cur = cur->right;
        else cur = cur->left;
    }
    return add_range_next_to(tree, prev, start, end, data);
}

rbnode *find_addr(rbtree *tree, uintptr_t addr) {
    rbnode *node = tree->root;
    while (node) {
        if ((node->start <= addr) && (node->end > addr)) return node;
        if (addr < node->start) node = node->left;
        else node = node->right;
    }
    return NULL;
}

// node must be a valid node in the tree
int remove_node(rbtree *tree, rbnode *node) {
// printf("Removing %p\n", node); print_rbtree(tree); fflush(stdout);
    if (tree->is_unstable) {
        printf_log(LOG_NONE, "Warning, unstable Red-Black tree; trying to add a node anyways\n");
    }
    tree->is_unstable = 1;

    if (node->left && node->right) {
        // Swap node and its successor
        // Do NOT free the successor as a reference to it can exist
        rbnode *cur = node->right, *prev;
        while (cur) {
            prev = cur;
            cur = cur->left;
        }
        // Swap the position of node and prev != node
        uint8_t tmp8 = node->meta;
        node->meta = prev->meta;
        prev->meta = tmp8;
        prev->left = node->left;
        node->left = NULL;
        if (prev->left) prev->left->parent = prev;
        if (node->meta & IS_LEFT) {
            cur = node->parent;
            node->parent = prev->parent;
            prev->parent = cur;
            cur = node->right;
            node->right = prev->right;
            prev->right = cur;
            if (cur) cur->parent = prev;
        } else {
            node->right = prev->right;
            prev->right = node;
            prev->parent = node->parent;
            node->parent = prev;
        }
        if (node->right) node->right->parent = node; // Should be overriden later
        if (!prev->parent) {
            tree->root = prev; // prev is already black
        } else if (prev->meta & IS_LEFT) {
            prev->parent->left = prev;
        } else {
            prev->parent->right = prev;
        }
    }
    rbnode *child = node->left ? node->left : node->right, *parent = node->parent;
    if (child) {
        child->parent = parent;
        if (!parent) {
            tree->root = child;
            child->meta |= IS_BLACK; // Needs to be an or
            tree->is_unstable = 0;
            return 0;
        } else if (node->meta & IS_LEFT) {
            child->meta |= IS_LEFT;
            parent->left = child;
        } else {
            child->meta &= ~IS_LEFT;
            parent->right = child;
        }
    } else {
        if (!parent) {
            tree->root = NULL;
            rbtreeFree(node);
            tree->is_unstable = 0;
            return 0;
        } else if (node->meta & IS_LEFT) {
            parent->left = NULL;
        } else {
            parent->right = NULL;
        }
    }
    // Node has been removed, now to fix the tree
    if (!(node->meta & IS_BLACK)) {
        rbtreeFree(node);
        tree->is_unstable = 0;
        return 0;
    }
    rbtreeFree(node);

    // Add a black node before child
    // Notice that the sibling cannot be NULL.
    while (parent && (!child || (child->meta & IS_BLACK))) {
        if ((child && child->meta & IS_LEFT) || (!child && !parent->left)) {
            node = parent->right;
            if (!(node->meta & IS_BLACK)) {
                // rotate ===
                rbnode *y, *z;
                y = node;
                z = parent;
                // ((Bchild), Bz, ((y->left), Ry, (y->right)))
                // y = RED, rchild of z
                // z = BLACK, child of z->parent OR ROOT
                // target = (((Bchild), Rz, (y->left)), By, (y->right))
                if (z->parent) {
                    if (z->meta & IS_LEFT) {
                        z->parent->left = y;
                    } else {
                        z->parent->right = y;
                    }
                } else {
                    tree->root = y;
                }
                y->meta = z->meta; // black + same side as z
                z->meta = IS_LEFT; // red + left
                y->parent = z->parent;
                z->parent = y;
                z->right = y->left;
                y->left = z;
                if (z->right) {
                    z->right->meta &= ~IS_LEFT;
                    z->right->parent = z;
                }
                // ===
                node = parent->right;
            }
            if (node->right && !(node->right->meta & IS_BLACK)) {
                case4_l: {
                rbnode *y, *z;
                y = node;
                z = parent;
                // ((Bchild), ?z, ((?y->left), By, (Ry->right)))
                // y = BLACK, rchild of z
                // z = ?, child of z->parent OR ROOT
                // target = (((Bchild), Bz, (?y->left)), ?y, (By->right))
                if (z->parent) {
                    if (z->meta & IS_LEFT) {
                        z->parent->left = y;
                    } else {
                        z->parent->right = y;
                    }
                }
                y->meta = z->meta; // same color as z + same side as z
                z->meta = IS_BLACK | IS_LEFT; // black + left
                y->parent = z->parent;
                z->parent = y;
                z->right = y->left;
                y->left = z;
                if (z->right) {
                    z->right->meta &= ~IS_LEFT;
                    z->right->parent = z;
                }
                if (!y->parent) tree->root = y;
                node->right->meta |= IS_BLACK;
                tree->is_unstable = 0;
                return 0; }
            } else if (!node->left || (node->left->meta & IS_BLACK)) {
                // case2_l:
                child = parent; // Remember that child can be NULL
                parent = child->parent;
                node->meta &= ~IS_BLACK;
            } else {
                // case3_l:
                rbnode *y, *z;
                y = node->left;
                z = node;
                // (((y->left), Ry, (y->right)), Bz, (Bz->right))
                // y = RED, rchild of z
                // z = BLACK, child of z->parent
                // target = ((y->left), By, ((y->right), Rz, (z->right)))
                if (z->meta & IS_LEFT) {
                    z->parent->left = y;
                } else {
                    z->parent->right = y;
                }
                y->meta = z->meta; // black + same side as z
                z->meta = 0; // red + right
                y->parent = z->parent;
                z->parent = y;
                z->left = y->right;
                y->right = z;
                if (z->left) {
                    z->left->meta |= IS_LEFT;
                    z->left->parent = z;
                }
                node = y;
                goto case4_l;
            }
        } else {
            node = parent->left;
            if (!(node->meta & IS_BLACK)) {
                // rotate ===
                rbnode *y, *z;
                y = node;
                z = parent;
                // (((y->left), Ry, (y->right)), Bz, (Bchild))
                // y = RED, lchild of z
                // z = BLACK, child of z->parent OR ROOT
                // target = ((y->left), By, ((y->right), Rz, (Bchild)))
                if (z->parent) {
                    if (z->meta & IS_LEFT) {
                        z->parent->left = y;
                    } else {
                        z->parent->right = y;
                    }
                }
                y->meta = z->meta; // black + same side as z
                z->meta = 0; // red + right
                y->parent = z->parent;
                z->parent = y;
                z->left = y->right;
                y->right = z;
                if (z->left) {
                    z->left->meta |= IS_LEFT;
                    z->left->parent = z;
                }
                if (!y->parent) tree->root = y;
                // ===
                node = parent->left;
            }
            if (node->left && !(node->left->meta & IS_BLACK)) {
                case4_r: {
                rbnode *y, *z;
                y = node;
                z = y->parent;
                // (((?y->left), By, (Ry->right)), ?z, (Bchild))
                // y = BLACK, rchild of z
                // z = ?, child of z->parent OR ROOT
                // target = ((?y->left), ?y, ((Ry->right), Bz, (Bchild)))
                if (z->parent) {
                    if (z->meta & IS_LEFT) {
                        z->parent->left = y;
                    } else {
                        z->parent->right = y;
                    }
                }
                y->meta = z->meta; // same color as z + same side as z
                z->meta = IS_BLACK; // black + right
                y->parent = z->parent;
                z->parent = y;
                z->left = y->right;
                y->right = z;
                if (z->left) {
                    z->left->meta |= IS_LEFT;
                    z->left->parent = z;
                }
                if (!y->parent) tree->root = y;
                node->left->meta |= IS_BLACK;
                tree->is_unstable = 0;
                return 0; }
            } else if (!node->right || (node->right->meta & IS_BLACK)) {
                // case2_r:
                child = parent;
                parent = child->parent;
                node->meta &= ~IS_BLACK;
            } else {
                // case3_r:
                rbnode *y, *z;
                y = node->right;
                z = node;
                // ((Bz->left), Bz, ((y->left), Ry, (y->right)))
                // y = RED, rchild of z
                // z = BLACK, child of z->parent
                // target = (((Bz->left), Rz, (y->left)), By, (y->right))
                if (z->meta & IS_LEFT) {
                    z->parent->left = y;
                } else {
                    z->parent->right = y;
                }
                y->meta = z->meta; // black + same side as z
                z->meta = IS_LEFT; // red + left
                y->parent = z->parent;
                z->parent = y;
                z->right = y->left;
                y->left = z;
                if (z->right) {
                    z->right->meta &= ~IS_LEFT;
                    z->right->parent = z;
                }
                node = y;
                goto case4_r;
            }
        }
    }
    if (child)
        child->meta |= IS_BLACK;
    tree->is_unstable = 0;
    return 0;
}

rbnode *first_node(rbtree *tree) {
    rbnode *node = tree->root, *prev = node;
    while (node) {
        prev = node;
        node = node->left;
    }
    return prev;
}
rbnode *pred_node(rbnode *node) {
    if (!node) return NULL;
    if (node->left) {
        node = node->left;
        while (node->right) node = node->right;
        return node;
    } else {
        while (node->parent && node->meta & IS_LEFT) node = node->parent;
        return node->parent;
    }
}
rbnode *succ_node(rbnode *node) {
    if (!node) return NULL;
    if (node->right) {
        node = node->right;
        while (node->left) node = node->left;
        return node;
    } else {
        while (node->parent && !(node->meta & IS_LEFT)) node = node->parent;
        return node->parent;
    }
}

uint32_t rb_get(rbtree *tree, uintptr_t addr) {
    rbnode *node = find_addr(tree, addr);
    if (node) return node->data;
    else return 0;
}

int rb_get_end(rbtree* tree, uintptr_t addr, uint32_t* val, uintptr_t* end) {
    rbnode *node = tree->root, *next = NULL;
    while (node) {
        if ((node->start <= addr) && (node->end > addr)) {
            *val = node->data;
            *end = node->end;
            return 1;
        }
        if (node->end <= addr) {
            node = node->right;
        } else {
            next = node;
            node = node->left;
        }
    }
    *val = 0;
    if (next) {
        *end = next->start;
    } else {
        *end = (uintptr_t)-1;
    }
    return 0;
}

int rb_set(rbtree *tree, uintptr_t start, uintptr_t end, uint32_t data) {
// printf("rb_set( "); print_rbtree(tree); printf(" , 0x%lX, 0x%lX, %hhu);\n", start, end, data); fflush(stdout);
dynarec_log(LOG_DEBUG, "set 0x%lX, 0x%lX, 0x%x\n", start, end, data);
    if (!tree->root) {
        return add_range(tree, start, end, data);
    }
    
    rbnode *node = tree->root, *prev = NULL, *last = NULL;
    while (node) {
        if (node->start < start) {
            prev = node;
            node = node->right;
        } else if (node->start == start) {
            if (node->left) {
                prev = node->left;
                while (prev->right) prev = prev->right;
            }
            if (node->right) {
                last = node->right;
                while (last->left) last = last->left;
            }
            break;
        } else {
            last = node;
            node = node->left;
        }
    }

    // prev is the largest node starting strictly before start, or NULL if there is none
    // node is the node starting exactly at start, or NULL if there is none
    // last is the smallest node starting strictly after start, or NULL if there is none
    // Note that prev may contain start

    if (prev && (prev->end >= start) && (prev->data == data)) {
        // Merge with prev
        if (end <= prev->end) return 0; // Nothing to do!
        
        if (node && (node->end > end)) {
            node->start = end;
            prev->end = end;
            return 0;
        } else if (node && (node->end == end)) {
            remove_node(tree, node);
            prev->end = end;
            return 0;
        } else if (node) {
            remove_node(tree, node);
        }
        while (last && (last->start < end) && (last->end <= end)) {
            // Remove the entire node
            node = last;
            last = succ_node(last);
            remove_node(tree, node);
        }
        if (last && (last->start <= end) && (last->data == data)) {
            // Merge node and last
            prev->end = last->end;
            remove_node(tree, last);
            return 0;
        }
        if (last && (last->start < end)) last->start = end;
        prev->end = end;
        return 0;
    } else if (prev && (prev->end > start)) {
        if (prev->end > end) {
            // Split in three
            // Note that here, succ(prev) = last and node = NULL
            int ret;
            ret = add_range_next_to(tree, prev->right ? last : prev, end, prev->end, prev->data);
            ret = ret ? ret : add_range_next_to(tree, prev->right ? succ_node(prev) : prev, start, end, data);
            prev->end = start;
            return ret;
        }
        // Cut prev and continue
        prev->end = start;
    }

    if (node) {
        // Change node
        if (node->end >= end) {
            if (node->data == data) return 0; // Nothing to do!
            // Cut node
            if (node->end > end) {
                int ret = add_range_next_to(tree, node->right ? last : node, end, node->end, node->data);
                node->end = end;
                node->data = data;
                return ret;
            }
            // Fallthrough
        }
        
        // Overwrite and extend node
        while (last && (last->start < end) && (last->end <= end)) {
            // Remove the entire node
            prev = last;
            last = succ_node(last);
            remove_node(tree, prev);
        }
        if (last && (last->start <= end) && (last->data == data)) {
            // Merge node and last
            remove_node(tree, node);
            last->start = start;
            return 0;
        }
        if (last && (last->start < end)) last->start = end;
        if (node->end < end) node->end = end;
        node->data = data;
        return 0;
    }

    while (last && (last->start < end) && (last->end <= end)) {
        // Remove the entire node
        node = last;
        last = succ_node(last);
        remove_node(tree, node);
    }
    if (!last) {
        // Add a new node next to prev, the largest node of the tree
        // It exists since the tree is nonempty
        return add_range_next_to(tree, prev, start, end, data);
    }
    if ((last->start <= end) && (last->data == data)) {
        // Extend
        last->start = start;
        return 0;
    } else if (last->start < end) {
        // Cut
        last->start = end;
    }
    // Probably 'last->left ? prev : last' is enough
    return add_range_next_to(tree, last->left ? pred_node(last) : last, start, end, data);
}

int rb_unset(rbtree *tree, uintptr_t start, uintptr_t end) {
// printf("rb_unset( "); print_rbtree(tree); printf(" , 0x%lX, 0x%lX);\n", start, end); fflush(stdout);
dynarec_log(LOG_DEBUG, "rb_unset(tree, 0x%lX, 0x%lX);\n", start, end);
    if (!tree->root) return 0;

    rbnode *node = tree->root, *prev = NULL, *next = NULL;
    while (node) {
        if (node->start < start) {
            prev = node;
            node = node->right;
        } else if (node->start == start) {
            if (node->left) {
                prev = node->left;
                while (prev->right) prev = prev->right;
            }
            if (node->right) {
                next = node->right;
                while (next->left) next = next->left;
            }
            break;
        } else {
            next = node;
            node = node->left;
        }
    }

    if (node) {
        if (node->end > end) {
            node->start = end;
            return 0;
        } else if (node->end == end) {
            remove_node(tree, node);
            return 0;
        } else {
            remove_node(tree, node);
        }
    } else if (prev && (prev->end > start)) {
        if (prev->end > end) {
            // Split prev
            int ret = add_range_next_to(tree, prev->right ? next : prev, end, prev->end, prev->data);
            prev->end = start;
            return ret;
        } else if (prev->end == end) {
            prev->end = start;
            return 0;
        } // else fallthrough
    }
    while (next && (next->start < end) && (next->end <= end)) {
        // Remove the entire node
        node = next;
        next = succ_node(next);
        remove_node(tree, node);
    }
    if (next && (next->start < end)) {
        // next->end > end: cut the node
        next->start = end;
    }
    return 0;
}

uintptr_t rb_get_righter(rbtree* tree)
{
dynarec_log(LOG_DEBUG, "rb_get_righter(tree);\n");
    if (!tree->root) return 0;

    rbnode *node = tree->root;
    while (node) {
        if(!node->right)
            return node->start;
        node = node->right;
    }
    return 0;
}

#include <stdio.h>
void print_rbnode(const rbnode *node, unsigned depth, uintptr_t minstart, uintptr_t maxend, unsigned *bdepth) {
    if (!node) {
        if (!*bdepth || *bdepth == depth + 1) {
            *bdepth = depth + 1;
            printf("[%u]", depth);
        } else
            printf("<invalid black depth %u>", depth);
        return;
    }
    if (node->start < minstart) {
        printf("<invalid start>");
        return;
    }
    if (node->end > maxend) {
        printf("<invalid end>");
        return;
    }
    printf("(");
    if (node->left && !(node->left->meta & IS_LEFT)) {
        printf("<invalid meta>");
    } else if (node->left && (node->left->parent != node)) {
        printf("<invalid parent %p instead of %p>", node->left->parent, node);
    } else if (node->left && !(node->meta & IS_BLACK) && !(node->left->meta & IS_BLACK)) {
        printf("<invalid red-red node> ");
        print_rbnode(node->left, depth + ((node->meta & IS_BLACK) ? 1 : 0), minstart, node->start, bdepth);
    } else {
        print_rbnode(node->left, depth + ((node->meta & IS_BLACK) ? 1 : 0), minstart, node->start, bdepth);
    }
    printf(", (%c/%p) %lX-%lX: %hhu, ", node->meta & IS_BLACK ? 'B' : 'R', node, node->start, node->end, node->data);
    if (node->right && (node->right->meta & IS_LEFT)) {
        printf("<invalid meta>");
    } else if (node->right && (node->right->parent != node)) {
        printf("<invalid parent %p instead of %p>", node->right->parent, node);
    } else if (node->right && !(node->meta & IS_BLACK) && !(node->right->meta & IS_BLACK)) {
        printf("<invalid red-red node> ");
        print_rbnode(node->right, depth + ((node->meta & IS_BLACK) ? 1 : 0), node->end, maxend, bdepth);
    } else {
        print_rbnode(node->right, depth + ((node->meta & IS_BLACK) ? 1 : 0), node->end, maxend, bdepth);
    }
    printf(")");
}
void print_rbtree(const rbtree *tree) {
    if (!tree) {
        printf("<NULL>\n");
        return;
    }
    if (tree->root && tree->root->parent) {
        printf("Root has parent\n");
        return;
    }
    if (tree->root && !(tree->root->meta & IS_BLACK)) {
        printf("Root is red\n");
        return;
    }
    unsigned bdepth = 0;
    print_rbnode(tree->root, 0, 0, (uintptr_t)-1, &bdepth);
    printf("\n");
}

#ifdef RBTREE_TEST
int main() {
    rbtree* tree = init_rbtree();
    print_rbtree(tree); fflush(stdout);
    /*int ret;
    ret = rb_set(tree, 0x43, 0x44, 0x01);
    printf("%d; ", ret); print_rbtree(tree); fflush(stdout);
    ret = rb_set(tree, 0x42, 0x43, 0x01);
    printf("%d; ", ret); print_rbtree(tree); fflush(stdout);
    ret = rb_set(tree, 0x41, 0x42, 0x01);
    printf("%d; ", ret); print_rbtree(tree); fflush(stdout);
    ret = rb_set(tree, 0x40, 0x41, 0x01);
    printf("%d; ", ret); print_rbtree(tree); fflush(stdout);
    ret = rb_set(tree, 0x20, 0x40, 0x03);
    printf("%d; ", ret); print_rbtree(tree); fflush(stdout);
    ret = rb_set(tree, 0x10, 0x20, 0x01);
    printf("%d; ", ret); print_rbtree(tree); fflush(stdout);

    uint32_t val = rb_get(tree, 0x33);
    printf("0x33 has attribute %hhu\n", val); fflush(stdout);*/
    /* rbnode *node = find_addr(tree, 0x33);
    printf("0x33 is at %p: ", node); print_rbnode(node, 0); printf("\n"); fflush(stdout);
    ret = remove_node(tree, node);
    printf("%d; ", ret); print_rbtree(tree); fflush(stdout);
    node = find_addr(tree, 0x20);
    printf("0x20 is at %p\n", node);
    node = find_addr(tree, 0x1F);
    printf("0x1F is at %p: ", node); print_rbnode(node, 0); printf("\n"); fflush(stdout);
    ret = remove_node(tree, node);
    printf("%d; ", ret); print_rbtree(tree); fflush(stdout); */
    /* ret = rb_set(tree, 0x15, 0x42, 0x00);
    printf("%d; ", ret); print_rbtree(tree); fflush(stdout); */
    /*rb_unset(tree, 0x15, 0x42);
    print_rbtree(tree); fflush(stdout);*/
    
    // tree->root = node27; print_rbtree(tree); fflush(stdout);
    // rb_set(tree, 2, 3, 1); print_rbtree(tree); fflush(stdout);
    // add_range_next_to(tree, node24, 0x0E7000, 0x0E8000, 69); print_rbtree(tree); fflush(stdout);
    // print_rbtree(tree); fflush(stdout);
    // uint32_t val = rb_get(tree, 0x11003000);
    // printf("0x11003000 has attribute %hhu\n", val); fflush(stdout);
    // remove_node(tree, node0); print_rbtree(tree); fflush(stdout);
    // add_range_next_to(tree, node1, 0x0E7000, 0x0E8000, 69); print_rbtree(tree); fflush(stdout);
rb_set(tree, 0x130000, 0x140000, 7);
    print_rbtree(tree); fflush(stdout);
rb_set(tree, 0x141000, 0x142000, 135);
    print_rbtree(tree); fflush(stdout);
rb_set(tree, 0x140000, 0x141000, 135);
    print_rbtree(tree); fflush(stdout);
rb_set(tree, 0x140000, 0x141000, 7);
    print_rbtree(tree); fflush(stdout);
rb_set(tree, 0x140000, 0x141000, 135);
    print_rbtree(tree); fflush(stdout);
    uint32_t val = rb_get(tree, 0x141994); printf("0x141994 has attribute %hhu\n", val); fflush(stdout);
    delete_rbtree(tree);
}
#endif

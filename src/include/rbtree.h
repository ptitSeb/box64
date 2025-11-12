/*
 * The primary function of red-black trees in Box64 is to provide an O(log n) method for managing memory mappings.
 * This provides a mechanism that allows retrieving the memory range or associated data based on an address.
 * Unlike standard red-black trees, each node here includes two additional fields, "start" and "end," to denote a specific memory range.
 *
 * Box64 currently uses seven red-black trees, each serving distinct purposes:
 *
 * 1. memprot:
 *    Tracks memory protection flags (PROT_READ, PROT_WRITE, PROT_EXEC, PROT_CUSTOM) for each memory range.
 *
 * 2. mapallmem:
 *    Mirrors `/proc/self/maps`, tracking all memory allocations to prevent overlaps.
 *    Data field indicates allocation type:
 *      - MEM_ALLOCATED - Box64's custom allocator and direct mmap64() syscalls
 *      - MEM_RESERVED - reserved for box32_dynarec_mmap
 *      - MEM_MMAP - mmap from emulated code 
 *      - MEM_BOX - mmap from Box64 native code
 *      - MEM_STACK - thread stacks
 *      - MEM_EXTERNAL - memory found in /proc/self/maps not allocated by Box64
 *      - MEM_ELF - ELF binaries and shared libraries
 *   
 * 3. blockstree:
 *    Contains memory ranges for a free-list (blocklist_s). Each node represents a different free-list.
 *    The data field stores the index of that free-list in the array `p_blocks` (i.e., the array that holds each free-list pointer).
 *
 *    Free-list structure managed by each blocklist_t:
 *     ┌────────┬────────┬───────────────────────────────┬────────┬────────┐
 *     │ m.prev │ m.next │            PAYLOAD            │ n.prev │ n.next │
 *     │ 0      │ offs   │  (allocsize - 2·sizeof(mark)) │  offs  │  0     │
 *     └────────┴────────┴───────────────────────────────┴────────┴────────┘
 *     ↑                                                                   ↑
 *     p (free-list start)                                      p + allocsize (free-list end)
 *
 * 4. db_sizes:
 *    Tracks, for each dynablock size, how many dynablocks of exactly that size currently exist (per-size count).
 *
 * 5. envmap:
 *    Maps each live memory range to its mapping_t, which contains:
 *      - the file’s lowercase name,
 *      - a pointer to that file’s per-file box64env_t,
 *      - and the base address;
 *    indicating which module owns that range.
 *
 * 6. rbt_dynmem:
 *    Maps dynarec memory ranges to chunk pointers (blocklist_t*) for O(log n) lookup.
 *    Structure: Each mmaplist_t contains a dynamically-sized array of chunk pointers (blocklist_t**).
 *
 *    [ mmaplist0 ] → [ mmaplist1 ] → [ mmaplist2 ] → …
 *          │              │              │
 *      chunks[0]      chunks[0]      chunks[0]    (each chunk[i] is a blocklist_t*)
 *      chunks[1]      chunks[1]      chunks[1]
 *        ...            ...            ...
 *      chunks[n-1]    chunks[n-1]    chunks[n-1]  (array grows dynamically as needed)
 *
 * 7. volatileRanges:
 *    Tracks every “volatile” region inside a loaded PE module.
 *
 * Before the introduction of the red-black tree in Box64, the rationale for memory management was a sparse array, which takes O(n) complexity for accessing data.
 * After transitioning from a sparse array to a red-black tree, the memory usage has decreased slightly for processes that consumed a lot of RAM (for example, Steam uses about 100 MB less memory, and each Wine process uses about 15 MB less).
 *
 * Note that each rbnode is allocated via customMalloc. Since each rbtree node is 56 bytes,
 * it's handled by the bitmap allocator (map64_customMalloc) which provides 64-byte blocks,
 * resulting in 8 bytes of internal fragmentation per node (14% overhead).
 */

#ifndef RBTREE_H
#define RBTREE_H

#include <stdint.h>

typedef struct rbtree rbtree_t;

/** 
 * rbtree_init() - Initializes a new red-black tree.
 * @name: The name of the red-black tree. If null, the default name "(rbtree)" is assigned.
 *
 * This function allocates memory for a new red-black tree and initializes its properties.
 * Return: A pointer to the newly created red-black tree.
 */
rbtree_t* rbtree_init(const char* name);

/** 
 * rbtree_delete() - Deletes an entire red-black tree.
 * @tree: Pointer to the red-black tree to delete.
 *
 * This function recursively deletes all nodes in the red-black tree starting from the root 
 * and then frees the memory of the tree structure itself via delete_rbnode().
 */
void rbtree_delete(rbtree_t* tree);

/** 
 * rb_get() - Retrieves data associated with a specific address in the red-black tree.
 * @tree: Pointer to the red-black tree from which data is to be retrieved.
 * @addr: The memory address used as a key to find the corresponding node in the tree.
 *
 * This function searches the red-black tree for a node that corresponds to the specified address.
 * Return: The data associated with the address if found; otherwise, 0.
 */
uint32_t rb_get(rbtree_t* tree, uintptr_t addr);

/** 
 * rb_get_64() - Retrieves data associated with a specific address in the red-black tree.
 * @tree: Pointer to the red-black tree from which data is to be retrieved.
 * @addr: The memory address used as a key to find the corresponding node in the tree.
 *
 * This function searches the red-black tree for a node that corresponds to the specified address.
 * Return: The 64bits data associated with the address if found; otherwise, 0.
 */
uint64_t rb_get_64(rbtree_t* tree, uintptr_t addr);

/** 
 * rb_get_end() - Searches for a node within a specified address range in a red-black tree and retrieves its data and end address.
 * @tree: Pointer to the red-black tree to be searched.
 * @addr: The address to search for within the nodes of the red-black tree.
 * @val: Pointer to store the data of the node that contains the address if found.
 * @end: Pointer to store the end address of the node that contains the address, or the start of the next node if not found, or UINTPTR_MAX if no next node exists.
 *
 * This function traverses the red-black tree starting from the root, searching for a node where the 'addr' falls between the node's start and end addresses (exclusive of end).
 * If such a node is found, the function stores the node's data in '*val' and the node's end address in '*end', then returns 1 to indicate success.
 * If no such node is found, the function sets '*val' to 0 and '*end' to the start address of the next node in the tree structure or to UINTPTR_MAX if there is no subsequent node.
 * Return: 1 if a node containing the address is found, otherwise 0.
 */
int rb_get_end(rbtree_t* tree, uintptr_t addr, uint32_t* val, uintptr_t* end);

/** 
 * rb_get_end_64() - Searches for a node within a specified address range in a red-black tree and retrieves its data and end address.
 * @tree: Pointer to the red-black tree to be searched.
 * @addr: The address to search for within the nodes of the red-black tree.
 * @val: Pointer to store the data of the node that contains the address if found.
 * @end: Pointer to store the end address of the node that contains the address, or the start of the next node if not found, or UINTPTR_MAX if no next node exists.
 *
 * This function traverses the red-black tree starting from the root, searching for a node where the 'addr' falls between the node's start and end addresses (exclusive of end).
 * If such a node is found, the function stores the node's data in '*val' and the node's end address in '*end', then returns 1 to indicate success.
 * If no such node is found, the function sets '*val' to 0 and '*end' to the start address of the next node in the tree structure or to UINTPTR_MAX if there is no subsequent node.
 * Return: 1 if a node containing the address is found, otherwise 0.
 */
int rb_get_end_64(rbtree_t* tree, uintptr_t addr, uint64_t* val, uintptr_t* end);

/**
 * rb_set() - Set an address range in a red-black tree.
 * @tree: Pointer to the red-black tree where the address range will be set.
 * @start: The starting address of the range to be set.
 * @end: The ending address of the range to be set.
 * @data: The data value to associate with the address range.
 *
 * This function adds a new address range with associated data to the red-black tree. 
 * However, it is not always necessary to create a new node for each new address.
 * If the range is adjacent to the existing nodes with the same data, the existing nodes will be extended to contain the new address.
 *       +---------+---------+---------+ +---------+---------+
 *       |            data A           | |       data A      |
 *       +---------+---------+---------+ +---------+---------+
 * 
 *                                ||
 *                               \||/ 
 *                                \/
 * 
 *       +---------+---------+---------+---------+---------+
 *       |                      data A                     |
 *       +---------+---------+---------+---------+---------+
 * 
 * If the range overlaps with existing nodes, it will merge or modify nodes to maintain non-overlapping, contiguous ranges.
 * This includes extending, splitting, or merging nodes based on the overlap conditions and data consistency. 
 * It handles multiple edge cases:
 * 1. Overlap with same data: The existing node will be extended.
 * 
 * (Case 1: Partial Overlap)
 *       +---------+---------+---------+---------+---------+
 *       |                      data A                     |
 *       +---------+---------+---------+---------+---------+
 *                           ^.....      overlap      .....^
 *                           +---------+---------+---------+---------+---------+
 *                           |                      data A                     |
 *                           +---------+---------+---------+---------+---------+
 * 
 *                                ||
 *                               \||/ Extend A
 *                                \/
 *   
 *       +---------+---------+---------+---------+---------+---------+---------+
 *       |                                data A                               |
 *       +---------+---------+---------+---------+---------+---------+---------+
 * 
 * (Case 2: Overlap on Both Ends)
 *       +---------+---------+---------+---------+---------+
 *       |                     data A                      |
 *       +---------+---------+---------+---------+---------+
 *                 ^.....      overlap      .....^
 *                 +---------+---------+---------+
 *                 |           data A            |
 *                 +---------+---------+---------+
 * 
 *                                ||
 *                               \||/ Do nothing
 *                                \/
 *   
 *       +---------+---------+---------+---------+---------+
 *       |                     data A                      |
 *       +---------+---------+---------+---------+---------+
 * 
 * 
 * 2. Overlap with different data: The overlapped part will be overwritten with the data from the new address range.
 *    The following graph shows that memory B is going to be added to the red-black tree where memory B overlaps with the existing node memory A.
 * 
 * (Case 1: Overlap on Both Ends)
 *       +---------+---------+---------+---------+---------+
 *       |                     data A                      |
 *       +---------+---------+---------+---------+---------+
 *                 ^.....      overlap      .....^
 *                 +---------+---------+---------+
 *                 |           data B            |
 *                 +---------+---------+---------+
 * 
 *                                ||
 *                               \||/ Split the existing node into three new nodes
 *                                \/
 *   
 *       +---------+ +---------+---------+---------+ +---------+
 *       |  data A | |            data B           | |  data A |
 *       +---------+ +---------+---------+---------+ +---------+
 * 
 * (Case 2: Partial Overlap)
 *       +---------+---------+---------+---------+---------+
 *       |                      data A                     |
 *       +---------+---------+---------+---------+---------+
 *                           ^.....      overlap      .....^
 *                           +---------+---------+---------+---------+---------+
 *                           |                      data B                     |
 *                           +---------+---------+---------+---------+---------+
 * 
 *                                ||
 *                               \||/ Adjust A by changing the pointers to exclude the segments that overlap with B
 *                                \/
 *   
 *       +---------+---------+ +---------+---------+---------+---------+---------+
 *       |       data A      | |                      data B                     | 
 *       +---------+---------+ +---------+---------+---------+---------+---------+
 * 
 * (Case 3: Complete Encapsulation)
 *                 +---------+---------+---------+
 *                 |            data A           |
 *                 +---------+---------+---------+
 *                                
 *                                ||
 *                               \||/ Remove A entirely
 *                                \/
 *       +---------+---------+---------+---------+---------+
 *       |                      data B                     |
 *       +---------+---------+---------+---------+---------+
 *                                         
 * The function ensures the tree remains balanced and correctly represents the address space with minimal nodes.
 *
 * Return: 0 on success, or -1 on failure. 
 */
int rb_set(rbtree_t* tree, uintptr_t start, uintptr_t end, uint32_t data);


/**
 * rb_set_64() - Set an address range in a red-black tree.
 * @tree: Pointer to the red-black tree where the address range will be set.
 * @start: The starting address of the range to be set.
 * @end: The ending address of the range to be set.
 * @data: The data value to associate with the address range.
 *
 * This function adds a new address range with associated data to the red-black tree. 
 */

int rb_set_64(rbtree_t* tree, uintptr_t start, uintptr_t end, uint64_t data);

/**
 * rb_unset() - Removes a range of values from the red-black tree.
 * @tree: Pointer to the red-black tree.
 * @start: The start address of the range to remove.
 * @end: The end address of the range to remove.
 *
 * This function removes or adjusts nodes in the red-black tree that overlap with the specified
 * range [start, end). It traverses the tree to find overlapping nodes, removes entire nodes
 * that are completely within the range, and modifies nodes that partially overlap by adjusting
 * their start or end values accordingly.
 *
 * Return: 0 on success.
 */
int rb_unset(rbtree_t* tree, uintptr_t start, uintptr_t end);

/**
 * rb_inc() - Increment by 1 an address range in a red-black tree. Create the node if needed.
 * @tree: Pointer to the red-black tree where the address range will be set.
 * @start: The starting address of the range to be set.
 * @end: The ending address of the range to be set.
 *
 * Return: The new value for the node.
 */
uint64_t rb_inc(rbtree_t* tree, uintptr_t start, uintptr_t end);

/**
 * rb_dec() - Decrement by 1 an address range in a red-black tree. Do not create the node if absent. Delete the node if data == 0.
 * @tree: Pointer to the red-black tree where the address range will be set.
 * @start: The starting address of the range to be set.
 * @end: The ending address of the range to be set.
 *
 * Return: The new value for the node (or 0 if absent / removed).
 */
uint64_t rb_dec(rbtree_t* tree, uintptr_t start, uintptr_t end);

/**
 * rb_get_rightmost() - Retrieves the start value of the right-most node in a red-black tree.
 * @tree: Pointer to the red-black tree whose right-most node's start value is to be retrieved.
 *
 * This function traverses the red-black tree from the root to the right-most node, which is the node
 * with the highest key value in the tree.
 * Return: The start value of the right-most node if the tree is not empty; otherwise, 0.
 */
uintptr_t rb_get_rightmost(rbtree_t* tree);

/**
 * rb_get_leftmost() - Retrieves the start value of the left-most node in a red-black tree.
 * @tree: Pointer to the red-black tree whose left-most node's start value is to be retrieved.
 *
 * This function traverses the red-black tree from the root to the left-most node, which is the node
 * with the lowest key value in the tree.
 * Return: The start value of the left-most node if the tree is not empty; otherwise, 0.
 */
uintptr_t rb_get_leftmost(rbtree_t* tree);

/**
 * @brief Print a rbtree (for debugging purpose)
 * 
 * @param tree 
 */
void rbtree_print(const rbtree_t* tree);

#endif // RBTREE_H

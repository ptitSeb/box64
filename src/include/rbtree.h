/*
 * The primary function of red-black trees in Box64 is to provide a fast and efficient method (O(log(n))) for managing memory mappings. 
 * Each red-black tree node includes two additional pointers, "start" and "end," to denote a specific memory range. 
 * When a range of memory is mapped from the system, it is recorded in red-black trees based on the type and characteristics of that memory range.
 *
 * Box64 utilizes four red-black trees, each serving distinct purposes:
 * 1. memprot: Manages memory protection settings where each node's data field represents the permissions of the memory range. 
 *    The "rb_set" function within this tree can set or change permissions for a specified range.
 * 2. mapallmem: Tracks all memory mappings. Its nodes' data field can indicate whether the memory is included in this mapping.
 * 3. mmapmem: Similar to "mapallmem" but specifically for memory mapped through the mmap system call. 
 *    The nodes' data fields here differentiate whether the memory is exclusively mapped by mmap.
 * 4. blockstree: Specifically contains memory ranges for blocks, where each node represents a different block. 
 *    The "data" field in this tree represents the index of the block in the "blocklist_s" array "p_blocks".
 *
 * Currently, there is an overlap between "mapallmem" and "mmapmem", where if memory is mapped in "mmapmem", it is also considered mapped in "mapallmem". 
 * If "mapallmem" and "mmapmem" were to be merged in the future, the "data" field could then take values indicating:
 * - 0: Memory is not mapped.
 * - 1: Memory is only mapped in "mapallmem".
 * - 2: Memory is mapped in both "mapallmem" and "mmapmem".
 * 
 * Before the introduction of the red-black tree in Box64, the rationale for memory management was a sparse array, which takes O(n) complexity for accessing data.
 * After transitioning from a sparse array to a red-black tree, the memory usage has decreased slightly for processes that consumed a lot of RAM (for example, Steam uses about 100 MB less memory, and each Wine process uses about 15 MB less).
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
 * rb_get_righter() - Retrieves the start value of the right-most node in a red-black tree.
 * @tree: Pointer to the red-black tree whose right-most node's start value is to be retrieved.
 *
 * This function traverses the red-black tree from the root to the right-most node, which is the node
 * with the highest key value in the tree. 
 * Return: The start value of the right-most node if the tree is not empty; otherwise, 0.
 */
uintptr_t rb_get_righter(rbtree_t* tree);

/**
 * rb_get_lefter() - Retrieves the start value of the left-most node in a red-black tree.
 * @tree: Pointer to the red-black tree whose left-most node's start value is to be retrieved.
 *
 * This function traverses the red-black tree from the root to the left-most node, which is the node
 * with the lowest key value in the tree. 
 * Return: The start value of the left-most node if the tree is not empty; otherwise, 0.
 */
uintptr_t rb_get_lefter(rbtree_t* tree);

#endif // RBTREE_H

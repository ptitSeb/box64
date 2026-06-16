#ifndef __BOX64_INTELLIGENT_CACHE_H_
#define __BOX64_INTELLIGENT_CACHE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// Intelligent Code Cache for Box64
// Frame-aware cache strategy to reduce stutter during gameplay
// ============================================================================

// Cache strategy types
typedef enum {
    CACHE_STRATEGY_DEFAULT = 0,     // Standard LRU
    CACHE_STRATEGY_GAMEPLAY,        // Preserve gameplay blocks
    CACHE_STRATEGY_UI,              // Preserve UI/menu blocks  
    CACHE_STRATEGY_LOADING,         // Loading optimized
    CACHE_STRATEGY_HYBRID,          // Adaptive based on frame type
} cache_strategy_t;

// Block metadata for intelligent cache
typedef struct cache_block_meta {
    uintptr_t block_addr;
    uintptr_t native_code_addr;
    size_t native_code_size;
    uint64_t last_access_time;
    uint64_t access_count;
    uint64_t frame_access_count;     // Accesses in current frame
    uint32_t optimization_level;    // From hybrid JIT
    uint8_t  frame_type;            // Frame type when last accessed
    uint8_t  priority;              // Block priority
    bool     in_gameplay_hotset;    // Part of hot gameplay set
    bool     pinned;                // Don't evict
} cache_block_meta_t;

// Cache statistics
typedef struct cache_stats {
    uint64_t total_blocks;
    uint64_t total_size;
    uint64_t gameplay_blocks;
    uint64_t ui_blocks;
    uint64_t loading_blocks;
    uint64_t hot_blocks;
    uint64_t pinned_blocks;
    uint64_t evictions;
    uint64_t cache_hits;
    uint64_t cache_misses;
    uint64_t frame_hits;           // Hits within same frame
    uint64_t frame_misses;         // Misses within same frame
    double   hit_rate;
    double   frame_hit_rate;
} cache_stats_t;

// Cache configuration
typedef struct cache_config {
    // Size limits
    size_t max_total_size;         // Maximum cache size
    size_t max_gameplay_size;      // Max for gameplay blocks
    size_t max_ui_size;            // Max for UI blocks
    size_t max_hot_size;          // Max for hot blocks
    
    // Block limits
    uint32_t max_total_blocks;
    uint32_t max_gameplay_blocks;
    uint32_t max_hot_blocks;
    
    // Frame timing
    uint64_t frame_time_window_ns;  // Window for "same frame" detection (e.g., 16666666 for 60fps)
    
    // Behavior
    bool enable_frame_awareness;    // Enable frame-aware caching
    bool enable_priority_eviction; // High priority blocks evict low priority first
    bool enable_hotset;            // Maintain dedicated hot set
    bool enable_prefetch;          // Enable prefetching
    
    // Thresholds
    uint64_t hotset_min_accesses;   // Min accesses for hotset inclusion
    uint32_t pinned_max_per_frame; // Max pinned blocks per frame
} cache_config_t;

// Frame context for cache decisions
typedef struct frame_cache_context {
    uint64_t frame_number;
    uint64_t frame_start_time;
    uint8_t  frame_type;           // From hybrid_jit.h
    uint32_t blocks_compiled;
    uint32_t blocks_accessed;
    uint32_t blocks_evicted;
} frame_cache_context_t;

// Global cache state
typedef struct intelligent_cache {
    cache_config_t config;
    cache_stats_t stats;
    frame_cache_context_t current_frame;
    
    // Cache blocks (simplified - real impl would use proper hash table)
    cache_block_meta_t* blocks;
    uint32_t block_count;
    uint32_t block_capacity;
    
    // Hot set (frequently accessed gameplay blocks)
    cache_block_meta_t* hotset;
    uint32_t hotset_count;
    uint32_t hotset_capacity;
    
    // Eviction tracking
    uint64_t last_pressure_time;
    uint64_t recent_evictions;
    
    // Control
    bool initialized;
    bool frame_awareness_active;
} intelligent_cache_t;

extern intelligent_cache_t g_intelligent_cache;

// ============================================================================
// Intelligent Cache API
// ============================================================================

// Initialize intelligent cache
void Box64IntCache_Init(void);

// Register a compiled block in cache
void Box64IntCache_RegisterBlock(
    uintptr_t x86_addr,
    uintptr_t native_addr,
    size_t native_size,
    uint32_t optimization_level
);

// Access a block (record hit/miss)
void Box64IntCache_AccessBlock(uintptr_t x86_addr);

// Invalidate a block
void Box64IntCache_InvalidateBlock(uintptr_t x86_addr);

// Mark block as pinned (don't evict)
void Box64IntCache_PinBlock(uintptr_t x86_addr);

// Unpin block
void Box64IntCache_UnpinBlock(uintptr_t x86_addr);

// Check if block is in cache (fast lookup)
bool Box64IntCache_ContainsBlock(uintptr_t x86_addr);

// Get block metadata
const cache_block_meta_t* Box64IntCache_GetBlockMeta(uintptr_t x86_addr);

// Frame-aware operations
void Box64IntCache_BeginFrame(uint8_t frame_type);
void Box64IntCache_EndFrame(void);

// Eviction control
void Box64IntCache_SetStrategy(cache_strategy_t strategy);
void Box64IntCache_TriggerEviction(size_t bytes_needed);
void Box64IntCache_ApplyPressure(void);

// Get statistics
void Box64IntCache_GetStats(cache_stats_t* stats);

// Configuration
const cache_config_t* Box64IntCache_GetConfig(void);
void Box64IntCache_SetConfig(const cache_config_t* config);

// Memory pressure callback (from system)
void Box64IntCache_OnMemoryPressure(bool critical);

// Cleanup
void Box64IntCache_Destroy(void);

// ============================================================================
// Frame-aware helpers
// ============================================================================

// Check if block is part of current frame's working set
static inline bool Box64IntCache_IsInCurrentFrameSet(uintptr_t x86_addr) {
    if (!g_intelligent_cache.initialized) return false;
    if (!g_intelligent_cache.config.enable_frame_awareness) return false;
    
    // Check if block was accessed in current frame
    const cache_block_meta_t* meta = Box64IntCache_GetBlockMeta(x86_addr);
    if (!meta) return false;
    
    return (meta->frame_access_count > 0);
}

// Get recommended eviction candidate
static inline uintptr_t Box64IntCache_GetEvictionCandidate(void) {
    if (!g_intelligent_cache.initialized) return 0;
    
    // Find lowest priority, non-pinned block
    // This is a simplified version - real impl would use proper data structure
    uintptr_t best = 0;
    uint32_t best_priority = 0xFFFFFFFF;
    uint64_t best_last_access = UINT64_MAX;
    
    for (uint32_t i = 0; i < g_intelligent_cache.block_count; i++) {
        cache_block_meta_t* block = &g_intelligent_cache.blocks[i];
        
        if (block->pinned) continue;
        if (block->frame_access_count > 0) continue; // Don't evict current frame blocks
        
        uint32_t priority = block->priority;
        
        if (priority < best_priority ||
            (priority == best_priority && block->last_access_time < best_last_access)) {
            best_priority = priority;
            best_last_access = block->last_access_time;
            best = block->block_addr;
        }
    }
    
    return best;
}

#endif // __BOX64_INTELLIGENT_CACHE_H_

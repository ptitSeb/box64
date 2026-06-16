#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include "intelligent_cache.h"
#include "debug.h"
#include "box64context.h"
#include "hybrid_jit.h"

// ============================================================================
// Intelligent Code Cache Implementation
// Frame-aware caching strategy to reduce stutter
// ============================================================================

// Default configuration
static const cache_config_t default_config = {
    .max_total_size = 128 * 1024 * 1024,     // 128MB default
    .max_gameplay_size = 64 * 1024 * 1024,    // 64MB for gameplay
    .max_ui_size = 16 * 1024 * 1024,          // 16MB for UI
    .max_hot_size = 96 * 1024 * 1024,        // 96MB for hot blocks
    
    .max_total_blocks = 65536,               // Max 64K blocks
    .max_gameplay_blocks = 32768,            // 32K gameplay blocks
    .max_hot_blocks = 16384,                 // 16K hot blocks
    
    .frame_time_window_ns = 16666666,        // 60 FPS = 16.67ms
    
    .enable_frame_awareness = true,
    .enable_priority_eviction = true,
    .enable_hotset = true,
    .enable_prefetch = true,
    
    .hotset_min_accesses = 100,              // Min accesses for hotset
    .pinned_max_per_frame = 256,              // Max pinned blocks per frame
};

// Global state
intelligent_cache_t g_intelligent_cache = {
    .initialized = false,
};

static pthread_mutex_t g_intcache_mutex = PTHREAD_MUTEX_INITIALIZER;

// Time utilities
static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// ============================================================================
// Internal helpers
// ============================================================================

static cache_block_meta_t* find_block(uintptr_t x86_addr) {
    for (uint32_t i = 0; i < g_intelligent_cache.block_count; i++) {
        if (g_intelligent_cache.blocks[i].block_addr == x86_addr) {
            return &g_intelligent_cache.blocks[i];
        }
    }
    return NULL;
}

static cache_block_meta_t* find_or_create_block(uintptr_t x86_addr) {
    cache_block_meta_t* block = find_block(x86_addr);
    if (block) return block;
    
    // Need to create new block
    if (g_intelligent_cache.block_count >= g_intelligent_cache.block_capacity) {
        uint32_t new_capacity = g_intelligent_cache.block_capacity == 0 ? 4096 : 
                                g_intelligent_cache.block_capacity * 2;
        cache_block_meta_t* new_blocks = realloc(g_intelligent_cache.blocks,
                                                 new_capacity * sizeof(cache_block_meta_t));
        if (!new_blocks) return NULL;
        g_intelligent_cache.blocks = new_blocks;
        g_intelligent_cache.block_capacity = new_capacity;
    }
    
    cache_block_meta_t* new_block = &g_intelligent_cache.blocks[g_intelligent_cache.block_count];
    memset(new_block, 0, sizeof(cache_block_meta_t));
    new_block->block_addr = x86_addr;
    g_intelligent_cache.block_count++;
    
    return new_block;
}

static void evict_block(cache_block_meta_t* block) {
    if (!block) return;
    
    // Update stats
    g_intelligent_cache.stats.evictions++;
    
    if (block->in_gameplay_hotset) {
        // Remove from hotset
        for (uint32_t i = 0; i < g_intelligent_cache.hotset_count; i++) {
            if (g_intelligent_cache.hotset[i].block_addr == block->block_addr) {
                // Swap with last and decrement
                g_intelligent_cache.hotset[i] = g_intelligent_cache.hotset[g_intelligent_cache.hotset_count - 1];
                g_intelligent_cache.hotset_count--;
                break;
            }
        }
    }
    
    // Mark block as invalid (don't actually free - let dynacache handle it)
    block->native_code_addr = 0;
    block->pinned = false;
    block->in_gameplay_hotset = false;
    
    // Note: Actual memory freeing is handled by the underlying dynarec system
}

static void evict_low_priority_blocks(size_t bytes_needed) {
    size_t evicted = 0;
    uint32_t evicted_count = 0;
    
    // Sort blocks by priority (simple bubble for now - real impl would use proper structure)
    // Priority: pinned > hot > gameplay > ui > default
    
    for (uint32_t i = 0; i < g_intelligent_cache.block_count && evicted < bytes_needed; i++) {
        cache_block_meta_t* block = &g_intelligent_cache.blocks[i];
        
        // Skip pinned blocks
        if (block->pinned) continue;
        
        // Skip blocks accessed in current frame
        if (block->frame_access_count > 0) continue;
        
        // Skip blocks in hotset if hotset is enabled
        if (g_intelligent_cache.config.enable_hotset && block->in_gameplay_hotset) continue;
        
        // Skip blocks with high priority
        if (block->priority >= 2) continue;
        
        evict_block(block);
        evicted += block->native_code_size;
        evicted_count++;
    }
    
    #ifdef DEBUG
    dynarec_log(LOG_DEBUG, "Box64IntCache: Evicted %u blocks (%zu bytes)\n", evicted_count, evicted);
    #endif
}

// ============================================================================
// Initialization
// ============================================================================

void Box64IntCache_Init(void) {
    if (g_intelligent_cache.initialized) return;
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    if (g_intelligent_cache.initialized) {
        pthread_mutex_unlock(&g_intcache_mutex);
        return;
    }
    
    // Initialize with default config
    memcpy(&g_intelligent_cache.config, &default_config, sizeof(cache_config_t));
    
    // Check for environment variable overrides
    const char* env = getenv("BOX64_INTCACHE_SIZE");
    if (env) {
        size_t size = atoll(env);
        if (size > 0) {
            g_intelligent_cache.config.max_total_size = size;
        }
    }
    
    env = getenv("BOX64_INTCACHE_FRAME_AWARE");
    if (env && atoi(env) == 0) {
        g_intelligent_cache.config.enable_frame_awareness = false;
    }
    
    env = getenv("BOX64_INTCACHE_HOTSET");
    if (env && atoi(env) == 0) {
        g_intelligent_cache.config.enable_hotset = false;
    }
    
    // Initialize state
    g_intelligent_cache.block_count = 0;
    g_intelligent_cache.block_capacity = 0;
    g_intelligent_cache.blocks = NULL;
    
    g_intelligent_cache.hotset_count = 0;
    g_intelligent_cache.hotset_capacity = 1024;
    g_intelligent_cache.hotset = calloc(g_intelligent_cache.hotset_capacity, 
                                         sizeof(cache_block_meta_t));
    
    g_intelligent_cache.stats.total_blocks = 0;
    g_intelligent_cache.stats.total_size = 0;
    g_intelligent_cache.stats.evictions = 0;
    g_intelligent_cache.stats.cache_hits = 0;
    g_intelligent_cache.stats.cache_misses = 0;
    
    g_intelligent_cache.current_frame.frame_number = 0;
    g_intelligent_cache.current_frame.blocks_compiled = 0;
    g_intelligent_cache.current_frame.blocks_accessed = 0;
    g_intelligent_cache.current_frame.blocks_evicted = 0;
    
    g_intelligent_cache.last_pressure_time = 0;
    g_intelligent_cache.recent_evictions = 0;
    
    g_intelligent_cache.frame_awareness_active = true;
    g_intelligent_cache.initialized = true;
    
    dynarec_log(LOG_INFO, "Box64 Intelligent Cache initialized: size=%zu MB, frame_aware=%d\n",
                g_intelligent_cache.config.max_total_size / (1024 * 1024),
                g_intelligent_cache.config.enable_frame_awareness);
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

// ============================================================================
// Block management
// ============================================================================

void Box64IntCache_RegisterBlock(
    uintptr_t x86_addr,
    uintptr_t native_addr,
    size_t native_size,
    uint32_t optimization_level
) {
    if (!g_intelligent_cache.initialized) Box64IntCache_Init();
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    cache_block_meta_t* block = find_or_create_block(x86_addr);
    if (!block) {
        pthread_mutex_unlock(&g_intcache_mutex);
        return;
    }
    
    block->native_code_addr = native_addr;
    block->native_code_size = native_size;
    block->optimization_level = optimization_level;
    block->last_access_time = get_time_ns();
    block->access_count = 1;
    block->frame_access_count = 1;
    block->frame_type = g_intelligent_cache.current_frame.frame_type;
    
    // Set priority based on optimization level
    if (optimization_level >= 3) { // JIT_OPT_HOT
        block->priority = 3; // CRITICAL
    } else if (optimization_level >= 2) { // JIT_OPT_NORMAL
        block->priority = 2; // HIGH
    } else {
        block->priority = 1; // NORMAL
    }
    
    // Check if should add to hotset
    if (g_intelligent_cache.config.enable_hotset && 
        block->access_count >= g_intelligent_cache.config.hotset_min_accesses) {
        if (g_intelligent_cache.hotset_count < g_intelligent_cache.hotset_capacity) {
            block->in_gameplay_hotset = true;
            g_intelligent_cache.hotset[g_intelligent_cache.hotset_count++] = *block;
            g_intelligent_cache.stats.hot_blocks++;
        }
    }
    
    // Update stats
    g_intelligent_cache.stats.total_size += native_size;
    g_intelligent_cache.stats.total_blocks++;
    g_intelligent_cache.current_frame.blocks_compiled++;
    
    // Check if we need to evict
    if (g_intelligent_cache.stats.total_size > g_intelligent_cache.config.max_total_size) {
        pthread_mutex_unlock(&g_intcache_mutex);
        Box64IntCache_TriggerEviction(64 * 1024); // Evict 64KB
        pthread_mutex_lock(&g_intcache_mutex);
    }
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

void Box64IntCache_AccessBlock(uintptr_t x86_addr) {
    if (!g_intelligent_cache.initialized) Box64IntCache_Init();
    if (!g_intelligent_cache.config.enable_frame_awareness) return;
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    cache_block_meta_t* block = find_block(x86_addr);
    if (!block) {
        g_intelligent_cache.stats.cache_misses++;
        g_intelligent_cache.stats.frame_misses++;
        pthread_mutex_unlock(&g_intcache_mutex);
        return;
    }
    
    // Update access info
    block->last_access_time = get_time_ns();
    block->access_count++;
    block->frame_access_count++;
    
    g_intelligent_cache.stats.cache_hits++;
    g_intelligent_cache.stats.frame_hits++;
    g_intelligent_cache.current_frame.blocks_accessed++;
    
    // Update hotset if applicable
    if (g_intelligent_cache.config.enable_hotset &&
        !block->in_gameplay_hotset &&
        block->access_count >= g_intelligent_cache.config.hotset_min_accesses) {
        if (g_intelligent_cache.hotset_count < g_intelligent_cache.hotset_capacity) {
            block->in_gameplay_hotset = true;
            g_intelligent_cache.hotset[g_intelligent_cache.hotset_count++] = *block;
            g_intelligent_cache.stats.hot_blocks++;
        }
    }
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

void Box64IntCache_InvalidateBlock(uintptr_t x86_addr) {
    if (!g_intelligent_cache.initialized) return;
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    cache_block_meta_t* block = find_block(x86_addr);
    if (block) {
        evict_block(block);
    }
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

void Box64IntCache_PinBlock(uintptr_t x86_addr) {
    if (!g_intelligent_cache.initialized) Box64IntCache_Init();
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    cache_block_meta_t* block = find_block(x86_addr);
    if (block) {
        block->pinned = true;
        g_intelligent_cache.stats.pinned_blocks++;
    }
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

void Box64IntCache_UnpinBlock(uintptr_t x86_addr) {
    if (!g_intelligent_cache.initialized) return;
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    cache_block_meta_t* block = find_block(x86_addr);
    if (block) {
        block->pinned = false;
        if (g_intelligent_cache.stats.pinned_blocks > 0)
            g_intelligent_cache.stats.pinned_blocks--;
    }
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

bool Box64IntCache_ContainsBlock(uintptr_t x86_addr) {
    if (!g_intelligent_cache.initialized) return false;
    
    // Fast lock-free check
    return find_block(x86_addr) != NULL;
}

const cache_block_meta_t* Box64IntCache_GetBlockMeta(uintptr_t x86_addr) {
    if (!g_intelligent_cache.initialized) return NULL;
    return find_block(x86_addr);
}

// ============================================================================
// Frame-aware operations
// ============================================================================

void Box64IntCache_BeginFrame(uint8_t frame_type) {
    if (!g_intelligent_cache.initialized) Box64IntCache_Init();
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    // End previous frame
    if (g_intelligent_cache.current_frame.frame_number > 0) {
        Box64IntCache_EndFrame();
    }
    
    g_intelligent_cache.current_frame.frame_number++;
    g_intelligent_cache.current_frame.frame_start_time = get_time_ns();
    g_intelligent_cache.current_frame.frame_type = frame_type;
    g_intelligent_cache.current_frame.blocks_compiled = 0;
    g_intelligent_cache.current_frame.blocks_accessed = 0;
    g_intelligent_cache.current_frame.blocks_evicted = 0;
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

void Box64IntCache_EndFrame(void) {
    if (!g_intelligent_cache.initialized) return;
    if (g_intelligent_cache.current_frame.frame_number == 0) return;
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    uint64_t frame_time = get_time_ns() - g_intelligent_cache.current_frame.frame_start_time;
    
    // Reset frame access counts for all blocks
    for (uint32_t i = 0; i < g_intelligent_cache.block_count; i++) {
        g_intelligent_cache.blocks[i].frame_access_count = 0;
    }
    
    // Unpin blocks that were pinned for this frame (but respect max pinned)
    uint32_t pinned_count = 0;
    for (uint32_t i = 0; i < g_intelligent_cache.block_count; i++) {
        if (g_intelligent_cache.blocks[i].pinned) {
            if (pinned_count >= g_intelligent_cache.config.pinned_max_per_frame) {
                g_intelligent_cache.blocks[i].pinned = false;
            } else {
                pinned_count++;
            }
        }
    }
    
    // Calculate hit rates
    uint64_t total_accesses = g_intelligent_cache.stats.cache_hits + 
                               g_intelligent_cache.stats.cache_misses;
    if (total_accesses > 0) {
        g_intelligent_cache.stats.hit_rate = 
            (double)g_intelligent_cache.stats.cache_hits / total_accesses;
    }
    
    uint64_t frame_accesses = g_intelligent_cache.stats.frame_hits + 
                              g_intelligent_cache.stats.frame_misses;
    if (frame_accesses > 0) {
        g_intelligent_cache.stats.frame_hit_rate = 
            (double)g_intelligent_cache.stats.frame_hits / frame_accesses;
    }
    
    // Debug logging
    #ifdef DEBUG
    if (frame_time > g_intelligent_cache.config.frame_time_window_ns) {
        dynarec_log(LOG_DEBUG, 
                    "Box64IntCache Frame %llu: type=%d, accessed=%u, compiled=%u, evicted=%u, time=%llu us\n",
                    (unsigned long long)g_intelligent_cache.current_frame.frame_number,
                    g_intelligent_cache.current_frame.frame_type,
                    g_intelligent_cache.current_frame.blocks_accessed,
                    g_intelligent_cache.current_frame.blocks_compiled,
                    g_intelligent_cache.current_frame.blocks_evicted,
                    (unsigned long long)(frame_time / 1000));
    }
    #endif
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

// ============================================================================
// Eviction control
// ============================================================================

void Box64IntCache_SetStrategy(cache_strategy_t strategy) {
    if (!g_intelligent_cache.initialized) Box64IntCache_Init();
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    // Adjust configuration based on strategy
    switch (strategy) {
        case CACHE_STRATEGY_GAMEPLAY:
            g_intelligent_cache.config.enable_frame_awareness = true;
            g_intelligent_cache.config.enable_hotset = true;
            break;
        case CACHE_STRATEGY_UI:
            g_intelligent_cache.config.enable_frame_awareness = true;
            g_intelligent_cache.config.enable_hotset = false;
            break;
        case CACHE_STRATEGY_LOADING:
            g_intelligent_cache.config.enable_frame_awareness = false;
            g_intelligent_cache.config.enable_hotset = false;
            break;
        case CACHE_STRATEGY_HYBRID:
            g_intelligent_cache.config.enable_frame_awareness = true;
            g_intelligent_cache.config.enable_hotset = true;
            break;
        default:
            break;
    }
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

void Box64IntCache_TriggerEviction(size_t bytes_needed) {
    if (!g_intelligent_cache.initialized) return;
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    if (g_intelligent_cache.stats.total_size + bytes_needed <= 
        g_intelligent_cache.config.max_total_size) {
        pthread_mutex_unlock(&g_intcache_mutex);
        return;
    }
    
    evict_low_priority_blocks(bytes_needed);
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

void Box64IntCache_ApplyPressure(void) {
    if (!g_intelligent_cache.initialized) return;
    
    uint64_t now = get_time_ns();
    
    // Don't apply pressure too frequently
    if (now - g_intelligent_cache.last_pressure_time < 100000000) { // 100ms
        return;
    }
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    g_intelligent_cache.last_pressure_time = now;
    
    // Check if we're approaching limits
    size_t threshold = g_intelligent_cache.config.max_total_size * 9 / 10;
    
    if (g_intelligent_cache.stats.total_size > threshold) {
        size_t excess = g_intelligent_cache.stats.total_size - threshold;
        evict_low_priority_blocks(excess);
    }
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

// ============================================================================
// Statistics and configuration
// ============================================================================

void Box64IntCache_GetStats(cache_stats_t* stats) {
    if (!g_intelligent_cache.initialized) Box64IntCache_Init();
    if (!stats) return;
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    memcpy(stats, &g_intelligent_cache.stats, sizeof(cache_stats_t));
    
    // Count blocks by type
    stats->gameplay_blocks = 0;
    stats->ui_blocks = 0;
    stats->loading_blocks = 0;
    
    for (uint32_t i = 0; i < g_intelligent_cache.block_count; i++) {
        cache_block_meta_t* block = &g_intelligent_cache.blocks[i];
        
        if (block->in_gameplay_hotset) {
            stats->gameplay_blocks++;
        } else if (block->frame_type == 1) { // UI_MENU
            stats->ui_blocks++;
        } else if (block->frame_type == 2) { // LOADING
            stats->loading_blocks++;
        }
    }
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

const cache_config_t* Box64IntCache_GetConfig(void) {
    if (!g_intelligent_cache.initialized) return &default_config;
    return &g_intelligent_cache.config;
}

void Box64IntCache_SetConfig(const cache_config_t* config) {
    if (!g_intelligent_cache.initialized) return;
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    if (config) {
        memcpy(&g_intelligent_cache.config, config, sizeof(cache_config_t));
    }
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

void Box64IntCache_OnMemoryPressure(bool critical) {
    if (!g_intelligent_cache.initialized) return;
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    if (critical) {
        // Aggressive eviction
        size_t target = g_intelligent_cache.config.max_total_size / 2;
        
        while (g_intelligent_cache.stats.total_size > target && 
               g_intelligent_cache.block_count > 0) {
            evict_low_priority_blocks(1024 * 1024); // 1MB at a time
        }
        
        dynarec_log(LOG_INFO, "Box64IntCache: Critical memory pressure, evicted to %zu MB\n",
                    g_intelligent_cache.stats.total_size / (1024 * 1024));
    } else {
        // Gentle pressure
        Box64IntCache_ApplyPressure();
    }
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

void Box64IntCache_Destroy(void) {
    if (!g_intelligent_cache.initialized) return;
    
    pthread_mutex_lock(&g_intcache_mutex);
    
    if (g_intelligent_cache.blocks) {
        free(g_intelligent_cache.blocks);
        g_intelligent_cache.blocks = NULL;
    }
    
    if (g_intelligent_cache.hotset) {
        free(g_intelligent_cache.hotset);
        g_intelligent_cache.hotset = NULL;
    }
    
    g_intelligent_cache.block_count = 0;
    g_intelligent_cache.block_capacity = 0;
    g_intelligent_cache.hotset_count = 0;
    g_intelligent_cache.initialized = false;
    
    pthread_mutex_unlock(&g_intcache_mutex);
}

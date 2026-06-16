#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "hybrid_jit.h"
#include "debug.h"
#include "box64context.h"
#include "soc.h"

// ============================================================================
// Hybrid JIT System Implementation
// Dual-mode JIT: Fast JIT + Hot-block Aggressive JIT
// ============================================================================

// Nanoseconds per second
#define NS_PER_SEC 1000000000ULL

// Default configuration
static const hybrid_jit_config_t default_config = {
    .hot_threshold_fast = 100,          // 100 executions = hot
    .hot_threshold_aggressive = 1000,    // 1000 executions = aggressive
    .min_execution_count = 10,          // Min 10 executions before optimization
    .consecutive_hot_count = 3,         // 3 consecutive hot = recompile
    .max_fast_cache_size = 64 * 1024 * 1024,    // 64MB fast cache
    .max_hot_cache_size = 128 * 1024 * 1024,    // 128MB hot cache
    .max_total_cache_size = 256 * 1024 * 1024,   // 256MB total
    .target_frame_time_ns = 16666666,    // 60 FPS target (16.67ms)
    .max_jit_time_per_frame_ns = 500000, // 0.5ms max JIT time per frame
    .enable_thermal_throttling = true,
    .enable_power_throttling = true,
    .enable_stats = false,
    .enable_profiling = false,
};

// Global state
hybrid_jit_state_t g_hybrid_jit = {
    .config = {0},
    .initialized = false,
};

// Mutex for thread-safe operations
static pthread_mutex_t g_hybrid_jit_mutex = PTHREAD_MUTEX_INITIALIZER;

// ============================================================================
// Time utilities
// ============================================================================

static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * NS_PER_SEC + ts.tv_nsec;
}

// ============================================================================
// Profile management
// ============================================================================

static block_profile_t* find_or_create_profile(uintptr_t block_addr) {
    // Search existing profiles
    for (uint32_t i = 0; i < g_hybrid_jit.profile_count; i++) {
        if (g_hybrid_jit.profiles[i].block_addr == block_addr) {
            return &g_hybrid_jit.profiles[i];
        }
    }
    
    // Need to create new profile
    if (g_hybrid_jit.profile_count >= g_hybrid_jit.profile_capacity) {
        uint32_t new_capacity = g_hybrid_jit.profile_capacity == 0 ? 256 : 
                                 g_hybrid_jit.profile_capacity * 2;
        block_profile_t* new_profiles = realloc(g_hybrid_jit.profiles, 
                                                 new_capacity * sizeof(block_profile_t));
        if (!new_profiles) return NULL;
        g_hybrid_jit.profiles = new_profiles;
        g_hybrid_jit.profile_capacity = new_capacity;
    }
    
    block_profile_t* profile = &g_hybrid_jit.profiles[g_hybrid_jit.profile_count];
    memset(profile, 0, sizeof(block_profile_t));
    profile->block_addr = block_addr;
    profile->optimization_level = JIT_OPT_FAST;
    profile->is_hot = false;
    profile->needs_recompile = false;
    g_hybrid_jit.profile_count++;
    
    return profile;
}

// ============================================================================
// Initialization
// ============================================================================

void Box64HybridJIT_Init(void) {
    if (g_hybrid_jit.initialized) return;
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    if (g_hybrid_jit.initialized) {
        pthread_mutex_unlock(&g_hybrid_jit_mutex);
        return;
    }
    
    // Initialize with default config
    memcpy(&g_hybrid_jit.config, &default_config, sizeof(hybrid_jit_config_t));
    
    // Check for environment variable overrides
    const char* env = getenv("BOX64_HYBRID_JIT_HOT_THRESHOLD");
    if (env) {
        g_hybrid_jit.config.hot_threshold_fast = atoll(env);
    }
    
    env = getenv("BOX64_HYBRID_JIT_AGGRESSIVE_THRESHOLD");
    if (env) {
        g_hybrid_jit.config.hot_threshold_aggressive = atoll(env);
    }
    
    env = getenv("BOX64_HYBRID_JIT_STATS");
    if (env && atoi(env)) {
        g_hybrid_jit.config.enable_stats = true;
    }
    
    env = getenv("BOX64_HYBRID_JIT_PROFILE");
    if (env && atoi(env)) {
        g_hybrid_jit.config.enable_profiling = true;
        g_hybrid_jit.profiling_enabled = true;
    }
    
    // Initialize state
    g_hybrid_jit.profile_count = 0;
    g_hybrid_jit.profile_capacity = 0;
    g_hybrid_jit.profiles = NULL;
    
    g_hybrid_jit.fast_cache_size = 0;
    g_hybrid_jit.hot_cache_size = 0;
    g_hybrid_jit.total_compilations = 0;
    g_hybrid_jit.recompilations = 0;
    g_hybrid_jit.hot_compilations = 0;
    
    memset(&g_hybrid_jit.current_frame, 0, sizeof(frame_context_t));
    g_hybrid_jit.current_frame.type = FRAME_TYPE_UNKNOWN;
    g_hybrid_jit.last_frame_type = FRAME_TYPE_UNKNOWN;
    
    g_hybrid_jit.thermal_throttling = false;
    g_hybrid_jit.power_throttling = false;
    
    g_hybrid_jit.total_executions = 0;
    g_hybrid_jit.hot_block_executions = 0;
    g_hybrid_jit.aggressive_jit_time_ns = 0;
    
    g_hybrid_jit.initialized = true;
    
    dynarec_log(LOG_INFO, "Box64 Hybrid JIT initialized: hot=%llu, aggressive=%llu\n",
                (unsigned long long)g_hybrid_jit.config.hot_threshold_fast,
                (unsigned long long)g_hybrid_jit.config.hot_threshold_aggressive);
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
}

// ============================================================================
// Core JIT API
// ============================================================================

void Box64HybridJIT_RecordExecution(uintptr_t block_addr, uint32_t execution_time_ns) {
    if (!g_hybrid_jit.initialized) return;
    
    // Fast path: don't lock for common case
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    block_profile_t* profile = find_or_create_profile(block_addr);
    if (!profile) {
        pthread_mutex_unlock(&g_hybrid_jit_mutex);
        return;
    }
    
    // Update execution statistics
    profile->execution_count++;
    profile->last_execution_time = get_time_ns();
    g_hybrid_jit.total_executions++;
    
    // Update timing statistics
    if (profile->execution_count == 1) {
        profile->avg_execution_time_ns = execution_time_ns;
        profile->min_execution_time_ns = execution_time_ns;
        profile->max_execution_time_ns = execution_time_ns;
    } else {
        // Running average
        uint64_t total = (uint64_t)profile->avg_execution_time_ns * (profile->execution_count - 1);
        profile->avg_execution_time_ns = (total + execution_time_ns) / profile->execution_count;
        
        if (execution_time_ns < profile->min_execution_time_ns)
            profile->min_execution_time_ns = execution_time_ns;
        if (execution_time_ns > profile->max_execution_time_ns)
            profile->max_execution_time_ns = execution_time_ns;
    }
    
    // Check hot block status
    bool was_hot = profile->is_hot;
    
    if (profile->execution_count >= g_hybrid_jit.config.hot_threshold_fast) {
        profile->is_hot = true;
        profile->consecutive_hot_count++;
        g_hybrid_jit.hot_block_executions++;
        
        // Check if should upgrade to aggressive
        if (profile->optimization_level < JIT_OPT_HOT) {
            if (profile->execution_count >= g_hybrid_jit.config.hot_threshold_aggressive) {
                if (profile->consecutive_hot_count >= g_hybrid_jit.config.consecutive_hot_count) {
                    profile->needs_recompile = true;
                    profile->optimization_level = JIT_OPT_HOT;
                }
            } else if (profile->consecutive_hot_count >= 2) {
                // Upgrade to normal optimization
                if (profile->optimization_level < JIT_OPT_NORMAL) {
                    profile->optimization_level = JIT_OPT_NORMAL;
                    profile->needs_recompile = true;
                }
            }
        }
    }
    
    // Update cache statistics
    if (profile->optimization_level >= JIT_OPT_HOT) {
        profile->cache_hits++;
    }
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
}

bool Box64HybridJIT_ShouldRecompile(uintptr_t block_addr) {
    if (!g_hybrid_jit.initialized) return false;
    
    // Check if thermal throttling is active - disable aggressive recompilation
    if (g_hybrid_jit.thermal_throttling && g_hybrid_jit.config.enable_thermal_throttling) {
        return false;
    }
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    for (uint32_t i = 0; i < g_hybrid_jit.profile_count; i++) {
        if (g_hybrid_jit.profiles[i].block_addr == block_addr) {
            bool result = g_hybrid_jit.profiles[i].needs_recompile;
            
            // Check cache size limits
            if (g_hybrid_jit.hot_cache_size >= g_hybrid_jit.config.max_hot_cache_size) {
                // At cache limit, only recompile critical blocks
                result = result && (g_hybrid_jit.profiles[i].optimization_level >= JIT_OPT_HOT);
            }
            
            pthread_mutex_unlock(&g_hybrid_jit_mutex);
            return result;
        }
    }
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
    return false;
}

jit_opt_level_t Box64HybridJIT_GetOptimizationLevel(uintptr_t block_addr) {
    if (!g_hybrid_jit.initialized) return JIT_OPT_FAST;
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    for (uint32_t i = 0; i < g_hybrid_jit.profile_count; i++) {
        if (g_hybrid_jit.profiles[i].block_addr == block_addr) {
            jit_opt_level_t level = g_hybrid_jit.profiles[i].optimization_level;
            pthread_mutex_unlock(&g_hybrid_jit_mutex);
            return level;
        }
    }
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
    return JIT_OPT_FAST;
}

void Box64HybridJIT_MarkForRecompile(uintptr_t block_addr) {
    if (!g_hybrid_jit.initialized) return;
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    for (uint32_t i = 0; i < g_hybrid_jit.profile_count; i++) {
        if (g_hybrid_jit.profiles[i].block_addr == block_addr) {
            g_hybrid_jit.profiles[i].needs_recompile = true;
            g_hybrid_jit.recompilations++;
            pthread_mutex_unlock(&g_hybrid_jit_mutex);
            return;
        }
    }
    
    // Create profile if it doesn't exist
    block_profile_t* profile = find_or_create_profile(block_addr);
    if (profile) {
        profile->needs_recompile = true;
        g_hybrid_jit.recompilations++;
    }
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
}

// ============================================================================
// Block Priority
// ============================================================================

block_priority_t Box64HybridJIT_GetBlockPriority(uintptr_t block_addr) {
    if (!g_hybrid_jit.initialized) return BLOCK_PRIORITY_NORMAL;
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    for (uint32_t i = 0; i < g_hybrid_jit.profile_count; i++) {
        if (g_hybrid_jit.profiles[i].block_addr == block_addr) {
            block_profile_t* p = &g_hybrid_jit.profiles[i];
            block_priority_t priority;
            
            // Calculate priority based on multiple factors
            uint64_t score = p->execution_count * 10;
            score += p->consecutive_hot_count * 100;
            score += (p->optimization_level >= JIT_OPT_HOT) ? 500 : 0;
            
            // High priority: very hot, frequently executed
            if (score > 5000 || (p->is_hot && p->execution_count > 10000)) {
                priority = BLOCK_PRIORITY_CRITICAL;
            }
            // Normal priority: moderately hot
            else if (score > 1000 || p->is_hot) {
                priority = BLOCK_PRIORITY_HIGH;
            }
            // Low priority: not hot, infrequently executed
            else if (p->execution_count < 50) {
                priority = BLOCK_PRIORITY_LOW;
            }
            // Default
            else {
                priority = BLOCK_PRIORITY_NORMAL;
            }
            
            pthread_mutex_unlock(&g_hybrid_jit_mutex);
            return priority;
        }
    }
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
    return BLOCK_PRIORITY_NORMAL;
}

// ============================================================================
// Frame-aware operations
// ============================================================================

void Box64HybridJIT_BeginFrame(frame_type_t type) {
    if (!g_hybrid_jit.initialized) return;
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    // End previous frame
    if (g_hybrid_jit.current_frame.frame_start_time_ns > 0) {
        Box64HybridJIT_EndFrame();
    }
    
    g_hybrid_jit.current_frame.type = type;
    g_hybrid_jit.current_frame.frame_number++;
    g_hybrid_jit.current_frame.frame_start_time_ns = get_time_ns();
    g_hybrid_jit.current_frame.blocks_executed = 0;
    g_hybrid_jit.current_frame.blocks_compiled = 0;
    g_hybrid_jit.current_frame.blocks_recompiled = 0;
    g_hybrid_jit.current_frame.jit_time_ns = 0;
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
}

void Box64HybridJIT_EndFrame(void) {
    if (!g_hybrid_jit.initialized) return;
    if (g_hybrid_jit.current_frame.frame_start_time_ns == 0) return;
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    g_hybrid_jit.current_frame.frame_end_time_ns = get_time_ns();
    g_hybrid_jit.last_frame_time_ns = g_hybrid_jit.current_frame.frame_end_time_ns - 
                                       g_hybrid_jit.current_frame.frame_start_time_ns;
    g_hybrid_jit.last_frame_type = g_hybrid_jit.current_frame.type;
    
    // Detect frame type changes for adaptive optimization
    // If frame time is very high, might be loading
    // If frame time is very low, might be menu
    if (g_hybrid_jit.last_frame_time_ns > g_hybrid_jit.config.target_frame_time_ns * 3) {
        g_hybrid_jit.current_frame.type = FRAME_TYPE_LOADING;
    }
    
    // Log stats in debug mode
    #ifdef DEBUG
    if (g_hybrid_jit.config.enable_stats && 
        (g_hybrid_jit.current_frame.blocks_compiled > 0 || 
         g_hybrid_jit.current_frame.blocks_recompiled > 0)) {
        dynarec_log(LOG_DEBUG, 
                    "Box64 Hybrid JIT Frame %llu: type=%d, blocks=%u, compiled=%u, recompiled=%u, jit_time=%ums\n",
                    (unsigned long long)g_hybrid_jit.current_frame.frame_number,
                    g_hybrid_jit.current_frame.type,
                    g_hybrid_jit.current_frame.blocks_executed,
                    g_hybrid_jit.current_frame.blocks_compiled,
                    g_hybrid_jit.current_frame.blocks_recompiled,
                    g_hybrid_jit.current_frame.jit_time_ns / 1000000);
    }
    #endif
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
}

frame_type_t Box64HybridJIT_DetectFrameType(void) {
    // Simple heuristic for frame type detection
    // Can be enhanced with more sophisticated detection
    
    if (g_hybrid_jit.current_frame.frame_start_time_ns == 0) {
        return FRAME_TYPE_UNKNOWN;
    }
    
    uint64_t frame_time = get_time_ns() - g_hybrid_jit.current_frame.frame_start_time_ns;
    
    if (frame_time > g_hybrid_jit.config.target_frame_time_ns * 5) {
        return FRAME_TYPE_LOADING;
    }
    
    // Check last frame type
    return g_hybrid_jit.last_frame_type;
}

// ============================================================================
// Cache management
// ============================================================================

size_t Box64HybridJIT_GetFastCacheSize(void) {
    if (!g_hybrid_jit.initialized) return 0;
    return g_hybrid_jit.fast_cache_size;
}

size_t Box64HybridJIT_GetHotCacheSize(void) {
    if (!g_hybrid_jit.initialized) return 0;
    return g_hybrid_jit.hot_cache_size;
}

void Box64HybridJIT_CleanupCache(void) {
    if (!g_hybrid_jit.initialized) return;
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    // Simple LRU-like cleanup: remove lowest priority blocks
    // This is a placeholder - actual implementation would coordinate
    // with the dynablock/dynacache system
    
    // Check if we need to cleanup
    size_t total_size = g_hybrid_jit.fast_cache_size + g_hybrid_jit.hot_cache_size;
    if (total_size < g_hybrid_jit.config.max_total_cache_size) {
        pthread_mutex_unlock(&g_hybrid_jit_mutex);
        return;
    }
    
    // Find blocks to evict (lowest priority, not hot)
    uint32_t evicted = 0;
    for (uint32_t i = 0; i < g_hybrid_jit.profile_count && 
                      total_size >= g_hybrid_jit.config.max_total_cache_size * 8 / 10; 
         i++) {
        if (g_hybrid_jit.profiles[i].optimization_level < JIT_OPT_HOT &&
            g_hybrid_jit.profiles[i].consecutive_hot_count == 0) {
            // This block is cold, can be evicted
            // Actual eviction is handled by dynacache
            evicted++;
            // Estimate size (this should come from actual cache)
            total_size -= 4096; // Assume 4KB per block
        }
    }
    
    #ifdef DEBUG
    dynarec_log(LOG_DEBUG, "Box64 Hybrid JIT: Cleaned up %u cold blocks\n", evicted);
    #endif
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
}

// ============================================================================
// Thermal/Power callbacks
// ============================================================================

void Box64HybridJIT_SetThermalState(bool throttling) {
    if (!g_hybrid_jit.initialized) return;
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    if (throttling != g_hybrid_jit.thermal_throttling) {
        g_hybrid_jit.thermal_throttling = throttling;
        
        if (throttling) {
            dynarec_log(LOG_INFO, "Box64 Hybrid JIT: Thermal throttling activated, reducing aggressiveness\n");
        } else {
            dynarec_log(LOG_INFO, "Box64 Hybrid JIT: Thermal throttling deactivated\n");
        }
    }
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
}

void Box64HybridJIT_SetPowerState(bool on_battery) {
    if (!g_hybrid_jit.initialized) return;
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    if (on_battery != g_hybrid_jit.power_throttling) {
        g_hybrid_jit.power_throttling = on_battery;
        
        if (on_battery) {
            dynarec_log(LOG_INFO, "Box64 Hybrid JIT: Power saving mode activated\n");
        } else {
            dynarec_log(LOG_INFO, "Box64 Hybrid JIT: Power saving mode deactivated\n");
        }
    }
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
}

// ============================================================================
// Configuration and Statistics
// ============================================================================

const hybrid_jit_config_t* Box64HybridJIT_GetConfig(void) {
    if (!g_hybrid_jit.initialized) return &default_config;
    return &g_hybrid_jit.config;
}

void Box64HybridJIT_SetConfig(const hybrid_jit_config_t* config) {
    if (!g_hybrid_jit.initialized) return;
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    if (config) {
        memcpy(&g_hybrid_jit.config, config, sizeof(hybrid_jit_config_t));
    }
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
}

void Box64HybridJIT_GetStats(
    uint64_t* total_executions,
    uint64_t* hot_executions,
    uint32_t* total_compilations,
    uint32_t* recompilations,
    uint32_t* hot_compilations,
    uint64_t* jit_time_ns
) {
    if (!g_hybrid_jit.initialized) {
        *total_executions = 0;
        *hot_executions = 0;
        *total_compilations = 0;
        *recompilations = 0;
        *hot_compilations = 0;
        *jit_time_ns = 0;
        return;
    }
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    if (total_executions) *total_executions = g_hybrid_jit.total_executions;
    if (hot_executions) *hot_executions = g_hybrid_jit.hot_block_executions;
    if (total_compilations) *total_compilations = g_hybrid_jit.total_compilations;
    if (recompilations) *recompilations = g_hybrid_jit.recompilations;
    if (hot_compilations) *hot_compilations = g_hybrid_jit.hot_compilations;
    if (jit_time_ns) *jit_time_ns = g_hybrid_jit.aggressive_jit_time_ns;
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
}

void Box64HybridJIT_EnableProfiling(bool enable) {
    if (!g_hybrid_jit.initialized) return;
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    g_hybrid_jit.profiling_enabled = enable;
    g_hybrid_jit.config.enable_profiling = enable;
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
}

void Box64HybridJIT_DumpProfiles(void) {
    if (!g_hybrid_jit.initialized) return;
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    dynarec_log(LOG_INFO, "=== Box64 Hybrid JIT Profile Dump ===\n");
    dynarec_log(LOG_INFO, "Total profiles: %u\n", g_hybrid_jit.profile_count);
    dynarec_log(LOG_INFO, "Total executions: %llu\n", (unsigned long long)g_hybrid_jit.total_executions);
    dynarec_log(LOG_INFO, "Hot block executions: %llu\n", (unsigned long long)g_hybrid_jit.hot_block_executions);
    dynarec_log(LOG_INFO, "Total compilations: %u, Recompilations: %u\n", 
                g_hybrid_jit.total_compilations, g_hybrid_jit.recompilations);
    
    // Dump top 20 hottest blocks
    dynarec_log(LOG_INFO, "Top 20 hot blocks:\n");
    uint32_t count = 0;
    for (uint32_t i = 0; i < g_hybrid_jit.profile_count && count < 20; i++) {
        if (g_hybrid_jit.profiles[i].is_hot) {
            dynarec_log(LOG_INFO, 
                        "  0x%lx: exec=%llu, avg=%uns, opt=%d, hot=%d, needs_recompile=%d\n",
                        (unsigned long)g_hybrid_jit.profiles[i].block_addr,
                        (unsigned long long)g_hybrid_jit.profiles[i].execution_count,
                        g_hybrid_jit.profiles[i].avg_execution_time_ns,
                        g_hybrid_jit.profiles[i].optimization_level,
                        g_hybrid_jit.profiles[i].is_hot,
                        g_hybrid_jit.profiles[i].needs_recompile);
            count++;
        }
    }
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
}

void Box64HybridJIT_ResetStats(void) {
    if (!g_hybrid_jit.initialized) return;
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    g_hybrid_jit.total_executions = 0;
    g_hybrid_jit.hot_block_executions = 0;
    g_hybrid_jit.total_compilations = 0;
    g_hybrid_jit.recompilations = 0;
    g_hybrid_jit.hot_compilations = 0;
    g_hybrid_jit.aggressive_jit_time_ns = 0;
    
    // Reset per-block stats
    for (uint32_t i = 0; i < g_hybrid_jit.profile_count; i++) {
        g_hybrid_jit.profiles[i].execution_count = 0;
        g_hybrid_jit.profiles[i].consecutive_hot_count = 0;
        g_hybrid_jit.profiles[i].is_hot = false;
        g_hybrid_jit.profiles[i].needs_recompile = false;
    }
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
}

void Box64HybridJIT_Destroy(void) {
    if (!g_hybrid_jit.initialized) return;
    
    pthread_mutex_lock(&g_hybrid_jit_mutex);
    
    if (g_hybrid_jit.profiles) {
        free(g_hybrid_jit.profiles);
        g_hybrid_jit.profiles = NULL;
    }
    
    g_hybrid_jit.profile_count = 0;
    g_hybrid_jit.profile_capacity = 0;
    g_hybrid_jit.initialized = false;
    
    pthread_mutex_unlock(&g_hybrid_jit_mutex);
}

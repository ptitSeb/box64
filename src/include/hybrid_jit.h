#ndef __BOX64_HYBRID_JIT_H_
#define __BOX64_HYBRID_JIT_H_

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// Hybrid JIT System for Box64
// Dual-mode JIT: Fast JIT + Hot-block Aggressive JIT
// ============================================================================

// JIT optimization levels
typedef enum {
    JIT_OPT_NONE = 0,           // No optimization (interpreter fallback)
    JIT_OPT_FAST = 1,           // Fast JIT (default, low overhead)
    JIT_OPT_NORMAL = 2,         // Normal JIT with common optimizations
    JIT_OPT_HOT = 3,            // Hot block JIT with aggressive optimization
    JIT_OPT_PROFILE = 4,       // Profile-guided optimization
} jit_opt_level_t;

// Block execution tracking
typedef struct block_profile {
    uintptr_t block_addr;
    uint64_t execution_count;
    uint64_t last_execution_time;
    uint32_t avg_execution_time_ns;
    uint32_t min_execution_time_ns;
    uint32_t max_execution_time_ns;
    uint32_t cache_hits;
    uint32_t cache_misses;
    bool is_hot;
    bool needs_recompile;
    uint8_t optimization_level;  // Current optimization level
    uint8_t consecutive_hot_count;  // Times block was detected as hot
} block_profile_t;

// Hybrid JIT configuration
typedef struct {
    // Hot block thresholds
    uint64_t hot_threshold_fast;     // Executions before marking as hot
    uint64_t hot_threshold_aggressive; // Executions before aggressive JIT
    
    // Optimization triggers
    uint32_t min_execution_count;   // Minimum executions before optimization
    uint32_t consecutive_hot_count;  // Consecutive hot detections before recompile
    
    // Code cache limits
    size_t max_fast_cache_size;     // Maximum fast JIT cache size
    size_t max_hot_cache_size;      // Maximum hot JIT cache size
    size_t max_total_cache_size;     // Maximum total cache size
    
    // Performance targets
    uint32_t target_frame_time_ns;  // Target time per frame (e.g., 16666666 for 60fps)
    uint32_t max_jit_time_per_frame_ns; // Max time to spend JITing per frame
    
    // Thermal/power aware
    bool enable_thermal_throttling;  // Reduce JIT aggressiveness when hot
    bool enable_power_throttling;   // Reduce JIT aggressiveness when on battery
    
    // Debug/Stats
    bool enable_stats;
    bool enable_profiling;
} hybrid_jit_config_t;

// Frame context for frame-aware optimization
typedef enum {
    FRAME_TYPE_GAMEPLAY = 0,
    FRAME_TYPE_UI_MENU = 1,
    FRAME_TYPE_LOADING = 2,
    FRAME_TYPE_CUTSCENE = 3,
    FRAME_TYPE_UNKNOWN = 4,
} frame_type_t;

typedef struct {
    frame_type_t type;
    uint64_t frame_number;
    uint64_t frame_start_time_ns;
    uint64_t frame_end_time_ns;
    uint32_t blocks_executed;
    uint32_t blocks_compiled;
    uint32_t blocks_recompiled;
    uint32_t jit_time_ns;
} frame_context_t;

// Block priority for cache management
typedef enum {
    BLOCK_PRIORITY_LOW = 0,
    BLOCK_PRIORITY_NORMAL = 1,
    BLOCK_PRIORITY_HIGH = 2,
    BLOCK_PRIORITY_CRITICAL = 3,
} block_priority_t;

// Global hybrid JIT state
typedef struct {
    hybrid_jit_config_t config;
    
    // Profiling data
    block_profile_t* profiles;
    uint32_t profile_count;
    uint32_t profile_capacity;
    
    // Cache management
    size_t fast_cache_size;
    size_t hot_cache_size;
    uint32_t total_compilations;
    uint32_t recompilations;
    uint32_t hot_compilations;
    
    // Frame tracking
    frame_context_t current_frame;
    frame_type_t last_frame_type;
    uint64_t last_frame_time_ns;
    
    // Thermal/Power state
    bool thermal_throttling;
    bool power_throttling;
    
    // Statistics
    uint64_t total_executions;
    uint64_t hot_block_executions;
    uint64_t aggressive_jit_time_ns;
    
    // Control
    bool initialized;
    bool profiling_enabled;
} hybrid_jit_state_t;

extern hybrid_jit_state_t g_hybrid_jit;

// ============================================================================
// Hybrid JIT API
// ============================================================================

// Initialize hybrid JIT system
void Box64HybridJIT_Init(void);

// Record block execution (called for every executed x86 block)
void Box64HybridJIT_RecordExecution(uintptr_t block_addr, uint32_t execution_time_ns);

// Check if a block should be recompiled with higher optimization
bool Box64HybridJIT_ShouldRecompile(uintptr_t block_addr);

// Get recommended optimization level for a block
jit_opt_level_t Box64HybridJIT_GetOptimizationLevel(uintptr_t block_addr);

// Mark block for recompilation
void Box64HybridJIT_MarkForRecompile(uintptr_t block_addr);

// Get block priority based on execution patterns
block_priority_t Box64HybridJIT_GetBlockPriority(uintptr_t block_addr);

// Frame-aware operations
void Box64HybridJIT_BeginFrame(frame_type_t type);
void Box64HybridJIT_EndFrame(void);
frame_type_t Box64HybridJIT_DetectFrameType(void);

// Cache management
size_t Box64HybridJIT_GetFastCacheSize(void);
size_t Box64HybridJIT_GetHotCacheSize(void);
void Box64HybridJIT_CleanupCache(void);

// Thermal/Power callbacks (non-polling)
void Box64HybridJIT_SetThermalState(bool throttling);
void Box64HybridJIT_SetPowerState(bool on_battery);

// Configuration
const hybrid_jit_config_t* Box64HybridJIT_GetConfig(void);
void Box64HybridJIT_SetConfig(const hybrid_jit_config_t* config);

// Statistics
void Box64HybridJIT_GetStats(
    uint64_t* total_executions,
    uint64_t* hot_executions,
    uint32_t* total_compilations,
    uint32_t* recompilations,
    uint32_t* hot_compilations,
    uint64_t* jit_time_ns
);

// Profiling (for development/debug)
void Box64HybridJIT_EnableProfiling(bool enable);
void Box64HybridJIT_DumpProfiles(void);
void Box64HybridJIT_ResetStats(void);

// Cleanup
void Box64HybridJIT_Destroy(void);

// ============================================================================
// Convenience functions
// ============================================================================

// Check if JIT should be aggressive based on frame type
static inline bool Box64HybridJIT_ShouldBeAggressive(void) {
    if (g_hybrid_jit.thermal_throttling) return false;
    if (g_hybrid_jit.power_throttling) return false;
    
    // More aggressive during gameplay, less during menus
    return (g_hybrid_jit.current_frame.type == FRAME_TYPE_GAMEPLAY ||
            g_hybrid_jit.current_frame.type == FRAME_TYPE_CUTSCENE);
}

// Get time budget for JIT compilation in current frame
static inline uint32_t Box64HybridJIT_GetTimeBudget(void) {
    if (g_hybrid_jit.thermal_throttling) return g_hybrid_jit.config.max_jit_time_per_frame_ns / 4;
    if (g_hybrid_jit.power_throttling) return g_hybrid_jit.config.max_jit_time_per_frame_ns / 2;
    if (g_hybrid_jit.current_frame.type == FRAME_TYPE_GAMEPLAY) {
        return g_hybrid_jit.config.max_jit_time_per_frame_ns / 2; // 50% of frame budget
    }
    return g_hybrid_jit.config.max_jit_time_per_frame_ns; // Full budget during loading/menus
}

#endif // __BOX64_HYBRID_JIT_H_

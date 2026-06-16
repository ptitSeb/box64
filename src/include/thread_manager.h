#ifndef __BOX64_THREAD_MANAGER_H_
#define __BOX64_THREAD_MANAGER_H_

#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

// ============================================================================
// Thread Manager for Box64
// Big.LITTLE Thread Pinning System
// ============================================================================

// Thread types for affinity assignment
typedef enum {
    THREAD_TYPE_MAIN_EMU = 0,      // Main emulation thread
    THREAD_TYPE_JIT_COMPILER,      // JIT compilation threads
    THREAD_TYPE_HELPER,            // Background/helper threads
    THREAD_TYPE_DYNAREC,           // Dynamic recompiler threads
    THREAD_TYPE_SIGNAL,            // Signal handling threads
} thread_type_t;

// Thread affinity mode
typedef enum {
    AFFINITY_AUTO = 0,             // Automatic based on SOC
    AFFINITY_P_CORES_ONLY,         // All threads to P-cores
    AFFINITY_E_CORES_ONLY,         // All threads to E-cores
    AFFINITY_BALANCED,             // Balanced P and E cores
    AFFINITY_DISABLED,             // No affinity (system default)
} affinity_mode_t;

// Thread info structure
typedef struct thread_info {
    pthread_t thread;
    thread_type_t type;
    uint32_t assigned_cores[8];
    uint32_t num_cores;
    bool pinned;
    struct thread_info* next;
} thread_info_t;

// Thread manager configuration
typedef struct {
    affinity_mode_t mode;
    bool respect_thermal;
    bool allow_migration;
    uint32_t thermal_threshold;     // Temperature in millidegrees
    uint32_t max_p_cores_for_jit;   // Max P-cores for JIT threads
    uint32_t max_p_cores_for_emu;   // Max P-cores for emulation
} thread_manager_config_t;

// Global thread manager
typedef struct {
    thread_manager_config_t config;
    thread_info_t* threads;
    pthread_mutex_t mutex;
    bool initialized;
    
    // SOC-aware core assignments
    uint32_t p_core_mask;
    uint32_t e_core_mask;
    uint32_t all_core_mask;
    
    // For balanced mode
    uint32_t next_p_core_index;
    uint32_t next_e_core_index;
    
    // Thermal state
    bool thermal_throttling_active;
    uint32_t throttle_count;
} thread_manager_t;

extern thread_manager_t g_thread_manager;

// ============================================================================
// Thread Manager API
// ============================================================================

// Initialize thread manager (called once at startup)
void Box64ThreadManager_Init(void);

// Register a thread with the manager
int Box64ThreadManager_Register(pthread_t thread, thread_type_t type);

// Unregister a thread
int Box64ThreadManager_Unregister(pthread_t thread);

// Set thread affinity based on type
int Box64ThreadManager_SetAffinity(pthread_t thread, thread_type_t type);

// Pin current thread to appropriate core
int Box64ThreadManager_PinCurrentThread(thread_type_t type);

// Set affinity mode
void Box64ThreadManager_SetMode(affinity_mode_t mode);
affinity_mode_t Box64ThreadManager_GetMode(void);

// Update thermal state (called occasionally, not continuously)
void Box64ThreadManager_UpdateThermalState(bool throttling);

// Check if migration is allowed
bool Box64ThreadManager_CanMigrate(void);

// Force rebalancing of threads
void Box64ThreadManager_Rebalance(void);

// Get thread statistics
void Box64ThreadManager_GetStats(uint32_t* pinned_count, uint32_t* migrated_count);

// Cleanup
void Box64ThreadManager_Destroy(void);

// ============================================================================
// Convenience macros for common operations
// ============================================================================

// Pin main emulation thread to P-cores
#define Box64ThreadManager_PinEmuThread() Box64ThreadManager_PinCurrentThread(THREAD_TYPE_MAIN_EMU)

// Pin JIT thread to P-cores
#define Box64ThreadManager_PinJITThread() Box64ThreadManager_PinCurrentThread(THREAD_TYPE_JIT_COMPILER)

// Pin helper thread to E-cores
#define Box64ThreadManager_PinHelperThread() Box64ThreadManager_PinCurrentThread(THREAD_TYPE_HELPER)

// ============================================================================
// SOC-specific thread pinning strategies
// ============================================================================

// Qualcomm Snapdragon strategy
void Box64ThreadManager_SnapdragonStrategy(void);

// MediaTek Dimensity strategy  
void Box64ThreadManager_DimensityStrategy(void);

// Generic big.LITTLE strategy
void Box64ThreadManager_BigLittleStrategy(void);

// Check if we should use P-cores for given thread type
bool Box64ThreadManager_ShouldUsePCores(thread_type_t type);

#endif // __BOX64_THREAD_MANAGER_H_

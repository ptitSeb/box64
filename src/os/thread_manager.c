#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "thread_manager.h"
#include "soc.h"
#include "debug.h"

// ============================================================================
// Thread Manager Implementation
// Big.LITTLE Thread Pinning for Box64
// ============================================================================

thread_manager_t g_thread_manager = {
    .initialized = false,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

static pthread_once_t g_thread_manager_init_once = PTHREAD_ONCE_INIT;

// ============================================================================
// Internal helpers
// ============================================================================

static void build_core_masks(void) {
    const soc_info_t* soc = Box64SOC_GetInfo();
    
    g_thread_manager.all_core_mask = 0;
    g_thread_manager.p_core_mask = 0;
    g_thread_manager.e_core_mask = 0;
    
    uint32_t core_id = 0;
    uint32_t* p_cores;
    uint32_t p_count;
    Box64SOC_GetPCores(&p_cores, &p_count);
    
    uint32_t* e_cores;
    uint32_t e_count;
    Box64SOC_GetECores(&e_cores, &e_count);
    
    for (uint32_t i = 0; i < p_count; i++) {
        if (p_cores[i] < 32) {
            g_thread_manager.p_core_mask |= (1U << p_cores[i]);
        }
    }
    
    for (uint32_t i = 0; i < e_count; i++) {
        if (e_cores[i] < 32) {
            g_thread_manager.e_core_mask |= (1U << e_cores[i]);
        }
    }
    
    g_thread_manager.all_core_mask = g_thread_manager.p_core_mask | g_thread_manager.e_core_mask;
    
    // If no cores detected, use all available cores
    if (g_thread_manager.all_core_mask == 0) {
        uint32_t nproc = sysconf(_SC_NPROCESSORS_CONF);
        if (nproc > 0 && nproc <= 32) {
            g_thread_manager.all_core_mask = (1U << nproc) - 1;
            g_thread_manager.p_core_mask = g_thread_manager.all_core_mask;
        }
    }
}

static void apply_affinity_mask(pthread_t thread, uint32_t mask) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    
    for (uint32_t i = 0; i < 32; i++) {
        if (mask & (1U << i)) {
            CPU_SET(i, &cpuset);
        }
    }
    
    // Try to set affinity for the thread
    int ret = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (ret != 0) {
        #ifdef DEBUG
        dynarec_log(LOG_DEBUG, "Box64ThreadManager: Failed to set affinity for thread %lu (err=%d)\n", 
                    (unsigned long)thread, ret);
        #endif
    }
}

static uint32_t select_core_for_thread_type(thread_type_t type) {
    const soc_info_t* soc = Box64SOC_GetInfo();
    
    // If thermal throttling, prefer E-cores for non-critical threads
    if (g_thread_manager.thermal_throttling_active && 
        (type == THREAD_TYPE_HELPER || type == THREAD_TYPE_DYNAREC)) {
        // Use E-cores when throttling
    }
    
    switch (type) {
        case THREAD_TYPE_MAIN_EMU:
        case THREAD_TYPE_JIT_COMPILER: {
            // P-cores for main emulation and JIT
            uint32_t* p_cores;
            uint32_t p_count;
            Box64SOC_GetPCores(&p_cores, &p_count);
            
            if (p_count > 0) {
                uint32_t idx = g_thread_manager.next_p_core_index % p_count;
                g_thread_manager.next_p_core_index++;
                return p_cores[idx];
            }
            // Fallback to first available core
            return 0;
        }
        
        case THREAD_TYPE_HELPER:
        case THREAD_TYPE_SIGNAL: {
            // E-cores for helpers and signals
            uint32_t* e_cores;
            uint32_t e_count;
            Box64SOC_GetECores(&e_cores, &e_count);
            
            if (e_count > 0) {
                uint32_t idx = g_thread_manager.next_e_core_index % e_count;
                g_thread_manager.next_e_core_index++;
                return e_cores[idx];
            }
            // Fallback to P-cores if no E-cores
            uint32_t* p_cores;
            uint32_t p_count;
            Box64SOC_GetPCores(&p_cores, &p_count);
            if (p_count > 1) { // Leave at least one P-core for emulation
                return p_cores[p_count - 1];
            }
            return 0;
        }
        
        case THREAD_TYPE_DYNAREC: {
            // DynaRec can use any core, but prefer P-cores
            uint32_t* p_cores;
            uint32_t p_count;
            Box64SOC_GetPCores(&p_cores, &p_count);
            
            if (p_count > 0) {
                return p_cores[0];
            }
            return 0;
        }
        
        default:
            return 0;
    }
}

static void select_cores_for_thread_type(thread_type_t type, uint32_t* cores, uint32_t* count) {
    const soc_info_t* soc = Box64SOC_GetInfo();
    *count = 0;
    
    switch (type) {
        case THREAD_TYPE_MAIN_EMU:
        case THREAD_TYPE_JIT_COMPILER: {
            // Use P-cores exclusively
            uint32_t* p_cores;
            uint32_t p_count;
            Box64SOC_GetPCores(&p_cores, &p_count);
            
            uint32_t max_cores = (type == THREAD_TYPE_MAIN_EMU) ? 
                                 g_thread_manager.config.max_p_cores_for_emu : 
                                 g_thread_manager.config.max_p_cores_for_jit;
            
            if (max_cores == 0 || max_cores > p_count) max_cores = p_count;
            
            for (uint32_t i = 0; i < max_cores && i < 8; i++) {
                cores[i] = p_cores[i];
                (*count)++;
            }
            break;
        }
        
        case THREAD_TYPE_HELPER:
        case THREAD_TYPE_SIGNAL: {
            // Use E-cores if available, else P-cores
            uint32_t* e_cores;
            uint32_t e_count;
            Box64SOC_GetECores(&e_cores, &e_count);
            
            if (e_count > 0) {
                for (uint32_t i = 0; i < e_count && i < 8; i++) {
                    cores[i] = e_cores[i];
                    (*count)++;
                }
            } else {
                uint32_t* p_cores;
                uint32_t p_count;
                Box64SOC_GetPCores(&p_cores, &p_count);
                
                if (p_count > 1) {
                    for (uint32_t i = 1; i < p_count && i < 8; i++) {
                        cores[i - 1] = p_cores[i];
                        (*count)++;
                    }
                }
            }
            break;
        }
        
        default:
            // Use all cores
            for (uint32_t i = 0; i < soc->total_cores && i < 8; i++) {
                cores[i] = i;
                (*count)++;
            }
            break;
    }
}

// ============================================================================
// SOC-specific strategies
// ============================================================================

void Box64ThreadManager_SnapdragonStrategy(void) {
    const soc_info_t* soc = Box64SOC_GetInfo();
    
    if (!soc->is_snapdragon) return;
    
    // Snapdragon: typical big.LITTLE layout
    // Snapdragon 8 Gen 3: 1x X4 + 3x A720 + 2x A520
    // Snapdragon 8 Gen 2: 1x X3 + 2x A715 + 2x A710 + 3x A510
    // Snapdragon 8 Gen 1: 1x X2 + 3x A710 + 4x A510
    
    // For Snapdragon, we can be more aggressive with P-core allocation
    // since Snapdragon's P-cores handle SMT better
    
    g_thread_manager.config.max_p_cores_for_emu = soc->p_cores;  // Allow all P-cores for emulation
    g_thread_manager.config.max_p_cores_for_jit = (soc->p_cores + 1) / 2;  // Half for JIT
    
    // Snapdragon's Prime core (X4/X3/X2) can be used for single-threaded optimization
    // But let's keep it available for burst workloads
    
    dynarec_log(LOG_DEBUG, "Box64ThreadManager: Applied Snapdragon strategy\n");
}

void Box64ThreadManager_DimensityStrategy(void) {
    const soc_info_t* soc = Box64SOC_GetInfo();
    
    if (!soc->is_dimensity) return;
    
    // MediaTek Dimensity: similar big.LITTLE but different power characteristics
    // Dimensity 9300: 1x X4 + 3x A720 + 4x A720 (no E-cores!)
    // Dimensity 9200: 1x X3 + 3x A715 + 4x A510
    // Dimensity 9000: 1x X2 + 3x A710 + 4x A510
    
    // Dimensity flagships have no E-cores in some configurations
    if (soc->e_cores == 0) {
        // All cores are "P-cores" in terms of allocation
        // But we can still differentiate by frequency in real-time
        g_thread_manager.config.max_p_cores_for_emu = soc->total_cores - 1;  // Leave 1 core
        g_thread_manager.config.max_p_cores_for_jit = soc->total_cores - 2;  // Leave 2 cores
    } else {
        // Standard allocation
        g_thread_manager.config.max_p_cores_for_emu = soc->p_cores;
        g_thread_manager.config.max_p_cores_for_jit = (soc->p_cores + 1) / 2;
    }
    
    // MediaTek CorePilot can handle thread migration better than manual pinning
    // But for consistent performance, we still want initial pinning
    
    dynarec_log(LOG_DEBUG, "Box64ThreadManager: Applied Dimensity strategy\n");
}

void Box64ThreadManager_BigLittleStrategy(void) {
    const soc_info_t* soc = Box64SOC_GetInfo();
    
    if (!Box64SOC_HasBigLittle()) return;
    
    // Generic big.LITTLE strategy
    // Use P-cores for emulation, E-cores for helpers
    
    uint32_t* p_cores;
    uint32_t p_count;
    Box64SOC_GetPCores(&p_cores, &p_count);
    
    uint32_t* e_cores;
    uint32_t e_count;
    Box64SOC_GetECores(&e_cores, &e_count);
    
    // For generic, be conservative: 75% of P-cores for emulation
    g_thread_manager.config.max_p_cores_for_emu = (p_count * 3 + 3) / 4;
    g_thread_manager.config.max_p_cores_for_jit = p_count / 2;
    
    dynarec_log(LOG_DEBUG, "Box64ThreadManager: Applied generic big.LITTLE strategy\n");
}

bool Box64ThreadManager_ShouldUsePCores(thread_type_t type) {
    if (g_thread_manager.config.mode == AFFINITY_DISABLED) return false;
    if (g_thread_manager.config.mode == AFFINITY_E_CORES_ONLY) return false;
    if (g_thread_manager.config.mode == AFFINITY_P_CORES_ONLY) return true;
    
    // AUTO mode
    switch (type) {
        case THREAD_TYPE_MAIN_EMU:
        case THREAD_TYPE_JIT_COMPILER:
            return true;
        case THREAD_TYPE_HELPER:
        case THREAD_TYPE_SIGNAL:
            return false;
        default:
            return true;
    }
}

// ============================================================================
// Initialization
// ============================================================================

static void thread_manager_init_internal(void) {
    if (g_thread_manager.initialized) return;
    
    pthread_mutex_lock(&g_thread_manager.mutex);
    
    if (g_thread_manager.initialized) {
        pthread_mutex_unlock(&g_thread_manager.mutex);
        return;
    }
    
    // Initialize SOC detection first
    Box64SOC_Init();
    
    // Build core masks
    build_core_masks();
    
    // Set default configuration
    memset(&g_thread_manager.config, 0, sizeof(g_thread_manager.config));
    g_thread_manager.config.mode = AFFINITY_AUTO;
    g_thread_manager.config.respect_thermal = true;
    g_thread_manager.config.allow_migration = false;
    g_thread_manager.config.thermal_threshold = 80000; // 80°C
    g_thread_manager.config.max_p_cores_for_emu = 4;
    g_thread_manager.config.max_p_cores_for_jit = 2;
    
    g_thread_manager.thermal_throttling_active = false;
    g_thread_manager.throttle_count = 0;
    g_thread_manager.next_p_core_index = 0;
    g_thread_manager.next_e_core_index = 0;
    g_thread_manager.threads = NULL;
    
    // Apply SOC-specific strategy
    if (Box64SOC_IsSnapdragon()) {
        Box64ThreadManager_SnapdragonStrategy();
    } else if (Box64SOC_IsDimensity()) {
        Box64ThreadManager_DimensityStrategy();
    } else if (Box64SOC_HasBigLittle()) {
        Box64ThreadManager_BigLittleStrategy();
    }
    
    g_thread_manager.initialized = true;
    
    dynarec_log(LOG_INFO, "Box64ThreadManager initialized: P-mask=0x%X, E-mask=0x%X\n",
                g_thread_manager.p_core_mask, g_thread_manager.e_core_mask);
    
    pthread_mutex_unlock(&g_thread_manager.mutex);
}

void Box64ThreadManager_Init(void) {
    pthread_once(&g_thread_manager_init_once, thread_manager_init_internal);
}

// ============================================================================
// Thread Management API
// ============================================================================

int Box64ThreadManager_Register(pthread_t thread, thread_type_t type) {
    if (!g_thread_manager.initialized) Box64ThreadManager_Init();
    
    pthread_mutex_lock(&g_thread_manager.mutex);
    
    // Allocate new thread info
    thread_info_t* info = calloc(1, sizeof(thread_info_t));
    if (!info) {
        pthread_mutex_unlock(&g_thread_manager.mutex);
        return -1;
    }
    
    info->thread = thread;
    info->type = type;
    info->pinned = false;
    
    // Select cores for this thread type
    select_cores_for_thread_type(type, info->assigned_cores, &info->num_cores);
    
    // Apply affinity
    uint32_t mask = 0;
    for (uint32_t i = 0; i < info->num_cores; i++) {
        mask |= (1U << info->assigned_cores[i]);
    }
    
    if (mask) {
        apply_affinity_mask(thread, mask);
        info->pinned = true;
    }
    
    // Add to list
    info->next = g_thread_manager.threads;
    g_thread_manager.threads = info;
    
    pthread_mutex_unlock(&g_thread_manager.mutex);
    
    #ifdef DEBUG
    dynarec_log(LOG_DEBUG, "Box64ThreadManager: Registered thread %lu (type=%d), mask=0x%X\n",
                (unsigned long)thread, type, mask);
    #endif
    
    return 0;
}

int Box64ThreadManager_Unregister(pthread_t thread) {
    if (!g_thread_manager.initialized) return 0;
    
    pthread_mutex_lock(&g_thread_manager.mutex);
    
    thread_info_t** ptr = &g_thread_manager.threads;
    while (*ptr) {
        if ((*ptr)->thread == thread) {
            thread_info_t* to_free = *ptr;
            *ptr = to_free->next;
            free(to_free);
            pthread_mutex_unlock(&g_thread_manager.mutex);
            return 0;
        }
        ptr = &(*ptr)->next;
    }
    
    pthread_mutex_unlock(&g_thread_manager.mutex);
    return -1;
}

int Box64ThreadManager_SetAffinity(pthread_t thread, thread_type_t type) {
    if (!g_thread_manager.initialized) Box64ThreadManager_Init();
    if (g_thread_manager.config.mode == AFFINITY_DISABLED) return 0;
    
    uint32_t cores[8];
    uint32_t count;
    select_cores_for_thread_type(type, cores, &count);
    
    uint32_t mask = 0;
    for (uint32_t i = 0; i < count; i++) {
        mask |= (1U << cores[i]);
    }
    
    if (mask) {
        apply_affinity_mask(thread, mask);
    }
    
    return 0;
}

int Box64ThreadManager_PinCurrentThread(thread_type_t type) {
    if (!g_thread_manager.initialized) Box64ThreadManager_Init();
    if (g_thread_manager.config.mode == AFFINITY_DISABLED) return 0;
    
    pthread_t thread = pthread_self();
    
    return Box64ThreadManager_SetAffinity(thread, type);
}

void Box64ThreadManager_SetMode(affinity_mode_t mode) {
    if (!g_thread_manager.initialized) Box64ThreadManager_Init();
    
    pthread_mutex_lock(&g_thread_manager.mutex);
    g_thread_manager.config.mode = mode;
    pthread_mutex_unlock(&g_thread_manager.mutex);
    
    // Re-apply affinity to all threads if mode changed
    if (mode != AFFINITY_DISABLED) {
        Box64ThreadManager_Rebalance();
    }
}

affinity_mode_t Box64ThreadManager_GetMode(void) {
    if (!g_thread_manager.initialized) Box64ThreadManager_Init();
    return g_thread_manager.config.mode;
}

void Box64ThreadManager_UpdateThermalState(bool throttling) {
    if (!g_thread_manager.initialized) Box64ThreadManager_Init();
    
    pthread_mutex_lock(&g_thread_manager.mutex);
    
    if (throttling != g_thread_manager.thermal_throttling_active) {
        g_thread_manager.thermal_throttling_active = throttling;
        
        if (throttling) {
            g_thread_manager.throttle_count++;
            
            // When throttling, migrate non-critical threads to E-cores
            if (g_thread_manager.config.respect_thermal) {
                thread_info_t* t = g_thread_manager.threads;
                while (t) {
                    if (t->type == THREAD_TYPE_HELPER || t->type == THREAD_TYPE_SIGNAL) {
                        uint32_t mask = g_thread_manager.e_core_mask;
                        if (mask == 0) mask = g_thread_manager.all_core_mask;
                        apply_affinity_mask(t->thread, mask);
                    }
                    t = t->next;
                }
            }
            
            dynarec_log(LOG_INFO, "Box64ThreadManager: Thermal throttling detected, migrating threads\n");
        }
    }
    
    pthread_mutex_unlock(&g_thread_manager.mutex);
}

bool Box64ThreadManager_CanMigrate(void) {
    if (!g_thread_manager.initialized) Box64ThreadManager_Init();
    
    // Don't migrate during active gameplay if disabled
    if (!g_thread_manager.config.allow_migration) return false;
    
    // Don't migrate if thermal throttling (we've already migrated)
    if (g_thread_manager.thermal_throttling_active) return false;
    
    return true;
}

void Box64ThreadManager_Rebalance(void) {
    if (!g_thread_manager.initialized) return;
    
    pthread_mutex_lock(&g_thread_manager.mutex);
    
    thread_info_t* t = g_thread_manager.threads;
    while (t) {
        select_cores_for_thread_type(t->type, t->assigned_cores, &t->num_cores);
        
        uint32_t mask = 0;
        for (uint32_t i = 0; i < t->num_cores; i++) {
            mask |= (1U << t->assigned_cores[i]);
        }
        
        if (mask) {
            apply_affinity_mask(t->thread, mask);
        }
        
        t = t->next;
    }
    
    pthread_mutex_unlock(&g_thread_manager.mutex);
}

void Box64ThreadManager_GetStats(uint32_t* pinned_count, uint32_t* migrated_count) {
    if (!g_thread_manager.initialized) Box64ThreadManager_Init();
    
    pthread_mutex_lock(&g_thread_manager.mutex);
    
    *pinned_count = 0;
    *migrated_count = 0;
    
    thread_info_t* t = g_thread_manager.threads;
    while (t) {
        if (t->pinned) (*pinned_count)++;
        t = t->next;
    }
    
    *migrated_count = g_thread_manager.throttle_count;
    
    pthread_mutex_unlock(&g_thread_manager.mutex);
}

void Box64ThreadManager_Destroy(void) {
    if (!g_thread_manager.initialized) return;
    
    pthread_mutex_lock(&g_thread_manager.mutex);
    
    // Free all thread info
    thread_info_t* t = g_thread_manager.threads;
    while (t) {
        thread_info_t* next = t->next;
        free(t);
        t = next;
    }
    g_thread_manager.threads = NULL;
    g_thread_manager.initialized = false;
    
    pthread_mutex_unlock(&g_thread_manager.mutex);
}

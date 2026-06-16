#ifndef __BOX64_SOC_H_
#define __BOX64_SOC_H_

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// SOC (System on Chip) Detection for Box64
// Optimized for Snapdragon (Adreno) and MediaTek (Mali/Immortalis)
// ============================================================================

// SOC Vendor Types
typedef enum {
    SOC_VENDOR_UNKNOWN = 0,
    SOC_VENDOR_QUALCOMM,
    SOC_VENDOR_MEDIATEK,
    SOC_VENDOR_SAMSUNG,
    SOC_VENDOR_APPLE,
    SOC_VENDOR_HUAWEI,
} soc_vendor_t;

// Qualcomm Snapdragon Series
typedef enum {
    SNAPDRAGON_6XX = 0,       // 630, 636, 660, 670, 675
    SNAPDRAGON_7XX,           // 710, 712, 720, 730, 732, 750, 765
    SNAPDRAGON_8XX,           // 835, 845, 855, 855+, 865, 870
    SNAPDRAGON_8GEN1,         // 8 Gen 1, 8+ Gen 1
    SNAPDRAGON_8GEN2,         // 8 Gen 2
    SNAPDRAGON_8GEN3,         // 8 Gen 3
} snapdragon_series_t;

// MediaTek Dimensity Series
typedef enum {
    DIMENSITY_LOW,            // 700, 800, 820, 900
    DIMENSITY_MID,            // 1000, 1000+, 1100, 1200, 1300
    DIMENSITY_HIGH,           // 8000, 8100, 8200
    DIMENSITY_FLAGSHIP,       // 9000, 9000+, 9200, 9300, 9400
} dimensity_series_t;

// Generic ARM Core Types (for big.LITTLE classification)
typedef enum {
    CORE_TYPE_UNKNOWN = 0,
    CORE_TYPE_EFFICIENCY,     // A55, A510, LITTLE cores
    CORE_TYPE_PERFORMANCE,     // A76, A77, A78, A710, A715, A720, X1, X2, X3, X4
} core_type_t;

// SOC Information Structure
typedef struct {
    soc_vendor_t vendor;
    bool is_snapdragon;
    bool is_dimensity;
    bool is_exynos;
    
    // Qualcomm specific
    snapdragon_series_t snapdragon_series;
    uint32_t gpu_model;       // Adreno GPU model (e.g., 740)
    
    // MediaTek specific
    dimensity_series_t dimensity_series;
    uint32_t mali_gpu_class;   // Mali GPU class (e.g., G710, G720)
    
    // CPU Topology
    uint32_t total_cores;
    uint32_t p_cores;          // Performance cores
    uint32_t e_cores;          // Efficiency cores
    uint32_t max_cpu_freq;     // Max frequency in Hz
    uint32_t min_cpu_freq;     // Min frequency in Hz
    
    // Feature flags (cached at startup)
    bool has_sve;
    bool has_sve2;
    bool has_i8mm;
    bool has_bf16;
    bool has_fp16;
    bool has_dotprod;
    bool has_pmull;
    bool has_rcpc;
    
    // Memory info (snapshot)
    uint64_t total_memory;
    uint64_t available_memory;
    
    // Thermal state (snapshot)
    int thermal_zone_count;
    bool thermal_throttling;
    
    // Initialized flag
    bool initialized;
} soc_info_t;

// CPU Core Information
typedef struct {
    uint32_t cpu_id;
    uint32_t core_id;
    uint32_t cluster_id;
    core_type_t type;
    uint32_t max_freq;
    uint32_t min_freq;
    char core_name[64];
} cpu_core_info_t;

// Global SOC info (singleton)
extern soc_info_t g_soc_info;
extern cpu_core_info_t* g_cpu_cores;
extern uint32_t g_cpu_core_count;

// ============================================================================
// SOC Detection API
// ============================================================================

// Initialize SOC detection (called once at startup)
void Box64SOC_Init(void);

// Get cached SOC info (fast, no I/O)
const soc_info_t* Box64SOC_GetInfo(void);

// Get CPU core info
const cpu_core_info_t* Box64SOC_GetCoreInfo(uint32_t core_id);

// Get performance/efficiency core lists
void Box64SOC_GetPCores(uint32_t** cores, uint32_t* count);
void Box64SOC_GetECores(uint32_t** cores, uint32_t* count);

// Check if SOC supports specific features
bool Box64SOC_HasFeature(const char* feature);

// SOC-specific helpers
bool Box64SOC_IsSnapdragon(void);
bool Box64SOC_IsDimensity(void);
bool Box64SOC_IsExynos(void);

// Performance hints based on SOC
const char* Box64SOC_GetGPUName(void);
uint32_t Box64SOC_GetGPUPowerLevel(void);

// big.LITTLE helpers
bool Box64SOC_HasBigLittle(void);
uint32_t Box64SOC_GetRecommendedCores(uint32_t thread_type);

// Thread affinity helpers
int Box64SOC_SetThreadAffinity(uint32_t cpu_mask);
int Box64SOC_SetThreadAffinitySingle(uint32_t core_id);
int Box64SOC_GetCurrentCore(void);

// Thermal helpers (cached snapshot)
bool Box64SOC_IsThermalThrottling(void);
int Box64SOC_GetThermalZoneTemp(int zone_id);

// Memory helpers (cached snapshot)
uint64_t Box64SOC_GetAvailableMemory(void);
bool Box64SOC_IsLowMemory(void);

// Cleanup (if needed)
void Box64SOC_Destroy(void);

#endif // __BOX64_SOC_H_

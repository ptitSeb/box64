#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <dirent.h>

#include "soc.h"
#include "debug.h"
#include "box64context.h"

// ============================================================================
// SOC Detection Implementation
// Optimized for Snapdragon (Adreno) and MediaTek (Mali/Immortalis)
// One-time snapshot only - no continuous polling
// ============================================================================

soc_info_t g_soc_info = {0};
static cpu_core_info_t* g_cpu_cores_internal = NULL;
static uint32_t g_cpu_core_count_internal = 0;
static pthread_once_t g_soc_init_once = PTHREAD_ONCE_INIT;
static bool g_initialized = false;

// Helper: Trim whitespace from string
static char* trim(char* str) {
    if (!str) return str;
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;
    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) *end-- = '\0';
    return str;
}

// Helper: Read file content safely
static bool read_file_line(const char* path, char* buffer, size_t size) {
    FILE* f = fopen(path, "r");
    if (!f) return false;
    if (fgets(buffer, size, f)) {
        fclose(f);
        return true;
    }
    fclose(f);
    return false;
}

// Helper: Read file as integer
static bool read_file_int(const char* path, int* value) {
    FILE* f = fopen(path, "r");
    if (!f) return false;
    if (fscanf(f, "%d", value) == 1) {
        fclose(f);
        return true;
    }
    fclose(f);
    return false;
}

// Helper: Read file as uint64
static bool read_file_uint64(const char* path, uint64_t* value) {
    FILE* f = fopen(path, "r");
    if (!f) return false;
    if (fscanf(f, "%lu", value) == 1) {
        fclose(f);
        return true;
    }
    fclose(f);
    return false;
}

// ============================================================================
// CPU Topology Detection
// ============================================================================

static int parse_cpu_topology(void) {
    // Parse /sys/devices/system/cpu/ for topology information
    uint32_t max_cpus = sysconf(_SC_NPROCESSORS_CONF);
    if (max_cpus == (uint32_t)-1 || max_cpus == 0) max_cpus = 4;
    
    // Allocate core info array
    g_cpu_cores_internal = calloc(max_cpus, sizeof(cpu_core_info_t));
    if (!g_cpu_cores_internal) return -1;
    
    uint32_t found_cores = 0;
    char path[256];
    char buffer[256];
    FILE* f;
    
    // Try to detect cluster topology from cpuinfo
    // First, read /proc/cpuinfo to get processor info
    f = fopen("/proc/cpuinfo", "r");
    if (f) {
        char line[512];
        uint32_t current_processor = 0;
        char processor_name[128] = {0};
        
        while (fgets(line, sizeof(line), f) && found_cores < max_cpus) {
            if (strncmp(line, "processor", 9) == 0) {
                char* colon = strchr(line, ':');
                if (colon) {
                    sscanf(colon + 1, "%u", &current_processor);
                }
            } else if (strstr(line, "Hardware") || strstr(line, "model name") || strstr(line, "model name")) {
                char* colon = strchr(line, ':');
                if (colon) {
                    strncpy(processor_name, trim(colon + 1), sizeof(processor_name) - 1);
                }
            } else if (strncmp(line, "\n", 1) == 0 || strlen(line) == 0) {
                // End of processor entry, save it
                if (current_processor < max_cpus && processor_name[0]) {
                    cpu_core_info_t* core = &g_cpu_cores_internal[found_cores];
                    core->cpu_id = current_processor;
                    core->core_id = current_processor;
                    core->cluster_id = 0;
                    strncpy(core->core_name, processor_name, sizeof(core->core_name) - 1);
                    found_cores++;
                }
                memset(processor_name, 0, sizeof(processor_name));
            }
        }
        fclose(f);
    }
    
    // Try /sys/devices/system/cpu/cpu*/topology/ for detailed topology
    for (uint32_t i = 0; i < max_cpus && found_cores < max_cpus; i++) {
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/topology/core_id", i);
        int core_id = -1;
        if (read_file_int(path, &core_id)) {
            // Core exists
            snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/topology/thread_siblings_list", i);
            int thread_siblings = -1;
            if (read_file_int(path, &thread_siblings)) {
                // Has thread siblings
            }
        }
        
        // Also check for cpu frequencies to determine P/E core
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", i);
        int max_freq = 0;
        if (read_file_int(path, &max_freq)) {
            if (i < found_cores) {
                g_cpu_cores_internal[i].max_freq = max_freq * 1000; // Convert to Hz
                
                // Determine core type based on frequency
                // Generally: P-cores run at higher frequencies than E-cores
                // P-cores: typically > 2.0 GHz
                // E-cores: typically < 2.0 GHz
                if (max_freq > 2000) {
                    g_cpu_cores_internal[i].type = CORE_TYPE_PERFORMANCE;
                } else {
                    g_cpu_cores_internal[i].type = CORE_TYPE_EFFICIENCY;
                }
            }
        }
    }
    
    // If we didn't find detailed info, try to classify based on CPU name
    if (g_cpu_cores_internal) {
        for (uint32_t i = 0; i < found_cores; i++) {
            char* name = g_cpu_cores_internal[i].core_name;
            // Android/Dimensity/Snapdragon typically report core names
            if (strstr(name, "A55") || strstr(name, "A510") || strstr(name, "Cortex-A55")) {
                g_cpu_cores_internal[i].type = CORE_TYPE_EFFICIENCY;
            } else if (strstr(name, "A76") || strstr(name, "A77") || strstr(name, "A78") || 
                       strstr(name, "A710") || strstr(name, "A715") || strstr(name, "A720") ||
                       strstr(name, "X1") || strstr(name, "X2") || strstr(name, "X3") ||
                       strstr(name, "Kryo") || strstr(name, "Cortex-A7")) {
                g_cpu_cores_internal[i].type = CORE_TYPE_PERFORMANCE;
            } else {
                // Unknown, try to determine from frequency if available
                if (g_cpu_cores_internal[i].max_freq > 2000000) { // > 2GHz in Hz
                    g_cpu_cores_internal[i].type = CORE_TYPE_PERFORMANCE;
                } else {
                    g_cpu_cores_internal[i].type = CORE_TYPE_EFFICIENCY;
                }
            }
        }
    }
    
    g_cpu_core_count_internal = found_cores;
    
    // Count P-cores and E-cores
    uint32_t p_count = 0, e_count = 0;
    for (uint32_t i = 0; i < found_cores; i++) {
        if (g_cpu_cores_internal[i].type == CORE_TYPE_PERFORMANCE) p_count++;
        else if (g_cpu_cores_internal[i].type == CORE_TYPE_EFFICIENCY) e_count++;
    }
    
    g_soc_info.p_cores = p_count;
    g_soc_info.e_cores = e_count;
    g_soc_info.total_cores = found_cores;
    
    return 0;
}

// ============================================================================
// SOC Vendor Detection
// ============================================================================

static void detect_soc_vendor(void) {
    char buffer[512] = {0};
    FILE* f;
    
    // Try /proc/cpuinfo first
    f = fopen("/proc/cpuinfo", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            // Look for Hardware or Model name fields
            if (strstr(line, "Hardware") || strstr(line, "model name") || strstr(line, "Model")) {
                char* colon = strchr(line, ':');
                if (colon) {
                    strncpy(buffer, trim(colon + 1), sizeof(buffer) - 1);
                    break;
                }
            }
        }
        fclose(f);
    }
    
    // Also try uname for kernel info
    struct utsname uts;
    if (uname(&uts) == 0) {
        // Sometimes the machine name contains SoC info
        if (buffer[0] == '\0') {
            strncpy(buffer, uts.machine, sizeof(buffer) - 1);
        }
    }
    
    // Detect Qualcomm Snapdragon
    if (strstr(buffer, "Snapdragon") || strstr(buffer, "SM-") || 
        strstr(buffer, "kryo") || strstr(buffer, "Qualcomm")) {
        g_soc_info.vendor = SOC_VENDOR_QUALCOMM;
        g_soc_info.is_snapdragon = true;
        
        // Detect Snapdragon series
        if (strstr(buffer, "SM8")) {
            // Could be 8 Gen series
            if (strstr(buffer, "SM8650") || strstr(buffer, "SM8635") || strstr(buffer, "SM8550"))
                g_soc_info.snapdragon_series = SNAPDRAGON_8GEN3;
            else if (strstr(buffer, "SM8550") || strstr(buffer, "SM8575"))
                g_soc_info.snapdragon_series = SNAPDRAGON_8GEN2;
            else if (strstr(buffer, "SM8450") || strstr(buffer, "SM8475"))
                g_soc_info.snapdragon_series = SNAPDRAGON_8GEN1;
            else
                g_soc_info.snapdragon_series = SNAPDRAGON_8XX;
        } else if (strstr(buffer, "SM7")) {
            g_soc_info.snapdragon_series = SNAPDRAGON_7XX;
        } else if (strstr(buffer, "SM6")) {
            g_soc_info.snapdragon_series = SNAPDRAGON_6XX;
        } else {
            // Default to best guess based on Kryo name
            if (strstr(buffer, "Kryo") && strstr(buffer, "Gold"))
                g_soc_info.snapdragon_series = SNAPDRAGON_8XX;
            else if (strstr(buffer, "Kryo") && strstr(buffer, "Silver"))
                g_soc_info.snapdragon_series = SNAPDRAGON_7XX;
            else
                g_soc_info.snapdragon_series = SNAPDRAGON_7XX; // Safe default
        }
        
        // Try to detect Adreno GPU
        FILE* gpu_f = fopen("/sys/class/kgsl/kgsl-3d0/gpu_model", "r");
        if (!gpu_f) gpu_f = fopen("/sys/class/devfreq/gpu-0/device/name", "r");
        if (!gpu_f) gpu_f = fopen("/sys/class/drm/card0/device/gpu_name", "r");
        if (gpu_f) {
            if (fgets(buffer, sizeof(buffer), gpu_f)) {
                if (strstr(buffer, "Adreno") || strstr(buffer, "adreno")) {
                    char* num = strstr(buffer, "6");
                    if (num && strlen(num) >= 3) {
                        g_soc_info.gpu_model = atoi(num);
                    } else {
                        num = strstr(buffer, "7");
                        if (num && strlen(num) >= 3) {
                            g_soc_info.gpu_model = atoi(num);
                        }
                    }
                }
            }
            fclose(gpu_f);
        }
        
        return;
    }
    
    // Detect MediaTek Dimensity
    if (strstr(buffer, "Dimensity") || strstr(buffer, "MT6") || 
        strstr(buffer, "MT7") || strstr(buffer, "MT8") || strstr(buffer, "MT9") ||
        strstr(buffer, "MediaTek") || strstr(buffer, "MTK")) {
        g_soc_info.vendor = SOC_VENDOR_MEDIATEK;
        g_soc_info.is_dimensity = true;
        
        // Detect Dimensity series
        if (strstr(buffer, "Dimensity 9") || strstr(buffer, "Dimensity 10") || 
            strstr(buffer, "MT6890") || strstr(buffer, "MT6983") || strstr(buffer, "MT6990")) {
            g_soc_info.dimensity_series = DIMENSITY_FLAGSHIP;
        } else if (strstr(buffer, "Dimensity 8") || strstr(buffer, "MT6895") || 
                   strstr(buffer, "MT6891")) {
            g_soc_info.dimensity_series = DIMENSITY_HIGH;
        } else if (strstr(buffer, "Dimensity 1") || strstr(buffer, "MT6873") ||
                   strstr(buffer, "MT6883") || strstr(buffer, "MT6885") || strstr(buffer, "MT6889")) {
            g_soc_info.dimensity_series = DIMENSITY_MID;
        } else {
            // Default to low-end for older/basic chips
            g_soc_info.dimensity_series = DIMENSITY_LOW;
        }
        
        // Try to detect Mali GPU class
        FILE* gpu_f = fopen("/sys/class/kgsl/kgsl-3d0/gpu_model", "r");
        if (!gpu_f) gpu_f = fopen("/sys/class/devfreq/gpu-0/device/name", "r");
        if (!gpu_f) gpu_f = fopen("/sys/class/drm/card0/device/gpu_name", "r");
        if (gpu_f) {
            if (fgets(buffer, sizeof(buffer), gpu_f)) {
                if (strstr(buffer, "Mali") || strstr(buffer, "mali")) {
                    // Extract Mali class (G710, G720, etc)
                    char* g = strstr(buffer, "G7");
                    if (g) {
                        g_soc_info.mali_gpu_class = atoi(g + 1);
                    } else {
                        g = strstr(buffer, "G6");
                        if (g) g_soc_info.mali_gpu_class = 600 + atoi(g + 1);
                    }
                }
            }
            fclose(gpu_f);
        }
        
        return;
    }
    
    // Detect Samsung Exynos
    if (strstr(buffer, "Exynos") || strstr(buffer, "exynos")) {
        g_soc_info.vendor = SOC_VENDOR_SAMSUNG;
        g_soc_info.is_exynos = true;
        return;
    }
    
    // Unknown vendor
    g_soc_info.vendor = SOC_VENDOR_UNKNOWN;
}

// ============================================================================
// ARM Feature Detection
// ============================================================================

static void detect_arm_features(void) {
    // Parse /proc/cpuinfo for ARM features
    FILE* f = fopen("/proc/cpuinfo", "r");
    if (!f) return;
    
    char line[512];
    uint32_t features = 0;
    
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "Features") || strstr(line, "ASIMD") || strstr(line, "fp")) {
            char* colon = strchr(line, ':');
            if (colon) {
                // Check for specific features
                if (strstr(colon, "asimd")) features |= (1 << 0);  // SIMD/NEON
                if (strstr(colon, "fp16")) features |= (1 << 1);   // FP16
                if (strstr(colon, "dotprod")) features |= (1 << 2); // Dot product
                if (strstr(colon, "pmull")) features |= (1 << 3);   // PMULL
                if (strstr(colon, "sve")) features |= (1 << 4);    // SVE
                if (strstr(colon, "sve2")) features |= (1 << 5);   // SVE2
                if (strstr(colon, "i8mm")) features |= (1 << 6);   // Int8 Matrix Multiply
                if (strstr(colon, "bf16")) features |= (1 << 7);   // BFloat16
                if (strstr(colon, "rcpc")) features |= (1 << 8);  // RCPC
                if (strstr(colon, "crc")) features |= (1 << 9);    // CRC32
            }
        }
    }
    fclose(f);
    
    g_soc_info.has_fp16 = (features >> 1) & 1;
    g_soc_info.has_dotprod = (features >> 2) & 1;
    g_soc_info.has_pmull = (features >> 3) & 1;
    g_soc_info.has_sve = (features >> 4) & 1;
    g_soc_info.has_sve2 = (features >> 5) & 1;
    g_soc_info.has_i8mm = (features >> 6) & 1;
    g_soc_info.has_bf16 = (features >> 7) & 1;
    g_soc_info.has_rcpc = (features >> 8) & 1;
}

// ============================================================================
// Memory Info (Snapshot)
// ============================================================================

static void read_memory_info(void) {
    FILE* f = fopen("/proc/meminfo", "r");
    if (!f) return;
    
    char line[256];
    uint64_t mem_total = 0, mem_available = 0;
    
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line + 9, "%lu", &mem_total);
            mem_total *= 1024; // Convert from kB to bytes
        } else if (strncmp(line, "MemAvailable:", 13) == 0) {
            sscanf(line + 13, "%lu", &mem_available);
            mem_available *= 1024; // Convert from kB to bytes
        }
    }
    fclose(f);
    
    g_soc_info.total_memory = mem_total;
    g_soc_info.available_memory = mem_available;
}

// ============================================================================
// Thermal Info (Snapshot)
// ============================================================================

static void read_thermal_info(void) {
    // Count thermal zones
    DIR* dir = opendir("/sys/class/thermal");
    if (!dir) return;
    
    int zone_count = 0;
    bool throttling = false;
    char path[256];
    char buffer[64];
    int temp;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "thermal_zone", 13) == 0) {
            zone_count++;
            
            // Check temperature
            snprintf(path, sizeof(path), "/sys/class/thermal/%s/temp", entry->d_name);
            if (read_file_int(path, &temp)) {
                // Check if temperature is above 80°C (potential throttling)
                if (temp > 80000) throttling = true;
            }
        }
    }
    closedir(dir);
    
    g_soc_info.thermal_zone_count = zone_count;
    g_soc_info.thermal_throttling = throttling;
}

// ============================================================================
// CPU Frequency Detection
// ============================================================================

static void read_cpu_frequency(void) {
    uint32_t max_freq = 0;
    uint32_t min_freq = UINT32_MAX;
    char path[128];
    int freq;
    
    for (int i = 0; i < 8; i++) { // Check first 8 CPUs
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", i);
        if (read_file_int(path, &freq) && freq > 0) {
            if ((uint32_t)freq > max_freq) max_freq = freq;
        }
        
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_min_freq", i);
        if (read_file_int(path, &freq) && freq > 0) {
            if ((uint32_t)freq < min_freq) min_freq = freq;
        }
    }
    
    if (max_freq > 0) {
        g_soc_info.max_cpu_freq = max_freq * 1000; // Convert to Hz
    }
    if (min_freq < UINT32_MAX) {
        g_soc_info.min_cpu_freq = min_freq * 1000; // Convert to Hz
    }
}

// ============================================================================
// Main Initialization
// ============================================================================

static void box64_soc_init_internal(void) {
    if (g_initialized) return;
    
    // Zero out the global structure
    memset(&g_soc_info, 0, sizeof(g_soc_info));
    
    // Detect SOC vendor
    detect_soc_vendor();
    
    // Parse CPU topology
    parse_cpu_topology();
    
    // Detect ARM features
    detect_arm_features();
    
    // Read memory info (snapshot)
    read_memory_info();
    
    // Read thermal info (snapshot)
    read_thermal_info();
    
    // Read CPU frequency
    read_cpu_frequency();
    
    g_initialized = true;
    
    // Log detected info in debug mode
    #ifdef DEBUG
    dynarec_log(LOG_INFO, "BOX64 SOC Detection:\n");
    dynarec_log(LOG_INFO, "  Vendor: %d\n", g_soc_info.vendor);
    dynarec_log(LOG_INFO, "  Snapdragon: %s\n", g_soc_info.is_snapdragon ? "yes" : "no");
    dynarec_log(LOG_INFO, "  Dimensity: %s\n", g_soc_info.is_dimensity ? "yes" : "no");
    dynarec_log(LOG_INFO, "  Total cores: %u\n", g_soc_info.total_cores);
    dynarec_log(LOG_INFO, "  P-cores: %u, E-cores: %u\n", g_soc_info.p_cores, g_soc_info.e_cores);
    dynarec_log(LOG_INFO, "  Max freq: %u MHz\n", g_soc_info.max_cpu_freq / 1000000);
    dynarec_log(LOG_INFO, "  SVE: %s, SVE2: %s, I8MM: %s, BF16: %s\n",
                g_soc_info.has_sve ? "yes" : "no",
                g_soc_info.has_sve2 ? "yes" : "no",
                g_soc_info.has_i8mm ? "yes" : "no",
                g_soc_info.has_bf16 ? "yes" : "no");
    #endif
}

void Box64SOC_Init(void) {
    pthread_once(&g_soc_init_once, box64_soc_init_internal);
}

// ============================================================================
// API Implementation
// ============================================================================

const soc_info_t* Box64SOC_GetInfo(void) {
    if (!g_initialized) Box64SOC_Init();
    return &g_soc_info;
}

const cpu_core_info_t* Box64SOC_GetCoreInfo(uint32_t core_id) {
    if (!g_initialized) Box64SOC_Init();
    if (core_id >= g_cpu_core_count_internal) return NULL;
    return &g_cpu_cores_internal[core_id];
}

bool Box64SOC_IsSnapdragon(void) {
    if (!g_initialized) Box64SOC_Init();
    return g_soc_info.is_snapdragon;
}

bool Box64SOC_IsDimensity(void) {
    if (!g_initialized) Box64SOC_Init();
    return g_soc_info.is_dimensity;
}

bool Box64SOC_IsExynos(void) {
    if (!g_initialized) Box64SOC_Init();
    return g_soc_info.is_exynos;
}

bool Box64SOC_HasBigLittle(void) {
    if (!g_initialized) Box64SOC_Init();
    return (g_soc_info.p_cores > 0 && g_soc_info.e_cores > 0);
}

const char* Box64SOC_GetGPUName(void) {
    if (!g_initialized) Box64SOC_Init();
    if (g_soc_info.is_snapdragon) {
        static char name[32];
        snprintf(name, sizeof(name), "Adreno %u", g_soc_info.gpu_model);
        return name;
    } else if (g_soc_info.is_dimensity) {
        static char name[32];
        snprintf(name, sizeof(name), "Mali-G%u", g_soc_info.mali_gpu_class);
        return name;
    }
    return "Unknown GPU";
}

uint32_t Box64SOC_GetGPUPowerLevel(void) {
    if (!g_initialized) Box64SOC_Init();
    // Estimate power level based on GPU model
    if (g_soc_info.is_snapdragon) {
        // Adreno 6xx: 2-5W, 7xx: 5-10W
        if (g_soc_info.gpu_model >= 700) return 10;
        if (g_soc_info.gpu_model >= 600) return 5;
        return 2;
    } else if (g_soc_info.is_dimensity) {
        // Mali G710+: 4-8W
        if (g_soc_info.mali_gpu_class >= 710) return 8;
        if (g_soc_info.mali_gpu_class >= 600) return 4;
        return 2;
    }
    return 5; // Default
}

bool Box64SOC_HasFeature(const char* feature) {
    if (!g_initialized) Box64SOC_Init();
    if (!feature) return false;
    
    if (strcmp(feature, "sve") == 0) return g_soc_info.has_sve;
    if (strcmp(feature, "sve2") == 0) return g_soc_info.has_sve2;
    if (strcmp(feature, "i8mm") == 0) return g_soc_info.has_i8mm;
    if (strcmp(feature, "bf16") == 0) return g_soc_info.has_bf16;
    if (strcmp(feature, "fp16") == 0) return g_soc_info.has_fp16;
    if (strcmp(feature, "dotprod") == 0) return g_soc_info.has_dotprod;
    if (strcmp(feature, "pmull") == 0) return g_soc_info.has_pmull;
    
    return false;
}

void Box64SOC_GetPCores(uint32_t** cores, uint32_t* count) {
    if (!g_initialized) Box64SOC_Init();
    static uint32_t p_cores_list[16];
    uint32_t n = 0;
    
    for (uint32_t i = 0; i < g_cpu_core_count_internal && n < 16; i++) {
        if (g_cpu_cores_internal[i].type == CORE_TYPE_PERFORMANCE) {
            p_cores_list[n++] = i;
        }
    }
    
    *cores = p_cores_list;
    *count = n;
}

void Box64SOC_GetECores(uint32_t** cores, uint32_t* count) {
    if (!g_initialized) Box64SOC_Init();
    static uint32_t e_cores_list[16];
    uint32_t n = 0;
    
    for (uint32_t i = 0; i < g_cpu_core_count_internal && n < 16; i++) {
        if (g_cpu_cores_internal[i].type == CORE_TYPE_EFFICIENCY) {
            e_cores_list[n++] = i;
        }
    }
    
    *cores = e_cores_list;
    *count = n;
}

uint32_t Box64SOC_GetRecommendedCores(uint32_t thread_type) {
    if (!g_initialized) Box64SOC_Init();
    // thread_type: 0 = main, 1 = JIT, 2 = helper
    if (thread_type == 0) { // Main emulation thread -> P-cores
        return g_soc_info.p_cores > 0 ? g_soc_info.p_cores : g_soc_info.total_cores;
    } else if (thread_type == 1) { // JIT threads -> P-cores
        return g_soc_info.p_cores > 0 ? g_soc_info.p_cores : g_soc_info.total_cores;
    } else { // Helper threads -> E-cores
        return g_soc_info.e_cores > 0 ? g_soc_info.e_cores : g_soc_info.total_cores;
    }
}

bool Box64SOC_IsThermalThrottling(void) {
    if (!g_initialized) Box64SOC_Init();
    return g_soc_info.thermal_throttling;
}

int Box64SOC_GetThermalZoneTemp(int zone_id) {
    char path[128];
    int temp;
    snprintf(path, sizeof(path), "/sys/class/thermal/thermal_zone%d/temp", zone_id);
    if (read_file_int(path, &temp)) {
        return temp; // Temperature in millidegrees
    }
    return -1;
}

uint64_t Box64SOC_GetAvailableMemory(void) {
    if (!g_initialized) Box64SOC_Init();
    return g_soc_info.available_memory;
}

bool Box64SOC_IsLowMemory(void) {
    if (!g_initialized) Box64SOC_Init();
    // Low memory if less than 2GB available
    return g_soc_info.available_memory < (2ULL * 1024 * 1024 * 1024);
}

void Box64SOC_Destroy(void) {
    if (g_cpu_cores_internal) {
        free(g_cpu_cores_internal);
        g_cpu_cores_internal = NULL;
    }
    g_cpu_core_count_internal = 0;
    g_initialized = false;
    memset(&g_soc_info, 0, sizeof(g_soc_info));
}

// ============================================================================
// Thread Affinity (POSIX/Linux/Android)
// ============================================================================

int Box64SOC_SetThreadAffinity(uint32_t cpu_mask) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    
    for (uint32_t i = 0; i < 32; i++) {
        if (cpu_mask & (1U << i)) {
            CPU_SET(i, &cpuset);
        }
    }
    
    return sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
}

int Box64SOC_SetThreadAffinitySingle(uint32_t core_id) {
    return Box64SOC_SetThreadAffinity(1U << core_id);
}

int Box64SOC_GetCurrentCore(void) {
    return sched_getcpu();
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "box64context.h"
#include "debug.h"
#include "core.h"
#include "fileutils.h"
#include "json.h"
#include "elfloader.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "emu/x87emu_private.h"
#include "box64cpu.h"
#include "box64cpu_util.h"
#include "x64trace.h"
#include "build_info.h"

#define NASM      "nasm"
#define X86_64_LD "x86_64-linux-gnu-ld"

extern box64context_t* my_context;
extern FILE* ftrace;

uint64_t regs[16] = { 0 };
uint64_t ymmregs[16][4] = { { 0 } };
uint64_t mmregs[8] = { 0 };
uint64_t flags = 0;

bool check_regs[16] = { 0 };
bool check_xmmregs[16] = { 0 };
bool check_ymmregs[16] = { 0 };
bool check_mmregs[8] = { 0 };
bool check_flags = false;

int cputype = 0;

const char* regname[] = { "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI", "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15" };

#define MAX_MEMORY_REGIONS   32
#define MAX_MEMORY_DATA      32
#define MAX_MEMORY_DATA_SIZE 4096

struct {
    uint64_t start;
    uint64_t size;
} memory_regions[MAX_MEMORY_REGIONS] = { { 0 } };

struct {
    uint64_t start;
    uint64_t size;
    uint8_t data[MAX_MEMORY_DATA_SIZE];
} memory_data[MAX_MEMORY_DATA] = { { 0 } };

static inline uint64_t fromstr(const char* str)
{
    size_t len = strlen(str);
    int base = len > 2 && (str[1] == 'x' || str[1] == 'X') ? 16 : 10;
    size_t j = 0;
    char clean[len + 1];

    for (size_t i = 0; i < len; ++i)
        if (str[i] != '_')
            clean[j++] = str[i];
    clean[j] = '\0';

    return strtoull(clean, NULL, base);
}

static struct json_value_s* json_find(struct json_object_s* object, const char* key)
{
    struct json_object_element_s* element = object->start;
    while (element) {
        if (strcmp(element->name->string, key) == 0) {
            return element->value;
        }
        element = element->next;
    }
    return NULL;
}

static void json_fill_array(struct json_array_s* array, uint64_t* values)
{
    struct json_array_element_s* element = array->start;
    int i = 0;
    while (element) {
        if (element->value->type == json_type_string) {
            struct json_string_s* string = (struct json_string_s*)element->value->payload;
            values[i] = fromstr(string->string);
            i++;
        }
        element = element->next;
    }
}

static struct json_value_s* extractTestConfig(const char* filepath)
{
    FILE* file = fopen(filepath, "r");
    if (!file) return NULL;

    char* line = NULL;
    size_t line_capacity = 0;
    char* json = calloc(4096, 1);
    size_t json_capacity = 4096;
    size_t json_length = 0;
    bool in_config = false;

    ssize_t line_length = 0;
    while ((line_length = getline(&line, &line_capacity, file)) != -1) {
        if (!strcmp(line, "%ifdef CONFIG\n")) {
            in_config = true;
            continue;
        } else if (!strcmp(line, "%endif\n")) {
            in_config = false;
            break;
        }
        if (in_config) {
            size_t chunk_length = (size_t)line_length;
            size_t needed = json_length + chunk_length + 1;
            if (needed > json_capacity) {
                while (json_capacity < needed)
                    json_capacity *= 2;
                json = realloc(json, json_capacity);
            }
            memcpy(json + json_length, line, chunk_length);
            json_length += chunk_length;
            json[json_length] = '\0';
        }
    }

    free(line);
    fclose(file);
    struct json_value_s* config = json_parse(json, json_length);
    free(json);
    return config;
}

#define MAX_TEST_VARS           32
#define MAX_TEST_VAR_VALUES     16

struct test_var_combos {
    int num_vars;
    char* names[MAX_TEST_VARS];
    int num_values[MAX_TEST_VARS];
    char* values[MAX_TEST_VARS][MAX_TEST_VAR_VALUES];
    int total_combos;
};

static struct test_var_combos getTestVariableCombos(const char* filepath)
{
    struct test_var_combos combos = { 0 };
    combos.total_combos = 1;

    struct json_value_s* config = extractTestConfig(filepath);
    if (!config || config->type != json_type_object) {
        free(config);
        return combos;
    }

    struct json_value_s* vars = json_find(config->payload, "Variables");
    if (vars && vars->type == json_type_object) {
        struct json_object_s* object = (struct json_object_s*)vars->payload;
        struct json_object_element_s* element = object->start;
        while (element && combos.num_vars < MAX_TEST_VARS) {
            int v = combos.num_vars;
            combos.names[v] = strdup(element->name->string);
            if (element->value->type == json_type_string) {
                struct json_string_s* value = (struct json_string_s*)element->value->payload;
                combos.values[v][0] = strdup(value->string);
                combos.num_values[v] = 1;
            } else if (element->value->type == json_type_array) {
                struct json_array_s* array = (struct json_array_s*)element->value->payload;
                struct json_array_element_s* arr_elem = array->start;
                int vi = 0;
                while (arr_elem && vi < MAX_TEST_VAR_VALUES) {
                    if (arr_elem->value->type == json_type_string) {
                        struct json_string_s* val = (struct json_string_s*)arr_elem->value->payload;
                        combos.values[v][vi] = strdup(val->string);
                        vi++;
                    }
                    arr_elem = arr_elem->next;
                }
                combos.num_values[v] = vi;
            }
            if (combos.num_values[v] > 0) {
                combos.total_combos *= combos.num_values[v];
                combos.num_vars++;
            } else {
                free(combos.names[v]);
            }
            element = element->next;
        }
    }

    free(config);
    return combos;
}

static int isDynarecOff(struct test_var_combos* combos, int combo_index)
{
    int stride = 1;
    for (int v = combos->num_vars - 1; v >= 0; v--) {
        int idx = (combo_index / stride) % combos->num_values[v];
        if (!strcmp(combos->names[v], "BOX64_DYNAREC") && !strcmp(combos->values[v][idx], "0"))
            return 1;
        stride *= combos->num_values[v];
    }
    return 0;
}

static int shouldSkipCombo(struct test_var_combos* combos, int combo_index)
{
    if (!isDynarecOff(combos, combo_index))
        return 0;
    int stride = 1;
    for (int v = combos->num_vars - 1; v >= 0; v--) {
        int idx = (combo_index / stride) % combos->num_values[v];
        // when BOX64_DYNAREC=0, only keep the one where all BOX64_DYNAREC_* vars are at first value
        if (!strncmp(combos->names[v], "BOX64_DYNAREC_", 14) && idx != 0)
            return 1;
        stride *= combos->num_values[v];
    }
    return 0;
}

static void applyTestVarCombo(struct test_var_combos* combos, int combo_index)
{
    int dynarec_off = isDynarecOff(combos, combo_index);
    int stride = 1;
    for (int v = combos->num_vars - 1; v >= 0; v--) {
        int value_index = (combo_index / stride) % combos->num_values[v];
        if (!dynarec_off || strncmp(combos->names[v], "BOX64_DYNAREC_", 14))
            setenv(combos->names[v], combos->values[v][value_index], 1);
        stride *= combos->num_values[v];
    }
}

static void printTestVarCombo(struct test_var_combos* combos, int combo_index, int display_index, int display_total)
{
    int dynarec_off = isDynarecOff(combos, combo_index);
    int indices[MAX_TEST_VARS];
    int stride = 1;
    for (int v = combos->num_vars - 1; v >= 0; v--) {
        indices[v] = (combo_index / stride) % combos->num_values[v];
        stride *= combos->num_values[v];
    }
    fprintf(stdout, "\033[44;37m[BOX64] Running combination %d/%d:", display_index, display_total);
    for (int v = 0; v < combos->num_vars; v++) {
        if (!dynarec_off || strncmp(combos->names[v], "BOX64_DYNAREC_", 14))
            fprintf(stdout, " %s=%s", combos->names[v], combos->values[v][indices[v]]);
    }
    fprintf(stdout, "\033[0m\n");
    fflush(stdout);
}

static void freeTestVarCombos(struct test_var_combos* combos)
{
    for (int v = 0; v < combos->num_vars; v++) {
        free(combos->names[v]);
        for (int i = 0; i < combos->num_values[v]; i++)
            free(combos->values[v][i]);
    }
}

static void loadTest(const char** filepath, const char* include_path)
{
    struct json_value_s* config = extractTestConfig(*filepath);
    if (!config || config->type != json_type_object) {
        printf_log(LOG_NONE, "Failed to parse JSON configuration.\n");
        exit(1);
    }


    struct json_value_s* regdata = json_find(config->payload, "RegData");
    int i = 0;

#define REG(name)                                                                   \
    if (regdata && regdata->type == json_type_object) {                             \
        struct json_value_s* r##name = json_find(regdata->payload, #name);          \
        if (r##name && r##name->type == json_type_string) {                         \
            struct json_string_s* string = (struct json_string_s*)r##name->payload; \
            regs[i] = fromstr(string->string);                                      \
            check_regs[i] = true;                                                   \
        }                                                                           \
        i++;                                                                        \
    }

    REG(RAX);
    REG(RCX);
    REG(RDX);
    REG(RBX);
    REG(RSP);
    REG(RBP);
    REG(RSI);
    REG(RDI);
    REG(R8);
    REG(R9);
    REG(R10);
    REG(R11);
    REG(R12);
    REG(R13);
    REG(R14);
    REG(R15);
#undef REG

    i = 0;

#define REG(name)                                                                \
    if (regdata && regdata->type == json_type_object) {                          \
        struct json_value_s* r##name = json_find(regdata->payload, #name);       \
        if (r##name && r##name->type == json_type_array) {                       \
            struct json_array_s* array = (struct json_array_s*)r##name->payload; \
            json_fill_array(array, ymmregs[i]);                                  \
            check_xmmregs[i] = true;                                             \
            if (array->length == 4) check_ymmregs[i] = true;                     \
        }                                                                        \
        i++;                                                                     \
    }

    REG(XMM0);
    REG(XMM1);
    REG(XMM2);
    REG(XMM3);
    REG(XMM4);
    REG(XMM5);
    REG(XMM6);
    REG(XMM7);
    REG(XMM8);
    REG(XMM9);
    REG(XMM10);
    REG(XMM11);
    REG(XMM12);
    REG(XMM13);
    REG(XMM14);
    REG(XMM15);
#undef REG

    i = 0;

#define REG(name)                                                                   \
    if (regdata && regdata->type == json_type_object) {                             \
        struct json_value_s* r##name = json_find(regdata->payload, #name);          \
        if (r##name && r##name->type == json_type_string) {                         \
            struct json_string_s* string = (struct json_string_s*)r##name->payload; \
            mmregs[i] = fromstr(string->string);                                    \
            check_mmregs[i] = true;                                                 \
        }                                                                           \
        i++;                                                                        \
    }

    REG(MM0);
    REG(MM1);
    REG(MM2);
    REG(MM3);
    REG(MM4);
    REG(MM5);
    REG(MM6);
    REG(MM7);
#undef REG

    if (regdata && regdata->type == json_type_object) {
        struct json_value_s* flags_value = json_find(regdata->payload, "Flags");
        if (flags_value && flags_value->type == json_type_string) {
            struct json_string_s* string = (struct json_string_s*)flags_value->payload;
            flags = fromstr(string->string);
            check_flags = true;
        }
    }

    struct json_value_s* mode = json_find(config->payload, "Mode");
    if (mode && mode->type == json_type_string && !strcmp(((struct json_string_s*)mode->payload)->string, "32BIT")) {
        box64_is32bits = true;
        printf_log(LOG_INFO, "Test is in 32bits mode\n");
    }

    struct json_value_s* cputype_value = json_find(config->payload, "CpuType");
    if (cputype_value && cputype_value->type == json_type_string && !strcasecmp(((struct json_string_s*)cputype_value->payload)->string, "amd"))
        cputype = 1; // 0 -> Intel[default], 1 -> AMD
    else
        cputype = 0;

    struct json_value_s* json_memory_regions = json_find(config->payload, "MemoryRegions");
    if (json_memory_regions && json_memory_regions->type == json_type_object) {
        struct json_object_s* object = (struct json_object_s*)json_memory_regions->payload;
        struct json_object_element_s* element = object->start;
        i = 0;
        while (element && i < MAX_MEMORY_REGIONS) {
            struct json_string_s* element_key = element->name;
            struct json_value_s* value = element->value;
            assert(value->type == json_type_string);
            struct json_string_s* element_value = (struct json_string_s*)value->payload;
            memory_regions[i].start = fromstr(element_key->string);
            memory_regions[i].size = fromstr(element_value->string);
            i++;
            element = element->next;
        }
    }

    struct json_value_s* json_memory_data = json_find(config->payload, "MemoryData");
    if (json_memory_data && json_memory_data->type == json_type_object) {
        struct json_object_s* object = (struct json_object_s*)json_memory_data->payload;
        struct json_object_element_s* element = object->start;
        i = 0;
        while (element && i < MAX_MEMORY_DATA) {
            struct json_string_s* element_key = element->name;
            struct json_value_s* value = element->value;
            assert(value->type == json_type_string);
            struct json_string_s* element_value = (struct json_string_s*)value->payload;
            memory_data[i].start = fromstr(element_key->string);
            uint8_t* data = (uint8_t*)element_value->string;
            while (*data) {
                if (*data == ' ') {
                    ++data;
                    continue;
                }
                if (*data == '0' && (*(data + 1) == 'x' || *(data + 1) == 'X')) {
                    data += 2;
                }
                uint8_t* lastbyte = data;
                while (*lastbyte && *lastbyte != ' ' && *(lastbyte + 1) && *(lastbyte + 1) != ' ') {
                    lastbyte += 2;
                }
                size_t len = lastbyte - data;
                if (len < 2) {
                    printf_log(LOG_NONE, "Invalid MemoryData item %s\n", element_key->string);
                    break;
                }
                lastbyte -= 2;
                static uint8_t byte_str[3] = { 0 };
                while (lastbyte >= data) {
                    byte_str[0] = *lastbyte;
                    byte_str[1] = *(lastbyte + 1);
                    memory_data[i].data[memory_data[i].size] = (uint8_t)strtol((const char*)byte_str, NULL, 16);
                    memory_data[i].size++;
                    if (memory_data[i].size >= MAX_MEMORY_DATA_SIZE) {
                        printf_log(LOG_NONE, "MemoryData item %s too big (max=%d bytes)\n", element_key->string, MAX_MEMORY_DATA_SIZE);
                        break;
                    }
                    lastbyte -= 2;
                }
                data += len;
            }
            i++;
            element = element->next;
        }
    }

#define BINNAME "/tmp/binfileXXXXXX"
    char* binname = box_malloc(strlen(BINNAME) + 1);
    memcpy(binname, BINNAME, strlen(BINNAME) + 1);
#undef BINNAME
    char objname[] = "/tmp/objfileXXXXXX";

    int fd;
    fd = mkstemp(objname);
    close(fd);

    const char* nasm_cmd[] = { NASM, *filepath, box64_is32bits ? "-felf32" : "-felf64", "-o", objname,
        include_path ? "-i" : NULL, include_path ? include_path : NULL, NULL };
    pid_t fork_result = fork();
    if (fork_result == 0) {
        execvp(nasm_cmd[0], (char* const*)nasm_cmd);
        exit(1);
    } else {
        int status;
        waitpid(fork_result, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf_log(LOG_NONE, NASM " failed with exit code: %d\n", WEXITSTATUS(status));
            unlink(objname);
            exit(1);
        }
    }

    close(mkstemp(binname));
    const char* ld_cmd[] = { X86_64_LD, objname, "-Ttext=0x10000", "-n", "-w", "-m", box64_is32bits ? "elf_i386" : "elf_x86_64", "-o", binname, NULL };

    fork_result = fork();
    if (fork_result == 0) {
        execvp(ld_cmd[0], (char* const*)ld_cmd);
        exit(1);
    } else {
        int status;
        waitpid(fork_result, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf_log(LOG_NONE, X86_64_LD " failed with exit code: %d\n", WEXITSTATUS(status));
            unlink(objname);
            exit(1);
        }
    }

    printf_log(LOG_DEBUG, "Test binary compiled as %s\n", binname);
    *filepath = binname;

    unlink(objname);
    free(config);
}

static void setupZydis(box64context_t* context)
{
#ifdef HAVE_TRACE
    if ((BOX64ENV(trace_init) && strcmp(BOX64ENV(trace_init), "0")) || (BOX64ENV(trace) && strcmp(BOX64ENV(trace), "0"))) {
        context->x64trace = 1;
    }
    if (context->x64trace) {
        printf_log(LOG_INFO, "Initializing Zydis lib\n");
        if (InitX64Trace(context)) {
            printf_log(LOG_INFO, "Zydis init failed. No x86 trace activated\n");
            context->x64trace = 0;
        }
    }
#endif
}

static int runSingleTest(const char* filepath, const char* include_path)
{
    box64_pagesize = sysconf(_SC_PAGESIZE);
    LoadEnvVariables();
    InitializeSystemInfo();
    ftrace = stdout;
    if (!BOX64ENV(nobanner)) PrintBox64Version(1);

#ifdef DYNAREC
    if (DetectHostCpuFeatures())
        PrintHostCpuFeatures();
    else {
        printf_log(LOG_INFO, "Minimum CPU requirements not met, disabling DynaRec\n");
        SET_BOX64ENV(dynarec, 0);
    }
#endif

    if (!box64env.is_cputype_overridden && cputype) SET_BOX64ENV(cputype, cputype);

    PrintEnvVariables(&box64env, LOG_INFO);
    my_context = NewBox64Context(0);

    loadTest(&filepath, include_path); // will modify filepath to point to the binary file
    my_context->fullpath = box_strdup(filepath);

    FILE* f = fopen(my_context->fullpath, "rb");
    unlink(my_context->fullpath);

    elfheader_t* elf_header = LoadAndCheckElfHeader(f, my_context->fullpath, 1);
    if (!elf_header) {
        printf_log(LOG_NONE, "Error: failed to load ELF header from test binary\n");
        exit(1);
    }
    AddElfHeader(my_context, elf_header);
    CalcLoadAddr(elf_header);
    AllocLoadElfMemory(my_context, elf_header, 1);

    setupZydis(my_context);
    my_context->ep = GetEntryPoint(my_context->maplib, elf_header);

    my_context->stack = (void*)0xC0000000;
    my_context->stacksz = 4096;
    mmap((void*)my_context->stack, my_context->stacksz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    mmap((void*)0xE0000000, 16 * 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    mmap((void*)0xE8000000, 32 * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

    for (int i = 0; i < MAX_MEMORY_REGIONS && memory_regions[i].size; ++i) {
        if (!memory_regions[i].start) break;
        mmap((void*)memory_regions[i].start, memory_regions[i].size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    }

    for (int i = 0; i < MAX_MEMORY_DATA && memory_data[i].size; ++i) {
        if (!memory_data[i].start) break;
        if (!memory_data[i].size) continue;
        memcpy((void*)memory_data[i].start, memory_data[i].data, memory_data[i].size);
    }

    x64emu_t* emu = NewX64Emu(my_context, my_context->ep,
        (uintptr_t)my_context->stack, my_context->stacksz, 0);

    bool have_mmregs = false;
    for (int i = 0; i < 8; ++i) {
        if (!check_mmregs[i])
            continue;
        emu->mmx[i].q = mmregs[i];
        emu->x87[i].q = mmregs[i];
        have_mmregs = true;
    }
    if (have_mmregs) {
        emu->top = 0;
        emu->fpu_stack = 8;
        emu->fpu_tags = 0;
    }

    ResetFlags(emu);
    SetRIP(emu, my_context->ep);
    DynaRun(emu);

    int retcode = 0;
    for (int i = 0; i < 16; ++i) {
        if (check_regs[i]) {
            if (regs[i] != emu->regs[_RAX + i].q[0]) {
                printf_log(LOG_NONE, "%-5s: expected %016zx, got %016zx\n", regname[i], regs[i], emu->regs[_RAX + i].q[0]);
                retcode += 1;
            }
        }
    }

    for (int i = 0; i < 16; ++i) {
        if (check_xmmregs[i]) {
            if (ymmregs[i][0] != emu->xmm[i].q[0] || ymmregs[i][1] != emu->xmm[i].q[1]) {
                printf_log(LOG_NONE, "XMM%-2d: expected %016zx-%016zx, got %016zx-%016zx\n", i, ymmregs[i][1], ymmregs[i][0], emu->xmm[i].q[1], emu->xmm[i].q[0]);
                retcode += 1;
            }
        }
        if (check_ymmregs[i]) {
            if (ymmregs[i][2] != emu->ymm[i].q[0] || ymmregs[i][3] != emu->ymm[i].q[1]) {
                printf_log(LOG_NONE, "YMM%-2d: expected %016zx-%016zx, got %016zx-%016zx\n", i, ymmregs[i][3], ymmregs[i][2], emu->ymm[i].q[1], emu->ymm[i].q[0]);
                retcode += 1;
            }
        }
    }

    if (check_flags) {
        if (flags != emu->eflags.x64) {
            printf_log(LOG_NONE, "FLAGS: expected %016zx, got %016zx\n", flags, emu->eflags.x64);
            retcode += 1;
        }
    }

    if (retcode == 0)
        printf_log(LOG_INFO, "Passed\n");
    else
        printf_log(LOG_NONE, "Failed with %d errors\n", retcode);

    return retcode;
}

int unittest(int argc, const char** argv)
{
    const char* filepath = NULL;
    const char* include_path = NULL;
    int combo_number = -1;  // -1 means run all

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--test") || !strcmp(argv[i], "-t")) {
            if (i + 1 < argc)
                filepath = argv[++i];
        } else if (!strcmp(argv[i], "--include") || !strcmp(argv[i], "-i")) {
            if (i + 1 < argc)
                include_path = argv[++i];
        } else if (!strcmp(argv[i], "--number") || !strcmp(argv[i], "-n")) {
            if (i + 1 < argc)
                combo_number = atoi(argv[++i]);
        }
    }

    if (!filepath) {
        fprintf(stdout, "Usage: %s -t <filepath> [-i <include>] [-n <combo_number>]\n", argv[0]);
        return 0;
    }

    struct test_var_combos combos = getTestVariableCombos(filepath);

    int actual_total = 0;
    for (int c = 0; c < combos.total_combos; c++) {
        if (!shouldSkipCombo(&combos, c))
            actual_total++;
    }

    if (combo_number != -1) {
        if (combo_number < 1 || combo_number > actual_total) {
            fprintf(stderr, "[BOX64] Error: combination number %d is out of range (valid: 1-%d)\n", combo_number, actual_total);
            freeTestVarCombos(&combos);
            return 1;
        }
    }

    int retcode = 0;
    int simple_run = (combo_number != -1) || (actual_total == 1);
    int display_index = 0;
    for (int c = 0; c < combos.total_combos; c++) {
        if (shouldSkipCombo(&combos, c))
            continue;
        display_index++;
        if (combo_number != -1 && display_index != combo_number)
            continue;
        if (actual_total > 1)
            printTestVarCombo(&combos, c, display_index, actual_total);
        if (simple_run) {
            applyTestVarCombo(&combos, c);
            if (runSingleTest(filepath, include_path) != 0)
                retcode++;
        } else {
            pid_t pid = fork();
            if (pid == 0) {
                applyTestVarCombo(&combos, c);
                freeTestVarCombos(&combos);
                _exit(runSingleTest(filepath, include_path));
            } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status)) {
                    if (WEXITSTATUS(status) != 0) retcode++;
                } else {
                    retcode++;
                }
            } else {
                fprintf(stderr, "fork() failed\n");
                retcode++;
            }
        }
        if (combo_number != -1)
            break;
    }

    int combos_run = (combo_number != -1) ? 1 : actual_total;
    if (combos_run > 1) {
        if (retcode == 0)
            fprintf(stdout, "[BOX64] All %d combinations passed\n", combos_run);
        else
            fprintf(stdout, "[BOX64] %d out of %d combinations failed\n", retcode, combos_run);
    }

    freeTestVarCombos(&combos);
    return retcode ? 1 : 0;
}

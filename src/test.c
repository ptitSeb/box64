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

bool check_regs[16] = { 0 };
bool check_xmmregs[16] = { 0 };
bool check_ymmregs[16] = { 0 };

const char* regname[] = { "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI", "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15" };

#define MAX_MEMORY_REGIONS 32
#define MAX_MEMORY_DATA      32
#define MAX_MEMORY_DATA_SIZE 256

struct {
    uint64_t start;
    uint64_t size;
} memory_regions[MAX_MEMORY_REGIONS] = { { 0 } };

struct {
    uint64_t start;
    uint64_t size;
    uint8_t data[MAX_MEMORY_DATA_SIZE];
} memory_data[MAX_MEMORY_DATA] = { { 0 } };

inline uint64_t fromstr(const char* str)
{
    int base = strlen(str) > 2 && (str[1] == 'x' || str[1] == 'X') ? 16 : 10;
    return strtoull(str, NULL, base);
}

static struct json_value_s* json_find(struct json_object_s* object, const char* key)
{
    struct json_object_element_s *element = object->start;
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

static void loadTest(const char** filepath, const char* include_path)
{
    FILE* file = fopen(*filepath, "r");
    if (!file) {
        printf_log(LOG_NONE, "Failed to open test file: %s\n", *filepath);
        exit(1);
    }

    // read file line by line
    char line[1024];
    char json[4096] = { 0 };
    bool in_config = false;
    while (fgets(line, sizeof(line), file)) {
        if (!strcmp(line, "%ifdef CONFIG\n")) {
            in_config = true;
            continue;
        } else if (!strcmp(line, "%endif\n")) {
            in_config = false;
            break;
        }
        if (in_config) {
            strncat(json, line, sizeof(json) - strlen(json) - 1);
        }
    }

    fclose(file);
    struct json_value_s *config = json_parse(json, strlen(json));
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

    struct json_value_s* mode = json_find(config->payload, "Mode");
    if (mode && mode->type == json_type_string && !strcmp(((struct json_string_s*)mode->payload)->string, "32BIT")) {
        box64_is32bits = true;
        printf_log(LOG_INFO, "Test is in 32bits mode\n");
    }

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
            exit(1);
        }
    }

    close(mkstemp(binname));
    const char* ld_cmd[] = { X86_64_LD, objname, "-Ttext=0x10000", "-w", "-m", box64_is32bits ? "elf_i386" : "elf_x86_64", "-o", binname, NULL };

    fork_result = fork();
    if (fork_result == 0) {
        execvp(ld_cmd[0], (char* const*)ld_cmd);
        exit(1);
    } else {
        int status;
        waitpid(fork_result, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf_log(LOG_NONE, X86_64_LD " failed with exit code: %d\n", WEXITSTATUS(status));
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

int unittest(int argc, const char** argv)
{
    if (argc < 3 || (strcmp(argv[1], "--test") && strcmp(argv[1], "-t"))) {
        printf_log(LOG_NONE, "Usage: %s -t <filepath> [-i <include>]\n", argv[0]);
        return 0;
    }

    const char* include_path = NULL;
    if (argc > 4 && (!strcmp(argv[3], "-i") || !strcmp(argv[3], "--include"))) {
        include_path = argv[4];
    }

    box64_pagesize = 4096;
    LoadEnvVariables();
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

    my_context = NewBox64Context(argc - 1);

    loadTest(&argv[2], include_path); // will modify argv[2] to point to the binary file
    my_context->fullpath = box_strdup(argv[2]);

    FILE* f = fopen(my_context->fullpath, "rb");
    unlink(my_context->fullpath);

    elfheader_t* elf_header = LoadAndCheckElfHeader(f, my_context->fullpath, 1);
    AddElfHeader(my_context, elf_header);
    CalcLoadAddr(elf_header);
    AllocLoadElfMemory(my_context, elf_header, 1);

    setupZydis(my_context);
    my_context->ep = GetEntryPoint(my_context->maplib, elf_header);

    my_context->stack = (void*)0xC0000000;
    my_context->stacksz = 4096;
    mmap((void*)my_context->stack, my_context->stacksz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    mmap((void*)0xE0000000, 16 * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    mmap((void*)0xE800F000, 2 * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

    for (int i = 0; i < MAX_MEMORY_REGIONS && memory_regions[i].size; ++i) {
        if (!memory_regions[i].start) break;
        mmap((void*)memory_regions[i].start, memory_regions[i].size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    }

    for (int i = 0; i < MAX_MEMORY_DATA && memory_data[i].size; ++i) {
        if (!memory_data[i].start) break;
        if (!memory_data[i].size) continue;
        memcpy((void*)memory_data[i].start, memory_data[i].data, memory_data[i].size);
    }

    x64emu_t* emu = NewX64Emu(my_context, my_context->ep,
        (uintptr_t)my_context->stack, my_context->stacksz, 0);

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

    if (retcode == 0)
        printf_log(LOG_INFO, "Passed\n");
    else
        printf_log(LOG_NONE, "Failed with %d errors\n", retcode);


    return retcode;
}

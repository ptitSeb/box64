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
            int base = strlen(string->string) > 2 && (string->string[1] == 'x' || string->string[1] == 'X') ? 16 : 10;
            values[i] = strtoull(string->string, NULL, base);
            i++;
        }
        element = element->next;
    }
}

static void loadTest(const char** filepath)
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

#define REG(name)                                                                                                      \
    if (regdata && regdata->type == json_type_object) {                                                                \
        struct json_value_s* r##name = json_find(regdata->payload, #name);                                             \
        if (r##name && r##name->type == json_type_string) {                                                            \
            struct json_string_s* string = (struct json_string_s*)r##name->payload;                                    \
            int base = strlen(string->string) > 2 && (string->string[1] == 'x' || string->string[1] == 'X') ? 16 : 10; \
            regs[i] = strtoull(string->string, NULL, base);                                                            \
            check_regs[i] = true;                                                                                      \
        }                                                                                                              \
        i++;                                                                                                           \
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

#define BINNAME "/tmp/binfileXXXXXX"
    char* binname = box_malloc(strlen(BINNAME) + 1);
    memcpy(binname, BINNAME, strlen(BINNAME) + 1);
#undef BINNAME
    char objname[] = "/tmp/objfileXXXXXX";

    int fd;
    fd = mkstemp(objname);
    close(fd);

    const char* nasm_cmd[] = { NASM, *filepath, box64_is32bits ? "-felf32" : "-felf64", "-o", objname, NULL };
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
    const char* ld_cmd[] = { X86_64_LD, objname, "-w", "-m", box64_is32bits ? "elf_i386" : "elf_x86_64", "-o", binname, NULL };

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
        printf_log(LOG_NONE, "Usage: %s -t <filepath>\n", argv[0]);
        return 0;
    }

    box64_pagesize = 4096;
    LoadEnvVariables();
    ftrace = stdout;
    my_context = NewBox64Context(argc - 1);

    loadTest(&argv[2]); // will modify argv[2] to point to the binary file
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

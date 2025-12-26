#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "callback.h"
#include "box32context.h"
#include "librarian.h"
#include "library.h"

// workaround for Globals symbols

#define GLOB(A, B, C) \
    if (GetGlobalNoWeakSymbolStartEnd(my_context->maplib, #A, &globoffs, &globend, -1, NULL, 0, NULL)) {  \
        printf_log(LOG_DEBUG, "Global " #A " workaround, @%p <- %p\n", (void*)globoffs, &A);              \
        memcpy((void*)globoffs, &A, C);                                                                   \
    }                                                                                                     \
    if (B && GetGlobalNoWeakSymbolStartEnd(my_context->maplib, #A, &globoffs, &globend, 2, B, 1, NULL)) { \
        printf_log(LOG_DEBUG, "Global " #A " workaround, @%p <- %p\n", (void*)globoffs, &A);              \
        memcpy((void*)globoffs, &A, C);                                                                   \
    }

#define TOGLOB(A, B, C) \
    if (GetGlobalNoWeakSymbolStartEnd(my_context->maplib, #A, &globoffs, &globend, -1, NULL, 0, NULL)) {  \
        printf_log(LOG_DEBUG, "Global " #A " workaround, @%p -> %p\n", (void*)globoffs, &A);              \
        memcpy(&A, (void*)globoffs, C);                                                                   \
    }                                                                                                     \
    if (B && GetGlobalNoWeakSymbolStartEnd(my_context->maplib, #A, &globoffs, &globend, 2, B, 1, NULL)) { \
        printf_log(LOG_DEBUG, "Global " #A " workaround, @%p -> %p\n", (void*)globoffs, &A);              \
        memcpy(&A, (void*)globoffs, C);                                                                   \
    }


// **************** NCurses ****************
extern int COLS;
extern int LINES;
extern int TABSIZE;
extern void* curscr;
extern void* newscr;
extern void* stdscr;
extern void* acs_map[64];
extern void* UP;
extern void* BC;
extern uint8_t PC;
extern uint16_t ospeed;
extern void* ttytype[32];

void my32_checkGlobalTInfo()
{
    uintptr_t globoffs, globend;
    GLOB(COLS, NULL, sizeof(int))
    GLOB(LINES, NULL, sizeof(int))
    GLOB(TABSIZE, NULL, sizeof(int))
    GLOB(curscr, NULL, sizeof(ptr_t))
    GLOB(newscr, NULL, sizeof(ptr_t))
    GLOB(stdscr, NULL, sizeof(ptr_t))
    GLOB(acs_map, NULL, sizeof(ptr_t))
    GLOB(UP, NULL, sizeof(ptr_t))
    GLOB(BC, NULL, sizeof(ptr_t))
    GLOB(PC, NULL, sizeof(uint8_t))
    GLOB(ospeed, NULL, sizeof(uint16_t))
    GLOB(ttytype, NULL, sizeof(ptr_t))
}

void my32_updateGlobalTInfo()
{
    uintptr_t globoffs, globend;
    TOGLOB(COLS, NULL, sizeof(int))
    TOGLOB(LINES, NULL, sizeof(int))
    TOGLOB(TABSIZE, NULL, sizeof(int))
    TOGLOB(curscr, NULL, sizeof(ptr_t))
    TOGLOB(newscr, NULL, sizeof(ptr_t))
    TOGLOB(stdscr, NULL, sizeof(ptr_t))
    TOGLOB(acs_map, NULL, sizeof(ptr_t))
    TOGLOB(UP, NULL, sizeof(ptr_t))
    TOGLOB(BC, NULL, sizeof(ptr_t))
    TOGLOB(PC, NULL, sizeof(uint8_t))
    TOGLOB(ospeed, NULL, sizeof(uint16_t))
    TOGLOB(ttytype, NULL, sizeof(ptr_t))
}

// **************** getopts ****************
extern char *optarg;
extern int optind, opterr, optopt;

void my32_updateGlobalOpt()
{
    uintptr_t globoffs, globend;
    TOGLOB(optarg, "GLIBC_2.2.5", sizeof(ptr_t));
    TOGLOB(optind, "GLIBC_2.2.5", sizeof(int));
    TOGLOB(opterr, "GLIBC_2.2.5", sizeof(int));
    TOGLOB(optopt, "GLIBC_2.2.5", sizeof(int));
}

void my32_checkGlobalOpt()
{
    uintptr_t globoffs, globend;
    GLOB(optarg, "GLIBC_2.2.5", sizeof(ptr_t));
    GLOB(optind, "GLIBC_2.2.5", sizeof(int));
    GLOB(opterr, "GLIBC_2.2.5", sizeof(int));
    GLOB(optopt, "GLIBC_2.2.5", sizeof(int));
}

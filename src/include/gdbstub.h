#ifndef __GDBSTUB_H_
#define __GDBSTUB_H_

#include <stdbool.h>

#include "emu/x64emu_private.h"

typedef enum gdbstub_action_s {
    GDBSTUB_ACTION_NONE,
    GDBSTUB_ACTION_STEP,
    GDBSTUB_ACTION_DETACH,
} gdbstub_action_t;

typedef struct box64context_s  box64context_t;
typedef enum gdbstub_action_s gdbstub_action_t;
typedef struct gdbstub_s gdbstub_t;

bool GdbStubInit(x64emu_t *emu, char *addr_str, int port);
gdbstub_action_t GdbStubStep(gdbstub_t *stub);

#endif // __GDBSTUB_H_

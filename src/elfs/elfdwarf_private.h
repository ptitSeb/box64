#ifndef __ELFDWARF_PRIVATE_H_
#define __ELFDWARF_PRIVATE_H_

#include "emu/x64emu_private.h"

typedef struct dwarf_unwind_s {
    uint8_t reg_count;
    uint64_t *regs;
} dwarf_unwind_t;
typedef struct elfheader_s elfheader_t;

dwarf_unwind_t *init_dwarf_unwind_registers(x64emu_t *emu);
void free_dwarf_unwind_registers(dwarf_unwind_t **unwind_struct);

// Returns the callee's address on success or NULL on failure (may be NULL regardless).
// If success equals 2, the frame is a signal frame.
uintptr_t get_parent_registers(dwarf_unwind_t *emu, const elfheader_t *ehdr, uintptr_t addr, char *success);

#endif // __ELFDWARF_PRIVATE_H_

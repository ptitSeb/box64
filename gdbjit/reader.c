#include "gdbjit.h"
#include <stdio.h>

static char block_name_buffer[16];

static enum gdb_status read_debug_info(struct gdb_reader_funcs* self, struct gdb_symbol_callbacks* cbs, void* memory, long memory_sz)
{
    gdbjit_block_t* block = (gdbjit_block_t*)memory;

    struct gdb_object* object = cbs->object_open(cbs);
    struct gdb_symtab* symtab = cbs->symtab_open(cbs, object, block->filename);
    sprintf(block_name_buffer, "%x", block->x64start);
    cbs->block_open(cbs, symtab, NULL, block->start, block->end, block_name_buffer);

    cbs->line_mapping_add(cbs, symtab, block->nlines, block->lines);

    cbs->symtab_close(cbs, symtab);
    cbs->object_close(cbs, object);

    return GDB_SUCCESS;
}

enum gdb_status unwind_frame(struct gdb_reader_funcs* self, struct gdb_unwind_callbacks* cbs)
{
  return GDB_SUCCESS;
}

struct gdb_frame_id get_frame_id(struct gdb_reader_funcs* self, struct gdb_unwind_callbacks* cbs)
{
  struct gdb_frame_id frame = {0xdeadbeef, 0};
  return frame;
}

void destroy_reader(struct gdb_reader_funcs* self) {}

__attribute__((visibility("default"))) struct gdb_reader_funcs* gdb_init_reader(void)
{
  static struct gdb_reader_funcs funcs = {GDB_READER_INTERFACE_VERSION, NULL, read_debug_info, unwind_frame, get_frame_id, destroy_reader};
  return &funcs;
}

__attribute__((visibility("default"))) int plugin_is_GPL_compatible (void)
{
    return 0;
}
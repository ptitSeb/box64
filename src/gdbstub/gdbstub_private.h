#ifndef __GDBSTUB_PRIVATE_H_
#define __GDBSTUB_PRIVATE_H_

#include <stdint.h>

#include "x64emu.h"
#include "khash.h"

#define GDBSTUB_MAX_PKT_SIZE 1024

struct conn_s {
    int listen_fd;
    int socket_fd;
};

KHASH_MAP_INIT_INT64(bps, bool);

typedef struct gdbstub_s {
    struct conn_s conn;
    x64emu_t* emu;
    bool cont;
    kh_bps_t *bps; // breakpoints

    // Packet buffer, should be enough, replace this with a dynamic buffer maybe.
    uint8_t pktbuf[GDBSTUB_MAX_PKT_SIZE];
} gdbstub_t;

#endif // __GDBSTUB_PRIVATE_H_

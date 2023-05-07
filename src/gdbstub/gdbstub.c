#include <arpa/inet.h>
#include <assert.h>
#include <poll.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "regs.h"
#include "debug.h"
#include "gdbstub.h"
#include "gdbstub_private.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "elfs/elfloader_private.h"

#define GDBSTUB_PKT_PREFIX '$'
#define GDBSTUB_PKT_SUFFIX '#'
#define GDBSTUB_PKT_ACK    "+"

static uint8_t ComputeCheckSum(uint8_t *buf, size_t len) {
    uint8_t cs = 0;
    for (size_t i = 0; i < len; ++i)
        cs += buf[i];
    return cs;
}

static void HexToStr(uint8_t *num, char *str, int bytes) {
    static char hexchars[] = "0123456789abcdef";

    for (int i = 0; i < bytes; i++) {
        uint8_t c = *(num + i);
        *(str + i * 2) = hexchars[c>>4];
        *(str + i * 2 + 1) = hexchars[c&0xf];
    }
    str[bytes * 2] = '\0';
}

static uint8_t CharToHex(char ch) {
    uint8_t letter = ch & 0x40;
    uint8_t offset = (letter >> 3) | (letter >> 6);
    return (ch + offset) & 0xf;
}

static uint8_t ConvertCheckSumToHex(char l, char h) {
    return (CharToHex(l)<<4) | CharToHex(h);
}

static bool SocketPoll(int socket_fd, int timeout, int events)
{
    struct pollfd pfd = (struct pollfd) {
        .fd = socket_fd,
        .events = events,
    };

    return (poll(&pfd, 1, timeout) > 0) && (pfd.revents & events);
}

static bool ConnInit(struct conn_s *conn, char *addr_str, int port) {
    int fd, fd2, optval = 1;
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(addr_str),
        .sin_port = htons(port)
    };

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return false;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) goto fail;
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) goto fail;
    if (listen(fd, 1) < 0) goto fail;
    if ((fd2 = accept(fd, NULL, NULL)) < 0) goto fail;

    conn->listen_fd = fd;
    conn->socket_fd = fd2;

    return true;
fail:
    close(fd);
    return false;
}

static uint8_t ConnGetchar(struct conn_s *conn) {
    static uint8_t c;
    int _ = read(conn->socket_fd, &c, 1);
    return c;
}

bool GdbStubInit(x64emu_t *emu, char *addr_str, int port) {
    emu->context->gdbstub = (gdbstub_t *)box_calloc(1, sizeof(gdbstub_t));
    emu->context->gdbstub->emu = emu;
    if (!ConnInit(&emu->context->gdbstub->conn, addr_str, port)) {
        return false;
    }
    emu->context->gdbstub->bps = kh_init(bps);

    return true;
}

static uintptr_t GdbStubUnifyAddr(gdbstub_t *stub, uintptr_t addr) {
    for (int i = 0; i < stub->emu->context->elfsize; i++) {
        elfheader_t *h = stub->emu->context->elfs[i];
        #define TRY()   \
            for (int j = 0; j < h->multiblock_n; j++) {                                                     \
                if (addr >= h->multiblock_offs[j] && addr <= h->multiblock_offs[j]+h->multiblock_size[j]) { \
                    return addr;                                                                            \
                }                                                                                           \
            }
        TRY();
        // Gdb might send us a address before mmap (read from elf file),
        // so add delta try again
        addr += h->delta;
        TRY();
        #undef TRY
    }

    return 0;
}

static void GdbStubSendStr(gdbstub_t *stub, char *str)
{
    size_t len = strlen(str);

    while (len > 0 && SocketPoll(stub->conn.socket_fd, -1, POLLOUT)) {
        ssize_t nwrite = write(stub->conn.socket_fd, str, len);
        if (nwrite == -1)
            break;
        len -= nwrite;
    }
}

static void GdbStubRecvPkt(gdbstub_t *stub) {
    static uint8_t c;
    int offset = 0;

    // Step forward until the packet prefix.
    while (true) {
        c = ConnGetchar(&stub->conn);
        if (c == GDBSTUB_PKT_PREFIX) break;
    }

    while (true) {
        c = ConnGetchar(&stub->conn);
        if (c == GDBSTUB_PKT_SUFFIX) {
            char l = ConnGetchar(&stub->conn), h = ConnGetchar(&stub->conn);
            assert(ConvertCheckSumToHex(l, h) == ComputeCheckSum(stub->pktbuf, offset));

            stub->pktbuf[offset] = '\0';
            break;
        }
        stub->pktbuf[offset++] = c;
    }

    printf_log(LOG_DEBUG, "[GDBSTUB] Recv packet: %s\n", (char *)stub->pktbuf);
    GdbStubSendStr(stub, GDBSTUB_PKT_ACK);
}

static void GdbStubSendPkt(gdbstub_t *stub, char *raw) {
    static char packet[GDBSTUB_MAX_PKT_SIZE];
    static char checksumbuf[4];
    uint8_t checksum;
    size_t len = strlen(raw);

    // 2: prefix '$' and suffix '#'
    // 2: checksum
    // 1: NUL
    assert(len + 2 + 2 + 1 < GDBSTUB_MAX_PKT_SIZE);

    packet[0] = '$';
    memcpy(packet + 1, raw, len);
    packet[len + 1] = '#';
    checksum = ComputeCheckSum(raw, len);
    snprintf(checksumbuf, 3, "%02x", checksum);
    memcpy(packet + len + 2, checksumbuf, 2);
    packet[len + 2 + 2] = '\0';

    GdbStubSendStr(stub, packet);
    printf_log(LOG_DEBUG, "[GDBSTUB] Send packet: %s\n", packet);
}

static void GdbStubHandleQuery(gdbstub_t *stub, char *payload) {
    if (strstr(payload, "Supported")) {
        GdbStubSendPkt(stub, "");
    } else if (strstr(payload, "Attached")) {
        GdbStubSendPkt(stub, "1");
    } else if (strstr(payload, "Symbol")) {
        GdbStubSendPkt(stub, "OK");
    } else {
        GdbStubSendPkt(stub, "");
    }
}

static void GdbStubHandleVCont(gdbstub_t *stub, char *payload) {
    static char buf[32];
    if (strstr("Cont?", payload)) {
        GdbStubSendPkt(stub, "vCont;s;c;");
    } else {
        GdbStubSendPkt(stub, "");
    }
}

static void GdbStubHandleRegsRead(gdbstub_t *stub, char *payload) {
    static char packet[GDBSTUB_MAX_PKT_SIZE] = {0};
    // rax -> r15
    for (int i = 0; i < 16; i++) {
        HexToStr((uint8_t *)&stub->emu->regs[i].q[0], &packet[i*sizeof(uint64_t)*2], sizeof(uint64_t));
    }
    // rip
    HexToStr((uint8_t *)&stub->emu->ip.q[0], &packet[16*sizeof(uint64_t)*2], sizeof(uint64_t));
    // eflags, 32bit
    HexToStr((uint8_t *)&stub->emu->eflags, &packet[17*sizeof(uint64_t)*2], sizeof(uint32_t));
    // cs -> gs, 32bit
    for (int i = 0; i < 6; i++) {
        HexToStr((uint8_t *)&stub->emu->segs[i], &packet[(35+i)*sizeof(uint32_t)*2], sizeof(uint32_t));
    }
    GdbStubSendPkt(stub, packet);
}

static void GdbStubHandleMemRead(gdbstub_t *stub, char *payload) {
    static uint8_t buf[512];
    static char packet[GDBSTUB_MAX_PKT_SIZE] = {0};
    size_t maddr, mlen;
    sscanf(payload, "%lx,%lx", &maddr, &mlen);

    maddr = GdbStubUnifyAddr(stub, maddr);
    if (maddr) {
        memcpy(buf, (void *)maddr, mlen);
        HexToStr(buf, packet, mlen);
        GdbStubSendPkt(stub, packet);
    } else {
        GdbStubSendPkt(stub, "E14");
    }
}

static void GdbStubHandleSetBp(gdbstub_t *stub, char *payload) {
    size_t type, addr, kind;
    sscanf(payload, "%zx,%zx,%zx", &type, &addr, &kind);
    addr = GdbStubUnifyAddr(stub, addr);
    if (type != 0 || addr == 0) {
        GdbStubSendPkt(stub, "E01");
        return;
    }
    int absent;
    khint64_t k;
    k = kh_put(bps, stub->bps, addr, &absent);
    kh_value(stub->bps, k) = true;
    GdbStubSendPkt(stub, "OK");
    printf_log(LOG_DEBUG, "[GDBSTUB] Set breakpoint: %p\n", addr);
}

static gdbstub_action_t GdbStubHandlePkt(gdbstub_t *stub) {
    uint8_t req = stub->pktbuf[0];
    char *payload = (char *)&stub->pktbuf[1];

    switch (req) {
    case 'c': // continue
        stub->cont = true;
        GdbStubSendPkt(stub, "OK");
        break;
    case 'g':
        GdbStubHandleRegsRead(stub, payload);
        break;
    case 'm': // read mem
        GdbStubHandleMemRead(stub, payload);
        break;
    case 'p': // read reg
        // TODO
        GdbStubSendPkt(stub, "");
        break;
    case 'q':
        GdbStubHandleQuery(stub, payload);
        break;
    case 's':
        GdbStubSendPkt(stub, "S05");
        return GDBSTUB_ACTION_STEP;
    case 'v':
        GdbStubHandleVCont(stub, payload);
        break;
    case 'z': // delete bp
        // TODO
        GdbStubSendPkt(stub, "");
        break;
    case '?': // why halted?
        GdbStubSendPkt(stub, "S05");
        break;
    case 'D':
        return GDBSTUB_ACTION_DETACH;
    case 'G': // regs write
        // TODO
        GdbStubSendPkt(stub, "");
        break;
    case 'M': // mem write
        // TODO
        GdbStubSendPkt(stub, "");
        break;
    case 'P': // reg write
        // TODO
        GdbStubSendPkt(stub, "");
        break;
    case 'X': // mem xwrite (binary)
        // TODO
        GdbStubSendPkt(stub, "");
        break;
    case 'Z': // set bp
        GdbStubHandleSetBp(stub, payload);
        break;
    default:
        GdbStubSendPkt(stub, "");
        break;
    }

    return GDBSTUB_ACTION_NONE;
}

// Step forward gdbstub, handles one packet a time. returns an action.
gdbstub_action_t GdbStubStep(gdbstub_t *stub) {
    if (stub->cont) return GDBSTUB_ACTION_STEP;
    GdbStubRecvPkt(stub);
    return GdbStubHandlePkt(stub);
}

void GdbStubCheckBp(gdbstub_t *stub, uintptr_t addr) {
    khint64_t k = kh_get(bps, stub->bps, addr);
    if (k != kh_end(stub->bps)) {
        stub->cont = false;
        printf_log(LOG_DEBUG, "[GDBSTUB] Hit breakpoint: %p\n", addr);
        GdbStubSendPkt(stub, "S05");
    }
}

void GdbStubDestroy(gdbstub_t *stub) {
    kh_destroy(bps, stub->bps);
}

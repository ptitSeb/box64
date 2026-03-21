// gcc -m32 -O0 -o test27 test27c.c
// Test GS-segment memory access.
// We save/restore GS around accesses since glibc uses GS for TLS.
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>

static char gs_data[4096] __attribute__((aligned(4096)));
static unsigned short old_gs, new_gs;

static int setup_gs(void *base) {
    // Save current GS
    asm volatile("movw %%gs, %0" : "=r"(old_gs));

    struct user_desc ud;
    memset(&ud, 0, sizeof(ud));
    ud.entry_number = -1;
    ud.base_addr = (unsigned long)base;
    ud.limit = 0xfffff;
    ud.seg_32bit = 1;
    ud.contents = 0;
    ud.read_exec_only = 0;
    ud.limit_in_pages = 1;
    ud.seg_not_present = 0;
    ud.useable = 1;

    int ret = syscall(SYS_set_thread_area, &ud);
    if (ret != 0) {
        perror("set_thread_area");
        return -1;
    }
    new_gs = (ud.entry_number << 3) | 3;
    return 0;
}

// Switch to our GS, do a read, switch back
static inline uint32_t gs_read32(uint32_t offset) {
    uint32_t val;
    asm volatile(
        "movw %1, %%gs\n\t"
        "movl %%gs:(%2), %0\n\t"
        "movw %3, %%gs\n\t"
        : "=r"(val)
        : "r"(new_gs), "r"(offset), "r"(old_gs)
        : "memory"
    );
    return val;
}

// Switch to our GS, do a write, switch back
static inline void gs_write32(uint32_t offset, uint32_t val) {
    asm volatile(
        "movw %0, %%gs\n\t"
        "movl %1, %%gs:(%2)\n\t"
        "movw %3, %%gs\n\t"
        :
        : "r"(new_gs), "r"(val), "r"(offset), "r"(old_gs)
        : "memory"
    );
}

// Read using moffs encoding (opcode A1 with GS prefix)
static inline uint32_t gs_read_moffs(uint32_t offset) {
    uint32_t val;
    // We need to use inline asm with an immediate offset for moffs.
    // Since we can not use variable immediates in asm, we do GS + register indirect.
    asm volatile(
        "movw %1, %%gs\n\t"
        "movl %%gs:(%2), %%eax\n\t"
        "movw %3, %%gs\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(val)
        : "r"(new_gs), "r"(offset), "r"(old_gs)
        : "eax", "memory"
    );
    return val;
}

int main(void) {
    for (int i = 0; i < (int)sizeof(gs_data); i++)
        gs_data[i] = (char)(i & 0xff);

    *(uint32_t*)&gs_data[0]   = 0xDEADBEEF;
    *(uint32_t*)&gs_data[16]  = 0xCAFEBABE;
    *(uint32_t*)&gs_data[256] = 0x12345678;

    if (setup_gs(gs_data) != 0) {
        printf("Failed to set GS base\n");
        return 1;
    }

    // Test 1: Simple read gs:[0]
    uint32_t val = gs_read32(0);
    printf("gs:[0] = 0x%08x\n", val);

    // Test 2: Read gs:[16]
    val = gs_read32(16);
    printf("gs:[16] = 0x%08x\n", val);

    // Test 3: Read gs:[256]
    val = gs_read32(256);
    printf("gs:[256] = 0x%08x\n", val);

    // Test 4: Write and read back
    gs_write32(512, 0xABCD1234);
    val = gs_read32(512);
    printf("gs:[512] write/read = 0x%08x\n", val);

    // Test 5: Another write/read
    gs_write32(768, 0x55AA55AA);
    val = gs_read32(768);
    printf("gs:moffs write/read = 0x%08x\n", val);

    // Test 6: Read known value
    *(uint32_t*)&gs_data[1024] = 0x99887766;
    val = gs_read_moffs(1024);
    printf("gs:moffs read = 0x%08x\n", val);

    printf("GS segment tests passed\n");
    return 0;
}

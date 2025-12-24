#ifndef __BRIDGE_PRIVATE_H_
#define __BRIDGE_PRIVATE_H_
#include <stdint.h>

// the generic wrapper pointer functions
typedef void (*wrapper_t)(x64emu_t* emu, uintptr_t fnc);

#pragma pack(push, 1)
typedef union onebridge_s {
    struct {
    uint8_t CC;     // CC int 0x3
    uint8_t S, C;   // 'S' 'C', just a signature
    wrapper_t w;    // wrapper
    uintptr_t f;    // the function for the wrapper
    uint8_t C3;     // C2 or C3 ret
    uint16_t N;     // N in case of C2 ret
    uint8_t func;   // 1 if name_or_func is actualy a function (so no name in that case)
    void* name_or_func;   // name of the function bridged
    };
    struct {
    uint8_t B8;     // B8 00 11 22 33 mov eax, num
    uint32_t num;
    uint8_t _0F; uint8_t _05;   // 0F 05 syscall
    uint8_t _C3;    // C3 ret
    const char* syscall_name;   //not used for now
    };
    uint64_t dummy[4];
} onebridge_t;
#pragma pack(pop)

#endif //__BRIDGE_PRIVATE_H_
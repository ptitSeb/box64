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
    };
    uint64_t dummy[4];
} onebridge_t;
#pragma pack(pop)

#endif //__BRIDGE_PRIVATE_H_
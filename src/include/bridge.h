#ifndef __BRIDGE_H_
#define __BRIDGE_H_
#include <stdint.h>

typedef struct x64emu_s x64emu_t;
typedef struct bridge_s bridge_t;
typedef struct box64context_s box64context_t;
typedef void (*wrapper_t)(x64emu_t* emu, uintptr_t fnc);

bridge_t *NewBridge(void);
void FreeBridge(bridge_t** bridge);

uintptr_t AddBridge(bridge_t* bridge, wrapper_t w, void* fnc, int N, const char* name);
uintptr_t CheckBridged(bridge_t* bridge, void* fnc);
uintptr_t AddCheckBridge(bridge_t* bridge, wrapper_t w, void* fnc, int N, const char* name);
uintptr_t AddAutomaticBridge(bridge_t* bridge, wrapper_t w, void* fnc, int N, const char* name);
void* GetNativeFnc(uintptr_t fnc);
void* GetNativeFncOrFnc(uintptr_t fnc);

uintptr_t AddVSyscall(bridge_t* bridge, int num);

int hasAlternate(void* addr);
void* getAlternate(void* addr);
void addAlternate(void* addr, void* alt);
void cleanAlternate(void);
const char* getBridgeName(void* addr);

void init_bridge_helper(void);
void fini_bridge_helper(void);

#endif //__BRIDGE_H_
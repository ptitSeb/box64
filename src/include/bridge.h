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
uintptr_t CheckBridged2(bridge_t* bridge, void* fnc, void* fnc2);
uintptr_t AddBridge2(bridge_t* bridge, wrapper_t w, void* fnc, void* fnc2, int N, const char* name);
uintptr_t AddCheckBridge(bridge_t* bridge, wrapper_t w, void* fnc, int N, const char* name);
uintptr_t AddCheckBridge2(bridge_t* bridge, wrapper_t w, void* fnc, void* fnc2, int N, const char* name);
uintptr_t AddAutomaticBridge(bridge_t* bridge, wrapper_t w, void* fnc, int N, const char* name);
uintptr_t AddAutomaticBridgeAlt(bridge_t* bridge, wrapper_t w, void* fnc, void* alt, int N, const char* name);
void* GetNativeFnc(uintptr_t fnc);
void* GetNativeFncOrFnc(uintptr_t fnc);
void* GetNativeOrAlt(void* fnc, void* alt);

uintptr_t AddVSyscall(bridge_t* bridge, int num);

const char* getBridgeName(void* addr);
void* getBridgeFnc2(void* addr);

void init_bridge_helper(void);
void fini_bridge_helper(void);

// Is what pointed at addr a native call? And if yes, to what function?
int isNativeCallInternal(uintptr_t addr, int is32bits, uintptr_t* calladdress, uint16_t* retn);

#endif //__BRIDGE_H_

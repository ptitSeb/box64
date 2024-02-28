#ifndef __DYNAREC_NEXT_H__
#define __DYNAREC_NEXT_H__

#ifdef ARM64
void arm64_next(void) EXPORTDYN;
void arm64_prolog(x64emu_t* emu, void* addr) EXPORTDYN;
void arm64_epilog(void) EXPORTDYN;
#define native_next         arm64_next
#define native_prolog       arm64_prolog
#define native_epilog       arm64_epilog
#elif defined(LA64)
void la64_next(void) EXPORTDYN;
void la64_prolog(x64emu_t* emu, void* addr) EXPORTDYN;
void la64_epilog(void) EXPORTDYN;
#define native_next         la64_next
#define native_prolog       la64_prolog
#define native_epilog       la64_epilog
#elif defined(RV64)
void rv64_next(void) EXPORTDYN;
void rv64_prolog(x64emu_t* emu, void* addr) EXPORTDYN;
void rv64_epilog(void) EXPORTDYN;
#define native_next         rv64_next
#define native_prolog       rv64_prolog
#define native_epilog       rv64_epilog
#else
#error Unsupported architecture
#endif

#endif //__DYNAREC_NEXT_H__
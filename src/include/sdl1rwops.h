#ifndef __SDL1RWOPS_H__
#define __SDL1RWOPS_H__

typedef struct SDL1_RWops_s SDL1_RWops_t;   // the actual SDL1 SDL_RWops
typedef struct x64emu_s x64emu_t;

typedef SDL1_RWops_t* (*sdl1_allocrw)(void);
typedef void (*sdl1_freerw)(SDL1_RWops_t*);

// each function will be added to dictionary, and each native functions will be wrapped so they run in emulated world
SDL1_RWops_t* AddNativeRW(x64emu_t* emu, SDL1_RWops_t* ops);
SDL1_RWops_t* RWNativeStart(x64emu_t* emu, SDL1_RWops_t* ops);   // put Native RW function, wrapping emulated (callback style) ones if needed
void RWNativeEnd(SDL1_RWops_t* ops);     // put back emulated function back in place

int32_t RWNativeSeek(SDL1_RWops_t *ops, int32_t offset, int32_t whence);
uint32_t RWNativeRead(SDL1_RWops_t* ops, void* ptr, uint32_t size, uint32_t maxnum);
int32_t RWNativeWrite(SDL1_RWops_t *ops, const void *ptr, int32_t size, int32_t num);
int32_t RWNativeClose(SDL1_RWops_t* ops);
void RWSetType(SDL1_RWops_t* r, int t);

#endif
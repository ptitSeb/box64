#ifndef __SDL2RWOPS_H__
#define __SDL2RWOPS_H__

typedef struct SDL2_RWops_s SDL2_RWops_t;   // the actual SDL2 SDL_RWops
typedef struct x64emu_s x64emu_t;

typedef SDL2_RWops_t* (*sdl2_allocrw)(void);
typedef void (*sdl2_freerw)(SDL2_RWops_t*);

typedef struct SDL2RWSave_s {
    int   anyEmu;
    void* size;
    void* seek;
    void* read;
    void* write;
    void* close;
    void* s1;
    void* s2;
} SDL2RWSave_t;

// each function will be added to dictionary, and each native functions will be wrapped so they run in emulated world
SDL2_RWops_t* AddNativeRW2(x64emu_t* emu, SDL2_RWops_t* ops);
SDL2_RWops_t* RWNativeStart2(x64emu_t* emu, SDL2_RWops_t* ops); // put native RW functions, wrapping the emulated (callback style) ones if needed
void RWNativeEnd2(SDL2_RWops_t* ops);                           // put emulated function back in place
int isRWops(SDL2_RWops_t* ops); // 1 if ops seems to be a valid RWops, 0 if not

int64_t RWNativeSeek2(SDL2_RWops_t *ops, int64_t offset, int32_t whence);
uint32_t RWNativeRead2(SDL2_RWops_t* ops, void* ptr, uint32_t size, uint32_t maxnum);
int32_t RWNativeWrite2(SDL2_RWops_t *ops, const void *ptr, int32_t size, int32_t num);
int32_t RWNativeClose2(SDL2_RWops_t* ops);

#endif

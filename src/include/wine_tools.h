#ifndef __WINE_TOOLS_H__
#define __WINE_TOOLS_H__

void wine_prereserve(const char* reserve);

extern int wine_preloaded;
void* get_wine_prereserve(void);

#ifdef DYNAREC
void dynarec_wine_prereserve(void);
#endif

#endif //__WINE_TOOLS_H__

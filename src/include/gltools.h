#ifndef __GL_TOOLS_H__
#define __GL_TOOLS_H__

typedef struct box64context_s box64context_t;

typedef void* (*glprocaddress_t)(const char* name);

void freeGLProcWrapper(box64context_t* context);

void* getGLProcAddress(x64emu_t* emu, const char* my, glprocaddress_t procaddr, const char* rname);

#ifdef BOX32
void* getGLProcAddress32(x64emu_t* emu, const char* my, glprocaddress_t procaddr, const char* rname);
#endif

#endif //__GL_TOOLS_H__
#ifndef __AUXVAL_H__
#define __AUXVAL_H__

typedef struct x64emu_s x64emu_t;

#ifndef BUILD_LIB
int init_auxval(int argc, const char **argv, char **env);
#endif

unsigned long real_getauxval(unsigned long type);
unsigned long my_getauxval(x64emu_t* emu, unsigned long type);

#endif //__AUXVAL_H__
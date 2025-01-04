/*
 * Compile with (on x86_64 archs):
 *  gcc -m32 -DV1 -shared -o test21_v1.so test21.c
 *  gcc -m32 -DV2 -shared -o test21_v2.so test21.c
 *  gcc -m32 -o test21 test21.c -ldl -Wl,-z,origin,-rpath='$ORIGIN'
 */

#if defined(V1)
int __attribute__((noinline)) getVersion() { asm(""); return 1; }
int acquireVersion() { return getVersion(); }

#elif defined(V2)
int __attribute__((noinline)) getVersion() { asm(""); return 2; }
int returnVersion() { return getVersion(); }

#else
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef int(*iFv_t)(void);

int main(int argc, char **argv) { 
    void* v1 = dlopen("test21_v1.so", RTLD_NOW);
    void* v2 = dlopen("test21_v2.so", RTLD_NOW);
    if(!v1 || !v2) {
        printf("Error openning libs: v1=%p, v2=%p\n", v1, v2); // typo: opening
        exit(-1);
    }
    iFv_t returnVersion = (iFv_t)dlsym(v2, "returnVersion");
    iFv_t acquireVersion = (iFv_t)dlsym(v1, "acquireVersion");
    if(!returnVersion || !acquireVersion) {
        printf("Error getting symbol return=%p/acquire=%p\n", returnVersion, acquireVersion);
        exit(-2);
    }
    printf("%d/%d\n", returnVersion(), acquireVersion()); return 0; 
    dlclose(v2);
    dlclose(v1);
}

#endif

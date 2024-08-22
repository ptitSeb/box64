/*
 * Compile with (on x86_64 archs):
 *  gcc -m32 -DV1 -shared -o test15_v1.so test15.c
 *  gcc -m32 -DV2 -shared -o test15_v2.so test15.c -Wl,--version-script=test15_version.map
 *  gcc -m32 -shared -o test15_dv1.so test15.c -L. -Wl,-z,origin,-rpath='$ORIGIN' -l:test15_v1.so
 *  gcc -m32 -DMAIN -o test15 test15.c -L. -Wl,-z,origin,-rpath='$ORIGIN' -l:test15_dv1.so -l:test15_v2.so
 */

#if defined(MAIN)
int returnVersion();
int acquireVersion1();
int acquireVersion2();
#include <stdio.h>

int main(int argc, char **argv) { printf("%d, %d/%d\n", returnVersion(), acquireVersion1(), acquireVersion2()); return 0; }

#elif defined(V2)
int __attribute__((noinline)) getVersion() { asm(""); return 2; }
int returnVersion() { return getVersion(); }

#elif defined(V1)
int __attribute__((noinline)) getVersion() { asm(""); return 1; }
int acquireVersion() { return getVersion(); }

#else
int getVersion();
int acquireVersion();

int acquireVersion1() { return getVersion(); }
int acquireVersion2() { return acquireVersion(); }

#endif

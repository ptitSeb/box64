/*
 * Compile with (on x86_64 archs):
 *  gcc -m32 -DV1 -shared -o test16_v1.so test16.c -Wl,--version-script=test16_version.map
 *  gcc -m32 -DV2 -shared -o test16_v2.so test16.c
 *  gcc -m32 -shared -o test16_dv1.so test16.c -L. -Wl,-z,origin,-rpath='$ORIGIN' -l:test16_v1.so
 *  gcc -m32 -DMAIN -o test16 test16.c -L. -Wl,-z,origin,-rpath='$ORIGIN' -l:test16_dv1.so -l:test16_v2.so
 */

#if defined(MAIN)
int returnVersion();
int acquireVersion1();
int acquireVersion2();
#include <stdio.h>

int main(int argc, char **argv) { printf("%d, %d/%d\n", returnVersion(), acquireVersion1(), acquireVersion2()); return 0; }

#elif defined(V1)
int __attribute__((noinline)) getVersion() { asm(""); return 1; }
int acquireVersion() { return getVersion(); }

#elif defined(V2)
int __attribute__((noinline)) getVersion() { asm(""); return 2; }
int returnVersion() { return getVersion(); }

#else
int getVersion();
int returnVersion();

int acquireVersion1() { return getVersion(); }
int acquireVersion2() { return returnVersion(); }

#endif

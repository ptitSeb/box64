#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define EXPORT __attribute__((visibility("default")))

// Build with `x86_64-linux-gnu-gcc -g -shared -fPIC x64_functions_2.c -o libx64functions2.so`

EXPORT int x64_tolower(int c) {
    return tolower(c);
}
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define EXPORT __attribute__((visibility("default")))

EXPORT int x64_tolower(int c) {
    return tolower(c);
}
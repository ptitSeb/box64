#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define EXPORT __attribute__((visibility("default")))

// Build with `x86_64-linux-gnu-gcc -g -shared -fPIC x64_functions_1.c -o libx64functions1.so`

EXPORT char* hello_word_str(void) {
    char* buff = (char*)malloc(30);
    memset(buff, 0, 30);

    const char* str1 = "hello ";
    const char* str2 = "world!";
    strcat(buff, str1);
    strcat(buff, str2);
    return buff;
}

EXPORT void x64_free(void* ptr) {
    free(ptr);
}
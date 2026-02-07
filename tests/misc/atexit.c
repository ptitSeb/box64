#include <stdio.h>
#include <dlfcn.h>

void test(void* handle, int do_dlclose)
{
    if (handle) {
        int (*JNI_OnLoad)(void*, void*);
        JNI_OnLoad = dlsym(handle, "JNI_OnLoad");
        if (JNI_OnLoad) {
            JNI_OnLoad((void*)0x3a5000, (void*)0x3c6000);
        } else {
            printf("%s\n", dlerror());
        }
        if (do_dlclose) {
            dlclose(handle); // java does not dlclose libAtExit.so
        }
        handle = NULL;
    } else {
        printf("%s\n", dlerror());
    }
}

int main(int argc, char* argv[])
{
    void* handle = dlopen("libAtExit.so", RTLD_LAZY);
    test(handle, 1 /* do_dlclose */);
    handle = dlopen("libAtExit.so", RTLD_LAZY);
    test(handle, 0 /* but java does not dlclose JNI shared library */);
    return 0;
}

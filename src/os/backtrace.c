#include "backtrace.h"

#if !defined(ANDROID) && !defined(_WIN32)

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <execinfo.h>

#include "debug.h"
#include "box64context.h"

#define BT_BUF_SIZE 100
void ShowNativeBT(int log_minimum)
{
    // grab current name
    // and temporarily rename binary to original box64
    // to get exact backtrace
    size_t boxpath_lenth = strlen(my_context->box64path) + 1;
    char current_name[boxpath_lenth];
    memcpy(current_name, my_context->orig_argv[0], boxpath_lenth);
    memcpy(my_context->orig_argv[0], my_context->box64path, boxpath_lenth);
    // show native bt
    int nptrs;
    void* buffer[BT_BUF_SIZE];
    char** strings;

    nptrs = backtrace(buffer, BT_BUF_SIZE);
    strings = backtrace_symbols(buffer, nptrs);
    if (strings) {
        for (int j = 0; j < nptrs; j++)
            printf_log(log_minimum, "NativeBT: %s\n", strings[j]);
        free(strings);
    } else {
        for (int j = 0; j < nptrs; j++)
            printf_log(log_minimum, "NativeBT: %p\n", buffer[j]);
    }
    // restore modified name
    memcpy(my_context->box64path, my_context->orig_argv[0], boxpath_lenth);
}
#undef BT_BUF_SIZE

#else
void ShowNativeBT(int log_minimum) { }
#endif
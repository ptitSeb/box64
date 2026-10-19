// SPDX-License-Identifier: MIT
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>


#if !defined(HAVE_OBSTACK_VPRINTF) && !defined(ANDROID)
#include <obstack.h>

int obstack_vprintf(struct obstack *obstack,  const char *fmt, va_list args)
{
    char buffer[200];
    char *buf = buffer;
    va_list copy;
    
    va_copy(copy, args);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, copy);
    va_end(copy);

    if (len < 0) return -1;

    if ((size_t)len >= sizeof(buffer)) 
    {
        buf = malloc((size_t)len + 1);
        if (!buf) return -1;

        va_copy(copy, args);
        int n = vsnprintf(buf, len + 1, fmt, copy);
        va_end(copy);

        if (n != len) 
        {
            free(buf);
            return -1;
        }
        len = n;
    }

    if(len > 0) obstack_grow(obstack, buf, len);
    if(buf != buffer) free(buf);

	return len;
}
#endif
#include <stdio.h>
#include <stdarg.h>

int my_func(const char* fmt, va_list a)
{
    return vprintf(fmt, a);
}

int my_func1(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = my_func(fmt, args);
    va_end(args);
    return ret;
}

int main(int argc, char **argv)
{
    int ret = my_func1("Hello %s World, pi=%g!\n", "x86_64", 3.14159265);
    return 0;
}



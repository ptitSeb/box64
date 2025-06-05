#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>

#include "os.h"

int __mingw_sprintf(char* buffer, const char* format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = vsprintf(buffer, format, args);
    va_end(args);

    return ret;
}

int __isnanf(float x)
{
    union { float x; unsigned int i; } u = { x };
    return (u.i & 0x7fffffff) > 0x7f800000;
}

int __isnan(double x)
{
    union { double x; unsigned __int64 i; } u = { x };
    return (u.i & ~0ull >> 1) > 0x7ffull << 52;
}

double math_error(int type, const char *name, double arg1, double arg2, double retval)
{
    return retval;
}

int __fpclassify(double x)
{
	union {double f; uint64_t i;} u = {x};
	int e = u.i>>52 & 0x7ff;
	if (!e) return u.i<<1 ? FP_SUBNORMAL : FP_ZERO;
	if (e==0x7ff) return u.i<<12 ? FP_NAN : FP_INFINITE;
	return FP_NORMAL;
}

int __fpclassifyf(float x)
{
	union {float f; uint32_t i;} u = {x};
	int e = u.i>>23 & 0xff;
	if (!e) return u.i<<1 ? FP_SUBNORMAL : FP_ZERO;
	if (e==0xff) return u.i<<9 ? FP_NAN : FP_INFINITE;
	return FP_NORMAL;
}

int fegetround (void)
{
    return 0;
}

int fesetround (int __rounding_direction)
{
    return 0;
}

div_t __cdecl div(int num, int denom)
{
    div_t ret;

    ret.quot = num / denom;
    ret.rem = num % denom;
    return ret;
}

ldiv_t __cdecl ldiv(long num, long denom)
{
    ldiv_t ret;

    ret.quot = num / denom;
    ret.rem = num % denom;
    return ret;
}

void _assert (const char *_Message, const char *_File, unsigned _Line)
{
    // NYI
}

char* strerror(int e)
{
    return "error";
}

int snprintf(char* restrict s, size_t n, const char* restrict fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int result = _vsnprintf(s, n, fmt, args);
    va_end(args);
    return result;
}

char* strdup(const char* s)
{
    char* buf = (char*)RtlAllocateHeap(GetProcessHeap(), 0, strlen(s) + 1);
    if (buf) strcpy(buf, s);
    return buf;
}

long long atoll(const char* str)
{
    ULONG tmp;
    RtlCharToInteger(str, 10, &tmp);
    return (LONGLONG)tmp;
}

long long strtoll(const char* restrict str, char** restrict str_end, int base)
{
    // FIXME: it kinda work, but not identical to the C version.
    ULONG tmp;
    if (base == 0) {
        NTSTATUS status = RtlCharToInteger(str, 10, &tmp);
        if (status != STATUS_SUCCESS) RtlCharToInteger(str, 16, &tmp);
    } else {
        RtlCharToInteger(str, base, &tmp);
    }
    return (LONGLONG)tmp;
}

BOXFILE* box_fopen(const char* filename, const char* mode)
{
    DWORD dwDesiredAccess = 0;
    DWORD dwCreationDisposition = 0;

    if (strcmp(mode, "r") == 0) {
        dwDesiredAccess = GENERIC_READ;
        dwCreationDisposition = OPEN_EXISTING;
    } else if (strcmp(mode, "w") == 0) {
        dwDesiredAccess = GENERIC_WRITE;
        dwCreationDisposition = CREATE_ALWAYS;
    } else if (strcmp(mode, "a") == 0) {
        dwDesiredAccess = FILE_APPEND_DATA;
        dwCreationDisposition = OPEN_ALWAYS;
    } else {
        return NULL;
    }

    HANDLE hFile = CreateFileA(
        filename,
        dwDesiredAccess,
        FILE_SHARE_READ,
        NULL,
        dwCreationDisposition,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE) return NULL;

    BOXFILE* file = (BOXFILE*)WinMalloc(sizeof(BOXFILE));
    if (!file) {
        CloseHandle(hFile);
        return NULL;
    }

    file->hFile = hFile;
    file->buf_pos = 0;
    file->buf_size = 0;
    file->eof = 0;

    return file;
}

char* box_fgets(char* str, int num, BOXFILE* stream)
{
    if (stream == NULL || str == NULL || num <= 0 || stream->eof) return NULL;

    int i = 0;
    while (i < num - 1) {
        if (stream->buf_pos >= stream->buf_size) {
            DWORD bytesRead;
            if (!ReadFile(stream->hFile, stream->buffer, BOXFILE_BUFSIZE, &bytesRead, NULL) || bytesRead == 0) {
                stream->eof = 1;
                break;
            }
            stream->buf_size = bytesRead;
            stream->buf_pos = 0;
        }

        char c = stream->buffer[stream->buf_pos++];
        str[i++] = c;

        if (c == '\n')
            break;
    }

    if (i == 0) return NULL;

    str[i] = '\0';
    return str;
}

int box_fclose(BOXFILE* stream)
{
    if (stream == NULL) return EOF;
    BOOL closed = CloseHandle(stream->hFile);
    WinFree(stream);
    return closed ? 0 : EOF;
}
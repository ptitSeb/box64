#ifndef __COMPILER_H_
#define __COMPILER_H_

/* Things missing from mingw64 right now */
#define ThreadWow64Context (29)

NTSTATUS WINAPI RtlWow64GetCurrentCpuArea(USHORT *, void **, void **);
NTSYSAPI NTSTATUS WINAPI LdrGetDllHandle(LPCWSTR, ULONG, const UNICODE_STRING*, HMODULE*);
NTSYSAPI void* WINAPI RtlFindExportedRoutineByName(HMODULE, const char*);

#endif //__COMPILER_H_

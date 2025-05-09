#ifndef __COMPILER_H_
#define __COMPILER_H_

/* Things missing from mingw64 right now */
#define ThreadWow64Context (29)

NTSTATUS WINAPI RtlWow64GetCurrentCpuArea(USHORT *, void **, void **);

#endif //__COMPILER_H_

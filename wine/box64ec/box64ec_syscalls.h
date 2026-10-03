// SPDX-License-Identifier: MIT
/*
 * Wine direct-syscall helpers for box64ec.
 */
#ifndef BOX64EC_SYSCALLS_H
#define BOX64EC_SYSCALLS_H

#include <windows.h>
#include <winternl.h>
#include <stdint.h>

extern void* Box64EC_WineSyscallDispatcher;
extern uint64_t Box64EC_WineNtContinueSyscallId;
/* Native ARM64EC #KiUserExceptionDispatcher (via ntdll CHPE redirection). */
extern uintptr_t Box64EC_KiUserExceptionDispatcher;

NTSTATUS Box64EC_InitSyscalls(void);
NTSTATUS Box64EC_ContinueNative(ARM64_NT_CONTEXT* ctx, BOOLEAN alert);

/* Asm: issues NtContinue via Wine dispatcher or SVC. */
NTSTATUS Box64EC_NtContinueNative(ARM64_NT_CONTEXT* ctx, BOOLEAN alert);

/* Asm: mov sp,x1; br x0 to continue into KiUser (H62). */
void Box64EC_JumpSetStack(uintptr_t pc, uintptr_t sp) __attribute__((noreturn));

#endif

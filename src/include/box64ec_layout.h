// SPDX-License-Identifier: MIT
#ifndef BOX64EC_LAYOUT_H
#define BOX64EC_LAYOUT_H

/* x64emu_t offsets used by ARM64EC assembly, asserted in box64ec_callret.c. */
#define BOX64EC_EMU_CALLRET_STACK_OFFSET   0xc30
#define BOX64EC_EMU_CALLRET_SP_OFFSET      0xc38
#define BOX64EC_EMU_CALLRET_SP_TOP_OFFSET  0xc40
#define BOX64EC_EMU_JIT_FRAME_OFFSET       0xc50
#define BOX64EC_EMU_XMM_OFFSET             0x090
#define BOX64EC_EMU_MXCSR_OFFSET           0x320
#define BOX64EC_EMU_LIVE_MXCSR_OFFSET      0xc58
#define BOX64EC_EMU_LIVE_FPCR_OFFSET       0xc5c
#define BOX64EC_EMU_LIVE_FPSR_OFFSET       0xc60
#define BOX64EC_EMU_EFLAGS_OFFSET          0x080
#define BOX64EC_EMU_DF_OFFSET              0x44c
#define BOX64EC_EMU_SW_OFFSET              0x310
#define BOX64EC_EMU_TOP_OFFSET             0x314
#define BOX64EC_EMU_CW_OFFSET              0x31c
#define BOX64EC_EMU_QUIT_OFFSET            0x540
/* box64ec_thr_t, asserted in box64ec_fastpath.c */
#define BOX64EC_THR_PENDING_EXIT_OFFSET      0x000
#define BOX64EC_THR_PENDING_NATIVE_SP_OFFSET 0x008
#define BOX64EC_THR_ENTRY_SP_OFFSET          0x010
#define BOX64EC_THR_ENTER_DEPTH_OFFSET       0x020
#define BOX64EC_THR_PENDING_CALLBACK_DEPTH_OFFSET 0x02c
#define BOX64EC_THR_PENDING_CALLBACK_SEQ_OFFSET   0x038
#define BOX64EC_THR_EXCEPTION_PENDING_OFFSET 0x040
/* PEB ChpeV2ProcessInfo, its CrossProcessWorkList and the list's work_list.first (Wine winternl.h) */
#define BOX64EC_PEB_CHPEV2_INFO_OFFSET       0x338
#define BOX64EC_CHPEV2_WORK_LIST_OFFSET      0x010
#define BOX64EC_WORK_LIST_FIRST_OFFSET       0x008

/* Fast native call record tag, plus the CALLRET bit when the call's pair is above it */
#define BOX64EC_FAST_RECORD_TAG_LOW        0x5354
#define BOX64EC_FAST_RECORD_TAG_HIGH       0x4641
#define BOX64EC_FAST_RECORD_TAG            \
    ((BOX64EC_FAST_RECORD_TAG_HIGH << 16) | BOX64EC_FAST_RECORD_TAG_LOW)
#define BOX64EC_FAST_RECORD_CALLRET        1

#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static inline uint32_t read_eflags(void) {
  uint32_t f;
  __asm__ volatile(
      "pushfl\n\t"
      "popl %0\n\t"
      : "=r"(f)
      :
      : "memory");
  return f;
}

static inline void write_eflags(uint32_t f) {
  __asm__ volatile(
      "pushl %0\n\t"
      "popfl\n\t"
      :
      : "r"(f)
      : "cc", "memory");
}

static inline uint32_t do_arpl(uint32_t *eax_inout, uint32_t ebx, uint32_t eflags_in) {
  uint32_t flags_out;
  uint32_t eax = *eax_inout;

  __asm__ volatile(
      "pushl %[fin]\n\t"
      "popfl\n\t"
      "arpl %%bx, %%ax\n\t"
      "pushfl\n\t"
      "popl %[fout]\n\t"
      : [fout] "=r"(flags_out), "+a"(eax)
      : "b"(ebx), [fin] "r"(eflags_in)
      : "cc", "memory");

  *eax_inout = eax;
  return flags_out;
}

int main(void) {
  // Only check the status flags ARPL is documented to preserve (all but ZF).
  // We use a mask of classic status flags: CF, PF, AF, ZF, SF, OF.
  const uint32_t kStatusMask = (1u << 0) | (1u << 2) | (1u << 4) | (1u << 6) | (1u << 7) | (1u << 11);
  const uint32_t kZF = (1u << 6);

  // Start from current flags so we don't fight reserved / privileged bits.
  uint32_t base = read_eflags();
  // Baseline: CF=1, PF=1, AF=0, ZF=0, SF=1, OF=1.
  uint32_t wanted = (base & ~kStatusMask) | (1u << 0) | (1u << 2) | (1u << 7) | (1u << 11);
  wanted &= ~kZF;

  for (uint32_t dst_rpl = 0; dst_rpl < 4; ++dst_rpl) {
    for (uint32_t src_rpl = 0; src_rpl < 4; ++src_rpl) {
      uint32_t eax = 0xFFFF0000u | (0x1FCu + dst_rpl); // low 16 bits end with RPL
      uint32_t ebx = 0xEEEE0000u | (0x201u + src_rpl); // low 16 bits end with RPL

      // Reset flags to known state before each ARPL.
      write_eflags(wanted);

      uint32_t flags = do_arpl(&eax, ebx, wanted);

      uint16_t dst_before = (uint16_t)(0x1FCu + dst_rpl);
      uint16_t src = (uint16_t)(0x201u + src_rpl);

      uint16_t exp_dst = dst_before;
      uint32_t need_update = ((dst_before & 0x3) < (src & 0x3));
      if (need_update) {
        exp_dst = (uint16_t)((dst_before & 0xFFFCu) | (src & 0x3u));
      }

      // Check destination low 16 and upper 16 untouched.
      if (((uint16_t)eax) != exp_dst) {
        printf("FAIL: dst mismatch dst_rpl=%u src_rpl=%u got=0x%04x exp=0x%04x\n",
               dst_rpl, src_rpl, (unsigned)((uint16_t)eax), (unsigned)exp_dst);
        return 1;
      }
      if ((eax & 0xFFFF0000u) != 0xFFFF0000u) {
        printf("FAIL: upper eax modified dst_rpl=%u src_rpl=%u got=0x%08x\n", dst_rpl, src_rpl, eax);
        return 1;
      }

      // Check ZF and that other status flags were preserved.
      uint32_t exp_flags = (wanted & ~kZF) | (need_update ? kZF : 0);
      if (((flags ^ exp_flags) & kStatusMask) != 0) {
        printf("FAIL: flags mismatch dst_rpl=%u src_rpl=%u got=0x%08x exp=0x%08x\n",
               dst_rpl, src_rpl, flags, exp_flags);
        return 1;
      }
    }
  }

  printf("arpl: ok\n");
  return 0;
}


// gcc -mfpmath=387 -O0 -o test34 test34.c -lm
// Test x87 FLD1, FLDZ, FLDPI and related constant-loading instructions.
// This catches a bug where the dynarec loads float bit patterns into FPRs
// that always hold doubles, producing denormals that underflow to zero.
#include <stdio.h>
#include <stdint.h>
#include <math.h>

int main(void) {
    volatile double d_one, d_zero, d_pi, d_log2e, d_log210, d_ln2, d_log102;
    volatile float f_one, f_zero, f_pi;
    volatile double d_neg_one;
    volatile float f_neg_one;

    // Test FLD1 -> store as double
    asm volatile("fld1\n\t"
                 "fstpl %0" : "=m"(d_one) : : "st");
    printf("fld1 (double) = %a (%.17g)\n", d_one, d_one);

    // Test FLD1 -> store as float (this is the buggy path)
    asm volatile("fld1\n\t"
                 "fstps %0" : "=m"(f_one) : : "st");
    printf("fld1 (float)  = %a (%.9g)\n", (double)f_one, (double)f_one);

    // Test FLD1 + FCHS -> store as float (produces -1.0, also buggy)
    asm volatile("fld1\n\t"
                 "fchs\n\t"
                 "fstps %0" : "=m"(f_neg_one) : : "st");
    printf("fld1+fchs (float) = %a (%.9g)\n", (double)f_neg_one, (double)f_neg_one);

    // Test FLD1 + FCHS -> store as double
    asm volatile("fld1\n\t"
                 "fchs\n\t"
                 "fstpl %0" : "=m"(d_neg_one) : : "st");
    printf("fld1+fchs (double) = %a (%.17g)\n", d_neg_one, d_neg_one);

    // Test FLDZ -> store as float and double
    asm volatile("fldz\n\t"
                 "fstps %0" : "=m"(f_zero) : : "st");
    printf("fldz (float)  = %a (%.9g)\n", (double)f_zero, (double)f_zero);
    asm volatile("fldz\n\t"
                 "fstpl %0" : "=m"(d_zero) : : "st");
    printf("fldz (double) = %a (%.17g)\n", d_zero, d_zero);

    // Test FLDPI -> store as float and double
    asm volatile("fldpi\n\t"
                 "fstps %0" : "=m"(f_pi) : : "st");
    printf("fldpi (float)  = %a (%.9g)\n", (double)f_pi, (double)f_pi);
    asm volatile("fldpi\n\t"
                 "fstpl %0" : "=m"(d_pi) : : "st");
    printf("fldpi (double) = %a (%.17g)\n", d_pi, d_pi);

    // Test FLDL2E
    asm volatile("fldl2e\n\t"
                 "fstpl %0" : "=m"(d_log2e) : : "st");
    printf("fldl2e (double) = %a (%.17g)\n", d_log2e, d_log2e);

    // Test FLDL2T (log2(10))
    asm volatile("fldl2t\n\t"
                 "fstpl %0" : "=m"(d_log210) : : "st");
    printf("fldl2t (double) = %a (%.17g)\n", d_log210, d_log210);

    // Test FLDLN2 (ln(2))
    asm volatile("fldln2\n\t"
                 "fstpl %0" : "=m"(d_ln2) : : "st");
    printf("fldln2 (double) = %a (%.17g)\n", d_ln2, d_ln2);

    // Test FLDLG2 (log10(2))
    asm volatile("fldlg2\n\t"
                 "fstpl %0" : "=m"(d_log102) : : "st");
    printf("fldlg2 (double) = %a (%.17g)\n", d_log102, d_log102);

    // Test FLD1 arithmetic: 1.0 + 1.0 = 2.0
    volatile double d_two;
    asm volatile("fld1\n\t"
                 "fld1\n\t"
                 "faddp\n\t"
                 "fstpl %0" : "=m"(d_two) : : "st");
    printf("fld1+fld1 (double) = %a (%.17g)\n", d_two, d_two);

    // Test FLD1 arithmetic stored as float: 1.0 + 1.0 = 2.0
    volatile float f_two;
    asm volatile("fld1\n\t"
                 "fld1\n\t"
                 "faddp\n\t"
                 "fstps %0" : "=m"(f_two) : : "st");
    printf("fld1+fld1 (float) = %a (%.9g)\n", (double)f_two, (double)f_two);

    // Test FPTAN: computes tan(st0) and pushes 1.0
    // Push pi/4, compute tan(pi/4), then FPTAN pushes 1.0
    // We only verify the pushed 1.0, not the tangent result (which differs
    // between x86 80-bit microcode and other architectures' libm).
    volatile double d_tan_result, d_fptan_one;
    asm volatile("fldpi\n\t"       // st0 = pi
                 "pushl $4\n\t"
                 "fildl (%%esp)\n\t" // st0 = 4, st1 = pi
                 "addl $4, %%esp\n\t"
                 "fdivp\n\t"       // st0 = pi/4
                 "fptan\n\t"       // st0 = 1.0, st1 = tan(pi/4)
                 "fstpl %0\n\t"    // pop 1.0
                 "fstpl %1"        // pop tan result
                 : "=m"(d_fptan_one), "=m"(d_tan_result) : : "st", "memory");
    printf("fptan pushes 1.0 = %a (%.17g)\n", d_fptan_one, d_fptan_one);

    // Test FPTAN stored as float
    volatile float f_fptan_one;
    asm volatile("fldpi\n\t"
                 "pushl $4\n\t"
                 "fildl (%%esp)\n\t"
                 "addl $4, %%esp\n\t"
                 "fdivp\n\t"
                 "fptan\n\t"
                 "fstps %0\n\t"   // pop 1.0 as float
                 "fstp %%st(0)"    // discard tan result
                 : "=m"(f_fptan_one) : : "st", "st(1)", "memory");
    printf("fptan pushes 1.0 (float) = %a (%.9g)\n", (double)f_fptan_one, (double)f_fptan_one);

    // Verify that values are what they should be (integral checks)
    int errors = 0;
    if (f_one != 1.0f) { printf("ERROR: fld1 float != 1.0\n"); errors++; }
    if (f_neg_one != -1.0f) { printf("ERROR: fld1+fchs float != -1.0\n"); errors++; }
    if (d_one != 1.0) { printf("ERROR: fld1 double != 1.0\n"); errors++; }
    if (d_neg_one != -1.0) { printf("ERROR: fld1+fchs double != -1.0\n"); errors++; }
    if (f_zero != 0.0f) { printf("ERROR: fldz float != 0.0\n"); errors++; }
    if (d_zero != 0.0) { printf("ERROR: fldz double != 0.0\n"); errors++; }
    if (f_fptan_one != 1.0f) { printf("ERROR: fptan float != 1.0\n"); errors++; }
    if (d_fptan_one != 1.0) { printf("ERROR: fptan double != 1.0\n"); errors++; }
    if (f_two != 2.0f) { printf("ERROR: fld1+fld1 float != 2.0\n"); errors++; }
    if (d_two != 2.0) { printf("ERROR: fld1+fld1 double != 2.0\n"); errors++; }
    printf("x87 constant tests: %d errors\n", errors);

    return errors;
}

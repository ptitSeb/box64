#ifndef __X64TEST_H_
#define __X64TEST_H_

typedef struct x64test_s x64test_t;
int RunTest(x64test_t* test);

void x64test_step(x64emu_t* ref, uintptr_t ip);
void x64test_check(x64emu_t* ref, uintptr_t ip);

#endif // __X64TEST_H_

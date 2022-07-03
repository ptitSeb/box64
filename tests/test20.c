#include <stdio.h>
int myfunc1() { return 1; }
int myfunc2() { return 2; }

// Prototype for the common entry point
/*extern "C" */int myfunc();
__asm__ (".type myfunc, @gnu_indirect_function");
// Make the dispatcher function. This returns a pointer to the desired function version
typeof(myfunc) * myfunc_dispatch (void) __asm__ ("myfunc");
typeof(myfunc) * myfunc_dispatch (void)  {
if (0) 
  return &myfunc1;
else
  return &myfunc2;
}

int main() {
   printf("\nCalled function number %i\n", myfunc());
   return 0;
}


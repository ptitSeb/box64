#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/// build with `gcc -march=core2 -g -O2 test19.c -o test19 -no-pie -rdynamic`

#define BT_BUF_SIZE 100

void myfunc3()
{
  int nptrs;
  void *buffer[BT_BUF_SIZE];
  char **strings;

  nptrs = backtrace(buffer, BT_BUF_SIZE);
  printf("backtrace() returned %d addresses\n", nptrs);

 /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
    would produce similar output to the following: */

  strings = backtrace_symbols(buffer, nptrs);
  if (strings == NULL) {
    perror("backtrace_symbols");
    exit(EXIT_FAILURE);
  }

  for (int j = 0; j < nptrs; j++) {
    // clean-up output so it can be compared
    char* p = strchr(strings[j], '[');
    if(p)
      p[-1] = '\0';
    p = strchr(strings[j], '(');
    if(p)
      *p = ':';
    p = strchr(p?p:strings[j], '+');
    if(p)
      *p = '\0';
    p = strchr(p?p:strings[j], ')');
    if(p)
      *p = '\0';
    p = strchr(strings[j], ':');
    if(!p)
      p = strings[j];
    if(!strcmp(p, ":ExitEmulation"))
      p = "???";
    printf("%s\n", p);
  }
  free(strings);
}

static void   /* "static" means don't export the symbol... */
myfunc2(void)
{
  myfunc3();
}

void myfunc(int ncalls)
{
  if (ncalls > 1)
    myfunc(ncalls - 1);
  else
    myfunc2();
}

int main(int argc, char *argv[])
{
  int ncall = 4;
  if (argc == 2) {
    ncall = atoi(argv[1]);
  }

  myfunc(ncall);
  exit(EXIT_SUCCESS);
}

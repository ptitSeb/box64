#include <stdio.h>

int main(int argc, char **argv)
{
    printf("Hello, argc=%d argv[%d]=%s\n", argc, argc-1, argv[argc-1]);
    return 0;
}

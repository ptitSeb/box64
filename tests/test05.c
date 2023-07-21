#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int fact(int i) {
    if (i<2) return i;
    return i*fact(i-1);
}

#define SET(M) dels[M/8] |= (1<<(M%8))
#define GET(M) ((dels[M/8]>>(M%8))&1)

int main(int argc, const char** argv)
{
    int j = 5;
    if(argc>1)
        j = atoi(argv[1]);
    if(j==0)
        j=5;
    if(j>15) j=15;
    
    int k = fact(j);
    printf("fact(%d)=%d\n", j, k);

    uint8_t* dels = (char*)calloc((k+7)/8, 1);
    SET(0);
    SET(1);
 
    for (int i=2; i<k; i++)
        if (!GET(i)) {
            int m = 2 * i;
            while (m < k) {
                SET(m);
                m += i;
            }
        }
 
    printf("Prime list 0..%d: ", k);
    for (int i=0; i<k; i++)
        if (!GET(i))
            printf("%d ", i);
    printf("\n");

    free(dels);

    signed char sc = -5;
    unsigned char uc = 83;
    signed short int ss = -53;
    unsigned short int us = 65500;

    int it = sc+uc+ss+us;

    printf("(un)signed char = %hhd/%hhu (un)signed int = %hd/%hu total=%d\n", sc, uc, ss, us, it);

    printf("%hu/5=%hu, %hu%%5=%hu\n", us, us/5, us, us%5);
    printf("%d/5=%d, %d%%5=%d\n", it, it/5, it, it%5);

    printf("%d/%hd=%d + %d", it, ss, it/ss, it%ss);

    return 0;
}
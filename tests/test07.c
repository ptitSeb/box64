#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(int argc, char **argv)
{
	long double zero = 0.0;

	double si = sin(M_PI / 2.0);

	int a = 3;
	float b = 1.5;
	float mul = a * b;

	printf("0 is %Lf, sin(pi/2) is %f and 3*1.5 is %f.\n", zero, si, mul);
	return 0;
}

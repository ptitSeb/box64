#include <math.h>

double __cdecl ldexp(double x, int n)
{
	return scalbn(x, n);
}

#include "WaveletKernelGeneratorHelper.h"

int positive_mod(int dividend, int divisor)
{
	return ((dividend % divisor) >= 0) ? dividend % divisor : (dividend % divisor) + divisor;
}

int positive_div(int dividend, int divisor)
{
	return (dividend >= 0) ? dividend / divisor : (dividend - divisor + 1) / divisor;
}
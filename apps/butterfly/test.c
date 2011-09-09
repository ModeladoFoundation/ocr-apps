#include "FFT.h"

int main()
{
	point *X = buildDataPoints("Input/default.txt");
	point *fftPoints = FourierTransform(X, getNumPoints(), 1);

	return 0;
}

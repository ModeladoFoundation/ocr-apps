#include "ocr.h"
#include "rag_ocr.h"
#include "common.h"

void sinc_interp(float *X, struct complexData *Y, struct complexData *YI, int Nz, float B, int M, int lenY)
{
	int m, n;
	int lenYI;
	float T;
	float Tprime;
	float offset;
	float Gcurrent;
	float Gleft, Gright;
	float *Xtmp;
	ocrGuid_t Xtmp_dbg;

	for(m=0, T=0; m<lenY-1; m++) {
		T += (X[m+1]-X[m]);
	}

	T /= lenY;	// Mean of input sample spacing
	Tprime = 1/B;	// Output sample spacing
	lenYI = lenY/M;	// Length of output

	Xtmp = (float*)bsm_malloc(&Xtmp_dbg, lenY*sizeof(float));

	offset = fmodf(X[0], Tprime);

	// Shift input time vector relative to first output sample
	for(m=0; m<lenY; m++) {
		Xtmp[m] = X[m] - X[0] + offset;
	}

	for(m=0; m<lenYI; m++)
	{
		YI[m].real = 0;
		YI[m].imag = 0;

		Gcurrent = m*Tprime;

		Gleft = Gcurrent - Nz*Tprime; // time sinc begins
		if(Gleft < 0) {
			Gleft = 0;
		}

		Gright = Gcurrent + Nz*Tprime; // time sinc ends
		if(Gright > (lenYI-1)*Tprime) {
			Gright = (lenYI-1)*Tprime;
		}

		for(n=0; n<lenY; n++)
		{
			if(Xtmp[n] >= Gleft && Xtmp[n] <= Gright) {
				YI[m].real += Y[n].real*sinc(B*(Xtmp[n]-Gcurrent));
				YI[m].imag += Y[n].imag*sinc(B*(Xtmp[n]-Gcurrent));
			}
			else if(Xtmp[n] > Gright) {
				break;
			}
		}

		YI[m].real *= T/Tprime;
		YI[m].imag *= T/Tprime;
	}

	bsm_free(Xtmp,Xtmp_dbg);
}

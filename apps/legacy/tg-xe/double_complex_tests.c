#include<stdio.h>
#include<math.h>
#include<complex.h>

int main()
{
    double complex result, arg1, arg2, arg3;
    int int1;
    double f1,f2;

    arg1 = 0 + 0*I;
    result = ccosf(arg1);
    printf("double complex ccosf(%.16f %.16f) = %.16f %.16f\n",creal(arg1), cimag(arg1), creal(result), cimag(result));

    result = cexpf(arg1);
    printf("double complex cexpf(%.16f %.16f) = %.16f %.16f\n",creal(arg1), cimag(arg1), creal(result), cimag(result));

    arg1 = 0 + M_PI*I;
    result = ccosf(arg1);
    printf("double complex ccosf(%.16f %.16f) = %.16f %.16f\n",creal(arg1), cimag(arg1), creal(result), cimag(result));

    arg1 = M_PI + 0*I;
    result = ccosf(arg1);
    printf("double complex ccosf(%.16f %.16f) = %.16f %.16f\n",creal(arg1), cimag(arg1), creal(result), cimag(result));

    result = conjf(arg1);
    printf("double complex conjf(%.16f %.16f) = %.16f %.16f\n",creal(arg1), cimag(arg1), creal(result), cimag(result));

    f1 = nanf("NaN");
    f2 = f1;
    arg1 = f1 + f2*I;
    result = ccosf(arg1);
    printf("double complex conjf(%.16f %.16f) = %.16f %.16f\n",creal(arg1), cimag(arg1), creal(result), cimag(result));

    return 0;
}

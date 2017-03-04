#include<stdio.h>
#include<math.h>
#include<complex.h>

int main()
{
    float complex result, arg1, arg2, arg3;
    int int1;
    float f1,f2;

    arg1 = 0 + 0*I;
    result = ccosf(arg1);
    printf("float complex ccosf(%.8f %.8f) = %.8f %.8f\n",crealf(arg1), cimagf(arg1), crealf(result), cimagf(result));

    result = cexpf(arg1);
    printf("float complex cexpf(%.8f %.8f) = %.8f %.8f\n",crealf(arg1), cimagf(arg1), crealf(result), cimagf(result));

    arg1 = 0 + M_PI*I;
    result = ccosf(arg1);
    printf("float complex ccosf(%.8f %.8f) = %.8f %.8f\n",crealf(arg1), cimagf(arg1), crealf(result), cimagf(result));

    arg1 = M_PI + 0*I;
    result = ccosf(arg1);
    printf("float complex ccosf(%.8f %.8f) = %.8f %.8f\n",crealf(arg1), cimagf(arg1), crealf(result), cimagf(result));

    result = conjf(arg1);
    printf("float complex conjf(%.8f %.8f) = %.8f %.8f\n",crealf(arg1), cimagf(arg1), crealf(result), cimagf(result));

    f1 = nanf("NaN");
    f2 = f1;
    arg1 = f1 + f2*I;
    result = ccosf(arg1);
    printf("float complex conjf(%.8f %.8f) = %.8f %.8f\n",crealf(arg1), cimagf(arg1), crealf(result), cimagf(result));

    return 0;
}

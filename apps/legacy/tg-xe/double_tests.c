#define _GNU_SOURCE
#include<stdio.h>
#include<math.h>

// the _GNU_SOURCE is there so that sincos is available in native x86

int main()
{
    double result, arg1, arg2, arg3;
    int int1;
    double frac;

    arg1 = 2;
    result = sqrt(arg1);
    printf("double sqrt(%.16f) = %.16f\n",arg1, result);

    arg1 = 0;
    result = cos(arg1);
    printf("double cos(%.16f) = %.16f\n",arg1, result);

    result = exp(arg1);
    printf("double exp(%.16f) = %.16f\n",arg1, result);

    arg1 = M_PI_4;
    result = cos(arg1);
    printf("double cos(%.16f) = %.16f\n",arg1, result);

    arg1 = M_PI_2;
    result = cos(arg1);
    printf("double cos(%.16f) = %.16f\n",arg1, result);

    arg1 = M_PI_2;
    sincos(arg1,&arg2,&arg3);
    printf("double sincos(%.16f) = %.16f %.16f\n",arg1, arg2, arg3);

    arg1 = M_PI_2;
    result = ceil(arg1);
    printf("double ceil(%.16f) = %.16f\n",arg1, result);

    arg1 = M_PI_2;
    result = floor(arg1);
    printf("double floor(%.16f) = %.16f\n",arg1, result);

    arg1 = -1.0;
    result = fabs(arg1);
    printf("double fabs(%.16f) = %.16f\n",arg1, result);

    arg1 = 2560;
    result = frexp(arg1,&int1);
    printf("double frexp(%.16f) = %.16f %d\n",arg1, result, int1);

    arg1 = M_PI;
    result = modf(arg1,&frac);
    printf("double modf(%.16f) = %.16f %.16f\n",arg1, result, frac);

    arg1 = 5.0;
    arg2 = 2.0;
    result = remquo(arg1,arg2,&int1);
    printf("double remquo(%.16f,%.16f) = %.16f %d\n",arg1,arg2, result, int1);

    arg3 = 3.0;
    result = fma(arg1,arg2,arg3);
    printf("double fma(%.16f,%.16f,%.16f) = %.16f\n",arg1,arg2,arg3,result);

    int1 = 2;
    result = scalbn(arg1,int1);
    printf("double scalbnf(%.16f,%d) = %.16f\n",arg1,int1,result);

    arg1 = nan("NaN");
    arg2 = arg1;
    arg3 = arg1;

    result = cos(arg1);
    printf("double cos(%.16f) = %.16f\n",arg1, result);

    result = ceil(arg1);
    printf("double ceil(%.16f) = %.16f\n",arg1, result);

    result = floor(arg1);
    printf("double floor(%.16f) = %.16f\n",arg1, result);

    result = fabs(arg1);
    printf("double fabs(%.16f) = %.16f\n",arg1, result);

    result = frexp(arg1,&int1);
    printf("double frexp(%.16f) = %.16f %d\n",arg1, result, int1);

    result = modf(arg1,&frac);
    printf("double modf(%.16f) = %.16f %.16f\n",arg1, result, frac);

    result = remquo(arg1,arg2,&int1);
    printf("double remquo(%.16f,%.16f) = %.16f %d\n",arg1,arg2, result, int1);

    result = fma(arg1,arg2,arg3);
    printf("double fma(%.16f,%.16f,%.16f) = %.16f\n",arg1,arg2,arg3,result);

    result = scalbn(arg1,int1);
    printf("double scalbn(%.16f,%d) = %.16f\n",arg1,int1,result);

    arg1 = INFINITY;
    result = atan(arg1);
    printf("double atan(%f) = %.16f\n",arg1,result);

    return 0;
}

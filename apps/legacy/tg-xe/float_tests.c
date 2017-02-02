#define _GNU_SOURCE
#include<stdio.h>
#include<math.h>

// the _GNU_SOURCE is there so that sincosf is available in native x86

int main()
{
    float result, arg1, arg2, arg3;
    int int1;
    float frac;

    arg1 = 2;
    result = sqrtf(arg1);
    printf("float sqrtf(%.8f) = %.8f\n",arg1, result);

    arg1 = 0;
    result = cosf(arg1);
    printf("float cosf(%.8f) = %.8f\n",arg1, result);

    result = expf(arg1);
    printf("float expf(%.8f) = %.8f\n",arg1, result);

    arg1 = M_PI_4;
    result = cosf(arg1);
    printf("float cosf(%.8f) = %.8f\n",arg1, result);

    arg1 = M_PI_2;
    result = cosf(arg1);
    printf("float cosf(%.8f) = %.8f\n",arg1, result);

    arg1 = M_PI_2;
    sincosf(arg1,&arg2,&arg3);
    printf("float sincosf(%.8f) = %.8f %.8f\n",arg1, arg2, arg3);

    arg1 = M_PI_2;
    result = ceilf(arg1);
    printf("float ceilf(%.8f) = %.8f\n",arg1, result);

    arg1 = M_PI_2;
    result = floorf(arg1);
    printf("float floorf(%.8f) = %.8f\n",arg1, result);

    arg1 = -1.0;
    result = fabsf(arg1);
    printf("float fabsf(%.8f) = %.8f\n",arg1, result);

    arg1 = 2560;
    result = frexpf(arg1,&int1);
    printf("float frexpf(%.8f) = %.8f %d\n",arg1, result, int1);

    arg1 = M_PI;
    result = modff(arg1,&frac);
    printf("float modff(%.8f) = %.8f %.8f\n",arg1, result, frac);

    arg1 = 5.0;
    arg2 = 2.0;
    result = remquof(arg1,arg2,&int1);
    printf("float remquof(%.8f,%.8f) = %.8f %d\n",arg1,arg2, result, int1);

    arg3 = 3.0;
    result = fmaf(arg1,arg2,arg3);
    printf("float fmaf(%.8f,%.8f,%.8f) = %.8f\n",arg1,arg2,arg3,result);

    int1 = 2;
    result = scalbnf(arg1,int1);
    printf("float scalbnf(%.8f,%d) = %.8f\n",arg1,int1,result);

    arg1 = nanf("NaN");
    arg2 = arg1;
    arg3 = arg1;

    result = cosf(arg1);
    printf("float cosf(%.8f) = %.8f\n",arg1, result);

    result = ceilf(arg1);
    printf("float ceilf(%.8f) = %.8f\n",arg1, result);

    result = floorf(arg1);
    printf("float floorf(%.8f) = %.8f\n",arg1, result);

    result = fabsf(arg1);
    printf("float fabsf(%.8f) = %.8f\n",arg1, result);

    result = frexpf(arg1,&int1);
    printf("float frexpf(%.8f) = %.8f %d\n",arg1, result, int1);

    result = modff(arg1,&frac);
    printf("float modff(%.8f) = %.8f %.8f\n",arg1, result, frac);

    result = remquof(arg1,arg2,&int1);
    printf("float remquof(%.8f,%.8f) = %.8f %d\n",arg1,arg2, result, int1);

    result = fmaf(arg1,arg2,arg3);
    printf("float fmaf(%.8f,%.8f,%.8f) = %.8f\n",arg1,arg2,arg3,result);

    result = scalbnf(arg1,int1);
    printf("float scalbnf(%.8f,%d) = %.8f\n",arg1,int1,result);

    arg1 = INFINITY;
    result = atanf(arg1);
    printf("float atanf(%.8f) = %.8f\n",arg1,result);

    return 0;
}

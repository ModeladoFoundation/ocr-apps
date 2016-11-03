/******************************************************************
 * sincos
 *
 * Input:
 *   x - floating point value
 *
 * Output:
 *   sin of x (float) and cos of x (float)
 *
 * Description:
 *   This routine returns the sin and cos of x.
 *
 *****************************************************************/

#include <math.h>

void sincosf(float x, float* sinx, float* cosx) {
    __asm__ (
        "sincosF %0, %1, %2, 32\n\t"
        : "=r" (*sinx), "=r" (*cosx)
        : "r" (x)
        :
    );
}


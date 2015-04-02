
/* @(#)z_sinf.c 1.0 98/08/13 */
/******************************************************************
 * Sine
 *
 * Input:
 *   x - floating point value
 *
 * Output:
 *   sine of x
 *
 * Description:
 *   This routine returns the sine of x.
 *
 *****************************************************************/

#include <math.h>

void sincosf(float x, float* sinx, float* cosx) {
    __asm__ (
        "sincosF %1, %0, %2, 32\n\t"
        : "=r" (*sinx), "=r" (*cosx)
        : "r" (x)
        :
    );
}


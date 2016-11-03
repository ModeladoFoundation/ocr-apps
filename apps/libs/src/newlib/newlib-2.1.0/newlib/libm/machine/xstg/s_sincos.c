/******************************************************************
 * sincos
 *
 * Input:
 *   x - double value
 *
 * Output:
 *   sin of x (double) and cos of x (double)
 *
 * Description:
 *   This routine returns the sin and cos of x.
 *
 *****************************************************************/

#include <math.h>

void sincos(double x, double* sinx, double* cosx) {
    __asm__ (
        "sincosF %0, %1, %2, 64\n\t"
        : "=r" (*sinx), "=r" (*cosx)
        : "r" (x)
        :
    );
}

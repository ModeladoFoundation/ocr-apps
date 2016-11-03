/******************************************************************
 * mulh
 *
 * Input:
 *   x - floating point value
 *   y - floating point value
 *
 * Output:
 *   high (float) - upper 64 bits of x * y
 *   low (float) - lower 64 bits of x * y
 *
 * Description:
 *   This routine returns the 128-bit value of two 64-bit numbers,
 *   x and y, multiplied together.
 *   Support for both unsigned and signed operations.
 *
 *****************************************************************/

#include <math.h>

void mulhu64(double x, double y, double* high, double* low) {
    __asm__ (
        "mulhU %0, %1, %2, %3, 64\n\t"
        : "=r" (*high), "=r" (*low)
        : "r" (x), "r" (y)
        :
    );
}

void mulhs64(double x, double y, double* high, double* low) {
    __asm__ (
        "mulhS %0, %1, %2, %3, 64\n\t"
        : "=r" (*high), "=r" (*low)
        : "r" (x), "r" (y)
        :
    );
}

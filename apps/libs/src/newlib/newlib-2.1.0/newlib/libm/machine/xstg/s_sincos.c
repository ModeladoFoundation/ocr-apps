
/* @(#)z_sin.c 1.0 98/08/13 */
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

void sincos(double x, double* sinx, double* cosx) {
  float sinxf, cosxf;
  sincosf ((float) x, &sinxf, &cosxf);
  *sinx = (double) sinxf;
  *cosx = (double) cosxf;
}

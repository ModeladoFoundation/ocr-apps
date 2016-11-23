/*************************************************************************************/
/*      Copyright 2015 Barcelona Supercomputing Center                               */
/*                                                                                   */
/*      This file is part of the NANOS++ library.                                    */
/*                                                                                   */
/*      NANOS++ is free software: you can redistribute it and/or modify              */
/*      it under the terms of the GNU Lesser General Public License as published by  */
/*      the Free Software Foundation, either version 3 of the License, or            */
/*      (at your option) any later version.                                          */
/*                                                                                   */
/*      NANOS++ is distributed in the hope that it will be useful,                   */
/*      but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/*      GNU Lesser General Public License for more details.                          */
/*                                                                                   */
/*      You should have received a copy of the GNU Lesser General Public License     */
/*      along with NANOS++.  If not, see <http://www.gnu.org/licenses/>.             */
/*************************************************************************************/

#ifndef UTIL_H
#define UTIL_H

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/times.h>

#   include <mkl.h>
#   define malloc(x) mkl_malloc(x, 128)
#   define free(x)   mkl_free(x)

enum blas_cmach_type {
	blas_base      = 151,
	blas_t         = 152,
	blas_rnd       = 153,
	blas_ieee      = 154,
	blas_emin      = 155,
	blas_emax      = 156,
	blas_eps       = 157,
	blas_prec      = 158,
	blas_underflow = 159,
	blas_overflow  = 160,
	blas_sfmin     = 161};

double BLAS_dfpinfo( enum blas_cmach_type cmach );

static float get_time()
{
	static double gtod_ref_time_sec = 0.0;

	struct timeval tv;
	gettimeofday(&tv, NULL);

	// If this is the first invocation of through dclock(), then initialize the
	// "reference time" global variable to the seconds field of the tv struct.
	if (gtod_ref_time_sec == 0.0)
		gtod_ref_time_sec = (double) tv.tv_sec;

	// Normalize the seconds field of the tv struct so that it is relative to the
	// "reference time" that was recorded during the first invocation of dclock().
	const double norm_sec = (double) tv.tv_sec - gtod_ref_time_sec;

	// Compute the number of seconds since the reference time.
	const double t = norm_sec + tv.tv_usec * 1.0e-6;

	return (float) t;
}

#endif // UTIL_H


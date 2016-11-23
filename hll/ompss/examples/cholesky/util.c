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

#include "util.h"

#include <assert.h>

#include <math.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static inline void BLAS_error(const char *rname, int err, int val, int x) {
	fprintf( stderr, "%s %d %d %d\n", rname, err, val, x );
	abort();
}

static inline double BLAS_dpow_di(double x, unsigned n) {
	double rv = 1.0;

	if (n < 0) {
		n = -n;
		x = 1.0 / x;
	}

	for (; n; n >>= 1, x *= x) {
		if (n & 1)
			rv *= x;
	}

	return rv;
}

double BLAS_dfpinfo( enum blas_cmach_type cmach ) {
	const double b = 2.0;
	const int t = 53, l = 1024, m = -1021;

	/* for (i = 0; i < t; ++i) eps *= half; */
	const double eps = BLAS_dpow_di( b, -t );
	/* for (i = 0; i >= m; --i) r *= half; */
	const double r = BLAS_dpow_di( b, m-1 );

	double o = 1.0;
	o -= eps;
	/* for (i = 0; i < l; ++i) o *= b; */
	o = (o * BLAS_dpow_di( b, l-1 )) * b;

	switch (cmach) {
		case blas_eps: return eps;
		case blas_sfmin: return r;
		default:
			BLAS_error( __func__, -1, cmach, 0 );
			break;
	}
	return 0.0;
}


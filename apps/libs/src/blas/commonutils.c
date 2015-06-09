/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#define BLAS_TYPE__common

#include "blas.h"
#include <stdio.h>
#include <string.h>



BLAS_int_t ilaenv(
    BLAS_int_t ispec,
    char * name,
    char * opts,
    BLAS_int_t n1,
    BLAS_int_t n2,
    BLAS_int_t n3,
    BLAS_int_t n4 )
{

    switch (name[0]) {
    case 's':
    {
        if (strcmp (name, "sgetrf") == 0) {
            // When setting environment variables for GETRF, also consider the following:
            //   *  Option 1001 to GETF2
            //   *  Any environment variable pertaining to GEMM
            if (ispec == 1) {  // Asking for "optimal" block size.
                printf ("***************** ilaenv is returning 96 for request from %s\n", name);
                return 96;
            }
        }

        if (strcmp (name, "sgetf2") == 0) {
            if (ispec == 1001) { // ***** INTEL EXTENSION:  Indicate whether GETRF should improve accuracy by increasing precision of certain intermediate results.
                                 // In particular, before GETF2 calls SCAL to multiply by reciprocal (instead of doing a divide), we increase the precision of the
                                 // reciprocation:  single becomes double;  double becomes long double.  Return 1 if this is desired; return 0 if not.
                printf ("***************** ilaenv is returning directive that %s should increase precision of certain intermediate results.\n", name);
                return 1;
            }
        }

        if (strcmp (name, "spotrf") == 0) {
            // When setting environment variables for POTRF, also consider the following:
            //   *  Any environment variable pertaining to GEMM
            if (ispec == 1) {  // Asking for "optimal" block size.
                printf ("***************** ilaenv is returning 96 for request from %s\n", name);
                return 96;
            }
        }

        if (strcmp (name, "strsm") == 0) {
            if (ispec == 1) { // ***** INTEL EXTENSION:  Asking for "optimal" block size, in terms of approximate number of flops
                printf ("***************** ilaenv is returning 20000 for request from %s\n", name);
                return 20000;
            }
        }

        printf ("Unknown request to ilaenv:  name = %s, opts = %s, ispec = %ld\n", name, opts, (u64) ispec);
        return -1;
    }
    break;
    case 'd':
    {
        if (strcmp (name, "dgetrf") == 0) {
            // When setting environment variables for GETRF, also consider the following:
            //   *  Option 1001 to GETF2
            //   *  Any environment variable pertaining to GEMM
            if (ispec == 1) {  // Asking for "optimal" block size.
                printf ("***************** ilaenv is returning 64 for request from %s\n", name);
                return 64;
            }
        }

        if (strcmp (name, "dgetf2") == 0) {
            if (ispec == 1001) { // ***** INTEL EXTENSION:  Indicate whether GETRF should improve accuracy by increasing precision of certain intermediate results.
                                 // In particular, before GETF2 calls SCAL to multiply by reciprocal (instead of doing a divide), we increase the precision of the
                                 // reciprocation:  single becomes double;  double becomes long double.  Return 1 if this is desired; return 0 if not.
                printf ("***************** ilaenv is returning directive that %s should increase precision of certain intermediate results.\n", name);
                return 1;
            }
        }

        if (strcmp (name, "dpotrf") == 0) {
            // When setting environment variables for POTRF, also consider the following:
            //   *  Any environment variable pertaining to GEMM
            if (ispec == 1) {  // Asking for "optimal" block size.
                printf ("***************** ilaenv is returning 64 for request from %s\n", name);
                return 64;
            }
        }

        if (strcmp (name, "dtrsm") == 0) {
            if (ispec == 1) { // ***** INTEL EXTENSION:  Asking for "optimal" block size, in terms of approximate number of flops
                printf ("***************** ilaenv is returning 20000 for request from %s\n", name);
                return 20000;
            }
        }

        printf ("Unknown request to ilaenv:  name = %s, opts = %s, ispec = %ld\n", name, opts, (u64) ispec);
        return -1;
    }
    break;
    case 'c':
    {
        if (strcmp (name, "cgetrf") == 0) {
            // When setting environment variables for GETRF, also consider the following:
            //   *  Option 1001 to GETF2
            //   *  Any environment variable pertaining to GEMM
            if (ispec == 1) {  // Asking for "optimal" block size.
                printf ("***************** ilaenv is returning 64 for request from %s\n", name);
                return 64;
            }
        }

        if (strcmp (name, "cgetf2") == 0) {
            if (ispec == 1001) { // ***** INTEL EXTENSION:  Indicate whether GETRF should improve accuracy by increasing precision of certain intermediate results.
                                 // In particular, before GETF2 calls SCAL to multiply by reciprocal (instead of doing a divide), we increase the precision of the
                                 // reciprocation:  single becomes double;  double becomes long double.  Return 1 if this is desired; return 0 if not.
                printf ("***************** ilaenv is returning directive that %s should increase precision of certain intermediate results.\n", name);
                return 1;
            }
        }

        if (strcmp (name, "cpotrf") == 0) {
            // When setting environment variables for POTRF, also consider the following:
            //   *  Any environment variable pertaining to GEMM
            if (ispec == 1) {  // Asking for "optimal" block size.
                printf ("***************** ilaenv is returning 64 for request from %s\n", name);
                return 64;
            }
        }

        if (strcmp (name, "ctrsm") == 0) {
            if (ispec == 1) { // ***** INTEL EXTENSION:  Asking for "optimal" block size, in terms of approximate number of flops
                printf ("***************** ilaenv is returning 20000 for request from %s\n", name);
                return 20000;
            }
        }

        printf ("Unknown request to ilaenv:  name = %s, opts = %s, ispec = %ld\n", name, opts, (u64) ispec);
        return -1;
    }
    break;
    case 'z':
    {
        if (strcmp (name, "zgetrf") == 0) {
            // When setting environment variables for GETRF, also consider the following:
            //   *  Option 1001 to GETF2
            //   *  Any environment variable pertaining to GEMM
            if (ispec == 1) {  // Asking for "optimal" block size.
                printf ("***************** ilaenv is returning 48 for request from %s\n", name);
                return 48;
            }
        }

        if (strcmp (name, "zgetf2") == 0) {
            if (ispec == 1001) { // ***** INTEL EXTENSION:  Indicate whether GETRF should improve accuracy by increasing precision of certain intermediate results.
                                 // In particular, before GETF2 calls SCAL to multiply by reciprocal (instead of doing a divide), we increase the precision of the
                                 // reciprocation:  single becomes double;  double becomes long double.  Return 1 if this is desired; return 0 if not.
                printf ("***************** ilaenv is returning directive that %s should increase precision of certain intermediate results.\n", name);
                return 1;
            }
        }

        if (strcmp (name, "zpotrf") == 0) {
            // When setting environment variables for POTRF, also consider the following:
            //   *  Any environment variable pertaining to GEMM
            if (ispec == 1) {  // Asking for "optimal" block size.
                printf ("***************** ilaenv is returning 48 for request from %s\n", name);
                return 48;
            }
        }

        if (strcmp (name, "ztrsm") == 0) {
            if (ispec == 1) { // ***** INTEL EXTENSION:  Asking for "optimal" block size, in terms of approximate number of flops
                printf ("***************** ilaenv is returning 20000 for request from %s\n", name);
                return 20000;
            }
        }

        printf ("Unknown request to ilaenv:  name = %s, opts = %s, ispec = %ld\n", name, opts, (u64) ispec);
        return -1;
    }
    break;
    default:
    {
    }
    break;
    } // switch
} // ilaenv


void xerbla (char errorSource[], BLAS_int_t errorDetail)
{
    printf ("**** Error in BLAS library function detected: %s;  Detail code = %ld\n", errorSource, (u64) errorDetail);
} // xerbla

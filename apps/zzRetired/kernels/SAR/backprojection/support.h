// May 27th 2011 Reservoir Labs Inc proprietary

#define JUNK_SIZE_X86 (1<<22)
long long int *junk;
int max_errors           = 10;
double epsilon           = 1e-5;
int max_trials           = 1;
int check_results        = 0;

//
//  Test whether a and b are equal (where epsilon is the error).
//  I.e.,
//
//  |a-b| <= epsilon |a|   or |a-b| <= epsilon |b|
//
int fequal(double x, double y) {
  double diff;
  int isnan_x, isnan_y;
  int isinf_x, isinf_y;

  isnan_x = isnan(x);
  isnan_y = isnan(y);
  if (isnan_x && isnan_y) return 1;
  if (isnan_x || isnan_y) return 0;

  isinf_x = isinf(x);
  isinf_y = isinf(y);

  if (isinf_x && isinf_y) {
    return (x > 0 && y > 0) || (x < 0 && y < 0);
  }
  if (isinf_x || isinf_y) return 0;

  diff = fabs(x-y);

  if ((x == 0.0 && y != 0.0) ||
      (x != 0.0 && y == 0.0)) {
    return diff <= epsilon;
  }

  if (x <= epsilon && -epsilon <= x &&
      y <= epsilon && -epsilon <= y) {
    return diff <= epsilon;
  }

  return diff <= epsilon * fabs(x) ||
    diff <= epsilon * fabs(y);
}


/*
   Checks for a submatrix embedded in a bigger matrix at (0,0).
   This version must be used in parametric codes when the initialization and
   the check are not performed on the full static array.
   This usually happens when we conservatively allocate a big chunk of memory
   but we do not want to initialize it all and compare it all for performance
   issues.
   ____________________
   |                    |
   |_____________       |
   |            |       |
   |            |       |
   |____________|_______|
*/
int check_matrices_portions(const char * name1, const char * name2,
			    const void * A, const void * B,
			    int m, int n, int Ndim) {
  int i, j;
  const real_t * X = (const real_t *)A;
  const real_t * Y = (const real_t *)B;
  int errors = 0;

  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      real_t x = X[i*Ndim+j];
      real_t y = Y[i*Ndim+j];
      if (! fequal(x, y)) {
	if (++errors >= max_errors) goto EXIT;
	printf("(%s[%d][%d]=%lg) != (%s[%d][%d]=%lg) (diff=%lg)\n",
	       name1, i, j, x,
	       name2, i, j, y,
	       x-y);
      }
    }
  }
EXIT:

  if (errors > 0) {
    printf("Number of errors=%d (max reported errors %d)\n",
	   errors, max_errors);
  }

  return errors > 0;
}

#include <getopt.h>

static void setParams(int opt) {
    int i;
    char c;

    int val = 0, index = 0, sign = 1;
    for (i=0; i<strlen(optarg); i++) {
	c = optarg[i];

	if (c < '0' || c > '9') {
	    if (val > 0) {
		if (index >= 16) {
		    printf("ERROR: Cannot specify more than 16 parameter values\n");
		    exit(1);
		}
		printf("Parameter %d Value: %d\n", index, val);
		PARAMS[index++] = val * sign;
	    } else if (val == 0 && c == '-') {
		sign = -1;
	    }
	    val = 0;
	    sign = 1;
	    continue;
	}

	val = val*10 + (c - '0');
    }

    if (val > 0) {
	if (index >= 16) {
	    printf("ERROR: Cannot specify more than 16 parameter values\n");
	    exit(1);
	}
	printf("Parameter %d Value: %d\n", index, val);
	PARAMS[index++] = val * sign;
    }

    for (i = index; i<16; i++) {
	PARAMS[i] = -1;
    }
}

// Parses command line arguments.
static void process_arguments(int argc, char** argv) {
    int opt;
    int error = 0;
    const char * prog = argv[0];

    while ((opt = getopt(argc, argv, "a:cft:T:vpP:e:E:")) >= 0) {
	switch (opt) {
	case 'a': setParams(opt);            break;
	case 'c': check_results = 1;            break;
	case 'T': max_trials    = atol(optarg); break;
	case 'e': epsilon = atof(optarg); break;
	case 'E': max_errors = atol(optarg); break;
	default:  error = 1;
	    fprintf(stderr, "unrecognized option '-%c'\n", optopt);
	    break;
	}
    }

    if (error) {
	printf(
	       "Usage: %s [-c][-T <trials>][-v]\n"
	       "\t-a add parameters and specify their values\n"
	       "\t-c enable results checking\n"
	       "\t-T <trials> specifiy the number of trials to run (default 1)\n"
	       "\t-v print extra messages during execution\n"
	       "\t-p show a portion of the results\n"
	       "\t-P <proc> declare that the program runs on proc processors\n"
	       "\t-e <float> use the given error bounds\n"
	       "\t-E <num>  limit to the given number of errors\n",
	       prog
	       );
	exit(1);
    }
}


void * coldify_caches() {
    int i, j;
    size_t size = JUNK_SIZE_X86;
    junk = (long long int *)malloc(size*sizeof(long long int));

    // We don't want to use OpenMP to initialize the memory on Tilera for
    // two reasons: (1) it is really slow, and (2) we only need to invalidate
    // the home cache in tile 0.
#pragma omp parallel for private(i)
    for (i=0; i < size; i+=8) {
	// try to have each core touch one or more disjoint cache lines
	for (j=i; j<=i+7; j++) {
	    junk[j] += j;
	}
    }

    return junk;
}

// Return the current time in seconds
static double get_time_in_seconds(void) {
    struct timeval tv;
    int _tod = gettimeofday(&tv, NULL);
    if (_tod != 0) abort();
    return (double) tv.tv_sec + tv.tv_usec * 1e-6;
}

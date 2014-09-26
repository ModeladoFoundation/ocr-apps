#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

static double* p;
static double* q;
static double* r;

void init_cg(uint32_t n)
{
    p = (double*)malloc(sizeof(double)*n);
    q = (double*)malloc(sizeof(double)*n);
    r = (double*)malloc(sizeof(double)*n);
}

void free_cg()
{
    free(p); free(q); free(r);
}

double conj_grad(uint32_t* colidx, uint64_t* rowstr, double* x, double* z, double* a, uint32_t n)
{
    double rho = 0;
    bzero(q, sizeof(double)*n); bzero(z, sizeof(double)*n);
    int i;
    #pragma omp parallel for private(i),reduction(+:rho)
    for(i=0; i<n; ++i) {
        p[i] = r[i] = x[i];
        rho += r[i]*r[i];
    }

    uint32_t it;
    for(it=0; it<25; ++it) {

        #pragma omp parallel for private(i)
        for(i=0; i<n; ++i) {
            q[i] = 0;
            int j;
            for(j=rowstr[i]; j<rowstr[i+1]; ++j)
                q[i] += a[j]*p[colidx[j]];
        }

        double alpha = 0;
        #pragma omp parallel for private(i),reduction(+:alpha)
        for(i=0; i<n; ++i)
            alpha += p[i]*q[i];
        alpha = rho/alpha;

        double rho0 = rho;

        #pragma omp for private(i)
        for(i=0; i<n; ++i) {
           z[i] = z[i] + alpha*p[i];
           r[i] = r[i] - alpha*q[i];
        }

        rho = 0;
        #pragma omp parallel for private(i),reduction(+:rho)
        for(i=0; i<n; ++i)
            rho += r[i]*r[i];

        double beta = rho/rho0;
        #pragma omp parallel for private(i)
        for(i=0; i<n; ++i)
           p[i] = r[i] + beta*p[i];
    }

    #pragma omp parallel for private(i)
    for(i=0; i<n; ++i) {
       r[i] = 0;
       uint64_t j;
       for(j=rowstr[i]; j<rowstr[i+1]; ++j)
          r[i] += a[j]*z[colidx[j]];
    }

    double sum = 0;
    #pragma omp parallel for private(i),reduction(+:sum)
    for(i=0; i<n; ++i) {
         double d = x[i]-r[i];
         sum += d*d;
    }

    return sqrt(sum);
}

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "cg.h"

int main(int argc, char* argv[])
{
    class_t class;
    if(argc>1) {
        if(argc>3) {
            printf("cg [-t] [S,W,A,B,C,D,E] (default: S -t)\n");
            return 0;
        }
        int on = 0;
        while(--argc) {
            if(strcmp(argv[argc], "-t")==0)
                on = 1;
            else
                select_class(&class, *argv[argc]);
        }
        if(class.c == 'U') {
            printf("cg [-t] [S,W,A,B,C,D,E] (default: S -t)\n");
            return 0;
        }
        class.on = on;
    }
    else
        select_class(&class, 'S');

    init_timer(&class, class.on);
    printf("CG Benchmark: size=%d, iterations=%d\n", class.na, class.niter);

    timer_start();

    uint64_t* rowstr = (uint64_t*) malloc(sizeof(uint64_t)*(class.na+1));
    double* x = (double*) malloc(sizeof(double)*class.na);
    double* z = (double*) malloc(sizeof(double)*class.na);
    double* a;
    uint32_t* colidx;

    if(makea(&class, &a, &colidx, rowstr) == -1) {
        free_timer();
        free(x); free(z); free(rowstr);
        return 0;
    }
    uint32_t i;
    for(i=0; i<=class.na; ++i)
        x[i] = 1;

    init_cg(class.na);
    conj_grad(colidx, rowstr, x, z, a, class.na);
    double norm_temp1=0;
    double norm_temp2=0;
    uint32_t j;
    for(j=0; j<class.na; ++j) {
        norm_temp1 += x[j]*z[j];
        norm_temp2 += z[j]*z[j];
    }
    norm_temp2 = 1.0/sqrt(norm_temp2);
    for(j=0; j<class.na; ++j)
        x[j] = norm_temp2*z[j];

    for(i=0; i<=class.na; ++i)
        x[i] = 1;

    timer_stop(0);

    printf("Iteration               ||r||               zeta\n");

    timer_start();

    double zeta;
    uint32_t it;
    for(it = 1; it<=class.niter; ++it) {
        if(class.on) timer_start(it+1);
        double rnorm = conj_grad(colidx, rowstr, x, z, a, class.na);
        if(class.on) timer_stop(it+1);

        norm_temp1 = 0;
        norm_temp2 = 0;
        for(j=0; j<class.na; ++j) {
            norm_temp1 += x[j]*z[j];
            norm_temp2 += z[j]*z[j];
        }
        norm_temp2 = 1.0/sqrt(norm_temp2);
        for(j=0; j<class.na; ++j)
            x[j] = norm_temp2*z[j];
        zeta = class.shift+1/norm_temp1;
        printf("%9d, %20.13e %10.13f\n", it, rnorm, zeta);
    }

    timer_stop(1);
    printf("Benchmark completed\n");

    double err = abs(zeta-class.zvv)/class.zvv;
    if(err<=1e-10) {
        double mflops;
        double t_bench = timer_read(1);
        if(t_bench)
            mflops = 2*class.niter*class.na*
                     (((double)3+class.nonzer*(class.nonzer+1))+((double)25*(5+class.nonzer*(class.nonzer+1))+3))/
                     t_bench/1000000;
        printf("Verification SUCCESSFUL (zeta=%.13f, error=%.13f\n", zeta, err);
        print_results(&class, t_bench, mflops);

        if(class.on) {
            printf("Timing (sec,%%)\n");
            double t_cg = 0;
            for(it = 1; it<=class.niter; ++it)
                t_cg += timer_read(it+1);
            double t_init = timer_read(0);
            double wall = t_init+timer_read(1);
            printf("init:      %10.3f (%3.1f%%)\n", t_init, t_init/wall*100);
            printf("cg:        %10.3f (%3.1f%%)\n", t_cg, t_cg/wall*100);
            printf("norm:      %10.3f (%3.1f%%)\n", (t_bench-t_cg), (t_bench-t_cg)/wall*100);
            printf("wall time: %10.3f\n", wall);
        }
    }
    else
        printf("Verification FAILED (zeta=%.13f, correct zeta=%.13f\n", zeta, class.zvv);

    free(a); free(x); free(z);
    free(colidx); free(rowstr);
    free_timer();
    free_cg();

    return 0;
}

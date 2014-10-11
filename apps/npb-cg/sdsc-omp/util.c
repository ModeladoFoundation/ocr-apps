#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <inttypes.h>

#include "cg.h"

static unsigned char tp;
static double* t;
static struct timeval tv;

double time()
{
  gettimeofday(&tv, 0);
  return tv.tv_sec+1.0e-6*tv.tv_usec;
}

void init_timer(class_t* class, unsigned char on)
{
    tp = -1;
    t = (double*)malloc(sizeof(double)*(2+(on ? class->niter:0)));
}

void timer_start()
{
    t[++tp] = time();
}

void timer_stop(unsigned char tr)
{
    t[tr]=time()-t[tr];
}

double timer_read(unsigned char tr)
{
    return t[tr];
}

void free_timer()
{
    free(t);
}

void print_results(class_t* class, double t, double mops)
{
    printf("CG Benchmark Completed\n");
    printf("Class           = %12c\n", class->c);
    printf("Size            = %12d\n", class->na);
    printf("Iterations      = %12d\n", class->niter);
    printf("Time in seconds = %12.2f\n", t);
    printf("MFLOPS total    = %12.2f\n", mops);
}

void select_class(class_t* c, char class)
{
    c->c=class;
    c->on=1;
    switch(class) {
        case class_S:
          c->na=1400,
          c->nonzer=7,
          c->shift=10.,
          c->niter=15,
          c->zvv=8.5971775078648;
          break;
        case class_W:
          c->na=7000,
          c->nonzer=8,
          c->shift=12.,
          c->niter=15,
          c->zvv=10.362595087124;
          break;
        case class_A:
          c->na=14000,
          c->nonzer=11,
          c->shift=20.,
          c->niter=15,
          c->zvv=17.130235054029;
          break;
        case class_B:
          c->na=75000,
          c->nonzer=13,
          c->shift=60.,
          c->niter=75,
          c->zvv=22.712745482631;
          break;
        case class_C:
          c->na=150000,
          c->nonzer=15,
          c->shift=110.,
          c->niter=75,
          c->zvv=28.973605592845;
          break;
        case class_D:
          c->na=1500000,
          c->nonzer=21,
          c->shift=500.,
          c->niter=100,
          c->zvv=52.514532105794;
          break;
        case class_E:
          c->na=9000000,
          c->nonzer=26,
          c->shift=1500.,
          c->niter=100,
          c->zvv=77.522164599383;
          break;
        default:
          c->c = class_U;
    }
}

static double tran  = 314159265.0;
static uint64_t amult = 1220703125;
static const double d2m46 = 1.421085471520200e-014;
static const uint64_t i246m1 = 0x00003FFFFFFFFFFF;

double randd()
{
    uint64_t x = (uint64_t)tran;
    x = (x*amult)&i246m1;
    tran = (double)x;
    return d2m46*tran;
}

int randi(int i)
{
    return i*randd();
}

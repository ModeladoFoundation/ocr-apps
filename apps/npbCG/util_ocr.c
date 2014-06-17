#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <inttypes.h>
#include <ocr.h>

#include "cg_ocr.h"

static inline
double time()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec+1.0e-6*tv.tv_usec;
}

void timer_init(timerdb_t** timerdb, ocrGuid_t* guid, classdb_t* class, u8 on)
{
    ocrDbCreate(guid, (void**)timerdb, sizeof(timerdb_t)+sizeof(double)*(2+(on ? class->niter:0)),
                0, NULL_GUID, NO_ALLOC);
    (*timerdb)->tp = -1;
}

void timer_start(timerdb_t* timerdb)
{
    timerdb->t[++timerdb->tp] = time();
}

void timer_stop(timerdb_t* timerdb, u8 tr)
{
    timerdb->t[tr]=time()-timerdb->t[tr];
}

double timer_read(timerdb_t* timerdb, u8 tr)
{
    return timerdb->t[tr];
}

void print_results(classdb_t* class, double t, double mops)
{
    printf("CG Benchmark Completed\n");
    printf("Class           = %12c\n", class->c);
    printf("Size            = %12d\n", class->na);
    printf("Iterations      = %12d\n", class->niter);
    printf("Time in seconds = %12.2f\n", t);
    printf("MFLOPS total    = %12.2f\n", mops);
}

void class_init(classdb_t** class, ocrGuid_t* guid, char c)
{
    ocrDbCreate(guid, (void**)class, sizeof(classdb_t), 0, NULL_GUID, NO_ALLOC);
    (*class)->c=c;
    (*class)->on=1;
    switch(c) {
        case class_S:
          (*class)->na=1400,
          (*class)->nonzer=7,
          (*class)->shift=10.,
          (*class)->niter=15,
          (*class)->zvv=8.5971775078648;
          break;
        case class_W:
          (*class)->na=7000,
          (*class)->nonzer=8,
          (*class)->shift=12.,
          (*class)->niter=15,
          (*class)->zvv=10.362595087124;
          break;
        case class_A:
          (*class)->na=14000,
          (*class)->nonzer=11,
          (*class)->shift=20.,
          (*class)->niter=15,
          (*class)->zvv=17.130235054029;
          break;
        case class_B:
          (*class)->na=75000,
          (*class)->nonzer=13,
          (*class)->shift=60.,
          (*class)->niter=75,
          (*class)->zvv=22.712745482631;
          break;
        case class_C:
          (*class)->na=150000,
          (*class)->nonzer=15,
          (*class)->shift=110.,
          (*class)->niter=75,
          (*class)->zvv=28.973605592845;
          break;
        case class_D:
          (*class)->na=1500000,
          (*class)->nonzer=21,
          (*class)->shift=500.,
          (*class)->niter=100,
          (*class)->zvv=52.514532105794;
          break;
        case class_E:
          (*class)->na=9000000,
          (*class)->nonzer=26,
          (*class)->shift=1500.,
          (*class)->niter=100,
          (*class)->zvv=77.522164599383;
          break;
        default:
          (*class)->c = class_U;
    }
}

static const double d2m46 = 1.421085471520200e-014;
static const uint64_t i246m1 = 0x00003FFFFFFFFFFF;

void rand_init(randdb_t** randdb, ocrGuid_t* guid)
{
    ocrDbCreate(guid, (void**)randdb, sizeof(randdb_t), 0, NULL_GUID, NO_ALLOC);
    (*randdb)->tran = 314159265.0;
    (*randdb)->amult = 1220703125;
}

double randd(randdb_t* randdb)
{
    u64 x = (u64)randdb->tran;
    x = (x*randdb->amult)&i246m1;
    randdb->tran = (double)x;
    return d2m46*randdb->tran;
}

int randi(randdb_t* randdb, int i)
{
    return i*randd(randdb);
}

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#define MAXRANK 128
#define MAXITER 10
#define PHASES 54

char pl[PHASES][10];

main(int argc, char ** argv){
if(argc != 3) {
  printf("input must be exactly nrank iters\n");
  exit(0);
}
strcpy(pl[0], "pAp");
strcpy(pl[1], "startMG0");
strcpy(pl[2], "MG00");
strcpy(pl[3], "HE");
strcpy(pl[4], "UN");
strcpy(pl[5], "smooth0");
strcpy(pl[6], "MG01");
strcpy(pl[7], "HE");
strcpy(pl[8], "UN");
strcpy(pl[9], "spmv0");
strcpy(pl[10], "MG02");
strcpy(pl[11], "MG10");
strcpy(pl[12], "HE");
strcpy(pl[13], "UN");
strcpy(pl[14], "smooth1");
strcpy(pl[15], "MG11");
strcpy(pl[16], "HE");
strcpy(pl[17], "UN");
strcpy(pl[18], "spmv1");
strcpy(pl[19], "MG12");
strcpy(pl[20], "MG20");
strcpy(pl[21], "HE");
strcpy(pl[22], "UN");
strcpy(pl[23], "smooth2");
strcpy(pl[24], "MG21");
strcpy(pl[25], "HE");
strcpy(pl[26], "UN");
strcpy(pl[27], "spmv2");
strcpy(pl[28], "MG22");
strcpy(pl[29], "MG30");
strcpy(pl[30], "HE");
strcpy(pl[31], "UN");
strcpy(pl[32], "smooth3");
strcpy(pl[33], "MG31");
strcpy(pl[34], "MG23");
strcpy(pl[35], "HE");
strcpy(pl[36], "UN");
strcpy(pl[37], "smooth2");
strcpy(pl[38], "MG24");
strcpy(pl[39], "MG13");
strcpy(pl[40], "HE");
strcpy(pl[41], "UN");
strcpy(pl[42], "smooth1");
strcpy(pl[43], "MG14");
strcpy(pl[44], "MG03");
strcpy(pl[45], "HE");
strcpy(pl[46], "UN");
strcpy(pl[47], "smooth0");
strcpy(pl[48], "MG04");
strcpy(pl[49], "rtz");
strcpy(pl[50], "spmv");
strcpy(pl[51], "HE");
strcpy(pl[52], "UN");
strcpy(pl[53], "spmv");

int iters, nrank;
nrank = atoi(argv[1]);
iters = atoi(argv[2]);
long long start[PHASES][MAXITER][MAXRANK], starttime;
long long end[PHASES][MAXITER][MAXRANK], endtime;
double time;
int i = 0, rank, iter, phase;

for(i=0;i<PHASES*nrank*iters;i++) {
scanf("%d %d %d %ld %ld", &rank, &iter, &phase, &starttime, &endtime);
printf("i %d rank %d iter %d phase %d\n", i, rank, iter, phase);
start[phase][iter][rank] = starttime;
end[phase][iter][rank] = endtime;
}


/*
for(rank=0;rank<nrank;rank++)
for(iter=0;iter<iters;iter++)
for(phase=0;phase<PHASES;phase++)
printf("rank %d iter %d phase %d starttime %ld endtime %ld\n", rank, iter, phase, start[phase][iter][rank], end[phase][iter][rank]);
*/

double avg, max, wavg, wmax, wtime;
int maxrank, wphase, wmaxrank;
int tempiter;

printf("nrank = %d iters %d \n", nrank, iters);

for(iter=0;iter<iters;iter++)
for(phase=0;phase<PHASES;phase++) {
    avg = 0;
    max = 0;
    maxrank = 0;
    wavg = 1e-8;
    wmax = 0;
    wmaxrank=0;
    for(rank=0;rank<nrank;rank++) {
        time = end[phase][iter][rank] - start[phase][iter][rank];
        avg += time;
        if(max < time) {
            max = time;
            maxrank = rank;
        }
        if(iter != 0 || phase != 0) {
            wphase = phase;
            tempiter = iter;
            if(phase == 0) {
                wphase=PHASES;
                tempiter--;
//                printf("start %ld %ld \n", start[phase][iter][rank], end[wphase-1][tempiter][rank]);
            }
            wtime = start[phase][iter][rank] - end[wphase-1][tempiter][rank];
            wavg += wtime;
            if(wmax < wtime) {
                wmaxrank = rank;
                wmax = wtime;
            }

        }
        else {wmax=1; wavg = 1;}

    }
    avg /= nrank;
    wavg /= nrank;
    printf("iter %2d phase %2d wmax %12f wavg %12f wratio %12f wmaxrank %3d max %12f avg %12f ratio %12f maxrank %3d %s\n", iter, phase, wmax, wavg, wmax/wavg, wmaxrank,  max, avg, max/avg, maxrank, pl[phase]);


    }
}

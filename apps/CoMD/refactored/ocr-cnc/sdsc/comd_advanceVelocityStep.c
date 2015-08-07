#include "comd.h"

void comd_advanceVelocityStep (cncTag_t i, cncTag_t iter, BItem b1, comdCtx *ctx) {

//    if (i==0)
//    printf("CnC: advanceVelocityStep %lu, %d, %lu\n",i,  b1->i, iter);

    struct box *b = b1;


    real_t sump = 0.0, sumr = 0.0;

    // advance velocity
    for (int iOff = 0; iOff < b->nAtoms; iOff++) {
       b->atoms.p[iOff][0] += b->dt*b->atoms.f[iOff][0];
       b->atoms.p[iOff][1] += b->dt*b->atoms.f[iOff][1];
       b->atoms.p[iOff][2] += b->dt*b->atoms.f[iOff][2];

       // advance position
       int iSpecies = b->atoms.iSpecies[iOff];
       real_t invMass = 1.0/b->species[iSpecies].mass; ////////////////////////
       b->atoms.r[iOff][0] += b->dt*b->atoms.p[iOff][0]*invMass;
       b->atoms.r[iOff][1] += b->dt*b->atoms.p[iOff][1]*invMass;
       b->atoms.r[iOff][2] += b->dt*b->atoms.p[iOff][2]*invMass;

       sump += b->atoms.p[iOff][0] + b->atoms.p[iOff][1] + b->atoms.p[iOff][2];
       sumr += b->atoms.r[iOff][0] + b->atoms.r[iOff][1] + b->atoms.r[iOff][2];
    }


 //   if (i==0)
 //       printf("sump = %lf, sumr = %lf\n", sump, sumr);

    cncPut_B(b1, i, 1, 0, iter, ctx);

    if (i == 0)
        cncPrescribe_updateBoxStep(0, 0, iter, ctx);

}

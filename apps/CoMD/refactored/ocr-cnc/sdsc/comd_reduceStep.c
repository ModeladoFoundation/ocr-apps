#include "comd.h"


void comd_reduceStep (cncTag_t i, cncTag_t iter, BItem b, redcItem rd, ITItem it, TBoxesItem tb, comdCtx *ctx) {

//    if(i==0)
//        printf("reducesStep %d, %d, %d\n", i, iter, it );
    struct myReduction *r = rd;
    struct box *b1 = b;

    r->ePot = rd->ePot + b->ePot;
    r->eKin = rd->eKin + b->eKin;
    r->nAtoms = rd->nAtoms + b->nAtoms;


    if ((i < tb -1) && ( iter < it-1)) {
        //cncPut_redc(rd, i+1, iter, ctx);
        cncPrescribe_reduceStep(i+1, iter, ctx);
        cncPut_B(b, i, 0, 0, iter+1, ctx);
    }
    if ((i == tb -1) && (iter < (it -1))) {
        cncPut_B(b, i, 0, 0, iter+1, ctx);
        if (!(iter % ctx->printRate)) {
            real_t t,p,k;
//            p = r->ePot/32000;
//            k = r->eKin/32000;
            p = r->ePot/b1->atoms.nGlobal;
            k = r->eKin/b1->atoms.nGlobal;
            t = p+k;
            printf("%4lu  %18.12f %18.12f %18.12f %6d\n",iter, t,p,k, r->nAtoms);
        }
        r->ePot = 0.0;
        r->eKin = 0.0;
        r->nAtoms = 0;
        //cncPut_redc(rd, 0, iter+1, ctx);
        cncPrescribe_reduceStep(0, iter+1, ctx);
        for (int ii = 0; ii <  tb; ii++) {
            b1->i = ii;
            b1->ePot = 0;// b->ePot;
            b1->eKin = 0;//b->eKin;
  //          cncPrescribe_reduceStep(ii, iter+1, ctx);
            cncPrescribe_advanceVelocityStep(ii, iter+1, ctx);
//            printf("CnC: In reduceStep-- cncPrescribe_advanceVelocityStep: %d, %lu\n", ii, iter+1);
        }
    }
    if (iter == it -1) {
        if (i == tb -1) {
            real_t t,p,k;
//            p = r->ePot/32000;
//            k = r->eKin/32000;
            p = r->ePot/b1->atoms.nGlobal;;
            k = r->eKin/b1->atoms.nGlobal;;
            t = p+k;
            printf("%4lu  %18.12f %18.12f %18.12f %6d\n",iter, t,p,k, r->nAtoms);
        } else {
            cncPrescribe_reduceStep(i+1, iter, ctx);
        }
        b1->ePot = 0;// b->ePot;
        b1->eKin = 0;//b->eKin;
        cncPut_redc(rd, i+1, iter, ctx);
        cncPut_B(b, i, 5, 0, iter, ctx);
 //       printf("%d,%d\n", i,iter);
    }

}

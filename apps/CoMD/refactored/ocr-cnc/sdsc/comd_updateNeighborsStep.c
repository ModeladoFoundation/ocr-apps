#include "comd.h"

void comd_updateNeighborsStep (cncTag_t i, cncTag_t j, cncTag_t iter, AtomInfoItem ai1, BItem b1, TBoxesItem totalBoxes, comdCtx *ctx) {

    struct atomInfo *ai = ai1;
    struct box *b = b1;

//    if (i==0)
//    printf("updateNeighborsStep %d, %d, %d\n", i, j, iter);


    // move the atoms from "atomInfo" into "b"
    for (int kk = 0; kk < ai->n; kk++) {
        if (ai->nbrs[kk][0] == j) {
            int nj = b->nAtoms;
           // copyAtom(boxes, atoms, iId, iBox, nj, jBox);
            b->atoms.gid[nj] = ai->gid[kk];
            b->atoms.iSpecies[nj] = ai->iSpecies[kk];
            b->atoms.r[nj][0] = ai->r[kk][0]; b->atoms.r[nj][1] = ai->r[kk][1]; b->atoms.r[nj][2] = ai->r[kk][2];
            b->atoms.p[nj][0] = ai->p[kk][0]; b->atoms.p[nj][1] = ai->p[kk][1]; b->atoms.p[nj][2] = ai->p[kk][2];
            b->atoms.f[nj][0] = ai->f[kk][0]; b->atoms.f[nj][1] = ai->f[kk][1]; b->atoms.f[nj][2] = ai->    f[kk][2];
            b->atoms.U[nj] = ai->U[kk];
            b->nAtoms++;
        }
    }


    // pick the next neighbor
    int next = -1;
    for (int kk = 0; kk < ai->n; kk++) {
        if (ai->nbrs[kk][1] == -1) {
            next = ai->nbrs[kk][0];
            break;
        }
    }

    // mark all entries corresponding to this neighbor with "1"
    for (int kk = 0; kk < ai->n; kk++) {
        if (ai->nbrs[kk][0] == next) {
            ai->nbrs[kk][1] = 1;
        }
    }

    if (next == -1) { // this is the last box to be updated
        cncItemFree(ai1);
//       if (i < 1727)
//        if (i < 342)
        if (i < totalBoxes-1)
            cncPrescribe_updateBoxStep(i+1, 0, iter, ctx);
    } else {
        cncPrescribe_updateNeighborsStep(i, next, iter, ctx);
        cncPut_AtomInfo(ai1, i, next, iter, ctx);
    }

/*
     // To be used later
     if (0) {
         int r = rand() % 1728;
         cncPrescribe_updateNeighborsStep(i, r, iter, ctx);
         cncPut_AtomInfo(ai1, i, r, iter, ctx);
     }

     // to be populated later -- if last neighbor
     if (1) {
         cncItemFree(ai1);  /////////////// should be based on some condition
         if (i < 1727)
         cncPrescribe_updateBoxStep(i+1, 0, iter, ctx);
     }
*/
}


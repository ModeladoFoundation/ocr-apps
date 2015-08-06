#include "comd.h"

/*
 * typeof GID is int *
 * typeof ISP is int *
 * typeof R is real3 *
 * typeof P is real3 *
 * typeof F is real3 *
 * typeof U is real_t *
 */
void comd_cleanupInitStep(cncTag_t i, int *GID, int *ISP, real3 *R, real3 *P, real3 *F, real_t *U, comdCtx *ctx) {

   // destroy the items passed as they are not used again
   printf("Cleaning up initialized data\n");

   cncItemFree(GID);
   cncItemFree(ISP);
   cncItemFree(R);
   cncItemFree(P);
   cncItemFree(F);
   cncItemFree(U);

}

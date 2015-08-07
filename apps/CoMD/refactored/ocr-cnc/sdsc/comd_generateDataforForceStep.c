#include "comd.h"

#include "force.h"

void comd_generateDataforForceStep(cncTag_t i, cncTag_t iter, struct box *B, int totalBoxes, comdCtx *ctx) {

//   printf("CnC: Inside generateDataforForceStep %d\n", i);


    // sort the box -- required as updates are not ordered
//    sortAtomsInCell1(b);

 //   cncPut_B(B0, i, 3, 0, iter, ctx);

//    if ( i != 1727)
//    if (i != 342)
    if ( i != totalBoxes -1)
        cncPrescribe_generateDataforForceStep(i+1, iter, ctx);
//    if (i == 1727) {
//    if (i == 342) {
    if (i == totalBoxes -1) {
        cncPrescribe_generateForceTagsStep(iter, ctx);
    }
}



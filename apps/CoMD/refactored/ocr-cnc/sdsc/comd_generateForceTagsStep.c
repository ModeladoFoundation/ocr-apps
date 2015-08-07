#include "comd.h"


void comd_generateForceTagsStep (cncTag_t iter, TBoxesItem tb, comdCtx *ctx) {
//    printf("CnC: generateForceTags %d\n", iter);

    int i;
    for (i=0;i<tb;i++) {
//        printf("CnC: %d\n", i);
        cncPrescribe_forceStep(i, iter, ctx);
    }
}

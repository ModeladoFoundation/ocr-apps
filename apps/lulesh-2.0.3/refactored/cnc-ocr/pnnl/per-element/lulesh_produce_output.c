#include "lulesh.h"

/**
 * Step function defintion for "produce_output"
 */
void produce_output(cncTag_t iteration, double final_energy, luleshCtx *ctx) {

    //
    // OUTPUTS
    //

    // Put "final_origin_energy" items
    double *final_origin_energy = cncCreateItem_final_origin_energy();
    *final_origin_energy = final_energy;
    cncPut_final_origin_energy(final_origin_energy, ctx);


}

#include "lulesh.h"

/*
 * typeof energy is double
 */
void produce_output(cncTag_t iteration, energyItem final_energy, luleshCtx *ctx) {

    cncPut_final_origin_energy(final_energy.handle, ctx);

}

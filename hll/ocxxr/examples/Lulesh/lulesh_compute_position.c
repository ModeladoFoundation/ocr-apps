#include "lulesh.h"

/**
 * Step function defintion for "compute_position"
 */
void lulesh_compute_position(cncTag_t iteration, cncTag_t node_id, double delta_time, vector velocity, vertex previous_position, luleshCtx *ctx) {


	vertex position = move(previous_position, mult(velocity, delta_time));

    //
    // OUTPUTS
    //

    // Put "position_out" items
    vertex *position_out = cncItemAlloc(sizeof(*position_out));
	*position_out = position;
    cncPut_position(position_out, iteration, node_id, ctx);


}

#include "lulesh.h"

/**
 * Step function defintion for "reduce_force"
 */
void lulesh_reduce_force(cncTag_t iteration, cncTag_t node_id, vector *neighbor_stress_partial, vector *neighbor_hourglass_partial, luleshCtx *ctx) {

    //
    // INPUTS
    //

	vector force = vector_new(0.0, 0.0, 0.0);
    { // Access "neighbor_stress_partial" inputs
        s64 _local_element_id;
        for (_local_element_id = 0; _local_element_id < 8; _local_element_id++) {
            /* TODO: Do something with neighbor_stress_partial[_local_element_id] */
			vector stress_partial = neighbor_stress_partial[_local_element_id];
			vector hourglass_partial = neighbor_hourglass_partial[_local_element_id];
			force.x += stress_partial.x + hourglass_partial.x;
			force.y += stress_partial.y + hourglass_partial.y;
			force.z += stress_partial.z + hourglass_partial.z;
        }
    }


    //
    // OUTPUTS
    //

    // Put "force_out" items
    vector *force_out = cncItemAlloc(sizeof(*force_out));
	*force_out = force;
    cncPut_force(force_out, iteration, node_id, ctx);


}

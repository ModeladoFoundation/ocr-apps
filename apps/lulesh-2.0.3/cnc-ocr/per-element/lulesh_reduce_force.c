#include "lulesh.h"

/*
 * typeof neighbor_stress_partial is vector
 * typeof neighbor_hourglass_partial is vector
 */
void reduce_force(cncTag_t iteration, cncTag_t node_id, neighbor_stress_partialItem *neighbor_stress_partial, neighbor_hourglass_partialItem *neighbor_hourglass_partial, luleshCtx *ctx) {

	//
	// INPUTS
	//
	vector force = vector_new(0.0, 0.0, 0.0);

	{ // Access "neighbor_stress_partial", "neighbor_hourglass_partial" inputs
		s64 _local_element_id;
		for (_local_element_id = 0; _local_element_id < ((8)-(0)); _local_element_id++) {
			vector stress_partial = neighbor_stress_partial[_local_element_id].item;
			vector hourglass_partial = neighbor_hourglass_partial[_local_element_id].item;
			force.x += stress_partial.x + hourglass_partial.x;
			force.y += stress_partial.y + hourglass_partial.y;
			force.z += stress_partial.z + hourglass_partial.z;
		}
	}

	//
	// OUTPUTS
	//
	// Put "force_out" items
	vector *force_out;
	cncHandle_t force_outHandle = cncCreateItem_force(&force_out);
	*force_out = force;
	cncPut_force(force_outHandle, iteration, node_id, ctx);
}

#include "lulesh.h"

/**
 * Step function defintion for "compute_volume"
 */
void compute_volume(cncTag_t iteration, cncTag_t element_id, vertex *neighbor_position, luleshCtx *ctx) {

    //
    // INPUTS
    //
	vertex node_vertices[8];
    { // Access "neighbor_position" inputs
        s64 _local_node_id;
        for (_local_node_id = 0; _local_node_id < 8; _local_node_id++) {
			node_vertices[_local_node_id] = neighbor_position[_local_node_id];
        }
    }
    // Constants
   	double initial_volume = ctx->domain.element_volume[element_id];

   	// Compute
   	double volume = (
   			dot(cross( vertex_sub(node_vertices[6], node_vertices[3]),
   								 vertex_sub(node_vertices[2], node_vertices[0])),
   			vector_add(vertex_sub(node_vertices[3], node_vertices[1]),
   								 vertex_sub(node_vertices[7], node_vertices[2]))) +
   			dot(cross( vertex_sub(node_vertices[6], node_vertices[4]),
   								 vertex_sub(node_vertices[7], node_vertices[0])),
   			vector_add(vertex_sub(node_vertices[4], node_vertices[3]),
   								 vertex_sub(node_vertices[5], node_vertices[7]))) +
   			dot(cross( vertex_sub(node_vertices[6], node_vertices[1]),
   								 vertex_sub(node_vertices[5], node_vertices[0])),
   			vector_add(vertex_sub(node_vertices[1], node_vertices[4]),
   								 vertex_sub(node_vertices[2], node_vertices[5])))) * (1.0/12.0);

   	double relative_volume = volume / initial_volume;

   	volume = fabs(relative_volume - 1.0) < ctx->cutoffs.v ? 1.0 : relative_volume;

   	// Check for negative volumes
    if (volume <= 0.0) {
    	exit(1);
    }

    //
    // OUTPUTS
    //

    // Put "volume_out" items
    double *volume_out = cncCreateItem_volume();
	*volume_out = volume;
    cncPut_volume(volume_out, iteration, element_id, ctx);


}

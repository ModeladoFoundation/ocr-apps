#include "lulesh.hpp"

/**
 * Step function defintion for "compute_velocity"
 */
extern "C"
void lulesh_compute_velocity(cncTag_t iteration, cncTag_t node_id, double delta_time, vector force, vector previous_velocity, luleshCtx *ctx0) {
    LuleshCtx *ctx = static_cast<LuleshCtx*>(ctx0);

	vector acceleration = divide(force, ctx->ii.domain.node_mass[node_id]);

	if(ctx->ii.mesh.nodes_node_neighbors[node_id][0] == -2) acceleration.x = 0;
	if(ctx->ii.mesh.nodes_node_neighbors[node_id][2] == -2) acceleration.y = 0;
	if(ctx->ii.mesh.nodes_node_neighbors[node_id][4] == -2) acceleration.z = 0;

	vector velocity = vector_add(previous_velocity,	mult(acceleration, delta_time));
	if( fabs(velocity.x) < ctx->cutoffs.u ) velocity.x = 0.0;
	if( fabs(velocity.y) < ctx->cutoffs.u ) velocity.y = 0.0;
	if( fabs(velocity.z) < ctx->cutoffs.u ) velocity.z = 0.0;


    //
    // OUTPUTS
    //

    // Put "velocity_out" items
    vector *velocity_out = (decltype(velocity_out))cncItemAlloc(sizeof(*velocity_out));
	*velocity_out = velocity;
    cncPut_velocity(velocity_out, iteration, node_id, ctx);


}

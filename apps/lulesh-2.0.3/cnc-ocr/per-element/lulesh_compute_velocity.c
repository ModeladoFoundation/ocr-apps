#include "lulesh.h"

/*
 * typeof delta_time is double
 * typeof force is vector
 * typeof velocity is vector
 */
void compute_velocity(cncTag_t iteration, cncTag_t node_id, delta_timeItem delta_time_in, forceItem force_in, velocityItem previous_velocity_in, luleshCtx *ctx) {

	double delta_time = delta_time_in.item;
	vector force = force_in.item;
	vector previous_velocity = previous_velocity_in.item;

	vector acceleration = divide(force, ctx->domain.node_mass[node_id]);

	if(ctx->mesh.nodes_node_neighbors[node_id][0] == -2) acceleration.x = 0;
	if(ctx->mesh.nodes_node_neighbors[node_id][2] == -2) acceleration.y = 0;
	if(ctx->mesh.nodes_node_neighbors[node_id][4] == -2) acceleration.z = 0;

	vector velocity = vector_add(previous_velocity,	mult(acceleration, delta_time));
	if( fabs(velocity.x) < ctx->cutoffs.u ) velocity.x = 0.0;
	if( fabs(velocity.y) < ctx->cutoffs.u ) velocity.y = 0.0;
	if( fabs(velocity.z) < ctx->cutoffs.u ) velocity.z = 0.0;

	//
	// OUTPUTS
	//

	// Put "velocity_out" items
	vector *velocity_out;
	cncHandle_t velocity_outHandle = cncCreateItem_velocity(&velocity_out);
	*velocity_out = velocity;
	cncPut_velocity(velocity_outHandle, iteration, node_id, ctx);
}

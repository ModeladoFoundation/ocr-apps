#include "lulesh.h"

/*
 * typeof delta_time is double
 * typeof velocity is vector
 * typeof position is vertex
 */
void compute_position(cncTag_t iteration, cncTag_t node_id, delta_timeItem delta_time_in, velocityItem velocity_in, positionItem previous_position_in, luleshCtx *ctx) {

	double delta_time = delta_time_in.item;
	vector velocity = velocity_in.item;
	vertex previous_position = previous_position_in.item;

	//
	// OUTPUTS
	//
	vertex position = move(previous_position, mult(velocity, delta_time));

	// Put "position_out" items
	vertex *position_out;
	cncHandle_t position_outHandle = cncCreateItem_position(&position_out);
	*position_out = position;
	cncPut_position(position_outHandle, iteration, node_id, ctx);

}

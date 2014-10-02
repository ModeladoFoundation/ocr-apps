#include "lulesh.h"

void lulesh_init(luleshArgs *args, luleshCtx *ctx) {

	// Put "stress_partial" items
	vector *stress_partial;
	cncHandle_t stress_partialHandle = cncCreateItem_stress_partial(&stress_partial);
	*stress_partial = vector_new(0.0, 0.0, 0.0);
	cncPut_stress_partial(stress_partialHandle, -1, -1, ctx);

	// Put "hourglass_partial" items
	vector *hourglass_partial;
	cncHandle_t hourglass_partialHandle = cncCreateItem_hourglass_partial(&hourglass_partial);
	*hourglass_partial = vector_new(0.0, 0.0, 0.0);
	cncPut_hourglass_partial(hourglass_partialHandle, -1, -1, ctx);

	// Put "velocity_gradient" items
	vector *velocity_gradient;
	cncHandle_t velocity_gradientHandle = cncCreateItem_velocity_gradient(&velocity_gradient);
	*velocity_gradient = vector_new(0.0, 0.0, 0.0);
	cncPut_velocity_gradient(velocity_gradientHandle, -1, -1, ctx);

	{ // Put "force" items
		s64 _node_id;
		for (_node_id = 0; _node_id < ((NODES)-(0)); _node_id++) {
			vector *force;
			cncHandle_t forceHandle = cncCreateItem_force(&force);
			*force = ctx->domain.initial_force[_node_id];
			cncPut_force(forceHandle, 0, _node_id, ctx);
		}
	}

	{ // Put "position" items
		s64 _node_id;
		for (_node_id = 0; _node_id < ((NODES)-(0)); _node_id++) {
			vertex *position;
			cncHandle_t positionHandle = cncCreateItem_position(&position);
			*position = ctx->domain.initial_position[_node_id];
			cncPut_position(positionHandle, 0, _node_id, ctx);
		}
	}

	{ // Put "velocity" items
		s64 _node_id;
		for (_node_id = 0; _node_id < ((NODES)-(0)); _node_id++) {
			vector *velocity;
			cncHandle_t velocityHandle = cncCreateItem_velocity(&velocity);
			*velocity = ctx->domain.initial_velocity[_node_id];
			cncPut_velocity(velocityHandle, 0, _node_id, ctx);
		}
	}

	{ // Put "volume" items
		s64 _element_id;
		for (_element_id = 0; _element_id < ((ELEMENTS)-(0)); _element_id++) {
			double *volume;
			cncHandle_t volumeHandle = cncCreateItem_volume(&volume);
			*volume = ctx->domain.initial_volume[_element_id];
			cncPut_volume(volumeHandle, 0, _element_id, ctx);
		}
	}

	{ // Put "viscosity" items
		s64 _element_id;
		for (_element_id = 0; _element_id < ((ELEMENTS)-(0)); _element_id++) {
			double *viscosity;
			cncHandle_t viscosityHandle = cncCreateItem_viscosity(&viscosity);
			*viscosity = ctx->domain.initial_viscosity[_element_id];
			cncPut_viscosity(viscosityHandle, 0, _element_id, ctx);
		}
	}

	{ // Put "pressure" items
		s64 _element_id;
		for (_element_id = 0; _element_id < ((ELEMENTS)-(0)); _element_id++) {
			double *pressure;
			cncHandle_t pressureHandle = cncCreateItem_pressure(&pressure);
			*pressure = ctx->domain.initial_pressure[_element_id];
			cncPut_pressure(pressureHandle, 0, _element_id, ctx);
		}
	}

	{ // Put "energy" items
		s64 _element_id;
		for (_element_id = 0; _element_id < ((ELEMENTS)-(0)); _element_id++) {
			double *energy;
			cncHandle_t energyHandle = cncCreateItem_energy(&energy);
			*energy = ctx->domain.initial_energy[_element_id];
			cncPut_energy(energyHandle, 0, _element_id, ctx);
		}
	}

	{ // Put "sound_speed" items
		s64 _element_id;
		for (_element_id = 0; _element_id < ((ELEMENTS)-(0)); _element_id++) {
			double *sound_speed;
			cncHandle_t sound_speedHandle = cncCreateItem_sound_speed(&sound_speed);
			*sound_speed = ctx->domain.initial_speed_sound[_element_id];
			cncPut_sound_speed(sound_speedHandle, 0, _element_id, ctx);
		}
	}

	// Put "elapsed_time" items
	double *elapsed_time;
	cncHandle_t elapsed_timeHandle = cncCreateItem_elapsed_time(&elapsed_time);
	*elapsed_time = ctx->domain.initial_delta_time; // Count the first time step
	cncPut_elapsed_time(elapsed_timeHandle, 1, ctx);

	// Put "delta_time" items
	double *delta_time;
	cncHandle_t delta_timeHandle = cncCreateItem_delta_time(&delta_time);
	*delta_time = ctx->domain.initial_delta_time;
	cncPut_delta_time(delta_timeHandle, 1, ctx);

	// Prescribe first steps
	int iteration = 0;
	int node_id;
	for(node_id = 0; node_id < ctx->mesh.number_nodes; ++node_id ) {
		cncPrescribe_reduce_force(iteration + 1, node_id, ctx);
		cncPrescribe_compute_velocity(iteration + 1, node_id, ctx);
		cncPrescribe_compute_position(iteration + 1, node_id, ctx);
	}

	int element_id;
	for(element_id = 0; element_id < ctx->mesh.number_elements; ++element_id ) {
		cncPrescribe_compute_stress_partial(iteration + 1, element_id, ctx);
		cncPrescribe_compute_hourglass_partial(iteration + 1, element_id, ctx);
		cncPrescribe_compute_volume(iteration + 1, element_id, ctx);
		cncPrescribe_compute_volume_derivative(iteration + 1, element_id, ctx);
		cncPrescribe_compute_gradients(iteration + 1, element_id, ctx);
		cncPrescribe_compute_viscosity_terms(iteration + 1, element_id, ctx);
		cncPrescribe_compute_energy(iteration + 1, element_id, ctx);
		cncPrescribe_compute_characteristic_length(iteration + 1, element_id, ctx);
		cncPrescribe_compute_time_constraints(iteration + 1, element_id, ctx);
	}

	cncPrescribe_compute_delta_time(iteration + 1, ctx);

	// Set finalizer function's tag
	lulesh_await(ctx);

}

/*
 * typeof final_origin_energy is double
 */
void lulesh_finalize(final_origin_energyItem final_origin_energy, luleshCtx *ctx) {

	printf("\nDomain Size = %d\n", EDGE_ELEMENTS);
	printf("Final Origin Energy =  %2.6e\n", final_origin_energy.item);

}

/* Mapping neighbor_velocity onto velocity */
velocityItemKey get_elements_node_neighbors_for_velocity(cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, luleshCtx *ctx) {

	velocityItemKey _result;

	_result.iteration = iteration;
	_result.node_id = ctx->mesh.elements_node_neighbors[element_id][local_node_id];

	return _result;

}

/* Mapping neighbor_position onto position */
positionItemKey get_elements_node_neighbors_for_position(cncTag_t element_id, cncTag_t local_node_id, cncTag_t iteration, luleshCtx *ctx) {

	positionItemKey _result;

	_result.iteration = iteration;
	_result.node_id = ctx->mesh.elements_node_neighbors[element_id][local_node_id];

	return _result;

}

/* Mapping neighbor_stress_partial onto stress_partial */
stress_partialItemKey get_map_id_for_node_element_neighbors_for_stress_partial(cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx) {

	int element_id = ctx->mesh.nodes_element_neighbors[node_id][local_element_id];
	int map_id = (node_id << 3) | local_element_id;

	stress_partialItemKey _result;
	_result.iteration = element_id < 0 ? -1 : iteration;
	_result.map_id = element_id < 0 ? -1 : map_id;

	return _result;

}

/* Mapping neighbor_hourglass_partial onto hourglass_partial */
hourglass_partialItemKey get_map_id_for_node_element_neighbors_for_hourglass_partial(cncTag_t node_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx) {

	int element_id = ctx->mesh.nodes_element_neighbors[node_id][local_element_id];
	int map_id = (node_id << 3) | local_element_id;

	hourglass_partialItemKey _result;
	_result.iteration = element_id < 0 ? -1 : iteration;
	_result.map_id = element_id < 0 ? -1 : map_id;

	return _result;

}

/* Mapping neighbor_velocity_gradient onto velocity_gradient */
velocity_gradientItemKey get_elements_element_neighbors_for_velocity_gradient(cncTag_t element_id, cncTag_t local_element_id, cncTag_t iteration, luleshCtx *ctx) {
	int neighbor_element_id =
			ctx->mesh.elements_element_neighbors[element_id][local_element_id];

	velocity_gradientItemKey _result;
	_result.iteration = iteration;
	_result.element_id = neighbor_element_id;

	if(neighbor_element_id == -2) {
		_result.iteration = -1;
		_result.element_id = -1; // zero
	} else if(neighbor_element_id == -1) {
		_result.element_id = element_id; // self
	}

	return _result;
}

/******************************************************************************
 * helper functions
 */

vertex vertex_new(double x, double y, double z) {
	return (vertex) {x, y, z};
}

vector vector_new(double x, double y, double z) {
	return (vector) {x, y, z};
}

vector vertex_sub(vertex a, vertex b) {
	return (vector) {a.x-b.x, a.y-b.y, a.z-b.z};
}

vector vector_sub(vector a, vector b) {
	return (vector) {a.x-b.x, a.y-b.y, a.z-b.z};
}

vector vector_add(vector a, vector b) {
	return (vector) {a.x+b.x, a.y+b.y, a.z+b.z};
}

vertex vertex_add(vertex a, vertex b) {
	return (vertex) {a.x+b.x, a.y+b.y, a.z+b.z};
}

vertex move(vertex a, vector b) {
	return (vertex) {a.x+b.x, a.y+b.y, a.z+b.z};
}

vector divide(vector a, double b) {
	return (vector) {a.x/b, a.y/b, a.z/b};
}

vector mult(vector a, double b) {
	return (vector) {a.x*b, a.y*b, a.z*b};
}

double dot(vector a, vector b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
};

vector cross(vector a, vector b) {
	return (vector) {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
};

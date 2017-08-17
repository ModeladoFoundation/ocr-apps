#include "lulesh.hpp"

/**
 * Step function defintion for "compute_viscosity_terms"
 */
extern "C"
void lulesh_compute_viscosity_terms(cncTag_t iteration, cncTag_t element_id, double volume, double volume_derivative, vector position_gradient, vector velocity_gradient, vector *neighbor_velocity_gradient, luleshCtx *ctx0) {
    LuleshCtx *ctx = static_cast<LuleshCtx*>(ctx0);

	//
	// INPUTS
	//

	vector velocity_gradients[6];
	{ // Access "neighbor_velocity_gradient" inputs
		s64 _local_element_id;
		for (_local_element_id = 0; _local_element_id < 6; _local_element_id++) {
			velocity_gradients[_local_element_id] = neighbor_velocity_gradient[_local_element_id];
		}
	}


	// Constants
	double ptiny = 1.e-36;
	double mass = ctx->ii.domain.element_mass[element_id];
	double volo = ctx->ii.domain.element_volume[element_id];
	double monoq_limiter_mult = ctx->constants.monoq_limiter_mult;
	double monoq_max_slope = ctx->constants.monoq_max_slope;
	double qlc_monoq = ctx->constants.qlc_monoq;
	double qqc_monoq = ctx->constants.qqc_monoq;

	// Compute

	// [CalcMonotonicQRegionForElems]
	double quadratic_viscosity_term;
	double linear_viscosity_term;

	if (volume_derivative > 0.0) {
		linear_viscosity_term  = 0.0;
		quadratic_viscosity_term = 0.0;
	} else {
		double rho = mass / (volo * volume);
		double temp_gradients[6] = {0, 0, 0, 0, 0, 0};

		vector normal = {	1.0 / (velocity_gradient.x + ptiny ),
				1.0 / (velocity_gradient.y + ptiny ),
				1.0 / (velocity_gradient.z + ptiny ) };

		double defaults[6] = {
				velocity_gradient.x, velocity_gradient.x,
				velocity_gradient.y, velocity_gradient.y,
				velocity_gradient.z, velocity_gradient.z };

		double normals[6] = {
				normal.x, normal.x,
				normal.y, normal.y,
				normal.z, normal.z };

		int face_id;
		for(face_id = 0; face_id < 6; face_id++) {
			// If we have a neighbor, use that gradient
			if(ctx->ii.mesh.elements_element_neighbors[element_id][face_id] >= 0) {
				if(face_id == 4 || face_id == 5) {
					temp_gradients[face_id] = velocity_gradients[face_id].z;
				} else if(face_id == 0 || face_id == 1) {
					temp_gradients[face_id] = velocity_gradients[face_id].x;
				} else if(face_id == 2 || face_id == 3) {
					temp_gradients[face_id] = velocity_gradients[face_id].y;
				}
			} else if(ctx->ii.mesh.elements_element_neighbors[element_id][face_id] == -2) {
				// Use default for the -xyz boundaries
				temp_gradients[face_id] = defaults[face_id];
			} else {
				// Zero out the +xyz boundaries
				temp_gradients[face_id] = 0.0;

			}
			temp_gradients[face_id] *= normals[face_id];
		}

		vector phi = {	0.5 * (temp_gradients[0] + temp_gradients[1]),
				0.5 * (temp_gradients[2] + temp_gradients[3]),
				0.5 * (temp_gradients[4] + temp_gradients[5]) };

		for(face_id = 0; face_id < 6; ++face_id) {
			temp_gradients[face_id] *= monoq_limiter_mult;
		}

		if(temp_gradients[0] < phi.x) phi.x = temp_gradients[0];
		if(temp_gradients[1] < phi.x) phi.x = temp_gradients[1];
		if(phi.x < 0.0) phi.x = 0.0;
		if(phi.x > monoq_limiter_mult) phi.x = monoq_max_slope;

		if(temp_gradients[2] < phi.y ) phi.y = temp_gradients[2];
		if(temp_gradients[3] < phi.y ) phi.y = temp_gradients[3];
		if(phi.y < 0.0) phi.y = 0.0;
		if(phi.y > monoq_max_slope) phi.y = monoq_max_slope;

		if(temp_gradients[4] < phi.z) phi.z = temp_gradients[4];
		if(temp_gradients[5] < phi.z) phi.z = temp_gradients[5];
		if(phi.z < 0.0) phi.z = 0.0;
		if(phi.z > monoq_max_slope) phi.z = monoq_max_slope;

		vector delvx = {	velocity_gradient.x * position_gradient.x,
				velocity_gradient.y * position_gradient.y,
				velocity_gradient.z * position_gradient.z };

		if(delvx.x > 0.0) delvx.x = 0.0;
		if(delvx.y > 0.0) delvx.y = 0.0;
		if(delvx.z > 0.0) delvx.z = 0.0;

		linear_viscosity_term = -qlc_monoq * rho *
				(delvx.x * (1.0 - phi.x) +
						delvx.y * (1.0 - phi.y) +
						delvx.z * (1.0 - phi.z) );

		quadratic_viscosity_term = qqc_monoq * rho *
				(delvx.x * delvx.x * (1.0 - phi.x * phi.x) +
						delvx.y * delvx.y * (1.0 - phi.y * phi.y) +
						delvx.z * delvx.z * (1.0 - phi.z * phi.z) );

	}

	//
	// OUTPUTS
	//

	// Put "linear_viscosity_term_out" items
	double *linear_viscosity_term_out = (decltype(linear_viscosity_term_out))cncItemAlloc(sizeof(*linear_viscosity_term_out));
	*linear_viscosity_term_out = linear_viscosity_term;
	cncPut_linear_viscosity_term(linear_viscosity_term_out, iteration, element_id, ctx);

	// Put "quadratic_viscosity_term_out" items
	double *quadratic_viscosity_term_out = (decltype(quadratic_viscosity_term_out))cncItemAlloc(sizeof(*quadratic_viscosity_term_out));
	*quadratic_viscosity_term_out = quadratic_viscosity_term;
	cncPut_quadratic_viscosity_term(quadratic_viscosity_term_out, iteration, element_id, ctx);

}

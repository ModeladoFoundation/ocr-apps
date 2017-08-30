#include "lulesh.hpp"

/**
 * Step function defintion for "compute_gradients"
 */
extern "C"
void lulesh_compute_gradients(cncTag_t iteration, cncTag_t element_id, double volume, vertex *neighbor_position, vector *neighbor_velocity, luleshCtx *ctx0) {
    LuleshCtx *ctx = static_cast<LuleshCtx*>(ctx0);

    //
    // INPUTS
    //
	vertex node_vertices[8];
	vector node_velocites[8];
    { // Access "neighbor_position" inputs
        s64 _local_node_id;
        for (_local_node_id = 0; _local_node_id < 8; _local_node_id++) {
			node_vertices[_local_node_id] = neighbor_position[_local_node_id];
			node_velocites[_local_node_id] = neighbor_velocity[_local_node_id];
        }
    }
	// Compute
	double ptiny = 1.e-36;
	double initial_volume = ctx->ii.domain.element_volume[element_id];

	// Compute

	// [CalcMonotonicQGradientsForElems]
	double vol = initial_volume * volume;
	double norm = 1.0 / ( vol + ptiny );

	double delx_zeta, delx_xi, delx_eta; // Position gradient
	double delv_zeta, delv_xi, delv_eta; // Velocity gradient

	vector dj = mult(vertex_sub(
			vertex_add(vertex_add(node_vertices[0], node_vertices[1]),
					vertex_add(node_vertices[5], node_vertices[4])),
					vertex_add(vertex_add(node_vertices[3], node_vertices[2]),
							vertex_add(node_vertices[6], node_vertices[7]))), -0.25);

	vector di = mult(vertex_sub(
			vertex_add(vertex_add(node_vertices[1], node_vertices[2]),
					vertex_add(node_vertices[6], node_vertices[5])),
					vertex_add(vertex_add(node_vertices[0], node_vertices[3]),
							vertex_add(node_vertices[7], node_vertices[4]))), 0.25);

	vector dk = mult(vertex_sub(
			vertex_add(vertex_add(node_vertices[4], node_vertices[5]),
					vertex_add(node_vertices[6], node_vertices[7])),
					vertex_add(vertex_add(node_vertices[0], node_vertices[1]),
							vertex_add(node_vertices[2], node_vertices[3]))), 0.25);

	vector dv_eta = mult(vector_sub(
			vector_add(vector_add(node_velocites[0], node_velocites[1]),
					vector_add(node_velocites[5], node_velocites[4])),
					vector_add(vector_add(node_velocites[3], node_velocites[2]),
							vector_add(node_velocites[6], node_velocites[7]))), -0.25);

	vector dv_xi = mult(vector_sub(
			vector_add(vector_add(node_velocites[1], node_velocites[2]),
					vector_add(node_velocites[6], node_velocites[5])),
					vector_add(vector_add(node_velocites[0], node_velocites[3]),
							vector_add(node_velocites[7], node_velocites[4]))), 0.25);

	vector dv_zeta = mult(vector_sub(
			vector_add(vector_add(node_velocites[4], node_velocites[5]),
					vector_add(node_velocites[6], node_velocites[7])),
					vector_add(vector_add(node_velocites[0], node_velocites[1]),
							vector_add(node_velocites[2], node_velocites[3]))), 0.25);

	vector a_zeta = cross(di, dj);
	vector a_xi = cross(dj, dk);
	vector a_eta = cross(dk, di);

	delx_zeta = vol / sqrt(dot(a_zeta, a_zeta) + ptiny);
	delx_xi = vol / sqrt(dot(a_xi, a_xi) + ptiny);
	delx_eta = vol / sqrt(dot(a_eta, a_eta) + ptiny);

	a_zeta = mult(a_zeta, norm);
	a_xi = mult(a_xi, norm);
	a_eta = mult(a_eta, norm);

	delv_zeta = dot(a_zeta, dv_zeta);
	delv_xi = dot(a_xi, dv_xi);
	delv_eta = dot(a_eta, dv_eta);

	vector position_gradient = {delx_xi, delx_eta, delx_zeta};
	vector velocity_gradient = {delv_xi, delv_eta, delv_zeta};

    //
    // OUTPUTS
    //

    // Put "position_gradient_out" items
    vector *position_gradient_out = (decltype(position_gradient_out))cncItemAlloc(sizeof(*position_gradient_out));
	*position_gradient_out = position_gradient;
    cncPut_position_gradient(position_gradient_out, iteration, element_id, ctx);

    // Put "velocity_gradient_out" items
    vector *velocity_gradient_out = (decltype(velocity_gradient_out))cncItemAlloc(sizeof(*velocity_gradient_out));
	*velocity_gradient_out = velocity_gradient;
    cncPut_velocity_gradient(velocity_gradient_out, iteration, element_id, ctx);


}

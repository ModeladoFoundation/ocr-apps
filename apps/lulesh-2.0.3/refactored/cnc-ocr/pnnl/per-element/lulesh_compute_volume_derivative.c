#include "lulesh.h"

/**
 * Step function defintion for "compute_volume_derivative"
 */
void lulesh_compute_volume_derivative(cncTag_t iteration, cncTag_t element_id, double delta_time, vertex *neighbor_position, vector *neighbor_velocity, luleshCtx *ctx) {

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
	vertex temp_verticies[8] = {
			{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
			{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}};

	double dt2 = 0.5 * delta_time;

	int i;
	for (i = 0; i < 8; ++i) {
		temp_verticies[i] =	move(node_vertices[i], mult(node_velocites[i], -dt2));
	}

	vector d60 = vertex_sub(temp_verticies[6], temp_verticies[0]);
	vector d53 = vertex_sub(temp_verticies[5], temp_verticies[3]);
	vector d71 = vertex_sub(temp_verticies[7], temp_verticies[1]);
	vector d42 = vertex_sub(temp_verticies[4], temp_verticies[2]);

	// [CalcElemShapeFunctionDerivatives]
	vector cofactors[3] = {{0,0,0}, {0,0,0}, {0,0,0}};

	vector d0 = mult(vector_sub(vector_add(d60, d53),vector_add(d71, d42)),0.125);
	vector d1 = mult(vector_add(vector_sub(d60, d53),vector_sub(d71, d42)),0.125);
	vector d2 = mult(vector_add(vector_add(d60, d53),vector_add(d71, d42)),0.125);

	cofactors[0].x =    (d1.y * d2.z) - (d1.z * d2.y); // cjxi
	cofactors[1].x =  - (d0.y * d2.z) + (d0.z * d2.y); // cjet
	cofactors[2].x =    (d0.y * d1.z) - (d0.z * d1.y); // cjze

	cofactors[0].y =  - (d1.x * d2.z) + (d1.z * d2.x);
	cofactors[1].y =    (d0.x * d2.z) - (d0.z * d2.x);
	cofactors[2].y =  - (d0.x * d1.z) + (d0.z * d1.x);

	cofactors[0].z =    (d1.x * d2.y) - (d1.y * d2.x);
	cofactors[1].z =  - (d0.x * d2.y) + (d0.y * d2.x);
	cofactors[2].z =    (d0.x * d1.y) - (d0.y * d1.x);

	double inv_detJ = 1.0/ (8.0 *
			(d1.x * cofactors[1].x + d1.y * cofactors[1].y + d1.z * cofactors[1].z));

	vector zeros = {0, 0, 0};
	vector b[4] = {
			vector_sub(vector_sub(
					vector_sub(zeros, cofactors[0]), cofactors[1]), cofactors[2]),
					vector_sub(vector_sub(cofactors[0], cofactors[1]), cofactors[2]),
					vector_sub(vector_add(cofactors[0], cofactors[1]), cofactors[2]),
					vector_sub(vector_add(
							vector_sub(zeros, cofactors[0]), cofactors[1]), cofactors[2])
	};

	vector d06 = vector_sub(node_velocites[0], node_velocites[6]);
	vector d17 = vector_sub(node_velocites[1], node_velocites[7]);
	vector d24 = vector_sub(node_velocites[2], node_velocites[4]);
	vector d35 = vector_sub(node_velocites[3], node_velocites[5]);

	// [CalcElemVelocityGradient] ?
	double dxx = inv_detJ *
			(b[0].x * d06.x + b[1].x * d17.x + b[2].x * d24.x + b[3].x * d35.x);
	double dyy = inv_detJ *
			(b[0].y * d06.y + b[1].y * d17.y + b[2].y * d24.y + b[3].y * d35.y);
	double dzz = inv_detJ *
			(b[0].z * d06.z + b[1].z * d17.z + b[2].z * d24.z + b[3].z * d35.z);

	double volume_derivative = dxx + dyy + dzz;

    //
    // OUTPUTS
    //

    // Put "volume_derivative_out" items
    double *volume_derivative_out = cncItemAlloc(sizeof(*volume_derivative_out));
	*volume_derivative_out = volume_derivative;
    cncPut_volume_derivative(volume_derivative_out, iteration, element_id, ctx);


}

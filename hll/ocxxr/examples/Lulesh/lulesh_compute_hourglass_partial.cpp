#include "lulesh.hpp"

/**
 * Step function defintion for "compute_hourglass_partial"
 */
extern "C"
void lulesh_compute_hourglass_partial(cncTag_t iteration, cncTag_t element_id, double element_volume, double sound_speed, vertex *neighbor_position, vector *neighbor_velocity, luleshCtx *ctx0) {
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
	// [CalcElemVolumeDerivative]
	vertex a01 = vertex_add(node_vertices[0], node_vertices[1]);
	vertex a03 = vertex_add(node_vertices[0], node_vertices[3]);
	vertex a04 = vertex_add(node_vertices[0], node_vertices[4]);
	vertex a12 = vertex_add(node_vertices[1], node_vertices[2]);
	vertex a15 = vertex_add(node_vertices[1], node_vertices[5]);
	vertex a23 = vertex_add(node_vertices[2], node_vertices[3]);
	vertex a26 = vertex_add(node_vertices[2], node_vertices[6]);
	vertex a37 = vertex_add(node_vertices[3], node_vertices[7]);
	vertex a45 = vertex_add(node_vertices[4], node_vertices[5]);
	vertex a47 = vertex_add(node_vertices[4], node_vertices[7]);
	vertex a56 = vertex_add(node_vertices[5], node_vertices[6]);
	vertex a67 = vertex_add(node_vertices[6], node_vertices[7]);

	vertex dvs[8][6] =
	{	{a23, a12, a15, a45, a37, a47},
			{a03, a23, a26, a56, a04, a45},
			{a01, a03, a37, a67, a15, a56},
			{a12, a01, a04, a47, a26, a67},
			{a56, a67, a37, a03, a15, a01},
			{a67, a47, a04, a01, a26, a12},
			{a47, a45, a15, a12, a37, a23},
			{a45, a56, a26, a23, a04, a03}	};

	vector v[8] = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
			{0,0,0}, {0,0,0},	{0,0,0}, {0,0,0}};

	int f;
	for(f = 0; f< 8; f++) {

		double x =
				dvs[f][0].y * dvs[f][1].z - dvs[f][1].y * dvs[f][0].z +
				dvs[f][2].y * dvs[f][3].z - dvs[f][3].y * dvs[f][2].z -
				dvs[f][4].y * dvs[f][5].z + dvs[f][5].y * dvs[f][4].z;

		double y =
				-dvs[f][0].x * dvs[f][1].z + dvs[f][1].x * dvs[f][0].z -
				dvs[f][2].x * dvs[f][3].z + dvs[f][3].x * dvs[f][2].z +
				dvs[f][4].x * dvs[f][5].z - dvs[f][5].x * dvs[f][4].z;
		double z =
				-dvs[f][0].y * dvs[f][1].x + dvs[f][1].y * dvs[f][0].x -
				dvs[f][2].y * dvs[f][3].x + dvs[f][3].y * dvs[f][2].x +
				dvs[f][4].y * dvs[f][5].x - dvs[f][5].y * dvs[f][4].x;

		v[f].x = x * 1.0/12.0;
		v[f].y = y * 1.0/12.0;
		v[f].z = z * 1.0/12.0;

	}

	// [CalcFBHourglassForceForElems]
	int gamma[4][8] = {
			{1, 1, -1, -1, -1, -1, 1, 1},
			{1, -1, -1, 1, -1, 1, 1, -1},
			{1, -1, 1, -1, 1, -1, 1, -1},
			{-1, 1, -1, 1, 1, -1, 1, -1} };

	double volo = ctx->ii.domain.element_volume[element_id];

	double determ_value = element_volume * volo;

	double volinv = 1.0 / determ_value;
	double coefficient = - ctx->constants.hgcoef * 0.01 * sound_speed *
			ctx->ii.domain.element_mass[element_id] / cbrt(determ_value);

	double hourgam[8][4] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},
			{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};

	int i1;
	for(i1 = 0; i1 < 4; ++i1) {
		vector hourmod = {0, 0, 0};
		int i;
		for(i = 0; i < 8; i++) {
			hourmod.x += node_vertices[i].x * (double)gamma[i1][i];
			hourmod.y += node_vertices[i].y * (double)gamma[i1][i];
			hourmod.z += node_vertices[i].z * (double)gamma[i1][i];
		}
		for(i = 0; i < 8; i++) {
			hourgam[i][i1] = gamma[i1][i] - volinv * (dot(v[i], hourmod));
		}
	}

	vector forces_out[8] = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
			{0,0,0}, {0,0,0},	{0,0,0}, {0,0,0}};

	int i, j;

	for(j = 0; j < 4; j++) {
		vector temp = {0, 0, 0};
		for(i = 0; i < 8; i++) {
			temp.x += node_velocites[i].x * hourgam[i][j];
			temp.y += node_velocites[i].y * hourgam[i][j];
			temp.z += node_velocites[i].z * hourgam[i][j];
		}
		for(i = 0; i < 8; i++) {
			forces_out[i].x += temp.x * hourgam[i][j];
			forces_out[i].y += temp.y * hourgam[i][j];
			forces_out[i].z += temp.z * hourgam[i][j];
		}
	}

	//int i;
	for(i = 0; i < 8; i++) {
		forces_out[i].x *= coefficient;
		forces_out[i].y *= coefficient;
		forces_out[i].z *= coefficient;
	}

	//
	// OUTPUTS
	//

	int local_node_id;
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
		int node_id = ctx->ii.mesh.elements_node_neighbors[element_id][local_node_id];
		// From the perspective of the node
		int local_element_id;
		for(local_element_id = 0; local_element_id < 8; local_element_id++) {
			if(ctx->ii.mesh.nodes_element_neighbors[node_id][local_element_id] == element_id){
				// Then this is where we want to store the force value
				int _map_id = (node_id << 3) | local_element_id;
				// Put "hourglass_partialout" items
				vector *hourglass_partial_out = (decltype(hourglass_partial_out))cncItemAlloc(sizeof(*hourglass_partial_out));
					*hourglass_partial_out = forces_out[local_node_id];
				cncPut_hourglass_partial(hourglass_partial_out, iteration, _map_id, ctx);
			}
		}
	}
}

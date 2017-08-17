#include "lulesh.hpp"

/**
 * Step function defintion for "compute_stress_partial"
 */
extern "C"
void lulesh_compute_stress_partial(cncTag_t iteration, cncTag_t element_id, double pressure, double viscosity, vertex *neighbor_position, luleshCtx *ctx0) {
    LuleshCtx *ctx = static_cast<LuleshCtx*>(ctx0);

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

	// Compute
	// [IntegrateStressTermsForElems]
	double stress = - pressure - viscosity;


	// [CalcElemNodeNormals]
	int face_ids[6][4] = {
			{0, 1, 2, 3}, {0, 4, 5, 1}, {1, 5, 6, 2},
			{2, 6, 7, 3}, {3, 7, 4, 0}, {4, 7, 6, 5}};

	vector b[8] = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
			{0,0,0}, {0,0,0},	{0,0,0}, {0,0,0}};

	int i, j;
	for(i = 0; i < 6; i++) {
		vector fb1 = mult(
				vertex_sub(
						vertex_add(node_vertices[face_ids[i][3]], node_vertices[face_ids[i][2]]),
						vertex_add(node_vertices[face_ids[i][1]], node_vertices[face_ids[i][0]])),
						0.5);
		vector fb2 = mult(
				vertex_sub(
						vertex_add(node_vertices[face_ids[i][2]], node_vertices[face_ids[i][1]]),
						vertex_add(node_vertices[face_ids[i][3]], node_vertices[face_ids[i][0]])),
						0.5);

		// [SumElemFaceNormal]
		double x = fb1.y * fb2.z - fb1.z * fb2.y;
		double y = fb1.z * fb2.x - fb1.x * fb2.z;
		double z = fb1.x * fb2.y - fb1.y * fb2.x;

		vector area = {x*.25, y*.25, z*.25};

		// [SumElemStressesToNodeForces]
		for(j = 0; j < 4; j++) {
			b[face_ids[i][j]].x += area.x;
			b[face_ids[i][j]].y += area.y;
			b[face_ids[i][j]].z += area.z;
		}
	}

	vector forces_out[8] = {
			{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
			{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0} };

	for(i = 0; i < 8; i++) {
		forces_out[i] = vector_sub(forces_out[i], mult(b[i], stress));
	}

	//
	// OUTPUTS
	//

	// Produce
	int local_node_id;
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
		int node_id = ctx->ii.mesh.elements_node_neighbors[element_id][local_node_id];
		// From the perspective of the node
		int local_element_id;
		for(local_element_id = 0; local_element_id < 8; local_element_id++) {
			if(ctx->ii.mesh.nodes_element_neighbors[node_id][local_element_id]==element_id){
				// Then this is where we want to store the force value
				int _map_id = (node_id << 3) | local_element_id;
				vector *stress_partial_out = (decltype(stress_partial_out))cncItemAlloc(sizeof(*stress_partial_out));
				*stress_partial_out = forces_out[local_node_id];
				cncPut_stress_partial(stress_partial_out, iteration, _map_id, ctx);

			}
		}
	}

}

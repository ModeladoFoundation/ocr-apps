#include "lulesh.h"

/*
 * typeof volume is double
 * typeof neighbor_position is vertex
 */
void compute_characteristic_length(cncTag_t iteration, cncTag_t element_id, volumeItem volume_in, neighbor_positionItem *neighbor_position, luleshCtx *ctx) {

	double volume = volume_in.item;

	//
	// INPUTS
	//
	vertex node_vertices[8];

	{ // Access "neighbor_position" inputs
		s64 _local_node_id;
		for (_local_node_id = 0; _local_node_id < ((8)-(0)); _local_node_id++) {
			node_vertices[_local_node_id] = neighbor_position[_local_node_id].item;
		}
	}

	// Compute
	// [CalcElemCharacteristicLength]
	int face_ids[6][4] = {
			{0, 1, 2, 3}, {4, 5, 6, 7}, {0, 1, 5, 4},
			{1, 2, 6, 5}, {2, 3, 7, 6}, {3, 0, 4, 7}};

	double char_length = 0.0;

	int face_index;
	for(face_index = 0; face_index < 6; ++face_index) {

		vector d20 = vertex_sub(node_vertices[face_ids[face_index][2]],
				node_vertices[face_ids[face_index][0]]);

		vector d31 = vertex_sub(node_vertices[face_ids[face_index][3]],
				node_vertices[face_ids[face_index][1]]);

		vector f2031 = vector_sub(d20, d31);
		vector g2031 = vector_add(d20, d31);

		double area = dot(f2031, f2031) * dot(g2031, g2031) -
									dot(f2031, g2031) * dot(f2031, g2031);

		if(area > char_length)
			char_length = area;
	}

	volume = volume * ctx->domain.element_volume[element_id];

	double characteristic_length = 4.0 * volume / sqrt(char_length);

	//
	// OUTPUTS
	//

	// Put "characteristic_length_out" items
	double *characteristic_length_out;
	cncHandle_t characteristic_length_outHandle = cncCreateItem_characteristic_length(&characteristic_length_out);
	*characteristic_length_out = characteristic_length;
	cncPut_characteristic_length(characteristic_length_outHandle, iteration, element_id, ctx);

}

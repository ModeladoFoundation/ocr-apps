#include "lulesh.hpp"

extern "C" {

// initialize domain function
void init_graph_context(luleshCtx *ctx);

void init_graph_context(luleshCtx *ctx) {

	int mesh_size = EDGE_ELEMENTS;
	int max_iterations = 3;

	ctx->elements = ELEMENTS;
	ctx->nodes = NODES;

	// Initialize the domain constants, these will be used throughout the code
	ctx->constants = (struct constants) {
		3.0, 4.0/3.0, 1.0e+12, 1.0, 2.0, 0.5, 2.0/3.0,
		2.0, 1.0e+9, 1.0e-9, 0.0, -1.0e+15, 0.1, 1.0};

	ctx->cutoffs = (struct cutoffs) {
		1.0e-7, 1.0e-7, 1.0e-7, 1.0e-10, 1.0e-7 };

	ctx->constraints = (struct constraints) {
		1.0e-2, 1.0e-2, max_iterations };

	// Initialize the mesh (regular cubes, delta spaced)
	int edge_elements = mesh_size;
	int edge_nodes = EDGE_NODES;

	int row_id, column_id, plane_id;
	int element_id, node_id = 0;

	// Initialize the structure; keeps track of size and neighbors
	ctx->mesh.number_nodes = NODES;
	ctx->mesh.number_elements = ELEMENTS;

	double scale = ((double)edge_elements)/45.0;
	double einit = 3.948746e+7*scale*scale*scale;

	// Setup initial vertices and nodes neighboring nodes
	node_id = 0;
	double delta = 1.125 / ((double) edge_elements);
	for (plane_id = 0; plane_id < edge_nodes; ++plane_id) {
		double z = delta * plane_id;
		for (row_id = 0; row_id < edge_nodes; ++row_id) {
			double y = delta * row_id;
			for (column_id = 0; column_id < edge_nodes; ++column_id) {
				double x = delta * column_id;

				ctx->domain.initial_position[node_id] = vertex_new(x, y, z);
				ctx->domain.initial_force[node_id] = vector_new(0.0, 0.0, 0.0);
				ctx->domain.initial_velocity[node_id] = vector_new(0.0, 0.0, 0.0);

				ctx->mesh.nodes_node_neighbors[node_id][0] =
						(column_id-1) < 0 ? -2 : (node_id - 1);
				ctx->mesh.nodes_node_neighbors[node_id][1] =
						(column_id+1) >= edge_nodes ? -1 : (node_id + 1);
				ctx->mesh.nodes_node_neighbors[node_id][2] =
						(row_id-1) < 0 ? -2 : node_id - edge_nodes;
				ctx->mesh.nodes_node_neighbors[node_id][3] =
						(row_id+1) >= edge_nodes ? -1 : node_id + edge_nodes;
				ctx->mesh.nodes_node_neighbors[node_id][4] =
						(plane_id-1) < 0 ? -2 : node_id - edge_nodes * edge_nodes;
				ctx->mesh.nodes_node_neighbors[node_id][5] =
						(plane_id+1) >= edge_nodes ? -1 : node_id + (edge_nodes*edge_nodes);

				ctx->mesh.nodes_element_neighbors[node_id][0] = -2;
				ctx->mesh.nodes_element_neighbors[node_id][1] = -1;
				ctx->mesh.nodes_element_neighbors[node_id][2] = -2;
				ctx->mesh.nodes_element_neighbors[node_id][3] = -1;
				ctx->mesh.nodes_element_neighbors[node_id][4] = -2;
				ctx->mesh.nodes_element_neighbors[node_id][5] = -1;
				ctx->mesh.nodes_element_neighbors[node_id][6] = -2;
				ctx->mesh.nodes_element_neighbors[node_id][7] = -1;

				ctx->domain.node_mass[node_id] = 0; // Initialize

				node_id++;
			}
		}
	}

	// Setup elements node and element neighbors
	node_id = 0;
	element_id = 0;
	for (plane_id = 0; plane_id < edge_elements; plane_id++) {
		for (row_id = 0; row_id < edge_elements; row_id++) {
			for (column_id = 0; column_id < edge_elements; column_id++) {

				ctx->mesh.elements_node_neighbors[element_id][0] =
						node_id;
				ctx->mesh.elements_node_neighbors[element_id][1] =
						node_id + 1;
				ctx->mesh.elements_node_neighbors[element_id][2] =
						node_id + edge_nodes + 1;
				ctx->mesh.elements_node_neighbors[element_id][3] =
						node_id + edge_nodes;
				ctx->mesh.elements_node_neighbors[element_id][4] =
						node_id + edge_nodes * edge_nodes;
				ctx->mesh.elements_node_neighbors[element_id][5] =
						node_id + edge_nodes * edge_nodes + 1;
				ctx->mesh.elements_node_neighbors[element_id][6] =
						node_id + edge_nodes * edge_nodes + edge_nodes + 1;
				ctx->mesh.elements_node_neighbors[element_id][7] =
						node_id + edge_nodes * edge_nodes + edge_nodes;

				ctx->mesh.nodes_element_neighbors[
				                                  ctx->mesh.elements_node_neighbors[element_id][0]][6] = element_id;
				ctx->mesh.nodes_element_neighbors[
				                                  ctx->mesh.elements_node_neighbors[element_id][1]][7] = element_id;
				ctx->mesh.nodes_element_neighbors[
				                                  ctx->mesh.elements_node_neighbors[element_id][2]][4] = element_id;
				ctx->mesh.nodes_element_neighbors[
				                                  ctx->mesh.elements_node_neighbors[element_id][3]][5] = element_id;
				ctx->mesh.nodes_element_neighbors[
				                                  ctx->mesh.elements_node_neighbors[element_id][4]][2] = element_id;
				ctx->mesh.nodes_element_neighbors[
				                                  ctx->mesh.elements_node_neighbors[element_id][5]][3] = element_id;
				ctx->mesh.nodes_element_neighbors[
				                                  ctx->mesh.elements_node_neighbors[element_id][6]][0] = element_id;
				ctx->mesh.nodes_element_neighbors[
				                                  ctx->mesh.elements_node_neighbors[element_id][7]][1] = element_id;

				ctx->mesh.elements_element_neighbors[element_id][0] =
						(column_id-1) < 0 ? -2 : (element_id - 1);
				ctx->mesh.elements_element_neighbors[element_id][1] =
						(column_id+1) >= edge_elements ? -1 : (element_id + 1);
				ctx->mesh.elements_element_neighbors[element_id][2] =
						(row_id-1) < 0 ? -2 : element_id - edge_elements;
				ctx->mesh.elements_element_neighbors[element_id][3] =
						(row_id+1) >= edge_elements ? -1 : element_id + edge_elements;
				ctx->mesh.elements_element_neighbors[element_id][4] =
						(plane_id-1) < 0 ? -2 : element_id - edge_elements * edge_elements;
				ctx->mesh.elements_element_neighbors[element_id][5] =
						(plane_id+1) >= edge_elements ? -1 : element_id +
								(edge_elements*edge_elements);

				vertex c[] = {
						ctx->domain.initial_position[ctx->mesh.elements_node_neighbors[element_id][0]],
						ctx->domain.initial_position[ctx->mesh.elements_node_neighbors[element_id][1]],
						ctx->domain.initial_position[ctx->mesh.elements_node_neighbors[element_id][2]],
						ctx->domain.initial_position[ctx->mesh.elements_node_neighbors[element_id][3]],
						ctx->domain.initial_position[ctx->mesh.elements_node_neighbors[element_id][4]],
						ctx->domain.initial_position[ctx->mesh.elements_node_neighbors[element_id][5]],
						ctx->domain.initial_position[ctx->mesh.elements_node_neighbors[element_id][6]],
						ctx->domain.initial_position[ctx->mesh.elements_node_neighbors[element_id][7]] };

				// Compute
				double volume = (
						dot(cross(vertex_sub(c[6], c[3]), vertex_sub(c[2], c[0])),
								vector_add(vertex_sub(c[3], c[1]), vertex_sub(c[7], c[2]))) +
								dot(cross(vertex_sub(c[6], c[4]), vertex_sub(c[7], c[0])),
										vector_add(vertex_sub(c[4], c[3]), vertex_sub(c[5], c[7]))) +
										dot(cross(vertex_sub(c[6], c[1]), vertex_sub(c[5], c[0])),
												vector_add(vertex_sub(c[1], c[4]),vertex_sub(c[2], c[5])))) *
														(1.0/12.0);

				ctx->domain.element_volume[element_id] = volume;
				ctx->domain.element_mass[element_id] = volume;

				ctx->domain.initial_volume[element_id] = 1.0;
				ctx->domain.initial_viscosity[element_id] = 0.0;
				ctx->domain.initial_pressure[element_id] = 0.0;
				ctx->domain.initial_energy[element_id] = (element_id == 0 ? einit : 0.0);
				ctx->domain.initial_speed_sound[element_id] = 0.0;

				double node_v = volume / 8.0;
				ctx->domain.node_mass[ctx->mesh.elements_node_neighbors[element_id][0]] += node_v;
				ctx->domain.node_mass[ctx->mesh.elements_node_neighbors[element_id][1]] += node_v;
				ctx->domain.node_mass[ctx->mesh.elements_node_neighbors[element_id][2]] += node_v;
				ctx->domain.node_mass[ctx->mesh.elements_node_neighbors[element_id][3]] += node_v;
				ctx->domain.node_mass[ctx->mesh.elements_node_neighbors[element_id][4]] += node_v;
				ctx->domain.node_mass[ctx->mesh.elements_node_neighbors[element_id][5]] += node_v;
				ctx->domain.node_mass[ctx->mesh.elements_node_neighbors[element_id][6]] += node_v;
				ctx->domain.node_mass[ctx->mesh.elements_node_neighbors[element_id][7]] += node_v;

				element_id++;
				node_id++;
			}
			node_id++;
		}
		node_id += edge_nodes;
	}


	double delta_time = 0.5*cbrt(ctx->domain.element_volume[0])/sqrt(2.0*einit);

	ctx->domain.initial_delta_time = delta_time;
}
} // extern C

#include "ocxxr-main.hpp"

#define MY_PTR_INIT(CTX, FIELD1, FIELD2) (CTX->ii.FIELD1.FIELD2 = &(CTX->FIELD1.FIELD2[0]))
#define MY_DP_INIT(FIELD_NAME) MY_PTR_INIT(ctx, domain, FIELD_NAME)
#define MY_MP_INIT(FIELD_NAME, IDX) MY_PTR_INIT(ctx, mesh, FIELD_NAME[IDX])

#define MY_CONST_INIT(CTX, FIELD1, FIELD2) (CTX->ii.FIELD1.FIELD2 = CTX->FIELD1.FIELD2)
#define MY_DC_INIT(FIELD_NAME) MY_CONST_INIT(ctx, domain, FIELD_NAME)
#define MY_MC_INIT(FIELD_NAME) MY_CONST_INIT(ctx, mesh, FIELD_NAME)

void init_domain(LuleshCtx *ctx) {
    // Remains constant
    MY_DP_INIT(node_mass);
    MY_DP_INIT(element_mass);
    MY_DP_INIT(element_volume);
    // Initial per iteration values
    MY_DC_INIT(initial_delta_time);
    // Initial node values
    MY_DP_INIT(initial_force);
    MY_DP_INIT(initial_velocity);
    MY_DP_INIT(initial_position);
    // Initial element
    MY_DP_INIT(initial_volume);
    MY_DP_INIT(initial_viscosity);
    MY_DP_INIT(initial_pressure);
    MY_DP_INIT(initial_energy);
    MY_DP_INIT(initial_speed_sound);
}

void init_mesh(LuleshCtx *ctx) {
    MY_MC_INIT(number_nodes);
    MY_MC_INIT(number_elements);

    decltype(&ctx->_int_ptrs[0]) p = &ctx->_int_ptrs[0];

    ctx->ii.mesh.nodes_node_neighbors = p;
    p += NODES;
    for (int i=0; i<NODES; i++) {
        MY_MP_INIT(nodes_node_neighbors, i);
    }
    ctx->ii.mesh.nodes_element_neighbors = p;
    p += NODES;
    for (int i=0; i<NODES; i++) {
        MY_MP_INIT(nodes_element_neighbors, i);
    }

    ctx->ii.mesh.elements_node_neighbors = p;
    p += ELEMENTS;
    for (int i=0; i<ELEMENTS; i++) {
        MY_MP_INIT(elements_node_neighbors, i);
    }
    ctx->ii.mesh.elements_element_neighbors = p;
    p += ELEMENTS;
    for (int i=0; i<ELEMENTS; i++) {
        MY_MP_INIT(elements_element_neighbors, i);
    }
}

// expect 2.435244e+04
void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs> /* args */) {

    printf("Hello!\n");

    //void lulesh_ctx_init(ocrGuid_t contextGuid, luleshCtx *ctx);
    // Create a new graph context
    auto ctx = ocxxr::Datablock<LuleshCtx>::Create();

    // Init
    luleshCtx *context = ctx.data_ptr();
    lulesh_ctx_init(ctx.handle().guid(), context);

    // initialize graph context parameters
    init_graph_context(context);

    // OCXXR COPIES
    init_domain(ctx.data_ptr());
    init_mesh(ctx.data_ptr());

    // Launch the graph for execution
    lulesh_launch(NULL, context);

    // Exit when the graph execution completes
    CNC_SHUTDOWN_ON_FINISH(context);
}


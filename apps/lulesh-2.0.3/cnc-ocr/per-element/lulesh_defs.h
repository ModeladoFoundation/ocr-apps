#ifndef _CNCOCR_LULESH_TYPES_H_
#define _CNCOCR_LULESH_TYPES_H_

#define EDGE_ELEMENTS 4
#define EDGE_NODES (EDGE_ELEMENTS + 1)
#define NODES (EDGE_NODES * EDGE_NODES * EDGE_NODES)
#define ELEMENTS (EDGE_ELEMENTS * EDGE_ELEMENTS * EDGE_ELEMENTS)

typedef struct luleshArguments {
    /* TODO: Add struct members.
     * Use this struct to pass all arguments for
     * graph initialization. This should not contain any
     * pointers (unless you know you'll only be executing
     * in shared memory and thus passing pointers is safe).
     */
} luleshArgs;

typedef struct vector {
	double x, y, z;
} vector;

typedef struct vertex {
	double x, y, z;
} vertex;

/**
 * LULESH has some constants set up in the domain class
 * These would usually be read in from an input file
 */
struct constraints {
	double max_delta_time, stop_time;
	int maximum_iterations;
};

struct constants {
	double hgcoef, ss4o3, qstop, monoq_max_slope, monoq_limiter_mult, qlc_monoq,
	qqc_monoq, qqc, eosvmax, eosvmin, pmin, emin,	dvovmax, refdens;
} constants;

struct cutoffs {
	double e, p, q, v, u;
} cutoffs;

struct domain {
	// Remains constant
	double node_mass[NODES];
	double element_mass[ELEMENTS];
	double element_volume[ELEMENTS];
	// Initial per iteration values
	double initial_delta_time;
	// Initial node values
	vector initial_force[NODES];
	vector initial_velocity[NODES];
	vertex initial_position[NODES];
	// Initial element
	double initial_volume[ELEMENTS];
	double initial_viscosity[ELEMENTS];
	double initial_pressure[ELEMENTS];
	double initial_energy[ELEMENTS];
	double initial_speed_sound[ELEMENTS];
} domain;

struct mesh {
	int number_nodes;
	int number_elements;
	int nodes_node_neighbors[NODES][6]; 					// 6 * number_nodes
	int nodes_element_neighbors[NODES][8];				// 8 * number_nodes
	int elements_node_neighbors[ELEMENTS][8];			// 8 * number_elements
	int elements_element_neighbors[ELEMENTS][6];	// 6 * number_elements
} mesh;

vertex vertex_new(double x, double y, double z);

vector vector_new(double x, double y, double z);

vector vertex_sub(vertex a, vertex b);

vector vector_sub(vector a, vector b);

vector vector_add(vector a, vector b);

vertex vertex_add(vertex a, vertex b);

vertex move(vertex a, vector b);

vector divide(vector a, double b);

vector mult(vector a, double b);

double dot(vector a, vector b);

vector cross(vector a, vector b);

#endif /*_CNCOCR_LULESH_TYPES_H_*/

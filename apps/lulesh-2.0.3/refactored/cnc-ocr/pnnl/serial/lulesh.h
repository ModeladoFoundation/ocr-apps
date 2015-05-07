#include <math.h>
#include <stdio.h>
#include <stdlib.h>

struct vector;
struct vertex;

/**
 * Vector structure, useful for vector math
 */
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
	int max_iterations;
};

struct constants {
	double hgcoef, ss4o3, qstop, monoq_max_slope, monoq_limiter_mult, qlc_monoq,
				 qqc_monoq, qqc, eosvmax, eosvmin, pmin, emin,	dvovmax, refdens;
} constants;

struct cutoffs {
	double e, p, q, v, u;
} cutoffs;

struct domain {
	double * node_mass;
	double * element_mass;							// Remains constant
	double * element_initial_volume;
} domain;

struct mesh {
	int number_nodes;
	int number_elements;
	int ** nodes_node_neighbors; 				// 6 * number_nodes
	int ** nodes_element_neighbors;			// 8 * number_nodes
	int ** elements_node_neighbors;			// 8 * number_elements
	int ** elements_element_neighbors;	// 6 * number_elements
} mesh;


/**
 * Initialization, initializes the grid
 */
void initialize_domain(int size);
void run_lulesh(int domain_size, int maximum_iterations);

void free_memory();
/**
 * Computational steps
 */
void step_compute_force(int iteration, int element_id);
void step_compute_hourglass(int iteration, int element_id);

void step_reduce_force(int iteration, int node_id);
void step_compute_velocity(int iteration, int node_id);
void step_compute_position(int iteration, int node_id);

void step_compute_volume(int iteration, int element_id);
void step_compute_volume_derivative(int iteration, int element_id);
void step_compute_element_characteristic_length(int iteration, int element_id);

void step_compute_gradients(int iteration, int element_id);

void step_compute_viscosity_terms(int iteration, int element_id);
void step_compute_energy(int iteration, int element_id);

void step_compute_time_constraints(int iteration, int element_id);
void step_compute_delta_time(int iteration);

/**
 * Getters, will be replaced by CnC get calls in CnC versions
 */
vector get_force(int iteration, int mapped_node_id);
vector get_hourglass(int iteration, int mapped_node_id);
vector get_force_prime(int iteration, int node_id);

double get_delta_time(int iteration);
double get_elapsed_time(int iteration);
double get_hydro(int iteration, int element_id);
double get_courant(int iteration, int element_id);

double get_energy(int iteration, int element_id); 			// e
double get_pressure(int iteration, int element_id); 		// p
double get_viscosity(int iteration, int element_id);		// q
double get_volume(int iteration, int element_id);				// v
double get_sound_speed(int iteration, int element_id); 	// ss
double get_element_charastic_length(int iteration, int element_id);
double get_volume_derivative(int iteration, int element_id);

double get_linear_viscosity_term(int iteration, int element_id);
double get_quadratic_viscosity_term(int iteration, int element_id);

struct vertex get_position(int iteration, int node_id);
struct vector get_velocity(int iteration, int node_id);

struct vector get_position_gradient(int iteration, int element_id);
struct vector get_velocity_gradient(int iteration, int element_id);

/**
 * Helper functions
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

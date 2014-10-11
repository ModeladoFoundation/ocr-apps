#include <cnc/cnc.h>
#include <cnc/debug.h>

using namespace CnC;

struct vector;
struct vertex;

struct lulesh_context;

typedef std::pair<int,int> pair;

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


/******************************************************************************
 * Lulesh steps
 */

struct compute_stress_partial {
	int execute(const pair & t, lulesh_context & c) const;
};
struct compute_hourglass_partial {
	int execute(const pair & t, lulesh_context & c) const;
};
struct reduce_force {
	int execute(const pair & t, lulesh_context & c) const;
};

struct compute_velocity {
	int execute(const pair & t, lulesh_context & c) const;
};
struct compute_position {
	int execute(const pair & t, lulesh_context & c) const;
};

struct compute_volume {
	int execute(const pair & t, lulesh_context & c) const;
};

struct compute_volume_derivative {
	int execute(const pair & t, lulesh_context & c) const;
};
struct compute_characteristic_length {
	int execute(const pair & t, lulesh_context & c) const;
};
struct compute_gradients {
	int execute(const pair & t, lulesh_context & c) const;
};
struct compute_viscosity_terms {
	int execute(const pair & t, lulesh_context & c) const;
};
struct compute_energy {
	int execute(const pair & t, lulesh_context & c) const;
};

struct compute_time_constraints {
	int execute(const pair & t, lulesh_context & c) const;
};
struct compute_delta_time {
	int execute(const int & t, lulesh_context & c) const;
};

struct update_position
{
	// Iteration and node #
	int execute(const pair & t, lulesh_context & c) const;
};
struct compute_force
{
	// Iteration and element #
	int execute(const pair & t, lulesh_context & c) const;
};
struct calculate_energy
{
	// Iteration and element #
	int execute(const pair & t, lulesh_context & c) const;
};
struct calculate_dt
{
	// Iteration
	int execute(const int & t, lulesh_context & c) const;
};


struct single_use : public CnC::hashmap_tuner
{
    // provide number gets to each item
    int get_count( const int & tag ) const;
    int get_count( const pair & tag ) const;
};

struct double_use : public CnC::hashmap_tuner
{
    // provide number gets to each item
    int get_count( const int & tag ) const;
    int get_count( const pair & tag ) const;
};


/******************************************************************************
 * CnC context
 */
struct lulesh_context : public context< lulesh_context >

{
	step_collection < compute_delta_time > step_compute_delta_time;

	step_collection < compute_stress_partial > step_compute_stress_partial;
	step_collection < compute_hourglass_partial > step_compute_hourglass_partial;
	step_collection < reduce_force > step_reduce_force;

	step_collection < compute_velocity > step_compute_velocity;
	step_collection < compute_position > step_compute_position;
	step_collection < compute_volume > step_compute_volume;

	step_collection < compute_gradients > step_compute_gradients;
	step_collection < compute_volume_derivative > step_compute_volume_derivative;
	step_collection < compute_viscosity_terms > step_compute_viscosity_terms;
	step_collection < compute_energy > step_compute_energy;

	step_collection < compute_characteristic_length >
										step_compute_characteristic_length;

	step_collection < compute_time_constraints > step_compute_time_constraints;

	// item collections

	// per node per element
	item_collection< pair, vector, single_use > stress_partial;
	item_collection< pair, vector, single_use > hourglass_partial;

	// per node items
	item_collection< pair, vector, double_use > force;
	item_collection< pair, vertex > position;
	item_collection< pair, vector > velocity;

	// per element items
	item_collection< pair, double > volume;

	item_collection< pair, double, double_use > volume_derivative;
	item_collection< pair, double, single_use > characteristic_length;

	item_collection< pair, vector > velocity_gradient;
	item_collection< pair, vector > position_gradient;

	item_collection< pair, double, single_use > quadratic_viscosity_term;
	item_collection< pair, double, single_use > linear_viscosity_term;

	item_collection< pair, double, double_use > sound_speed;
	item_collection< pair, double, double_use > viscosity;
	item_collection< pair, double, double_use > pressure;
	item_collection< pair, double > energy;

	item_collection< pair, double, single_use > courant;
	item_collection< pair, double, single_use > hydro;

	// per iteration
	item_collection< int, double > delta_time;
	item_collection< int, double, single_use > elapsed_time;

	// We will need access to a few globals
	struct constraints constraints;
	struct constants constants;
	struct cutoffs cutoffs;
	struct domain domain;
	struct mesh mesh;

	// tag collections
	tag_collection< pair > iteration_node; // per iteration per node
	tag_collection< pair > iteration_element; // per iteration per element
	tag_collection< int > iteration; // per iteration

	lulesh_context() : context <lulesh_context>(),

			step_compute_delta_time( *this, "compute delta time"),

			step_compute_stress_partial( *this, "compute stress force partial"),
			step_compute_hourglass_partial( *this, "compute hourglass force partial"),
			step_reduce_force(*this, "combine stress and hourglass forces for nodes"),

			step_compute_velocity( *this, "compute velocity for nodes"),
			step_compute_position( *this, "compute position for nodes"),

			step_compute_volume( *this, "compute volume for elements"),

			step_compute_gradients( *this, "compute gradients for elements"),
			step_compute_volume_derivative( *this, "compute volume derivative"),
			step_compute_viscosity_terms( *this, "compute viscosity for elements"),
			step_compute_energy( *this, "compute energy for elements"),

			step_compute_characteristic_length(*this,"compute characteristic length"),
			step_compute_time_constraints( *this, "compute time constraints"),

			// Initialize each item collection
			stress_partial( *this, "stress force partial" ),
			hourglass_partial( *this, "hourglass force partial"),
			force( *this, "nodal force"),

			position( *this, "nodal position" ),
			velocity( *this, "nodal velocity" ),

			volume( *this, "element volume" ),

			volume_derivative( *this, "element volume derivative" ),
			characteristic_length( *this, "element characteristic length" ),

			velocity_gradient( *this, "element velocity gradient" ),
			position_gradient( *this, "element position gradient" ),

			quadratic_viscosity_term( *this, "element quadratic viscosity term" ),
			linear_viscosity_term( *this, "element linear viscosity term" ),

			sound_speed( *this, "element sound speed" ),
			viscosity( *this, "element viscosity" ),
			pressure( *this, "element pressure" ),
			energy( *this, "element energy" ),

			courant( *this, "element courant" ),
			hydro( *this, "element hydro" ),

			delta_time( *this, "delta time" ),
			elapsed_time( *this, "elapsed time" ),

			iteration( *this, "per iteration" ),
			iteration_node( *this, "per iteration per node" ),
			iteration_element( *this, "per iteration per element" )
			{

				step_compute_delta_time.controls(iteration);
				step_compute_delta_time.controls(iteration_node);
				step_compute_delta_time.controls(iteration_element);

				iteration.prescribes(step_compute_delta_time, *this );

				iteration_node.prescribes( step_reduce_force, *this );
				iteration_node.prescribes( step_compute_velocity, *this );
				iteration_node.prescribes( step_compute_position, *this );

				iteration_element.prescribes( step_compute_stress_partial, *this );
				iteration_element.prescribes( step_compute_hourglass_partial, *this );
				iteration_element.prescribes( step_compute_volume, *this );
				iteration_element.prescribes( step_compute_volume_derivative, *this );
				iteration_element.prescribes( step_compute_characteristic_length,*this);
				iteration_element.prescribes( step_compute_viscosity_terms, *this );
				iteration_element.prescribes( step_compute_energy, *this );
				iteration_element.prescribes( step_compute_gradients, *this);
				iteration_element.prescribes( step_compute_time_constraints, *this );

				// who consumes what...


				step_compute_delta_time.consumes(elapsed_time);
				step_compute_delta_time.consumes(delta_time);

				step_compute_delta_time.consumes(courant);
				step_compute_delta_time.consumes(hydro);

				step_compute_stress_partial.consumes(position);
				step_compute_stress_partial.consumes(pressure);
				step_compute_stress_partial.consumes(viscosity);

				step_compute_hourglass_partial.consumes(position);
				step_compute_hourglass_partial.consumes(velocity);
				step_compute_hourglass_partial.consumes(volume);
				step_compute_hourglass_partial.consumes(sound_speed);

				step_reduce_force.consumes(stress_partial);
				step_reduce_force.consumes(hourglass_partial);

				step_compute_velocity.consumes(delta_time);
				step_compute_velocity.consumes(force);
				step_compute_velocity.consumes(velocity);

				step_compute_position.consumes(delta_time);
				step_compute_position.consumes(velocity);
				step_compute_position.consumes(position);

				step_compute_volume.consumes(position);

				step_compute_gradients.consumes(volume);
				step_compute_gradients.consumes(position);
				step_compute_gradients.consumes(velocity);

				step_compute_volume_derivative.consumes(delta_time);
				step_compute_volume_derivative.consumes(position);
				step_compute_volume_derivative.consumes(velocity);

				step_compute_viscosity_terms.consumes(volume);
				step_compute_viscosity_terms.consumes(volume_derivative);
				step_compute_viscosity_terms.consumes(position_gradient);
				step_compute_viscosity_terms.consumes(velocity_gradient);

				step_compute_energy.consumes(volume);
				step_compute_energy.consumes(energy);
				step_compute_energy.consumes(pressure);
				step_compute_energy.consumes(viscosity);
				step_compute_energy.consumes(linear_viscosity_term);
				step_compute_energy.consumes(quadratic_viscosity_term);

				step_compute_characteristic_length.consumes(volume);
				step_compute_characteristic_length.consumes(position);

				step_compute_time_constraints.consumes(sound_speed);
				step_compute_time_constraints.consumes(volume_derivative);
				step_compute_time_constraints.consumes(characteristic_length);


				// who produces what
				step_compute_delta_time.produces(delta_time);
				step_compute_delta_time.produces(elapsed_time);

				step_compute_stress_partial.produces(stress_partial);
				step_compute_hourglass_partial.produces(hourglass_partial);
				step_reduce_force.produces(force);

				step_compute_velocity.produces(velocity);
				step_compute_position.produces(position);
				step_compute_volume.produces(volume);

				step_compute_gradients.produces(position_gradient);
				step_compute_gradients.produces(velocity_gradient);

				step_compute_volume_derivative.produces(volume_derivative);

				step_compute_viscosity_terms.produces(linear_viscosity_term);
				step_compute_viscosity_terms.produces(quadratic_viscosity_term);

				step_compute_energy.produces(energy);
				step_compute_energy.produces(pressure);
				step_compute_energy.produces(viscosity);
				step_compute_energy.produces(sound_speed);

				step_compute_characteristic_length.produces(characteristic_length);

				step_compute_time_constraints.produces(courant);
				step_compute_time_constraints.produces(hydro);

			}

};


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



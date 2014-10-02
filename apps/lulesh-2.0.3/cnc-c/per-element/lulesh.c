#include "lulesh.h"

// global constants
struct constraints constraints;
struct constants constants;
struct cutoffs cutoffs;
struct domain domain;
struct mesh mesh;

// item collections in CnC version
vector * forces_prime;
vector * force_map;
vector * hourglass_map;

vertex * node_vertices;
vector * node_velocites;
vector * velocity_gradients;
vector * position_gradients;
double ** element_volumes;
double * element_volume_derivatives;
double * element_characteristic_lengths;
double * quadratic_viscosity_term;
double * linear_viscosity_term;
double * element_sound_speed;
double * element_viscosity;
double * element_pressure;
double * element_energy;
double * element_courant;
double * element_hydro;

double global_delta_time;
double global_elapsed_time;

int main(int argc, char **args) {

	// -i (maximum iterations) -s (size)

//	run_lulesh(1, 9999999);
//	run_lulesh(2, 9999999);
//	run_lulesh(3, 9999999);
	run_lulesh(4, 9999999);
//	run_lulesh(16, 9999999);
//	run_lulesh(32, 9999999);
//	run_lulesh(64, 9999999);

}

void run_lulesh(int domain_size, int maximum_iterations) {

	initialize_domain(domain_size);

	int element_id, node_id;

	int iteration = 0;
	while((global_elapsed_time < constraints.stop_time) &&
			(iteration < maximum_iterations)) {

		// Increments the iteration
		step_compute_delta_time(iteration);

	//	printf("iteration %d, delta time %f, energy %f\n", (iteration+1),
	//					global_delta_time, get_energy(iteration, 0));

		// Compute force
		for(element_id = 0; element_id < mesh.number_elements; ++element_id) {
			step_compute_force(iteration, element_id);
		}

		for(element_id = 0; element_id < mesh.number_elements; ++element_id) {
			step_compute_hourglass(iteration, element_id);
		}

		for(node_id = 0; node_id < mesh.number_nodes; node_id++) {
		 	step_reduce_force(iteration, node_id);
		}

		// Update position
		for(node_id = 0; node_id < mesh.number_nodes; node_id++) {
			step_compute_velocity(iteration, node_id);
		}

		for(node_id = 0; node_id < mesh.number_nodes; node_id++) {
			step_compute_position(iteration, node_id);
		}

		// Volume
		for(element_id = 0; element_id < mesh.number_elements; ++element_id) {
			step_compute_volume(iteration, element_id);
		}

		// Energy
		for(element_id = 0; element_id < mesh.number_elements; ++element_id) {
			step_compute_element_characteristic_length(iteration, element_id);
		}

		for(element_id = 0; element_id < mesh.number_elements; ++element_id) {
			step_compute_volume_derivative(iteration, element_id);
		}

		for(element_id = 0; element_id < mesh.number_elements; ++element_id) {
			step_compute_gradients(iteration, element_id);
		}

		for(element_id = 0; element_id < mesh.number_elements; ++element_id) {
			step_compute_viscosity_terms(iteration, element_id);
		}

		for(element_id = 0; element_id < mesh.number_elements; ++element_id) {
			step_compute_energy(iteration, element_id);
		}

		for(element_id = 0; element_id < mesh.number_elements; ++element_id) {
			step_compute_time_constraints(iteration, element_id);
		}

		iteration++;

	}

	printf("\nDomain Size = %d\n", domain_size);
	printf("Final Origin Energy =  %2.6e\n", get_energy(iteration, 0));

	// Free allocated memory
	free_memory();

}


/******************************************************************************
 * step calculations
 */

void step_compute_force(int iteration, int element_id) {

	// [IntegrateStressForElems]

	// Consume
	double pressure = get_pressure(iteration, element_id);
	double viscosity = get_viscosity(iteration, element_id);

	int local_node_id;
	vertex node_vertices[8];
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
		int node_id = mesh.elements_node_neighbors[element_id][local_node_id];
		node_vertices[local_node_id] = get_position(iteration - 1, node_id);
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

	// Produce
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
			int node_id = mesh.elements_node_neighbors[element_id][local_node_id];
			// From the perspective of the node
			int local_element_id;
			for(local_element_id = 0; local_element_id < 8; local_element_id++) {
				if(mesh.nodes_element_neighbors[node_id][local_element_id]==element_id){
					// Then this is where we want to store the force value
					int map_id = (node_id << 3) | local_element_id;
					force_map[map_id] = forces_out[local_node_id];
				}
			}
	}
}

void step_compute_hourglass(int iteration, int element_id) {

	// Consume: volume, sound speed, neighbor vertices and velocities
	double element_volume = get_volume(iteration, element_id);
	double sound_speed = get_sound_speed(iteration, element_id);

	int local_node_id;
	vertex node_vertices[8];
	vector node_velocites[8];
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
		int node_id = mesh.elements_node_neighbors[element_id][local_node_id];
		node_vertices[local_node_id] = get_position(iteration, node_id);
		node_velocites[local_node_id] = get_velocity(iteration, node_id);
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


	double volo = domain.element_initial_volume[element_id];

	double determ_value = element_volume * volo;

	double volinv = 1.0 / determ_value;
	double coefficient = - constants.hgcoef * 0.01 * sound_speed *
			domain.element_mass[element_id] / cbrt(determ_value);

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

	// Produce
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
		int node_id = mesh.elements_node_neighbors[element_id][local_node_id];
		// From the perspective of the node
		int local_element_id;
		for(local_element_id = 0; local_element_id < 8; local_element_id++) {
			if(mesh.nodes_element_neighbors[node_id][local_element_id]==element_id){
				// Then this is where we want to store the force value
				int map_id = (node_id << 3) | local_element_id;
				hourglass_map[map_id] = forces_out[local_node_id];
			}
		}
	}
}

void step_reduce_force(int iteration, int node_id) {

	// Consume
	vector force_prime = vector_new(0.0, 0.0, 0.0);

	int local_element_id;
	for(local_element_id = 0; local_element_id < 8; local_element_id++) {
		int element_id = mesh.nodes_element_neighbors[node_id][local_element_id];
		if(element_id < 0) // If we are on a boundary
			continue;

		int map_id = (node_id << 3) | local_element_id;

		// Reduce
		vector hourglass = get_hourglass(iteration, map_id);
		vector force = get_force(iteration, map_id);

		force_prime.x += force.x + hourglass.x;
		force_prime.y += force.y + hourglass.y;
		force_prime.z += force.z + hourglass.z;
	}

	// Produce
	forces_prime[node_id] = force_prime;
}

void step_compute_velocity(int iteration, int node_id) {
	// Consume
	double delta_time = get_delta_time(iteration);
	vector force = get_force_prime(iteration, node_id);
	vector previous_velocity = get_velocity(iteration-1, node_id);

	// Compute
	vector acceleration = divide(force, domain.node_mass[node_id]);

	if(mesh.nodes_node_neighbors[node_id][0] == -2) acceleration.x = 0;
	if(mesh.nodes_node_neighbors[node_id][2] == -2) acceleration.y = 0;
	if(mesh.nodes_node_neighbors[node_id][4] == -2) acceleration.z = 0;

	vector velocity = vector_add(previous_velocity,
										mult(acceleration, delta_time));
	if( fabs(velocity.x) < cutoffs.u ) velocity.x = 0.0;
	if( fabs(velocity.y) < cutoffs.u ) velocity.y = 0.0;
	if( fabs(velocity.z) < cutoffs.u ) velocity.z = 0.0;

	// Produce
	node_velocites[node_id] = velocity;
}

void step_compute_position(int iteration, int node_id) {
	// Consume
	double delta_time = get_delta_time(iteration);
	vector velocity = get_velocity(iteration, node_id);
	vertex previous_position = get_position(iteration - 1, node_id);

	// Compute
	vertex new_position = move(previous_position, mult(velocity, delta_time));

	// Produce
	node_vertices[node_id] = new_position;
}

void step_compute_volume(int iteration, int element_id) {

	// Consume
	vertex c[] = {
			get_position(iteration, mesh.elements_node_neighbors[element_id][0]),
			get_position(iteration, mesh.elements_node_neighbors[element_id][1]),
			get_position(iteration, mesh.elements_node_neighbors[element_id][2]),
			get_position(iteration, mesh.elements_node_neighbors[element_id][3]),
			get_position(iteration, mesh.elements_node_neighbors[element_id][4]),
			get_position(iteration, mesh.elements_node_neighbors[element_id][5]),
			get_position(iteration, mesh.elements_node_neighbors[element_id][6]),
			get_position(iteration, mesh.elements_node_neighbors[element_id][7]) };

	// Constants
	double initial_volume = domain.element_initial_volume[element_id];

	// Compute
	double volume = (
			dot(cross(vertex_sub(c[6], c[3]), vertex_sub(c[2], c[0])),
					vector_add(vertex_sub(c[3], c[1]), vertex_sub(c[7], c[2]))) +
			dot(cross(vertex_sub(c[6], c[4]), vertex_sub(c[7], c[0])),
					vector_add(vertex_sub(c[4], c[3]), vertex_sub(c[5], c[7]))) +
			dot(cross(vertex_sub(c[6], c[1]), vertex_sub(c[5], c[0])),
					vector_add(vertex_sub(c[1], c[4]),vertex_sub(c[2], c[5])))) *
			(1.0/12.0);

	double relative_volume = volume / initial_volume;

	volume = fabs(relative_volume - 1.0) < cutoffs.v ? 1.0 : relative_volume;

	// Check for negative volumes
	if (volume <= 0.0) {
		exit(1);
	}

	// Produce

	element_volumes[(iteration+1)%2][element_id] = volume;

}

void step_compute_element_characteristic_length(int iteration, int element_id) {

	// Consume: volume, vertices (node neighbors)
	double volume = get_volume(iteration+1, element_id);

	int local_node_id;
	vertex node_vertices[8];
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
		int node_id = mesh.elements_node_neighbors[element_id][local_node_id];
		node_vertices[local_node_id] = get_position(iteration, node_id);
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

	volume = volume * domain.element_initial_volume[element_id];

	double element_charastic_length = 4.0 * volume / sqrt(char_length);

	// Produce
	element_characteristic_lengths[element_id] = element_charastic_length;

}

void step_compute_volume_derivative(int iteration, int element_id) {

	// Consume
	double delta_time = get_delta_time(iteration);

	int local_node_id;
	vertex node_vertices[8];
	vector node_velocites[8];
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
		int node_id = mesh.elements_node_neighbors[element_id][local_node_id];
		node_vertices[local_node_id] = get_position(iteration, node_id);
		node_velocites[local_node_id] = get_velocity(iteration, node_id);
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

	double element_volume_derivative = dxx + dyy + dzz;

	// Produce

	element_volume_derivatives[element_id] = element_volume_derivative;

}

void step_compute_gradients(int iteration, int element_id) {

	// Consume: volume, velocity (node neighbors), vertices (node neighbors)

	// [CalcQForElems]
	double volume = get_volume(iteration+1, element_id);


	int local_node_id;
	vertex node_vertices[8];
	vector node_velocites[8];
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
		int node_id = mesh.elements_node_neighbors[element_id][local_node_id];
		node_vertices[local_node_id] = get_position(iteration, node_id);
		node_velocites[local_node_id] = get_velocity(iteration, node_id);
	}

	// Constants
	double ptiny = 1.e-36;
	double initial_volume = domain.element_initial_volume[element_id];

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


	// Produce

	position_gradients[element_id] = (vector) {delx_xi, delx_eta, delx_zeta};
	velocity_gradients[element_id] = (vector) {delv_xi, delv_eta, delv_zeta};

}

void step_compute_viscosity_terms(int iteration, int element_id) {

	// Consume
	double volume = get_volume(iteration+1, element_id);

	vector velocity_gradient = get_velocity_gradient(iteration, element_id);
	vector position_gradient = get_position_gradient(iteration, element_id);
	double volume_derivative = get_volume_derivative(iteration, element_id);

	int local_element_id;
	vector velocity_gradients[6];
	for(local_element_id = 0; local_element_id < 6; local_element_id++) {
		int neighbor_element_id =
				mesh.elements_element_neighbors[element_id][local_element_id];
		velocity_gradients[local_element_id] =
				get_velocity_gradient(iteration, neighbor_element_id);
	}

	// Constants
	double ptiny = 1.e-36;
	double mass = domain.element_mass[element_id];
	double volo = domain.element_initial_volume[element_id];
	double monoq_limiter_mult = constants.monoq_limiter_mult;
	double monoq_max_slope = constants.monoq_max_slope;
	double qlc_monoq = constants.qlc_monoq;
	double qqc_monoq = constants.qqc_monoq;

	// Compute

	// [CalcMonotonicQRegionForElems]
	double norm = 1.0 / (velocity_gradient.x + ptiny);
	double qlin, qquad;

	if (volume_derivative > 0.0) {
			qlin  = 0.0;
			qquad = 0.0;
	} else {
		double rho = mass / (volo * volume);
		double phixi, phieta, phizeta ;
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
			if(mesh.elements_element_neighbors[element_id][face_id] >= 0) {
				if(face_id == 4 || face_id == 5) {
					temp_gradients[face_id] = velocity_gradients[face_id].z;
				} else if(face_id == 0 || face_id == 1) {
					temp_gradients[face_id] = velocity_gradients[face_id].x;
				} else if(face_id == 2 || face_id == 3) {
					temp_gradients[face_id] = velocity_gradients[face_id].y;
				}
			} else if(mesh.elements_element_neighbors[element_id][face_id] == -2) {
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

		qlin = -qlc_monoq * rho *
				(delvx.x * (1.0 - phi.x) +
				 delvx.y * (1.0 - phi.y) +
				 delvx.z * (1.0 - phi.z) );

		qquad = qqc_monoq * rho *
			 (delvx.x * delvx.x * (1.0 - phi.x * phi.x) +
				delvx.y * delvx.y * (1.0 - phi.y * phi.y) +
				delvx.z * delvx.z * (1.0 - phi.z * phi.z) );

	}

	// Produce
	linear_viscosity_term[element_id] = qlin;
	quadratic_viscosity_term[element_id] = qquad;

}

void step_compute_energy(int iteration, int element_id) {

	// Consume
	double previous_volume = get_volume(iteration, element_id);
	double volume = get_volume(iteration+1, element_id);

	double previous_energy = get_energy(iteration, element_id);
	double previous_pressure = get_pressure(iteration, element_id);
	double previous_viscosity = get_viscosity(iteration, element_id);

	double qlin = get_linear_viscosity_term(iteration, element_id);
	double qquad = get_quadratic_viscosity_term(iteration, element_id);


	// Constants
	double eosvmin = constants.eosvmin;
	double eosvmax = constants.eosvmax;
	double emin = constants.emin;
	double pmin = constants.pmin;
	double rho0 = constants.refdens;

	double delv = volume - previous_volume;

	// [ApplyMaterialPropertiesForElems]

	// Bound the updated and previous relative volumes with eosvmin/max
	if (eosvmin != 0.0) {
		if (volume < eosvmin) {
			volume = eosvmin;
		} if (previous_volume < eosvmin)
			previous_volume = eosvmin;
	}

	if (eosvmax != 0.0) {
		if (volume > eosvmax) {
			volume = eosvmax;
		}
		if (previous_volume > eosvmax)
			previous_volume = eosvmax;
	}

	// [EvalEOSForElems]
	double compression = 1.0 / volume - 1.;
	double vchalf = volume - delv * .5;
	double comp_half_step = 1.0 / vchalf - 1.0;
	double c1s = 2.0/3.0;
	double work = 0.0;
	const double sixth = 1.0 / 6.0;
	double pressure, viscosity, q_tilde;

	// Check for v > eosvmax or v < eosvmin
	if (eosvmin != 0.0 ) {
		if (volume <= eosvmin) {
			comp_half_step = compression; // impossible due to calling func?
		}
	}
	if (eosvmax != 0.0) {
		if (volume >= eosvmax) { // impossible due to calling func?
			previous_pressure        = 0.0;
			compression  = 0.0;
			comp_half_step = 0.0;
		}
	}

	double energy = previous_energy - 0.5 * delv *
			(previous_pressure + previous_viscosity) + 0.5 * work;

	if (energy  < emin) {
		energy = emin ;
	}

	// [CalcPressureForElems]
	double bvc = c1s * (comp_half_step + 1.0);
	double p_half_step = bvc * energy ;

	if(fabs(p_half_step) <  cutoffs.p)
		p_half_step = 0.0;
	if(volume >= eosvmax) // impossible condition here?
		p_half_step = 0.0;
	if(p_half_step < pmin)
		p_half_step = pmin;

	double vhalf = 1.0 / (1.0 + comp_half_step);
	if (delv > 0.0) {
		viscosity = 0.0; // = qq_old = ql_old
	} else {
		double ssc = ( c1s * energy + vhalf * vhalf * bvc * p_half_step ) / rho0;
		if (ssc <= .1111111e-36) {
			ssc = .3333333e-18;
		} else {
			ssc = sqrt(ssc);
		}
		viscosity = (ssc*qlin + qquad) ;
	}


	energy = energy + 0.5 * delv * (3.0 * (previous_pressure + previous_viscosity)
			- 4.0 * (p_half_step + viscosity));

	energy += 0.5 * work;

	if (fabs(energy) < cutoffs.e) {
		energy = 0.0;
	}
	if(energy  < emin ) {
		energy = emin ;
	}

	// [CalcPressureForElems]
	bvc = c1s * (compression + 1.0);
	pressure = bvc * energy;

	if(fabs(pressure) <  cutoffs.p)
		pressure = 0.0;
	if(volume >= eosvmax ) // impossible condition here?
		pressure = 0.0;
	if(pressure < pmin)
		pressure = pmin;

	if (delv > 0.0) {
		q_tilde = 0.0;
	} else {
		double ssc = ( c1s * energy + volume * volume * bvc * pressure ) / rho0;
		if (ssc <= .1111111e-36) {
			ssc = .3333333e-18;
		} else {
			ssc = sqrt(ssc) ;
		}
		q_tilde = (ssc*qlin + qquad) ;
	}

	energy = energy - (7.0 * (previous_pressure + previous_viscosity) - 8.0 *
			(p_half_step + viscosity) + (pressure + q_tilde)) * delv * sixth;

	if (fabs(energy) < cutoffs.e) {
		energy = 0.0;
	}
	if (energy  < emin) {
		energy = emin ;
	}

	// [CalcPressureForElems]
	bvc = c1s * (compression + 1.0);
	pressure = bvc * energy;

	if(fabs(pressure) <  cutoffs.p)
		pressure = 0.0;

	if(volume >= eosvmax) // impossible condition here?
		pressure = 0.0;
	if(pressure < pmin)
		pressure = pmin;

	if (delv <= 0.0) {
		double ssc = (c1s * energy + volume * volume * bvc * pressure ) / rho0 ;
		if (ssc <= .1111111e-36) {
			ssc = .3333333e-18;
		} else {
			ssc = sqrt(ssc);
		}
		viscosity = (ssc*qlin + qquad) ;
		if (fabs(viscosity) < cutoffs.q) viscosity = 0.0;
	}

	double sound_speed = (c1s * energy + volume * volume * bvc * pressure) / rho0;
	if (sound_speed <= .1111111e-36) {
		sound_speed = .3333333e-18;
	} else {
		sound_speed = sqrt(sound_speed);
	}

	// produce

	element_sound_speed[element_id] = sound_speed;
	element_viscosity[element_id] = viscosity;
	element_pressure[element_id] = pressure;
	element_energy[element_id] = energy;

}

void step_compute_time_constraints(int iteration, int element_id) {

	// Consume
	double sound_speed = get_sound_speed(iteration, element_id);
	double volume_derivative = get_volume_derivative(iteration, element_id);
	double element_charastic_length =
			get_element_charastic_length(iteration, element_id);

	// Constants
	double qqc = constants.qqc;
	double dvovmax = constants.dvovmax;

	// Compute

	// [CalcTimeConstraintsForElems]
	double qqc2 = 64.0 * qqc * qqc;
	double dtcourant = 1.0e+20;
	double dthydro = 1.0e+20;
	double dtf = sound_speed * sound_speed;

	if (volume_derivative < 0.0) {
		dtf = dtf + qqc2 * volume_derivative * volume_derivative *
				element_charastic_length * element_charastic_length;
	}

	dtf = sqrt(dtf) ;
	dtf = element_charastic_length / dtf ;

	if (volume_derivative != 0.0) {
		dtcourant = dtf;
	}

	// [CalcHydroConstraintForElems]
	if (volume_derivative != 0.0) {
		dthydro = dvovmax / (fabs(volume_derivative)+1.e-20);
	}

	// Produce

	element_courant[element_id] = dtcourant;
	element_hydro[element_id] = dthydro;

}

void step_compute_delta_time(int iteration) {

	// Consume
	double previous_delta_time = get_delta_time(iteration);
	double elapsed_time = get_elapsed_time(iteration);

	double min_courant = 1e20;
	double min_hydro = 1e20;

	int element_id;
	if(iteration != 0) {
		for(element_id = 0; element_id < mesh.number_elements; ++element_id) {
			double courant = get_courant(iteration, element_id);
			double hydro = get_hydro(iteration, element_id);
			if(courant < min_courant)
				min_courant = courant;
			if(hydro < min_hydro)
				min_hydro = hydro;
		}
	}

	// Constants
	double stop_time = constraints.stop_time;
	double max_delta_time = constraints.max_delta_time;

	// Compute
	double delta_time = 0;
	double dtfixed = -1.0e-6; // *edgeElems*tp/45.0) ;

	double deltatimemultlb = 1.1;
	double deltatimemultub = 1.2;

	double targetdt = stop_time - elapsed_time;

	if ((dtfixed <= 0.0) && (iteration != 0)) {
		double ratio;

		// This will require a reduction in parallel
		double gnewdt = 1.0e+20;

		if (min_courant < gnewdt)
			gnewdt = min_courant / 2.0;

		if (min_hydro < gnewdt)
			gnewdt = min_hydro * 2.0 / 3.0;

		delta_time = gnewdt;
		ratio = delta_time / previous_delta_time;

		if (ratio >= 1.0) {
			if (ratio < deltatimemultlb) {
				delta_time = previous_delta_time ;
			} else if (ratio > deltatimemultub) {
				delta_time= previous_delta_time * deltatimemultub;
			}
		}

		if (delta_time > max_delta_time) {
			delta_time = max_delta_time;
		}

	} else {
		delta_time = previous_delta_time;
	}

	// TRY TO PREVENT VERY SMALL SCALING ON THE NEXT CYCLE
	if ((targetdt > delta_time) && (targetdt < (4.0 * delta_time / 3.0)) )
		targetdt = 2.0 * delta_time / 3.0;

	if (targetdt < delta_time)
		delta_time = targetdt ;

	elapsed_time += delta_time;

	// Produce
	global_delta_time = delta_time;
	global_elapsed_time = elapsed_time;

}

/******************************************************************************
 * initialization
 */

void initialize_domain(int mesh_size) {

	// Initialize the domain constants, these will be used throughout the code
	constants = (struct constants) {
		3.0, 4.0/3.0, 1.0e+12, 1.0, 2.0, 0.5, 2.0/3.0,
		2.0, 1.0e+9, 1.0e-9, 0.0, -1.0e+15, 0.1, 1.0};

	cutoffs = (struct cutoffs) {
		1.0e-7, 1.0e-7, 1.0e-7, 1.0e-10, 1.0e-7 };

	constraints = (struct constraints) {
		1.0e-2, 1.0e-2, 9999999 };

	// Initialize the mesh (regular cubes, delta spaced)
	int edge_elements = mesh_size;
	int edge_nodes = mesh_size + 1;

	int row_id, column_id, plane_id;
	int i, element_id, node_id = 0;

	// Initialize the structure; keeps track of size and neighbors
	mesh.number_nodes = edge_nodes * edge_nodes * edge_nodes;
	mesh.number_elements = edge_elements * edge_elements * edge_elements;

	mesh.nodes_node_neighbors = malloc(sizeof(int *)*mesh.number_nodes);
	mesh.nodes_element_neighbors = malloc(sizeof(int *)*mesh.number_nodes);
	mesh.elements_node_neighbors = malloc(sizeof(int *)*mesh.number_elements);
	mesh.elements_element_neighbors = malloc(sizeof(int *)*mesh.number_elements);

	// Temps: Remove for CnC Lulesh
	node_vertices = malloc(sizeof(vector)*mesh.number_nodes);
	node_velocites = malloc(sizeof(vector)*mesh.number_nodes);
	element_volumes = malloc(sizeof(int *)*2);
	element_volumes[0] = malloc(sizeof(vector)*mesh.number_nodes);
	element_volumes[1] = malloc(sizeof(vector)*mesh.number_nodes);
	element_volume_derivatives = malloc(sizeof(vector)*mesh.number_nodes);
	forces_prime = malloc(sizeof(vector)*mesh.number_nodes);
	force_map = malloc(sizeof(vector)*mesh.number_nodes*8);
	hourglass_map = malloc(sizeof(vector)*mesh.number_nodes*8);
	velocity_gradients = malloc(sizeof(vector)*mesh.number_elements);
	position_gradients = malloc(sizeof(vector)*mesh.number_elements);
	element_characteristic_lengths = malloc(sizeof(double)*mesh.number_elements);
	linear_viscosity_term = malloc(sizeof(double)*mesh.number_elements);
	quadratic_viscosity_term = malloc(sizeof(double)*mesh.number_elements);
	element_sound_speed = malloc(sizeof(double)*mesh.number_elements);
	element_viscosity = malloc(sizeof(double)*mesh.number_elements);
	element_pressure = malloc(sizeof(double)*mesh.number_elements);
	element_energy = malloc(sizeof(double)*mesh.number_elements);
	element_courant = malloc(sizeof(double)*mesh.number_elements);
	element_hydro = malloc(sizeof(double)*mesh.number_elements);

	for(node_id = 0; node_id < mesh.number_nodes; node_id++) {
		mesh.nodes_node_neighbors[node_id] = malloc(sizeof(int)*6);
		mesh.nodes_element_neighbors[node_id] = malloc(sizeof(int)*8);
	}

	for(element_id = 0; element_id < mesh.number_elements; element_id++) {
		mesh.elements_node_neighbors[element_id] = malloc(sizeof(int)*8);
		mesh.elements_element_neighbors[element_id] = malloc(sizeof(int)*6);
	}

	domain.node_mass = calloc(mesh.number_nodes,sizeof(double));
	domain.element_mass = malloc(sizeof(double)*mesh.number_elements);
	domain.element_initial_volume = malloc(sizeof(double)*mesh.number_elements);

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

				node_vertices[node_id] = vertex_new(x, y, z);
				forces_prime[node_id] = vector_new(0.0, 0.0, 0.0);
				node_velocites[node_id] = vector_new(0.0, 0.0, 0.0);

				mesh.nodes_node_neighbors[node_id][0] =
						(column_id-1) < 0 ? -2 : (node_id - 1);
				mesh.nodes_node_neighbors[node_id][1] =
						(column_id+1) >= edge_nodes ? -1 : (node_id + 1);
				mesh.nodes_node_neighbors[node_id][2] =
						(row_id-1) < 0 ? -2 : node_id - edge_nodes;
				mesh.nodes_node_neighbors[node_id][3] =
						(row_id+1) >= edge_nodes ? -1 : node_id + edge_nodes;
				mesh.nodes_node_neighbors[node_id][4] =
						(plane_id-1) < 0 ? -2 : node_id - edge_nodes * edge_nodes;
				mesh.nodes_node_neighbors[node_id][5] =
						(plane_id+1) >= edge_nodes ? -1 : node_id + (edge_nodes*edge_nodes);

				mesh.nodes_element_neighbors[node_id][0] = -2;
				mesh.nodes_element_neighbors[node_id][1] = -1;
				mesh.nodes_element_neighbors[node_id][2] = -2;
				mesh.nodes_element_neighbors[node_id][3] = -1;
				mesh.nodes_element_neighbors[node_id][4] = -2;
				mesh.nodes_element_neighbors[node_id][5] = -1;
				mesh.nodes_element_neighbors[node_id][6] = -2;
				mesh.nodes_element_neighbors[node_id][7] = -1;

				node_id++;
			}
		}
	}

	// Setup elements node and element neighbors
	node_id = 0;
	element_id = 0;
	for (plane_id = 0; plane_id < edge_elements; ++plane_id) {
		for (row_id = 0; row_id < edge_elements; ++row_id) {
			for (column_id = 0; column_id < edge_elements; ++column_id) {

				mesh.elements_node_neighbors[element_id][0] =
						node_id;
				mesh.elements_node_neighbors[element_id][1] =
						node_id + 1;
				mesh.elements_node_neighbors[element_id][2] =
						node_id + edge_nodes + 1;
				mesh.elements_node_neighbors[element_id][3] =
						node_id + edge_nodes;
				mesh.elements_node_neighbors[element_id][4] =
						node_id + edge_nodes * edge_nodes;
				mesh.elements_node_neighbors[element_id][5] =
						node_id + edge_nodes * edge_nodes + 1;
				mesh.elements_node_neighbors[element_id][6] =
						node_id + edge_nodes * edge_nodes + edge_nodes + 1;
				mesh.elements_node_neighbors[element_id][7] =
						node_id + edge_nodes * edge_nodes + edge_nodes;

				mesh.nodes_element_neighbors[
				    mesh.elements_node_neighbors[element_id][0]][6] = element_id;
				mesh.nodes_element_neighbors[
				    mesh.elements_node_neighbors[element_id][1]][7] = element_id;
				mesh.nodes_element_neighbors[
				    mesh.elements_node_neighbors[element_id][2]][4] = element_id;
				mesh.nodes_element_neighbors[
				    mesh.elements_node_neighbors[element_id][3]][5] = element_id;
				mesh.nodes_element_neighbors[
				    mesh.elements_node_neighbors[element_id][4]][2] = element_id;
				mesh.nodes_element_neighbors[
				    mesh.elements_node_neighbors[element_id][5]][3] = element_id;
				mesh.nodes_element_neighbors[
				    mesh.elements_node_neighbors[element_id][6]][0] = element_id;
				mesh.nodes_element_neighbors[
				    mesh.elements_node_neighbors[element_id][7]][1] = element_id;

				mesh.elements_element_neighbors[element_id][0] =
						(column_id-1) < 0 ? -2 : (element_id - 1);
				mesh.elements_element_neighbors[element_id][1] =
						(column_id+1) >= edge_elements ? -1 : (element_id + 1);
				mesh.elements_element_neighbors[element_id][2] =
						(row_id-1) < 0 ? -2 : element_id - edge_elements;
				mesh.elements_element_neighbors[element_id][3] =
						(row_id+1) >= edge_elements ? -1 : element_id + edge_elements;
				mesh.elements_element_neighbors[element_id][4] =
						(plane_id-1) < 0 ? -2 : element_id - edge_elements * edge_elements;
				mesh.elements_element_neighbors[element_id][5] =
						(plane_id+1) >= edge_elements ? -1 : element_id +
						(edge_elements*edge_elements);

				vertex c[] = {
						get_position(0, mesh.elements_node_neighbors[element_id][0]),
						get_position(0, mesh.elements_node_neighbors[element_id][1]),
						get_position(0, mesh.elements_node_neighbors[element_id][2]),
						get_position(0, mesh.elements_node_neighbors[element_id][3]),
						get_position(0, mesh.elements_node_neighbors[element_id][4]),
						get_position(0, mesh.elements_node_neighbors[element_id][5]),
						get_position(0, mesh.elements_node_neighbors[element_id][6]),
						get_position(0, mesh.elements_node_neighbors[element_id][7]) };

				// Compute
				double volume = (
						dot(cross(vertex_sub(c[6], c[3]), vertex_sub(c[2], c[0])),
								vector_add(vertex_sub(c[3], c[1]), vertex_sub(c[7], c[2]))) +
						dot(cross(vertex_sub(c[6], c[4]), vertex_sub(c[7], c[0])),
								vector_add(vertex_sub(c[4], c[3]), vertex_sub(c[5], c[7]))) +
						dot(cross(vertex_sub(c[6], c[1]), vertex_sub(c[5], c[0])),
								vector_add(vertex_sub(c[1], c[4]),vertex_sub(c[2], c[5])))) *
						(1.0/12.0);

				double initial_volume = volume;
				element_volumes[0][element_id] = 1.0;
				element_volumes[1][element_id] = 1.0;
				domain.element_initial_volume[element_id] = initial_volume;
				domain.element_mass[element_id] = initial_volume;

				element_viscosity[element_id] = 0.0;
				element_pressure[element_id] = 0.0;
				element_energy[element_id] = element_id == 0.0 ? einit : 0.0;
				element_sound_speed[element_id] = 0.0;

				double node_v = initial_volume/8.0;
				domain.node_mass[mesh.elements_node_neighbors[element_id][0]] += node_v;
				domain.node_mass[mesh.elements_node_neighbors[element_id][1]] += node_v;
				domain.node_mass[mesh.elements_node_neighbors[element_id][2]] += node_v;
				domain.node_mass[mesh.elements_node_neighbors[element_id][3]] += node_v;
				domain.node_mass[mesh.elements_node_neighbors[element_id][4]] += node_v;
				domain.node_mass[mesh.elements_node_neighbors[element_id][5]] += node_v;
				domain.node_mass[mesh.elements_node_neighbors[element_id][6]] += node_v;
				domain.node_mass[mesh.elements_node_neighbors[element_id][7]] += node_v;

				element_id++;
				node_id++;
			}
			node_id++;
		}
		node_id += edge_nodes;
	}

	// Initialize delta time
	global_delta_time =0.5*cbrt(domain.element_initial_volume[0])/sqrt(2.0*einit);
	global_elapsed_time = 0.0;
}

/******************************************************************************
 * cleanup
 */

void free_memory() {
	int element_id, node_id;

	for(element_id = 0; element_id < mesh.number_elements; element_id++) {
		free(mesh.elements_node_neighbors[element_id]);
		free(mesh.elements_element_neighbors[element_id]);
	}

	for(node_id = 0; node_id < mesh.number_nodes; node_id++) {
		free(mesh.nodes_node_neighbors[node_id]);
		free(mesh.nodes_element_neighbors[node_id]);
	}

	free(mesh.nodes_node_neighbors);
	free(mesh.nodes_element_neighbors);
	free(mesh.elements_node_neighbors);
	free(mesh.elements_element_neighbors);

	free(domain.node_mass);
	free(domain.element_mass);
	free(domain.element_initial_volume);

	free(linear_viscosity_term);
	free(quadratic_viscosity_term);
	free(element_characteristic_lengths);
	free(element_volume_derivatives);
	free(element_volumes[0]);
	free(element_volumes[1]);
	free(velocity_gradients);
	free(position_gradients);
	free(node_vertices);
	free(node_velocites);
	free(forces_prime);
	free(force_map);
	free(hourglass_map);

	free(element_sound_speed);
	free(element_viscosity);
	free(element_pressure);
	free(element_energy);
	free(element_courant);
	free(element_hydro);
}

/******************************************************************************
 * getters, will be replaced with CnC get calls in CnC versions
 */
vector get_force_prime(int iteration, int node_id) {
	return forces_prime[node_id];
}

vector get_force(int iteration, int mapped_node_id) {
	return force_map[mapped_node_id];
}

vector get_hourglass(int iteration, int mapped_node_id) {
	return hourglass_map[mapped_node_id];
}

double get_elapsed_time(int iteration) {
	return global_elapsed_time;
}

double get_delta_time(int iteration) {
	return global_delta_time;
}

double get_hydro(int iteration, int element_id) {
	return element_hydro[element_id];
}

double get_courant(int iteration, int element_id) {
	return element_courant[element_id];
}

double get_energy(int iteration, int element_id) {
	return element_energy[element_id];
}

double get_pressure(int iteration, int element_id) {
	return element_pressure[element_id];
}

double get_viscosity(int iteration, int element_id) {
	return element_viscosity[element_id];
}

double get_sound_speed(int iteration, int element_id) {
	return element_sound_speed[element_id];
}

double get_volume(int iteration, int element_id) {
	return element_volumes[abs(iteration%2)][element_id];
}

struct vector get_position_gradient(int iteration, int element_id) {
	return position_gradients[element_id];
}

struct vector get_velocity_gradient(int iteration, int element_id) {
	return velocity_gradients[element_id];
}

double get_linear_viscosity_term(int iteration, int element_id) {
	return linear_viscosity_term[element_id];
}

double get_quadratic_viscosity_term(int iteration, int element_id) {
	return quadratic_viscosity_term[element_id];
}

double get_volume_derivative(int iteration, int element_id) {
	return element_volume_derivatives[element_id];
}

double get_element_charastic_length(int iteration, int element_id) {
	return element_characteristic_lengths[element_id];
}

vertex get_position(int iteration, int node_id) {
	return node_vertices[node_id];
}

vector get_velocity(int iteration, int node_id) {
	return node_velocites[node_id];
}

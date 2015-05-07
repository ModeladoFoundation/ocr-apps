#include "lulesh.h"

/******************************************************************************
 * tuner
 */
int single_use::get_count( const int & tag ) const
{
    return 1;
}

int single_use::get_count( const pair & tag ) const
{
    return 1;
}

int double_use::get_count( const int & tag ) const
{
    return 2;
}

int double_use::get_count( const pair & tag ) const
{
    return 2;
}

/******************************************************************************
 * step calculations
 */

int compute_stress_partial::execute(const pair & t, lulesh_context & c) const {

	int iteration = t.first;
	int element_id = t.second;

	// Consume
	double pressure, viscosity;
	c.pressure.get(pair(iteration - 1, element_id), pressure);
	c.viscosity.get(pair(iteration - 1, element_id), viscosity);

	int local_node_id;
	vertex node_vertices[8];
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
		int node_id = c.mesh.elements_node_neighbors[element_id][local_node_id];
		c.position.get(pair(iteration - 1, node_id), node_vertices[local_node_id]);
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
		int node_id = c.mesh.elements_node_neighbors[element_id][local_node_id];
		// From the perspective of the node
		int local_element_id;
		for(local_element_id = 0; local_element_id < 8; local_element_id++) {
			if(c.mesh.nodes_element_neighbors[node_id][local_element_id]==element_id){
				// Then this is where we want to store the force value
				int map_id = (node_id << 3) | local_element_id;
				c.stress_partial.put(pair(iteration, map_id),forces_out[local_node_id]);
			}
		}
	}

	return CnC::CNC_Success;
}

int compute_hourglass_partial::execute(const pair & t,lulesh_context & c)const {

	int iteration = t.first;
	int element_id = t.second;
	// Consume

	double element_volume, sound_speed;
	c.volume.get(pair(iteration - 1, element_id), element_volume);
	c.sound_speed.get(pair(iteration - 1, element_id), sound_speed);

	int local_node_id;
	vertex node_vertices[8];
	vector node_velocites[8];
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
		int node_id = c.mesh.elements_node_neighbors[element_id][local_node_id];
		c.position.get(pair(iteration - 1, node_id), node_vertices[local_node_id]);
		c.velocity.get(pair(iteration - 1, node_id), node_velocites[local_node_id]);
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

	double volo = c.domain.element_initial_volume[element_id];

	double determ_value = element_volume * volo;

	double volinv = 1.0 / determ_value;
	double coefficient = - c.constants.hgcoef * 0.01 * sound_speed *
			c.domain.element_mass[element_id] / cbrt(determ_value);

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
		int node_id = c.mesh.elements_node_neighbors[element_id][local_node_id];
		// From the perspective of the node
		int local_element_id;
		for(local_element_id = 0; local_element_id < 8; local_element_id++) {
			if(c.mesh.nodes_element_neighbors[node_id][local_element_id]==element_id){
				// Then this is where we want to store the force value
				int map_id = (node_id << 3) | local_element_id;
				c.hourglass_partial.put(
						pair(iteration,map_id), forces_out[local_node_id]);
			}
		}
	}

	return CnC::CNC_Success;
}

int reduce_force::execute(const pair & t, lulesh_context & c) const {

	int iteration = t.first;
	int node_id = t.second;

	vector force = vector_new(0.0, 0.0, 0.0);

	// Consume
	int local_element_id;
	for(local_element_id = 0; local_element_id < 8; local_element_id++) {
		int element_id = c.mesh.nodes_element_neighbors[node_id][local_element_id];
		if(element_id < 0) // If we are on a boundary
			continue;

		int map_id = (node_id << 3) | local_element_id;

		// Reduce
		vector stress_partial, hourglass_partial;
		c.stress_partial.get(pair(iteration, map_id), stress_partial);
		c.hourglass_partial.get(pair(iteration, map_id), hourglass_partial);

		force.x += stress_partial.x + hourglass_partial.x;
		force.y += stress_partial.y + hourglass_partial.y;
		force.z += stress_partial.z + hourglass_partial.z;
	}

	// Produce
	c.force.put(pair(iteration, node_id), force);

	return CnC::CNC_Success;
}

int compute_velocity::execute(const pair & t, lulesh_context & c) const {

	int iteration = t.first;
	int node_id = t.second;
	// Consume

	double delta_time;
	vector force, previous_velocity;

	c.delta_time.get(iteration, delta_time);
	c.force.get(pair(iteration, node_id), force);
	c.velocity.get(pair(iteration - 1, node_id), previous_velocity);

	// Compute
	vector acceleration = divide(force, c.domain.node_mass[node_id]);

	if(c.mesh.nodes_node_neighbors[node_id][0] == -2) acceleration.x = 0;
	if(c.mesh.nodes_node_neighbors[node_id][2] == -2) acceleration.y = 0;
	if(c.mesh.nodes_node_neighbors[node_id][4] == -2) acceleration.z = 0;

	vector velocity = vector_add(previous_velocity,
			mult(acceleration, delta_time));
	if( fabs(velocity.x) < c.cutoffs.u ) velocity.x = 0.0;
	if( fabs(velocity.y) < c.cutoffs.u ) velocity.y = 0.0;
	if( fabs(velocity.z) < c.cutoffs.u ) velocity.z = 0.0;

	// Produce
	c.velocity.put(pair(iteration, node_id), velocity);

	return CnC::CNC_Success;
}

int compute_position::execute(const pair & t, lulesh_context & c) const {

	int iteration = t.first;
	int node_id = t.second;

	// Consume
	double delta_time;
	vector velocity;
	vertex previous_position;

	c.delta_time.get(iteration, delta_time);
	c.velocity.get(pair(iteration, node_id), velocity);
	c.position.get(pair(iteration - 1, node_id), previous_position);

	// Compute// Compute
	vertex new_position = move(previous_position, mult(velocity, delta_time));

	// Produce
	c.position.put(pair(iteration, node_id), new_position);

	return CnC::CNC_Success;
}

int compute_volume::execute(const pair & t, lulesh_context & c) const {

	int iteration = t.first;
	int element_id = t.second;

	// Consume
	int local_node_id;
	vertex node_vertices[8];
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
		int node_id = c.mesh.elements_node_neighbors[element_id][local_node_id];
		c.position.get(pair(iteration, node_id), node_vertices[local_node_id]);
	}

	// Constants
	double initial_volume = c.domain.element_initial_volume[element_id];

	// Compute
	double volume = (
			dot(cross( vertex_sub(node_vertices[6], node_vertices[3]),
								 vertex_sub(node_vertices[2], node_vertices[0])),
			vector_add(vertex_sub(node_vertices[3], node_vertices[1]),
								 vertex_sub(node_vertices[7], node_vertices[2]))) +
			dot(cross( vertex_sub(node_vertices[6], node_vertices[4]),
								 vertex_sub(node_vertices[7], node_vertices[0])),
			vector_add(vertex_sub(node_vertices[4], node_vertices[3]),
								 vertex_sub(node_vertices[5], node_vertices[7]))) +
			dot(cross( vertex_sub(node_vertices[6], node_vertices[1]),
								 vertex_sub(node_vertices[5], node_vertices[0])),
			vector_add(vertex_sub(node_vertices[1], node_vertices[4]),
								 vertex_sub(node_vertices[2], node_vertices[5])))) * (1.0/12.0);

	double relative_volume = volume / initial_volume;

	volume = fabs(relative_volume - 1.0) < c.cutoffs.v ? 1.0 : relative_volume;

	// Check for negative volumes
	if (volume <= 0.0) {
		exit(1);
	}

	// Produce
	c.volume.put(pair(iteration, element_id), volume);

	return CnC::CNC_Success;
}

int compute_volume_derivative::execute(const pair & t, lulesh_context & c)const{

	int iteration = t.first;
	int element_id = t.second;
	// Consume
	double delta_time;
	c.delta_time.get(iteration, delta_time);

	int local_node_id;
	vertex node_vertices[8];
	vector node_velocites[8];
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
		int node_id = c.mesh.elements_node_neighbors[element_id][local_node_id];
		c.position.get(pair(iteration, node_id), node_vertices[local_node_id]);
		c.velocity.get(pair(iteration, node_id), node_velocites[local_node_id]);
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
	c.volume_derivative.put(pair(iteration,element_id),element_volume_derivative);

	return CnC::CNC_Success;
}

int compute_gradients::execute(const pair & t, lulesh_context & c) const {

	int iteration = t.first;
	int element_id = t.second;

	double volume;
	// Consume
	c.volume.get(pair(iteration, element_id), volume);

	int local_node_id;
	vertex node_vertices[8];
	vector node_velocites[8];
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
		int node_id = c.mesh.elements_node_neighbors[element_id][local_node_id];
		c.position.get(pair(iteration, node_id), node_vertices[local_node_id]);
		c.velocity.get(pair(iteration, node_id), node_velocites[local_node_id]);
	}

	// Compute
	double ptiny = 1.e-36;
	double initial_volume = c.domain.element_initial_volume[element_id];

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

	// Produce
	c.position_gradient.put(pair(iteration, element_id), position_gradient);
	c.velocity_gradient.put(pair(iteration, element_id), velocity_gradient);

	return CnC::CNC_Success;
}

int compute_viscosity_terms::execute(const pair & t, lulesh_context & c) const {

	int iteration = t.first;
	int element_id = t.second;

	// Consume
	double volume, volume_derivative;
	c.volume.get(pair(iteration, element_id), volume);
	c.volume_derivative.get(pair(iteration, element_id), volume_derivative);

	vector velocity_gradient, position_gradient;
	c.position_gradient.get(pair(iteration, element_id), position_gradient);
	c.velocity_gradient.get(pair(iteration, element_id), velocity_gradient);

	int local_element_id;
	vector velocity_gradients[6];
	for(local_element_id = 0; local_element_id < 6; local_element_id++) {
		int neighbor_element_id =
				c.mesh.elements_element_neighbors[element_id][local_element_id];
		if(neighbor_element_id == -2) {
			velocity_gradients[local_element_id] = vector_new(0.0, 0.0, 0.0);
		} else if(neighbor_element_id == -1) {
			velocity_gradients[local_element_id] = velocity_gradient;
		} else {
			c.velocity_gradient.get(pair(iteration, neighbor_element_id),
					velocity_gradients[local_element_id]);
		}
	}

	// Constants
	double ptiny = 1.e-36;
	double mass = c.domain.element_mass[element_id];
	double volo = c.domain.element_initial_volume[element_id];
	double monoq_limiter_mult = c.constants.monoq_limiter_mult;
	double monoq_max_slope = c.constants.monoq_max_slope;
	double qlc_monoq = c.constants.qlc_monoq;
	double qqc_monoq = c.constants.qqc_monoq;

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
			if(c.mesh.elements_element_neighbors[element_id][face_id] >= 0) {
				if(face_id == 4 || face_id == 5) {
					temp_gradients[face_id] = velocity_gradients[face_id].z;
				} else if(face_id == 0 || face_id == 1) {
					temp_gradients[face_id] = velocity_gradients[face_id].x;
				} else if(face_id == 2 || face_id == 3) {
					temp_gradients[face_id] = velocity_gradients[face_id].y;
				}
			} else if(c.mesh.elements_element_neighbors[element_id][face_id] == -2) {
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
	c.linear_viscosity_term.put(pair(iteration, element_id), qlin);
	c.quadratic_viscosity_term.put(pair(iteration, element_id), qquad);

	return CnC::CNC_Success;
}

int compute_energy::execute(const pair & t, lulesh_context & c) const {

	int iteration = t.first;
	int element_id = t.second;

	// Consume
	double previous_volume, volume;
	c.volume.get(pair(iteration - 1, element_id), previous_volume);
	c.volume.get(pair(iteration, element_id), volume);

	double previous_energy, previous_pressure, previous_viscosity;
	c.energy.get(pair(iteration - 1, element_id), previous_energy);
	c.pressure.get(pair(iteration - 1, element_id), previous_pressure);
	c.viscosity.get(pair(iteration - 1, element_id), previous_viscosity);

	double qlin, qquad;
	c.linear_viscosity_term.get(pair(iteration, element_id), qlin);
	c.quadratic_viscosity_term.get(pair(iteration, element_id), qquad);

	// Constants
	double eosvmin = c.constants.eosvmin;
	double eosvmax = c.constants.eosvmax;
	double emin = c.constants.emin;
	double pmin = c.constants.pmin;
	double rho0 = c.constants.refdens;

	// Compute

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

	if(fabs(p_half_step) <  c.cutoffs.p)
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

	if (fabs(energy) < c.cutoffs.e) {
		energy = 0.0;
	}
	if(energy  < emin ) {
		energy = emin ;
	}

	// [CalcPressureForElems]
	bvc = c1s * (compression + 1.0);
	pressure = bvc * energy;

	if(fabs(pressure) <  c.cutoffs.p)
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

	if (fabs(energy) < c.cutoffs.e) {
		energy = 0.0;
	}
	if (energy  < emin) {
		energy = emin ;
	}

	// [CalcPressureForElems]
	bvc = c1s * (compression + 1.0);
	pressure = bvc * energy;

	if(fabs(pressure) <  c.cutoffs.p)
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
		if (fabs(viscosity) < c.cutoffs.q) viscosity = 0.0;
	}

	double sound_speed = (c1s * energy + volume * volume * bvc * pressure) / rho0;
	if (sound_speed <= .1111111e-36) {
		sound_speed = .3333333e-18;
	} else {
		sound_speed = sqrt(sound_speed);
	}

	// Produce
	c.energy.put(pair(iteration, element_id), energy);
	c.pressure.put(pair(iteration, element_id), pressure);
	c.viscosity.put(pair(iteration, element_id), viscosity);
	c.sound_speed.put(pair(iteration, element_id), sound_speed);

	return CnC::CNC_Success;
}

int compute_characteristic_length::execute(
		const pair & t, lulesh_context & c) const {

	int iteration = t.first;
	int element_id = t.second;

	double volume;
	c.volume.get(pair(iteration, element_id), volume);

	int local_node_id;
	vertex node_vertices[8];
	for(local_node_id = 0; local_node_id < 8; local_node_id++) {
		int node_id = c.mesh.elements_node_neighbors[element_id][local_node_id];
		c.position.get(pair(iteration, node_id), node_vertices[local_node_id]);
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

	volume = volume * c.domain.element_initial_volume[element_id];

	double characteristic_length = 4.0 * volume / sqrt(char_length);

	// Produce
	c.characteristic_length.put(pair(iteration,element_id),characteristic_length);

	return CnC::CNC_Success;
}

int compute_time_constraints::execute(const pair & t, lulesh_context & c)const {

	int iteration = t.first;
	int element_id = t.second;

	// Consume
	double sound_speed, volume_derivative, characteristic_length;
	c.sound_speed.get(pair(iteration, element_id), sound_speed);
	c.volume_derivative.get(pair(iteration, element_id), volume_derivative);
	c.characteristic_length.get(pair(iteration,element_id),characteristic_length);

	// Constants
	double qqc = c.constants.qqc;
	double dvovmax = c.constants.dvovmax;

	// Compute
	// [CalcTimeConstraintsForElems]
	double qqc2 = 64.0 * qqc * qqc;
	double dtcourant = 1.0e+20;
	double dthydro = 1.0e+20;
	double dtf = sound_speed * sound_speed;

	if (volume_derivative < 0.0) {
		dtf = dtf + qqc2 * volume_derivative * volume_derivative *
				characteristic_length * characteristic_length;
	}

	dtf = sqrt(dtf) ;
	dtf = characteristic_length / dtf ;

	if (volume_derivative != 0.0) {
		dtcourant = dtf;
	}

	// [CalcHydroConstraintForElems]
	if (volume_derivative != 0.0) {
		dthydro = dvovmax / (fabs(volume_derivative)+1.e-20);
	}

	// Produce
	c.courant.put(pair(iteration, element_id), dtcourant);
	c.hydro.put(pair(iteration, element_id), dthydro);

	return CnC::CNC_Success;
}

int compute_delta_time::execute(const int & t, lulesh_context & c) const {

	int iteration = t;

	// Consume
	double previous_delta_time, previous_elapsed_time;
	c.delta_time.get(iteration, previous_delta_time);
	c.elapsed_time.get(iteration, elapsed_time);


	double min_courant = 1e20;
	double min_hydro = 1e20;


	if(iteration != 0) {
		int element_id;
		if(iteration != 0) {
			for(element_id = 0; element_id < c.mesh.number_elements; ++element_id) {
				double courant, hydro;
				c.courant.get(pair(iteration, element_id), courant);
				c.hydro.get(pair(iteration, element_id), hydro);
				if(courant < min_courant)
					min_courant = courant;
				if(hydro < min_hydro)
					min_hydro = hydro;
			}
		}
	}

	// Constants
	double stop_time = c.constraints.stop_time;
	double max_delta_time = c.constraints.max_delta_time;

	// Compute
	double delta_time = 0;
	double dtfixed = -1.0e-6; // *edgeElems*tp/45.0) ;

	double deltatimemultlb = 1.1;
	double deltatimemultub = 1.2;

	double targetdt = stop_time - previous_elapsed_time;

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

	if(elapsed_time < c.constraints.stop_time &&
			(iteration + 1) < c.constraints.maximum_iterations) {

		double elapsed_time = previous_elapsed_time + delta_time;

		// Produce
		c.delta_time.put(iteration + 1, delta_time);
		c.elapsed_time.put(iteration + 1, elapsed_time);

		int node_id;
		for(node_id = 0; node_id < c.mesh.number_nodes; ++node_id ) {
			c.iteration_node.put( pair(iteration + 1, node_id) );
		}

		int element_id;
		for(element_id = 0; element_id < c.mesh.number_elements; ++element_id ) {
			c.iteration_element.put( pair(iteration + 1, element_id) );
		}

		c.iteration.put(iteration + 1);
	}

	return CnC::CNC_Success;
}

/******************************************************************************
 * initialization
 */

int main( int argc, char* argv[] )
{
	int mesh_size = 4;
	int max_iterations = 50;

	lulesh_context cnc = lulesh_context();

//	CnC::debug::trace(cnc.iteration);

	// Initialize the domain constants, these will be used throughout the code
	cnc.constants = (struct constants) {
		3.0, 4.0/3.0, 1.0e+12, 1.0, 2.0, 0.5, 2.0/3.0,
		2.0, 1.0e+9, 1.0e-9, 0.0, -1.0e+15, 0.1, 1.0};

	cnc.cutoffs = (struct cutoffs) {
		1.0e-7, 1.0e-7, 1.0e-7, 1.0e-10, 1.0e-7 };

	cnc.constraints = (struct constraints) {
		1.0e-2, 1.0e-2, max_iterations };

	// Initialize the mesh (regular cubes, delta spaced)
	int edge_elements = mesh_size;
	int edge_nodes = mesh_size + 1;

	int row_id, column_id, plane_id;
	int i, element_id, node_id = 0;

	// Initialize the structure; keeps track of size and neighbors
	cnc.mesh.number_nodes = edge_nodes * edge_nodes * edge_nodes;
	cnc.mesh.number_elements = edge_elements * edge_elements * edge_elements;

	cnc.mesh.nodes_node_neighbors = (int **)
			malloc(sizeof(int *)*cnc.mesh.number_nodes);
	cnc.mesh.nodes_element_neighbors = (int **)
			malloc(sizeof(int *)*cnc.mesh.number_nodes);
	cnc.mesh.elements_node_neighbors = (int **)
			malloc(sizeof(int *)*cnc.mesh.number_elements);
	cnc.mesh.elements_element_neighbors = (int **)
			malloc(sizeof(int *)*cnc.mesh.number_elements);

	for(node_id = 0; node_id < cnc.mesh.number_nodes; node_id++) {
		cnc.mesh.nodes_node_neighbors[node_id] = (int *)malloc(sizeof(int)*6);
		cnc.mesh.nodes_element_neighbors[node_id] = (int *)malloc(sizeof(int)*8);
	}

	for(element_id = 0; element_id < cnc.mesh.number_elements; element_id++) {
		cnc.mesh.elements_node_neighbors[element_id] = (int*) malloc(sizeof(int)*8);
		cnc.mesh.elements_element_neighbors[element_id]=(int*)malloc(sizeof(int)*6);
	}

	cnc.domain.node_mass = (double *)
			calloc(cnc.mesh.number_nodes,sizeof(double));
	cnc.domain.element_mass =(double*)
			malloc(sizeof(double)*cnc.mesh.number_elements);
	cnc.domain.element_initial_volume = (double *)
			malloc(sizeof(double)*cnc.mesh.number_elements);

	double scale = ((double)edge_elements)/45.0;
	double einit = 3.948746e+7*scale*scale*scale;

	vertex node_positions[cnc.mesh.number_nodes];

	// Setup initial vertices and nodes neighboring nodes
	node_id = 0;
	double delta = 1.125 / ((double) edge_elements);
	for (plane_id = 0; plane_id < edge_nodes; ++plane_id) {
		double z = delta * plane_id;
		for (row_id = 0; row_id < edge_nodes; ++row_id) {
			double y = delta * row_id;
			for (column_id = 0; column_id < edge_nodes; ++column_id) {
				double x = delta * column_id;

				vertex initial_position = vertex_new(x, y, z);
				node_positions[node_id] = initial_position;

				cnc.position.put(pair(0, node_id), initial_position);
				cnc.force.put(pair(0, node_id), vector_new(0.0, 0.0, 0.0));
				cnc.velocity.put(pair(0, node_id), vector_new(0.0, 0.0, 0.0));

				cnc.mesh.nodes_node_neighbors[node_id][0] =
						(column_id-1) < 0 ? -2 : (node_id - 1);
				cnc.mesh.nodes_node_neighbors[node_id][1] =
						(column_id+1) >= edge_nodes ? -1 : (node_id + 1);
				cnc.mesh.nodes_node_neighbors[node_id][2] =
						(row_id-1) < 0 ? -2 : node_id - edge_nodes;
				cnc.mesh.nodes_node_neighbors[node_id][3] =
						(row_id+1) >= edge_nodes ? -1 : node_id + edge_nodes;
				cnc.mesh.nodes_node_neighbors[node_id][4] =
						(plane_id-1) < 0 ? -2 : node_id - edge_nodes * edge_nodes;
				cnc.mesh.nodes_node_neighbors[node_id][5] =
						(plane_id+1) >= edge_nodes ? -1 : node_id + (edge_nodes*edge_nodes);

				cnc.mesh.nodes_element_neighbors[node_id][0] = -2;
				cnc.mesh.nodes_element_neighbors[node_id][1] = -1;
				cnc.mesh.nodes_element_neighbors[node_id][2] = -2;
				cnc.mesh.nodes_element_neighbors[node_id][3] = -1;
				cnc.mesh.nodes_element_neighbors[node_id][4] = -2;
				cnc.mesh.nodes_element_neighbors[node_id][5] = -1;
				cnc.mesh.nodes_element_neighbors[node_id][6] = -2;
				cnc.mesh.nodes_element_neighbors[node_id][7] = -1;

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

				cnc.mesh.elements_node_neighbors[element_id][0] =
						node_id;
				cnc.mesh.elements_node_neighbors[element_id][1] =
						node_id + 1;
				cnc.mesh.elements_node_neighbors[element_id][2] =
						node_id + edge_nodes + 1;
				cnc.mesh.elements_node_neighbors[element_id][3] =
						node_id + edge_nodes;
				cnc.mesh.elements_node_neighbors[element_id][4] =
						node_id + edge_nodes * edge_nodes;
				cnc.mesh.elements_node_neighbors[element_id][5] =
						node_id + edge_nodes * edge_nodes + 1;
				cnc.mesh.elements_node_neighbors[element_id][6] =
						node_id + edge_nodes * edge_nodes + edge_nodes + 1;
				cnc.mesh.elements_node_neighbors[element_id][7] =
						node_id + edge_nodes * edge_nodes + edge_nodes;

				cnc.mesh.nodes_element_neighbors[
				    cnc.mesh.elements_node_neighbors[element_id][0]][6] = element_id;
				cnc.mesh.nodes_element_neighbors[
				    cnc.mesh.elements_node_neighbors[element_id][1]][7] = element_id;
				cnc.mesh.nodes_element_neighbors[
				    cnc.mesh.elements_node_neighbors[element_id][2]][4] = element_id;
				cnc.mesh.nodes_element_neighbors[
				    cnc.mesh.elements_node_neighbors[element_id][3]][5] = element_id;
				cnc.mesh.nodes_element_neighbors[
				    cnc.mesh.elements_node_neighbors[element_id][4]][2] = element_id;
				cnc.mesh.nodes_element_neighbors[
				    cnc.mesh.elements_node_neighbors[element_id][5]][3] = element_id;
				cnc.mesh.nodes_element_neighbors[
				    cnc.mesh.elements_node_neighbors[element_id][6]][0] = element_id;
				cnc.mesh.nodes_element_neighbors[
				    cnc.mesh.elements_node_neighbors[element_id][7]][1] = element_id;

				cnc.mesh.elements_element_neighbors[element_id][0] =
						(column_id-1) < 0 ? -2 : (element_id - 1);
				cnc.mesh.elements_element_neighbors[element_id][1] =
						(column_id+1) >= edge_elements ? -1 : (element_id + 1);
				cnc.mesh.elements_element_neighbors[element_id][2] =
						(row_id-1) < 0 ? -2 : element_id - edge_elements;
				cnc.mesh.elements_element_neighbors[element_id][3] =
						(row_id+1) >= edge_elements ? -1 : element_id + edge_elements;
				cnc.mesh.elements_element_neighbors[element_id][4] =
						(plane_id-1) < 0 ? -2 : element_id - edge_elements * edge_elements;
				cnc.mesh.elements_element_neighbors[element_id][5] =
						(plane_id+1) >= edge_elements ? -1 : element_id +
						(edge_elements*edge_elements);

				vertex c[] = {
						node_positions[cnc.mesh.elements_node_neighbors[element_id][0]],
						node_positions[cnc.mesh.elements_node_neighbors[element_id][1]],
						node_positions[cnc.mesh.elements_node_neighbors[element_id][2]],
						node_positions[cnc.mesh.elements_node_neighbors[element_id][3]],
						node_positions[cnc.mesh.elements_node_neighbors[element_id][4]],
						node_positions[cnc.mesh.elements_node_neighbors[element_id][5]],
						node_positions[cnc.mesh.elements_node_neighbors[element_id][6]],
						node_positions[cnc.mesh.elements_node_neighbors[element_id][7]] };

				// Compute
				double volume = (
						dot(cross(vertex_sub(c[6], c[3]), vertex_sub(c[2], c[0])),
								vector_add(vertex_sub(c[3], c[1]), vertex_sub(c[7], c[2]))) +
						dot(cross(vertex_sub(c[6], c[4]), vertex_sub(c[7], c[0])),
								vector_add(vertex_sub(c[4], c[3]), vertex_sub(c[5], c[7]))) +
						dot(cross(vertex_sub(c[6], c[1]), vertex_sub(c[5], c[0])),
								vector_add(vertex_sub(c[1], c[4]),vertex_sub(c[2], c[5])))) *
						(1.0/12.0);

				cnc.domain.element_initial_volume[element_id] = volume;
				cnc.domain.element_mass[element_id] = volume;

				cnc.volume.put(pair(0, element_id), 1.0);
				cnc.viscosity.put(pair(0, element_id), 0.0);
				cnc.pressure.put(pair(0, element_id), 0.0);
				cnc.energy.put(pair(0, element_id), element_id == 0.0 ? einit : 0.0);
				cnc.sound_speed.put(pair(0, element_id), 0.0);

				double node_v = volume / 8.0;
				cnc.domain.node_mass[cnc.mesh.elements_node_neighbors[element_id][0]] += node_v;
				cnc.domain.node_mass[cnc.mesh.elements_node_neighbors[element_id][1]] += node_v;
				cnc.domain.node_mass[cnc.mesh.elements_node_neighbors[element_id][2]] += node_v;
				cnc.domain.node_mass[cnc.mesh.elements_node_neighbors[element_id][3]] += node_v;
				cnc.domain.node_mass[cnc.mesh.elements_node_neighbors[element_id][4]] += node_v;
				cnc.domain.node_mass[cnc.mesh.elements_node_neighbors[element_id][5]] += node_v;
				cnc.domain.node_mass[cnc.mesh.elements_node_neighbors[element_id][6]] += node_v;
				cnc.domain.node_mass[cnc.mesh.elements_node_neighbors[element_id][7]] += node_v;

				element_id++;
				node_id++;
			}
			node_id++;
		}
		node_id += edge_nodes;
	}

	double delta_time = 0.5*cbrt(cnc.domain.element_initial_volume[0])/sqrt(2.0*einit);
	double elapsed_time = 0.0;

	cnc.elapsed_time.put(0, elapsed_time);
	cnc.delta_time.put(0, delta_time);

	cnc.iteration.put(0);
	cnc.wait();

	double energy;
	for(int iteration = 0; iteration < max_iterations; iteration++) {
		// Need a better way to do this
		cnc.energy.get(pair(iteration, 0), energy); // Look for last energy
	}

	std::cout << std::scientific;
	std::cout << "Final origin energy " << energy << "\n";

	for(element_id = 0; element_id < cnc.mesh.number_elements; element_id++) {
		free(cnc.mesh.elements_node_neighbors[element_id]);
		free(cnc.mesh.elements_element_neighbors[element_id]);
	}

	for(node_id = 0; node_id < cnc.mesh.number_nodes; node_id++) {
		free(cnc.mesh.nodes_node_neighbors[node_id]);
		free(cnc.mesh.nodes_element_neighbors[node_id]);
	}

	free(cnc.mesh.nodes_node_neighbors);
	free(cnc.mesh.nodes_element_neighbors);
	free(cnc.mesh.elements_node_neighbors);
	free(cnc.mesh.elements_element_neighbors);

	free(cnc.domain.node_mass);
	free(cnc.domain.element_mass);
	free(cnc.domain.element_initial_volume);

}

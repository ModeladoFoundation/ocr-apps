////////////////////////////////////////////////////////////////////////////////
// Author: Ellen Porter (ellen.porter@pnnl.gov)
////////////////////////////////////////////////////////////////////////////////

// CnC Context - initialized in mainEdt
$context {
    struct constraints constraints;
    struct constants constants;
    struct cutoffs cutoffs;
    struct domain domain;
    struct mesh mesh;
};

////////////////////////////////////////////////////////////////////////////////
// item collection declarations

// Per node per element per iteration
[ vector stress_partial: iteration, map_id ];
[ vector hourglass_partial: iteration, map_id ];

// Per node per iteration
[ vector force: iteration, node_id ];
[ vertex position: iteration, node_id ];
[ vector velocity: iteration, node_id ];

// Per element per iteration
[ double volume: iteration, element_id ];
[ double volume_derivative: iteration, element_id ];
[ double characteristic_length: iteration, element_id ];
[ vector velocity_gradient: iteration, element_id ];
[ vector position_gradient: iteration, element_id ];
[ double quadratic_viscosity_term: iteration, element_id ];
[ double linear_viscosity_term: iteration, element_id ];
[ double sound_speed: iteration, element_id ];
[ double viscosity: iteration, element_id ];
[ double pressure: iteration, element_id ];
[ double energy: iteration, element_id ];
[ double courant: iteration, element_id ];
[ double hydro: iteration, element_id ];

// Per iteration
[ double delta_time: iteration ];
[ double elapsed_time: iteration ];

////////////////////////////////////////////////////////////////////////////////
//virtual item collection declarations
[ vector neighbor_velocity: element_id, local_node_id, iteration =
	velocity using
	get_elements_node_neighbors_for_velocity ];

[ vertex neighbor_position: element_id, local_node_id, iteration =
	position using
	get_elements_node_neighbors_for_position ];

[ vector neighbor_stress_partial: node_id, local_element_id, iteration =
	stress_partial using
	get_map_id_for_node_element_neighbors_for_stress_partial ];

[ vector neighbor_hourglass_partial: node_id, local_element_id, iteration =
	hourglass_partial using
	get_map_id_for_node_element_neighbors_for_hourglass_partial ];

[ vector neighbor_velocity_gradient: element_id, local_element_id, iteration =
	velocity_gradient using
	get_elements_element_neighbors_for_velocity_gradient ];

// Singleton for final origin energy
[ double final_origin_energy:  () ];

////////////////////////////////////////////////////////////////////////////////
// Control tags and steps

// < int [2] iteration_node > ;   	// per iteration per node
// < int [2] iteration_element > ; 	// per iteration per element
// < int [1] iteration > ;  				// per iteration

// < iteration > :: 	( step_compute_delta_time );

// < iteration_node > :: 	( step_reduce_force ),
//							( step_compute_velocity ),
//							( step_compute_position );
//
// < iteration_element > :: 	(step_compute_stress_partial ),
//								(step_compute_hourglass_partial),
//								(step_compute_volume),
//								(step_compute_volume_derivative),
//								(step_compute_characteristic_length),
//								(step_compute_viscosity_terms),
//								(step_compute_energy),
//								(step_compute_gradients),
//								(step_compute_time_constraints);

////////////////////////////////////////////////////////////////////////////////
// Input output relationships
( $init: () )
  		-> [ stress_partial: -1, -1 ],
  		   [ hourglass_partial: -1, -1 ],
  		   [ velocity_gradient: -1, -1 ],

		   [ force: 0, {0..NODES} ],
		   [ position: 0, {0..NODES} ],
		   [ velocity: 0, {0..NODES} ],

  		   [ volume: 0, {0..ELEMENTS} ],
  		   [ viscosity: 0, {0..ELEMENTS} ],
  		   [ pressure: 0, {0..ELEMENTS} ],
  		   [ energy: 0, {0..ELEMENTS} ],
  		   [ sound_speed: 0, {0..ELEMENTS} ],

  		   [ elapsed_time: 0 ],
  		   [ delta_time: 0 ];

( $finalize: () )
		<- [ final_origin_energy: () ];

////////////////////////////////////////////////////////////////////////////////
// CnC steps
( compute_stress_partial: iteration, element_id )
		<-	[ pressure_in @ pressure: iteration - 1, element_id ],
			[ viscosity_in @ viscosity: iteration - 1, element_id ],
			[ neighbor_position: element_id, {0..8}, iteration - 1 ]
		->	[ stress_partial_out @ stress_partial: iteration, {0..8} ];

( compute_hourglass_partial: iteration, element_id )
		<-	[ element_volume_in @ volume: iteration - 1, element_id ],
			[ sound_speed_in @ sound_speed: iteration - 1, element_id ],
			[ neighbor_position: element_id, {0..8}, iteration - 1 ],
			[ neighbor_velocity: element_id, {0..8}, iteration - 1 ]
		-> 	[ hourglass_partial_out @ hourglass_partial: iteration, {0..8} ];

( reduce_force: iteration, node_id )
		<-	[ neighbor_stress_partial: node_id, {0..8}, iteration ],
			[ neighbor_hourglass_partial: node_id, {0..8}, iteration ]
		->	[ force_out @ force: iteration, node_id ];

( compute_velocity: iteration, node_id )
		<- 	[ delta_time_in @ delta_time: iteration ],
			[ force_in @ force: iteration, node_id ],
			[ previous_velocity_in @ velocity: iteration - 1, node_id ]
		->	[ velocity_out @ velocity: iteration, node_id ];

( compute_position: iteration, node_id )
		<-	[ delta_time_in @ delta_time: iteration ],
			[ velocity_in @ velocity: iteration, node_id ],
			[ previous_position_in @ position: iteration - 1, node_id ]
		-> 	[ position_out @ position: iteration, node_id ];

( compute_volume: iteration, element_id )
		<-	[ neighbor_position: element_id, {0..8}, iteration ]
		->	[ volume_out @ volume: iteration, element_id ];

( compute_volume_derivative: iteration, element_id )
		<-	[ delta_time_in @ delta_time: iteration ],
			[ neighbor_position: element_id, {0..8}, iteration ],
			[ neighbor_velocity: element_id, {0..8}, iteration ]
		->	[ volume_derivative_out @ volume_derivative: iteration, element_id ];

( compute_gradients: iteration, element_id )
		<-	[ volume_in @ volume: iteration, element_id ],
			[ neighbor_position: element_id, {0..8}, iteration ],
			[ neighbor_velocity: element_id, {0..8}, iteration ]
		->	[ position_gradient_out @ position_gradient: iteration, element_id ],
			[ velocity_gradient_out @ velocity_gradient: iteration, element_id ];

( compute_viscosity_terms: iteration, element_id )
		<-	[ volume_in @ volume: iteration, element_id ],
			[ volume_derivative_in @ volume_derivative: iteration, element_id ],
			[ position_gradient_in @ position_gradient: iteration, element_id ],
			[ velocity_gradient_in @ velocity_gradient: iteration, element_id ],
			[ neighbor_velocity_gradient: element_id, {0..6}, iteration ]
		->	[ linear_viscosity_term_out @ linear_viscosity_term: iteration, element_id ],
			[ quadratic_viscosity_term_out @ quadratic_viscosity_term: iteration, element_id ];

( compute_energy: iteration, element_id )
		<-	[ volume_in @ volume: iteration, element_id ],
			[ previous_volume_in @ volume: iteration - 1, element_id ],
			[ previous_energy_in @ energy: iteration - 1, element_id ],
			[ previous_pressure_in @ pressure: iteration - 1, element_id ],
			[ previous_viscosity_in @ viscosity: iteration - 1, element_id ],
			[ qlin_in @ linear_viscosity_term: iteration, element_id ],
			[ qquad_in @ quadratic_viscosity_term: iteration, element_id ]
		->	[ energy_out @ energy: iteration, element_id ],
			[ pressure_out @ pressure: iteration, element_id ],
			[ viscosity_out @ viscosity: iteration, element_id ],
			[ sound_speed_out @ sound_speed: iteration, element_id ];

( compute_characteristic_length: iteration, element_id )
		<-	[ volume_in @ volume: iteration, element_id ],
			[ neighbor_position: element_id, {0..8}, iteration ]
		-> 	[ characteristic_length_out @ characteristic_length: iteration, element_id ];

( compute_time_constraints: iteration, element_id )
		<-	[ sound_speed_in @ sound_speed: iteration, element_id ],
			[ volume_derivative_in @ volume_derivative: iteration, element_id ],
			[ characteristic_length_in @ characteristic_length: iteration, element_id ]
		-> 	[ courant_out @ courant: iteration, element_id ],
			[ hydro_out @ hydro: iteration, element_id ];

( compute_delta_time: iteration )
		<-	[ previous_delta_time_in @ delta_time: iteration ],
			[ previous_elapsed_time_in @ elapsed_time: iteration ],
			[ courant_in @ courant: iteration, {0..ELEMENTS} ],
			[ hydro_in @ hydro: iteration, {0..ELEMENTS} ]
		->	[ delta_time_out @ delta_time: iteration + 1 ],
			[ elapsed_time_out @ elapsed_time: iteration + 1 ];

( produce_output : iteration )
		<- [ final_energy @ energy: iteration, 0 ]
		-> [ final_origin_energy: () ];

// NOTE:
// iteration = iteration
// element_id = element id
// node_id = node id
// map_id = map id (hashed element id and node id)

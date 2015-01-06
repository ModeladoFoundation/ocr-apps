#include "tuners/policy_static.h"

CNC_BITWISE_SERIALIZABLE( md_dist_policy_t );
CNC_BITWISE_SERIALIZABLE( md_particle_attr_t );
CNC_BITWISE_SERIALIZABLE( md_particle_t );
CNC_BITWISE_SERIALIZABLE( md_force_energy_t );
CNC_BITWISE_SERIALIZABLE( md_cell_index_wrapper_t );
CNC_BITWISE_SERIALIZABLE( md_cell_neighbor_list_t );
CNC_BITWISE_SERIALIZABLE( md_cell_neighbor_shift_list_t );
CNC_BITWISE_SERIALIZABLE( md_particle_list_shared_t );

struct md_context;

/*struct md_force_redux_selector {
	bool operator()(const md_iter_c1_c2_t& ic1c2, md_iter_cell_t& _c) {
		_c=std::make_pair(ic1c2.first, ic1c2.second.first);
		return true;
	};
};*/

struct md_energy_redux_selector {
	bool operator()(const md_iter_particle_t& ip, Int_t& _p) {
		_p=ip.first;
		return true;
	};
};

struct md_write_output_redux_selector {
	bool operator()(const md_iter_particle_t& ip, Int_t& _p) {
		_p=ip.first;
		return true;
	};
};

struct md_cell_init_grid
{
    int execute( const md_cell_index_t& cell, md_context& domain ) const;
};

struct md_iter_cell_timestep_velocity_position
{
    int execute( const md_iter_cell_t& iter_cell, md_context& domain ) const;
};

struct md_iter_cell_timestep_redistribute
{
    int execute( const md_iter_cell_t& iter_cell, md_context& domain ) const;
};

struct md_iter_c1_c2_timestep_compute_force
{
    int execute( const md_iter_c1_c2_t& iter_c1_c2, md_context& domain ) const;
};

struct md_iter_timestep_write_output
{
    int execute( const Int_t& iter, md_context& domain ) const;
};

#include "tuners/steptuners.h"
#include "tuners/itemtuners.h"

/* CELL-BASED DECOMPOSITION */
struct md_context : public CnC::context< md_context >
{
	/****************************************************************/
						  /* GLOBAL CONSTANTS */
	/****************************************************************/
	Index_t val_md_num_particles;									/* Number of particles */
	Index_t val_md_num_iters;										/* Number of iterations */
	Index_t val_md_num_print_interval;								/* Interval (in iterations) for printing energy */
	Real_t 	val_md_dt;												/* time step */
	std::string val_md_outputdir;									/* destination directory for output files */
	Real_t 	val_md_epsilon;											/* epsilon */
	Real_t 	val_md_sigma;											/* sigma */
	Real_t 	val_md_cutoff;											/* cutoff distance */
	Real_t 	val_md_shift_pot;										/* potential shift */
	Real_t 	val_md_cellsize_x;										/* cellsize in x dim */
	Real_t 	val_md_cellsize_y;										/* cellsize in y dim */
	Real_t 	val_md_cellsize_z;										/* cellsize in z dim */
	Int_t val_md_period_x, val_md_period_y, val_md_period_z; 		/* periodicity condition */
	Real_t val_md_simbox_lo_x, val_md_simbox_hi_x;					/* simulation box dimensions - x */
	Real_t val_md_simbox_lo_y, val_md_simbox_hi_y;					/* simulation box dimensions - y */
	Real_t val_md_simbox_lo_z, val_md_simbox_hi_z; 					/* simulation box dimensions - z */
	Index_t val_md_simbox_cx, val_md_simbox_cy, val_md_simbox_cz; 	/* simulation box dimensions (in number of cells) */
	Index_t val_md_num_cells;										/* Number of cells */
	/****************************************************************/


	/****************************************************************/
					    /* DISTRIBUTION POLICY */
	/****************************************************************/
	md_dist_policy_t distribution;
	/****************************************************************/

	/****************************************************************/
							  /* TUNERS */
	/****************************************************************/
	md_cell_init_grid_tuner tuner_md_cell_init_grid;
    md_iter_cell_timestep_velocity_position_tuner tuner_md_iter_cell_timestep_velocity_position;
    md_iter_cell_timestep_redistribute_tuner tuner_md_iter_cell_timestep_redistribute;
    md_iter_c1_c2_timestep_compute_force_tuner tuner_md_iter_c1_c2_timestep_compute_force;
    md_iter_timestep_write_output_tuner tuner_md_iter_timestep_write_output;

	md_item_cell_nogetcount_tuner tuner_md_item_cell_nogetcount;
	md_item_int_cell_single_tuner tuner_md_item_int_cell_single;
	md_item_c1_c2_nogetcount_tuner tuner_md_item_c1_c2_nogetcount;
	md_item_iter_c1_c2_single_tuner tuner_md_item_iter_c1_c2_single;
	md_item_iter_particle_single_tuner tuner_md_iter_particle_single;
	md_item_iter_single_tuner tuner_md_item_iter_single;
	md_item_iter_triple_tuner tuner_md_item_iter_triple;
	/****************************************************************/

	/****************************************************************/

	/****************************************************************/
								/* STEPS */
	/****************************************************************/
	/* INITIALIZATION */
	CnC::step_collection<md_cell_init_grid, md_cell_init_grid_tuner> steps_md_cell_init_grid;

	/* TIMESTEP */
    CnC::step_collection<md_iter_cell_timestep_velocity_position, md_iter_cell_timestep_velocity_position_tuner> steps_md_iter_cell_timestep_velocity_position;

	/* REDISTRIBUTE TIMESTEP */
    CnC::step_collection<md_iter_cell_timestep_redistribute, md_iter_cell_timestep_redistribute_tuner> steps_md_iter_cell_timestep_redistribute;

	/* FORCE STEP */
    CnC::step_collection<md_iter_c1_c2_timestep_compute_force, md_iter_c1_c2_timestep_compute_force_tuner> steps_md_iter_c1_c2_timestep_compute_force;

	/* OUTPUT WRITE STEP */
	//TODO: This is just a makeshift implementation to get the output. Optimize it!!!
    CnC::step_collection<md_iter_timestep_write_output, md_iter_timestep_write_output_tuner> steps_md_iter_timestep_write_output;
	/****************************************************************/

	/****************************************************************/
								/* TAGS */
	/****************************************************************/
	/* CELL TAGS */
    CnC::tag_collection<md_cell_index_t> tags_md_cell_init_grid;

	/* ITERATION CELL TAGS */
    CnC::tag_collection<md_iter_cell_t> tags_md_iter_cell_timestep;
    CnC::tag_collection<md_iter_cell_t> tags_md_iter_cell_timestep_redistribute;

	/* FORCE TAGS */
    CnC::tag_collection<md_iter_c1_c2_t> tags_md_iter_c1_c2_timestep_compute_force;

	/* OUTPUT WRITE TAGS */
	//TODO: This is just a makeshift implementation to get the output. Optimize it!!!
    CnC::tag_collection<Int_t> tags_md_iter_timestep_write_output;
	/****************************************************************/

	/****************************************************************/
								/* ITEMS */
	/****************************************************************/
	// Temporary data collections used during cell initialization
	/* INIT PARTICLE LIST */
	// This list is used only during initialization to account for possibly distributed file initialization
	CnC::item_collection<md_rank_cell_t, md_particle_list_t, md_item_int_cell_single_tuner> items_md_cell_init_particlelist;

	/************************************************************/
	// Primary data collections
	/* CELLS */
	CnC::item_collection<md_cell_index_t, md_cell_neighbor_list_t, md_item_cell_nogetcount_tuner> items_md_cell_constant_neighbor_list;

	/* SHIFT VECTORS */
	// Used for storing shift vectors for cell-coordinates between interacting neighboring cells
	// If <c1,c2> -> shift_vector, then c1 > c2 and shift c2's coordinates
	CnC::item_collection<md_c1_c2_t, md_cell_neighbor_shift_list_t, md_item_c1_c2_nogetcount_tuner> items_md_c1_c2_constant_neighbor_shift_vector_list;

	/* PARTICLE LIST */
	CnC::item_collection<md_iter_cell_t, md_particle_list_shared_t, md_item_int_cell_single_tuner> items_md_iter_cell_timestep_particlelist;

	/* PARTICLE BUFFER BETWEEN NEIGHBORING CELLS */
	// used for redistributing atoms in each timestep
	CnC::item_collection<md_iter_c1_c2_t, md_particle_list_t, md_item_iter_c1_c2_single_tuner> items_md_iter_c1_c2_timestep_redistribute_particlelist;

	/* FORCE BUFFER FOR LOCAL CELL */
	//TODO: Make this a shared pointer
	// used for referencing the local particle list during force computation for a cell in each timestep
	CnC::item_collection<md_iter_c1_c2_t, md_particle_attr_list_t, md_item_iter_c1_c2_single_tuner> items_md_iter_c1_c2_timestep_compute_force_local_particleattrlist;

	/* FORCE BUFFER BETWEEN NEIGHBORING CELLS */
	// used for exchanging interacting particle lists between neighboring cells in each timestep
	CnC::item_collection<md_iter_c1_c2_t, md_particle_attr_list_t, md_item_iter_c1_c2_single_tuner> items_md_iter_c1_c2_timestep_compute_force_particleattrlist;

	/* FORCE */
	/* PARTICLE FORCE PER CELL INTERACTION */
	// used for exchanging partial forces between neighboring cells in each timestep
	CnC::item_collection<md_iter_c1_c2_t, md_force_energy_list_t, md_item_iter_c1_c2_single_tuner> items_md_iter_c1_c2_timestep_force_between_neighbors;

	// Uncomment +6 when using force reduce graph
	/* FORCE REDUCTION COUNTS PER CELL PER ITERATION */
	//CnC::item_collection<md_iter_cell_t, Int_t, md_item_tuner_single> items_md_iter_cell_timestep_force_per_cell_red_counts;

	/* LIST OF REDUCED FORCE PER PARTICLE IN EACH CELL PER ITERATION */
	// Used for velocity computation in main timestep
	//CnC::item_collection<md_iter_cell_t, md_force_energy_list_t, md_item_tuner_single> items_md_iter_cell_timestep_force_per_cell;

	/* ENERGY */
	/* NUMBER OF REDUCTION VALUES PER ITERATION (= NUMBER OF PARTICLES) */
	/* Used for PE, KE calculation and consolidation of particle output per iteration, hence use get_count = 3 */
	CnC::item_collection<Int_t, Int_t, md_item_iter_triple_tuner> items_md_iter_write_output_red_counts;

	/* POTENTIAL ENERGY PER ITERATION PER PARTICLE */
	CnC::item_collection<md_iter_particle_t, Real_t, md_item_iter_particle_single_tuner> items_md_iter_particle_write_output_potential_energy;

	/* FINAL POTENTIAL ENERGY PER ITERATION */
	CnC::item_collection<Int_t, Real_t, md_item_iter_single_tuner> items_md_iter_write_output_potential_energy;

	/* KINETIC ENERGY PER ITERATION PER PARTICLE */
	CnC::item_collection<md_iter_particle_t, Real_t, md_item_iter_particle_single_tuner> items_md_iter_particle_write_output_kinetic_energy;

	/* FINAL KINETIC ENERGY PER ITERATION */
	CnC::item_collection<Int_t, Real_t, md_item_iter_single_tuner> items_md_iter_write_output_kinetic_energy;

	/* OUTPUT */
	//TODO: This is just a makeshift implementation to get the output. Optimize it!!!
	CnC::item_collection<md_iter_particle_t, std::string, md_item_iter_particle_single_tuner> items_md_iter_particle_write_output_write_str_particle_values;
	CnC::item_collection<Int_t, std::string, md_item_iter_single_tuner> items_md_iter_write_output_str_all_particle_values;
	/************************************************************/

	/****************************************************************/
								/* GRAPHS */
	/****************************************************************/
	/* FORCE REDUCTION GRAPH */
	// This reduction graph is removed and the force reduction is performed per neighbor in the
	// velocity step. Uncomment this if a reduction collection is available in the language
	// Uncomment +1 when using force reduce graph
	// CnC::graph * graph_md_cell_reduction_force_per_cell;

	/* ENERGY REDUCTION GRAPH */
	CnC::graph * graph_md_particle_reduction_pe_per_particle;
	CnC::graph * graph_md_particle_reduction_ke_per_particle;

	/* OUTPUT REDUCTION GRAPH */
	//TODO: This is just a makeshift implementation to get the output. Optimize it!!!
	CnC::graph * graph_md_iter_write_output_reduction;
	/****************************************************************/

	/****************************************************************/
							/* DEBUG COLLECTIONS */
	/****************************************************************/
	// Collections for debugging purposes
	/****************************************************************/

	// CONSTRUCTOR
	md_context(
			std::string md_outputdir		= "output",
			Real_t md_epsilon				= 0.,
			Real_t md_sigma					= 0.,
			Real_t md_cutoff				= 0.,
			Real_t md_shift_pot				= 0.,
			Index_t md_num_iters			= 0,
			Real_t md_num_print_interval	= 0,
			Real_t md_dt					= 0.,
			Index_t md_simbox_cx			= 0,
			Index_t md_simbox_cy			= 0,
			Index_t md_simbox_cz			= 0
			):CnC::context< md_context >(),

	// TUNERS
	//tuner_md_cell_init_grid ( *this ),
    //tuner_md_iter_cell_timestep_velocity_position ( *this ),
    //tuner_md_iter_cell_timestep_redistribute ( *this ),
    //tuner_md_iter_c1_c2_timestep_compute_force ( *this ),
    //tuner_md_iter_timestep_write_output ( *this ),

	tuner_md_item_cell_nogetcount ( *this ),
	tuner_md_item_int_cell_single ( *this ),
	tuner_md_item_c1_c2_nogetcount ( *this ),
	tuner_md_item_iter_c1_c2_single ( *this ),
	tuner_md_iter_particle_single ( *this ),
	tuner_md_item_iter_single ( *this ),
	tuner_md_item_iter_triple ( *this ),

	// STEPS
	steps_md_cell_init_grid ( *this, tuner_md_cell_init_grid, "steps_md_cell_init_grid" ),
	steps_md_iter_cell_timestep_velocity_position ( *this, tuner_md_iter_cell_timestep_velocity_position, "steps_md_iter_cell_timestep_velocity_position" ),
	steps_md_iter_cell_timestep_redistribute ( *this, tuner_md_iter_cell_timestep_redistribute, "steps_md_iter_cell_timestep_redistribute" ),
	steps_md_iter_c1_c2_timestep_compute_force ( *this, tuner_md_iter_c1_c2_timestep_compute_force, "steps_md_iter_c1_c2_timestep_compute_force" ),
	steps_md_iter_timestep_write_output( *this, tuner_md_iter_timestep_write_output, "steps_md_iter_timestep_write_output" ),

	// TAGS
	tags_md_cell_init_grid ( *this, "tags_md_cell_init_grid" ),
	tags_md_iter_cell_timestep ( *this, "tags_md_iter_cell_timestep" ),
	tags_md_iter_cell_timestep_redistribute ( *this, "tags_md_iter_cell_timestep_redistribute" ),
	tags_md_iter_c1_c2_timestep_compute_force( *this, "tags_md_iter_c1_c2_timestep_compute_force" ),
	tags_md_iter_timestep_write_output( *this, "tags_md_iter_timestep_write_output" ),

	// ITEMS
	items_md_cell_constant_neighbor_list ( *this, "items_md_cell_constant_neighbor_list", tuner_md_item_cell_nogetcount ),
	items_md_c1_c2_constant_neighbor_shift_vector_list ( *this, "items_md_c1_c2_constant_neighbor_shift_vector_list", tuner_md_item_c1_c2_nogetcount ),
	items_md_cell_init_particlelist ( *this, "items_md_cell_init_particlelist", tuner_md_item_int_cell_single ),
	items_md_iter_cell_timestep_particlelist ( *this, "items_md_iter_cell_timestep_particlelist" , tuner_md_item_int_cell_single ),
	items_md_iter_c1_c2_timestep_redistribute_particlelist ( *this, "items_md_iter_c1_c2_timestep_redistribute_particlelist", tuner_md_item_iter_c1_c2_single ),
	// CELL FORCE REDUCTION
	items_md_iter_c1_c2_timestep_compute_force_local_particleattrlist ( *this , "items_md_iter_c1_c2_timestep_compute_force_local_particleattrlist", tuner_md_item_iter_c1_c2_single),
	items_md_iter_c1_c2_timestep_compute_force_particleattrlist ( *this , "items_md_iter_c1_c2_timestep_compute_force_particleattrlist", tuner_md_item_iter_c1_c2_single),
	items_md_iter_c1_c2_timestep_force_between_neighbors ( *this , "items_md_iter_c1_c2_timestep_force_between_neighbors", tuner_md_item_iter_c1_c2_single),
	// Uncomment +3 when using force reduce graph
	//items_md_iter_cell_timestep_force_per_cell_red_counts ( *this , "items_md_iter_cell_timestep_force_per_cell_red_counts"),
	//items_md_iter_cell_timestep_force_per_cell ( *this , "items_md_iter_cell_timestep_force_per_cell" ),
	//graph_md_cell_reduction_force_per_cell ( NULL ),
	// ENERGY (PE and KE) REDUCTION
	items_md_iter_particle_write_output_potential_energy ( *this, "items_md_iter_particle_write_output_potential_energy", tuner_md_iter_particle_single),
	items_md_iter_particle_write_output_kinetic_energy ( *this, "items_md_iter_particle_write_output_kinetic_energy", tuner_md_iter_particle_single),
	items_md_iter_write_output_red_counts ( *this, "items_md_iter_write_output_red_counts", tuner_md_item_iter_triple),
	items_md_iter_write_output_potential_energy ( *this, "items_md_iter_write_output_potential_energy", tuner_md_item_iter_single),
	items_md_iter_write_output_kinetic_energy ( *this, "items_md_iter_write_output_kinetic_energy", tuner_md_item_iter_single),
	graph_md_particle_reduction_pe_per_particle ( NULL ),
	graph_md_particle_reduction_ke_per_particle ( NULL ),
	//OUTPUT
	items_md_iter_particle_write_output_write_str_particle_values ( *this, "items_md_iter_particle_write_output_write_str_particle_values", tuner_md_iter_particle_single),
	items_md_iter_write_output_str_all_particle_values ( *this, "items_md_iter_write_output_str_all_particle_values", tuner_md_item_iter_single),
	graph_md_iter_write_output_reduction ( NULL ),

	//GLOBALS
	val_md_num_iters(md_num_iters),
	val_md_num_print_interval(md_num_print_interval),
	val_md_dt(md_dt),
	val_md_outputdir(md_outputdir),
	val_md_epsilon(md_epsilon),
	val_md_sigma(md_sigma),
	val_md_cutoff(md_cutoff),
	val_md_shift_pot(md_shift_pot),
	val_md_simbox_cx(md_simbox_cx),
	val_md_simbox_cy(md_simbox_cy),
	val_md_simbox_cz(md_simbox_cz),
	val_md_num_cells(val_md_simbox_cx * val_md_simbox_cy * val_md_simbox_cz),
	distribution(val_md_simbox_cz, val_md_simbox_cy, val_md_simbox_cz, CnC::tuner_base::numProcs())
    {
		Real_t e0 = 0.;
		md_energy_redux_selector es;
		graph_md_particle_reduction_pe_per_particle = CnC::make_reduce_graph( *this,   			// context
                                         "potential energy reduction",		 					// name
                                         items_md_iter_particle_write_output_potential_energy, 	// input collection
                                         items_md_iter_write_output_red_counts, 		// number of items per reduction
                                         items_md_iter_write_output_potential_energy,			// the final result for each reduction
                                         std::plus<Real_t>(), 		 							// the reduction operation
                                         e0,			  	 	 						 		// identity element
										 es );
		graph_md_particle_reduction_ke_per_particle = CnC::make_reduce_graph( *this,			// context
                                         "kinetic energy reduction",		 					// name
                                         items_md_iter_particle_write_output_kinetic_energy, 	// input collection
                                         items_md_iter_write_output_red_counts,		// number of items per reduction
                                         items_md_iter_write_output_kinetic_energy,      		// the final result for each reduction
                                         std::plus<Real_t>(), 		 							// the reduction operation
                                         e0,  	 	 						 					// identity element
										 es );
		std::string s0 = "";
		md_write_output_redux_selector wos;
		graph_md_iter_write_output_reduction = CnC::make_reduce_graph( *this,   						// context
                                         "write output reduction",		 								// name
                                         items_md_iter_particle_write_output_write_str_particle_values, // input collection
                                         items_md_iter_write_output_red_counts, 	 			// number of items per reduction
                                         items_md_iter_write_output_str_all_particle_values,			// the final result for each reduction
                                         std::plus<std::string>(), 		 								// the reduction operation
                                         s0,  	 	 						 							// identity element
										 wos );
		// Uncomment +10 when using force reduce graph
		// md_force_energy_list_t vf0;
		// md_force_redux_selector fs;
  		/*graph_md_cell_reduction_force_per_cell = CnC::make_reduce_graph( *this,	 			// context
                                         "force reduction",					 					// name
                                         items_md_iter_c1_c2_timestep_force_between_neighbors, 	// input collection
                                         items_md_iter_cell_timestep_force_per_cell_red_counts, // number of items per reduction
                                         items_md_iter_cell_timestep_force_per_cell,       		// the final result for each reduction
                                         std::plus<md_force_energy_list_t >(), 	 				// the reduction operation
                                         vf0,  	 	 						 					// identity element
										 fs );*/

    	tags_md_cell_init_grid.prescribes( steps_md_cell_init_grid, *this );
    	tags_md_iter_cell_timestep.prescribes( steps_md_iter_cell_timestep_velocity_position, *this );
    	tags_md_iter_cell_timestep_redistribute.prescribes( steps_md_iter_cell_timestep_redistribute, *this );
    	tags_md_iter_c1_c2_timestep_compute_force.prescribes( steps_md_iter_c1_c2_timestep_compute_force, *this );
		tags_md_iter_timestep_write_output.prescribes( steps_md_iter_timestep_write_output, *this );

		steps_md_cell_init_grid.controls(tags_md_iter_cell_timestep);
		steps_md_iter_cell_timestep_redistribute.controls(tags_md_iter_cell_timestep);
		steps_md_iter_cell_timestep_velocity_position.controls(tags_md_iter_cell_timestep_redistribute);
		steps_md_cell_init_grid.controls(tags_md_iter_c1_c2_timestep_compute_force);
		steps_md_iter_cell_timestep_redistribute.controls(tags_md_iter_c1_c2_timestep_compute_force);
		steps_md_iter_cell_timestep_velocity_position.controls(tags_md_iter_timestep_write_output);

		steps_md_cell_init_grid.consumes( items_md_cell_init_particlelist );
		// For entire simulation duration
		steps_md_cell_init_grid.produces( items_md_cell_constant_neighbor_list );
		steps_md_cell_init_grid.produces( items_md_c1_c2_constant_neighbor_shift_vector_list );
		steps_md_cell_init_grid.consumes( items_md_c1_c2_constant_neighbor_shift_vector_list );
		// For velocity position step
		steps_md_cell_init_grid.produces( items_md_iter_cell_timestep_particlelist );

		steps_md_iter_cell_timestep_velocity_position.consumes( items_md_cell_constant_neighbor_list );
		steps_md_iter_cell_timestep_velocity_position.consumes( items_md_iter_cell_timestep_particlelist );
		// steps_md_iter_cell_timestep_velocity_position.consumes( items_md_iter_cell_timestep_force_per_cell );
		steps_md_iter_cell_timestep_velocity_position.consumes( items_md_iter_c1_c2_timestep_force_between_neighbors );
		// For redistribution step
		steps_md_iter_cell_timestep_velocity_position.produces( items_md_iter_c1_c2_timestep_redistribute_particlelist );
		// For print output step
		steps_md_iter_cell_timestep_velocity_position.produces( items_md_iter_particle_write_output_write_str_particle_values );
		steps_md_iter_cell_timestep_velocity_position.produces( items_md_iter_particle_write_output_potential_energy );
		steps_md_iter_cell_timestep_velocity_position.produces( items_md_iter_particle_write_output_kinetic_energy );

		steps_md_iter_cell_timestep_redistribute.consumes( items_md_cell_constant_neighbor_list );
		steps_md_iter_cell_timestep_redistribute.consumes( items_md_c1_c2_constant_neighbor_shift_vector_list );
		steps_md_iter_cell_timestep_redistribute.consumes( items_md_iter_c1_c2_timestep_redistribute_particlelist );
		// For force calculation step
		steps_md_iter_cell_timestep_redistribute.produces( items_md_iter_c1_c2_timestep_compute_force_local_particleattrlist );
		steps_md_iter_cell_timestep_redistribute.produces( items_md_iter_c1_c2_timestep_compute_force_particleattrlist );
		// For force reduction step (reduction counts = num of neighbors + 1)
		// steps_md_iter_cell_timestep_redistribute.produces( items_md_iter_cell_timestep_force_per_cell_red_counts );
		// For force reduction step (this is the local force)
		steps_md_iter_cell_timestep_redistribute.produces( items_md_iter_c1_c2_timestep_force_between_neighbors );
		// For velocity position step
		steps_md_iter_cell_timestep_redistribute.produces( items_md_iter_cell_timestep_particlelist );

		steps_md_iter_c1_c2_timestep_compute_force.consumes( items_md_iter_c1_c2_timestep_compute_force_local_particleattrlist );
		steps_md_iter_c1_c2_timestep_compute_force.consumes( items_md_iter_c1_c2_timestep_compute_force_particleattrlist );
		steps_md_iter_c1_c2_timestep_compute_force.consumes( items_md_c1_c2_constant_neighbor_shift_vector_list );
		// For force reduction step (these are forces per cell pair)
		steps_md_iter_c1_c2_timestep_compute_force.produces( items_md_iter_c1_c2_timestep_force_between_neighbors );

		steps_md_iter_timestep_write_output.consumes( items_md_iter_write_output_str_all_particle_values );
		steps_md_iter_timestep_write_output.consumes( items_md_iter_write_output_potential_energy );
		steps_md_iter_timestep_write_output.consumes( items_md_iter_write_output_kinetic_energy );

		// Scheduler Statistics
		//CnC::debug::collect_scheduler_statistics(*this);
		//CnC::debug::init_timer(true);
		//CnC::debug::time(steps_md_cell_init_grid);
		//CnC::debug::time(steps_md_iter_cell_timestep_velocity_position);
		//CnC::debug::time(steps_md_iter_cell_timestep_redistribute);
		//CnC::debug::time(steps_md_iter_c1_c2_timestep_compute_force);
		//CnC::debug::time(steps_md_iter_timestep_write_output);

		//CnC::debug::trace(steps_md_cell_init_grid);
		//CnC::debug::trace(steps_md_iter_cell_timestep_velocity_position);
		//CnC::debug::trace(steps_md_iter_cell_timestep_redistribute);
		//CnC::debug::trace(steps_md_iter_c1_c2_timestep_compute_force);
		//CnC::debug::trace(steps_md_iter_timestep_write_output);

		//CnC::debug::trace_all(*this, 1);
	}

#ifdef _DIST_
	void serialize( CnC::serializer& md_context_buff )
	{
		md_context_buff & val_md_num_particles										/* Number of particles */
						& val_md_num_cells											/* Number of cells */
						& val_md_num_iters											/* Number of iterations */
						& val_md_num_print_interval									/* Interval (in iterations) for printing energy */
						& val_md_dt													/* time step */
						& val_md_epsilon											/* epsilon */
						& val_md_sigma												/* sigma */
						& val_md_cutoff												/* cutoff distance */
						& val_md_shift_pot											/* potential shift */
						& val_md_cellsize_x											/* cellsize in x dim */
						& val_md_cellsize_y											/* cellsize in y dim */
						& val_md_cellsize_z											/* cellsize in z dim */
						& val_md_outputdir											/* destination directory for output files */
						& val_md_period_x & val_md_period_y & val_md_period_z		/* periodicity condition */
						& val_md_simbox_lo_x & val_md_simbox_hi_x					/* simulation box x dimensions */
						& val_md_simbox_lo_y & val_md_simbox_hi_y					/* simulation box y dimensions */
						& val_md_simbox_lo_z & val_md_simbox_hi_z					/* simulation box z dimensions */
						& val_md_simbox_cx & val_md_simbox_cy & val_md_simbox_cz	/* simulation box dimensions (in number of cells) */
						& distribution;												/* distribution policy */
	}
#endif

	~md_context()
	{
		CnC::debug::finalize_timer("-");
	}
};

#include "tuners/steptuners.hpp"
#include "tuners/itemtuners.hpp"

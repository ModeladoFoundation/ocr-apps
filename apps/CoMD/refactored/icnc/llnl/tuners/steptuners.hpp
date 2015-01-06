/************************************************************************************************/
// Init Grid Tuner
/************************************************************************************************/
int md_cell_init_grid_tuner::compute_on( const md_cell_index_t& cell, md_context& domain) const {
	return domain.distribution.get_place_id(cell);
}
template< class dependency_consumer >
void md_cell_init_grid_tuner::depends( const md_cell_index_t& cell, md_context& domain, dependency_consumer& dC ) const
{
	// Currently hard-coded to read from place 0
	// If multiple processes are configured to read the input file in a distributed manner,
	// change this dependency accordingly, else use preschedule()
	dC.depends( domain.items_md_cell_init_particlelist, std::make_pair(0, cell) );
}
/************************************************************************************************/

/************************************************************************************************/
// Velocity Position Step Tuner
/************************************************************************************************/
int md_iter_cell_timestep_velocity_position_tuner::compute_on( const md_iter_cell_t& iter_cell, md_context& domain ) const {
	return domain.distribution.get_place_id(iter_cell.second);
}
template< class dependency_consumer >
void md_iter_cell_timestep_velocity_position_tuner::depends( const md_iter_cell_t& iter_cell, md_context& domain, dependency_consumer& dC ) const
{
	dC.depends( domain.items_md_cell_constant_neighbor_list, iter_cell.second );
	dC.depends( domain.items_md_iter_cell_timestep_particlelist, iter_cell );
	dC.depends( domain.items_md_iter_c1_c2_timestep_force_between_neighbors, std::make_pair(iter_cell.first - 1, std::make_pair(iter_cell.second, iter_cell.second)));
	//FIXME: This does not work if using reduction graph. segfault for the 351 and 440 atoms case. Why???
	//dC.depends( domain.items_md_iter_cell_timestep_force_per_cell, std::make_pair(iter_cell.first - 1, iter_cell.second));
}
/************************************************************************************************/

/************************************************************************************************/
// Redistribute Step Tuner
/************************************************************************************************/
int md_iter_cell_timestep_redistribute_tuner::compute_on( const md_iter_cell_t& iter_cell, md_context& domain ) const {
	return domain.distribution.get_place_id(iter_cell.second);
}
template< class dependency_consumer >
// !!! DYNAMIC DEPENDENCE HERE !!!
// Since neighbor list is not known. Use preschedule() instead
void md_iter_cell_timestep_redistribute_tuner::depends( const md_iter_cell_t& iter_cell, md_context& domain, dependency_consumer& dC ) const
{
	dC.depends( domain.items_md_cell_constant_neighbor_list, iter_cell.second );
	dC.depends( domain.items_md_c1_c2_constant_neighbor_shift_vector_list, std::make_pair(iter_cell.second, iter_cell.second) );
}
/************************************************************************************************/

/************************************************************************************************/
// Force Step Tuner
/************************************************************************************************/
int md_iter_c1_c2_timestep_compute_force_tuner::compute_on( const md_iter_c1_c2_t& ic1c2, md_context& domain ) const {
	return domain.distribution.get_place_id(ic1c2.second.first);
}

template< class dependency_consumer >
void md_iter_c1_c2_timestep_compute_force_tuner::depends( const md_iter_c1_c2_t& ic1c2, md_context& domain, dependency_consumer& dC ) const
{
	dC.depends( domain.items_md_iter_c1_c2_timestep_compute_force_local_particleattrlist, ic1c2 );
	// FIXME: This gives asegfault. Why???
	// dC.depends( domain.items_md_iter_c1_c2_timestep_compute_force_particleattrlist, ic1c2 );
	dC.depends( domain.items_md_c1_c2_constant_neighbor_shift_vector_list, ic1c2.second );
}
/************************************************************************************************/

/************************************************************************************************/
// Write Output Step Tuner
/************************************************************************************************/
int md_iter_timestep_write_output_tuner::compute_on( const Int_t& iter, md_context& domain ) const {
	return domain.distribution.get_place_id(iter);
}
template< class dependency_consumer >
void md_iter_timestep_write_output_tuner::depends( const Int_t& iter, md_context& domain, dependency_consumer& dC ) const
{
	if(iter % domain.val_md_num_print_interval == 0 || iter == domain.val_md_num_iters) {
		dC.depends( domain.items_md_iter_write_output_str_all_particle_values, iter );
		dC.depends( domain.items_md_iter_write_output_potential_energy, iter );
		dC.depends( domain.items_md_iter_write_output_kinetic_energy, iter );
	}
}
/************************************************************************************************/

struct md_cell_timestep_advance_velocity_func
{
	private:
	Index_t iter;
	Real_t md_dt;
	md_particle_list_shared_t pl;
	md_force_energy_t* fp;
	md_context& domain;
	bool print_output;

	public:
	md_cell_timestep_advance_velocity_func(Index_t iter_, md_particle_list_shared_t pl_, md_force_energy_t* fp_, md_context& domain_, Real_t md_dt_, bool print_output_);
	int operator()(Index_t i) const;
};

static inline void md_cell_timestep_advance_velocity(Index_t iter, md_particle_t& p, md_force_energy_t* fp, Real_t md_dt, md_context& domain, bool print_output);

static inline Int_t md_cell_timestep_advance_position(md_iter_cell_t iter_cell, md_cell_neighbor_list_t nl, md_particle_t& p, Real_t md_dt, md_context& domain);

#include "velocity_position.hpp"

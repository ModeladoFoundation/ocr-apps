struct md_cell_init_grid_tuner : public CnC::step_tuner<>
{
	int compute_on( const md_cell_index_t& cell, md_context& domain) const;
	template< class dependency_consumer >
	void depends( const md_cell_index_t& cell, md_context& domain, dependency_consumer& dC ) const;
	// bool preschedule() const { return false; }
};

struct md_iter_cell_timestep_velocity_position_tuner : public CnC::step_tuner<>
{
	int compute_on( const md_iter_cell_t& iter_cell, md_context& domain ) const;
	template< class dependency_consumer >
	void depends( const md_iter_cell_t& iter_cell, md_context& domain, dependency_consumer& dC ) const;
};

struct md_iter_cell_timestep_redistribute_tuner : public CnC::step_tuner<>
{
	int compute_on( const md_iter_cell_t& iter_cell, md_context& domain ) const;
	template< class dependency_consumer >
	void depends( const md_iter_cell_t& iter_cell, md_context& domain, dependency_consumer& dC ) const;
	// bool preschedule() const { return true; }
};

struct md_iter_c1_c2_timestep_compute_force_tuner : public CnC::step_tuner<>
{
	int compute_on( const md_iter_c1_c2_t& iter_c1_c2, md_context& domain ) const;
	template< class dependency_consumer >
	void depends( const md_iter_c1_c2_t& ic1c2, md_context& domain, dependency_consumer& dC ) const;
};

struct md_iter_particle_timestep_write_output_tuner : public CnC::step_tuner<>
{
    int compute_on( const md_iter_particle_t& ip, md_context& domain ) const;
	template< class dependency_consumer >
	void depends( const md_iter_particle_t& ip, md_context& domain, dependency_consumer& dC ) const;
};

struct md_iter_timestep_write_output_tuner : public CnC::step_tuner<>
{
    int compute_on( const Int_t& iter, md_context& domain ) const;
	template< class dependency_consumer >
	void depends( const Int_t& iter, md_context& domain, dependency_consumer& dC ) const;
};

md_cell_timestep_advance_velocity_func::md_cell_timestep_advance_velocity_func(Index_t iter_, md_particle_list_shared_t pl_, md_force_energy_t* fp_, md_context& domain_, Real_t md_dt_, bool print_output_) : pl(pl_), domain(domain_)
{
	iter = iter_;
	md_dt = md_dt_;
	fp = fp_;
	print_output = print_output_;
}

static inline void md_cell_timestep_advance_velocity(Index_t iter, md_particle_t& p, md_force_energy_t* fp, Real_t md_dt, md_context& domain, bool print_output) {
	md_iter_particle_t ip = std::make_pair(iter, p.a.pid);
	Real_t inv_mass = 1.0/p.mass;
	md_real_vec3_t delta_v = fp->f * inv_mass * md_dt;

	if(iter > 0) p.v += delta_v;

	if (print_output) {


		Real_t ke = 0.5 * p.mass * p.v.dot(p.v);
		domain.items_md_iter_particle_write_output_kinetic_energy.put(ip, ke);
		domain.items_md_iter_particle_write_output_potential_energy.put(ip, fp->pe);

		std::stringstream str_pval;
		str_pval << p.a.pid << ' ' << p.mass << ' ' << p.a.p.x << ' ' << p.a.p.y << ' ' << p.a.p.z << ' ' << p.v.x << ' ' << p.v.y << ' ' << p.v.z << ' ' << fp->f.x << ' ' << fp->f.y << ' ' << fp->f.z << "\n";

		domain.items_md_iter_particle_write_output_write_str_particle_values.put(ip, str_pval.str());
		if(ip.second == 1) domain.tags_md_iter_timestep_write_output.put(ip.first);
	}

	p.v += delta_v;
}

int md_cell_timestep_advance_velocity_func::operator()(Index_t i) const {
	md_particle_t& p = (*pl)[i];
	md_cell_timestep_advance_velocity(iter, p, fp+i, md_dt, domain, print_output);

	return CnC::CNC_Success;
}

static inline Int_t md_cell_timestep_advance_position(md_iter_cell_t iter_cell, md_cell_neighbor_list_t nl, md_particle_t& p, Real_t md_dt, md_context& domain) {
	p.a.p += p.v * md_dt;

	Index_t cx = (Index_t)floor((p.a.p.x - domain.val_md_simbox_lo_x) / domain.val_md_cellsize_x);
	Index_t cy = (Index_t)floor((p.a.p.y - domain.val_md_simbox_lo_y) / domain.val_md_cellsize_y);
	Index_t cz = (Index_t)floor((p.a.p.z - domain.val_md_simbox_lo_z) / domain.val_md_cellsize_z);

	bool perx = (domain.val_md_period_x == PERIODIC);
	bool pery = (domain.val_md_period_y == PERIODIC);
	bool perz = (domain.val_md_period_z == PERIODIC);

	if(perx) {
		Real_t val_md_simbox_x = domain.val_md_simbox_hi_x - domain.val_md_simbox_lo_x;
		if(cx == -1) {
			p.a.p.x += val_md_simbox_x;
			cx = domain.val_md_simbox_cx - 1;
		}
		else if(cx == domain.val_md_simbox_cx) {
			p.a.p.x -= val_md_simbox_x;
			cx = 0;
		}
	}

	if(pery) {
		Real_t val_md_simbox_y = domain.val_md_simbox_hi_y - domain.val_md_simbox_lo_y;
		if(cy == -1) {
			p.a.p.y += val_md_simbox_y;
			cy = domain.val_md_simbox_cy - 1;
		}
		else if(cy == domain.val_md_simbox_cy) {
			p.a.p.y -= val_md_simbox_y;
			cy = 0;
		}
	}

	if(perz) {
		Real_t val_md_simbox_z = domain.val_md_simbox_hi_z - domain.val_md_simbox_lo_z;
		if(cz == -1) {
			p.a.p.z += val_md_simbox_z;
			cz = domain.val_md_simbox_cz - 1;
		}
		else if(cz == domain.val_md_simbox_cz) {
			p.a.p.z -= val_md_simbox_z;
			cz = 0;
		}
	}

	md_cell_index_wrapper_t parent_cell = iter_cell.second;
	md_cell_index_wrapper_t new_parent_cell = md_cell_index_wrapper_t(cx, cy, cz);

	// check if in the same cell
	if(parent_cell == new_parent_cell)
	{
		return nl->size();
	}
	// check if in a neighboring cell
	Int_t i = 0;
	for (auto n = nl->begin() ; n != nl->end(); ++n, ++i) {
		if (new_parent_cell == (*n))
		{
			//fprintf(stderr, "Warn:: particle %d moved from %s to a neighbor %s.\n", p.a.pid, parent_cell.ctoa().c_str(), new_parent_cell.ctoa().c_str());
			return i;
		}
	}

	// check if outside the simulation box (assuming fixed-boundary condition, particle lost)
	if (cx < 0 || cx >= domain.val_md_simbox_cx || cy < 0 || cy >= domain.val_md_simbox_cy || cz < 0 || cz >= domain.val_md_simbox_cz) {
		fprintf(stderr, "Error:: iter %d, particle %d escaped the sim box from %s to %s::(%lf, %lf, %lf)::(%lf, %lf, %lf).\n", iter_cell.first, p.a.pid, parent_cell.ctoa().c_str(), new_parent_cell.ctoa().c_str(), p.a.p.x, p.a.p.y, p.a.p.z, p.v.x, p.v.y, p.v.z);
		_Exit(1);
	}

	// particle jumps more than one neighbor. Error!!
	fprintf(stderr, "Error:: particle %d moved from %s to a non-neighbor %s.\n", p.a.pid, parent_cell.ctoa().c_str(), new_parent_cell.ctoa().c_str());
	_Exit(1);
}

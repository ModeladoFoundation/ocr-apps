static inline md_force_energy_t md_cell_timestep_compute_forces_between_particles(md_real_vec3_t& p1, md_real_vec3_t& p2, Real_t md_epsilon, Real_t s6, Real_t rCut2, Real_t eShift) {
	md_real_vec3_t dr = p1 - p2;
	Real_t r2 = dr.dot(dr);

	md_force_energy_t ret;

	if (r2 <= rCut2 && r2 > 0.0) {
		r2 = 1.0/r2;
		Real_t r6 = s6 * (r2*r2*r2);
		Real_t eLocal = r6 * (r6 - 1.0) - eShift;

		// Force per particle pair (p1,p2)
		Real_t fr = - 4.0*md_epsilon*r6*r2*(12.0*r6 - 6.0);
		ret.f = (dr * fr);

		// Calculate half-energy per particle pair (= 4*md_epsilon*r6(r6-1.0)/2)
		ret.pe = 2.0*md_epsilon*eLocal;
	}
	return ret;
}

static inline void md_cell_timestep_compute_forces_between_particles(Index_t iter, md_cell_index_t c1, md_cell_index_t c2, md_cell_neighbor_shift_list_t nsl, md_particle_attr_list_t& pl1, md_particle_attr_list_t& pl2, md_context& domain) {

	Real_t md_epsilon, md_sigma, md_cutoff, md_shift_pot, s6, rCut2, rCut6, eShift;

	md_force_energy_t fp0;

	md_force_energy_list_t fp1;
	Int_t i;
	for(i = 0; i < pl1.size(); i++) {
		fp1.push_back(fp0);
	}

	md_force_energy_list_t fp2;
	Int_t j;
	for(j = 0; j < pl2.size(); j++) {
		fp2.push_back(fp0);
	}

	if(pl1.size()>0 && pl2.size()>0) {
		md_epsilon = domain.val_md_epsilon;
		md_sigma = domain.val_md_sigma;
		md_cutoff = domain.val_md_cutoff;
		md_shift_pot = domain.val_md_shift_pot;

		s6 = md_sigma*md_sigma*md_sigma*md_sigma*md_sigma*md_sigma;
		rCut2 = md_cutoff*md_cutoff;
		rCut6 = s6 / (rCut2*rCut2*rCut2);
		eShift = md_shift_pot * rCut6 * (rCut6 - 1.0);
	}

	for (int nvi = 0; nvi < nsl->size(); ++nvi) {
		Shift_t nxl = (*nsl)[nvi];
		if(nxl != 0) {
			Flag_t xshift = (nxl & 0x00FF0000) >> 16;
			Flag_t yshift = (nxl & 0x0000FF00) >> 8;
			Flag_t zshift = (nxl & 0x000000FF);

			Real_t val_md_simbox_x = domain.val_md_simbox_hi_x - domain.val_md_simbox_lo_x;
			Real_t val_md_simbox_y = domain.val_md_simbox_hi_y - domain.val_md_simbox_lo_y;
			Real_t val_md_simbox_z = domain.val_md_simbox_hi_z - domain.val_md_simbox_lo_z;

			md_real_vec3_t shift_vector(val_md_simbox_x * xshift, val_md_simbox_y * yshift, val_md_simbox_z * zshift);
			for (auto p2 = pl2.begin() ; p2 != pl2.end(); ++p2) {
				(*p2).p += shift_vector;
			}
		}
		i = 0;
		j = 0;
		for (auto p1 = pl1.begin() ; p1 != pl1.end(); ++p1, ++i) {
			for (auto p2 = pl2.begin() ; p2 != pl2.end(); ++p2, ++j) {
				assert((*p1).pid != (*p2).pid);

				md_force_energy_t ret = md_cell_timestep_compute_forces_between_particles((*p1).p, (*p2).p, md_epsilon, s6, rCut2, eShift);
				fp1[i].f  -= ret.f;  // Substract from p1
				fp1[i].pe += ret.pe; // Half potential energy per pair (p1,p2)
				fp2[j].f  += ret.f;  // Equal and opposite force; add to p2
				fp2[j].pe += ret.pe; // Half potential energy per pair (p1,p2)
			}
			j = 0;
		}

		// Reset the place co-ordinates. Applicable only in 2-cell case
		if(nsl->size() > 1 && nxl != 0) {
			Flag_t xshift = (nxl & 0x00FF0000) >> 16;
			Flag_t yshift = (nxl & 0x0000FF00) >> 8;
			Flag_t zshift = (nxl & 0x000000FF);

			Real_t val_md_simbox_x = domain.val_md_simbox_hi_x - domain.val_md_simbox_lo_x;
			Real_t val_md_simbox_y = domain.val_md_simbox_hi_y - domain.val_md_simbox_lo_y;
			Real_t val_md_simbox_z = domain.val_md_simbox_hi_z - domain.val_md_simbox_lo_z;

			md_real_vec3_t shift_vector(val_md_simbox_x * xshift, val_md_simbox_y * yshift, val_md_simbox_z * zshift);
			for (auto p2 = pl2.begin() ; p2 != pl2.end(); ++p2) {
				(*p2).p -= shift_vector;
			}
		}
	}
	md_c1_c2_t c1c2 = std::make_pair(c1, c2);
	md_iter_c1_c2_t ic1c2 = std::make_pair(iter, c1c2);
	domain.items_md_iter_c1_c2_timestep_force_between_neighbors.put(ic1c2, fp1);

	md_c1_c2_t c2c1 = std::make_pair(c2, c1);
	md_iter_c1_c2_t ic2c1 = std::make_pair(iter, c2c1);
	domain.items_md_iter_c1_c2_timestep_force_between_neighbors.put(ic2c1, fp2);
}

void md_cell_timestep_compute_local_forces(const md_iter_cell_t& iter_cell, const md_cell_neighbor_shift_list_t nsl, md_particle_list_shared_t pl, Index_t red_counts, md_context& domain) {

	md_force_energy_t fp;

	md_force_energy_list_t force_local;
	for (Int_t i = 0; i<pl->size(); ++i) {
		force_local.push_back(fp);
	}

	Real_t md_epsilon = domain.val_md_epsilon;
	Real_t md_sigma = domain.val_md_sigma;
	Real_t md_cutoff = domain.val_md_cutoff;
	Real_t md_shift_pot = domain.val_md_shift_pot;

	Real_t s6 = md_sigma*md_sigma*md_sigma*md_sigma*md_sigma*md_sigma;
	Real_t rCut2 = md_cutoff*md_cutoff;
	Real_t rCut6 = s6 / (rCut2*rCut2*rCut2);
	Real_t eShift = md_shift_pot * rCut6 * (rCut6 - 1.0);

	// This loop will iterate more than once only in case the simulation box size is 1
	// in any dimension.
	// Do NOT parallelize this loop
	// Preferably comment it for simulation size > 1
// /*
	for (int nvi = 0; nvi < nsl->size(); ++nvi) {
		Int_t i = 0;
		Shift_t nxl = (*nsl)[nvi];
		if(nxl != 0) {
			Flag_t xshift = (nxl & 0x00FF0000) >> 16;
			Flag_t yshift = (nxl & 0x0000FF00) >> 8;
			Flag_t zshift = (nxl & 0x000000FF);

			Real_t val_md_simbox_x = domain.val_md_simbox_hi_x - domain.val_md_simbox_lo_x;
			Real_t val_md_simbox_y = domain.val_md_simbox_hi_y - domain.val_md_simbox_lo_y;
			Real_t val_md_simbox_z = domain.val_md_simbox_hi_z - domain.val_md_simbox_lo_z;

			md_real_vec3_t shift_vector(val_md_simbox_x * xshift, val_md_simbox_y * yshift, val_md_simbox_z * zshift);
			for (auto p1 = pl->begin() ; p1 != pl->end(); ++p1, ++i) {
				for (auto p2 = pl->begin() ; p2 != pl->end(); ++p2) {
					if((*p1).a.pid != (*p2).a.pid) {
						md_real_vec3_t p2_new = (*p2).a.p + shift_vector;
						md_force_energy_t ret = md_cell_timestep_compute_forces_between_particles((*p1).a.p, p2_new, md_epsilon, s6, rCut2, eShift);
						force_local[i].f  -= ret.f;
						force_local[i].pe += ret.pe;
					}
				}
			}
		}
		else {
//	*/
			for (auto p1 = pl->begin() ; p1 != pl->end(); ++p1, ++i) {
				for (auto p2 = pl->begin() ; p2 != pl->end(); ++p2) {
					if((*p1).a.pid != (*p2).a.pid) {
						md_force_energy_t ret = md_cell_timestep_compute_forces_between_particles((*p1).a.p, (*p2).a.p, md_epsilon, s6, rCut2, eShift);
						force_local[i].f  -= ret.f;
						force_local[i].pe += ret.pe;
					}
				}
			}
// /*
		}
// */
	}

	// domain.items_md_iter_cell_timestep_force_per_cell_red_counts.put(iter_cell, red_counts);
	md_iter_c1_c2_t ic1c1 = std::make_pair(iter_cell.first, std::make_pair(iter_cell.second, iter_cell.second));
	domain.items_md_iter_c1_c2_timestep_force_between_neighbors.put(ic1c1, force_local);
}

void md_cell_timestep_compute_neighbor_forces(const md_iter_cell_t& iter_cell, md_cell_neighbor_list_t nl, md_particle_list_shared_t pl, md_context& domain) {
	Index_t i = iter_cell.first;
	md_cell_index_wrapper_t c1 = iter_cell.second;

	for (auto c2 = nl->begin() ; c2 != nl->end(); ++c2) {
		assert(c1 != (*c2));
		md_particle_attr_list_t pal;
		for (auto p = pl->begin() ; p != pl->end(); ++p) {
			pal.push_back((*p).a);
		}
		if(c1 > (*c2)) {
			md_iter_c1_c2_t ic1c2 = std::make_pair(i, std::make_pair(c1.get_index(), (*c2).get_index()));
			domain.items_md_iter_c1_c2_timestep_compute_force_local_particleattrlist.put(ic1c2, pal);
			domain.tags_md_iter_c1_c2_timestep_compute_force.put(ic1c2);
		}
		else {
			md_iter_c1_c2_t ic2c1 = std::make_pair(i, std::make_pair((*c2).get_index(), c1.get_index()));
			domain.items_md_iter_c1_c2_timestep_compute_force_particleattrlist.put(ic2c1, pal);
		}
	}
}

static inline void md_cell_timestep_compute_force(const md_iter_cell_t& iter_cell, md_cell_neighbor_list_t nl, md_cell_neighbor_shift_list_t nsl, md_particle_list_shared_t pl, md_context& domain) {
	md_cell_timestep_compute_neighbor_forces(iter_cell, nl, pl, domain);
	md_cell_timestep_compute_local_forces(iter_cell, nsl, pl, nl->size()+1, domain);
}

static inline void md_cell_timestep_compute_initial_force(const md_iter_cell_t& iter_cell, md_cell_neighbor_list_t nl, md_cell_neighbor_shift_list_t nsl, md_particle_list_shared_t pl, md_context& domain) {
	md_cell_timestep_compute_force(iter_cell, nl, nsl, pl, domain);
}

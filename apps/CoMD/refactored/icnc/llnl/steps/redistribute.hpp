static inline void md_cell_timestamp_redistribute_particles(const md_iter_cell_t& iter_cell, md_particle_list_t* particle_to_neighbors, md_cell_neighbor_list_t nl, md_context& domain) {
	Int_t i = 0;
	for (auto n = nl->begin() ; n != nl->end(); ++n, ++i) {
		md_iter_c1_c2_t ic2c1 = std::make_pair(iter_cell.first, std::make_pair((*n).get_index(), iter_cell.second));
		md_particle_list_t npl; npl = particle_to_neighbors[i];
		domain.items_md_iter_c1_c2_timestep_redistribute_particlelist.put(ic2c1, npl);
	}
	md_iter_c1_c2_t ic1c1 = std::make_pair(iter_cell.first, std::make_pair(iter_cell.second, iter_cell.second));
	md_particle_list_t pl_redist;
	pl_redist = particle_to_neighbors[nl->size()];
	domain.items_md_iter_c1_c2_timestep_redistribute_particlelist.put(ic1c1, pl_redist);
	domain.tags_md_iter_cell_timestep_redistribute.put(iter_cell);
}

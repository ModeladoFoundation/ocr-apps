int md_item_cell_nogetcount_tuner::consumed_on(const md_cell_index_t& cell) const {
	return domain.distribution.get_place_id(cell);
}

int md_item_int_cell_single_tuner::consumed_on(const md_int_cell_t& int_cell) const {
	return domain.distribution.get_place_id(int_cell.second);
}

int md_item_c1_c2_nogetcount_tuner::consumed_on(const md_c1_c2_t& c1c2) const {
	//const md_cell_index_t& c1 = c1c2.first;
	//const md_cell_index_t& c2 = c1c2.second;
	//if(c1 > c2) return domain.distribution.get_place_id(c1);
	//else return domain.distribution.get_place_id(c2);
	return domain.distribution.get_place_id(c1c2.first);
}

int md_item_iter_c1_c2_single_tuner::consumed_on(const md_iter_c1_c2_t& ic1c2) const {
	return domain.distribution.get_place_id(ic1c2.second.first);
}

int md_item_iter_particle_single_tuner::consumed_on(const md_iter_particle_t& ip) const {
	return domain.distribution.get_place_id(ip.first);
}

int md_item_iter_single_tuner::consumed_on(const Int_t iter) const {
	return domain.distribution.get_place_id(iter);
}

int md_item_iter_triple_tuner::consumed_on(const Int_t iter) const {
	return domain.distribution.get_place_id(iter);
}

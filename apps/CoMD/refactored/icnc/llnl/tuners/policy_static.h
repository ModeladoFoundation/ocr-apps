// Change this file to change cell distribution
struct md_dist_policy_t {
	Index_t val_md_policy_num_cells_x;
	Index_t val_md_policy_num_cells_y;
	Index_t val_md_policy_num_cells_z;
	Index_t val_md_policy_num_places;

	md_dist_policy_t(Index_t val_md_policy_num_cells_x_ = 0,
					 Index_t val_md_policy_num_cells_y_ = 0,
					 Index_t val_md_policy_num_cells_z_ = 0,
					 Index_t val_md_policy_num_places_  = 0) :
						val_md_policy_num_cells_x(val_md_policy_num_cells_x_),
						val_md_policy_num_cells_y(val_md_policy_num_cells_y_),
						val_md_policy_num_cells_z(val_md_policy_num_cells_z_),
						val_md_policy_num_places(val_md_policy_num_places_) {}

	md_dist_policy_t(const md_dist_policy_t& dp) :
						val_md_policy_num_cells_x(dp.val_md_policy_num_cells_x),
						val_md_policy_num_cells_y(dp.val_md_policy_num_cells_y),
						val_md_policy_num_cells_z(dp.val_md_policy_num_cells_z),
						val_md_policy_num_places(dp.val_md_policy_num_places) {}

	int get_place_id(const md_cell_index_t& cell) {
		Index_t cx = cell.first;
		Index_t cy = cell.second.first;
		Index_t cz = cell.second.second;
		return (cx + cy*val_md_policy_num_cells_x + cz*val_md_policy_num_cells_x*val_md_policy_num_cells_y) % val_md_policy_num_places;
	}

	int get_place_id(const Int_t iter) {
		return 0;
	}
#ifdef _DIST_
	void serialize( CnC::serializer& md_distribution_buff )
	{
		md_distribution_buff & val_md_policy_num_cells_x
							 & val_md_policy_num_cells_y
							 & val_md_policy_num_cells_z
							 & val_md_policy_num_places;
	}
#endif
};

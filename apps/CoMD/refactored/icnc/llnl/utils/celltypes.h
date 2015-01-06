/* CELLS */
/* Implementation of cell index in cartesian coordinates */
// Change this class to change the cell representation
struct md_cell_index_wrapper_t {
	typedef std::pair<Int_t, std::pair<Int_t, Int_t> > md_cell_index_t;

	private:
	md_cell_index_t cid;		/* cell index (cartesian) */

	public:
	md_cell_index_wrapper_t() {}
	md_cell_index_wrapper_t(const md_cell_index_wrapper_t& c) : cid (c.cid) {}
	md_cell_index_wrapper_t(md_cell_index_t cid_) : cid (cid_) {}
	md_cell_index_wrapper_t(Index_t x, Index_t y, Index_t z) { cid = std::make_pair(x, std::make_pair(y,z));}

	md_cell_index_t get_index() const { return cid; }
	md_int_vec3_t get_coords() const { return md_int_vec3_t(cid.first, cid.second.first, cid.second.second); }

	bool operator==(const md_cell_index_wrapper_t& c) const { return cid == c.cid; }
	bool operator!=(const md_cell_index_wrapper_t& c) const { return !((*this) == c); }
	bool operator>(const md_cell_index_t& c) const { return cid > c; }
	bool operator<(const md_cell_index_t& c) const { return cid < c; }
	bool operator<=(const md_cell_index_t& c) const { return !((*this) > c); }
	bool operator>=(const md_cell_index_t& c) const { return !((*this) < c); }
	bool operator>(const md_cell_index_wrapper_t& c) const { return cid > c.cid; }
	bool operator<=(const md_cell_index_wrapper_t& c) const { return !((*this) > c); }
	bool operator<(const md_cell_index_wrapper_t& c) const { return cid < c.cid; }
	bool operator>=(const md_cell_index_wrapper_t& c) const { return !((*this) < c); }
	std::string ctoa() const {
		std::stringstream stm; stm << "<" << cid.first << ", " << cid.second.first << ", " << cid.second.second << ">";
		return stm.str();
	}
};

typedef md_cell_index_wrapper_t::md_cell_index_t md_cell_index_t;

typedef std::shared_ptr< std::vector< md_cell_index_wrapper_t > > md_cell_neighbor_list_t;	/* cell neighbor list */

// NOTE: This vector will contain multiple values only in cases where the same cell neighbors an adjoining cell
// along different periodic boundaries; this case arises only during a simulation with periodic boundary conditions
// comprising only 1 or 2-cells in any dimension. In a typical simulation with a size of 3 or more cells in each
// dimension, the vector will always contain ONLY one entry
// typedef std::vector<Shift_t> md_cell_neighbor_shift_list_t;				/* cell neighbor coordinates shift list */
typedef std::shared_ptr< std::vector<Shift_t > > md_cell_neighbor_shift_list_t;				/* cell neighbor coordinates shift list */

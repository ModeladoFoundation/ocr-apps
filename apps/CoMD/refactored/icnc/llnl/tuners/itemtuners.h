struct md_item_cell_nogetcount_tuner : public CnC::hashmap_tuner {
	private:
	md_context& domain;

	public:
	md_item_cell_nogetcount_tuner(md_context& domain_) : domain(domain_) {}
	md_item_cell_nogetcount_tuner(md_item_cell_nogetcount_tuner& m_) : domain(m_.domain) {}

	int get_count(const md_cell_index_t& cell) const {
		return CnC::NO_GETCOUNT;
	}
	int consumed_on(const md_cell_index_t& cell) const;
};

struct md_item_int_cell_single_tuner : public CnC::hashmap_tuner {
	private:
	md_context& domain;

	public:
	md_item_int_cell_single_tuner(md_context& domain_) : domain(domain_) {}
	md_item_int_cell_single_tuner(md_item_int_cell_single_tuner& m_) : domain(m_.domain) {}

	int get_count(const md_int_cell_t& int_cell) const {
		return 1;
	}
	int consumed_on(const md_int_cell_t& int_cell) const;
};

struct md_item_c1_c2_nogetcount_tuner : public CnC::hashmap_tuner {
	private:
	md_context& domain;

	public:
	md_item_c1_c2_nogetcount_tuner(md_context& domain_) : domain(domain_) {}
	md_item_c1_c2_nogetcount_tuner(md_item_c1_c2_nogetcount_tuner& m_) : domain(m_.domain) {}

	int get_count(const md_c1_c2_t& c1c2) const {
		return CnC::NO_GETCOUNT;
	}
	int consumed_on(const md_c1_c2_t& c1c2) const;
};

struct md_item_iter_c1_c2_single_tuner : public CnC::hashmap_tuner {
	private:
	md_context& domain;

	public:
	md_item_iter_c1_c2_single_tuner(md_context& domain_) : domain(domain_) {}
	md_item_iter_c1_c2_single_tuner(md_item_iter_c1_c2_single_tuner& m_) : domain(m_.domain) {}

	int get_count(const md_iter_c1_c2_t& ic1c2) const {
		return 1;
	}
	int consumed_on(const md_iter_c1_c2_t& ic1c2) const;
};

struct md_item_iter_particle_single_tuner : public CnC::hashmap_tuner {
	private:
	md_context& domain;

	public:
	md_item_iter_particle_single_tuner(md_context& domain_) : domain(domain_) {}
	md_item_iter_particle_single_tuner(md_item_iter_particle_single_tuner& m_) : domain(m_.domain) {}

	int get_count(const md_iter_particle_t& ip) const {
		return 1;
	}
	int consumed_on(const md_iter_particle_t& ip) const;
};

struct md_item_iter_single_tuner : public CnC::hashmap_tuner {
	private:
	md_context& domain;

	public:
	md_item_iter_single_tuner(md_context& domain_) : domain(domain_) {}
	md_item_iter_single_tuner(md_item_iter_single_tuner& m_) : domain(m_.domain) {}

	int get_count(const Int_t iter) const {
		return 1;
	}
	int consumed_on(const Int_t iter) const;
};

struct md_item_iter_triple_tuner : public CnC::hashmap_tuner {
	private:
	md_context& domain;

	public:
	md_item_iter_triple_tuner(md_context& domain_) : domain(domain_) {}
	md_item_iter_triple_tuner(md_item_iter_triple_tuner& m_) : domain(m_.domain) {}

	int get_count(const Int_t iter) const {
		return 3;
	}
	int consumed_on(const Int_t iter) const;
};

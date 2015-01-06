/* FORCE ENERGY */
struct md_force_energy_t {
	md_real_vec3_t f; // forces in x, y and z
	Real_t pe;		  // potential energy

	md_force_energy_t() {
		f = 0.;
		pe = 0.;
	}

	void operator=(const md_force_energy_t& v) {
		f  = v.f;
		pe = v.pe;
	}

	md_force_energy_t& operator+=(const md_force_energy_t& v) {
		f  += v.f;
		pe += v.pe;
		return (*this);
	}

	md_force_energy_t operator+(const md_force_energy_t& v) const {
		md_force_energy_t rv;
		rv.f  = f  + v.f;
		rv.pe = pe + v.pe;
		return rv;
	}

	md_force_energy_t operator-(const md_force_energy_t& v) const {
		md_force_energy_t rv;
		rv.f  = f  - v.f;
		rv.pe = pe - v.pe;
		return rv;
	}

	md_force_energy_t operator*(Real_t val) const {
		md_force_energy_t rv;
		rv.f  = f  * val;
		rv.pe = pe * val;
		return rv;
	}
};

typedef std::vector<md_force_energy_t> md_force_energy_list_t;

md_force_energy_list_t operator+(const md_force_energy_list_t& f1, const md_force_energy_list_t& f2) {
	md_force_energy_list_t f;
	if(f1.size()==0) {
		f = f2;
		return f;
	}
	else if(f2.size()==0) {
		f = f1;
		return f;
	}
	else if(f1.size() != f2.size()) {
		fprintf(stderr, "Unequal reduction vector sizes %d and %d. Exiting...", f1.size(), f2.size());
		_Exit(1);
	}
	else {
		Int_t size = f1.size();
		for(Int_t i = 0; i < size; i++) {
			f.push_back(f1[i]+f2[i]);
		}
		return f;
	}
}


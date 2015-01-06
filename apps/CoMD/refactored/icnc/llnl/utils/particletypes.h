/* PARTICLES */
typedef Index_t md_particle_index_t;	/* particle index */

struct md_particle_attr_t {
	md_particle_index_t pid;	/* particle id */
	md_real_vec3_t p;			/* particle position */

	bool operator==(const md_particle_attr_t& p) const {
		return pid == p.pid;
	}
};

struct md_particle_t {
	md_particle_attr_t a;
	md_real_vec3_t v;		/* particle velocity */
	Real_t mass;			/* particle mass */

	bool operator==(const md_particle_t& p) const {
		return a == p.a;
	}
};

typedef std::vector<md_particle_t> md_particle_list_t;
typedef std::shared_ptr< std::vector<md_particle_t> > md_particle_list_shared_t;
typedef std::vector<md_particle_attr_t> md_particle_attr_list_t;

#include <vector>
#include <inttypes.h>

/* GENERIC TYPES */
typedef float        real4;
typedef double       real8;
typedef long double  real10;

typedef int   	 Index_t; /* array subscript and loop index */
typedef real8 	 Real_t;  /* floating point representation */
typedef uint32_t Shift_t; /* bit vector for cell orientation */
typedef int8_t 	 Flag_t;  /* flag for cell orientation */
typedef int   	 Int_t;	  /* general integer representation */

typedef enum { FIXED = 0, PERIODIC = 1 } periodicity ;
typedef enum {	PLUS_X_SHIFT  = 0x00010000,
				MINUS_X_SHIFT = 0x00FF0000,
				PLUS_Y_SHIFT  = 0x00000100,
				MINUS_Y_SHIFT = 0x0000FF00,
				PLUS_Z_SHIFT  = 0x00000001,
				MINUS_Z_SHIFT = 0x000000FF
			 } shift_flags;

// Template for a triplet of numbers
// Only specialize this with number data types
template<class T>
struct md_num_vec3_t {		/* triplet of real */
	T x, y, z;

	T dot(const md_num_vec3_t<T>& v) const {
		return x*v.x + y*v.y + z*v.z;
	}

	md_num_vec3_t() {
	}

	md_num_vec3_t(T val) {
		x = y = z = val;
	}

	md_num_vec3_t(T vx, T vy, T vz) {
		x = vx; y = vy; z = vz;
	}

	void operator=(const md_num_vec3_t<T>& v) {
		x = v.x;
		y = v.y;
		z = v.z;
	}

	md_num_vec3_t<T>& operator+=(const md_num_vec3_t<T>& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return (*this);
	}

	md_num_vec3_t<T> operator+(const md_num_vec3_t<T>& v) const {
		md_num_vec3_t<T> rv;
		rv.x = x + v.x;
		rv.y = y + v.y;
		rv.z = z + v.z;
		return rv;
	}

	md_num_vec3_t<T>& operator-=(const md_num_vec3_t<T>& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return (*this);
	}

	md_num_vec3_t<T> operator-(const md_num_vec3_t<T>& v) const {
		md_num_vec3_t<T> rv;
		rv.x = x - v.x;
		rv.y = y - v.y;
		rv.z = z - v.z;
		return rv;
	}

	md_num_vec3_t<T> operator*(T val) const {
		md_num_vec3_t<T> rv;
		rv.x = x * val;
		rv.y = y * val;
		rv.z = z * val;
		return rv;
	}
};

/* triplet of real */
typedef md_num_vec3_t<Real_t> md_real_vec3_t;

/* triplet of int */
typedef md_num_vec3_t<Index_t> md_int_vec3_t;

#include "celltypes.h"
#include "forcetypes.h"
#include "particletypes.h"
typedef std::pair<Int_t, md_cell_index_t> md_int_cell_t;

typedef md_int_cell_t md_rank_cell_t;
typedef md_int_cell_t md_iter_cell_t;
typedef std::pair<md_cell_index_t, md_cell_index_t> md_c1_c2_t;
typedef std::pair<Int_t, md_c1_c2_t> md_iter_c1_c2_t;
typedef std::pair<md_c1_c2_t, md_particle_index_t> md_c1_c2_particle_t;
typedef std::pair<Int_t, md_c1_c2_particle_t> md_iter_c1_c2_particle_t;
typedef std::pair<Int_t, md_particle_index_t> md_iter_particle_t;

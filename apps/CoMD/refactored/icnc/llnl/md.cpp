#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <map>
#include <set>
#include <math.h>
#include <assert.h>
#include <exception>
#include <tbb/tick_count.h>
#include <tbb/tbb_thread.h>

#ifdef _DIST_
#include <cnc/dist_cnc.h>
#include <cnc/internal/dist/distributor.h>
#else
#include <cnc/cnc.h>
#endif

#include <cnc/debug.h>
#include <cnc/reduce.h>

#include "utils/types.h"
#include "md.h"
#include "utils/io.h"
#include "steps/force.h"
#include "steps/redistribute.h"
#include "steps/velocity_position.h"

// Initialization step
int md_cell_init_grid::execute( const md_cell_index_t& cell, md_context& domain ) const
{

	bool perx = (domain.val_md_period_x == PERIODIC);
	bool pery = (domain.val_md_period_y == PERIODIC);
	bool perz = (domain.val_md_period_z == PERIODIC);

	md_particle_list_t pl;
	md_rank_cell_t rank_cell = std::make_pair(0, cell);
	domain.items_md_cell_init_particlelist.get(rank_cell, pl);

	Index_t xm, xp, ym, yp, zm, zp;
	Index_t cx, cy, cz;

	md_cell_index_wrapper_t cid = cell;

	md_int_vec3_t cell_coords = cid.get_coords();

	xm = xp = cell_coords.x;
	ym = yp = cell_coords.y;
	zm = zp = cell_coords.z;

	Index_t xbound = domain.val_md_simbox_cx-1;
	Index_t ybound = domain.val_md_simbox_cy-1;
	Index_t zbound = domain.val_md_simbox_cz-1;

	if(xm < 0 || xp > xbound || ym < 0 || yp > ybound || zm < 0 || zp > zbound)
	{
		fprintf(stderr, "Wrong cell id (%d, %d, %d)::(%d, %d, %d)::(%d, %d, %d)\n", xm, ym, zm, domain.val_md_simbox_cx, domain.val_md_simbox_cy, domain.val_md_simbox_cz, domain.val_md_period_x, domain.val_md_period_y, domain.val_md_period_z);
		_Exit(1);
	}

	if(perx || xm > 0) xm--;
	if(perx || xp < xbound) xp++;
	if(pery || ym > 0) ym--;
	if(pery || yp < ybound) yp++;
	if(perz || zm > 0) zm--;
	if(perz || zp < zbound) zp++;

	Index_t ii, jj, kk;
	Shift_t xshift, yshift, zshift;

	std::multimap<md_cell_index_wrapper_t, Shift_t> neighbor_shift_map;
	std::set<md_cell_index_wrapper_t> neighbor_set;

	for(int i = xm; i<=xp; i++) { // -1 <= xm <= xp <= xbound
		for(int j = ym; j<=yp; j++) { // -1 <= ym <= yp <= ybound
			for(int k = zm; k<=zp; k++) { // -1 <= zm <= zp <= zbound
				md_cell_index_wrapper_t nid = md_cell_index_wrapper_t(i, j, k);
				if(cid == nid) continue;
				ii = i; jj = j; kk = k;
				xshift = yshift = zshift = 0;
				if(i < 0) { ii = xbound; xshift = MINUS_X_SHIFT; }
				if(i > xbound) { ii = 0; xshift = PLUS_X_SHIFT;  }
				if(j < 0) {	jj = ybound; yshift = MINUS_Y_SHIFT; }
				if(j > ybound) { jj = 0; yshift = PLUS_Y_SHIFT;  }
				if(k < 0) { kk = zbound; zshift = MINUS_Z_SHIFT; }
				if(k > zbound) { kk = 0; zshift = PLUS_Z_SHIFT;  }
				Shift_t shift_vector = (xshift | yshift | zshift);
				md_cell_index_wrapper_t nid_new = md_cell_index_wrapper_t(ii, jj, kk);
				if(cid != nid_new) neighbor_set.insert(nid_new);
				if(cid >= nid_new) {
					neighbor_shift_map.insert(std::pair<md_cell_index_wrapper_t, Shift_t>(nid_new, shift_vector));
				}
			}
		}
	}

	md_cell_neighbor_list_t nl = std::make_shared< std::vector< md_cell_index_wrapper_t > > ();
	for(auto nid = neighbor_set.begin(); nid != neighbor_set.end(); ++nid) {
		nl->push_back(*nid);
		if(cid > (*nid)) {
			md_cell_neighbor_shift_list_t nsl = std::make_shared< std::vector< Shift_t > > ();
			std::pair <std::multimap<md_cell_index_wrapper_t, Shift_t>::iterator, std::multimap<md_cell_index_wrapper_t, Shift_t>::iterator> ret;
			ret = neighbor_shift_map.equal_range((*nid));
			for(auto nsv = ret.first; nsv != ret.second; ++nsv) { nsl->push_back(nsv->second); }
			md_c1_c2_t c1c2 = std::make_pair(cid.get_index(), nid->get_index());
			domain.items_md_c1_c2_constant_neighbor_shift_vector_list.put(c1c2, nsl);
		}
	}

	md_cell_neighbor_shift_list_t nsl = std::make_shared< std::vector< Shift_t > > ();
	nsl->push_back(0);
	if((perx && domain.val_md_simbox_cx == 1) || (pery && domain.val_md_simbox_cy == 1) || (perz && domain.val_md_simbox_cz == 1)) {
		std::pair <std::multimap<md_cell_index_wrapper_t, Shift_t>::iterator, std::multimap<md_cell_index_wrapper_t, Shift_t>::iterator> ret;
		ret = neighbor_shift_map.equal_range(cid);
		for(auto nsv = ret.first; nsv != ret.second; ++nsv) { nsl->push_back(nsv->second); }
	}
	md_c1_c2_t c1c1 = std::make_pair(cid.get_index(), cid.get_index());
	domain.items_md_c1_c2_constant_neighbor_shift_vector_list.put(c1c1, nsl);

	domain.items_md_cell_constant_neighbor_list.put(cell, nl);

	md_particle_list_shared_t pl_shared = std::make_shared< md_particle_list_t > ();
	for (auto p = pl.begin(); p != pl.end(); ++p) {
		pl_shared->push_back(*p);
	}

	// Initial force calculation
	md_iter_cell_t timestep0 = std::make_pair(0, cell);
	md_cell_timestep_compute_initial_force(timestep0, nl, nsl, pl_shared, domain);

	md_iter_cell_t timestep1 = std::make_pair(1, cell);

	domain.items_md_iter_cell_timestep_particlelist.put(timestep1, pl_shared);

	// Start main timestep loop
	domain.tags_md_iter_cell_timestep.put(timestep1);

	return CnC::CNC_Success;
}

// Velocity-position computation step
int md_iter_cell_timestep_velocity_position::execute( const md_iter_cell_t& iter_cell, md_context& domain ) const
{
	Index_t iter = iter_cell.first;
	md_cell_index_t cell = iter_cell.second;

	md_cell_neighbor_list_t nl; domain.items_md_cell_constant_neighbor_list.get(cell, nl);
	md_particle_list_shared_t plist; domain.items_md_iter_cell_timestep_particlelist.get(iter_cell, plist);

	// Gather all forces
	// From self
	md_iter_c1_c2_t ic1c1 = std::make_pair(iter-1, std::make_pair(cell, cell));
	md_force_energy_list_t fp; domain.items_md_iter_c1_c2_timestep_force_between_neighbors.get(ic1c1, fp);
	// From neighbors
	for(auto n = nl->begin(); n != nl->end(); ++n) {
		md_iter_c1_c2_t ic1c2 = std::make_pair(iter-1, std::make_pair(cell, n->get_index()));
		md_force_energy_list_t fp_temp; domain.items_md_iter_c1_c2_timestep_force_between_neighbors.get(ic1c2, fp_temp);
		for(Int_t f = 0; f < fp.size(); ++f) {
			fp[f] += fp_temp[f];
		}
	}

	//md_iter_cell_t ic = std::make_pair(iter-1, iter_cell.second);
	//md_force_energy_list_t fp; domain.items_md_iter_cell_timestep_force_per_cell.get(ic, fp);

	Real_t md_dt = domain.val_md_dt;
	Index_t md_num_iters = domain.val_md_num_iters;
	bool print_output = ((iter-1) % domain.val_md_num_print_interval == 0 || iter-1 == md_num_iters);
	Index_t i = 0;

	/****************************************************************************************/
	/*									On node parallelism									*/
	/****************************************************************************************/
	// Uncomment +4 for seuqential (comment other modes)
	i = 0;
	for (auto p = plist->begin() ; p != plist->end(); ++p, ++i) {
		md_cell_timestep_advance_velocity(iter-1, (*p), &fp[i], 0.5*md_dt, domain, print_output);
	}

	// Uncomment +1 for parallel_for (comment other modes)
	// CnC::parallel_for((Index_t)0, (Index_t)plist->size(), (Index_t)1, md_cell_timestep_advance_velocity_func(iter-1, plist, &fp[0], domain, 0.5*md_dt, print_output));

	/****************************************************************************************/

	if(iter <= md_num_iters) {
		md_particle_list_t particle_to_neighbors[nl->size()+1];
		for (auto p = plist->begin() ; p != plist->end(); ++p) {
			Int_t dest_cell = md_cell_timestep_advance_position(iter_cell, nl, (*p), md_dt, domain);
			if(dest_cell >= 0) particle_to_neighbors[dest_cell].push_back(*p);
		}

		md_cell_timestamp_redistribute_particles(iter_cell, particle_to_neighbors, nl, domain);
	}

	return CnC::CNC_Success;
}

// Particle redistribution step
int md_iter_cell_timestep_redistribute::execute( const md_iter_cell_t& iter_cell, md_context& domain ) const
{
	Index_t iter = iter_cell.first;
	Index_t md_num_iters = domain.val_md_num_iters;
	md_cell_index_wrapper_t cid = iter_cell.second;

	md_cell_neighbor_list_t nl; domain.items_md_cell_constant_neighbor_list.get(cid.get_index(), nl);

	md_c1_c2_t c1c1 = std::make_pair(cid.get_index(), cid.get_index());
	md_cell_neighbor_shift_list_t nsl; domain.items_md_c1_c2_constant_neighbor_shift_vector_list.get(c1c1, nsl);

	md_iter_c1_c2_t ic1c1 = std::make_pair(iter, std::make_pair(iter_cell.second, iter_cell.second));
	md_particle_list_t cpl; domain.items_md_iter_c1_c2_timestep_redistribute_particlelist.get(ic1c1, cpl);
	md_particle_list_shared_t pl_redist = std::make_shared< md_particle_list_t > ();
	pl_redist->insert(pl_redist->end(), cpl.begin(), cpl.end());

	for (auto n = nl->begin() ; n != nl->end(); ++n) {
		md_iter_c1_c2_t ic1c2 = std::make_pair(iter_cell.first, std::make_pair(iter_cell.second, (*n).get_index()));
		md_particle_list_t npl; domain.items_md_iter_c1_c2_timestep_redistribute_particlelist.get(ic1c2, npl);
		pl_redist->insert(pl_redist->end(), npl.begin(), npl.end());
	}

	// For prescheduling the step
	// Since neighboring cells are a dynamic dependency
	// domain.flush_gets();

	md_cell_timestep_compute_force(iter_cell, nl, nsl, pl_redist, domain);

	md_iter_cell_t timestep_plus_1 = std::make_pair(iter+1, iter_cell.second);
	domain.items_md_iter_cell_timestep_particlelist.put(timestep_plus_1, pl_redist);
	domain.tags_md_iter_cell_timestep.put(timestep_plus_1);

	return CnC::CNC_Success;
}

// Force computation step
int md_iter_c1_c2_timestep_compute_force::execute( const md_iter_c1_c2_t& iter_c1_c2, md_context& domain ) const
{
	// c1 > c2
	Index_t iter = iter_c1_c2.first;
	md_cell_index_wrapper_t c1 = iter_c1_c2.second.first;
	md_cell_index_wrapper_t c2 = iter_c1_c2.second.second;

	md_cell_index_t c1i = c1.get_index();
	md_cell_index_t c2i = c2.get_index();

	md_particle_attr_list_t pl_c1; domain.items_md_iter_c1_c2_timestep_compute_force_local_particleattrlist.get(iter_c1_c2, pl_c1);
	md_particle_attr_list_t pl_c2; domain.items_md_iter_c1_c2_timestep_compute_force_particleattrlist.get(iter_c1_c2, pl_c2);

	md_c1_c2_t c1c2 = std::make_pair(c1.get_index(), c2.get_index());
	md_cell_neighbor_shift_list_t nsl; domain.items_md_c1_c2_constant_neighbor_shift_vector_list.get(c1c2, nsl);

	md_cell_timestep_compute_forces_between_particles(iter, c1.get_index(), c2.get_index(), nsl, pl_c1, pl_c2, domain);
	return CnC::CNC_Success;
}

/**************************************************************************************************************************/
//TODO: This is just a makeshift implementation to get the output. Optimize it!!!
// Output step (write to file)
int md_iter_timestep_write_output::execute( const Int_t& iter, md_context& domain ) const
{
	std::string str_iter; domain.items_md_iter_write_output_str_all_particle_values.get(iter, str_iter);
	Real_t pe; domain.items_md_iter_write_output_potential_energy.get(iter, pe);
	Real_t ke; domain.items_md_iter_write_output_kinetic_energy.get(iter, ke);

	std::stringstream str_pval;
	str_pval << "ITEM: TIMESTEP\n" << iter << "\n";
	str_pval << "ITEM: NUMBER OF ATOMS\n" << domain.val_md_num_particles << "\n";
	str_pval << "ITEM: BOX BOUNDS " << (domain.val_md_period_x == 0 ? "ff " : "pp ") << (domain.val_md_period_y == 0 ? "ff " : "pp ") << (domain.val_md_period_z == 0 ? "ff\n" : "pp\n");
	str_pval << domain.val_md_simbox_lo_x << ' ' << domain.val_md_simbox_hi_x << "\n";
	str_pval << domain.val_md_simbox_lo_y << ' ' << domain.val_md_simbox_hi_y << "\n";
	str_pval << domain.val_md_simbox_lo_z << ' ' << domain.val_md_simbox_hi_z << "\n";
	str_pval << "ITEM: ATOMS id mass x y z vx vy vz fx fy fz\n" << str_iter;
	str_pval << "ITEM: POTENTIAL ENERGY\n" << pe << "\n";
	str_pval << "ITEM: KINETIC ENERGY\n" << ke << "\n";

	std::stringstream dest_file_path;
	dest_file_path << domain.val_md_outputdir << "/" << iter << ".itr";

	std::ofstream dest_file;
	dest_file.open (dest_file_path.str().c_str());
	dest_file << str_pval.str().c_str();
	dest_file.close();

	return CnC::CNC_Success;
}
/**************************************************************************************************************************/

int main(int argc, char* argv[])
{
#ifdef _DIST_
    CnC::dist_cnc_init< md_context > dc_init;
#endif

	Real_t epsilon = 1.0;
	std::stringstream e_str(argv[3]);
	e_str >> epsilon;
	Real_t sigma   = 1.0;
	std::stringstream s_str(argv[4]);
	s_str >> sigma;
	Real_t pot 	   = 0.0;
	std::stringstream p_str(argv[5]);
	p_str >> pot;
	Real_t cutoff  = 0.0;
	std::stringstream cut_str(argv[6]);
	cut_str >> cutoff;

	Index_t cx = 3;
	std::stringstream cx_str(argv[7]);
	cx_str >> cx;
	Index_t cy = 3;
	std::stringstream cy_str(argv[8]);
	cy_str >> cy;
	Index_t cz = 3;
	std::stringstream cz_str(argv[9]);
	cz_str >> cz;

	Index_t num_iters = 1000;
	std::stringstream i_str(argv[10]);
	i_str >> num_iters;
	Index_t print_iters = 100;
	std::stringstream print_str(argv[11]);
	print_str >> print_iters;
	Real_t timestep = 0.001;
	std::stringstream t_str(argv[12]);
	t_str >> timestep;

	assert(num_iters >= print_iters);

	//usage: md/distmd <input-file> <output-dir> <epsilon> <sigma> <cutoff> <shift-pot> <num-iterations> <print-interval> <timestep> <numcells-x> <numcells-y> <numcells-z>
	md_context domain(std::string(argv[2]), epsilon, sigma, cutoff, pot, num_iters, print_iters, timestep, cx, cy, cz);

	// Start timer
    tbb::tick_count t0 = tbb::tick_count::now();

	read_file(argv[1], domain);

	// Initialize reduction counts for energy
	for (Int_t i = 0; i <= num_iters; i++) {
		if (i % print_iters == 0 || i == num_iters) domain.items_md_iter_write_output_red_counts.put(i, domain.val_md_num_particles);
	}
    domain.wait();

	// End timer
	tbb::tick_count t1 = tbb::tick_count::now();
    std::cout << "Particles: " << domain.val_md_num_particles << ", " <<  " Total Time: " << (t1-t0).seconds() << " sec" << std::endl;
}

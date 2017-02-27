#include "XSbench_header.h"

// Generates randomized energy grid for each nuclide
// Note that this is done as part of initialization (serial), so
// rand() is used.
void generate_grids( NuclideGridPoint ** nuclide_grids,
                     long n_isotopes, long n_gridpoints ) {
	for( long i = 0; i < n_isotopes; i++ )
		for( long j = 0; j < n_gridpoints; j++ )
		{
			nuclide_grids[i][j].energy       =((double)rand()/(double)RAND_MAX);
			nuclide_grids[i][j].total_xs     =((double)rand()/(double)RAND_MAX);
			nuclide_grids[i][j].elastic_xs   =((double)rand()/(double)RAND_MAX);
			nuclide_grids[i][j].absorbtion_xs=((double)rand()/(double)RAND_MAX);
			nuclide_grids[i][j].fission_xs   =((double)rand()/(double)RAND_MAX);
			nuclide_grids[i][j].nu_fission_xs=((double)rand()/(double)RAND_MAX);
		}
}

// Verification version of this function (tighter control over RNG)
void generate_grids_v( NuclideGridPoint ** nuclide_grids,
                     long n_isotopes, long n_gridpoints, u64* seed ) {
	for( long i = 0; i < n_isotopes; i++ )
		for( long j = 0; j < n_gridpoints; j++ )
		{
			nuclide_grids[i][j].energy       = rn_v(seed);
			nuclide_grids[i][j].total_xs     = rn_v(seed);
			nuclide_grids[i][j].elastic_xs   = rn_v(seed);
			nuclide_grids[i][j].absorbtion_xs= rn_v(seed);
			nuclide_grids[i][j].fission_xs   = rn_v(seed);
			nuclide_grids[i][j].nu_fission_xs= rn_v(seed);
		}
}

// Sorts the nuclide grids by energy (lowest -> highest)
void sort_nuclide_grids( NuclideGridPoint ** nuclide_grids, long n_isotopes,
                         long n_gridpoints )
{
	int (*cmp) (const void *, const void *);
	cmp = NGP_compare;

	for( long i = 0; i < n_isotopes; i++ )
		qsort( nuclide_grids[i], n_gridpoints, sizeof(NuclideGridPoint),
		       cmp );

	// error debug check
	/*
	for( int i = 0; i < n_isotopes; i++ )
	{
		PRINTF("NUCLIDE %d ==============================\n", i);
		for( int j = 0; j < n_gridpoints; j++ )
			PRINTF("E%d = %lf\n", j, nuclide_grids[i][j].energy);
	}
	*/
}

void assignEnergyGridXsPtrs( int* full, GridPoint* energy_grid, long n_unionized_grid_points, int n)
{
	for( long i = 0; i < n_unionized_grid_points; i++ )
		energy_grid[i].xs_ptrs = &full[n * i];
}

// Allocates unionized energy grid, and assigns union of energy levels
// from nuclide grids to it.
GridPoint * generate_energy_grid( ocrDBK_t* DBK_uEnergy_grid,  ocrDBK_t* DBK_xs_grid, long n_isotopes, long n_gridpoints,
                                  NuclideGridPoint ** nuclide_grids, int mype ) {
	if( mype == 0 ) PRINTF("Generating Unionized Energy Grid...\n");

	long n_unionized_grid_points = n_isotopes*n_gridpoints;
	int (*cmp) (const void *, const void *);
	cmp = NGP_compare;

	GridPoint * energy_grid; // = (GridPoint *)malloc( n_unionized_grid_points
	                         //                      * sizeof( GridPoint ) );
    ocrDbCreate( DBK_uEnergy_grid, (void **) &energy_grid, n_unionized_grid_points*sizeof(NuclideGridPoint),
                    0, NULL_HINT, NO_ALLOC );
	if( mype == 0 ) PRINTF("Copying and Sorting all nuclide grids...\n");

    ocrDBK_t DBK_nuclide_grids, DBK_nuclide_grid_ptrs;
	NuclideGridPoint ** n_grid_sorted = gpmatrix( &DBK_nuclide_grids, &DBK_nuclide_grid_ptrs,
                                                    n_isotopes, n_gridpoints );

	memcpy( n_grid_sorted[0], nuclide_grids[0], n_isotopes*n_gridpoints*
	                                      sizeof( NuclideGridPoint ) );

	qsort( &n_grid_sorted[0][0], n_unionized_grid_points,
	       sizeof(NuclideGridPoint), cmp);

	if( mype == 0 ) PRINTF("Assigning energies to unionized grid...\n");

	for( long i = 0; i < n_unionized_grid_points; i++ )
		energy_grid[i].energy = n_grid_sorted[0][i].energy;


	//gpmatrix_free(n_grid_sorted);
    ocrDbDestroy(DBK_nuclide_grids);
    ocrDbDestroy(DBK_nuclide_grid_ptrs);

	int * full; // = (int *) malloc( n_isotopes * n_unionized_grid_points
	            //                 * sizeof(int) );

    ocrDbCreate( DBK_xs_grid, (void **) &full, n_isotopes*n_unionized_grid_points*sizeof(int),
                    0, NULL_HINT, NO_ALLOC );

	if( full == NULL )
	{
		PRINTF("ERROR - Out Of Memory!\n");
	}

    assignEnergyGridXsPtrs( full, energy_grid, n_unionized_grid_points, n_isotopes);

	// debug error checking

    /*
	for( int i = 0; i < n_unionized_grid_points; i++ )
		PRINTF("E%d = %f\n", i, energy_grid[i].energy);
    */

	return energy_grid;
}

// Searches each nuclide grid for the closest energy level and assigns
// pointer from unionized grid to the correct spot in the nuclide grid.
// This process is time consuming, as the number of binary searches
// required is:  binary searches = n_gridpoints * n_isotopes^2
void set_grid_ptrs( GridPoint * energy_grid, NuclideGridPoint ** nuclide_grids,
                    long n_isotopes, long n_gridpoints, int mype )
{
	if( mype == 0 ) PRINTF("Assigning pointers to Unionized Energy Grid...\n");

	//#pragma omp parallel for default(none) \
	//shared( energy_grid, nuclide_grids, n_isotopes, n_gridpoints, mype )
	for( long i = 0; i < n_isotopes * n_gridpoints ; i++ )
	{
		double quarry = energy_grid[i].energy;
		if( INFO && mype == 0 && i % 10000 == 0 )
			PRINTF( "\rAligning Unionized Grid...(%.2f%% complete)",
			       100.0 * (double) i / (n_isotopes*n_gridpoints) );
		for( long j = 0; j < n_isotopes; j++ )
		{
			// j is the nuclide i.d.
			// log n binary search
			energy_grid[i].xs_ptrs[j] =
				binary_search( nuclide_grids[j], quarry, n_gridpoints);
		}
	}
	if( mype == 0 ) PRINTF("\n");

	//test
	/*
	for( int i=0; i < n_isotopes * n_gridpoints; i++ )
		for( int j = 0; j < n_isotopes; j++ )
			PRINTF("E = %.4lf\tNuclide %d->%p->%.4lf\n",
			       energy_grid[i].energy,
                   j,
				   energy_grid[i].xs_ptrs[j],
				   (energy_grid[i].xs_ptrs[j])->energy
				   );
	*/
}

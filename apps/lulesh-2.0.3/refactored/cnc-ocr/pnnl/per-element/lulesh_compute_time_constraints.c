#include "lulesh.h"

/**
 * Step function defintion for "compute_time_constraints"
 */
void lulesh_compute_time_constraints(cncTag_t iteration, cncTag_t element_id, double sound_speed, double volume_derivative, double characteristic_length, luleshCtx *ctx) {

	// Constants
	double qqc = ctx->constants.qqc;
	double dvovmax = ctx->constants.dvovmax;

	// Compute
	// [CalcTimeConstraintsForElems]
	double qqc2 = 64.0 * qqc * qqc;
	double dtcourant = 1.0e+20;
	double dthydro = 1.0e+20;
	double dtf = sound_speed * sound_speed;

	if (volume_derivative < 0.0) {
		dtf = dtf + qqc2 * volume_derivative * volume_derivative *
				characteristic_length * characteristic_length;
	}

	dtf = sqrt(dtf) ;
	dtf = characteristic_length / dtf ;

	if (volume_derivative != 0.0) {
		dtcourant = dtf;
	}

	// [CalcHydroConstraintForElems]
	if (volume_derivative != 0.0) {
		dthydro = dvovmax / (fabs(volume_derivative)+1.e-20);
	}


	//
	// OUTPUTS
	//

	// Put "courant_out" items
	double *courant_out = cncItemAlloc(sizeof(*courant_out));
	*courant_out = dtcourant;
	cncPut_courant(courant_out, iteration, element_id, ctx);

	// Put "hydro_out" items
	double *hydro_out = cncItemAlloc(sizeof(*hydro_out));
	*hydro_out = dthydro;
	cncPut_hydro(hydro_out, iteration, element_id, ctx);

}

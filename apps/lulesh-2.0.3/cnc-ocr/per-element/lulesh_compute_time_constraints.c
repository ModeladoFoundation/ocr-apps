#include "lulesh.h"

/*
 * typeof sound_speed is double
 * typeof volume_derivative is double
 * typeof characteristic_length is double
 */
void compute_time_constraints(cncTag_t iteration, cncTag_t element_id, sound_speedItem sound_speed_in, volume_derivativeItem volume_derivative_in, characteristic_lengthItem characteristic_length_in, luleshCtx *ctx) {

	double sound_speed = sound_speed_in.item;
	double volume_derivative = volume_derivative_in.item;
	double characteristic_length = characteristic_length_in.item;

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
	double *courant_out;
	cncHandle_t courant_outHandle = cncCreateItem_courant(&courant_out);
	*courant_out = dtcourant;
	cncPut_courant(courant_outHandle, iteration, element_id, ctx);

	// Put "hydro_out" items
	double *hydro_out;
	cncHandle_t hydro_outHandle = cncCreateItem_hydro(&hydro_out);
	*hydro_out = dthydro;
	cncPut_hydro(hydro_outHandle, iteration, element_id, ctx);

}

#include "lulesh.h"

/**
 * Step function defintion for "compute_energy"
 */
void lulesh_compute_energy(cncTag_t iteration, cncTag_t element_id, double volume, double previous_volume, double previous_energy, double previous_pressure, double previous_viscosity, double qlin, double qquad, luleshCtx *ctx) {

	// Constants
	double eosvmin = ctx->constants.eosvmin;
	double eosvmax = ctx->constants.eosvmax;
	double emin = ctx->constants.emin;
	double pmin = ctx->constants.pmin;
	double rho0 = ctx->constants.refdens;

	// Compute

	double delv = volume - previous_volume;

	// [ApplyMaterialPropertiesForElems]

	// Bound the updated and previous relative volumes with eosvmin/max
	if (eosvmin != 0.0) {
		if (volume < eosvmin) {
			volume = eosvmin;
		} if (previous_volume < eosvmin)
			previous_volume = eosvmin;
	}

	if (eosvmax != 0.0) {
		if (volume > eosvmax) {
			volume = eosvmax;
		}
		if (previous_volume > eosvmax)
			previous_volume = eosvmax;
	}

	// [EvalEOSForElems]
	double compression = 1.0 / volume - 1.;
	double vchalf = volume - delv * .5;
	double comp_half_step = 1.0 / vchalf - 1.0;
	double c1s = 2.0/3.0;
	double work = 0.0;
	const double sixth = 1.0 / 6.0;
	double pressure, viscosity, q_tilde;

	// Check for v > eosvmax or v < eosvmin
	if (eosvmin != 0.0 ) {
		if (volume <= eosvmin) {
			comp_half_step = compression; // impossible due to calling func?
		}
	}
	if (eosvmax != 0.0) {
		if (volume >= eosvmax) { // impossible due to calling func?
			previous_pressure        = 0.0;
			compression  = 0.0;
			comp_half_step = 0.0;
		}
	}

	double energy = previous_energy - 0.5 * delv *
			(previous_pressure + previous_viscosity) + 0.5 * work;

	if (energy  < emin) {
		energy = emin ;
	}

	// [CalcPressureForElems]
	double bvc = c1s * (comp_half_step + 1.0);
	double p_half_step = bvc * energy ;

	if(fabs(p_half_step) <  ctx->cutoffs.p)
		p_half_step = 0.0;
	if(volume >= eosvmax) // impossible condition here?
		p_half_step = 0.0;
	if(p_half_step < pmin)
		p_half_step = pmin;

	double vhalf = 1.0 / (1.0 + comp_half_step);
	if (delv > 0.0) {
		viscosity = 0.0; // = qq_old = ql_old
	} else {
		double ssc = ( c1s * energy + vhalf * vhalf * bvc * p_half_step ) / rho0;
		if (ssc <= .1111111e-36) {
			ssc = .3333333e-18;
		} else {
			ssc = sqrt(ssc);
		}
		viscosity = (ssc*qlin + qquad) ;
	}


	energy = energy + 0.5 * delv * (3.0 * (previous_pressure + previous_viscosity)
			- 4.0 * (p_half_step + viscosity));

	energy += 0.5 * work;

	if (fabs(energy) < ctx->cutoffs.e) {
		energy = 0.0;
	}
	if(energy  < emin ) {
		energy = emin ;
	}

	// [CalcPressureForElems]
	bvc = c1s * (compression + 1.0);
	pressure = bvc * energy;

	if(fabs(pressure) <  ctx->cutoffs.p)
		pressure = 0.0;
	if(volume >= eosvmax ) // impossible condition here?
		pressure = 0.0;
	if(pressure < pmin)
		pressure = pmin;

	if (delv > 0.0) {
		q_tilde = 0.0;
	} else {
		double ssc = ( c1s * energy + volume * volume * bvc * pressure ) / rho0;
		if (ssc <= .1111111e-36) {
			ssc = .3333333e-18;
		} else {
			ssc = sqrt(ssc) ;
		}
		q_tilde = (ssc*qlin + qquad) ;
	}

	energy = energy - (7.0 * (previous_pressure + previous_viscosity) - 8.0 *
			(p_half_step + viscosity) + (pressure + q_tilde)) * delv * sixth;

	if (fabs(energy) < ctx->cutoffs.e) {
		energy = 0.0;
	}
	if (energy  < emin) {
		energy = emin ;
	}

	// [CalcPressureForElems]
	bvc = c1s * (compression + 1.0);
	pressure = bvc * energy;

	if(fabs(pressure) <  ctx->cutoffs.p)
		pressure = 0.0;

	if(volume >= eosvmax) // impossible condition here?
		pressure = 0.0;
	if(pressure < pmin)
		pressure = pmin;

	if (delv <= 0.0) {
		double ssc = (c1s * energy + volume * volume * bvc * pressure ) / rho0 ;
		if (ssc <= .1111111e-36) {
			ssc = .3333333e-18;
		} else {
			ssc = sqrt(ssc);
		}
		viscosity = (ssc*qlin + qquad) ;
		if (fabs(viscosity) < ctx->cutoffs.q) viscosity = 0.0;
	}

	double sound_speed = (c1s * energy + volume * volume * bvc * pressure) / rho0;
	if (sound_speed <= .1111111e-36) {
		sound_speed = .3333333e-18;
	} else {
		sound_speed = sqrt(sound_speed);
	}


    //
    // OUTPUTS
    //

    // Put "energy_out" items
    double *energy_out = cncItemAlloc(sizeof(*energy_out));
	*energy_out = energy;
    cncPut_energy(energy_out, iteration, element_id, ctx);

    // Put "pressure_out" items
    double *pressure_out = cncItemAlloc(sizeof(*pressure_out));
	*pressure_out = pressure;
    cncPut_pressure(pressure_out, iteration, element_id, ctx);

    // Put "viscosity_out" items
    double *viscosity_out = cncItemAlloc(sizeof(*viscosity_out));
	*viscosity_out = viscosity;
    cncPut_viscosity(viscosity_out, iteration, element_id, ctx);

    // Put "sound_speed_out" items
    double *sound_speed_out = cncItemAlloc(sizeof(*sound_speed_out));
	*sound_speed_out = sound_speed;
    cncPut_sound_speed(sound_speed_out, iteration, element_id, ctx);


}

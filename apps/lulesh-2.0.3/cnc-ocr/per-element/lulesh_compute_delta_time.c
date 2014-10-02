#include "lulesh.h"

/*
 * typeof delta_time is double
 * typeof elapsed_time is double
 * typeof courant is double
 * typeof hydro is double
 */
void compute_delta_time(cncTag_t iteration, delta_timeItem previous_delta_time_in, elapsed_timeItem previous_elapsed_time_in, courantItem *courant_in, hydroItem *hydro_in, luleshCtx *ctx) {

	double previous_delta_time = previous_delta_time_in.item;
	double previous_elapsed_time = previous_elapsed_time_in.item;

	//
	// INPUTS
	//
	double min_courant = 1e20;
	double min_hydro = 1e20;

	{ // Access "courant" inputs, "hydro" inputs
		s64 _element_id;
		for (_element_id = 0; _element_id < ((ELEMENTS)-(0)); ++_element_id) {
			if(courant_in[_element_id].item < min_courant)
				min_courant = courant_in[_element_id].item;
			if(hydro_in[_element_id].item < min_hydro)
				min_hydro = hydro_in[_element_id].item;
		}
	}

	// Constants
	double stop_time = ctx->constraints.stop_time;
	double max_delta_time = ctx->constraints.max_delta_time;

	// Compute
	double delta_time = 0;
	double dtfixed = -1.0e-6; // *edgeElems*tp/45.0) ;

	double deltatimemultlb = 1.1;
	double deltatimemultub = 1.2;

	double targetdt = stop_time - previous_elapsed_time;

	if ((dtfixed <= 0.0) && (iteration != 0)) {
		double ratio;

		// This will require a reduction in parallel
		double gnewdt = 1.0e+20;

		if (min_courant < gnewdt)
			gnewdt = min_courant / 2.0;

		if (min_hydro < gnewdt)
			gnewdt = min_hydro * 2.0 / 3.0;

		delta_time = gnewdt;
		ratio = delta_time / previous_delta_time;

		if (ratio >= 1.0) {
			if (ratio < deltatimemultlb) {
				delta_time = previous_delta_time ;
			} else if (ratio > deltatimemultub) {
				delta_time= previous_delta_time * deltatimemultub;
			}
		}

		if (delta_time > max_delta_time) {
			delta_time = max_delta_time;
		}

	} else {
		delta_time = previous_delta_time;

	}

	// TRY TO PREVENT VERY SMALL SCALING ON THE NEXT CYCLE
	if ((targetdt > delta_time) && (targetdt < (4.0 * delta_time / 3.0)) )
		targetdt = 2.0 * delta_time / 3.0;

	if (targetdt < delta_time)
		delta_time = targetdt ;

	if(previous_elapsed_time < ctx->constraints.stop_time &&
			(iteration + 1) < ctx->constraints.maximum_iterations) {

		double elapsed_time = previous_elapsed_time + delta_time;

		//
		// OUTPUTS
		//

		// Put "delta_time_out" items

		double *delta_time_out;
		cncHandle_t delta_time_outHandle = cncCreateItem_delta_time(&delta_time_out);
		*delta_time_out = delta_time;
		cncPut_delta_time(delta_time_outHandle, iteration + 1, ctx);

		// Put "elapsed_time_out" items
		double *elapsed_time_out;
		cncHandle_t elapsed_time_outHandle = cncCreateItem_elapsed_time(&elapsed_time_out);
		*elapsed_time_out = elapsed_time;
		cncPut_elapsed_time(elapsed_time_outHandle, iteration + 1, ctx);

		//
		// PRESCRIBE STEPS
		//

		int node_id;
		for(node_id = 0; node_id < ctx->mesh.number_nodes; ++node_id ) {
			cncPrescribe_reduce_force(iteration + 1, node_id, ctx);
			cncPrescribe_compute_velocity(iteration + 1, node_id, ctx);
			cncPrescribe_compute_position(iteration + 1, node_id, ctx);
		}

		int element_id;
		for(element_id = 0; element_id < ctx->mesh.number_elements; ++element_id ) {
			cncPrescribe_compute_stress_partial(iteration + 1, element_id, ctx);
			cncPrescribe_compute_hourglass_partial(iteration + 1, element_id, ctx);
			cncPrescribe_compute_volume(iteration + 1, element_id, ctx);
			cncPrescribe_compute_volume_derivative(iteration + 1, element_id, ctx);
			cncPrescribe_compute_gradients(iteration + 1, element_id, ctx);
			cncPrescribe_compute_viscosity_terms(iteration + 1, element_id, ctx);
			cncPrescribe_compute_energy(iteration + 1, element_id, ctx);
			cncPrescribe_compute_characteristic_length(iteration + 1, element_id, ctx);
			cncPrescribe_compute_time_constraints(iteration + 1, element_id, ctx);
		}

		cncPrescribe_compute_delta_time(iteration + 1, ctx);

	} else {
		// Done
		cncPrescribe_produce_output(iteration, ctx);

	}
	printf("Iteration %d\n", iteration);

}

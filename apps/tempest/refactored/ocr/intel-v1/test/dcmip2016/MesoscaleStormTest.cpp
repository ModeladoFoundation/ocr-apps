///////////////////////////////////////////////////////////////////////////////
///
///	\file	MesoscaleStormTest.cpp
///	\author  Paul Ullrich
///	\version June 23, 2013
///
///	<remarks>
///		Copyright 2000-2010 Paul Ullrich
///
///		This file is distributed as part of the Tempest source code package.
///		Permission is granted to use, copy, modify and distribute this
///		source code and its documentation under the terms of the GNU General
///		Public License.  This software is provided "as is" without express
///		or implied warranty.
///	</remarks>

#include "Tempest.h"

#include "KesslerPhysics.h"

///////////////////////////////////////////////////////////////////////////////

extern "C" {
	void mesoscale_storm_init(
	);

	void mesoscale_storm_test(
		double * dLon,
		double * dLat,
		double * dP,
		double * dZ,
		int * iZcoords,
		double * dU,
		double * dV,
		double * dT,
		double * dThetaV,
		double * dPs,
		double * dRho,
		double * dQ
	);
}

///////////////////////////////////////////////////////////////////////////////

///	<summary>
///		DCMIP 2016: Tropical Cyclone Test
///	</summary>
class MesoscaleStormTest : public TestCase {

protected:
	///	<summary>
	///		Model cap.
	///	</summary>
	double m_dZtop;

	///	<summary>
	///		Earth radius scaling parameter.
	///	</summary>
	double m_dEarthScaling;

public:
	///	<summary>
	///		Constructor.
	///	</summary>
	MesoscaleStormTest(
		double dZtop,
		double dEarthScaling
	) :
		m_dZtop(dZtop),
		m_dEarthScaling(dEarthScaling)
	{
		mesoscale_storm_init();
	}

public:
	///	<summary>
	///		Number of tracers used in this test.
	///	</summary>
	virtual int GetTracerCount() const {
		return 3;
	}

	///	<summary>
	///		Get the altitude of the model cap.
	///	</summary>
	virtual double GetZtop() const {
		return m_dZtop;
	}

	///	<summary>
	///		Flag indicating that a reference state is available.
	///	</summary>
	virtual bool HasReferenceState() const {
		return true;
	}

	///	<summary>
	///		Obtain test case specific physical constants.
	///	</summary>
	virtual void EvaluatePhysicalConstants(
		PhysicalConstants & phys
	) const {
		phys.SetEarthRadius(phys.GetEarthRadius() / m_dEarthScaling);
		phys.SetOmega(0.0);
	}

	///	<summary>
	///		Evaluate the topography at the given point.
	///	</summary>
	virtual double EvaluateTopography(
		const PhysicalConstants & phys,
		double dLon,
		double dLat
	) const {
		return (0.0);
	}

	///	<summary>
	///		Evaluate the reference state at the given point.
	///	</summary>
	virtual void EvaluateReferenceState(
		const PhysicalConstants & phys,
		double dZ,
		double dLon,
		double dLat,
		double * dState
	) const {

		// Store the state
		// State 0 = Zonal velocity (m/s)
		// State 1 = Meridional velocity (m/s)
		// State 2 = Theta (K)
		// State 3 = Vertical velocity (m/s)
		// State 4 = Density (kg/m^3)
		dState[0] = 0.0;
		dState[1] = 0.0;
		dState[2] = 0.0;
		dState[3] = 0.0;
		dState[4] = 0.0;
	}

	///	<summary>
	///		Evaluate the state vector at the given point.
	///	</summary>
	virtual void EvaluatePointwiseState(
		const PhysicalConstants & phys,
		const Time & time,
		double dZ,
		double dLon,
		double dLat,
		double * dState,
		double * dTracer
	) const {

		int iZcoords = 1;

		double dX = m_dEarthScaling;
		double dP;
		double dU;
		double dV;
		double dT;
		double dThetaV;
		double dPhis;
		double dPs;
		double dRho;
		double dQ;

		// Calculate the reference state
		//EvaluateReferenceState(phys, dZ, dLon, dLat, dState);
		mesoscale_storm_test(
			&dLon,
			&dLat,
			&dP,
			&dZ,
			&iZcoords,
			&dU,
			&dV,
			&dT,
			&dThetaV,
			&dPs,
			&dRho,
			&dQ);

		dState[0] = dU;
		dState[1] = dV;
		dState[2] = dThetaV;
		dState[3] = 0.0;
		dState[4] = dRho;

		dTracer[0] = dRho * dQ;
		dTracer[1] = 0.0;
		dTracer[2] = 0.0;
	}
};

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {

	// Initialize Tempest
	TempestInitialize(&argc, &argv);


try {
	// Model cap
	double dZtop;

	// Earth radius scaling parameter
	double dEarthScaling;

	// Parse the command line
	BeginTempestCommandLine("MesoscaleStormTest");
		SetDefaultResolution(20);
		SetDefaultLevels(10);
		SetDefaultOutputDeltaT("60s");
		SetDefaultDeltaT("60s");
		SetDefaultEndTime("60s");
		SetDefaultHorizontalOrder(4);
		SetDefaultVerticalOrder(1);

		CommandLineDouble(dZtop, "ztop", 10000.0);
		CommandLineDouble(dEarthScaling, "X", 120.0);

		ParseCommandLine(argc, argv);
	EndTempestCommandLine(argv)

	// Setup the Model
	AnnounceBanner("MODEL SETUP");

    EquationSet eqn(EquationSet::PrimitiveNonhydrostaticEquations);

    eqn.InsertTracer("RhoQv", "RhoQv");
    eqn.InsertTracer("RhoQc", "RhoQc");
    eqn.InsertTracer("RhoQr", "RhoQr");

	Model model(eqn);

	TempestSetupCubedSphereModel(model);

	model.GetGrid()->SetReferenceLength(
		model.GetGrid()->GetReferenceLength() / dEarthScaling);

	// Set the test case for the model
	AnnounceStartBlock("Initializing test case");
	model.SetTestCase(
		new MesoscaleStormTest(
			dZtop,
			dEarthScaling));
	AnnounceEndBlock("Done");

	// Add Kessler physics
	model.AttachWorkflowProcess(
		new KesslerPhysics(
			model,
			model.GetDeltaT()));

	// Begin execution
	AnnounceBanner("SIMULATION");
	model.Go();

	// Compute error norms
	AnnounceBanner("RESULTS");
	model.ComputeErrorNorms();
	AnnounceBanner();

} catch(Exception & e) {
	std::cout << e.ToString() << std::endl;
}

	// Deinitialize Tempest
	TempestDeinitialize();
}

///////////////////////////////////////////////////////////////////////////////


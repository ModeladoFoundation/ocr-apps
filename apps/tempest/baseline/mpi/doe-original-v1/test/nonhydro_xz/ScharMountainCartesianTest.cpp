///////////////////////////////////////////////////////////////////////////////
///
///	\file    ScharMountainCartesianTest.cpp
///	\author  Paul Ullrich, Jorge Guerra
///	\version January 4, 2014
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

///////////////////////////////////////////////////////////////////////////////

///	<summary>
///		Giraldo et al. (2007)
///
///		Schar Mountain Uniform Flow test case.
///	</summary>
class ScharMountainCartesianTest : public TestCase {

public:
	/// <summary>
	///		Grid dimension array (FOR CARTESIAN GRIDS).
	///	</summary>
	double m_dGDim[6];

	///	<summary>
	///		Parameter reference height for topography disturbance
	///	</summary>
	double m_dhC;

private:
	///	<summary>
	///		Uniform +X flow field.
	///	</summary>
	double m_dU0;

	///	<summary>
	///		Brunt-Vaisala frequency
	///	</summary>
	double m_dNbar;

	///	<summary>
	///		Reference pontential temperature
	///	</summary>
	double m_dTheta0;

	///	<summary>
	///		Parameter factor for temperature disturbance
	///	</summary>
	double m_dThetaC;

	///	<summary>
	///		Parameter reference length a for temperature disturbance
	///	</summary>
	double m_daC;

	///	<summary>
	///		Parameter reference length for mountain profile
	///	</summary>
	double m_dlC;

	///	<summary>
	///		Parameter Archimede's Constant (essentially Pi but to some digits)
	///	</summary>
	double m_dpiC;

	///	<summary>
	///		Flag indicating that Rayleigh friction is inactive.
	///	</summary>
	bool m_fNoRayleighFriction;

public:
	///	<summary>
	///		Constructor. (with physical constants defined privately here)
	///	</summary>
	ScharMountainCartesianTest(
		double dU0,
		double dNbar,
		double dTheta0,
		double dThetaC,
		double dhC,
		double daC,
		double dlC,
		double dpiC,
		bool fNoRayleighFriction
	) :
		m_dU0(dU0),
		m_dNbar(dNbar),
		m_dTheta0(dTheta0),
		m_dThetaC(dThetaC),
		m_dhC(dhC),
		m_daC(daC),
		m_dlC(dlC),
		m_dpiC(dpiC),
		m_fNoRayleighFriction(fNoRayleighFriction)
	{
		// Set the dimensions of the box
		m_dGDim[0] = -25000.;
		m_dGDim[1] = 25000.0;
		m_dGDim[2] = -1000.0;
		m_dGDim[3] = 1000.0;
		m_dGDim[4] = 0.0;
		m_dGDim[5] = 21000.0;
	}

public:
	///	<summary>
	///		Number of tracers used in this test.
	///	</summary>
	virtual int GetTracerCount() const {
		return 0;
	}

	///	<summary>
	///		Get the altitude of the model cap.
	///	</summary>
	virtual double GetZtop() const {
		return m_dGDim[5];
	}

	///	<summary>
	///		Obtain test case specific physical constants.
	///	</summary>
	virtual void EvaluatePhysicalConstants(
		PhysicalConstants & phys
	) const {
		// No Coriolis
		phys.SetOmega(0.0);
	}

	///	<summary>
	///		Flag indicating that a reference state is available.
	///	</summary>
	virtual bool HasReferenceState() const {
		return true;
	}

	///	<summary>
	///		Flag indicating whether or not Rayleigh friction strength is given.
	///	</summary>
	virtual bool HasRayleighFriction() const {
		return !m_fNoRayleighFriction;
	}

	///	<summary>
	///		Evaluate the topography at the given point. (cartesian version)
	///	</summary>
	virtual double EvaluateTopography(
       const PhysicalConstants & phys,
	   double dXp,
	   double dYp
	) const {
		// Specify the Schar Mountain (Test case 5 from Giraldo et al. 2008)
		double hsm = m_dhC * exp(-dXp/m_daC * dXp/m_daC) *
                     cos(M_PI * dXp / m_dlC) * cos(M_PI * dXp / m_dlC);
        //std::cout << hsm << "\n";

		return hsm;
	}

	///	<summary>
	///		Evaluate the Rayleigh friction strength at the given point.
	///	</summary>
	virtual double EvaluateRayleighStrength(
		double dZ,
		double dXp,
		double dYp
	) const {
		const double dRayleighStrength = 4.0e-2;//8.0e-3;
		const double dRayleighDepth = 10000.0;
		const double dRayleighWidth = 10000.0;

		double dNuDepth = 0.0;
		double dNuRight = 0.0;
		double dNuLeft  = 0.0;

		if (dZ > m_dGDim[5] - dRayleighDepth) {
			double dNormZ = (m_dGDim[5] - dZ) / dRayleighDepth;
			dNuDepth = 0.5 * dRayleighStrength * (1.0 + cos(M_PI * dNormZ));
		}
		if (dXp > m_dGDim[1] - dRayleighWidth) {
			double dNormX = (m_dGDim[1] - dXp) / dRayleighWidth;
			dNuRight = 0.5 * dRayleighStrength * (1.0 + cos(M_PI * dNormX));
		}
		if (dXp < m_dGDim[0] + dRayleighWidth) {
			double dNormX = (dXp - m_dGDim[0]) / dRayleighWidth;
			dNuLeft = 0.5 * dRayleighStrength * (1.0 + cos(M_PI * dNormX));
		}

		if ((dNuDepth >= dNuRight) && (dNuDepth >= dNuLeft)) {
			return dNuDepth;
		}
		if (dNuRight >= dNuLeft) {
			return dNuRight;
		}
		return dNuLeft;
	}

	///	<summary>
	///		Evaluate the reference state at the given point.
	///	</summary>
	virtual void EvaluateReferenceState(
		const PhysicalConstants & phys,
		double dZp,
		double dXp,
		double dYp,
		double * dState
	) const {
		const double dG = phys.GetG();
		const double dCv = phys.GetCv();
		const double dCp = phys.GetCp();
		const double dRd = phys.GetR();
		const double dP0 = phys.GetP0();

		// Base potential temperature field
		double dThetaBar = m_dTheta0 * exp(m_dNbar * m_dNbar / dG * dZp);

		// Set the uniform U, V, W field for all time
		dState[0] = m_dU0;
		dState[1] = 0.0;
		dState[3] = 0.0;

		// Zero gravity case
		if (dG == 0.0) {
			static double dT0 = 300.0;

			dState[2] = dT0 * pow(dP0, (dRd / dCp));
			dState[4] = dP0 / (dRd * dT0);

		// Stratification with gravity
		} else {
			// Set the initial density based on the Exner pressure
			double dExnerP = (dG * dG) / (dCp * m_dTheta0 * m_dNbar * m_dNbar);
			dExnerP *= (exp(-m_dNbar * m_dNbar / dG * dZp) - 1.0);
			dExnerP += 1.0;
			double dRho = dP0 / (dRd * dThetaBar) * pow(dExnerP,(dCv / dRd));
			dState[4] = dRho;

			// Set the initial potential temperature field
			//dState[2] = phys.PressureFromRhoTheta(dThetaBar * dRho);
			//dState[2] = (dThetaBar * dRho);
			dState[2] = dThetaBar;
		}
	}

	///	<summary>
	///		Evaluate the state vector at the given point.
	///	</summary>
	virtual void EvaluatePointwiseState(
		const PhysicalConstants & phys,
		const Time & time,
		double dZp,
		double dXp,
		double dYp,
		double * dState,
		double * dTracer
	) const {
		const double dG = phys.GetG();
		const double dCv = phys.GetCv();
		const double dCp = phys.GetCp();
		const double dRd = phys.GetR();
		const double dP0 = phys.GetP0();

		// Base potential temperature field
		double dThetaBar = m_dTheta0 * exp(m_dNbar * m_dNbar / dG * dZp);

		// Set the uniform U, V, W field for all time
		dState[0] = m_dU0;
		dState[1] = 0.0;
		dState[3] = 0.0;
		//dState[3] = sin(M_PI * dZp / m_dGDim[5]);

		// Set the initial density based on the Exner pressure
		double dExnerP = (dG * dG) / (dCp * m_dTheta0 * m_dNbar * m_dNbar);
		dExnerP *= (exp(-m_dNbar * m_dNbar / dG * dZp) - 1.0);
		dExnerP += 1.0;
		double dRho = dP0 / (dRd * dThetaBar) * pow(dExnerP,(dCv / dRd));
		dState[4] = dRho;

		// Set the initial theta field
		//dState[2] = phys.PressureFromRhoTheta(dThetaBar * dRho);
		//dState[2] = (dThetaBar * dRho);
		dState[2] = dThetaBar;
	}
};

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {

	// Initialize Tempest
	TempestInitialize(&argc, &argv);

try {
	// Uniform +X flow field.
	double dU0;

	// Brunt-Vaisala frequency
	double dNbar;

	// Reference pontential temperature
	double dTheta0;

	// Parameter factor for temperature disturbance
	double dThetaC;

	// Parameter reference height for temperature disturbance
	double dhC;

	// Parameter reference length a for temperature disturbance
	double daC;

	// Parameter reference length for mountain profile
	double dlC;

	// Parameter Archimede's Constant (essentially Pi but to some digits)
	double dpiC;

	// No Rayleigh friction
	bool fNoRayleighFriction;

	// Parse the command line
	BeginTempestCommandLine("ScharMountainCartesianTest");
		SetDefaultResolutionX(40);
		SetDefaultResolutionY(1);
		SetDefaultLevels(40);
		SetDefaultOutputDeltaT("300s");
		SetDefaultDeltaT("100000u");
		SetDefaultEndTime("3600s");
		SetDefaultHorizontalOrder(4);
		SetDefaultVerticalOrder(4);

		CommandLineDouble(dU0, "u0", 10.0);
		CommandLineDouble(dNbar, "Nbar", 0.01);
		CommandLineDouble(dTheta0, "Theta0", 280.0);
		CommandLineDouble(dThetaC, "ThetaC", 1.0);
		CommandLineDouble(dhC, "hC", 250.0);
		CommandLineDouble(daC, "aC", 5000.0);
		CommandLineDouble(dlC, "lC", 4000.0);
		CommandLineDouble(dlC, "piC", 3.14159265);
		CommandLineBool(fNoRayleighFriction, "norayleigh");

		ParseCommandLine(argc, argv);
	EndCommandLine(argv)

	// Create a new instance of the test
	ScharMountainCartesianTest * test =
		new ScharMountainCartesianTest(
			dU0,
			dNbar,
			dTheta0,
			dThetaC,
			dhC,
			daC,
			dlC,
			dpiC,
			fNoRayleighFriction);

	// Setup the Model
	AnnounceBanner("MODEL SETUP");

	Model model(EquationSet::PrimitiveNonhydrostaticEquations);

	TempestSetupCartesianModel(model, test->m_dGDim, 0.0, test->m_dhC);

	// Set the test case for the model
	AnnounceStartBlock("Initializing test case");
	model.SetTestCase(test);
	AnnounceEndBlock("Done");

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


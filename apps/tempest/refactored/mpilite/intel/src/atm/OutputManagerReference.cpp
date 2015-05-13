///////////////////////////////////////////////////////////////////////////////
///
///	\file    OutputManagerReference.cpp
///	\author  Paul Ullrich
///	\version May 7, 2013
///
///	<remarks>
///		Copyright 2000-2010 Paul Ullrich
///
///		This file is distributed as part of the Tempest source code package.
///		Permission is granted to use, copy, modify and distribute this
///		source code and its documentation under the terms of the GNU General
///		Public License.  This software is provided "as is" without express
///		or implied warranty.
///	</remarks

#include "OutputManagerReference.h"

#include "Model.h"
#include "Grid.h"
#include "ConsolidationStatus.h"

#include "TimeObj.h"
#include "Announce.h"

#include "mpi.h"

#include <iostream>
#include <cstdio>
#include <cmath>
#include <cfloat>
#include <sys/stat.h>

///////////////////////////////////////////////////////////////////////////////

OutputManagerReference::OutputManagerReference(
	Grid & grid,
	const Time & timeOutputFrequency,
	std::string strOutputDir,
	std::string strOutputPrefix,
	int nOutputsPerFile,
	int nXReference,
	int nYReference,
	bool fOutputAllVarsOnNodes,
	bool fRemoveReferenceProfile
) :
	OutputManager(
		grid,
		timeOutputFrequency,
		strOutputDir,
		strOutputPrefix,
		nOutputsPerFile),
	m_iGridStamp(-1),
	m_fFreshOutputFile(false),
	m_nXReference(nXReference),
	m_nYReference(nYReference),
	m_pActiveNcOutput(NULL),
	m_fOutputVorticity(false),
	m_fOutputDivergence(false),
	m_fOutputTemperature(false),
	m_fOutputAllVarsOnNodes(fOutputAllVarsOnNodes),
	m_fRemoveReferenceProfile(fRemoveReferenceProfile)
{

	// Get the reference box
	double dX0;
	double dX1;
	double dY0;
	double dY1;

	grid.GetReferenceGridBounds(dX0, dX1, dY0, dY1);

	// Initialize the coordinate arrays
	m_dXCoord.Initialize(m_nXReference);
	double dDeltaX = (dX1 - dX0) / static_cast<double>(m_nXReference);
	for (int i = 0; i < m_nXReference; i++) {
		m_dXCoord[i] =
			dDeltaX * (static_cast<double>(i) + 0.5) + dX0;
	}

	m_dYCoord.Initialize(m_nYReference);
	double dDeltaY = (dY1 - dY0) / static_cast<double>(m_nYReference);
	for (int j = 0; j < m_nYReference; j++) {
		m_dYCoord[j] =
			dDeltaY * (static_cast<double>(j) + 0.5) + dY0;
	}
}

///////////////////////////////////////////////////////////////////////////////

OutputManagerReference::~OutputManagerReference() {
	CloseFile();
}

///////////////////////////////////////////////////////////////////////////////

void OutputManagerReference::OutputVorticity(
	bool fOutputVorticity
) {
	m_fOutputVorticity = fOutputVorticity;

	if (!fOutputVorticity) {
		m_dataVorticity.Deinitialize();
	}
}

///////////////////////////////////////////////////////////////////////////////

void OutputManagerReference::OutputDivergence(
	bool fOutputDivergence
) {
	m_fOutputDivergence = fOutputDivergence;

	if (!fOutputDivergence) {
		m_dataDivergence.Deinitialize();
	}

}

///////////////////////////////////////////////////////////////////////////////

void OutputManagerReference::OutputTemperature(
	bool fOutputTemperature
) {
	m_fOutputTemperature = fOutputTemperature;

	if (!fOutputTemperature) {
		m_dataTemperature.Deinitialize();
	}
}

///////////////////////////////////////////////////////////////////////////////

bool OutputManagerReference::CalculatePatchCoordinates() {

	if (m_grid.GetGridStamp() == m_iGridStamp) {
		return false;
	}

	// Recalculate patch coordinates
	Announce("..Recalculating patch coordinates");

	// Construct array of reference coordinates
	DataVector<double> dXReference;
	dXReference.Initialize(m_nXReference * m_nYReference);

	DataVector<double> dYReference;
	dYReference.Initialize(m_nXReference * m_nYReference);

	int ix = 0;
	for (int j = 0; j < m_nYReference; j++) {
	for (int i = 0; i < m_nXReference; i++) {
		dXReference[ix] = m_dXCoord[i];
		dYReference[ix] = m_dYCoord[j];
		ix++;
	}
	}

	// Resize arrays
	m_dAlpha.Initialize(dXReference.GetRows());
	m_dBeta .Initialize(dXReference.GetRows());
	m_iPatch.Initialize(dXReference.GetRows());

	// Convert this reference point to a patch coordinate
	m_grid.ConvertReferenceToPatchCoord(
		dXReference,
		dYReference,
		m_dAlpha,
		m_dBeta,
		m_iPatch);

	// Allocate data arrays
	m_dataTopography.Initialize(
		1,
		1,
		m_nXReference * m_nYReference);

	m_dataStateNode.Initialize(
		m_grid.GetModel().GetEquationSet().GetComponents(),
		m_grid.GetRElements(),
		m_nXReference * m_nYReference);

	if (!m_fOutputAllVarsOnNodes) {
		m_dataStateREdge.Initialize(
			m_grid.GetModel().GetEquationSet().GetComponents(),
			m_grid.GetRElements() + 1,
			m_nXReference * m_nYReference);
	}

	if (m_grid.GetModel().GetEquationSet().GetTracers() != 0) {
		m_dataTracers.Initialize(
			m_grid.GetModel().GetEquationSet().GetTracers(),
			m_grid.GetRElements(),
			m_nXReference * m_nYReference);
	}

	if (m_fOutputVorticity) {
		m_dataVorticity.Initialize(
			1,
			m_grid.GetRElements(),
			m_nXReference * m_nYReference);
	}

	if (m_fOutputDivergence) {
		m_dataDivergence.Initialize(
			1,
			m_grid.GetRElements(),
			m_nXReference * m_nYReference);
	}

	if (m_fOutputTemperature) {
		m_dataTemperature.Initialize(
			1,
			m_grid.GetRElements(),
			m_nXReference * m_nYReference);
	}

	// Reduce/Interpolate topography array
	m_grid.ReduceInterpolate(
		m_dAlpha, m_dBeta, m_iPatch,
		DataType_Topography,
		DataLocation_None,
		m_fOutputAllVarsOnNodes,
		m_dataTopography,
		false);

	// Update grid stamp
	m_iGridStamp = m_grid.GetGridStamp();

	// Patch coordinates updated
	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool OutputManagerReference::OpenFile(
	const std::string & strFileName
) {
	// Determine processor rank; only proceed if root node
	int nRank;
	MPI_Comm_rank(MPI_COMM_WORLD, &nRank);

	// The active model
	const Model & model = m_grid.GetModel();

	// Open NetCDF file on root process
	if (nRank == 0) {

		// Check for existing NetCDF file
		if (m_pActiveNcOutput != NULL) {
			_EXCEPTIONT("NetCDF file already open");
		}

		// Append .nc extension to file
		std::string strNcFileName = strFileName + ".nc";

		// Open new NetCDF file
		m_pActiveNcOutput = new NcFile(strNcFileName.c_str(), NcFile::Replace);
		if (m_pActiveNcOutput == NULL) {
			_EXCEPTIONT("Error opening NetCDF file");
		}

		// Create nodal time dimension
		NcDim * dimTime =
			m_pActiveNcOutput->add_dim("time");

		m_varTime = m_pActiveNcOutput->add_var("time", ncDouble, dimTime);

		std::string strUnits =
			"days since " + model.GetStartTime().ToDateString();

		std::string strCalendarName = model.GetStartTime().GetCalendarName();

		m_varTime->add_att("long_name", "time");
		m_varTime->add_att("units", strUnits.c_str());
		m_varTime->add_att("calendar", strCalendarName.c_str());
		m_varTime->add_att("bounds", "time_bnds");

		// Create levels dimension
		NcDim * dimLev =
			m_pActiveNcOutput->add_dim("lev", m_grid.GetRElements());

		// Create interfaces dimension
		NcDim * dimILev =
			m_pActiveNcOutput->add_dim("ilev", m_grid.GetRElements()+1);

		// Create latitude dimension
		NcDim * dimLat =
			m_pActiveNcOutput->add_dim("lat", m_nYReference);

		// Create longitude dimension
		NcDim * dimLon =
			m_pActiveNcOutput->add_dim("lon", m_nXReference);

		// Output physical constants
		const PhysicalConstants & phys = model.GetPhysicalConstants();

		m_pActiveNcOutput->add_att("earth_radius", phys.GetEarthRadius());
		m_pActiveNcOutput->add_att("g", phys.GetG());
		m_pActiveNcOutput->add_att("omega", phys.GetOmega());
		m_pActiveNcOutput->add_att("alpha", phys.GetAlpha());
		m_pActiveNcOutput->add_att("Rd", phys.GetR());
		m_pActiveNcOutput->add_att("Cp", phys.GetCp());
		m_pActiveNcOutput->add_att("T0", phys.GetT0());
		m_pActiveNcOutput->add_att("P0", phys.GetP0());
		m_pActiveNcOutput->add_att("rho_water", phys.GetRhoWater());
		m_pActiveNcOutput->add_att("Rvap", phys.GetRvap());
		m_pActiveNcOutput->add_att("Mvap", phys.GetMvap());
		m_pActiveNcOutput->add_att("Lvap", phys.GetLvap());

		// Output grid parameters
		m_pActiveNcOutput->add_att("Ztop", m_grid.GetZtop());

		// Output equation set
		const EquationSet & eqn = model.GetEquationSet();

		m_pActiveNcOutput->add_att("equation_set", eqn.GetName().c_str());

		// Create variables
		for (int c = 0; c < eqn.GetComponents(); c++) {
			if ((m_fOutputAllVarsOnNodes) ||
				(m_grid.GetVarLocation(c) == DataLocation_Node)
			) {
				m_vecComponentVar.push_back(
					m_pActiveNcOutput->add_var(
						eqn.GetComponentShortName(c).c_str(),
						ncDouble, dimTime, dimLev, dimLat, dimLon));
			} else {
				m_vecComponentVar.push_back(
					m_pActiveNcOutput->add_var(
						eqn.GetComponentShortName(c).c_str(),
						ncDouble, dimTime, dimILev, dimLat, dimLon));
			}
		}

		for (int c = 0; c < eqn.GetTracers(); c++) {
			m_vecTracersVar.push_back(
				m_pActiveNcOutput->add_var(
					eqn.GetTracerShortName(c).c_str(),
					ncDouble, dimTime, dimLev, dimLat, dimLon));
		}

		// Vorticity variable
		if (m_fOutputVorticity) {
			m_varVorticity =
				m_pActiveNcOutput->add_var(
					"ZETA", ncDouble, dimTime, dimLev, dimLat, dimLon);
		}

		// Divergence variable
		if (m_fOutputDivergence) {
			m_varDivergence =
				m_pActiveNcOutput->add_var(
					"DELTA", ncDouble, dimTime, dimLev, dimLat, dimLon);
		}

		// Temperature variable
		if (m_fOutputTemperature) {
			m_varTemperature =
				m_pActiveNcOutput->add_var(
					"T", ncDouble, dimTime, dimLev, dimLat, dimLon);
		}

		// Output longitudes and latitudes
		NcVar * varLon = m_pActiveNcOutput->add_var("lon", ncDouble, dimLon);
		NcVar * varLat = m_pActiveNcOutput->add_var("lat", ncDouble, dimLat);

		varLon->put(m_dXCoord, m_dXCoord.GetRows());
		varLat->put(m_dYCoord, m_dYCoord.GetRows());

		varLon->add_att("long_name", "longitude");
		varLon->add_att("units", "degrees_east");

		varLat->add_att("long_name", "latitude");
		varLat->add_att("units", "degrees_north");

		// Output levels
		NcVar * varLev =
			m_pActiveNcOutput->add_var("lev", ncDouble, dimLev);

		varLev->put(
			m_grid.GetREtaStretchLevels(),
			m_grid.GetREtaStretchLevels().GetRows());

		varLev->add_att("long_name", "level");
		varLev->add_att("units", "level");

		// Output interface levels
		NcVar * varILev =
			m_pActiveNcOutput->add_var("ilev", ncDouble, dimILev);

		varILev->put(
			m_grid.GetREtaStretchInterfaces(),
			m_grid.GetREtaStretchInterfaces().GetRows());

		varILev->add_att("long_name", "interface level");
		varILev->add_att("units", "level");

		// Topography variable
		m_varTopography =
			m_pActiveNcOutput->add_var("Zs", ncDouble, dimLat, dimLon);

		// Fresh output file
		m_fFreshOutputFile = true;
	}

	// Wait for all processes to complete
	MPI_Barrier(MPI_COMM_WORLD);

	return true;
}

///////////////////////////////////////////////////////////////////////////////

void OutputManagerReference::CloseFile() {
	if (m_pActiveNcOutput != NULL) {
		delete(m_pActiveNcOutput);
		m_pActiveNcOutput = NULL;

		m_vecComponentVar.clear();
		m_vecTracersVar.clear();
	}
}

///////////////////////////////////////////////////////////////////////////////

void OutputManagerReference::Output(
	const Time & time
) {
	// Check for open file
	if (!IsFileOpen()) {
		_EXCEPTIONT("No file available for output");
	}

	// Get processor rank
	int nRank;
	MPI_Comm_rank(MPI_COMM_WORLD, &nRank);

	// Update reference grid
	CalculatePatchCoordinates();

	// Initial outputs to a new Output file
	if ((nRank == 0) && (m_fFreshOutputFile)) {

		// Output topography
		m_varTopography->put(
			&(m_dataTopography[0][0][0]),
			m_dYCoord.GetRows(),
			m_dXCoord.GetRows());
	}

	// Equation set
	const EquationSet & eqn = m_grid.GetModel().GetEquationSet();

	// Add new time
	if (nRank == 0) {
#pragma message "FIX: Doesn't give correct count of days"
		double dTimeDays = (time - m_grid.GetModel().GetStartTime()) / 86400.0;
		m_varTime->set_cur(m_ixOutputTime);
		m_varTime->put(&dTimeDays, 1);
	}

	// Vertically interpolate data to model levels
	if (m_fOutputAllVarsOnNodes) {
		for (int c = 0; c < eqn.GetComponents(); c++) {
			if (m_grid.GetVarLocation(c) == DataLocation_REdge) {
				m_grid.InterpolateREdgeToNode(c, 0);
			}
		}
	}

	// Perform Interpolate / Reduction on state data
	m_dataStateNode.Zero();

	m_grid.ReduceInterpolate(
		m_dAlpha, m_dBeta, m_iPatch,
		DataType_State, DataLocation_Node, m_fOutputAllVarsOnNodes,
		m_dataStateNode,
		!m_fRemoveReferenceProfile);

	if (!m_fOutputAllVarsOnNodes) {
		m_dataStateREdge.Zero();

#pragma message "Get rid of these calls when switching to w"
		m_grid.InterpolateNodeToREdge(0, 0);
		m_grid.InterpolateNodeToREdge(1, 0);

		m_grid.ReduceInterpolate(
			m_dAlpha, m_dBeta, m_iPatch,
			DataType_State, DataLocation_REdge, false,
			m_dataStateREdge,
			!m_fRemoveReferenceProfile);
	}

	// Perform Interpolate / Reduction on tracers data
	if (m_grid.GetModel().GetEquationSet().GetTracers() != 0) {
		m_dataTracers.Zero();

		m_grid.ReduceInterpolate(
			m_dAlpha, m_dBeta, m_iPatch,
			DataType_Tracers, DataLocation_Node, false,
			m_dataTracers, true);
	}

	// Perform Interpolate / Reduction on computed vorticity
	if (m_fOutputVorticity || m_fOutputDivergence) {
		m_grid.ComputeVorticityDivergence(0);

		if (m_fOutputVorticity) {
			m_grid.ReduceInterpolate(
				m_dAlpha, m_dBeta, m_iPatch,
				DataType_Vorticity, DataLocation_Node, false,
				m_dataVorticity);
		}
		if (m_fOutputDivergence) {
			m_grid.ReduceInterpolate(
				m_dAlpha, m_dBeta, m_iPatch,
				DataType_Divergence, DataLocation_Node, false,
				m_dataDivergence);
		}
	}

	// Perform Interpolate / Reduction on temperature
	if (m_fOutputTemperature) {
		m_grid.ComputeTemperature(0);

		m_grid.ReduceInterpolate(
			m_dAlpha, m_dBeta, m_iPatch,
			DataType_Temperature, DataLocation_Node, false,
			m_dataTemperature);
	}

	// Store state variable data
	if (nRank == 0) {
		for (int c = 0; c < eqn.GetComponents(); c++) {
			if ((m_fOutputAllVarsOnNodes) ||
				(m_grid.GetVarLocation(c) == DataLocation_Node)
			) {
				m_vecComponentVar[c]->set_cur(m_ixOutputTime, 0, 0, 0);
				m_vecComponentVar[c]->put(
					&(m_dataStateNode[c][0][0]),
					1,
					m_dataStateNode.GetColumns(),
					m_dYCoord.GetRows(),
					m_dXCoord.GetRows());

			} else {
				m_vecComponentVar[c]->set_cur(m_ixOutputTime, 0, 0, 0);
				m_vecComponentVar[c]->put(
					&(m_dataStateREdge[c][0][0]),
					1,
					m_dataStateREdge.GetColumns(),
					m_dYCoord.GetRows(),
					m_dXCoord.GetRows());
			}
		}

		// Store tracer variable data
		if (m_grid.GetModel().GetEquationSet().GetTracers() != 0) {
			for (int c = 0; c < eqn.GetTracers(); c++) {
				m_vecTracersVar[c]->set_cur(m_ixOutputTime, 0, 0, 0);
				m_vecTracersVar[c]->put(
					&(m_dataTracers[c][0][0]),
					1,
					m_dataTracers.GetColumns(),
					m_dYCoord.GetRows(),
					m_dXCoord.GetRows());
			}
		}

		// Store vorticity data
		if (m_fOutputVorticity) {
			m_varVorticity->set_cur(m_ixOutputTime, 0, 0, 0);
			m_varVorticity->put(
				&(m_dataVorticity[0][0][0]),
				1,
				m_dataVorticity.GetColumns(),
				m_dYCoord.GetRows(),
				m_dXCoord.GetRows());
		}

		// Store divergence data
		if (m_fOutputDivergence) {
			m_varDivergence->set_cur(m_ixOutputTime, 0, 0, 0);
			m_varDivergence->put(
				&(m_dataDivergence[0][0][0]),
				1,
				m_dataDivergence.GetColumns(),
				m_dYCoord.GetRows(),
				m_dXCoord.GetRows());
		}

		// Store temperature data
		if (m_fOutputTemperature) {
			m_varTemperature->set_cur(m_ixOutputTime, 0, 0, 0);
			m_varTemperature->put(
				&(m_dataTemperature[0][0][0]),
				1,
				m_dataTemperature.GetColumns(),
				m_dYCoord.GetRows(),
				m_dXCoord.GetRows());
		}

	}

	// No longer fresh file
	m_fFreshOutputFile = false;

	// Barrier
	MPI_Barrier(MPI_COMM_WORLD);
}

///////////////////////////////////////////////////////////////////////////////


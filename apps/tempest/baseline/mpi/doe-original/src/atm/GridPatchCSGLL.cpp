///////////////////////////////////////////////////////////////////////////////
///
///	\file    GridPatchCSGLL.cpp
///	\author  Paul Ullrich
///	\version February 25, 2013
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

#include "GridPatchCSGLL.h"
#include "GridCSGLL.h"
#include "Model.h"
#include "TestCase.h"
#include "GridSpacing.h"
#include "HorizontalDynamicsDG.h"
#include "VerticalStretch.h"

#include "Direction.h"
#include "CubedSphereTrans.h"
#include "PolynomialInterp.h"
#include "GaussQuadrature.h"
#include "GaussLobattoQuadrature.h"

#include "Announce.h"
#include "MathHelper.h"

#include <cmath>

///////////////////////////////////////////////////////////////////////////////

GridPatchCSGLL::GridPatchCSGLL(
	GridCSGLL & grid,
	int ixPatch,
	const PatchBox & box,
	int nHorizontalOrder,
	int nVerticalOrder
) :
	GridPatchGLL(
		grid,
		ixPatch,
		box,
		nHorizontalOrder,
		nVerticalOrder)
{

	// Get panels in each coordinate direction
	if (grid.GetABaseResolution() != grid.GetBBaseResolution()) {
		_EXCEPTIONT("Invalid grid; CubedSphere grids must be square");
	}

	int ixDest;
	int jxDest;
	bool fSwitchAB;
	bool fSwitchPar;
	bool fSwitchPerp;

	m_ixNeighborPanel.resize(8);

	// Towards the right
	CubedSphereTrans::RelativeCoord(
		m_nHorizontalOrder * grid.GetABaseResolution(),
		box.GetPanel(),
		box.GetAGlobalInteriorEnd(),
		box.GetBGlobalInteriorBegin(),
		m_ixNeighborPanel[(int)(Direction_Right)],
		ixDest, jxDest,
		fSwitchAB, fSwitchPar, fSwitchPerp);

	// Towards the top
	CubedSphereTrans::RelativeCoord(
		m_nHorizontalOrder * grid.GetABaseResolution(),
		box.GetPanel(),
		box.GetAGlobalInteriorBegin(),
		box.GetBGlobalInteriorEnd(),
		m_ixNeighborPanel[(int)(Direction_Top)],
		ixDest, jxDest,
		fSwitchAB, fSwitchPar, fSwitchPerp);

	// Towards the left
	CubedSphereTrans::RelativeCoord(
		m_nHorizontalOrder * grid.GetABaseResolution(),
		box.GetPanel(),
		box.GetAGlobalInteriorBegin()-1,
		box.GetBGlobalInteriorBegin(),
		m_ixNeighborPanel[(int)(Direction_Left)],
		ixDest, jxDest,
		fSwitchAB, fSwitchPar, fSwitchPerp);

	// Towards the bottom
	CubedSphereTrans::RelativeCoord(
		m_nHorizontalOrder * grid.GetABaseResolution(),
		box.GetPanel(),
		box.GetAGlobalInteriorBegin(),
		box.GetBGlobalInteriorBegin()-1,
		m_ixNeighborPanel[(int)(Direction_Bottom)],
		ixDest, jxDest,
		fSwitchAB, fSwitchPar, fSwitchPerp);

	// Towards the top-right
	CubedSphereTrans::RelativeCoord(
		m_nHorizontalOrder * grid.GetABaseResolution(),
		box.GetPanel(),
		box.GetAGlobalInteriorEnd(),
		box.GetBGlobalInteriorEnd(),
		m_ixNeighborPanel[(int)(Direction_TopRight)],
		ixDest, jxDest,
		fSwitchAB, fSwitchPar, fSwitchPerp);

	// Towards the top-left
	CubedSphereTrans::RelativeCoord(
		m_nHorizontalOrder * grid.GetABaseResolution(),
		box.GetPanel(),
		box.GetAGlobalInteriorBegin()-1,
		box.GetBGlobalInteriorEnd(),
		m_ixNeighborPanel[(int)(Direction_TopLeft)],
		ixDest, jxDest,
		fSwitchAB, fSwitchPar, fSwitchPerp);

	// Towards the bottom-left
	CubedSphereTrans::RelativeCoord(
		m_nHorizontalOrder * grid.GetABaseResolution(),
		box.GetPanel(),
		box.GetAGlobalInteriorBegin()-1,
		box.GetBGlobalInteriorBegin()-1,
		m_ixNeighborPanel[(int)(Direction_BottomLeft)],
		ixDest, jxDest,
		fSwitchAB, fSwitchPar, fSwitchPerp);

	// Towards the bottom-right
	CubedSphereTrans::RelativeCoord(
		m_nHorizontalOrder * grid.GetABaseResolution(),
		box.GetPanel(),
		box.GetAGlobalInteriorEnd(),
		box.GetBGlobalInteriorBegin()-1,
		m_ixNeighborPanel[(int)(Direction_BottomRight)],
		ixDest, jxDest,
		fSwitchAB, fSwitchPar, fSwitchPerp);
}

///////////////////////////////////////////////////////////////////////////////

void GridPatchCSGLL::InitializeDataLocal() {

	// Allocate data
	GridPatch::InitializeDataLocal();

	// Initialize the longitude and latitude at each node
	for (int i = 0; i < m_box.GetATotalWidth(); i++) {
	for (int j = 0; j < m_box.GetBTotalWidth(); j++) {
		CubedSphereTrans::RLLFromABP(
			m_box.GetANode(i),
			m_box.GetBNode(j),
			m_box.GetPanel(),
			m_dataLon[i][j],
			m_dataLat[i][j]);
	}
	}
}

///////////////////////////////////////////////////////////////////////////////

void GridPatchCSGLL::EvaluateTopography(
	const TestCase & test
) {
	const PhysicalConstants & phys = m_grid.GetModel().GetPhysicalConstants();

	for (int i = 0; i < m_box.GetATotalWidth(); i++) {
	for (int j = 0; j < m_box.GetBTotalWidth(); j++) {
		double dLon;
		double dLat;

		CubedSphereTrans::RLLFromABP(
			m_box.GetANode(i),
			m_box.GetBNode(j),
			m_box.GetPanel(),
			dLon, dLat);

		m_dataTopography[i][j] = test.EvaluateTopography(phys, dLon, dLat);
	}
	}
}

///////////////////////////////////////////////////////////////////////////////

void GridPatchCSGLL::EvaluateGeometricTerms() {

	// Physical constants
	const PhysicalConstants & phys = m_grid.GetModel().GetPhysicalConstants();

	// 2D equation set
	bool fIs2DEquationSet = false;
	if (m_grid.GetModel().GetEquationSet().GetDimensionality() == 2) {
		fIs2DEquationSet = true;
	}

	if ((fIs2DEquationSet) && (m_grid.GetZtop() != 1.0)) {
		_EXCEPTIONT("Ztop must be 1.0 for 2D equation sets");
	}

	// Obtain Gauss Lobatto quadrature nodes and weights
	DataVector<double> dGL;
	DataVector<double> dWL;

	GaussLobattoQuadrature::GetPoints(m_nHorizontalOrder, 0.0, 1.0, dGL, dWL);

	// Obtain Gaussian quadrature nodes and weights in the vertical
	DataVector<double> dGNode;
	DataVector<double> dWNode;

	GaussQuadrature::GetPoints(
		m_nVerticalOrder, 0.0, 1.0, dGNode, dWNode);

	// Obtain Gauss Lobatto quadrature nodes and weights in the vertical
	DataVector<double> dGREdge;
	DataVector<double> dWREdge;

	GaussLobattoQuadrature::GetPoints(
		m_nVerticalOrder+1, 0.0, 1.0, dGREdge, dWREdge);

	// Vertical elemental grid spacing
	double dElementDeltaXi =
		static_cast<double>(m_nVerticalOrder)
		/ static_cast<double>(m_grid.GetRElements());

	// Derivatives of basis functions
	GridCSGLL & gridCSGLL = dynamic_cast<GridCSGLL &>(m_grid);

	const DataMatrix<double> & dDxBasis1D = gridCSGLL.GetDxBasis1D();

	// Initialize the Coriolis force at each node
	for (int i = 0; i < m_box.GetATotalWidth(); i++) {
	for (int j = 0; j < m_box.GetBTotalWidth(); j++) {
		m_dataCoriolisF[i][j] = 2.0 * phys.GetOmega() * sin(m_dataLat[i][j]);
	}
	}

	// Initialize metric and Christoffel symbols in terrain-following coords
	for (int a = 0; a < GetElementCountA(); a++) {
	for (int b = 0; b < GetElementCountB(); b++) {

	for (int i = 0; i < m_nHorizontalOrder; i++) {
	for (int j = 0; j < m_nHorizontalOrder; j++) {

		// Nodal points
		int iElementA = m_box.GetAInteriorBegin() + a * m_nHorizontalOrder;
		int iElementB = m_box.GetBInteriorBegin() + b * m_nHorizontalOrder;

		int iA = iElementA + i;
		int iB = iElementB + j;

		// Gnomonic coordinates
		double dX = tan(m_box.GetANode(iA));
		double dY = tan(m_box.GetBNode(iB));
		double dDelta2 = (1.0 + dX * dX + dY * dY);
		double dDelta = sqrt(dDelta2);

		// Topography height and its derivatives
		double dZs = m_dataTopography[iA][iB];

		double dDaZs = 0.0;
		double dDbZs = 0.0;
		for (int s = 0; s < m_nHorizontalOrder; s++) {
			dDaZs += dDxBasis1D[s][i] * m_dataTopography[iElementA+s][iB];
			dDbZs += dDxBasis1D[s][j] * m_dataTopography[iA][iElementB+s];
		}
		dDaZs /= GetElementDeltaA();
		dDbZs /= GetElementDeltaB();

		// 2D equations
		if (fIs2DEquationSet) {
			dZs = 0.0;
			dDaZs = 0.0;
			dDbZs = 0.0;
		}

		// Initialize 2D Jacobian
		m_dataJacobian2D[iA][iB] =
			(1.0 + dX * dX) * (1.0 + dY * dY) / (dDelta * dDelta * dDelta);

		m_dataJacobian2D[iA][iB] *=
			  phys.GetEarthRadius()
			* phys.GetEarthRadius();

		// Initialize 2D contravariant metric
		double dContraMetricScale =
			dDelta2 / (1.0 + dX * dX) / (1.0 + dY * dY)
			/ (phys.GetEarthRadius() * phys.GetEarthRadius());

		m_dataContraMetric2DA[iA][iB][0] =
			dContraMetricScale * (1.0 + dY * dY);
		m_dataContraMetric2DA[iA][iB][1] =
			dContraMetricScale * dX * dY;

		m_dataContraMetric2DB[iA][iB][0] =
			dContraMetricScale * dX * dY;
		m_dataContraMetric2DB[iA][iB][1] =
			dContraMetricScale * (1.0 + dX * dX);

		// Initialize 2D covariant metric
		double dCovMetricScale =
			phys.GetEarthRadius() * phys.GetEarthRadius()
			* (1.0 + dX * dX) * (1.0 + dY * dY)
			/ (dDelta2 * dDelta2);

		m_dataCovMetric2DA[iA][iB][0] =
			dCovMetricScale * (1.0 + dX * dX);
		m_dataCovMetric2DA[iA][iB][1] =
			dCovMetricScale * (- dX * dY);

		m_dataCovMetric2DB[iA][iB][0] =
			dCovMetricScale * (- dX * dY);
		m_dataCovMetric2DB[iA][iB][1] =
			dCovMetricScale * (1.0 + dY * dY);

		// Christoffel symbol components at each node
		// (off-diagonal elements are doubled due to symmetry)
		m_dataChristoffelA[iA][iB][0] =
			2.0 * dX * dY * dY / dDelta2;
		m_dataChristoffelA[iA][iB][1] =
			- 2.0 * dY * (1.0 + dY * dY) / dDelta2;
		m_dataChristoffelA[iA][iB][2] = 0.0;
		m_dataChristoffelB[iA][iB][0] = 0.0;
		m_dataChristoffelB[iA][iB][1] =
			- 2.0 * dX * (1.0 + dX * dX) / dDelta2;
		m_dataChristoffelB[iA][iB][2] =
			2.0 * dX * dX * dY / dDelta2;

		// Vertical coordinate transform and its derivatives
		for (int k = 0; k < m_grid.GetRElements(); k++) {

			// Sub-element index
			int kx = k % m_nVerticalOrder;

			// Gal-Chen and Somerville (1975) linear terrain-following coord
			double dREta = m_grid.GetREtaLevel(k);

			double dREtaStretch;
			double dDxREtaStretch;
			m_grid.EvaluateVerticalStretchF(
				dREta, dREtaStretch, dDxREtaStretch);

			double dZ = dZs + (m_grid.GetZtop() - dZs) * dREtaStretch;

			double dDaR = (1.0 - dREtaStretch) * dDaZs;
			double dDbR = (1.0 - dREtaStretch) * dDbZs;
			double dDxR = (m_grid.GetZtop() - dZs) * dDxREtaStretch;

/*
			double dDaxR = - dDaZs / (m_grid.GetZtop() - dZs) * dDxR;
			double dDbxR = - dDbZs / (m_grid.GetZtop() - dZs) * dDxR;
			double dDxxR = 0.0;
*/
/*
			double dDaR = (1.0 - dREta) * dDaZs;
			double dDbR = (1.0 - dREta) * dDbZs;

			double dDxR = m_grid.GetZtop() - dZs;
			double dDaxR = - dDaZs;
			double dDbxR = - dDbZs;
			double dDxxR = 0.0;
*/
			// Calculate pointwise Jacobian
			m_dataJacobian[k][iA][iB] = dDxR * m_dataJacobian2D[iA][iB];

			// Element area associated with each model level GLL node
			m_dataElementArea[k][iA][iB] =
				m_dataJacobian[k][iA][iB]
				* dWL[i] * GetElementDeltaA()
				* dWL[j] * GetElementDeltaB()
				* dWNode[kx] * dElementDeltaXi;

			// Contravariant metric components
			m_dataContraMetricA[k][iA][iB][0] =
				m_dataContraMetric2DA[iA][iB][0];
			m_dataContraMetricA[k][iA][iB][1] =
				m_dataContraMetric2DA[iA][iB][1];
			m_dataContraMetricA[k][iA][iB][2] =
				- dContraMetricScale / dDxR * (
					(1.0 + dY * dY) * dDaR + dX * dY * dDbR);

			m_dataContraMetricB[k][iA][iB][0] =
				m_dataContraMetric2DB[iA][iB][0];
			m_dataContraMetricB[k][iA][iB][1] =
				m_dataContraMetric2DB[iA][iB][1];
			m_dataContraMetricB[k][iA][iB][2] =
				- dContraMetricScale / dDxR * (
					dX * dY * dDaR + (1.0 + dX * dX) * dDbR);

			// Covariant metric components
			m_dataCovMetricA[k][iA][iB][0] =
				m_dataCovMetric2DA[iA][iB][0] + dDaR * dDaR;
			m_dataCovMetricA[k][iA][iB][1] =
				m_dataCovMetric2DA[iA][iB][1] + dDaR * dDbR;
			m_dataCovMetricA[k][iA][iB][2] =
				dDaR * dDxR;

			m_dataCovMetricB[k][iA][iB][0] =
				m_dataCovMetric2DB[iA][iB][0] + dDbR * dDaR;
			m_dataCovMetricB[k][iA][iB][1] =
				m_dataCovMetric2DB[iA][iB][1] + dDbR * dDbR;
			m_dataCovMetricB[k][iA][iB][2] =
				dDbR * dDxR;
/*
			m_dataCovMetricXi[k][iA][iB][0] =
				dDxR * dDaR;
			m_dataCovMetricXi[k][iA][iB][1] =
				dDxR * dDbR;
			m_dataCovMetricXi[k][iA][iB][2] =
				dDxR * dDxR;
*/

/*
			m_dataContraMetricXi[k][iA][iB][0] =
				- dContraMetricScale / dDxR
					* ((1.0 + dY * dY) * dDaR + dX * dY * dDbR);
			m_dataContraMetricXi[k][iA][iB][1] =
				- dContraMetricScale / dDxR
					* (dX * dY * dDaR + (1.0 + dX * dX) * dDbR);
			m_dataContraMetricXi[k][iA][iB][2] =
				1.0 / (dDxR * dDxR)
					* (1.0 + dContraMetricScale
						* (  (1.0 + dY * dY) * dDaR * dDaR
						   + 2.0 * dX * dY * dDaR * dDbR
						   + (1.0 + dX * dX) * dDbR * dDbR));

			// Vertical Christoffel symbol components
			m_dataChristoffelXi[k][iA][iB][0] =
				- 2.0 * dX * dY * dY / dDelta2 * dDaR + dDaaR;

			m_dataChristoffelXi[k][iA][iB][1] =
				+ 2.0 * dY * (1.0 + dY * dY) / dDelta2 * dDaR
				+ 2.0 * dX * (1.0 + dX * dX) / dDelta2 * dDbR
				+ 2.0 * dDabR;

			m_dataChristoffelXi[k][iA][iB][2] = 2.0 * dDaxR;

			m_dataChristoffelXi[k][iA][iB][3] =
				- 2.0 * dX * dX * dY / dDelta2 * dDbR + dDbbR;

			m_dataChristoffelXi[k][iA][iB][4] = 2.0 * dDbxR;

			m_dataChristoffelXi[k][iA][iB][5] = dDxxR;

			m_dataChristoffelXi[k][iA][iB][0] /= dDxR;
			m_dataChristoffelXi[k][iA][iB][1] /= dDxR;
			m_dataChristoffelXi[k][iA][iB][2] /= dDxR;
			m_dataChristoffelXi[k][iA][iB][3] /= dDxR;
			m_dataChristoffelXi[k][iA][iB][4] /= dDxR;
			m_dataChristoffelXi[k][iA][iB][5] /= dDxR;
*/
			// Orthonormalization coefficients
			m_dataOrthonormNode[k][iA][iB][0] = - dDaR / dDxR;
			m_dataOrthonormNode[k][iA][iB][1] = - dDbR / dDxR;
			m_dataOrthonormNode[k][iA][iB][2] = 1.0 / dDxR;
		}

		// Metric terms at vertical interfaces
		for (int k = 0; k <= m_grid.GetRElements(); k++) {
			int kx = k % m_nVerticalOrder;

			// Gal-Chen and Somerville (1975) linear terrain-following coord
			double dREta = m_grid.GetREtaInterface(k);

			double dREtaStretch;
			double dDxREtaStretch;
			m_grid.EvaluateVerticalStretchF(
				dREta, dREtaStretch, dDxREtaStretch);

			double dZ = dZs + (m_grid.GetZtop() - dZs) * dREtaStretch;

			double dDaR = (1.0 - dREtaStretch) * dDaZs;
			double dDbR = (1.0 - dREtaStretch) * dDbZs;
			double dDxR = (m_grid.GetZtop() - dZs) * dDxREtaStretch;

			// Calculate pointwise Jacobian
			m_dataJacobianREdge[k][iA][iB] =
				(1.0 + dX * dX) * (1.0 + dY * dY) / (dDelta * dDelta * dDelta);

			m_dataJacobianREdge[k][iA][iB] *=
				dDxR
				* phys.GetEarthRadius()
				* phys.GetEarthRadius();

			// Element area associated with each model interface GLL node
			m_dataElementAreaREdge[k][iA][iB] =
				m_dataJacobianREdge[k][iA][iB]
				* dWL[i] * GetElementDeltaA()
				* dWL[j] * GetElementDeltaB()
				* dWREdge[kx] * dElementDeltaXi;

			if ((k != 0) && (k != m_grid.GetRElements()) && (kx == 0)) {
				m_dataElementAreaREdge[k][iA][iB] *= 2.0;
			}

			// Orthonormalization coefficients
			m_dataOrthonormREdge[k][iA][iB][0] = - dDaR / dDxR;
			m_dataOrthonormREdge[k][iA][iB][1] = - dDbR / dDxR;
			m_dataOrthonormREdge[k][iA][iB][2] = 1.0 / dDxR;
		}
	}
	}

	}
	}
}

///////////////////////////////////////////////////////////////////////////////

void GridPatchCSGLL::EvaluateTestCase(
	const TestCase & test,
	const Time & time,
	int iDataIndex
) {
	// Initialize the data at each node
	if (m_datavecStateNode.size() == 0) {
		_EXCEPTIONT("InitializeData must be called before InitialConditions");
	}
	if (iDataIndex >= m_datavecStateNode.size()) {
		_EXCEPTIONT("Invalid iDataIndex (out of range)");
	}

	// 2D equation set
	bool fIs2DEquationSet = false;
	if (m_grid.GetModel().GetEquationSet().GetDimensionality() == 2) {
		fIs2DEquationSet = true;
	}

	// Check dimensionality
	if (fIs2DEquationSet && (m_nVerticalOrder != 1)) {
		_EXCEPTIONT("VerticalOrder / Dimensionality mismatch:\n"
			"For 2D problems vertical order must be 1.");
	}

	// Evaluate topography
	EvaluateTopography(test);

	// Physical constants
	const PhysicalConstants & phys = m_grid.GetModel().GetPhysicalConstants();

	// Initialize the vertical height in each node
	if (fIs2DEquationSet) {
		for (int i = 0; i < m_box.GetATotalWidth(); i++) {
		for (int j = 0; j < m_box.GetBTotalWidth(); j++) {
			m_dataZLevels[0][i][j] = 0.0;
			m_dataZInterfaces[0][i][j] = 0.0;
			m_dataZInterfaces[1][i][j] = 1.0;
		}
		}

	} else {
		for (int i = 0; i < m_box.GetATotalWidth(); i++) {
		for (int j = 0; j < m_box.GetBTotalWidth(); j++) {

			// Gal-Chen and Sommerville (1975) vertical coordinate
			for (int k = 0; k < m_grid.GetRElements(); k++) {
				double dREta = m_grid.GetREtaLevel(k);

				double dREtaStretch;
				double dDxREtaStretch;
				m_grid.EvaluateVerticalStretchF(
					dREta, dREtaStretch, dDxREtaStretch);

				m_dataZLevels[k][i][j] =
					m_dataTopography[i][j]
						+ dREtaStretch
							* (m_grid.GetZtop() - m_dataTopography[i][j]);
			}
			for (int k = 0; k <= m_grid.GetRElements(); k++) {
				double dREta = m_grid.GetREtaInterface(k);

				double dREtaStretch;
				double dDxREtaStretch;
				m_grid.EvaluateVerticalStretchF(
					dREta, dREtaStretch, dDxREtaStretch);

				m_dataZInterfaces[k][i][j] =
					m_dataTopography[i][j]
						+ dREtaStretch
							* (m_grid.GetZtop() - m_dataTopography[i][j]);
			}
		}
		}
	}

	// Initialize the Rayleigh friction strength at each node
	if (test.HasRayleighFriction()) {
		for (int i = 0; i < m_box.GetATotalWidth(); i++) {
		for (int j = 0; j < m_box.GetBTotalWidth(); j++) {
			for (int k = 0; k < m_grid.GetRElements(); k++) {
				m_dataRayleighStrengthNode[k][i][j] =
					test.EvaluateRayleighStrength(
						m_dataZLevels[k][i][j],
						m_dataLon[i][j],
						m_dataLat[i][j]);
			}
			for (int k = 0; k < m_grid.GetRElements(); k++) {
				m_dataRayleighStrengthREdge[k][i][j] =
					test.EvaluateRayleighStrength(
						m_dataZInterfaces[k][i][j],
						m_dataLon[i][j],
						m_dataLat[i][j]);
			}
		}
		}
	}

	// Buffer vector for storing pointwise states
	int nComponents = m_grid.GetModel().GetEquationSet().GetComponents();
	int nTracers = m_grid.GetModel().GetEquationSet().GetTracers();

	DataVector<double> dPointwiseState;
	dPointwiseState.Initialize(nComponents);

	DataVector<double> dPointwiseRefState;
	dPointwiseRefState.Initialize(nComponents);

	DataVector<double> dPointwiseTracers;
	if (m_datavecTracers.size() > 0) {
		dPointwiseTracers.Initialize(nTracers);
	}

	// Evaluate the state on model levels
	//double dMaxWaveSpeed = 0.0;

	for (int k = 0; k < m_grid.GetRElements(); k++) {
	for (int i = 0; i < m_box.GetATotalWidth(); i++) {
	for (int j = 0; j < m_box.GetBTotalWidth(); j++) {

		// Evaluate pointwise state
		test.EvaluatePointwiseState(
			phys,
			time,
			m_dataZLevels[k][i][j],
			m_dataLon[i][j],
			m_dataLat[i][j],
			dPointwiseState,
			dPointwiseTracers);

		for (int c = 0; c < dPointwiseState.GetRows(); c++) {
			m_datavecStateNode[iDataIndex][c][k][i][j] = dPointwiseState[c];
		}

		// Transform state velocities
		double dUlon;
		double dUlat;

		dUlon = m_datavecStateNode[iDataIndex][0][k][i][j];
		dUlat = m_datavecStateNode[iDataIndex][1][k][i][j];

		dUlon /= phys.GetEarthRadius();
		dUlat /= phys.GetEarthRadius();

		CubedSphereTrans::VecTransABPFromRLL(
			tan(m_box.GetANode(i)),
			tan(m_box.GetBNode(j)),
			m_box.GetPanel(),
			dUlon, dUlat,
			m_datavecStateNode[iDataIndex][0][k][i][j],
			m_datavecStateNode[iDataIndex][1][k][i][j]);

		// Evaluate reference state
		if (m_grid.HasReferenceState()) {
			test.EvaluateReferenceState(
				m_grid.GetModel().GetPhysicalConstants(),
				m_dataZLevels[k][i][j],
				m_dataLon[i][j],
				m_dataLat[i][j],
				dPointwiseRefState);

			for (int c = 0; c < dPointwiseState.GetRows(); c++) {
				m_dataRefStateNode[c][k][i][j] = dPointwiseRefState[c];
			}

			// Transform reference velocities
			dUlon = m_dataRefStateNode[0][k][i][j];
			dUlat = m_dataRefStateNode[1][k][i][j];

			dUlon /= phys.GetEarthRadius();
			dUlat /= phys.GetEarthRadius();

			CubedSphereTrans::VecTransABPFromRLL(
				tan(m_box.GetANode(i)),
				tan(m_box.GetBNode(j)),
				m_box.GetPanel(),
				dUlon, dUlat,
				m_dataRefStateNode[0][k][i][j],
				m_dataRefStateNode[1][k][i][j]);
		}

		// Evaluate tracers
		for (int c = 0; c < dPointwiseTracers.GetRows(); c++) {
			m_datavecTracers[iDataIndex][c][k][i][j] = dPointwiseTracers[c];
		}
	}
	}
	}

	// Evaluate the state on model interfaces
	for (int k = 0; k <= m_grid.GetRElements(); k++) {
	for (int i = 0; i < m_box.GetATotalWidth(); i++) {
	for (int j = 0; j < m_box.GetBTotalWidth(); j++) {

		// Evaluate pointwise state
		test.EvaluatePointwiseState(
			m_grid.GetModel().GetPhysicalConstants(),
			time,
			m_dataZInterfaces[k][i][j],
			m_dataLon[i][j],
			m_dataLat[i][j],
			dPointwiseState,
			dPointwiseTracers);

		for (int c = 0; c < dPointwiseState.GetRows(); c++) {
			m_datavecStateREdge[iDataIndex][c][k][i][j] = dPointwiseState[c];
		}

		// Transform state velocities
		double dUlon;
		double dUlat;

		dUlon = m_datavecStateREdge[iDataIndex][0][k][i][j];
		dUlat = m_datavecStateREdge[iDataIndex][1][k][i][j];

		dUlon /= phys.GetEarthRadius();
		dUlat /= phys.GetEarthRadius();

		CubedSphereTrans::VecTransABPFromRLL(
			tan(m_box.GetANode(i)),
			tan(m_box.GetBNode(j)),
			m_box.GetPanel(),
			dUlon, dUlat,
			m_datavecStateREdge[iDataIndex][0][k][i][j],
			m_datavecStateREdge[iDataIndex][1][k][i][j]);

		if (m_grid.HasReferenceState()) {
			test.EvaluateReferenceState(
				m_grid.GetModel().GetPhysicalConstants(),
				m_dataZInterfaces[k][i][j],
				m_dataLon[i][j],
				m_dataLat[i][j],
				dPointwiseRefState);

			for (int c = 0; c < dPointwiseState.GetRows(); c++) {
				m_dataRefStateREdge[c][k][i][j] = dPointwiseRefState[c];
			}

			// Transform reference velocities
			dUlon = m_dataRefStateREdge[0][k][i][j];
			dUlat = m_dataRefStateREdge[1][k][i][j];

			dUlon /= phys.GetEarthRadius();
			dUlat /= phys.GetEarthRadius();

			CubedSphereTrans::VecTransABPFromRLL(
				tan(m_box.GetANode(i)),
				tan(m_box.GetBNode(j)),
				m_box.GetPanel(),
				dUlon, dUlat,
				m_dataRefStateREdge[0][k][i][j],
				m_dataRefStateREdge[1][k][i][j]);
		}
	}
	}
	}
}

///////////////////////////////////////////////////////////////////////////////

void GridPatchCSGLL::EvaluateTestCase_StateOnly(
	const TestCase & test,
	const Time & time,
	int iDataIndex
) {
	// Initialize the data at each node
	if (m_datavecStateNode.size() == 0) {
		_EXCEPTIONT("InitializeData must be called before InitialConditions");
	}
	if (iDataIndex >= m_datavecStateNode.size()) {
		_EXCEPTIONT("Invalid iDataIndex (out of range)");
	}

	// 2D equation set
	bool fIs2DEquationSet = false;
	if (m_grid.GetModel().GetEquationSet().GetDimensionality() == 2) {
		fIs2DEquationSet = true;
	}

	// Check dimensionality
	if (fIs2DEquationSet && (m_nVerticalOrder != 1)) {
		_EXCEPTIONT("VerticalOrder / Dimensionality mismatch:\n"
			"For 2D problems vertical order must be 1.");
	}

	// Physical constants
	const PhysicalConstants & phys = m_grid.GetModel().GetPhysicalConstants();

	// Buffer vector for storing pointwise states
	int nComponents = m_grid.GetModel().GetEquationSet().GetComponents();
	int nTracers = m_grid.GetModel().GetEquationSet().GetTracers();

	DataVector<double> dPointwiseState;
	dPointwiseState.Initialize(nComponents);

	DataVector<double> dPointwiseTracers;
	if (m_datavecTracers.size() > 0) {
		dPointwiseTracers.Initialize(nTracers);
	}

	// Loop over all nodes
	for (int k = 0; k < m_grid.GetRElements(); k++) {
	for (int i = 0; i < m_box.GetATotalWidth(); i++) {
	for (int j = 0; j < m_box.GetBTotalWidth(); j++) {

		// Evaluate pointwise state
		test.EvaluatePointwiseState(
			phys,
			time,
			m_dataZLevels[k][i][j],
			m_dataLon[i][j],
			m_dataLat[i][j],
			dPointwiseState,
			dPointwiseTracers);

		for (int c = 0; c < dPointwiseState.GetRows(); c++) {
			m_datavecStateNode[iDataIndex][c][k][i][j] = dPointwiseState[c];
		}

		// Transform state velocities
		double dUlon;
		double dUlat;

		dUlon = m_datavecStateNode[iDataIndex][0][k][i][j];
		dUlat = m_datavecStateNode[iDataIndex][1][k][i][j];

		dUlon /= phys.GetEarthRadius();
		dUlat /= phys.GetEarthRadius();

		CubedSphereTrans::VecTransABPFromRLL(
			tan(m_box.GetANode(i)),
			tan(m_box.GetBNode(j)),
			m_box.GetPanel(),
			dUlon, dUlat,
			m_datavecStateNode[iDataIndex][0][k][i][j],
			m_datavecStateNode[iDataIndex][1][k][i][j]);
	}
	}
	}
}

///////////////////////////////////////////////////////////////////////////////

void GridPatchCSGLL::ComputeCurlAndDiv(
	const GridData3D & dataUa,
	const GridData3D & dataUb
) const {
	// Parent grid
	const GridCSGLL & gridCSGLL = dynamic_cast<const GridCSGLL &>(m_grid);

	// Get the pointer to the HorizontalDynamics object
	const HorizontalDynamicsDG * pHorizontalDynamicsDG =
		dynamic_cast<const HorizontalDynamicsDG *>(
			m_grid.GetModel().GetHorizontalDynamics());

	// If SpectralElement dynamics are used, apply direct stiffness summation
	bool fDiscontinuous = false;
	if (pHorizontalDynamicsDG != NULL) {
		fDiscontinuous = true;
	}

	// Get derivatives of the basis functions
	const DataMatrix<double> & dDxBasis1D = gridCSGLL.GetDxBasis1D();

	// Get derivatives of the flux reconstruction function
	const DataVector<double> & dFluxDeriv1D = gridCSGLL.GetFluxDeriv1D();

	// Number of finite elements in each direction
	int nAFiniteElements = m_box.GetAInteriorWidth() / m_nHorizontalOrder;
	int nBFiniteElements = m_box.GetBInteriorWidth() / m_nHorizontalOrder;

	// Loop over all elements in the box
	for (int k = 0; k < gridCSGLL.GetRElements(); k++) {
	for (int a = 0; a < nAFiniteElements; a++) {
	for (int b = 0; b < nBFiniteElements; b++) {

		// Index of lower-left corner node
		int iElementA = a * m_nHorizontalOrder + m_box.GetHaloElements();
		int iElementB = b * m_nHorizontalOrder + m_box.GetHaloElements();

		for (int i = 0; i < m_nHorizontalOrder; i++) {
		for (int j = 0; j < m_nHorizontalOrder; j++) {

			int iA = iElementA + i;
			int iB = iElementB + j;

			// Pointwise field values
			double dUa = dataUa[k][iA][iB];
			double dUb = dataUb[k][iA][iB];

			// Compute derivatives at each node
			double dDaUa = 0.0;
			double dDaUb = 0.0;
			double dDbUa = 0.0;
			double dDbUb = 0.0;

			double dDaJUa = 0.0;
			double dDbJUb = 0.0;

			for (int s = 0; s < m_nHorizontalOrder; s++) {
				dDaUa += dataUa[k][iElementA+s][iB] * dDxBasis1D[s][i];
				dDaUb += dataUb[k][iElementA+s][iB] * dDxBasis1D[s][i];
				dDbUa += dataUa[k][iA][iElementB+s] * dDxBasis1D[s][j];
				dDbUb += dataUb[k][iA][iElementB+s] * dDxBasis1D[s][j];
/*
				dDaJUa +=
					m_dataJacobian2D[iElementA+s][iB]
					* dataUa[k][iElementA+s][iB]
					* dDxBasis1D[s][i];

				dDbJUb +=
					m_dataJacobian2D[iA][iElementB+s]
					* dataUb[k][iA][iElementB+s]
					* dDxBasis1D[s][j];
*/
			}

			dDaUa /= GetElementDeltaA();
			dDaUb /= GetElementDeltaA();
			dDbUa /= GetElementDeltaB();
			dDbUb /= GetElementDeltaB();

			//dDaJUa /= GetElementDeltaA();
			//dDbJUb /= GetElementDeltaA();

			if (fDiscontinuous) {
				double dUpdateDerivA =
					  dFluxDeriv1D[m_nHorizontalOrder-1] / GetElementDeltaA();
				double dUpdateDerivB =
					  dFluxDeriv1D[m_nHorizontalOrder-1] / GetElementDeltaB();

				if (i == 0) {
					double dUaL = dataUa[k][iA-1][iB];
					double dUaR = dataUa[k][iA  ][iB];
					double dUbL = dataUb[k][iA-1][iB];
					double dUbR = dataUb[k][iA  ][iB];

					dDaUa += 0.5 * dUpdateDerivA * (dUaR - dUaL);
					dDaUb += 0.5 * dUpdateDerivA * (dUbR - dUbL);
				}
				if (i == m_nHorizontalOrder-1) {
					double dUaL = dataUa[k][iA  ][iB];
					double dUaR = dataUa[k][iA+1][iB];
					double dUbL = dataUb[k][iA  ][iB];
					double dUbR = dataUb[k][iA+1][iB];

					dDaUa += 0.5 * dUpdateDerivA * (dUaR - dUaL);
					dDaUb += 0.5 * dUpdateDerivA * (dUbR - dUbL);
				}
				if (j == 0) {
					double dUaL = dataUa[k][iA][iB-1];
					double dUaR = dataUa[k][iA][iB  ];
					double dUbL = dataUb[k][iA][iB-1];
					double dUbR = dataUb[k][iA][iB  ];

					dDbUa += 0.5 * dUpdateDerivB * (dUaR - dUaL);
					dDbUb += 0.5 * dUpdateDerivB * (dUbR - dUbL);
				}
				if (j == m_nHorizontalOrder-1) {
					double dUaL = dataUa[k][iA][iB  ];
					double dUaR = dataUa[k][iA][iB+1];
					double dUbL = dataUb[k][iA][iB  ];
					double dUbR = dataUb[k][iA][iB+1];

					dDbUa += 0.5 * dUpdateDerivB * (dUaR - dUaL);
					dDbUb += 0.5 * dUpdateDerivB * (dUbR - dUbL);
				}
			}

			// Compute covariant derivatives at node
			double dCovDaUa = dDaUa
				+ m_dataChristoffelA[iA][iB][0] * dUa
				+ m_dataChristoffelA[iA][iB][1] * 0.5 * dUb;

			double dCovDaUb = dDaUb
				+ m_dataChristoffelB[iA][iB][0] * dUa
				+ m_dataChristoffelB[iA][iB][1] * 0.5 * dUb;

			double dCovDbUa = dDbUa
				+ m_dataChristoffelA[iA][iB][1] * 0.5 * dUa
				+ m_dataChristoffelA[iA][iB][2] * dUb;

			double dCovDbUb = dDbUb
				+ m_dataChristoffelB[iA][iB][1] * 0.5 * dUa
				+ m_dataChristoffelB[iA][iB][2] * dUb;

			// Compute curl at node
			m_dataVorticity[k][iA][iB] = m_dataJacobian2D[iA][iB] * (
				+ m_dataContraMetric2DA[iA][iB][0] * dCovDaUb
				+ m_dataContraMetric2DA[iA][iB][1] * dCovDbUb
				- m_dataContraMetric2DB[iA][iB][0] * dCovDaUa
				- m_dataContraMetric2DB[iA][iB][1] * dCovDbUa);

			// Compute the divergence at node
			m_dataDivergence[k][iA][iB] = dCovDaUa + dCovDbUb;
			//double dInvJacobian = 1.0 / m_dataJacobian2D[iA][iB];
			//m_dataDivergence[k][iA][iB] =
			//	dInvJacobian * (dDaJUa + dDbJUb);
		}
		}
	}
	}
	}
}

///////////////////////////////////////////////////////////////////////////////

void GridPatchCSGLL::ComputeVorticityDivergence(
	int iDataIndex
) {
	// Physical constants
	const PhysicalConstants & phys = m_grid.GetModel().GetPhysicalConstants();

	// Working data
	const GridData4D & dataState = GetDataState(iDataIndex, DataLocation_Node);

	if (dataState.GetComponents() < 2) {
		_EXCEPTIONT(
			"Insufficient components for vorticity calculation");
	}

	// Get the alpha and beta components of vorticity
	GridData3D dataUa;
	GridData3D dataUb;

	dataState.GetAsGridData3D(0, dataUa);
	dataState.GetAsGridData3D(1, dataUb);

	// Compute the radial component of the curl of the velocity field
	ComputeCurlAndDiv(dataUa, dataUb);
}

///////////////////////////////////////////////////////////////////////////////

void GridPatchCSGLL::InterpolateNodeToREdge(
	int iVar,
	int iDataIndex
) {

	// Working data
	GridData4D & dataNode  = GetDataState(iDataIndex, DataLocation_Node);
	GridData4D & dataREdge = GetDataState(iDataIndex, DataLocation_REdge);

	// Parent grid, containing the vertical remapping information
	GridCSGLL * pCSGLLGrid = dynamic_cast<GridCSGLL*>(&m_grid);
	if (pCSGLLGrid == NULL) {
		_EXCEPTIONT("Logic error");
	}

	// Loop over all elements in the box
	int nStride = dataNode.GetSize(2) * dataNode.GetSize(3);

	const LinearColumnInterpFEM & opInterpNodeToREdge =
		pCSGLLGrid->GetOpInterpNodeToREdge();

	for (int i = m_box.GetAInteriorBegin(); i < m_box.GetAInteriorEnd(); i++) {
	for (int j = m_box.GetBInteriorBegin(); j < m_box.GetBInteriorEnd(); j++) {

		opInterpNodeToREdge.Apply(
			&(dataNode[iVar][0][i][j]),
			&(dataREdge[iVar][0][i][j]),
			nStride,
			nStride);
	}
	}
}

///////////////////////////////////////////////////////////////////////////////

void GridPatchCSGLL::InterpolateREdgeToNode(
	int iVar,
	int iDataIndex
) {

	// Working data
	GridData4D & dataREdge = GetDataState(iDataIndex, DataLocation_REdge);
	GridData4D & dataNode  = GetDataState(iDataIndex, DataLocation_Node);

	// Parent grid, containing the vertical remapping information
	GridCSGLL * pCSGLLGrid = dynamic_cast<GridCSGLL*>(&m_grid);
	if (pCSGLLGrid == NULL) {
		_EXCEPTIONT("Logic error");
	}

	// Loop over all elements in the box
	int nStride = dataNode.GetSize(2) * dataNode.GetSize(3);

	const LinearColumnInterpFEM & opInterpREdgeToNode =
		pCSGLLGrid->GetOpInterpREdgeToNode();

	// Loop over all elements in the box
	for (int i = m_box.GetAInteriorBegin(); i < m_box.GetAInteriorEnd(); i++) {
	for (int j = m_box.GetBInteriorBegin(); j < m_box.GetBInteriorEnd(); j++) {

		opInterpREdgeToNode.Apply(
			&(dataREdge[iVar][0][i][j]),
			&(dataNode[iVar][0][i][j]),
			nStride,
			nStride);
	}
	}
}

///////////////////////////////////////////////////////////////////////////////

void GridPatchCSGLL::InterpolateData(
	const DataVector<double> & dAlpha,
	const DataVector<double> & dBeta,
	const DataVector<int> & iPatch,
	DataType eDataType,
	DataLocation eDataLocation,
	bool fInterpAllVariables,
	DataMatrix3D<double> & dInterpData,
	bool fIncludeReferenceState,
	bool fConvertToPrimitive
) {
	if ((dAlpha.GetRows() != dBeta.GetRows()) ||
		(dAlpha.GetRows() != iPatch.GetRows())
	) {
		_EXCEPTIONT("Point vectors must have equivalent length.");
	}

	// Vector for storage interpolated points
	DataVector<double> dAInterpCoeffs;
	dAInterpCoeffs.Initialize(m_nHorizontalOrder);

	DataVector<double> dBInterpCoeffs;
	dBInterpCoeffs.Initialize(m_nHorizontalOrder);

	DataVector<double> dADiffCoeffs;
	dADiffCoeffs.Initialize(m_nHorizontalOrder);

	DataVector<double> dBDiffCoeffs;
	dBDiffCoeffs.Initialize(m_nHorizontalOrder);

	DataVector<double> dAInterpPt;
	dAInterpPt.Initialize(m_nHorizontalOrder);

	// Physical constants
	const PhysicalConstants & phys = m_grid.GetModel().GetPhysicalConstants();

	// Loop throught all points
	for (int i = 0; i < dAlpha.GetRows(); i++) {

		// Element index
		if (iPatch[i] != GetPatchIndex()) {
			continue;
		}

		// Verify point lies within domain of patch
		const double Eps = 1.0e-10;
		if ((dAlpha[i] < m_box.GetAEdge(m_box.GetAInteriorBegin()) - Eps) ||
			(dAlpha[i] > m_box.GetAEdge(m_box.GetAInteriorEnd()) + Eps) ||
			(dBeta[i] < m_box.GetBEdge(m_box.GetBInteriorBegin()) - Eps) ||
			(dBeta[i] > m_box.GetBEdge(m_box.GetBInteriorEnd()) + Eps)
		) {
			_EXCEPTIONT("Point out of range");
		}

		// Determine finite element index
		int iA =
			(dAlpha[i] - m_box.GetAEdge(m_box.GetAInteriorBegin()))
				/ GetElementDeltaA();

		int iB =
			(dBeta[i] - m_box.GetBEdge(m_box.GetBInteriorBegin()))
				/ GetElementDeltaB();

		// Bound the index within the element
		if (iA < 0) {
			iA = 0;
		}
		if (iA >= (m_box.GetAInteriorWidth() / m_nHorizontalOrder)) {
			iA = m_box.GetAInteriorWidth() / m_nHorizontalOrder - 1;
		}
		if (iB < 0) {
			iB = 0;
		}
		if (iB >= (m_box.GetBInteriorWidth() / m_nHorizontalOrder)) {
			iB = m_box.GetBInteriorWidth() / m_nHorizontalOrder - 1;
		}

		iA = m_box.GetHaloElements() + iA * m_nHorizontalOrder;
		iB = m_box.GetHaloElements() + iB * m_nHorizontalOrder;

		// Compute interpolation coefficients
		PolynomialInterp::LagrangianPolynomialCoeffs(
			m_nHorizontalOrder,
			&(m_box.GetAEdges()[iA]),
			dAInterpCoeffs,
			dAlpha[i]);

		PolynomialInterp::LagrangianPolynomialCoeffs(
			m_nHorizontalOrder,
			&(m_box.GetBEdges()[iB]),
			dBInterpCoeffs,
			dBeta[i]);

		// Perform interpolation on all variables
		int nComponents;
		int nRElements = m_grid.GetRElements();

		double ** pData2D;

		// State Data: Perform interpolation on all variables
		if (eDataType == DataType_State) {
			if (eDataLocation == DataLocation_Node) {
				nComponents = m_datavecStateNode[0].GetComponents();
			} else {
				nComponents = m_datavecStateREdge[0].GetComponents();
				nRElements = m_grid.GetRElements() + 1;
			}

		// Tracer Data: Perform interpolation on all variables
		} else if (eDataType == DataType_Tracers) {
			nComponents = m_datavecTracers[0].GetComponents();

		// Topography Data: Special handling due to 2D nature of data
		} else if (eDataType == DataType_Topography) {
			nComponents = 1;
			pData2D = (double**)(m_dataTopography);

		// Vorticity Data
		} else if (eDataType == DataType_Vorticity) {
			nComponents = 1;

		// Divergence Data
		} else if (eDataType == DataType_Divergence) {
			nComponents = 1;

		// Temperature Data
		} else if (eDataType == DataType_Temperature) {
			nComponents = 1;

		} else {
			_EXCEPTIONT("Invalid DataType");
		}

		for (int c = 0; c < nComponents; c++) {

			int nRElements = m_grid.GetRElements();

			// Get a pointer to the 3D data structure
			const double *** pData;
			if (eDataType == DataType_State) {
				if (eDataLocation == DataLocation_Node) {
					pData = (const double ***)(m_datavecStateNode[0][c]);
				} else {
					pData = (const double ***)(m_datavecStateREdge[0][c]);
				}

			} else if (eDataType == DataType_Tracers) {
				pData = (const double ***)(m_datavecTracers[0][c]);

			} else if (eDataType == DataType_Topography) {
				pData = (const double ***)(&pData2D);
				nRElements = 1;

			} else if (eDataType == DataType_Vorticity) {
				pData = (const double ***)(double ***)(m_dataVorticity);

			} else if (eDataType == DataType_Divergence) {
				pData = (const double ***)(double ***)(m_dataDivergence);

			} else if (eDataType == DataType_Temperature) {
				pData = (const double ***)(double ***)(m_dataTemperature);
			}

			// Perform interpolation on all levels
			for (int k = 0; k < nRElements; k++) {

				dInterpData[c][k][i] = 0.0;

				for (int m = 0; m < m_nHorizontalOrder; m++) {
				for (int n = 0; n < m_nHorizontalOrder; n++) {
					dInterpData[c][k][i] +=
						  dAInterpCoeffs[m]
						* dBInterpCoeffs[n]
						* pData[k][iA+m][iB+n];
				}
				}

				// Do not include the reference state
				if ((eDataType == DataType_State) &&
					(!fIncludeReferenceState)
				) {
					if (eDataLocation == DataLocation_Node) {
						for (int m = 0; m < m_nHorizontalOrder; m++) {
						for (int n = 0; n < m_nHorizontalOrder; n++) {
							dInterpData[c][k][i] -=
								  dAInterpCoeffs[m]
								* dBInterpCoeffs[n]
								* m_dataRefStateNode[c][k][iA+m][iB+n];
						}
						}

					} else {
						for (int m = 0; m < m_nHorizontalOrder; m++) {
						for (int n = 0; n < m_nHorizontalOrder; n++) {
							dInterpData[c][k][i] -=
								  dAInterpCoeffs[m]
								* dBInterpCoeffs[n]
								* m_dataRefStateREdge[c][k][iA+m][iB+n];
						}
						}
					}
				}
			}
		}

		// Convert to primitive variables
		if ((eDataType == DataType_State) && (fConvertToPrimitive)) {
			for (int k = 0; k < m_grid.GetRElements(); k++) {
				double dUalpha = phys.GetEarthRadius()
					* dInterpData[0][k][i];
				double dUbeta = phys.GetEarthRadius()
					* dInterpData[1][k][i];

				CubedSphereTrans::VecTransRLLFromABP(
					tan(dAlpha[i]),
					tan(dBeta[i]),
					GetPatchBox().GetPanel(),
					dUalpha,
					dUbeta,
					dInterpData[0][k][i],
					dInterpData[1][k][i]);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void GridPatchCSGLL::TransformHaloVelocities(
	int iDataUpdate
) {
	// Indices of velocities
	const int UIx = 0;
	const int VIx = 1;

	// Velocity data
	GridData4D * pDataVelocity =
		&(GetDataState(iDataUpdate, m_grid.GetVarLocation(UIx)));

	if (pDataVelocity->GetComponents() < 2) {
		_EXCEPTIONT("Invalid number of components.");
	}

	// Panels in each coordinate direction
	int ixRightPanel  = GetNeighborPanel(Direction_Right);
	int ixTopPanel    = GetNeighborPanel(Direction_Top);
	int ixLeftPanel   = GetNeighborPanel(Direction_Left);
	int ixBottomPanel = GetNeighborPanel(Direction_Bottom);

	int ixTopRightPanel    = GetNeighborPanel(Direction_TopRight);
	int ixTopLeftPanel     = GetNeighborPanel(Direction_TopLeft);
	int ixBottomLeftPanel  = GetNeighborPanel(Direction_BottomLeft);
	int ixBottomRightPanel = GetNeighborPanel(Direction_BottomRight);

	// Post-process velocities across right edge
	if (ixRightPanel != m_box.GetPanel()) {
		int i;
		int j;

		int jBegin = m_box.GetBInteriorBegin()-1;
		int jEnd = m_box.GetBInteriorEnd()+1;

		i = m_box.GetAInteriorEnd();
		for (int k = 0; k < pDataVelocity->GetRElements(); k++) {
		for (j = jBegin; j < jEnd; j++) {
			CubedSphereTrans::VecPanelTrans(
				ixRightPanel,
				m_box.GetPanel(),
				(*pDataVelocity)[0][k][i][j],
				(*pDataVelocity)[1][k][i][j],
				tan(m_box.GetANode(i)),
				tan(m_box.GetBNode(j)));
		}
		}
	}

	// Post-process velocities across top edge
	if (ixTopPanel != m_box.GetPanel()) {
		int i;
		int j;

		int iBegin = m_box.GetAInteriorBegin()-1;
		int iEnd = m_box.GetAInteriorEnd()+1;

		j = m_box.GetBInteriorEnd();
		for (int k = 0; k < pDataVelocity->GetRElements(); k++) {
		for (i = iBegin; i < iEnd; i++) {
			CubedSphereTrans::VecPanelTrans(
				ixTopPanel,
				m_box.GetPanel(),
				(*pDataVelocity)[0][k][i][j],
				(*pDataVelocity)[1][k][i][j],
				tan(m_box.GetANode(i)),
				tan(m_box.GetBNode(j)));
		}
		}
	}

	// Post-process velocities across left edge
	if (ixLeftPanel != m_box.GetPanel()) {
		int i;
		int j;

		int jBegin = m_box.GetBInteriorBegin()-1;
		int jEnd = m_box.GetBInteriorEnd()+1;

		i = m_box.GetAInteriorBegin()-1;
		for (int k = 0; k < pDataVelocity->GetRElements(); k++) {
		for (j = jBegin; j < jEnd; j++) {
			CubedSphereTrans::VecPanelTrans(
				ixLeftPanel,
				m_box.GetPanel(),
				(*pDataVelocity)[0][k][i][j],
				(*pDataVelocity)[1][k][i][j],
				tan(m_box.GetANode(i)),
				tan(m_box.GetBNode(j)));
		}
		}
	}

	// Post-process velocities across bottom edge
	if (ixBottomPanel != m_box.GetPanel()) {
		int i;
		int j;

		int iBegin = m_box.GetAInteriorBegin()-1;
		int iEnd = m_box.GetAInteriorEnd()+1;

		j = m_box.GetBInteriorBegin()-1;
		for (int k = 0; k < pDataVelocity->GetRElements(); k++) {
		for (i = iBegin; i < iEnd; i++) {
			CubedSphereTrans::VecPanelTrans(
				ixBottomPanel,
				m_box.GetPanel(),
				(*pDataVelocity)[0][k][i][j],
				(*pDataVelocity)[1][k][i][j],
				tan(m_box.GetANode(i)),
				tan(m_box.GetBNode(j)));
		}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////


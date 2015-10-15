///////////////////////////////////////////////////////////////////////////////
///
///	\file    GridCSGLL.cpp
///	\author  Paul Ullrich
///	\version September 19, 2013
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

#include "GridCSGLL.h"
#include "GridPatchCSGLL.h"
#include "Model.h"
#include "TestCase.h"
#include "GridSpacing.h"

#include "Direction.h"
#include "CubedSphereTrans.h"
#include "PolynomialInterp.h"

#include "Announce.h"
#include "MathHelper.h"

#include <cmath>

///////////////////////////////////////////////////////////////////////////////

GridCSGLL::GridCSGLL(
	Model & model,
	int nBaseResolution,
	int nRefinementRatio,
	int nHorizontalOrder,
	int nVerticalOrder,
	int nRElements,
	VerticalStaggering eVerticalStaggering
) :
	// Call up the stack
	GridGLL::GridGLL(
		model,
		nBaseResolution,
		nBaseResolution,
		nRefinementRatio,
		nHorizontalOrder,
		nVerticalOrder,
		nRElements,
		eVerticalStaggering)
{
	// Set the reference length scale
	m_dReferenceLength = 0.5 * M_PI / 30.0;
}

///////////////////////////////////////////////////////////////////////////////

void GridCSGLL::Initialize() {

	// Call up the stack
	GridGLL::Initialize();

	// Distribute patches to processors
	Grid::DistributePatches();

	// Set up connectivity
	Grid::InitializeConnectivity();
}

///////////////////////////////////////////////////////////////////////////////

void GridCSGLL::AddDefaultPatches() {

	// Verify no Patches have been previously added
	if (GetPatchCount() != 0) {
		_EXCEPTIONT("AddDefaultPatches() must be called on an empty Grid");
	}

	// Determine number of usable processors
	int nCommSize;
	MPI_Comm_size(MPI_COMM_WORLD, &nCommSize);

	int nProcsPerDirection = Max((int)ISqrt(nCommSize / 6), 1);

	int nProcsPerPanel = nProcsPerDirection * nProcsPerDirection;

	int nDistributedPatches = 6 * nProcsPerPanel;

	if (nDistributedPatches < nCommSize) {
		Announce("WARNING: Patch / thread mismatch: "
			"%i threads will be unutilized",
			nCommSize - nDistributedPatches);
	}

	// Determine arrangement of elements on processors
	if (GetABaseResolution() % nProcsPerDirection != 0) {
		_EXCEPTIONT("\n(UNIMPLEMENTED) Currently elements must be "
			"equally divided among processors.");
	}

	int nElementsPerDirection = GetABaseResolution() / nProcsPerDirection;
	DataArray1D<int> iBoxBegin(nProcsPerDirection + 1);

	iBoxBegin[0] = 0;
	for (int n = 1; n < nProcsPerDirection; n++) {
		iBoxBegin[n] = n * nElementsPerDirection;
	}
	iBoxBegin[nProcsPerDirection] = GetABaseResolution();

	// Create master patch for each panel
	for (int n = 0; n < 6; n++) {
	for (int i = 0; i < nProcsPerDirection; i++) {
	for (int j = 0; j < nProcsPerDirection; j++) {

		double dDeltaA = 0.5 * M_PI / GetABaseResolution();

		GridSpacingGaussLobattoRepeated
			glspacing(dDeltaA, -0.25 * M_PI, m_nHorizontalOrder);

		PatchBox boxMaster(
			n, 0, m_model.GetHaloElements(),
			m_nHorizontalOrder * iBoxBegin[i],
			m_nHorizontalOrder * iBoxBegin[i+1],
			m_nHorizontalOrder * iBoxBegin[j],
			m_nHorizontalOrder * iBoxBegin[j+1],
			glspacing,
			glspacing);

		int ixPatch = n * nProcsPerPanel + i * nProcsPerDirection + j;

		Grid::AddPatch(
			new GridPatchCSGLL(
				(*this),
				ixPatch,
				boxMaster,
				m_nHorizontalOrder,
				m_nVerticalOrder));
	}
	}
	}
}

///////////////////////////////////////////////////////////////////////////////

GridPatch * GridCSGLL::AddPatch(
	int ixPatch,
	const PatchBox & box
) {
	return
		Grid::AddPatch(
			new GridPatchCSGLL(
				(*this),
				ixPatch,
				box,
				m_nHorizontalOrder,
				m_nVerticalOrder));
}

///////////////////////////////////////////////////////////////////////////////

void GridCSGLL::GetReferenceGridBounds(
	double & dX0,
	double & dX1,
	double & dY0,
	double & dY1
) {
	dX0 = - 180.0;
	dX1 = + 180.0;
	dY0 = -  90.0;
	dY1 = +  90.0;
}

///////////////////////////////////////////////////////////////////////////////

void GridCSGLL::ConvertReferenceToPatchCoord(
	const DataArray1D<double> & dXReference,
	const DataArray1D<double> & dYReference,
	DataArray1D<double> & dAlpha,
	DataArray1D<double> & dBeta,
	DataArray1D<int> & iPatch
) const {
	if ((dXReference.GetRows() != dYReference.GetRows()) ||
		(dXReference.GetRows() != dAlpha.GetRows()) ||
		(dXReference.GetRows() != dBeta.GetRows()) ||
		(dXReference.GetRows() != iPatch.GetRows())
	) {
		_EXCEPTIONT("Dimension mismatch: All arrays must have same length");
	}

	// Loop over all coordinates
	for (int i = 0; i < dXReference.GetRows(); i++) {

		int iPanel;

		CubedSphereTrans::ABPFromRLL(
			dXReference[i] * M_PI / 180.0,
			dYReference[i] * M_PI / 180.0,
			dAlpha[i],
			dBeta[i],
			iPanel);

		// Loop over all patches
		int n = 0;

		for (; n < GetPatchCount(); n++) {
			const GridPatch * pPatch = GetPatch(n);
			const PatchBox & box = pPatch->GetPatchBox();

			if (iPanel != box.GetPanel()) {
				continue;
			}
			if ((dAlpha[i] >= box.GetAEdge(box.GetAInteriorBegin())) &&
				(dAlpha[i] <= box.GetAEdge(box.GetAInteriorEnd())) &&
				(dBeta[i] >= box.GetBEdge(box.GetBInteriorBegin())) &&
				(dBeta[i] <= box.GetBEdge(box.GetBInteriorEnd()))
			) {
				iPatch[i] = pPatch->GetPatchIndex();
				break;
			}
		}

		if (n == GetPatchCount()) {
			_EXCEPTION5("Unable to find associated patch for node:\n"
				"(%1.5e, %1.5e) : (%1.5e, %1.5e, %i)",
				dXReference[i], dYReference[i],
				dAlpha[i], dBeta[i], iPanel);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void GridCSGLL::GetPatchFromCoordinateIndex(
	int iRefinementLevel,
	const DataArray1D<int> & vecIxA,
	const DataArray1D<int> & vecIxB,
	const DataArray1D<int> & vecPanel,
	DataArray1D<int> & vecPatchIndex,
	int nVectorLength
) {
	// Set vector length
	if (nVectorLength == (-1)) {
		nVectorLength = vecIxA.GetRows();
	}

	// Check arguments
	if ((vecIxA.GetRows() < nVectorLength) ||
		(vecIxB.GetRows() < nVectorLength) ||
		(vecPanel.GetRows() < nVectorLength)
	) {
		_EXCEPTIONT("Argument vector length mismatch");
	}
	if (iRefinementLevel < 0) {
		_EXCEPTIONT("Refinement level must be positive");
	}

	// Calculate local resolution
	int nLocalResolution =
		m_nHorizontalOrder * GetABaseResolution(iRefinementLevel);

	// Loop through all entries
	int iLastPatch = GridPatch::InvalidIndex;
	for (int i = 0; i < nVectorLength; i++) {

		int iA = vecIxA[i];
		int iB = vecIxB[i];
		int iP = vecPanel[i];

		// Transform to alternative panel if necessary
		if ((vecIxA[i] < 0) || (vecIxA[i] >= nLocalResolution) ||
			(vecIxB[i] < 0) || (vecIxB[i] >= nLocalResolution)
		) {
			bool fSwitchAB;
			bool fSwitchPar;
			bool fSwitchPerp;

			CubedSphereTrans::RelativeCoord(
				nLocalResolution,
				vecPanel[i], vecIxA[i], vecIxB[i],
				iP, iA, iB,
				fSwitchAB,
				fSwitchPar,
				fSwitchPerp);

			// Coordinate out of bounds
			if ((iA == (-1)) && (iB == (-1))) {
				vecPatchIndex[i] = GridPatch::InvalidIndex;
				continue;
			}
		}

		// Check the last patch searched
		if (iLastPatch != GridPatch::InvalidIndex) {
			const GridPatch * pPatch = GetPatch(iLastPatch);

			const PatchBox & box = pPatch->GetPatchBox();

			if (box.ContainsGlobalPoint(iP, iA, iB)) {
				vecPatchIndex[i] = pPatch->GetPatchIndex();
				continue;
			}
		}

		// Check all other patches
		int n;
		for (n = 0; n < GetPatchCount(); n++) {
			const GridPatch * pPatch = GetPatch(n);

			const PatchBox & box = pPatch->GetPatchBox();

			if (box.ContainsGlobalPoint(iP, iA, iB)) {
				vecPatchIndex[i] = pPatch->GetPatchIndex();
				iLastPatch = n;
				break;
			}
		}
		if (n == GetPatchCount()) {
			vecPatchIndex[i] = GridPatch::InvalidIndex;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void GridCSGLL::GetOpposingDirection(
	int ixPanelSrc,
	int ixPanelDest,
	Direction dir,
	Direction & dirOpposing,
	bool & fSwitchParallel,
	bool & fSwitchPerpendicular
) const {
	if ((ixPanelSrc < 0) || (ixPanelSrc > 5)) {
		_EXCEPTIONT("Invalid value for ixPanelSrc: Out of range");
	}
	if ((ixPanelDest < 0) || (ixPanelDest > 5)) {
		_EXCEPTIONT("Invalid value for ixPanelDest: Out of range");
	}

	// Get the opposing direction
	dirOpposing =
		CubedSphereTrans::OpposingDirection(
			ixPanelSrc, ixPanelDest, dir);

	// Check panel indices to determine parallel switch of vectors
	fSwitchParallel = false;
	if ((ixPanelSrc == 1) && (ixPanelDest == 5)) {
		fSwitchParallel = true;

	} else if ((ixPanelSrc == 2) && (ixPanelDest > 3)) {
		fSwitchParallel = true;

	} else if ((ixPanelSrc == 3) && (ixPanelDest == 4)) {
		fSwitchParallel = true;

	} else if (
		(ixPanelSrc == 4) &&
		((ixPanelDest == 2) || (ixPanelDest == 3))
	) {
		fSwitchParallel = true;

	} else if (
		(ixPanelSrc == 5) &&
		((ixPanelDest == 2) || (ixPanelDest == 1))
	) {
		fSwitchParallel = true;
	}

	// Check panel indices to determine perpendicular switch of vectors
	fSwitchPerpendicular = false;
	if ((ixPanelSrc == 1) && (ixPanelDest == 4)) {
		fSwitchPerpendicular = true;

	} else if ((ixPanelSrc == 2) && (ixPanelDest > 3)) {
		fSwitchPerpendicular = true;

	} else if ((ixPanelSrc == 3) && (ixPanelDest == 5)) {
		fSwitchPerpendicular = true;

	} else if (
		(ixPanelSrc == 4) &&
		((ixPanelDest == 1) || (ixPanelDest == 2))
	) {
		fSwitchPerpendicular = true;

	} else if (
		(ixPanelSrc == 5) &&
		((ixPanelDest == 3) || (ixPanelDest == 2))
	) {
		fSwitchPerpendicular = true;
	}
}

///////////////////////////////////////////////////////////////////////////////

void GridCSGLL::ApplyDSS(
	int iDataUpdate,
	DataType eDataType
) {
	// Exchange data between nodes
	Exchange(eDataType, iDataUpdate);

	// Post-process velocities across panel edges and
	// perform direct stiffness summation (DSS)
	for (int n = 0; n < GetActivePatchCount(); n++) {
		GridPatchCSGLL * pPatch =
			dynamic_cast<GridPatchCSGLL*>(GetActivePatch(n));

		const PatchBox & box = pPatch->GetPatchBox();

        // Patch-specific quantities
		int nElementCountA = pPatch->GetElementCountA();
		int nElementCountB = pPatch->GetElementCountB();

		// Apply panel transforms to velocity data
		if (eDataType == DataType_State) {
			pPatch->TransformHaloVelocities(iDataUpdate);
		}
		if (eDataType == DataType_TopographyDeriv) {
			pPatch->TransformTopographyDeriv();

			const DataArray3D<double> & dataTopographyDeriv =
				pPatch->GetTopographyDeriv();
		}

		// Panels in each coordinate direction
		int ixRightPanel =
			pPatch->GetNeighborPanel(Direction_Right);
		int ixTopPanel =
			pPatch->GetNeighborPanel(Direction_Top);
		int ixLeftPanel =
			pPatch->GetNeighborPanel(Direction_Left);
		int ixBottomPanel =
			pPatch->GetNeighborPanel(Direction_Bottom);

		int ixTopRightPanel =
			pPatch->GetNeighborPanel(Direction_TopRight);
		int ixTopLeftPanel =
			pPatch->GetNeighborPanel(Direction_TopLeft);
		int ixBottomLeftPanel =
			pPatch->GetNeighborPanel(Direction_BottomLeft);
		int ixBottomRightPanel =
			pPatch->GetNeighborPanel(Direction_BottomRight);

		// Loop through all components associated with this DataType
		int nComponents;
		if (eDataType == DataType_State) {
			nComponents = m_model.GetEquationSet().GetComponents();
		} else if (eDataType == DataType_Tracers) {
			nComponents = m_model.GetEquationSet().GetTracers();
		} else if (eDataType == DataType_Vorticity) {
			nComponents = 1;
		} else if (eDataType == DataType_Divergence) {
			nComponents = 1;
		} else if (eDataType == DataType_TopographyDeriv) {
			nComponents = 1;
		} else {
			_EXCEPTIONT("Invalid DataType");
		}

		// Perform Direct Stiffness Summation (DSS)
		for (int c = 0; c < nComponents; c++) {

			// Obtain the array of working data
			int nRElements = GetRElements();
			double *** pDataUpdate;
			if (eDataType == DataType_State) {
				pDataUpdate =
					pPatch->GetDataState(iDataUpdate, GetVarLocation(c))[c];

				if (GetVarLocation(c) == DataLocation_REdge) {
					nRElements++;
				}

			} else if (eDataType == DataType_Tracers) {
				pDataUpdate =
					pPatch->GetDataTracers(iDataUpdate)[c];
			} else if (eDataType == DataType_Vorticity) {
				pDataUpdate = pPatch->GetDataVorticity();
			} else if (eDataType == DataType_Divergence) {
				pDataUpdate = pPatch->GetDataDivergence();
			} else if (eDataType == DataType_TopographyDeriv) {
				pDataUpdate = pPatch->GetTopographyDeriv();

				nRElements = 2;
			}

			for (int k = 0; k < nRElements; k++) {

				// Average in the alpha direction
				for (int a = 0; a <= nElementCountA; a++) {
					int iA = a * m_nHorizontalOrder + box.GetHaloElements();

					// Do not average across cubed-sphere corners
					int jBegin = box.GetBInteriorBegin()-1;
					int jEnd = box.GetBInteriorEnd()+1;

					if (((a == 0) &&
							(ixTopLeftPanel == InvalidPanel)) ||
						((a == nElementCountA) &&
							(ixTopRightPanel == InvalidPanel))
					) {
						jEnd -= 2;
					}
					if (((a == 0) &&
							(ixBottomLeftPanel == InvalidPanel)) ||
						((a == nElementCountA) &&
							(ixBottomRightPanel == InvalidPanel))
					) {
						jBegin += 2;
					}

					// Perform averaging across edge
					for (int j = jBegin; j < jEnd; j++) {
						pDataUpdate[k][iA][j] = 0.5 * (
							+ pDataUpdate[k][iA  ][j]
							+ pDataUpdate[k][iA-1][j]);

						pDataUpdate[k][iA-1][j] = pDataUpdate[k][iA][j];
					}
				}

				// Average in the beta direction
				for (int b = 0; b <= nElementCountB; b++) {
					int iB = b * m_nHorizontalOrder + box.GetHaloElements();

					// Do not average across cubed-sphere corners
					int iBegin = box.GetAInteriorBegin()-1;
					int iEnd = box.GetAInteriorEnd()+1;

					if (((b == 0) &&
							(ixBottomLeftPanel == InvalidPanel)) ||
						((b == nElementCountA) &&
							(ixTopLeftPanel == InvalidPanel))
					) {
						iBegin += 2;
					}
					if (((b == 0) &&
							(ixBottomRightPanel == InvalidPanel)) ||
						((b == nElementCountA) &&
							(ixTopRightPanel == InvalidPanel))
					) {
						iEnd -= 2;
					}

					for (int i = iBegin; i < iEnd; i++) {
						pDataUpdate[k][i][iB] = 0.5 * (
							+ pDataUpdate[k][i][iB  ]
							+ pDataUpdate[k][i][iB-1]);

						pDataUpdate[k][i][iB-1] = pDataUpdate[k][i][iB];
					}
				}

				// Average at cubed-sphere corners (nodes of connectivity 3)
				if (ixTopRightPanel == InvalidPanel) {
					int iA = box.GetAInteriorEnd()-1;
					int iB = box.GetBInteriorEnd()-1;

					pDataUpdate[k][iA][iB] = (1.0/3.0) * (
						+ pDataUpdate[k][iA  ][iB  ]
						+ pDataUpdate[k][iA+1][iB  ]
						+ pDataUpdate[k][iA  ][iB+1]);
				}

				if (ixTopLeftPanel == InvalidPanel) {
					int iA = box.GetAInteriorBegin();
					int iB = box.GetBInteriorEnd()-1;

					pDataUpdate[k][iA][iB] = (1.0/3.0) * (
						+ pDataUpdate[k][iA  ][iB  ]
						+ pDataUpdate[k][iA-1][iB  ]
						+ pDataUpdate[k][iA  ][iB+1]);
				}

				if (ixBottomLeftPanel == InvalidPanel) {
					int iA = box.GetAInteriorBegin();
					int iB = box.GetBInteriorBegin();

					pDataUpdate[k][iA][iB] = (1.0/3.0) * (
						+ pDataUpdate[k][iA  ][iB  ]
						+ pDataUpdate[k][iA-1][iB  ]
						+ pDataUpdate[k][iA  ][iB-1]);
				}

				if (ixBottomRightPanel == InvalidPanel) {
					int iA = box.GetAInteriorEnd()-1;
					int iB = box.GetBInteriorBegin();

					pDataUpdate[k][iA][iB] = (1.0/3.0) * (
						+ pDataUpdate[k][iA  ][iB  ]
						+ pDataUpdate[k][iA+1][iB  ]
						+ pDataUpdate[k][iA  ][iB-1]);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////


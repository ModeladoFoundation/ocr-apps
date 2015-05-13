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

#include "GridPatchGLL.h"

#include "GaussLobattoQuadrature.h"

#include "PolynomialInterp.h"

///////////////////////////////////////////////////////////////////////////////

GridPatchGLL::GridPatchGLL(
	Grid & grid,
	int ixPatch,
	const PatchBox & box,
	int nHorizontalOrder,
	int nVerticalOrder
) :
	GridPatch(grid, ixPatch, box),
	m_nHorizontalOrder(nHorizontalOrder),
	m_nVerticalOrder(nVerticalOrder)
{
	// Verify that box boundaries are aligned with elements
	if (((box.GetAGlobalInteriorBegin() % nHorizontalOrder) != 0) ||
		((box.GetAGlobalInteriorEnd()   % nHorizontalOrder) != 0) ||
		((box.GetBGlobalInteriorBegin() % nHorizontalOrder) != 0) ||
		((box.GetBGlobalInteriorEnd()   % nHorizontalOrder) != 0)
	) {
		_EXCEPTION4(
			"GLL grid patch must be aligned with elements: %i %i %i %i",
			box.GetAGlobalInteriorBegin(),
			box.GetAGlobalInteriorEnd(),
			box.GetBGlobalInteriorBegin(),
			box.GetBGlobalInteriorEnd());
	}

	// Get the number of finite elements in each coordinate direction
	m_nElementCountA =
		(box.GetAInteriorEnd() - box.GetAInteriorBegin()) / nHorizontalOrder;
	m_nElementCountB =
		(box.GetBInteriorEnd() - box.GetBInteriorBegin()) / nHorizontalOrder;

	if ((box.GetAInteriorWidth() % m_nHorizontalOrder) != 0) {
		_EXCEPTIONT("Logic error: Invalid PatchBox alpha spacing");
	}
	if ((box.GetBInteriorWidth() % m_nHorizontalOrder) != 0) {
		_EXCEPTIONT("Logic error: Invalid PatchBox beta spacing");
	}

	// Element grid spacing
	m_dElementDeltaA =
		  box.GetAEdge(box.GetHaloElements() + m_nHorizontalOrder)
		- box.GetAEdge(box.GetHaloElements());

	m_dElementDeltaB =
		  box.GetBEdge(box.GetHaloElements() + m_nHorizontalOrder)
		- box.GetBEdge(box.GetHaloElements());
}

///////////////////////////////////////////////////////////////////////////////


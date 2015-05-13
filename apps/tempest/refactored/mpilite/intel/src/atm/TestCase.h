///////////////////////////////////////////////////////////////////////////////
///
///	\file    TestCase.h
///	\author  Paul Ullrich
///	\version February 24, 2013
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

#ifndef _TESTCASE_H_
#define _TESTCASE_H_

#include "PhysicalConstants.h"
#include "EquationSet.h"

#include "GridData4D.h"

class Time;

///////////////////////////////////////////////////////////////////////////////

///	<summary>
///		Interface for model test cases.
///	</summary>
class TestCase {

public:
	///	<summary>
	///		Virtual destructor.
	///	</summary>
	virtual ~TestCase() {
	}

public:
	///	<summary>
	///		Equation set type.
	///	</summary>
	virtual EquationSet::Type GetEquationSetType() const {
		return EquationSet::AdvectionEquations;
	}

	///	<summary>
	///		Get the altitude of the model cap.
	///	</summary>
	virtual double GetZtop() const {
		return 1.0;
	}

	///	<summary>
	///		Obtain test case specific physical constants.
	///	</summary>
	virtual void EvaluatePhysicalConstants(
		PhysicalConstants & phys
	) const {
	}

	///	<summary>
	///		Evaluate the topography at the given point.
	///	</summary>
	virtual double EvaluateTopography(
		const PhysicalConstants & phys,
		double dLon,
		double dLat
	) const {
		return 0.0;
	}

	///	<summary>
	///		Flag indicating whether or not Rayleigh friction strength is given.
	///	</summary>
	virtual bool HasRayleighFriction() const {
		return false;
	}

	///	<summary>
	///		Evaluate the Rayleigh friction strength at the given point.
	///	</summary>
	virtual double EvaluateRayleighStrength(
		double dZ,
		double dLon,
		double dLat
	) const {
		return 0.0;
	}

	///	<summary>
	///		Flag indicating whether or not a reference state is available.
	///	</summary>
	virtual bool HasReferenceState() const {
		return false;
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
	) const = 0;
};

///////////////////////////////////////////////////////////////////////////////

#endif


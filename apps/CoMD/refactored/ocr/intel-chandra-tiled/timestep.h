/// \file
/// Leapfrog time integrator

#ifndef __LEAPFROG_H
#define __LEAPFROG_H

#include "CoMDTypes.h"

double timestep(SimFlat* s, int n, real_t dt);
ocrGuid_t timestepLoopEdt( EDT_ARGS );

void computeForce(SimFlat* s);
ocrGuid_t computeForceEdt( EDT_ARGS );
ocrGuid_t advanceVelocityEdt( EDT_ARGS );
ocrGuid_t advancePositionEdt( EDT_ARGS );
void kineticEnergy(SimFlat* s);
ocrGuid_t kineticEnergyEdt( EDT_ARGS );
ocrGuid_t timestepLoopEdt( EDT_ARGS );
ocrGuid_t timestepEdt( EDT_ARGS );

/// Update local and remote link cells after atoms have moved.
void redistributeAtoms(struct SimFlatSt* sim);
ocrGuid_t redistributeAtomsEdt( EDT_ARGS );

ocrGuid_t printThingsEdt( EDT_ARGS );

#endif

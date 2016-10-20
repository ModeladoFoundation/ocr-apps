/// \file
/// Initialize the atom configuration.

#include "initAtoms.h"

#include <math.h>

#include "constants.h"
#include "decomposition.h"
#include "random.h"
#include "linkCells.h"
#include "timestep.h"
#include "performanceTimers.h"

static void computeVcmOCR(SimFlat* s, real_t vcmLocal[4]);

/// \details
/// Call functions such as createFccLattice and setTemperature to set up
/// initial atom positions and momenta.
void initAtoms(Atoms* atoms, LinkCell* boxes)
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

   int maxTotalAtoms = MAXATOMS*boxes->nTotalBoxes;

   ocrDbCreate( &(atoms->DBK_gid), (void **) &atoms->gid, maxTotalAtoms*sizeof(int),
                DB_PROP_NONE, NULL_HINT, NO_ALLOC );
   ocrDbCreate( &(atoms->DBK_iSpecies), (void **) &atoms->iSpecies, maxTotalAtoms*sizeof(int),
                DB_PROP_NONE, NULL_HINT, NO_ALLOC );
   ocrDbCreate( &(atoms->DBK_r), (void **) &atoms->r, maxTotalAtoms*sizeof(real3),
                DB_PROP_NONE, NULL_HINT, NO_ALLOC );
   ocrDbCreate( &(atoms->DBK_p), (void **) &atoms->p, maxTotalAtoms*sizeof(real3),
                DB_PROP_NONE, NULL_HINT, NO_ALLOC );
   ocrDbCreate( &(atoms->DBK_f), (void **) &atoms->f, maxTotalAtoms*sizeof(real3),
                DB_PROP_NONE, NULL_HINT, NO_ALLOC );
   ocrDbCreate( &(atoms->DBK_U), (void **) &atoms->U, maxTotalAtoms*sizeof(real_t),
                DB_PROP_NONE, NULL_HINT, NO_ALLOC );

   atoms->nLocal = 0;
   atoms->nGlobal = 0;

   for (int iOff = 0; iOff < maxTotalAtoms; iOff++)
   {
      atoms->gid[iOff] = 0;
      atoms->iSpecies[iOff] = 0;
      zeroReal3(atoms->r[iOff]);
      zeroReal3(atoms->p[iOff]);
      zeroReal3(atoms->f[iOff]);
      atoms->U[iOff] = 0.;
   }
}

//void destroyAtoms(Atoms *atoms)
//{
//}

/// Creates atom positions on a face centered cubic (FCC) lattice with
/// nx * ny * nz unit cells and lattice constant lat.
/// Set momenta to zero.
void createFccLattice(int nx, int ny, int nz, real_t lat, SimFlat* s)
{
    DEBUG_PRINTF(( "%s\n", __func__ ));
   const real_t* localMin = s->domain->localMin; // alias
   const real_t* localMax = s->domain->localMax; // alias

   int nb = 4; // number of atoms in the basis
   real3 basis[4] = { {0.25, 0.25, 0.25},
      {0.25, 0.75, 0.75},
      {0.75, 0.25, 0.75},
      {0.75, 0.75, 0.25} };

   // create and place atoms
   int begin[3];
   int end[3];
   for (int ii=0; ii<3; ++ii)
   {
      begin[ii] = floor(localMin[ii]/lat);
      end[ii]   = ceil (localMax[ii]/lat);
   }

   real_t px,py,pz;
   px=py=pz=0.0;
   for (int ix=begin[0]; ix<end[0]; ++ix)
      for (int iy=begin[1]; iy<end[1]; ++iy)
         for (int iz=begin[2]; iz<end[2]; ++iz)
            for (int ib=0; ib<nb; ++ib)
            {
               real_t rx = (ix+basis[ib][0]) * lat;
               real_t ry = (iy+basis[ib][1]) * lat;
               real_t rz = (iz+basis[ib][2]) * lat;
               if (rx < localMin[0] || rx >= localMax[0]) continue;
               if (ry < localMin[1] || ry >= localMax[1]) continue;
               if (rz < localMin[2] || rz >= localMax[2]) continue;
               int id = ib+nb*(iz+nz*(iy+ny*(ix)));
               putAtomInBox(s->boxes, s->atoms, id, 0, rx, ry, rz, px, py, pz);
            }

   // TODO
   // set total atoms in simulation
   //startTimer(commReduceTimer);
   //addIntParallel(&s->atoms->nLocal, &s->atoms->nGlobal, 1);
   //stopTimer(commReduceTimer);
    s->atoms->nGlobal = nb*nx*ny*nz;

   ASSERT(s->atoms->nGlobal == nb*nx*ny*nz);
}

void adjustVcm(SimFlat* s, real_t *oldVcm, real_t *newVcm )
{
   real_t vShift[3];
   vShift[0] = (newVcm[0] - oldVcm[0]);
   vShift[1] = (newVcm[1] - oldVcm[1]);
   vShift[2] = (newVcm[2] - oldVcm[2]);

   for (int iBox=0; iBox<s->boxes->nLocalBoxes; ++iBox)
   {
      for (int iOff=MAXATOMS*iBox, ii=0; ii<s->boxes->nAtoms[iBox]; ++ii, ++iOff)
      {
         int iSpecies = s->atoms->iSpecies[iOff];
         real_t mass = s->species[iSpecies].mass;

         s->atoms->p[iOff][0] += mass * vShift[0];
         s->atoms->p[iOff][1] += mass * vShift[1];
         s->atoms->p[iOff][2] += mass * vShift[2];
      }
   }

}

ocrGuid_t adjustVcmAndComputeKeEdt(EDT_ARGS)
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    ocrGuid_t DBK_rankH = depv[0].guid;
    ocrGuid_t DBK_sim = depv[1].guid;
    ocrGuid_t DBK_nAtoms = depv[2].guid;
    ocrGuid_t DBK_iSpecies = depv[3].guid;
    ocrGuid_t DBK_p = depv[4].guid;
    ocrGuid_t rpKeDBK = depv[5].guid;
    ocrGuid_t DBK_VcmGlobal = depv[6].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    SimFlat* sim = depv[1].ptr;
    int* nAtoms = depv[2].ptr;
    int* iSpecies = depv[3].ptr;
    real3* p = depv[4].ptr;
    reductionPrivate_t* rpKePTR = depv[5].ptr;
    real_t* vcmSum = depv[6].ptr;

    real_t totalMass = vcmSum[3];

    real_t oldVcm[3];
    oldVcm[0] = vcmSum[0]/totalMass;
    oldVcm[1] = vcmSum[1]/totalMass;
    oldVcm[2] = vcmSum[2]/totalMass;

    PTR_rankH->rpKePTR = rpKePTR;

    sim->PTR_rankH = PTR_rankH;

    sim->atoms = &sim->atoms_INST;
    sim->atoms->iSpecies = iSpecies;
    sim->atoms->p = p;

    sim->boxes = &sim->boxes_INST;
    sim->boxes->nAtoms = nAtoms;

    sim->species = &sim->species_INST;

    real_t vZero[3] = {0., 0., 0.};
    real_t* newVcm = vZero;

    adjustVcm(sim, oldVcm, newVcm );

    kineticEnergy(sim);

    return NULL_GUID;
}

/// Computes the center of mass velocity of the system.
void computeVcmOCR(SimFlat* s, real_t vcmLocal[4])
{
   vcmLocal[0] = 0.;
   vcmLocal[1] = 0.;
   vcmLocal[2] = 0.;
   vcmLocal[3] = 0.;

   // sum the momenta and particle masses
   for (int iBox=0; iBox<s->boxes->nLocalBoxes; ++iBox)
   {
      for (int iOff=MAXATOMS*iBox, ii=0; ii<s->boxes->nAtoms[iBox]; ++ii, ++iOff)
      {
         vcmLocal[0] += s->atoms->p[iOff][0];
         vcmLocal[1] += s->atoms->p[iOff][1];
         vcmLocal[2] += s->atoms->p[iOff][2];

         int iSpecies = s->atoms->iSpecies[iOff];
         vcmLocal[3] += s->species[iSpecies].mass;
      }
   }

   reductionLaunch(s->PTR_rankH->rpVcmPTR, s->PTR_rankH->rpVcmDBK, vcmLocal);
}

void setMomentumAndComputeVcm(SimFlat* s, real_t temperature)
{
    DEBUG_PRINTF(( "%s\n", __func__ ));
   // set initial velocities for the distribution
   for (int iBox=0; iBox<s->boxes->nLocalBoxes; ++iBox)
   {
      for (int iOff=MAXATOMS*iBox, ii=0; ii<s->boxes->nAtoms[iBox]; ++ii, ++iOff)
      {
         int iType = s->atoms->iSpecies[iOff];
         real_t mass = s->species[iType].mass;
         real_t sigma = sqrt(kB_eV * temperature/mass);
         uint64_t seed = mkSeed(s->atoms->gid[iOff], 123);
         s->atoms->p[iOff][0] = mass * sigma * gasdev(&seed);
         s->atoms->p[iOff][1] = mass * sigma * gasdev(&seed);
         s->atoms->p[iOff][2] = mass * sigma * gasdev(&seed);
      }
   }

   real_t currentVcm[4];
   computeVcmOCR(s, currentVcm);
}

/// Add a random displacement to the atom positions.
/// Atoms are displaced by a random distance in the range
/// [-delta, +delta] along each axis.
/// \param [in] delta The maximum displacement (along each axis).
void randomDisplacements(SimFlat* s, real_t delta)
{
    DEBUG_PRINTF(( "%s\n", __func__ ));
   for (int iBox=0; iBox<s->boxes->nLocalBoxes; ++iBox)
   {
      for (int iOff=MAXATOMS*iBox, ii=0; ii<s->boxes->nAtoms[iBox]; ++ii, ++iOff)
      {
         uint64_t seed = mkSeed(s->atoms->gid[iOff], 457);
         s->atoms->r[iOff][0] += (2.0*lcg61(&seed)-1.0) * delta;
         s->atoms->r[iOff][1] += (2.0*lcg61(&seed)-1.0) * delta;
         s->atoms->r[iOff][2] += (2.0*lcg61(&seed)-1.0) * delta;
      }
   }
}

/// Sets the temperature of system.
///
/// Selects atom velocities randomly from a boltzmann (equilibrium)
/// distribution that corresponds to the specified temperature.  This
/// random process will typically result in a small, but non zero center
/// of mass velocity and a small difference from the specified
/// temperature.  For typical MD runs these small differences are
/// unimportant, However, to avoid possible confusion, we set the center
/// of mass velocity to zero and scale the velocities to exactly match
/// the input temperature.

void adjustTemperature(SimFlat* s, real_t temperature )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    real_t eKinetic = s->eKinetic;

    int nGlobal = s->atoms->nGlobal;

    real_t temp = (eKinetic/nGlobal)/kB_eV/1.5;
    // scale the velocities to achieve the target temperature
    real_t scaleFactor = sqrt(temperature/temp);

    for (int iBox=0; iBox<s->boxes->nLocalBoxes; ++iBox)
    {
        for (int iOff=MAXATOMS*iBox, ii=0; ii<s->boxes->nAtoms[iBox]; ++ii, ++iOff)

        {
           s->atoms->p[iOff][0] *= scaleFactor;
           s->atoms->p[iOff][1] *= scaleFactor;
           s->atoms->p[iOff][2] *= scaleFactor;
        }
    }
}

ocrGuid_t adjustTemperatureEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_rankH = depv[0].guid;
    ocrGuid_t DBK_sim = depv[1].guid;
    ocrGuid_t DBK_nAtoms = depv[2].guid;
    ocrGuid_t DBK_p = depv[3].guid;
    ocrGuid_t eGlobalDBK = depv[4].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    SimFlat* sim = depv[1].ptr;
    int* nAtoms = depv[2].ptr;
    real3* p = depv[3].ptr;
    real_t* eGlobalPTR = depv[4].ptr;

    adjustTemperatureEdtParamv_t* adjustTemperatureEdtParamvPTR = (adjustTemperatureEdtParamv_t*) paramv;
    real_t temperature = adjustTemperatureEdtParamvPTR->temperature;

    DEBUG_PRINTF(("ePot %f eKin %f atoms %d\n", eGlobalPTR[0], eGlobalPTR[1], (int)eGlobalPTR[2]));

    sim->PTR_rankH = PTR_rankH;

    sim->atoms = &sim->atoms_INST;
    sim->atoms->p = p;

    sim->boxes = &sim->boxes_INST;
    sim->boxes->nAtoms = nAtoms;

    sim->species = &sim->species_INST;

    sim->eKinetic = eGlobalPTR[1];

    sim->atoms->nGlobal = (int) eGlobalPTR[2];

    adjustTemperature(sim, temperature);

    return NULL_GUID;
}

ocrGuid_t randomDisplacementsEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_rankH = depv[0].guid;
    ocrGuid_t DBK_sim = depv[1].guid;
    ocrGuid_t DBK_nAtoms = depv[2].guid;
    ocrDBK_t DBK_gid = depv[3].guid;
    ocrGuid_t DBK_r = depv[4].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    SimFlat* sim = depv[1].ptr;
    int* nAtoms = depv[2].ptr;
    int* gid = depv[3].ptr;
    real3* r = depv[4].ptr;

    randomDisplacementsEdtParamv_t* randomDisplacementsEdtParamvPTR = (randomDisplacementsEdtParamv_t*) paramv;
    real_t delta = randomDisplacementsEdtParamvPTR->delta;

    sim->PTR_rankH = PTR_rankH;

    sim->atoms = &sim->atoms_INST;
    sim->atoms->gid = gid;
    sim->atoms->r = r;

    sim->boxes = &sim->boxes_INST;
    sim->boxes->nAtoms = nAtoms;

    sim->species = &sim->species_INST;

    randomDisplacements(sim, delta);

    return NULL_GUID;
}

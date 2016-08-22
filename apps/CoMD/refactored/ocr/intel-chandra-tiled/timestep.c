/// \file
/// Leapfrog time integrator

#include "timestep.h"

#include "CoMDTypes.h"
#include "linkCells.h"
#include "parallel.h"
#include "performanceTimers.h"
#include "constants.h"

static void advanceVelocity(SimFlat* s, int nBoxes, real_t dt);
static void advancePosition(SimFlat* s, int nBoxes, real_t dt);


/// Advance the simulation time to t+dt using a leap frog method
/// (equivalent to velocity verlet).
///
/// Forces must be computed before calling the integrator the first time.
///
///  - Advance velocities half time step using forces
///  - Advance positions full time step using velocities
///  - Update link cells and exchange remote particles
///  - Compute forces
///  - Update velocities half time step using forces
///
/// This leaves positions, velocities, and forces at t+dt, with the
/// forces ready to perform the half step velocity update at the top of
/// the next call.
///
/// After nSteps the kinetic energy is computed for diagnostic output.
double timestep(SimFlat* s, int nSteps, real_t dt)
{
   for (int ii=0; ii<nSteps; ++ii)
   {
      //startTimer(velocityTimer);
      advanceVelocity(s, s->boxes->nLocalBoxes, 0.5*dt);
      //stopTimer(velocityTimer);

      //startTimer(positionTimer);
      advancePosition(s, s->boxes->nLocalBoxes, dt);
      //stopTimer(positionTimer);

      //startTimer(redistributeTimer);
      redistributeAtoms(s);
      //stopTimer(redistributeTimer);

      //startTimer(computeForceTimer);
      computeForce(s);
      //stopTimer(computeForceTimer);

      //startTimer(velocityTimer);
      advanceVelocity(s, s->boxes->nLocalBoxes, 0.5*dt);
      //stopTimer(velocityTimer);
   }

   kineticEnergy(s);

   return s->ePotential;
}

ocrGuid_t advanceVelocityEdt(EDT_ARGS)
{

    real_t dt = ((real_t*) paramv)[0];
    DEBUG_PRINTF(( "%s dt %f\n", __func__, dt ));

    ocrGuid_t DBK_sim = depv[0].guid;
    ocrGuid_t DBK_nAtoms = depv[1].guid;
    ocrGuid_t DBK_p = depv[2].guid;
    ocrGuid_t DBK_f = depv[3].guid;

    SimFlat* sim = depv[0].ptr;
    int* nAtoms = depv[1].ptr;
    real3* p = depv[2].ptr;
    real3* f = depv[3].ptr;

    sim->atoms = &sim->atoms_INST;
    sim->atoms->iSpecies = NULL;
    sim->atoms->p = p;
    sim->atoms->f = f;

    sim->boxes = &sim->boxes_INST;
    sim->boxes->nAtoms = nAtoms;

    sim->species = &sim->species_INST;

    advanceVelocity(sim, sim->boxes->nLocalBoxes, dt);

    return NULL_GUID;
}

ocrGuid_t advancePositionEdt(EDT_ARGS)
{
    real_t dt = ((real_t*) paramv)[0];
    DEBUG_PRINTF(( "%s dt %f\n", __func__, dt ));

    ocrGuid_t DBK_sim = depv[0].guid;
    ocrGuid_t DBK_nAtoms = depv[1].guid;
    ocrGuid_t DBK_iSpecies = depv[2].guid;
    ocrGuid_t DBK_r = depv[3].guid;
    ocrGuid_t DBK_p = depv[4].guid;

    SimFlat* sim = depv[0].ptr;
    int* nAtoms = depv[1].ptr;
    int* iSpecies = depv[2].ptr;
    real3* r = depv[3].ptr;
    real3* p = depv[4].ptr;

    sim->atoms = &sim->atoms_INST;
    sim->atoms->iSpecies = iSpecies;
    sim->atoms->r = r;
    sim->atoms->p = p;
    sim->atoms->f = NULL;

    sim->boxes = &sim->boxes_INST;
    sim->boxes->nAtoms = nAtoms;

    sim->species = &sim->species_INST;

    advancePosition(sim, sim->boxes->nLocalBoxes, dt);

    return NULL_GUID;
}


ocrGuid_t timestepEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    s64 itimestep = paramv[0];

#if 1
    s32 _idep;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_sim = depv[_idep++].guid;
    ocrDBK_t rpKeDBK = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    SimFlat* sim = depv[_idep++].ptr;
    reductionPrivate_t* rpKePTR = depv[_idep++].ptr;

    //rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    sim->atoms = &sim->atoms_INST;
    sim->boxes = &sim->boxes_INST;
    sim->atomExchange = &sim->atomExchange_INST;

    sim->PTR_rankH = PTR_rankH;

    ocrHint_t myEdtAffinityHNT = sim->PTR_rankH->myEdtAffinityHNT;

    ocrDBK_t DBK_pot = sim->DBK_pot;

    ocrDBK_t DBK_nAtoms = sim->boxes->DBK_nAtoms;

    ocrDBK_t DBK_gid = sim->atoms->DBK_gid;
    ocrDBK_t DBK_iSpecies = sim->atoms->DBK_iSpecies;
    ocrDBK_t DBK_r = sim->atoms->DBK_r;
    ocrDBK_t DBK_p = sim->atoms->DBK_p;
    ocrDBK_t DBK_f = sim->atoms->DBK_f;
    ocrDBK_t DBK_U = sim->atoms->DBK_U;

    u32 myRank = sim->PTR_rankH->myRank;
    //PRINTF("iBox %d t %d\n", iBox, itimestep);

    rpKePTR->all = 0;

    ocrGuid_t rpKeEVT = rpKePTR->returnEVT;

    ocrDbRelease(rpKeDBK);

    ocrGuid_t advanceVelocityTML, advanceVelocityEDT, advanceVelocityOEVT, advanceVelocityOEVTS;

    real_t dt = 0.5*PTR_cmd->dt; //TODO

    ocrEdtTemplateCreate( &advanceVelocityTML, advanceVelocityEdt, 1, 4 );

    ocrEdtCreate( &advanceVelocityEDT, advanceVelocityTML,
                  EDT_PARAM_DEF, (u64*) &dt, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &advanceVelocityOEVT );

    ocrEventCreate( &advanceVelocityOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( advanceVelocityOEVT, advanceVelocityOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_sim, advanceVelocityEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_nAtoms, advanceVelocityEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_p, advanceVelocityEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_f, advanceVelocityEDT, _idep++, DB_MODE_RO );

    ocrGuid_t advancePositionTML, advancePositionEDT, advancePositionOEVT, advancePositionOEVTS;

    dt = 0.0;//1.0*PTR_cmd->dt; //TODO

    ocrEdtTemplateCreate( &advancePositionTML, advancePositionEdt, 1, 6 );

    ocrEdtCreate( &advancePositionEDT, advancePositionTML,
                  EDT_PARAM_DEF, (u64*) &dt, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &advancePositionOEVT );

    ocrEventCreate( &advancePositionOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( advancePositionOEVT, advancePositionOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_sim, advancePositionEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_nAtoms, advancePositionEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_iSpecies, advancePositionEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_r, advancePositionEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_p, advancePositionEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( advanceVelocityOEVTS, advancePositionEDT, _idep++, DB_MODE_NULL );

    //redistribute atoms
    ocrGuid_t redistributeAtomsTML, redistributeAtomsEDT, redistributeAtomsOEVT, redistributeAtomsOEVTS;

    ocrEdtTemplateCreate( &redistributeAtomsTML, redistributeAtomsEdt, 0, 3 );

    ocrEdtCreate( &redistributeAtomsEDT, redistributeAtomsTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &myEdtAffinityHNT, &redistributeAtomsOEVT );

    ocrEventCreate( &redistributeAtomsOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( redistributeAtomsOEVT, redistributeAtomsOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, redistributeAtomsEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_sim, redistributeAtomsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( advancePositionOEVTS, redistributeAtomsEDT, _idep++, DB_MODE_NULL );

    //compute force
    ocrGuid_t computeForceTML, computeForceEDT, computeForceOEVT, computeForceOEVTS;

    ocrEdtTemplateCreate( &computeForceTML, computeForceEdt, 0, 4 );

    ocrEdtCreate( &computeForceEDT, computeForceTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &myEdtAffinityHNT, &computeForceOEVT );

    ocrEventCreate( &computeForceOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( computeForceOEVT, computeForceOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, computeForceEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_sim, computeForceEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_pot, computeForceEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( redistributeAtomsOEVTS, computeForceEDT, _idep++, DB_MODE_NULL );

    ocrGuid_t advanceVelocityOEVT1, advanceVelocityOEVTS1;

    dt = 0.0; //0.5*PTR_cmd->dt; //TODO

    ocrEdtTemplateCreate( &advanceVelocityTML, advanceVelocityEdt, 1, 5 );

    ocrEdtCreate( &advanceVelocityEDT, advanceVelocityTML,
                  EDT_PARAM_DEF, (u64*) &dt, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &advanceVelocityOEVT1 );

    ocrEventCreate( &advanceVelocityOEVTS1, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( advanceVelocityOEVT1, advanceVelocityOEVTS1, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_sim, advanceVelocityEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_nAtoms, advanceVelocityEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_p, advanceVelocityEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_f, advanceVelocityEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( computeForceOEVTS, advanceVelocityEDT, _idep++, DB_MODE_NULL );

    s32 printRate = PTR_cmd->printRate;
    s32 nSteps = PTR_cmd->nSteps;

    itimestep++;

    if( itimestep%printRate == 0 )
    {
        ////Compute Kinetic energy of the system
        ocrGuid_t kineticEnergyTML, kineticEnergyEDT, kineticEnergyOEVT, kineticEnergyOEVTS;

        ocrEdtTemplateCreate( &kineticEnergyTML, kineticEnergyEdt, 0, 7 );

        ocrEdtCreate( &kineticEnergyEDT, kineticEnergyTML,
                      EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &myEdtAffinityHNT, &kineticEnergyOEVT );

        ocrEventCreate( &kineticEnergyOEVTS, OCR_EVENT_STICKY_T, false );
        ocrAddDependence( kineticEnergyOEVT, kineticEnergyOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, kineticEnergyEDT, _idep++, DB_MODE_RW ); //TODO
        ocrAddDependence( DBK_sim, kineticEnergyEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_nAtoms, kineticEnergyEDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_iSpecies, kineticEnergyEDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_p, kineticEnergyEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( rpKeDBK, kineticEnergyEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( computeForceOEVTS, kineticEnergyEDT, _idep++, DB_MODE_NULL );

        ////Print things
        if(myRank == 0)
        {
            ocrGuid_t printThingsTML, printThingsEDT, printThingsOEVT, printThingsOEVTS;

            ocrEdtTemplateCreate( &printThingsTML, printThingsEdt, 1, 3 );

            ocrEdtCreate( &printThingsEDT, printThingsTML,
                          EDT_PARAM_DEF, (u64*) &itimestep, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, &myEdtAffinityHNT, &printThingsOEVT );

            ocrEventCreate( &printThingsOEVTS, OCR_EVENT_STICKY_T, false );
            ocrAddDependence( printThingsOEVT, printThingsOEVTS, 0, DB_MODE_NULL );

            _idep = 0;
            ocrAddDependence( DBK_sim, printThingsEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( rpKeEVT, printThingsEDT, _idep++, DB_MODE_RO );
            ocrAddDependence( kineticEnergyOEVTS, printThingsEDT, _idep++, DB_MODE_RW );
        }

    }

#endif

    return NULL_GUID;
}

ocrGuid_t printThingsEdt( EDT_ARGS )
{
    s64 itimestep = paramv[0];

    real_t* eGlobalPTR = depv[1].ptr;

    int nGlobal = (int) eGlobalPTR[2];
    real_t e_u = eGlobalPTR[0]/nGlobal;
    real_t e_k = eGlobalPTR[1]/nGlobal;

    real_t energy = (e_u + e_k);
    real_t temp = e_k / (kB_eV * 1.5);

    if( itimestep == 0 )
    {
        PRINTF( "\nInitial energy : %f, atom count : %u\n\n", energy, nGlobal );
        //timestamp( "Starting simulation\n");
        PRINTF( "Starting simulation\n");
        PRINTF( "#                                                                                         Performance\n");
        PRINTF( "#  Loop   Time(fs)       Total Energy   Potential Energy     Kinetic Energy  Temperature   (us/atom)     # Atoms\n");
    }

    PRINTF( "%7d %10.2f %18.12f %18.12f %18.12f %12.4f %10.4f %12d\n",
         itimestep, itimestep*1.0, energy, e_u, e_k, temp, -1.0, nGlobal );

    return NULL_GUID;
}

_OCR_TASK_FNC_( timestepLoopEdt )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));
    s64 itimestep = paramv[0];

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrGuid_t DBK_sim = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    SimFlat* sim = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    s64 id = PTR_rankH->myRank;

    ocrDBK_t rpKeDBK = PTR_rankH->rpKeDBK;

    // Do one timestep
    ocrGuid_t timestepTML, timestepEDT, timestepOEVT, timestepOEVTS;

    ocrEdtTemplateCreate( &timestepTML, timestepEdt, 1, 3 );

    ocrEdtCreate( &timestepEDT, timestepTML,
                  EDT_PARAM_DEF, (u64*) &itimestep, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &timestepOEVT );

    ocrEventCreate( &timestepOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( timestepOEVT, timestepOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, timestepEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_sim, timestepEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( rpKeDBK, timestepEDT, _idep++, DB_MODE_RW );

    itimestep += 1;

    s32 nSteps = PTR_cmd->nSteps;

    if( itimestep < nSteps )
    {
    //start next timestep
        ocrGuid_t timestepLoopTML, timestepLoopEDT, timestepLoopOEVT, timestepLoopOEVTS;

        ocrEdtTemplateCreate( &timestepLoopTML, timestepLoopEdt, 1, 4 );

        ocrEdtCreate( &timestepLoopEDT, timestepLoopTML,
                      EDT_PARAM_DEF, (u64*)&itimestep, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, &timestepLoopOEVT );

        ocrEventCreate( &timestepLoopOEVTS, OCR_EVENT_STICKY_T, false );
        ocrAddDependence( timestepLoopOEVT, timestepLoopOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, timestepLoopEDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_sim, timestepLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( rpKeDBK, timestepLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( timestepOEVTS, timestepLoopEDT, _idep++, DB_MODE_NULL );
    }
    else //Trigger wrapup EVT
        //rpKePTR->returnEVT = PTR_globalOcrParamH->finalOnceEVT;
        ocrAddDependence(timestepOEVTS, PTR_globalOcrParamH->finalOnceEVT, 0, DB_MODE_NULL);

    return NULL_GUID;
}

void computeForce(SimFlat* s)
{
//   s->pot->force(s);
}

ocrGuid_t computeForceEdt( EDT_ARGS )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    s32 _idep;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_sim = depv[_idep++].guid;
    ocrDBK_t DBK_pot = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    SimFlat* sim = depv[_idep++].ptr;
    BasePotential* pot = depv[_idep++].ptr;

    sim->atoms = &sim->atoms_INST;
    sim->boxes = &sim->boxes_INST;
    sim->species = &sim->species_INST;
    sim->atomExchange = &sim->atomExchange_INST;

    sim->PTR_rankH = PTR_rankH;
    sim->pot = pot;

    ocrHint_t myEdtAffinityHNT = sim->PTR_rankH->myEdtAffinityHNT;

    ocrDBK_t DBK_nAtoms = sim->boxes->DBK_nAtoms;

    ocrDBK_t DBK_gid = sim->atoms->DBK_gid;
    ocrDBK_t DBK_iSpecies = sim->atoms->DBK_iSpecies;
    ocrDBK_t DBK_r = sim->atoms->DBK_r;
    ocrDBK_t DBK_p = sim->atoms->DBK_p;
    ocrDBK_t DBK_f = sim->atoms->DBK_f;
    ocrDBK_t DBK_U = sim->atoms->DBK_U;

    ocrDBK_t DBK_parms = sim->atomExchange->DBK_parms;

    ocrGuid_t forceTML, forceEDT, forceOEVT, forceOEVTS;

    ocrEdtTemplateCreate( &forceTML, sim->pot->force_edt, 0, 10 );

    ocrEdtCreate( &forceEDT, forceTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &forceOEVT );

    ocrEventCreate( &forceOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( forceOEVT, forceOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, forceEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_sim, forceEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_nAtoms, forceEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_gid, forceEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_iSpecies, forceEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_r, forceEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_p, forceEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_f, forceEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_U, forceEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_pot, forceEDT, _idep++, DB_MODE_RO );

    return NULL_GUID;
}


void advanceVelocity(SimFlat* s, int nBoxes, real_t dt)
{
   for (int iBox=0; iBox<nBoxes; iBox++)
   {
      for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++)
      {
         s->atoms->p[iOff][0] += dt*s->atoms->f[iOff][0];
         s->atoms->p[iOff][1] += dt*s->atoms->f[iOff][1];
         s->atoms->p[iOff][2] += dt*s->atoms->f[iOff][2];

         //if(dt !=0.0 )
         //PRINTF("iBox %d iOff %d: %f %f %f\n", iBox, iOff, s->atoms->f[iOff][0],s->atoms->f[iOff][1], s->atoms->f[iOff][2]);
      }
   }
}

void advancePosition(SimFlat* s, int nBoxes, real_t dt)
{
   for (int iBox=0; iBox<nBoxes; iBox++)
   {
      for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++)
      {
         int iSpecies = s->atoms->iSpecies[iOff];
         real_t invMass = 1.0/s->species[iSpecies].mass;
         s->atoms->r[iOff][0] += dt*s->atoms->p[iOff][0]*invMass;
         s->atoms->r[iOff][1] += dt*s->atoms->p[iOff][1]*invMass;
         s->atoms->r[iOff][2] += dt*s->atoms->p[iOff][2]*invMass;
      }
   }
}

/// Calculates total kinetic and potential energy across all tasks.  The
/// local potential energy is a by-product of the force routine.
void kineticEnergy(SimFlat* s)
{
    //DEBUG_PRINTF(( "%s\n", __func__ ));

   real_t eLocal[3];
   eLocal[0] = s->ePotential;
   eLocal[1] = 0.0;
   eLocal[2] = 0.0;
   int nAtoms = 0;
   for (int iBox=0; iBox<s->boxes->nLocalBoxes; iBox++)
   {
      for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++)
      {
         int iSpecies = s->atoms->iSpecies[iOff];
         real_t invMass = 0.5/s->species[iSpecies].mass;
         eLocal[1] += ( s->atoms->p[iOff][0] * s->atoms->p[iOff][0] +
         s->atoms->p[iOff][1] * s->atoms->p[iOff][1] +
         s->atoms->p[iOff][2] * s->atoms->p[iOff][2] )*invMass;
      }
    eLocal[2] += s->boxes->nAtoms[iBox];
   }
    DEBUG_PRINTF(("%f %f eLocal[2] %f\n", eLocal[0], eLocal[1], eLocal[2]));

   real_t eSum[2];
   //startTimer(commReduceTimer);
   //addRealParallel(eLocal, eSum, 2);
   reductionLaunch(s->PTR_rankH->rpKePTR, s->PTR_rankH->rpKeDBK, eLocal);
   //stopTimer(commReduceTimer);

   //s->ePotential = eSum[0];
   //s->eKinetic = eSum[1];
}

ocrGuid_t kineticEnergyEdt(EDT_ARGS)
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    ocrGuid_t DBK_rankH = depv[0].guid;
    ocrGuid_t DBK_sim = depv[1].guid;
    ocrGuid_t DBK_nAtoms = depv[2].guid;
    ocrGuid_t DBK_iSpecies = depv[3].guid;
    ocrGuid_t DBK_p = depv[4].guid;
    ocrGuid_t rpKeDBK = depv[5].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    SimFlat* sim = depv[1].ptr;
    int* nAtoms = depv[2].ptr;
    int* iSpecies = depv[3].ptr;
    real3* p = depv[4].ptr;
    reductionPrivate_t* rpKePTR = depv[5].ptr;

    PTR_rankH->rpKePTR = rpKePTR;

    sim->PTR_rankH = PTR_rankH;

    sim->atoms = &sim->atoms_INST;
    sim->boxes = &sim->boxes_INST;
    sim->species = &sim->species_INST;

    sim->atoms->iSpecies = iSpecies;
    sim->atoms->p = p;

    sim->boxes->nAtoms = nAtoms;

    real_t vZero[3] = {0., 0., 0.};
    real_t* newVcm = vZero;

    kineticEnergy(sim);

    return NULL_GUID;
}

/// \details
/// This function provides one-stop shopping for the sequence of events
/// that must occur for a proper exchange of halo atoms after the atom
/// positions have been updated by the integrator.
///
/// - updateLinkCells: Since atoms have moved, some may be in the wrong
///   link cells.
/// - haloExchange (atom version): Sends atom data to remote tasks.
/// - sort: Sort the atoms.
///
/// \see updateLinkCells
/// \see initAtomHaloExchange
/// \see sortAtomsInCell
void redistributeAtoms(SimFlat* sim)
{
   updateLinkCells(sim->boxes, sim->atoms);

   //startTimer(atomHaloTimer);
   //haloExchange(sim->atomExchange, sim);
   //stopTimer(atomHaloTimer);

   for (int ii=0; ii<sim->boxes->nTotalBoxes; ++ii)
      ;
      //sortAtomsInCell(sim->atoms, sim->boxes, ii);
}

ocrGuid_t redistributeAtomsEdt( EDT_ARGS )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    s32 _idep;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_sim = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    SimFlat* sim = depv[_idep++].ptr;

    sim->atoms = &sim->atoms_INST;
    sim->boxes = &sim->boxes_INST;
    sim->atomExchange = &sim->atomExchange_INST;

    sim->PTR_rankH = PTR_rankH;

    ocrHint_t myEdtAffinityHNT = sim->PTR_rankH->myEdtAffinityHNT;

    ocrDBK_t DBK_pot = sim->DBK_pot;

    ocrDBK_t DBK_nAtoms = sim->boxes->DBK_nAtoms;

    ocrDBK_t DBK_gid = sim->atoms->DBK_gid;
    ocrDBK_t DBK_iSpecies = sim->atoms->DBK_iSpecies;
    ocrDBK_t DBK_r = sim->atoms->DBK_r;
    ocrDBK_t DBK_p = sim->atoms->DBK_p;
    ocrDBK_t DBK_f = sim->atoms->DBK_f;
    ocrDBK_t DBK_U = sim->atoms->DBK_U;

    ocrDBK_t DBK_parms = sim->atomExchange->DBK_parms;

    ocrGuid_t updateLinkCellsTML, updateLinkCellsEDT, updateLinkCellsOEVT, updateLinkCellsOEVTS;

    ocrEdtTemplateCreate( &updateLinkCellsTML, updateLinkCellsEdt, 0, 9 );

    ocrEdtCreate( &updateLinkCellsEDT, updateLinkCellsTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &updateLinkCellsOEVT );

    ocrEventCreate( &updateLinkCellsOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( updateLinkCellsOEVT, updateLinkCellsOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, updateLinkCellsEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_sim, updateLinkCellsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_nAtoms, updateLinkCellsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_gid, updateLinkCellsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_iSpecies, updateLinkCellsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_r, updateLinkCellsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_p, updateLinkCellsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_f, updateLinkCellsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_U, updateLinkCellsEDT, _idep++, DB_MODE_RW );

    ocrGuid_t haloExchangeTML, haloExchangeEDT, haloExchangeOEVT, haloExchangeOEVTS;

    ocrEdtTemplateCreate( &haloExchangeTML, haloExchangeEdt, 0, 4 );

    ocrEdtCreate( &haloExchangeEDT, haloExchangeTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &myEdtAffinityHNT, &haloExchangeOEVT );

    ocrEventCreate( &haloExchangeOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( haloExchangeOEVT, haloExchangeOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, haloExchangeEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_sim, haloExchangeEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_parms, haloExchangeEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( updateLinkCellsOEVTS, haloExchangeEDT, _idep++, DB_MODE_RW );

    ocrGuid_t sortAtomsInCellsTML, sortAtomsInCellsEDT, sortAtomsInCellsOEVT, sortAtomsInCellsOEVTS;

    ocrEdtTemplateCreate( &sortAtomsInCellsTML, sortAtomsInCellsEdt, 0, 8 );

    ocrEdtCreate( &sortAtomsInCellsEDT, sortAtomsInCellsTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &sortAtomsInCellsOEVT );

    ocrEventCreate( &sortAtomsInCellsOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( sortAtomsInCellsOEVT, sortAtomsInCellsOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, sortAtomsInCellsEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_sim, sortAtomsInCellsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_nAtoms, sortAtomsInCellsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_gid, sortAtomsInCellsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_iSpecies, sortAtomsInCellsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_r, sortAtomsInCellsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_p, sortAtomsInCellsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( haloExchangeOEVTS, sortAtomsInCellsEDT, _idep++, DB_MODE_RW );

    return NULL_GUID;
}

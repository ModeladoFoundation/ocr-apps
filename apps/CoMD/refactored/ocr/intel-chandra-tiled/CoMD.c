/*
Author Chandra S. Martha
Copywrite Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <assert.h>

#include "CoMDTypes.h"
#include "decomposition.h"
#include "linkCells.h"
#include "eam.h"
#include "ljForce.h"
#include "initAtoms.h"
#include "memUtils.h"
#include "yamlOutput.h"
#include "parallel.h"
//#include "performanceTimers.h"
#include "mycommand.h"
#include "timestep.h"
#include "constants.h"

#include "SPMDappUtils.h"

void initSpecies(SpeciesData* species, BasePotential* pot)
{
   //SpeciesData* species = comdMalloc(sizeof(SpeciesData));

   strcpy(species->name, pot->name);
   species->atomicNo = pot->atomicNo;
   species->mass = pot->mass;

}

/// Check that the user input meets certain criteria.
void sanityChecks(u64 id, Command cmd, double cutoff, double latticeConst, char latticeType[8])
{
    DEBUG_PRINTF(( "%s\n", __func__ ));
   int failCode = 0;

   // Check that domain grid matches number of ranks. (fail code 1)
   int nProcs = cmd.xproc * cmd.yproc * cmd.zproc;
   //if (nProcs != getNRanks())
   //{
   //   failCode |= 1;
   //   if (printRank() )
   //      fprintf(screenOut,
   //              "\nNumber of MPI ranks must match xproc * yproc * zproc\n");
   //}

   // Check whether simuation is too small (fail code 2)
   double minx = 2*cutoff*cmd.xproc;
   double miny = 2*cutoff*cmd.yproc;
   double minz = 2*cutoff*cmd.zproc;
   double sizex = cmd.nx*latticeConst;
   double sizey = cmd.ny*latticeConst;
   double sizez = cmd.nz*latticeConst;

   if ( sizex < minx || sizey < miny || sizez < minz)
   {
      failCode |= 2;
      if (id==0)
         fprintf(screenOut,"\nSimulation too small.\n"
                 "  Increase the number of unit cells to make the simulation\n"
                 "  at least (%3.2f, %3.2f. %3.2f) Ansgstroms in size\n",
                 minx, miny, minz);
   }

   // Check for supported lattice structure (fail code 4)
   if (strcasecmp(latticeType, "FCC") != 0)
   {
      failCode |= 4;
      if (id==0)
         fprintf(screenOut,
                 "\nOnly FCC Lattice type supported, not %s. Fatal Error.\n",
                 latticeType);
   }
   int checkCode = failCode;
   //bcastParallel(&checkCode, sizeof(int), 0);
   // This assertion can only fail if different tasks failed different
   // sanity checks.  That should not be possible.
   assert(checkCode == failCode);

   if (failCode != 0)
      exit(failCode);
}

/// decide whether to get LJ or EAM potentials
ocrDBK_t initPotential(
   BasePotential** pot, int doeam, const char* potDir, const char* potName, const char* potType)
{
   if (doeam)
      return initEamPot(pot, potDir, potName, potType);
   else
      return initLjPot(pot);

   //assert(pot);
}

void initSimulation(SimFlat* sim, rankH_t* PTR_rankH, u64 id)
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    Command cmd = PTR_rankH->globalParamH.cmdParamH;

   sim->nSteps = cmd.nSteps;
   sim->printRate = cmd.printRate;
   sim->dt = cmd.dt;
   sim->ePotential = 0.0;
   sim->eKinetic = 0.0;
   //sim->atomExchange = NULL;

   sim->DBK_pot = initPotential(&sim->pot, cmd.doeam, cmd.potDir, cmd.potName, cmd.potType);
   real_t latticeConstant = cmd.lat;
   if (cmd.lat < 0.0)
      latticeConstant = sim->pot->lat;

   // ensure input parameters make sense.
   sanityChecks(id, cmd, sim->pot->cutoff, latticeConstant, sim->pot->latticeType);

   sim->species = &sim->species_INST;
   initSpecies(sim->species, sim->pot);

   real3 globalExtent;
   globalExtent[0] = cmd.nx * latticeConstant;
   globalExtent[1] = cmd.ny * latticeConstant;
   globalExtent[2] = cmd.nz * latticeConstant;

   sim->domain = &sim->domain_INST;
   initDecomposition(id, sim->domain, cmd.xproc, cmd.yproc, cmd.zproc, globalExtent);

   sim->boxes = &sim->boxes_INST;
   initLinkCells(sim->boxes, sim->domain, sim->pot->cutoff);
   sim->atoms = &sim->atoms_INST;
   initAtoms(sim->atoms, sim->boxes);

   // create lattice with desired temperature and displacement.
   createFccLattice(cmd.nx, cmd.ny, cmd.nz, latticeConstant, sim);

   setMomentumAndComputeVcm(sim, cmd.temperature);

   //randomDisplacements(sim, cmd.initialDelta);

   sim->atomExchange = &sim->atomExchange_INST;
   initAtomHaloExchange(sim->atomExchange, sim->domain, sim->boxes);

   // Forces must be computed before we call the time stepper.
   //startTimer(redistributeTimer);
   //redistributeAtoms(sim);
   //stopTimer(redistributeTimer);

   //startTimer(computeForceTimer);
   //computeForce(sim);
   //stopTimer(computeForceTimer);

   //kineticEnergy(sim);

   //return sim;
}

void getAffinityHintsForDBandEdt( ocrHint_t* PTR_myDbkAffinityHNT, ocrHint_t* PTR_myEdtAffinityHNT )
{
    ocrGuid_t currentAffinity = NULL_GUID;
    ocrAffinityGetCurrent(&currentAffinity);

    ocrHintInit( PTR_myEdtAffinityHNT, OCR_HINT_EDT_T );
    ocrHintInit( PTR_myDbkAffinityHNT, OCR_HINT_DB_T );

    ocrSetHintValue( PTR_myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
    ocrSetHintValue( PTR_myDbkAffinityHNT, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(currentAffinity) );

}

void initOcrObjects( rankH_t* PTR_rankH, u64 id, u64 nRanks )
{
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    u32 _paramc, _depc, _idep;

    ocrHint_t myEdtAffinityHNT, myDbkAffinityHNT;

    getAffinityHintsForDBandEdt( &myDbkAffinityHNT, &myEdtAffinityHNT );

    PTR_rankH->myEdtAffinityHNT = myEdtAffinityHNT;
    PTR_rankH->myDbkAffinityHNT = myDbkAffinityHNT;

    ocrDbCreate( &PTR_rankH->rpKeDBK, (void**) &PTR_rankH->rpKePTR, sizeof(reductionPrivate_t), 0, NULL_HINT, NO_ALLOC );
    ocrDbCreate( &PTR_rankH->rpVcmDBK, (void**) &PTR_rankH->rpVcmPTR, sizeof(reductionPrivate_t), 0, NULL_HINT, NO_ALLOC );
    ocrDbCreate( &PTR_rankH->rpmaxOccupancyDBK, (void**) &PTR_rankH->rpmaxOccupancyPTR, sizeof(reductionPrivate_t), 0, NULL_HINT, NO_ALLOC );
    ocrDbCreate( &PTR_rankH->rpPerfTimerDBK, (void**) &PTR_rankH->rpPerfTimerPTR, sizeof(reductionPrivate_t), 0, NULL_HINT, NO_ALLOC );

    ocrEdtTemplateCreate( &PTR_rankTemplateH->advanceVelocityTML, advanceVelocityEdt, 1, 5 );
    ocrEdtTemplateCreate( &PTR_rankTemplateH->advancePositionTML, advancePositionEdt, 1, 6 );
    ocrEdtTemplateCreate( &PTR_rankTemplateH->redistributeAtomsTML, redistributeAtomsEdt, 1, 3 );
    ocrEdtTemplateCreate( &PTR_rankTemplateH->computeForceTML, computeForceEdt, 0, 4 );
    ocrEdtTemplateCreate( &PTR_rankTemplateH->kineticEnergyTML, kineticEnergyEdt, 0, 7 );
    ocrEdtTemplateCreate( &PTR_rankTemplateH->printThingsTML, printThingsEdt, 1, 5 );
    ocrEdtTemplateCreate( &PTR_rankTemplateH->timestepLoopTML, timestepLoopEdt, 1, 5 );
    ocrEdtTemplateCreate( &PTR_rankTemplateH->timestepTML, timestepEdt, 1, 4 );
    ocrEdtTemplateCreate( &PTR_rankTemplateH->updateLinkCellsTML, updateLinkCellsEdt, 0, 9 );
    ocrEdtTemplateCreate( &PTR_rankTemplateH->haloExchangeTML, haloExchangeEdt, 2, 4 );
    ocrEdtTemplateCreate( &PTR_rankTemplateH->sortAtomsInCellsTML, sortAtomsInCellsEdt, 0, 8 );
    ocrEdtTemplateCreate( &PTR_rankTemplateH->exchangeDataTML, exchangeDataEdt, 2, 3 );
    ocrEdtTemplateCreate( &PTR_rankTemplateH->loadAtomsBufferTML, loadAtomsBufferEdt, 2, 14 );
    ocrEdtTemplateCreate( &PTR_rankTemplateH->unloadAtomsBufferTML, unloadAtomsBufferEdt, 1, 15 );

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = 3;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;

    PTR_rankH->rpKePTR->nrank = nRanks;
    PTR_rankH->rpKePTR->myrank = id;
    PTR_rankH->rpKePTR->ndata = 3; //TODO
    PTR_rankH->rpKePTR->reductionOperator = REDUCTION_F8_ADD;
    PTR_rankH->rpKePTR->rangeGUID = PTR_globalOcrParamH->KeReductionRangeGUID;
    PTR_rankH->rpKePTR->reductionTML = NULL_GUID;
    PTR_rankH->rpKePTR->new = 1;  //first time
    PTR_rankH->rpKePTR->all = 1;  //go up and down (ALL_REDUCE)

    ocrEventCreateParams(&(PTR_rankH->rpKeEVT), OCR_EVENT_CHANNEL_T, true, &params);//TODO
    PTR_rankH->rpKePTR->returnEVT = PTR_rankH->rpKeEVT;

    //ocrDbRelease(PTR_rankH->rpKeDBK);

    PTR_rankH->rpVcmPTR->nrank = nRanks;
    PTR_rankH->rpVcmPTR->myrank = id;
    PTR_rankH->rpVcmPTR->ndata = 4; //TODO
    PTR_rankH->rpVcmPTR->reductionOperator = REDUCTION_F8_ADD;
    PTR_rankH->rpVcmPTR->rangeGUID = PTR_globalOcrParamH->VcmReductionRangeGUID;
    PTR_rankH->rpVcmPTR->reductionTML = NULL_GUID;
    PTR_rankH->rpVcmPTR->new = 1;  //first time
    PTR_rankH->rpVcmPTR->all = 1;  //go up and down (ALL_REDUCE)

    ocrEventCreateParams(&(PTR_rankH->rpVcmEVT), OCR_EVENT_CHANNEL_T, true, &params);//TODO
    PTR_rankH->rpVcmPTR->returnEVT = PTR_rankH->rpVcmEVT;

    //ocrDbRelease(PTR_rankH->rpVcmDBK);

    PTR_rankH->rpmaxOccupancyPTR->nrank = nRanks;
    PTR_rankH->rpmaxOccupancyPTR->myrank = id;
    PTR_rankH->rpmaxOccupancyPTR->ndata = 1; //TODO
    PTR_rankH->rpmaxOccupancyPTR->reductionOperator = REDUCTION_U4_MAX;
    PTR_rankH->rpmaxOccupancyPTR->rangeGUID = PTR_globalOcrParamH->maxOccupancyReductionRangeGUID;
    PTR_rankH->rpmaxOccupancyPTR->reductionTML = NULL_GUID;
    PTR_rankH->rpmaxOccupancyPTR->new = 1;  //first time
    PTR_rankH->rpmaxOccupancyPTR->all = 1;  //go up and down (ALL_REDUCE)

    ocrEventCreateParams(&(PTR_rankH->rpmaxOccupancyEVT), OCR_EVENT_CHANNEL_T, true, &params);//TODO
    PTR_rankH->rpmaxOccupancyPTR->returnEVT = PTR_rankH->rpmaxOccupancyEVT;

    //ocrDbRelease(PTR_rankH->rpmaxOccupancyDBK);

    PTR_rankH->rpPerfTimerPTR->nrank = nRanks;
    PTR_rankH->rpPerfTimerPTR->myrank = id;
    PTR_rankH->rpPerfTimerPTR->ndata = numberOfTimers; //TODO
    PTR_rankH->rpPerfTimerPTR->reductionOperator = REDUCTION_F8_ADD;
    PTR_rankH->rpPerfTimerPTR->rangeGUID = PTR_globalOcrParamH->perfTimerReductionRangeGUID;
    PTR_rankH->rpPerfTimerPTR->reductionTML = NULL_GUID;
    PTR_rankH->rpPerfTimerPTR->new = 1;  //first time
    PTR_rankH->rpPerfTimerPTR->all = 0;  //go up and down (ALL_REDUCE)

    ocrEventCreateParams(&(PTR_rankH->rpPerfTimerEVT), OCR_EVENT_CHANNEL_T, true, &params);//TODO
    PTR_rankH->rpPerfTimerPTR->returnEVT = PTR_rankH->rpPerfTimerEVT;

    //ocrDbRelease(PTR_rankH->rpPerfTimerDBK);

    DEBUG_PRINTF(("nRanks %d id %d\n", nRanks, id));
}

void printSimulationData(SimFlat* s, int maxOcc)
{
   // Only rank 0 prints
    if(s->PTR_rankH->myRank != 0 )
       return;

    PRINTF("Simulation data: \n");
    PRINTF("  Total atoms        : %d\n",
            s->atoms->nGlobal);
    PRINTF("  Min global bounds  : [ %14.10f, %14.10f, %14.10f ]\n",
            s->domain->globalMin[0], s->domain->globalMin[1], s->domain->globalMin[2]);
    PRINTF("  Max global bounds  : [ %14.10f, %14.10f, %14.10f ]\n",
            s->domain->globalMax[0], s->domain->globalMax[1], s->domain->globalMax[2]);
    printSeparator();
    PRINTF("Decomposition data: \n");
    PRINTF("  Processors         : %6d,%6d,%6d\n",
            s->domain->procGrid[0], s->domain->procGrid[1], s->domain->procGrid[2]);
    PRINTF("  Local boxes        : %6d,%6d,%6d = %8d\n",
            s->boxes->gridSize[0], s->boxes->gridSize[1], s->boxes->gridSize[2],
            s->boxes->gridSize[0]*s->boxes->gridSize[1]*s->boxes->gridSize[2]);
    PRINTF("  Box size           : [ %14.10f, %14.10f, %14.10f ]\n",
            s->boxes->boxSize[0], s->boxes->boxSize[1], s->boxes->boxSize[2]);
    PRINTF("  Box factor         : [ %14.10f, %14.10f, %14.10f ] \n",
            s->boxes->boxSize[0]/s->pot->cutoff,
            s->boxes->boxSize[1]/s->pot->cutoff,
            s->boxes->boxSize[2]/s->pot->cutoff);
    PRINTF( "  Max Link Cell Occupancy: %d of %d\n",
            maxOcc, MAXATOMS);
    printSeparator();
    PRINTF("Potential data: \n");
    s->pot->print(s->pot);

    // Memory footprint diagnostics
    int perAtomSize = 10*sizeof(real_t)+2*sizeof(int);
    float mbPerAtom = perAtomSize/1024/1024;
    float totalMemLocal = (float)(perAtomSize*s->atoms->nLocal)/1024/1024;
    float totalMemGlobal = (float)(perAtomSize*s->atoms->nGlobal)/1024/1024;

    int nLocalBoxes = s->boxes->gridSize[0]*s->boxes->gridSize[1]*s->boxes->gridSize[2];
    int nTotalBoxes = (s->boxes->gridSize[0]+2)*(s->boxes->gridSize[1]+2)*(s->boxes->gridSize[2]+2);
    float paddedMemLocal = (float) nLocalBoxes*(perAtomSize*MAXATOMS)/1024/1024;
    float paddedMemTotal = (float) nTotalBoxes*(perAtomSize*MAXATOMS)/1024/1024;

    printSeparator();
    PRINTF("Memory data: \n");
    PRINTF( "  Intrinsic atom footprint = %4d B/atom \n", perAtomSize);
    PRINTF( "  Total atom footprint     = %7.3f MB (%6.2f MB/node)\n", totalMemGlobal, totalMemLocal);
    PRINTF( "  Link cell atom footprint = %7.3f MB/node\n", paddedMemLocal);
    PRINTF( "  Link cell atom footprint = %7.3f MB/node (including halo cell data\n", paddedMemTotal);
}

ocrGuid_t printSimulationData1Edt( EDT_ARGS )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    s32 _idep;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_sim = depv[_idep++].guid;
    ocrDBK_t DBK_pot = depv[_idep++].guid;
    ocrDBK_t DBK_maxOcc = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    SimFlat* sim = depv[_idep++].ptr;
    BasePotential* pot = depv[_idep++].ptr;
    int* maxOccPTR = depv[_idep++].ptr;

    sim->atoms = &sim->atoms_INST;
    sim->boxes = &sim->boxes_INST;
    sim->species = &sim->species_INST;
    sim->atomExchange = &sim->atomExchange_INST;

    sim->PTR_rankH = PTR_rankH;
    sim->pot = pot;

    ocrDBK_t DBK_nAtoms = sim->boxes->DBK_nAtoms;

    ocrDBK_t DBK_gid = sim->atoms->DBK_gid;
    ocrDBK_t DBK_iSpecies = sim->atoms->DBK_iSpecies;
    ocrDBK_t DBK_r = sim->atoms->DBK_r;
    ocrDBK_t DBK_p = sim->atoms->DBK_p;
    ocrDBK_t DBK_f = sim->atoms->DBK_f;
    ocrDBK_t DBK_U = sim->atoms->DBK_U;

    ocrDBK_t DBK_parms = sim->atomExchange->DBK_parms;

    int maxOcc = maxOccPTR[0];

     printSimulationData(sim, maxOcc);

    if(sim->PTR_rankH->myRank==0) timestamp("Initialization Finished\n");

    return NULL_GUID;
}

ocrGuid_t printSimulationDataEdt( EDT_ARGS )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    s32 _idep;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_sim = depv[_idep++].guid;
    ocrDBK_t rpmaxOccupancyDBK = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    SimFlat* sim = depv[_idep++].ptr;
    reductionPrivate_t* rpmaxOccupancyPTR = depv[_idep++].ptr;

    //PTR_rankH->rpmaxOccupancyPTR = rpmaxOccupancyPTR;
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    sim->atoms = &sim->atoms_INST;
    sim->boxes = &sim->boxes_INST;
    sim->species = &sim->species_INST;
    sim->atomExchange = &sim->atomExchange_INST;

    sim->PTR_rankH = PTR_rankH;
    sim->pot = NULL;

    ocrHint_t myEdtAffinityHNT = sim->PTR_rankH->myEdtAffinityHNT;

    ocrDBK_t DBK_nAtoms = sim->boxes->DBK_nAtoms;

    ocrDBK_t DBK_gid = sim->atoms->DBK_gid;
    ocrDBK_t DBK_iSpecies = sim->atoms->DBK_iSpecies;
    ocrDBK_t DBK_r = sim->atoms->DBK_r;
    ocrDBK_t DBK_p = sim->atoms->DBK_p;
    ocrDBK_t DBK_f = sim->atoms->DBK_f;
    ocrDBK_t DBK_U = sim->atoms->DBK_U;

    ocrDBK_t DBK_parms = sim->atomExchange->DBK_parms;
    ocrDBK_t DBK_pot = sim->DBK_pot;

    int maxOcc = maxOccupancy(sim->boxes);

    int maxOccPTR[1] = {maxOcc};

    reductionLaunch(rpmaxOccupancyPTR, rpmaxOccupancyDBK, maxOccPTR);

    ocrGuid_t printSimulationData1TML, printSimulationData1EDT, printSimulationData1OEVT, printSimulationData1OEVTS;

    ocrEdtTemplateCreate( &printSimulationData1TML, printSimulationData1Edt, 0, 4 );

    ocrEdtCreate( &printSimulationData1EDT, printSimulationData1TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &printSimulationData1OEVT );

    ocrEdtTemplateDestroy( printSimulationData1TML );

    _idep = 0;
    ocrAddDependence( DBK_rankH, printSimulationData1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_sim, printSimulationData1EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_pot, printSimulationData1EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( rpmaxOccupancyPTR->returnEVT, printSimulationData1EDT, _idep++, DB_MODE_RO );

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_initSimulation )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));

    s32 _idep, _paramc, _depc;

    u64 id = paramv[0];

    ocrGuid_t DBK_rankH = depv[0].guid;
    ocrDBK_t DBK_sim = depv[1].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    SimFlat* sim = depv[1].ptr;

    ocrHint_t myEdtAffinityHNT, myDbkAffinityHNT;

    getAffinityHintsForDBandEdt( &myDbkAffinityHNT, &myEdtAffinityHNT );

    //rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    sim->PTR_rankH = PTR_rankH;

    u64 nRanks = (PTR_cmd->xproc)*(PTR_cmd->yproc)*(PTR_cmd->zproc);

    initTimers(sim->perfTimer);
    profileStart(sim->perfTimer, totalTimer);
    if(id==0) timestamp("Starting Initialization\n");

    initOcrObjects( PTR_rankH, id, nRanks );

    initSimulation( sim, PTR_rankH, id );

    ocrEVT_t rpVcmEVT = PTR_rankH->rpVcmEVT;
    ocrEVT_t rpKeEVT = PTR_rankH->rpKeEVT;
    ocrEVT_t rpmaxOccupancyEVT = PTR_rankH->rpmaxOccupancyEVT;
    ocrEVT_t rpPerfTimerEVT = PTR_rankH->rpPerfTimerEVT;

    ocrDBK_t DBK_pot = sim->DBK_pot;
    ocrDBK_t DBK_nAtoms = sim->boxes->DBK_nAtoms;

    ocrDBK_t DBK_gid = sim->atoms->DBK_gid;
    ocrDBK_t DBK_iSpecies = sim->atoms->DBK_iSpecies;
    ocrDBK_t DBK_r = sim->atoms->DBK_r;
    ocrDBK_t DBK_p = sim->atoms->DBK_p;
    ocrDBK_t DBK_f = sim->atoms->DBK_f;
    ocrDBK_t DBK_U = sim->atoms->DBK_U;

    ocrDBK_t DBK_parms = sim->atomExchange->DBK_parms;

    ocrDBK_t rpKeDBK = PTR_rankH->rpKeDBK;
    ocrDBK_t rpVcmDBK = PTR_rankH->rpVcmDBK;
    ocrDBK_t rpmaxOccupancyDBK = PTR_rankH->rpmaxOccupancyDBK;
    ocrDBK_t rpPerfTimerDBK = PTR_rankH->rpPerfTimerDBK;

    ocrTML_t redistributeAtomsTML = PTR_rankTemplateH->redistributeAtomsTML;
    ocrTML_t computeForceTML = PTR_rankTemplateH->computeForceTML;
    ocrTML_t kineticEnergyTML = PTR_rankTemplateH->kineticEnergyTML;
    ocrTML_t printThingsTML = PTR_rankTemplateH->printThingsTML;
    ocrTML_t timestepLoopTML = PTR_rankTemplateH->timestepLoopTML;

    ocrDbRelease(DBK_sim);
    ocrDbRelease(DBK_rankH);

    ocrDbRelease(DBK_pot);

    ocrDbRelease(DBK_nAtoms);

    ocrDbRelease(DBK_gid);
    ocrDbRelease(DBK_iSpecies);
    ocrDbRelease(DBK_r);
    ocrDbRelease(DBK_p);
    ocrDbRelease(DBK_f);
    ocrDbRelease(DBK_U);

    ocrDbRelease(rpKeDBK);
    ocrDbRelease(rpmaxOccupancyDBK);


    //Vcm reduction tree has been set up.

    ocrGuid_t adjustVcmAndComputeKeTML, adjustVcmAndComputeKeEDT, adjustVcmAndComputeKeOEVT, adjustVcmAndComputeKeOEVTS;

    ocrEdtTemplateCreate( &adjustVcmAndComputeKeTML, adjustVcmAndComputeKeEdt, 0, 7 );

    ocrEdtCreate( &adjustVcmAndComputeKeEDT, adjustVcmAndComputeKeTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &adjustVcmAndComputeKeOEVT );

    ocrEdtTemplateDestroy( adjustVcmAndComputeKeTML );

    _idep = 0;
    ocrAddDependence( DBK_rankH, adjustVcmAndComputeKeEDT, _idep++, DB_MODE_RW ); //TODO
    ocrAddDependence( DBK_sim, adjustVcmAndComputeKeEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_nAtoms, adjustVcmAndComputeKeEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_iSpecies, adjustVcmAndComputeKeEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_p, adjustVcmAndComputeKeEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( rpKeDBK, adjustVcmAndComputeKeEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( rpVcmEVT, adjustVcmAndComputeKeEDT, _idep++, DB_MODE_RO );

    ////Adjust momentum again: adjustTemperature()

    ocrGuid_t adjustTemperatureTML, adjustTemperatureEDT, adjustTemperatureOEVT, adjustTemperatureOEVTS;

    adjustTemperatureEdtParamv_t adjustTemperatureEdtParamv;
    adjustTemperatureEdtParamv.temperature = PTR_cmd->temperature;

    ocrEdtTemplateCreate( &adjustTemperatureTML, adjustTemperatureEdt, sizeof(adjustTemperatureEdtParamv_t)/sizeof(u64), 5 );

    ocrEdtCreate( &adjustTemperatureEDT, adjustTemperatureTML,
                  EDT_PARAM_DEF, (u64*) &adjustTemperatureEdtParamv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &adjustTemperatureOEVT );

    ocrEdtTemplateDestroy( adjustTemperatureTML );

    createEventHelper( &adjustTemperatureOEVTS, 1);
    ocrAddDependence( adjustTemperatureOEVT, adjustTemperatureOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, adjustTemperatureEDT, _idep++, DB_MODE_RO ); //TODO
    ocrAddDependence( DBK_sim, adjustTemperatureEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_nAtoms, adjustTemperatureEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_p, adjustTemperatureEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( rpKeEVT, adjustTemperatureEDT, _idep++, DB_MODE_RO );

    //randomDisplacements( s, delta );

    ocrGuid_t randomDisplacementsTML, randomDisplacementsEDT, randomDisplacementsOEVT, randomDisplacementsOEVTS;

    randomDisplacementsEdtParamv_t randomDisplacementsEdtParamv;
    randomDisplacementsEdtParamv.delta = PTR_cmd->initialDelta;

    ocrEdtTemplateCreate( &randomDisplacementsTML, randomDisplacementsEdt, sizeof(randomDisplacementsEdtParamv_t)/sizeof(u64), 5 );

    ocrEdtCreate( &randomDisplacementsEDT, randomDisplacementsTML,
                  EDT_PARAM_DEF, (u64*) &randomDisplacementsEdtParamv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &randomDisplacementsOEVT );
    ocrEdtTemplateDestroy( randomDisplacementsTML );

    createEventHelper( &randomDisplacementsOEVTS, 1);
    ocrAddDependence( randomDisplacementsOEVT, randomDisplacementsOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, randomDisplacementsEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_sim, randomDisplacementsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_nAtoms, randomDisplacementsEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_r, randomDisplacementsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( adjustTemperatureOEVTS, randomDisplacementsEDT, _idep++, DB_MODE_RO );

    //redistribute atoms
    ocrGuid_t redistributeAtomsEDT, redistributeAtomsOEVT, redistributeAtomsOEVTS;

    u64 itimestep = 0;
    ocrEdtCreate( &redistributeAtomsEDT, redistributeAtomsTML,
                  EDT_PARAM_DEF, &itimestep, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &myEdtAffinityHNT, &redistributeAtomsOEVT );

    createEventHelper( &redistributeAtomsOEVTS, 1);
    ocrAddDependence( redistributeAtomsOEVT, redistributeAtomsOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, redistributeAtomsEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_sim, redistributeAtomsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( randomDisplacementsOEVTS, redistributeAtomsEDT, _idep++, DB_MODE_NULL );

    //compute force
    //
    ocrGuid_t computeForceEDT, computeForceOEVT, computeForceOEVTS;

    ocrEdtCreate( &computeForceEDT, computeForceTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &myEdtAffinityHNT, &computeForceOEVT );

    createEventHelper( &computeForceOEVTS, 1);
    ocrAddDependence( computeForceOEVT, computeForceOEVTS, 0, DB_MODE_NULL );

    _idep = 0;

    ocrAddDependence( DBK_rankH, computeForceEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_sim, computeForceEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_pot, computeForceEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( redistributeAtomsOEVTS, computeForceEDT, _idep++, DB_MODE_NULL );


    ////Compute Kinetic energy of the system
    ocrGuid_t kineticEnergyEDT, kineticEnergyOEVT, kineticEnergyOEVTS;

    ocrEdtCreate( &kineticEnergyEDT, kineticEnergyTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &kineticEnergyOEVT );

    createEventHelper( &kineticEnergyOEVTS, 1);
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
    ocrGuid_t printSimulationDataTML, printSimulationDataEDT, printSimulationDataOEVT, printSimulationDataOEVTS;

    ocrEdtTemplateCreate( &printSimulationDataTML, printSimulationDataEdt, 1, 4 );

    ocrEdtCreate( &printSimulationDataEDT, printSimulationDataTML,
                  EDT_PARAM_DEF, (u64*) &itimestep, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &myEdtAffinityHNT, &printSimulationDataOEVT );

    ocrEdtTemplateDestroy( printSimulationDataTML );

    createEventHelper( &printSimulationDataOEVTS, 1);
    ocrAddDependence( printSimulationDataOEVT, printSimulationDataOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, printSimulationDataEDT, _idep++, DB_MODE_RW ); //TODO
    ocrAddDependence( DBK_sim, printSimulationDataEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( rpmaxOccupancyDBK, printSimulationDataEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( kineticEnergyOEVTS, printSimulationDataEDT, _idep++, DB_MODE_NULL );

    ////Print things
    ocrGuid_t printThingsEDT, printThingsOEVT, printThingsOEVTS;

    ocrEdtCreate( &printThingsEDT, printThingsTML,
                  EDT_PARAM_DEF, (u64*) &itimestep, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &printThingsOEVT );

    createEventHelper( &printThingsOEVTS, 1);
    ocrAddDependence( printThingsOEVT, printThingsOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, printThingsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_sim, printThingsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( rpKeEVT, printThingsEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( rpPerfTimerDBK, printThingsEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( printSimulationDataOEVTS, printThingsEDT, _idep++, DB_MODE_NULL );

    //start timestep loop
    if( PTR_cmd->nSteps > 0 )
    {
    ocrGuid_t timestepLoopEDT, timestepLoopOEVT, timestepLoopOEVTS;

    ocrEdtCreate( &timestepLoopEDT, timestepLoopTML,
                  EDT_PARAM_DEF, (u64*) &itimestep, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL);

    _idep = 0;
    ocrAddDependence(DBK_rankH, timestepLoopEDT, _idep++, DB_MODE_RW);
    ocrAddDependence(DBK_sim, timestepLoopEDT, _idep++, DB_MODE_RW);
    ocrAddDependence(printThingsOEVTS, timestepLoopEDT, _idep++, DB_MODE_NULL);
    ocrAddDependence(NULL_GUID, timestepLoopEDT, _idep++, DB_MODE_NULL);
    ocrAddDependence(NULL_GUID, timestepLoopEDT, _idep++, DB_MODE_NULL);
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_compute )
{
    return NULL_GUID;
}
_OCR_TASK_FNC_( FNC_finalize )
{
    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_comdMain )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));
    s32 _idep, _paramc, _depc;

    u64 id = paramv[0];

    ocrGuid_t DBK_rankH = depv[0].guid;

    rankH_t* PTR_rankH = depv[0].ptr;

    //rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    ocrHint_t myDbkAffinityHNT, myEdtAffinityHNT;
    getAffinityHintsForDBandEdt( &myDbkAffinityHNT, &myEdtAffinityHNT );

    ocrDBK_t DBK_sim;
    SimFlat* sim;

    ocrDbCreate( &(DBK_sim), (void **) &sim, sizeof(SimFlat),
                 DB_PROP_NONE, &myDbkAffinityHNT, NO_ALLOC );

    ocrDbRelease(DBK_sim);

    MyOcrTaskStruct_t TS_initSimulation; _paramc = 1; _depc = 2;
    ocrGuid_t TS_initSimulation_OEVTS;
    ocrEventCreate( &TS_initSimulation_OEVTS, OCR_EVENT_STICKY_T, false );

    TS_initSimulation.FNC = FNC_initSimulation;
    ocrEdtTemplateCreate( &TS_initSimulation.TML, TS_initSimulation.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_initSimulation.EDT, TS_initSimulation.TML,
                  EDT_PARAM_DEF, &id, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &TS_initSimulation.OET );

    ocrEdtTemplateDestroy( TS_initSimulation.TML );

    ocrAddDependence( TS_initSimulation.OET, TS_initSimulation_OEVTS, 0, DB_MODE_NULL );


    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_initSimulation.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_sim, TS_initSimulation.EDT, _idep++, DB_MODE_RW );

    #if 0
    MyOcrTaskStruct_t TS_compute; _paramc = 0; _depc = 3;
    ocrGuid_t TS_compute_OEVTS;
    ocrEventCreate( &TS_compute_OEVTS, OCR_EVENT_STICKY_T, false );

    TS_compute.FNC = FNC_compute;
    ocrEdtTemplateCreate( &TS_compute.TML, TS_compute.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_compute.EDT, TS_compute.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_compute.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_sim, TS_compute.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( TS_initSimulation_OEVTS, TS_compute.EDT, _idep++, DB_MODE_NULL );

    MyOcrTaskStruct_t TS_finalize; _paramc = 0; _depc = 3;

    TS_finalize.FNC = FNC_finalize;
    ocrEdtTemplateCreate( &TS_finalize.TML, TS_finalize.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_finalize.EDT, TS_finalize.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_finalize.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_sim, TS_finalize.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( TS_compute_OEVTS, TS_finalize.EDT, _idep++, DB_MODE_NULL );

    #endif

    return NULL_GUID;
}

typedef struct
{
    u64 id;
    int nNbrs;

} PRM_channelSetupEdt_t;

_OCR_TASK_FNC_( channelSetupEdt )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));
    s32 _idep, _paramc, _depc;

    PRM_channelSetupEdt_t* PRM_channelSetupEdt = (PRM_channelSetupEdt_t*) paramv;

    u64 id = PRM_channelSetupEdt->id;
    int nNbrs = PRM_channelSetupEdt->nNbrs;

    ocrGuid_t DBK_rankH = depv[nNbrs].guid;

    rankH_t* PTR_rankH = depv[nNbrs].ptr;

    //rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    u64 i;

    //Capture all the events needed for inter-rank depedencies
    for(i=0;i<nNbrs;i++)
    {
        ocrGuid_t* eventsPTR = depv[i].ptr;
#ifdef CHANNEL_EVENTS_AT_RECEIVER
        PTR_rankH->haloSendEVTs[i] = eventsPTR[0];
        PTR_rankH->haloTagSendEVTs[i] = eventsPTR[1];
#ifdef DOUBLE_BUFFERED_EVTS
        PTR_rankH->haloSendEVTs[nNbrs+i] = eventsPTR[2];
        PTR_rankH->haloTagSendEVTs[nNbrs+i] = eventsPTR[3];
#endif
#else
        PTR_rankH->haloRecvEVTs[i] = eventsPTR[0];
        PTR_rankH->haloTagRecvEVTs[i] = eventsPTR[1];
#ifdef DOUBLE_BUFFERED_EVTS
        PTR_rankH->haloRecvEVTs[nNbrs+i] = eventsPTR[2];
        PTR_rankH->haloTagRecvEVTs[nNbrs+i] = eventsPTR[3];
#endif
#endif
        DEBUG_PRINTF(("Rank s %d d %d Send "GUIDF" Recv "GUIDF" \n", id, i, PTR_rankH->haloSendEVTs[i], PTR_rankH->haloRecvEVTs[i]));
    }

    ocrDbRelease(DBK_rankH);

    ocrHint_t myDbkAffinityHNT, myEdtAffinityHNT;

    getAffinityHintsForDBandEdt( &myDbkAffinityHNT, &myEdtAffinityHNT );

    MyOcrTaskStruct_t TS_comdMain; _paramc = 1; _depc = 1;

    TS_comdMain.FNC = FNC_comdMain;
    ocrEdtTemplateCreate( &TS_comdMain.TML, TS_comdMain.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_comdMain.EDT, TS_comdMain.TML,
                  EDT_PARAM_DEF, &id, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_comdMain.EDT, _idep++, DB_MODE_RW );

    return NULL_GUID;
}

_OCR_TASK_FNC_( initEdt )
{
    PRM_initEdt_t* PTR_PRM_initEdt = (PRM_initEdt_t*) paramv;

    u64 id = PTR_PRM_initEdt->id;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;
    ocrGuid_t DBK_globalParamH = depv[1].guid;

    void *PTR_cmdLineArgs = depv[0].ptr;
    globalParamH_t *PTR_globalParamH = (globalParamH_t *) depv[1].ptr;

    ocrGuid_t DBK_rankH;
    rankH_t *PTR_rankH;

    //DB creation calls
    ocrDbCreate( &DBK_rankH, (void**) &PTR_rankH, sizeof(rankH_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

    memcpy( &(PTR_rankH->globalParamH), PTR_globalParamH, sizeof(globalParamH_t) );

    ocrHint_t myEdtAffinityHNT, myDbkAffinityHNT;
    getAffinityHintsForDBandEdt( &myDbkAffinityHNT, &myEdtAffinityHNT );

    s32 gx = PTR_PRM_initEdt->edtGridDims[0];
    s32 gy = PTR_PRM_initEdt->edtGridDims[1];
    s32 gz = PTR_PRM_initEdt->edtGridDims[2];

    PTR_rankH->myRank = id;
    PTR_rankH->nRanks = gx*gy*gz;

    int ix0, iy0, iz0, ix, iy, iz;

    globalCoordsFromRank_Cart3D( id, gx, gy, gz, &ix0, &iy0, &iz0 );

    int nNbrs = 6;

    ocrGuid_t channelSetupTML, stickyEVT;
    ocrEDT_t channelSetupEDT;

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = 2;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;

    PRM_channelSetupEdt_t PRM_channelSetupEdt;
    PRM_channelSetupEdt.id = id;
    PRM_channelSetupEdt.nNbrs = nNbrs;

    ocrEdtTemplateCreate( &channelSetupTML, channelSetupEdt, sizeof(PRM_channelSetupEdt_t)/sizeof(u64), nNbrs+1 );
    ocrEdtCreate( &channelSetupEDT, channelSetupTML, EDT_PARAM_DEF, (u64*)&PRM_channelSetupEdt, EDT_PARAM_DEF, NULL, EDT_PROP_NONE,
                    &myEdtAffinityHNT, NULL );

    int nbr, i, j, k;
    int nbrImage;

    //6 neighbors: -x, +x, -y, +y, -z, +z
    for( nbr = 0; nbr < nNbrs; nbr++ )
    {
        i = j = k = 0;

        switch(nbr)
        {
            case 0:
                i = -1;                 // neighbor offset
                nbrImage = 1;           // this is the tag the same nbr uses to refer to the current "rank"
                break;
            case 1:
                i = 1;
                nbrImage = 0;
                break;
            case 2:
                j = -1;
                nbrImage = 3;
                break;
            case 3:
                j = 1;
                nbrImage = 2;
                break;
            case 4:
                k = -1;
                nbrImage = 5;
                break;
            case 5:
                k = 1;
                nbrImage = 4;
                break;
        }

        ix = MOD(ix0+i, gx); //periodic index
        iy = MOD(iy0+j, gy);
        iz = MOD(iz0+k, gz);
        u32 nbrRank = globalRankFromCoords_Cart3D(ix, iy, iz, gx, gy, gz);

        //Collective event create for sends
        ocrGuidFromIndex(&(stickyEVT), PTR_rankH->globalParamH.ocrParamH.haloRangeGUID, nNbrs*id + nbr);//send
        ocrEventCreate( &stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );

        ocrGuid_t* eventsPTR;
        ocrGuid_t eventsDBK;
        int nEvents = 1;
#ifdef DOUBLE_BUFFERED_EVTS
        nEvents = 2;
#endif
        ocrDbCreate( &eventsDBK, (void**) &eventsPTR, nEvents*2*sizeof(ocrGuid_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

#ifdef CHANNEL_EVENTS_AT_RECEIVER
        ocrEventCreateParams( &(PTR_rankH->haloRecvEVTs[nbr]), OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &(PTR_rankH->haloTagRecvEVTs[nbr]), OCR_EVENT_CHANNEL_T, false, &params );

        eventsPTR[0] = PTR_rankH->haloRecvEVTs[nbr]; //channel event of the send operation from rank i to nbrRank
        eventsPTR[1] = PTR_rankH->haloTagRecvEVTs[nbr];
#ifdef DOUBLE_BUFFERED_EVTS
        ocrEventCreateParams( &(PTR_rankH->haloRecvEVTs[nNbrs+nbr]), OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &(PTR_rankH->haloTagRecvEVTs[nNbrs+nbr]), OCR_EVENT_CHANNEL_T, false, &params );
        eventsPTR[2] = PTR_rankH->haloRecvEVTs[nNbrs+nbr]; //channel event of the send operation from rank i to nbrRank
        eventsPTR[3] = PTR_rankH->haloTagRecvEVTs[nNbrs+nbr];
#endif
#else
        ocrEventCreateParams( &(PTR_rankH->haloSendEVTs[nbr]), OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &(PTR_rankH->haloTagSendEVTs[nbr]), OCR_EVENT_CHANNEL_T, false, &params );

        eventsPTR[0] = PTR_rankH->haloSendEVTs[nbr]; //channel event of the send operation from rank i to nbrRank
        eventsPTR[1] = PTR_rankH->haloTagSendEVTs[nbr];
#ifdef DOUBLE_BUFFERED_EVTS
        ocrEventCreateParams( &(PTR_rankH->haloSendEVTs[nNbrs+nbr]), OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &(PTR_rankH->haloTagSendEVTs[nNbrs+nbr]), OCR_EVENT_CHANNEL_T, false, &params );

        eventsPTR[2] = PTR_rankH->haloSendEVTs[nNbrs+nbr]; //channel event of the send operation from rank i to nbrRank
        eventsPTR[3] = PTR_rankH->haloTagSendEVTs[nNbrs+nbr];
#endif
#endif

        ocrDbRelease( eventsDBK );

        ocrEventSatisfy( stickyEVT, eventsDBK );

        //Map the sends to receive events
        //receive: (id, nbr) will be the send event from the (nbrRank,nbrImage)
        ocrGuidFromIndex( &(stickyEVT), PTR_rankH->globalParamH.ocrParamH.haloRangeGUID, nNbrs*nbrRank + nbrImage );
        ocrEventCreate( &stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );

        //DEBUG_PRINTF(("s %d se %d r %d re %d s(%d %d %d) r(%d %d %d)\n", id, nbr, nbrRank, nbrImage, ix0, iy0, iz0, ix, iy, iz ));
        //PRINTF("Send rank %d %d "GUIDF" \n", id, nbr, PTR_rankH->haloSendEVTs[nbr]);

        ocrAddDependence( stickyEVT, channelSetupEDT, nbr, DB_MODE_RW ); //TODO
    }

    ocrDbRelease(DBK_rankH);

    //ocrAddDependence( DBK_cmdLineArgs, channelSetupEDT, nNbrs, DB_MODE_RO );
    ocrAddDependence( DBK_rankH, channelSetupEDT, nNbrs, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t wrapUpEdt( u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[] )
{
    PRINTF("Shutting down\n");
    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;

    void * PTR_cmdLineArgs = depv[0].ptr;
    u32 argc = getArgc( PTR_cmdLineArgs );

    //Pack the PTR_cmdLineArgs into the "cannonical" char** argv
    ocrGuid_t argv_g;
    char** argv;
    ocrDbCreate( &argv_g, (void**)&argv, sizeof(char*)*argc, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    for( u32 a = 0; a < argc; ++a )
       argv[a] = getArgv( PTR_cmdLineArgs, a );

    Command cmd = parseCommandLine(argc, argv);
    printCmdYaml(screenOut, &cmd);

    u64 nRanks = (cmd.xproc)*(cmd.yproc)*(cmd.zproc);

    //Create OCR objects to pass down to the child EDTs
    //for collective operatations among the child EDTs
    globalOcrParamH_t ocrParamH;
    ocrGuidRangeCreate(&(ocrParamH.haloRangeGUID), 6*nRanks, GUID_USER_EVENT_STICKY);
    //TODO: Either have 6 nbrs or 27 nbrs depending on the halo-exchange implementation
    ocrGuidRangeCreate(&(ocrParamH.KeReductionRangeGUID), nRanks, GUID_USER_EVENT_STICKY);
    ocrGuidRangeCreate(&(ocrParamH.VcmReductionRangeGUID),nRanks, GUID_USER_EVENT_STICKY);
    ocrGuidRangeCreate(&(ocrParamH.maxOccupancyReductionRangeGUID),nRanks, GUID_USER_EVENT_STICKY);
    ocrGuidRangeCreate(&(ocrParamH.perfTimerReductionRangeGUID),nRanks, GUID_USER_EVENT_STICKY);

    ocrGuid_t wrapUpTML, wrapUpEDT;
    ocrEdtTemplateCreate( &wrapUpTML, wrapUpEdt, 0, 1 );
    ocrEdtCreate( &wrapUpEDT, wrapUpTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
    ocrEventCreate( &(ocrParamH.finalOnceEVT), OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG );
    ocrEdtTemplateDestroy( wrapUpTML );

    ocrAddDependence(ocrParamH.finalOnceEVT, wrapUpEDT, 0, DB_MODE_RO);

    //A datablock to store the commandline and the OCR objectes created above
    ocrGuid_t DBK_globalParamH;
    globalParamH_t* PTR_globalParamH;
    ocrDbCreate( &DBK_globalParamH, (void**) &PTR_globalParamH, sizeof(globalParamH_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

    memcpy(&(PTR_globalParamH->cmdParamH), &cmd, sizeof(Command) );
    memcpy(&(PTR_globalParamH->ocrParamH), &ocrParamH, sizeof(globalOcrParamH_t) );

    //Release the changes so they are visible for any dependent EDTs below
    ocrDbRelease(DBK_globalParamH);

    s64 edtGridDims[3] = { cmd.xproc, cmd.yproc, cmd.zproc };

    //All SPMD EDTs depend on the following dependencies
    ocrGuid_t spmdDepv[2] = {DBK_cmdLineArgs, DBK_globalParamH};

    //2-D Cartesian grid of SPMD EDTs get mapped to a 2-D Cartesian grid of PDs
#ifdef USE_STATIC_SCHEDULER
    DEBUG_PRINTF(("Using STATIC scheduler\n"));
    forkSpmdEdts_staticScheduler_Cart3D( initEdt, edtGridDims, spmdDepv );
#else
    DEBUG_PRINTF(("NOT Using STATIC scheduler\n"));
    forkSpmdEdts_Cart3D( initEdt, edtGridDims, spmdDepv );
#endif

    return NULL_GUID;
}

/// \file
/// Main program
///
/// \mainpage CoMD: A Classical Molecular Dynamics Mini-app
///
/// CoMD is a reference implementation of typical classical molecular
/// dynamics algorithms and workloads.  It is created and maintained by
/// The Exascale Co-Design Center for Materials in Extreme Environments
/// (ExMatEx).  http://codesign.lanl.gov/projects/exmatex.  The
/// code is intended to serve as a vehicle for co-design by allowing
/// others to extend and/or reimplement it as needed to test performance of
/// new architectures, programming models, etc.
///
/// The current version of CoMD is available from:
/// http://exmatex.github.io/CoMD
///
/// To contact the developers of CoMD send email to: exmatex-comd@llnl.gov.
///
/// Table of Contents
/// =================
///
/// Click on the links below to browse the CoMD documentation.
///
/// \subpage pg_openmp_specifics
///
/// \subpage pg_md_basics
///
/// \subpage pg_building_comd
///
/// \subpage pg_running_comd
///
/// \subpage pg_measuring_performance
///
/// \subpage pg_problem_selection_and_scaling
///
/// \subpage pg_verifying_correctness
///
/// \subpage pg_comd_architecture
///
/// \subpage pg_optimization_targets
///
/// \subpage pg_whats_new

#include "comd.h"

//#include <stdio.h>
//#include <stdlib.h>
#include "string.h"
//#include <strings.h>
//#include <unistd.h>
//#include <assert.h>
#ifndef CNCOCR_TG
 #include <sys/time.h>
#endif

#include "CoMDTypes.h"
#include "decomposition.h"
#include "linkCells.h"
#include "initAtoms.h"
#include "memUtils.h"
//#include "yamlOutput.h"
#include "mycommand.h"
#include "ljForce.h"
#include "eam.h"
#include "parallel.h"

//#include "haloExchange.h"
#include "constants.h"




struct DomainSt* initDecomposition(int xproc, int yproc, int zproc,
                                   real3 globalExtent, comdCtx *c);

LinkCell* initLinkCells(const struct DomainSt* domain, real_t cutoff, comdCtx *c);
Atoms* initAtoms(struct LinkCellSt* boxes, comdCtx *c);

//static void printSimulationDataYaml(FILE* file, SimFlat* s);


/// decide whether to get LJ or EAM potentials
BasePotential* initPotential(
   int doeam, const char* potDir, const char* potName, const char* potType, comdCtx *ctx)
{
   BasePotential* pot = NULL;

   if (doeam) {
     pot = initEamPot(potDir, potName, potType);
   } else {
     pot = initLjPot(ctx);
   }
   assert(pot);
   return pot;
}

SpeciesData* initSpecies(BasePotential* pot, comdCtx *ctx)
{
 //  SpeciesData* species = comdMalloc(sizeof(SpeciesData));
   SpeciesData* species = cncItemAlloc(sizeof(*species));

   cncPut_SPECIES(species, 1, ctx);

   strcpy(species->name, pot->name);
   species->atomicNo = pot->atomicNo;
   species->mass = pot->mass;

   return species;
}

void sumAtoms(SimFlat* s)
{
   // sum atoms across all processers
   s->atoms->nLocal = 0;
   for (int i = 0; i < s->boxes->nLocalBoxes; i++)
   {
      s->atoms->nLocal += s->boxes->nAtoms[i];
   }

//   startTimer(commReduceTimer);
   addIntParallel(&s->atoms->nLocal, &s->atoms->nGlobal, 1);
//   stopTimer(commReduceTimer);
}


/// Check that the user input meets certain criteria.
void sanityChecks(Command cmd, double cutoff, double latticeConst, char latticeType[8]) {
   int failCode = 0;

   // Check that domain grid matches number of ranks. (fail code 1)
   int nProcs = cmd.xproc * cmd.yproc * cmd.zproc;
   if (nProcs != getNRanks())
   {
      failCode |= 1;
      if (printRank() )
        printf("\nNumber of MPI ranks must match xproc * yproc * zproc\n");
   }

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
      if (printRank())
         printf("\nSimulation too small.\n Increase the number of unit cells to make the simulation\n at least (%3.2f, %3.2f. %3.2f) Ansgstroms in size\n", minx, miny, minz);
   }

   // Check for supported lattice structure (fail code 4)
   if (strcasecmp(latticeType, "FCC") != 0)
   {
      failCode |= 4;
      if ( printRank() )
         printf("\nOnly FCC Lattice type supported, not %s. Fatal Error.\n",latticeType);
   }
   int checkCode = failCode;
//   bcastParallel(&checkCode, sizeof(int), 0);  // ToDo: Manu
   // This assertion can only fail if different tasks failed different
   // sanity checks.  That should not be possible.
   assert(checkCode == failCode);

//   if (failCode != 0)
//      exit(failCode);
}


SimFlat* initSimulationNew(Command *cmd, comdCtx *ctx) {

   SimFlat *sim = cncItemAlloc(sizeof(*sim));

   cncPut_SF(sim, 1, ctx);


   sim->nSteps = cmd->nSteps;
   sim->printRate = cmd->printRate;
   sim->dt = cmd->dt;
   sim->domain = NULL;
   sim->boxes = NULL;
   sim->atoms = NULL;
   sim->ePotential = 0.0;
   sim->eKinetic = 0.0;


   sim->pot = initPotential(cmd->doeam, cmd->potDir, cmd->potName, cmd->potType, ctx);
   real_t latticeConstant = cmd->lat;
   if (cmd->lat < 0.0)
      latticeConstant = sim->pot->lat;

   // ensure input parameters make sense.
//   sanityChecks(cmd, sim->pot->cutoff, latticeConstant, sim->pot->latticeType);   /////////////////////////////////ToDo Manu: need to uncomment this

   sim->species = initSpecies(sim->pot, ctx);

   real3 globalExtent;
   globalExtent[0] = cmd->nx * latticeConstant;
   globalExtent[1] = cmd->ny * latticeConstant;
   globalExtent[2] = cmd->nz * latticeConstant;

   sim->domain = initDecomposition(
      cmd->xproc, cmd->yproc, cmd->zproc, globalExtent, ctx);


   sim->boxes = initLinkCells(sim->domain, sim->pot->cutoff, ctx);


   sim->atoms = initAtoms(sim->boxes, ctx);


   // create lattice with desired temperature and displacement.
   createFccLattice(cmd->nx, cmd->ny, cmd->nz, latticeConstant, sim);
   setTemperature(sim, cmd->temperature);
   randomDisplacements(sim, cmd->initialDelta);

//   sim->atomExchange = initAtomHaloExchange(sim->domain, sim->boxes);

   // Forces must be computed before we call the time stepper.
 //  startTimer(redistributeTimer);
//   redistributeAtoms(sim);          /////////////////////////////////ToDo Manu: need to uncomment this
 //  stopTimer(redistributeTimer);

//   startTimer(computeForceTimer);
//   computeForce(sim);              /////////////////////////////////ToDo Manu: need to uncomment this
//   stopTimer(computeForceTimer);

//   kineticEnergy(sim);             /////////////////////////////////ToDo Manu: need to uncomment this

//   printf("======== %d, %d\n", sim->nSteps, sim->atoms);

   return sim;
}

Command parseContext(comdCtx * ctx)
{
   Command cmd;

   int imem;
   for (imem=0;imem<10 /*1024*/;imem++) {
     cmd.potDir[imem] = 0;
     cmd.potName[imem] = 0;
     cmd.potType[imem] = 0;
   }

   strcpy(cmd.potDir,  "pots");
   strcpy(cmd.potType, "funcfl");
   strcpy(cmd.potName, "Cu_u6.eam");

   cmd.doeam = ctx->doeam;
   cmd.nx = ctx->nx;
   cmd.ny = ctx->ny;
   cmd.nz = ctx->nz;
   cmd.xproc = ctx->xproc;
   cmd.yproc = ctx->yproc;
   cmd.zproc = ctx->zproc;
   cmd.nSteps = ctx->nSteps;
   cmd.printRate = ctx->printRate;
   cmd.dt = ctx->dt;
   cmd.lat = ctx->lat;
   cmd.temperature = ctx->temperature;
   cmd.initialDelta = ctx->initialDelta;


   return cmd;
}



//void comd_cncInitialize(int argc, char** argv, comdCtx *ctx) {
void comd_cncInitialize(comdArgs *args, comdCtx *ctx) {
    int totalBoxes;
    int MAXIT;
    int numNbrs;

    Command cmd = parseContext(ctx);

    printf("CnC:: Before initSimulation %d, %d \n", cmd.nx, cmd.nSteps);
    SimFlat* sim = initSimulationNew(&cmd, ctx);

//    printf("Initial validation ... \n");
//    Validate* validate = initValidate(sim);

    // This is the CoMD main loop

    // creating IT
    struct cmdInfo *ci = cncItemAlloc(sizeof(*ci));
    ci->nSteps = cmd.nSteps;
    ci->nx = cmd.nx; ci->ny = cmd.ny; ci->nz = cmd.nz;
    ci->xproc = cmd.xproc; ci->yproc = cmd.yproc; ci->zproc = cmd.zproc;
    ci->printRate = cmd.printRate;
    ci->dt = sim->dt;
    if (cmd.lat < 0)
        ci->lat =  sim->pot->lat;
    else
        ci->lat = cmd.lat;
    ci->temperature = cmd.temperature;
    ci->initialDelta = cmd.initialDelta;
    cncPut_CMD(ci, 1, ctx);

    const int nSteps = sim->nSteps;
    const int printRate = sim->printRate;
    int iStep = 0;

    struct eamPot *potCnC = cncItemAlloc(sizeof(*potCnC));

    if (ctx->doeam) {
      // Initialize EAM potential
      EamPotential* pot = (EamPotential *)sim->pot;
      potCnC->mass = pot->mass;
      potCnC->lat = pot->lat;
      potCnC->cutoff = pot->cutoff;
      potCnC->atomicNo = pot->atomicNo;

      potCnC->phi.n = pot->phi->n;
      potCnC->phi.x0 = pot->phi->x0;
      potCnC->phi.invDx = pot->phi->invDx;

      potCnC->rho.n = pot->rho->n;
      potCnC->rho.x0 = pot->rho->x0;
      potCnC->rho.invDx = pot->rho->invDx;

      potCnC->f.n = pot->f->n;
      potCnC->f.x0 = pot->f->x0;
      potCnC->f.invDx = pot->f->invDx;


      int i;
      for (i = 0; i < potCnC->phi.n+2; i++){
          potCnC->phi.values[i] = pot->phi->values[i];
      }

      for (i = 0; i < potCnC->rho.n+2; i++){
          potCnC->rho.values[i] = pot->rho->values[i];
      }

      for (i = 0; i < potCnC->f.n+2; i++){
          potCnC->f.values[i] = pot->f->values[i];
      }
   }
   cncPut_EAMPOT(potCnC, 0, ctx);


    /////////////////
    totalBoxes = sim->boxes->nLocalBoxes;
    MAXIT = nSteps+1;
    /////////////////
    numNbrs = 27;

    printf("CnC: Starting computation with totalBoxes = %d\n", totalBoxes);

    int i,j;
    struct box *b;

    int sum = 0;
    real_t sump = 0.0, sumr = 0.0;
    printf("-----------------------------------------\n");
    printf("Total number of atoms == %d\n", sim->atoms->nGlobal);
    for (i=0; i<totalBoxes; i++){
        b = cncItemAlloc(sizeof(*b));

        // Copy initialization data into CnC related data structures
        for (int iOff=MAXATOMS*i,ii=0; ii<sim->boxes->nAtoms[i]; ii++,iOff++) {
            // copy velocity
            b->atoms.p[ii][0] = sim->atoms->p[iOff][0];
            b->atoms.p[ii][1] = sim->atoms->p[iOff][1];
            b->atoms.p[ii][2] = sim->atoms->p[iOff][2];
            sump += b->atoms.p[ii][0]+b->atoms.p[ii][1]+b->atoms.p[ii][2];

            // copy position
            b->atoms.r[ii][0] = sim->atoms->r[iOff][0];
            b->atoms.r[ii][1] = sim->atoms->r[iOff][1];
            b->atoms.r[ii][2] = sim->atoms->r[iOff][2];
            sumr += b->atoms.r[ii][0]+b->atoms.r[ii][1]+b->atoms.r[ii][2];

            // copy force
            b->atoms.f[ii][0] = sim->atoms->f[iOff][0];
            b->atoms.f[ii][1] = sim->atoms->f[iOff][1];
            b->atoms.f[ii][2] = sim->atoms->f[iOff][2];

            // copy energy
            b->atoms.U[ii] = sim->atoms->U[iOff];

            b->atoms.iSpecies[ii] = sim->atoms->iSpecies[iOff];
            b->atoms.gid[ii] = sim->atoms->gid[iOff];

        }

        if (i == 0) {
            // Record the starting time of the computation
            struct timeval *start = cncItemAlloc(sizeof(*start));
#ifndef CNCOCR_TG
            gettimeofday(start, 0);
#endif
            cncPut_time(start, 0, ctx);
        }

        // copy other data structures of "atoms"
        b->atoms.nLocal = sim->atoms->nLocal;
        b->atoms.nGlobal = sim->atoms->nGlobal;
        b->species[0].atomicNo = sim->species[0].atomicNo;
        b->species[0].mass = sim->species[0].mass;
        strcpy(b->species[0].name, sim->species[0].name);


        // initialize other "box" related structures
        b->ePot = 0.0;
        b->eKin = 0.0;
        b->dt = sim->dt;
        b->gridSize[0] = sim->boxes->gridSize[0];
        b->gridSize[1] = sim->boxes->gridSize[1];
        b->gridSize[2] = sim->boxes->gridSize[2];
        b->nLocalBoxes = sim->boxes->nLocalBoxes;
        b->localMin[0] = sim->boxes->localMin[0];
        b->localMin[1] = sim->boxes->localMin[1];
        b->localMin[2] = sim->boxes->localMin[2];
        b->localMax[0] = sim->boxes->localMax[0];
        b->localMax[1] = sim->boxes->localMax[1];
        b->localMax[2] = sim->boxes->localMax[2];

        b->boxSize[0] = sim->boxes->boxSize[0];
        b->boxSize[1] = sim->boxes->boxSize[1];
        b->boxSize[2] = sim->boxes->boxSize[2];
        b->invBoxSize[0] = sim->boxes->invBoxSize[0];
        b->invBoxSize[1] = sim->boxes->invBoxSize[1];
        b->invBoxSize[2] = sim->boxes->invBoxSize[2];
        b->nAtoms = sim->boxes->nAtoms[i];

        b->globalExtent[0] = sim->domain->globalExtent[0];
        b->globalExtent[1] = sim->domain->globalExtent[1];
        b->globalExtent[2] = sim->domain->globalExtent[2];

        // initialize ljPotential
        b->potSigma = 2.315;
        b->potEpsilon = 0.167;
        b->potCutoff = 2.5*b->potSigma;
        b->potMass = 63.55 * amuToInternalMass;


        sum += b->nAtoms;

        b->i = i;
/////////////        cncPut_B(b, i, 0, 0, 1, ctx);
        cncPut_B(b, i, 1, 0, 0, ctx);

    //    cncPrescribe_advanceVelocityStep(i, 1, ctx);
    }

    // cleanup the initialized datastructures
    cncPrescribe_cleanupInitStep(1, ctx);

//////////    cncPrescribe_reduceStep(0, 1, ctx);
    cncPrescribe_reduceStep(0, 0, ctx);

    cncPrescribe_updateBoxStep(0, 0, 0, ctx);

/*
    int *s = cncItemAlloc(sizeof(*s));
    *s = 0;
    cncPut_s(s, 0, 1, ctx);
*/

    struct myReduction *rd = cncItemAlloc(sizeof(*rd));
    rd->i = 0;
    rd->ePot = 0.0;
    rd->eKin = 0.0;
    rd->nAtoms = 0;
    cncPut_redc(rd, 0, 1, ctx);

    // creating IT
    int *t = cncItemAlloc(sizeof(*t));
    *t = MAXIT;
    cncPut_IT(t, 0, ctx);

    // creating TBoxes
    t = cncItemAlloc(sizeof(*t));
    *t = totalBoxes;
    cncPut_TBoxes(t, 0, ctx);

   // cncPrescribe_generateForceTagsStep(1, ctx);


    // creating Nbs
    t = cncItemAlloc(sizeof(*t));
    *t = numNbrs;
    cncPut_Nbs(t, 0, ctx);

    printf(" Loop      Total Energy     Potential Energy    Kinetic Energy  #Atoms\n");
//    cncPrescribe_cncEnvOut(totalBoxes-1, MAXIT-1, ctx);
    comd_await(totalBoxes-1, MAXIT-1, ctx);
}


void comd_cncFinalize(cncTag_t i, cncTag_t iter, struct box *B, struct myReduction *r, struct timeval *start, comdCtx *ctx) {
  //  printf("Box %d, iteration %d ends\n", i, iter);
    real_t p,k,t;
//    p = r->ePot/32000;
//    k = r->eKin/32000;
    p = r->ePot/B->atoms.nGlobal;
    k = r->eKin/B->atoms.nGlobal;
    t = p+k;

    printf("Total energy = %18.12f, Potential energy = %18.12f, Kinetic energy = %18.12f\n",t,p,k);

#ifndef CNCOCR_TG
    struct timeval end;
    gettimeofday(&end, 0);
    t = end.tv_sec - start->tv_sec;
    t += (end.tv_usec - start->tv_usec) / 1000000.0;
#else
    t = 0.0;
#endif
    printf("Time taken: %f seconds\n", t);
}

#ifdef CNCOCR_TG
void abort(void) { }
#endif

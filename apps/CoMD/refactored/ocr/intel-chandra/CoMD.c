/*
Author Chandra S. Martha
Copywrite Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.

*/

#define ENABLE_EXTENSION_LABELING
#define ENABLE_EXTENSION_AFFINITY

#include "ocr.h"
#include "extensions/ocr-labeling.h" //currently needed for labeled guids
#include "extensions/ocr-affinity.h" //needed for affinity

#include "string.h" //if memcpy is needed
#include "stdio.h"  //needed for PRINTF debugging
#include "math.h" //for integer abs

#include "reduction.h"
#include "macros.h"

#include "constants.h"
#include "comd.h"
#include "command.h"
#include "potentials.h"

typedef struct
{
  real3_t domain;
  u32 lattice[3];
  u32 grid[3];
  u32 b_num;
  real3_t b_size;
  real3_t inv_b_size;
} boxDataStH_t;

typedef struct
{
    u32 step;
    u32 steps;
    u32 period;
    real_t dt;
    real3_t lattice;

    potential_t pot;

    boxDataStH_t boxDataStH;

    real_t e_potential;
    real_t e_kinetic;

    u32 nAtoms;
    double energy0;
    u32 nAtoms0;

    ocrGuid_t haloRangeGUID, vcmReductionRangeGUID, KeReductionRangeGUID;
    ocrGuid_t redistributeRangeGuid, positionRangeGuid;
    ocrGuid_t finalOnceEVT;
} sharedBlock_t;

typedef struct
{
    u32 nrank;
    u32 myrank;
    u32 timestep;
    u32 npx;
    u32 npy;
    u32 npz;
    u32 maxIter;
    u32 debug;

    //ocrGuid_t vcmReductionReturnEVT; //TODO
    //ocrGuid_t KeReductionReturnEVT;

    ocrHint_t myAffinityHNT;

    ocrGuid_t haloRangeGUID;

    ocrGuid_t comdTML;
    ocrGuid_t spmvTML;
    ocrGuid_t mgTML;
    ocrGuid_t haloExchangeTML;
    ocrGuid_t smoothTML;
    ocrGuid_t packTML;
    ocrGuid_t unpackTML;

    //ocrGuid_t haloSendEVT[26];
    //ocrGuid_t haloRecvEVT[26];

    ocrGuid_t positionEVTs[27]; //Make 27 copies of the local position completion
    ocrGuid_t redistributeEVTs[27];

    ocrGuid_t nbr_positionEVTs[27]; //Capture neighbors' events
    ocrGuid_t nbr_redistributeEVTs[27];

    //ocrGuid_t nbr_linkCellGuids[27];
    //ocrGuid_t nbr_atomDataGuids[27];

    ocrGuid_t finalOnceEVT;
    u32 grid[3];
    real_t domain[3];
} privateBlock_t;

extern double wtime();

void getPartitionID(s64 i, s64 lb_g, s64 ub_g, s64 R, s64* id)
{
    s64 N = ub_g - lb_g + 1;
    s64 s, e;

    s64 r;

    for( r = 0; r < R; r++ )
    {
        s = r*N/R + lb_g;
        e = (r+1)*N/R + lb_g - 1;
        if( s <= i && i <= e )
            break;
    }

    *id = r;
}

void splitDimension(s64 Num_procs, s64* Num_procsx, s64* Num_procsy, s64* Num_procsz)
{
    s64 nx, ny, nz;

    nz = (int) pow(Num_procs+1,0.33);
    for(; nz>0; nz--)
    {
        if (!(Num_procs%nz))
        {
            ny = Num_procs/nz;
            break;
        }
    }
    *Num_procsz = nz;

    Num_procs = Num_procs/nz;

    ny = (int) sqrt(Num_procs+1);
    for(; ny>0; ny--)
    {
        if (!(Num_procs%nz))
        {
            nx = Num_procs/ny;
            break;
        }
    }

    *Num_procsy = ny;

    *Num_procsx = Num_procs/(*Num_procsy);
}

u8 init_simulation( command_t* cmd_p, sharedBlock_t* sbPTR )
{
    sbPTR->step = 0;
    sbPTR->steps = cmd_p->steps;
    sbPTR->period = cmd_p->period;
    sbPTR->dt = cmd_p->dt;
    sbPTR->e_potential = 0.0;
    sbPTR->e_kinetic = 0.0;

    u8 insane = 0;
    if( cmd_p->doeam )
      insane = init_eam( cmd_p->pot_dir, cmd_p->pot_name, cmd_p->pot_type, &sbPTR->pot, sbPTR->dt );
    else
      init_lj( &sbPTR->pot, sbPTR->dt );
    //if( insane ) return insane;

    real_t lattice_const = cmd_p->lat < 0 ? sbPTR->pot.lat : cmd_p->lat;
    double minx, miny, minz;
    double sizex = cmd_p->nx*lattice_const;
    double sizey = cmd_p->ny*lattice_const;
    double sizez = cmd_p->nz*lattice_const;

    minx = miny = minz = 2*sbPTR->pot.cutoff;
    if( sizex < minx || sizey < miny || sizez < minz )
    {
        insane |= 1;
        PRINTF( "\nSimulation too small.\n"
               "  Increase the number of unit cells to make the simulation\n"
               "  at least (%3.2f, %3.2f. %3.2f) Ansgstroms in size\n", minx, miny, minz );
    }
    if( sbPTR->pot.lattice_type!=FCC )
    {
        insane |= 2;
        PRINTF( "\nOnly FCC Lattice type supported. Fatal Error.\n");
    }
    if( insane ) return insane;

    sbPTR->boxDataStH.lattice[0] = cmd_p->nx;
    sbPTR->boxDataStH.lattice[1] = cmd_p->ny;
    sbPTR->boxDataStH.lattice[2] = cmd_p->nz;

    sbPTR->boxDataStH.domain[0] = cmd_p->nx*lattice_const;
    sbPTR->boxDataStH.domain[1] = cmd_p->ny*lattice_const;
    sbPTR->boxDataStH.domain[2] = cmd_p->nz*lattice_const;
    sbPTR->boxDataStH.grid[0] = sbPTR->boxDataStH.domain[0]/sbPTR->pot.cutoff;
    sbPTR->boxDataStH.grid[1] = sbPTR->boxDataStH.domain[1]/sbPTR->pot.cutoff;
    sbPTR->boxDataStH.grid[2] = sbPTR->boxDataStH.domain[2]/sbPTR->pot.cutoff;
    sbPTR->boxDataStH.b_num = sbPTR->boxDataStH.grid[0]*sbPTR->boxDataStH.grid[1]*sbPTR->boxDataStH.grid[2];
    sbPTR->boxDataStH.b_size[0] = sbPTR->boxDataStH.domain[0]/sbPTR->boxDataStH.grid[0];
    sbPTR->boxDataStH.b_size[1] = sbPTR->boxDataStH.domain[1]/sbPTR->boxDataStH.grid[1];
    sbPTR->boxDataStH.b_size[2] = sbPTR->boxDataStH.domain[2]/sbPTR->boxDataStH.grid[2];
    sbPTR->boxDataStH.inv_b_size[0] = 1/sbPTR->boxDataStH.b_size[0];
    sbPTR->boxDataStH.inv_b_size[1] = 1/sbPTR->boxDataStH.b_size[1];
    sbPTR->boxDataStH.inv_b_size[2] = 1/sbPTR->boxDataStH.b_size[2];

    sbPTR->nAtoms = 4*(cmd_p->nx*cmd_p->ny*cmd_p->nz); //TODO: Hard-coded to copper BCC linkcells

    PRINTF( "Simulation data: \n");
    PRINTF( "  Total atoms        : %d\n", sbPTR->nAtoms );
    PRINTF( "  Min bounds  : [ %14.10f, %14.10f, %14.10f ]\n", 0.0,0.0,0.0 );
    PRINTF( "  Max bounds  : [ %14.10f, %14.10f, %14.10f ]\n\n",
           sbPTR->boxDataStH.domain[0], sbPTR->boxDataStH.domain[1], sbPTR->boxDataStH.domain[2] );
    PRINTF( "  Boxes        : %6d,%6d,%6d = %8d\n",
           sbPTR->boxDataStH.grid[0], sbPTR->boxDataStH.grid[1], sbPTR->boxDataStH.grid[2], sbPTR->boxDataStH.b_num );
    PRINTF( "  Box size           : [ %14.10f, %14.10f, %14.10f ]\n",
           sbPTR->boxDataStH.b_size[0], sbPTR->boxDataStH.b_size[1], sbPTR->boxDataStH.b_size[2] );
    PRINTF( "  Box factor         : [ %14.10f, %14.10f, %14.10f ] \n",
           sbPTR->boxDataStH.b_size[0]/sbPTR->pot.cutoff,
           sbPTR->boxDataStH.b_size[1]/sbPTR->pot.cutoff,
           sbPTR->boxDataStH.b_size[2]/sbPTR->pot.cutoff );
    //PRINTF( "  Max Link Cell Occupancy: %d of %d\n\n", sbPTR->boxDataStH.max_occupancy, MAXATOMS );
    PRINTF( "\nPotential data: \n");
    sbPTR->pot.print( &sbPTR->pot );

    // Memory footprint diagnostics
    int perAtomSize = 10*sizeof(real_t)+2*sizeof(int);
    float atomMemGlobal = (float)(perAtomSize*sbPTR->nAtoms)/1024/1024;

    int nTotalBoxes = sbPTR->boxDataStH.b_num;
    float linkCellMemTotal = (float) nTotalBoxes*(perAtomSize*MAXATOMS)/1024/1024;

    PRINTF( "\n" );
    PRINTF("Memory data: \n");
    PRINTF( "  Intrinsic atom footprint = %4d B/atom \n", perAtomSize);
    PRINTF( "  Total atom footprint     = %7.3f MB\n", atomMemGlobal);
    PRINTF( "  Link cell atom footprint = %7.3f MB\n\n", linkCellMemTotal);

    ocrGuidRangeCreate(&(sbPTR->haloRangeGUID), 27*sbPTR->boxDataStH.b_num, GUID_USER_EVENT_STICKY);
    ocrGuidRangeCreate(&(sbPTR->vcmReductionRangeGUID), sbPTR->boxDataStH.b_num, GUID_USER_EVENT_STICKY);
    ocrGuidRangeCreate(&(sbPTR->KeReductionRangeGUID), sbPTR->boxDataStH.b_num, GUID_USER_EVENT_STICKY);

    //ocrGuidRangeCreate(&(sbPTR->positionRangeGuid), sbPTR->boxDataStH.b_num, GUID_USER_EVENT_STICKY);
    //ocrGuidRangeCreate(&(sbPTR->redistributeRangeGuid), sbPTR->boxDataStH.b_num, GUID_USER_EVENT_STICKY);

    return 0;
}

//indices of vectors (arbitrary order)

void bomb(char * s) {
PRINTF("ERROR %s TERMINATING\n", s);
ocrShutdown();
return;
}

real_t lcg61( u64* seed )
{
  //*seed *= 437799614237992725;
  //*seed %= 2305843009213693951;
  //return *seed*(1.0/2305843009213693951);
   double convertToDouble = 1.0/UINT64_C(2305843009213693951);

   *seed *= UINT64_C(437799614237992725);
   *seed %= UINT64_C(2305843009213693951);

   return *seed*convertToDouble;
}

real_t gasdev( u64* seed )
{
  real_t rsq,v1,v2;
  do {
     v1 = 2.0*lcg61(seed)-1.0;
     v2 = 2.0*lcg61(seed)-1.0;
     rsq = v1*v1+v2*v2;
  } while (rsq >= 1.0 || rsq == 0.0 );

  return v2 * sqrt(-2.0*log(rsq)/rsq);
}

u64 mk_seed( u32 id, u32 callSite )
{
  //u64 s1 = (i*2654435761)&0xFFFFFFFF;
  //u64 s2 = ((i+c)*2654435761)&0xFFFFFFFF;
  //u64 seed = (s1<<32)+s2;
  //lcg61(&seed); lcg61(&seed);
  //lcg61(&seed); lcg61(&seed);
  //lcg61(&seed); lcg61(&seed);
  //lcg61(&seed); lcg61(&seed);
  //lcg61(&seed); lcg61(&seed);
  //return seed;
   uint32_t s1 = id * UINT32_C(2654435761);
   uint32_t s2 = (id+callSite) * UINT32_C(2654435761);

   uint64_t iSeed = (UINT64_C(0x100000000) * s1) + s2;
   for (unsigned jj=0; jj<10; ++jj)
      lcg61(&iSeed);
   return iSeed;
}

void putAtomInBox(atomData_t* atoms,
                  const int gid, const int iType,
                  const real_t x,  const real_t y,  const real_t z,
                  const real_t px, const real_t py, const real_t pz)
{
   int iOff = atoms->nAtoms;
   atoms->gid[iOff] = gid;
   atoms->iSpecies[iOff] = iType;

   atoms->r[iOff][0] = x;
   atoms->r[iOff][1] = y;
   atoms->r[iOff][2] = z;

   atoms->p[iOff][0] = px;
   atoms->p[iOff][1] = py;
   atoms->p[iOff][2] = pz;

   //PRINTF("%d %f %f %f\n", gid, x, y, z);

   ++atoms->nAtoms;
}

/// Creates atom positions on a face centered cubic (FCC) lattice with
/// nx * ny * nz unit cells and lattice constant lat.
/// Set momenta to zero.
void createFccLattice( int nx, int ny, int nz, real_t lat, SimFlat_t* s )
{
   const real_t* localMin = s->boxes->min; // alias
   const real_t* localMax = s->boxes->max; // alias

   int nb = 4; // number of atoms in the basis
   real3_t basis[4] = { {0.25, 0.25, 0.25},
      {0.25, 0.75, 0.75},
      {0.75, 0.25, 0.75},
      {0.75, 0.75, 0.25} };

   // create and place atoms
   int begin[3];
   int end[3];
   for (int ii=0; ii<3; ++ii)
   {
      begin[ii] = floor(localMin[ii]/lat)-1;
      end[ii]   = ceil (localMax[ii]/lat)+1;
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
               //PRINTF("id %d ix %d %d %d %d %d %d\n", id, ix, iy, iz, nx, ny, nz);
               putAtomInBox(s->atoms, id, 0, rx, ry, rz, px, py, pz);
               //PRINTF("id %d ix %f %f %f\n", id, rx, ry, rz );
            }

   // set total atoms in simulation
   //addIntParallel(&s->atoms->nLocal, &s->atoms->nGlobal, 1);
   //reduce PTR_atomData->atoms

   //ASSERT(PTR_atomData->nGlobal == nb*nx*ny*nz);
}

void setMomentum(SimFlat_t* s, real_t temperature)
{
    // set initial velocities for the distribution
    for (int iOff=0, ii=0; ii<s->atoms->nAtoms; ++ii, ++iOff)
    {
       int iType = s->atoms->iSpecies[iOff];
       real_t mass = s->species->mass[iType];
       real_t sigma = sqrt(kB_eV * temperature/mass);
       uint64_t seed = mk_seed(s->atoms->gid[iOff], 123);

       s->atoms->p[iOff][0] = 1.0;//mass * sigma;// * gasdev(&seed); //TODO
       s->atoms->p[iOff][1] = 1.0;//mass * sigma;// * gasdev(&seed);
       s->atoms->p[iOff][2] = 1.0;//mass * sigma;// * gasdev(&seed);

       PRINTF("Mom HERE %f %f %f %f\n", s->atoms->p[iOff][0],s->atoms->p[iOff][1],s->atoms->p[iOff][2], mass);
    }
}

/// Computes the center of mass velocity of the system.
void computeLocalVcm(SimFlat_t* s, real_t* vcmLocal)
{
   // sum the momenta and particle masses
    for (int iOff=0, ii=0; ii<s->atoms->nAtoms; ++ii, ++iOff)
    {
         vcmLocal[0] += s->atoms->p[iOff][0];
         vcmLocal[1] += s->atoms->p[iOff][1];
         vcmLocal[2] += s->atoms->p[iOff][2];

         int iSpecies = s->atoms->iSpecies[iOff];
         vcmLocal[3] += s->species->mass[iSpecies];

        //PRINTF("Local vcm iS %d %f %f %f %f\n", iSpecies, s->atoms->p[iOff][0], s->atoms->p[iOff][1], s->atoms->p[iOff][2], s->species->mass[iSpecies]);
    }
        PRINTF("Local vcm iS %f %f %f %f\n", vcmLocal[0], vcmLocal[1], vcmLocal[2], vcmLocal[3] );

    //vcmLocal[0] = vcmLocal[1] = vcmLocal[2] =0.0;
    //vcmLocal[3] = 1.0; //TODO

}

/// Add a random displacement to the atom positions.
/// Atoms are displaced by a random distance in the range
/// [-delta, +delta] along each axis.
/// \param [in] delta The maximum displacement (along each axis).
void randomDisplacements(atomData_t* atoms, real_t delta)
{
    for (int iOff=0, ii=0; ii<atoms->nAtoms; ++ii, ++iOff)
    {
       uint64_t seed = mk_seed(atoms->gid[iOff], 457);

       atoms->r[iOff][0] += (2.0*lcg61(&seed)-1.0) * delta;
       atoms->r[iOff][1] += (2.0*lcg61(&seed)-1.0) * delta;
       atoms->r[iOff][2] += (2.0*lcg61(&seed)-1.0) * delta;

       //PRINTF("r %f %f %f\n", atoms->r[iOff][0], atoms->r[iOff][1],atoms->r[iOff][2] );
    }
}


void initLinkCells( int iBox, linkCellH_t* linkCellH_p, privateBlock_t* pbPTR, sharedBlock_t* sbPTR )
{
//#ifndef TG_ARCH
//    memset( linkCellH_p,0,sizeof(linkCellH_t ));
//#else
//    for( u32 m=0; m<sizeof(linkCellH_t); ++m ) ((char*) linkCellH_p)[m]=0;
//#endif

    u32 gx = pbPTR->grid[0];
    u32 gy = pbPTR->grid[1];
    u32 gz = pbPTR->grid[2];

    int iBox0 = iBox;

    u32 ix, iy, iz;
    ix = iBox % gx;
    iBox /= gx;
    iy = iBox % gy;
    iz = iBox / gy;

    linkCellH_p->coordinates[0] = ix;
    linkCellH_p->coordinates[1] = iy;
    linkCellH_p->coordinates[2] = iz;

    //PRINTF("iBox %d: %d %d %d\n", iBox0, ix, iy, iz);

    linkCellH_p->min[0] = linkCellH_p->coordinates[0]*sbPTR->boxDataStH.b_size[0];
    linkCellH_p->min[1] = linkCellH_p->coordinates[1]*sbPTR->boxDataStH.b_size[1];
    linkCellH_p->min[2] = linkCellH_p->coordinates[2]*sbPTR->boxDataStH.b_size[2];
    linkCellH_p->max[0] = linkCellH_p->min[0] + sbPTR->boxDataStH.b_size[0];
    linkCellH_p->max[1] = linkCellH_p->min[1] + sbPTR->boxDataStH.b_size[1];
    linkCellH_p->max[2] = linkCellH_p->min[2] + sbPTR->boxDataStH.b_size[2];

}

void initAtoms(linkCellH_t* linkCellH_p, atomData_t* PTR_atomData, sharedBlock_t* sbPTR)
{
#ifndef TG_ARCH
    memset(PTR_atomData,0,sizeof(atomData_t));
#else
    for( u32 m=0; m<sizeof(atomData_t); ++m ) ((char*) PTR_atomData)[m]=0;
#endif
    if( linkCellH_p->coordinates[0]==0 ) PTR_atomData->nmask |= 1;
    if( linkCellH_p->coordinates[0]==sbPTR->boxDataStH.grid[0]-1 ) PTR_atomData->nmask |= 2;
    if( linkCellH_p->coordinates[1]==0 ) PTR_atomData->nmask |=4;
    if( linkCellH_p->coordinates[1]==sbPTR->boxDataStH.grid[1]-1 ) PTR_atomData->nmask |= 8;
    if( linkCellH_p->coordinates[2]==0 ) PTR_atomData->nmask |=16;
    if( linkCellH_p->coordinates[2]==sbPTR->boxDataStH.grid[2]-1 ) PTR_atomData->nmask |= 32;
}

extern ocrGuid_t ljforce_edt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t setVcm(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_linkCell = depv[0].guid;
    ocrGuid_t DBK_atomData = depv[1].guid;
    ocrGuid_t DBK_mass = depv[2].guid;
    ocrGuid_t DBK_vcmSum = depv[3].guid;

    linkCellH_t* PTR_linkCell = depv[0].ptr;
    atomData_t* PTR_atomData = depv[1].ptr;
    mass_t* mass_p = depv[2].ptr;
    real_t* vcmSum = depv[3].ptr;

    SimFlat_t s;
    s.atoms = PTR_atomData;
    s.boxes = PTR_linkCell;
    s.species = mass_p;

    real_t oldVcm[3];

    real_t totalMass = vcmSum[3];
    oldVcm[0] = vcmSum[0]/totalMass;
    oldVcm[1] = vcmSum[1]/totalMass;
    oldVcm[2] = vcmSum[2]/totalMass;

    PRINTF("vcmSum %f %f %f %f\n", vcmSum[0], vcmSum[1], vcmSum[2], vcmSum[3]); //TODO
    //PRINTF("vcmSum %f %f %f\n", oldVcm[0], oldVcm[1], oldVcm[2]);

    real_t vZero[3] = {0., 0., 0.};
    real_t* newVcm = vZero;

    real_t vShift[3];
    vShift[0] = (newVcm[0] - oldVcm[0]);
    vShift[1] = (newVcm[1] - oldVcm[1]);
    vShift[2] = (newVcm[2] - oldVcm[2]);

    for (int iOff=0, ii=0; ii<s.atoms->nAtoms; ++ii, ++iOff)
    {
       int iSpecies = s.atoms->iSpecies[iOff];
       real_t mass = s.species->mass[iSpecies];

       s.atoms->p[iOff][0] += mass * vShift[0];
       s.atoms->p[iOff][1] += mass * vShift[1];
       s.atoms->p[iOff][2] += mass * vShift[2];
    }

    //PRINTF("HELLO\n");

   return NULL_GUID;
}

void kineticEnergy( SimFlat_t* s, real_t* eLocal )
{
    eLocal[0] = 0; //s->ePotential;
    eLocal[1] = 0;

    for (int iOff=0,ii=0; ii<s->atoms->nAtoms; ii++,iOff++)
    {
       int iSpecies = s->atoms->iSpecies[iOff];
       real_t invMass_2 = s->species->inv_mass_2[iSpecies];

       eLocal[0] += s->atoms->u[iOff];
       eLocal[1] += ( s->atoms->p[iOff][0] * s->atoms->p[iOff][0] +
       s->atoms->p[iOff][1] * s->atoms->p[iOff][1] +
       s->atoms->p[iOff][2] * s->atoms->p[iOff][2] )*invMass_2;
    }

}


/// Calculates total kinetic and potential energy across all tasks.  The
/// local potential energy is a by-product of the force routine.
ocrGuid_t kineticEnergyEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    real_t epsilon = 0.167*4;//TODO

    ocrGuid_t DBK_linkCell = depv[0].guid;
    ocrGuid_t DBK_atomData = depv[1].guid;
    ocrGuid_t DBK_mass = depv[2].guid;
    ocrGuid_t rpKeDBK = depv[3].guid;

    linkCellH_t* PTR_linkCell = depv[0].ptr;
    atomData_t* PTR_atomData = depv[1].ptr;
    mass_t* mass_p = depv[2].ptr;
    reductionPrivate_t* rpKePTR = depv[3].ptr;

    SimFlat_t s;
    s.atoms = PTR_atomData;
    s.boxes = PTR_linkCell;
    s.species = mass_p;

    real_t eLocal[3]; //TODO
    kineticEnergy(&s, eLocal);
    eLocal[0] *= epsilon;
    eLocal[2] = PTR_atomData->nAtoms;

    reductionLaunch(rpKePTR, rpKeDBK, eLocal);

    return NULL_GUID; //TODO: rpKePTR->returnEVT;

    //real_t eSum[2];

    //s->ePotential = eSum[0];
    //s->eKinetic = eSum[1];
}

typedef struct
{
    real_t temperature;
    int nGlobal;
} adjustTemperatureEdtParamv_t;

typedef struct
{
    real_t delta;
} randomDisplacementsEdtParamv_t;

void adjustTemperature(SimFlat_t* s, real_t* eGlobalPTR, int nGlobal, real_t temperature )
{
    real_t eKinetic = eGlobalPTR[1];

    real_t temp = (eKinetic/nGlobal)/kB_eV/1.5;
    // scale the velocities to achieve the target temperature
    real_t scaleFactor = sqrt(temperature/temp);

    for (int iOff=0,ii=0; ii<s->atoms->nAtoms; ii++,iOff++)
    {
       s->atoms->p[iOff][0] *= scaleFactor;
       s->atoms->p[iOff][1] *= scaleFactor;
       s->atoms->p[iOff][2] *= scaleFactor;
    }
}

ocrGuid_t adjustTemperatureEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_linkCell = depv[0].guid;
    ocrGuid_t DBK_atomData = depv[1].guid;
    ocrGuid_t DBK_mass = depv[2].guid;
    ocrGuid_t eGlobalDBK = depv[3].guid;

    linkCellH_t* PTR_linkCell = depv[0].ptr;
    atomData_t* PTR_atomData = depv[1].ptr;
    mass_t* mass_p = depv[2].ptr;
    real_t* eGlobalPTR = depv[3].ptr;

    adjustTemperatureEdtParamv_t* adjustTemperatureEdtParamvPTR = (adjustTemperatureEdtParamv_t*) paramv;
    real_t temperature = adjustTemperatureEdtParamvPTR->temperature;
    int nGlobal = adjustTemperatureEdtParamvPTR->nGlobal;

    //PRINTF("ePot %f eKin %f atoms %d\n", eGlobalPTR[0], eGlobalPTR[1], (int)eGlobalPTR[2]);

    SimFlat_t s;
    s.atoms = PTR_atomData;
    s.boxes = PTR_linkCell;
    s.species = mass_p;

    adjustTemperature(&s, eGlobalPTR, nGlobal, temperature);

    return NULL_GUID;
}

ocrGuid_t randomDisplacementsEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_atomData = depv[0].guid;

    atomData_t* PTR_atomData = depv[0].ptr;

    randomDisplacementsEdtParamv_t* randomDisplacementsEdtParamvPTR = (randomDisplacementsEdtParamv_t*) paramv;
    real_t delta = randomDisplacementsEdtParamvPTR->delta;

    randomDisplacements( PTR_atomData, delta );

    return NULL_GUID;
}

static inline void copy_atom(u8 s, u8 d, atomData_t* sPTR_atomData, atomData_t* dPTR_atomData, linkCellH_t* sPTR_linkCell, linkCellH_t* dPTR_linkCell)
{
  memcpy(dPTR_atomData->r[d],sPTR_atomData->r[s],sizeof(real3_t));
  memcpy(dPTR_atomData->p[d],sPTR_atomData->p[s],sizeof(real3_t));
  memcpy(dPTR_atomData->f[d],sPTR_atomData->f[s],sizeof(real3_t));
  //dPTR_linkCell->gid[d]=sPTR_linkCell->gid[s]; dPTR_atomData->s[d]=sPTR_atomData->s[s];
  dPTR_atomData->gid[d]=sPTR_atomData->gid[s]; dPTR_atomData->iSpecies[d]=sPTR_atomData->iSpecies[s];
}

static inline void move_atom(u8 s, atomData_t* sPTR_atomData, atomData_t* dPTR_atomData, linkCellH_t* sPTR_linkCell, linkCellH_t* dPTR_linkCell)
{
  copy_atom( s, dPTR_atomData->nAtoms, sPTR_atomData, dPTR_atomData, sPTR_linkCell, dPTR_linkCell );
  if( --sPTR_atomData->nAtoms && s!=sPTR_atomData->nAtoms )
    copy_atom( sPTR_atomData->nAtoms, s, sPTR_atomData, sPTR_atomData, sPTR_linkCell, sPTR_linkCell );
  if( ++dPTR_atomData->nAtoms > dPTR_linkCell->max_occupancy )
    dPTR_linkCell->max_occupancy = dPTR_atomData->nAtoms;

  ASSERT(dPTR_atomData->nAtoms<=MAXATOMS);
}

//depv: DBK_atomDataH0, DBK_linkCellH0, DBK_atomDataH1, DBK_linkCellH1
ocrGuid_t move_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  atomData_t* PTR_atomData0 = (atomData_t*)depv[0].ptr;
  linkCellH_t* PTR_linkCell0 = (linkCellH_t*)depv[1].ptr;
  atomData_t* PTR_atomData1 = (atomData_t*)depv[2].ptr;
  linkCellH_t* PTR_linkCell1 = (linkCellH_t*)depv[3].ptr;
  u8 a;
    for(a=0; a<PTR_atomData0->nAtoms; ++a)
    {
        if(PTR_atomData0->r[a][0] <  PTR_linkCell1->max[0] &&
           PTR_atomData0->r[a][0] >= PTR_linkCell1->min[0] &&
           PTR_atomData0->r[a][1] <  PTR_linkCell1->max[1] &&
           PTR_atomData0->r[a][1] >= PTR_linkCell1->min[1] &&
           PTR_atomData0->r[a][2] <  PTR_linkCell1->max[2] &&
           PTR_atomData0->r[a][2] >= PTR_linkCell1->min[2])
        {
            move_atom(a, PTR_atomData0, PTR_atomData1, PTR_linkCell0, PTR_linkCell1);
            PRINTF("Moving atom %d\n", PTR_atomData0->gid[a]);
        }
    }

  return NULL_GUID;
}

//depv: DBK_atomDataH0, DBK_linkCellH0
ocrGuid_t redistributeAtomsEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("redistributeAtomsEdt\n");
    ocrGuid_t PDaffinityGuid = NULL_GUID;
    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&PDaffinityGuid);
    ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif

    ocrGuid_t DBK_linkCellH = depv[0].guid;
    ocrGuid_t DBK_atomDataH = depv[1].guid;

    linkCellH_t* PTR_linkCell = (linkCellH_t*)depv[0].ptr;
    atomData_t* PTR_atomData = (atomData_t*)depv[1].ptr;

    u32 x=0;
    u8 a,f;
    for(a=0; a<PTR_atomData->nAtoms; ++a)
    {
        f=0;

        if(PTR_atomData->r[a][2] >= PTR_linkCell->max[2])
          f += 18;
        else if(PTR_atomData->r[a][2] >= PTR_linkCell->min[2])
          f += 9;
        if(PTR_atomData->r[a][1] >= PTR_linkCell->max[1])
          f += 6;
        else if(PTR_atomData->r[a][1] >= PTR_linkCell->min[1])
          f += 3;
        if(PTR_atomData->r[a][0] >= PTR_linkCell->max[0])
          f += 2;
        else if(PTR_atomData->r[a][0] >= PTR_linkCell->min[0])
          f += 1;

        if(f!=13)
        {
          //if(f>13) --f;
          x |= (1<<f);
        }
    }

    ocrGuid_t* nbr_atomDataGuids = PTR_linkCell->nbr_atomDataGuids;
    ocrGuid_t* nbr_linkCellGuids = PTR_linkCell->nbr_linkCellGuids;

    ocrGuid_t tmp, edtg;
    ocrEdtTemplateCreate(&tmp, move_edt, 0, 4);
    for(f=0; f<27; ++f)
    {
        if(f==13) continue;

        if((1<<f)&x)
        {
            //PRINTF("HERE %d\n", f);
            ocrEdtCreate(&edtg, tmp, 0, NULL, 4, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), NULL);
            ocrAddDependence(DBK_atomDataH, edtg, 0, DB_MODE_EW);
            ocrAddDependence(DBK_linkCellH, edtg, 1, DB_MODE_EW);
            ocrAddDependence(nbr_atomDataGuids[f], edtg, 2, DB_MODE_EW);
            ocrAddDependence(nbr_linkCellGuids[f], edtg, 3, DB_MODE_EW);
        }
    }

    ocrEdtTemplateDestroy(tmp);

    return NULL_GUID;
}

ocrGuid_t printThingsEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    s32 itimestep = paramv[0];

    real_t* eGlobalPTR = depv[0].ptr;

    int nGlobal = (int) eGlobalPTR[2];
    real_t e_u = eGlobalPTR[0]/nGlobal;
    real_t e_k = eGlobalPTR[1]/nGlobal;

    real_t energy = (e_u + e_k);
    real_t temp = e_k / kB_eV_1_5;

    PRINTF("STATUS: %7d %18.12f %18.12f %18.12f %12.5f %d\n", itimestep, energy, e_u, e_k, temp, nGlobal);

    return NULL_GUID;
}

void advanceVelocity(atomData_t* atoms, real_t dt)
{
    for (int iOff=0, ii=0; ii<atoms->nAtoms; ++ii, ++iOff)
    {
         atoms->p[iOff][0] += dt*atoms->f[iOff][0];
         atoms->p[iOff][1] += dt*atoms->f[iOff][1];
         atoms->p[iOff][2] += dt*atoms->f[iOff][2];

         //PRINTF("%f p %f %f %f f %f %f %f\n", dt, atoms->p[iOff][0], atoms->p[iOff][1],atoms->p[iOff][2],
         //atoms->f[iOff][0],atoms->f[iOff][1],atoms->f[iOff][2] );
    }
}

ocrGuid_t advanceVelocityEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_atomData = depv[0].guid;

    atomData_t* PTR_atomData = depv[0].ptr;

    real_t dt = ((real_t*) paramv)[0];

    advanceVelocity( PTR_atomData, dt );

    return NULL_GUID;
}

ocrGuid_t advancePositionEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_atomData = depv[0].guid;
    ocrGuid_t DBK_mass = depv[1].guid;

    atomData_t* PTR_atomData = depv[0].ptr;
    mass_t* mass_p = depv[1].ptr;

    real_t dt = ((real_t*) paramv)[0];

    SimFlat_t s;
    s.atoms = PTR_atomData;
    s.species = mass_p;

    for (int iOff=0, ii=0; ii<s.atoms->nAtoms; ++ii, ++iOff)
    {
       int iSpecies = s.atoms->iSpecies[iOff];
       real_t invmass = s.species->inv_mass[iSpecies];

       s.atoms->r[iOff][0] += invmass * dt * s.atoms->p[iOff][0];
       s.atoms->r[iOff][1] += invmass * dt * s.atoms->p[iOff][1];
       s.atoms->r[iOff][2] += invmass * dt * s.atoms->p[iOff][2];

       //PRINTF("r1 t %f im %f r %f %f %f\n", dt, invmass, s.atoms->r[iOff][0], s.atoms->r[iOff][1],s.atoms->r[iOff][2] );
    }

    return NULL_GUID;
}

ocrGuid_t timestepEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    s32 itimestep = paramv[0];

#if 1
    ocrGuid_t cmd_g = depv[0].guid;
    ocrGuid_t sbDBK = depv[1].guid;
    ocrGuid_t pbDBK = depv[2].guid;
    ocrGuid_t rpKeDBK = depv[3].guid;
    ocrGuid_t DBK_linkCell = depv[4].guid;
    ocrGuid_t DBK_atomData = depv[5].guid;
    ocrGuid_t DBK_mass = depv[6].guid;

    command_t* cmd_p = depv[0].ptr;
    sharedBlock_t* sbPTR = depv[1].ptr;
    privateBlock_t* pbPTR = depv[2].ptr;
    reductionPrivate_t* rpKePTR = depv[3].ptr;
    linkCellH_t* PTR_linkCell = depv[4].ptr;
    atomData_t* PTR_atomData = depv[5].ptr;
    mass_t* mass_p = depv[6].ptr;

    u32 iBox = pbPTR->myrank;
    //PRINTF("iBox %d t %d\n", iBox, itimestep);

    rpKePTR->all = 0;

    ocrGuid_t rpKeEVT = rpKePTR->returnEVT;

    ocrDbRelease(rpKeDBK);

    ocrGuid_t nbr_atomDataGuids[27];
    u32 _inbr;
    for( _inbr = 0; _inbr <= 26; ++_inbr )
    {
        nbr_atomDataGuids[_inbr] = PTR_linkCell->nbr_atomDataGuids[_inbr];
        //PRINTF("H %d, nbr_atomDataGuids %lx %p\n", _inbr, nbr_atomDataGuids[_inbr], PTR_linkCell->nbr_atomDataGuids);
    }

    ocrGuid_t advanceVelocityTML, advanceVelocityEDT, advanceVelocityOEVT, advanceVelocityOEVTS;

    real_t dt = 0.5*cmd_p->dt; //TODO

    ocrEdtTemplateCreate( &advanceVelocityTML, advanceVelocityEdt, 1, 1 );

    ocrEdtCreate( &advanceVelocityEDT, advanceVelocityTML,
                  EDT_PARAM_DEF, (u64*) &dt, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &pbPTR->myAffinityHNT, &advanceVelocityOEVT );

    ocrEventCreate( &advanceVelocityOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( advanceVelocityOEVT, advanceVelocityOEVTS, 0, DB_MODE_NULL );

    s32 _idep = 0;
    ocrAddDependence( DBK_atomData, advanceVelocityEDT, _idep++, DB_MODE_RW );

    ocrGuid_t advancePositionTML, advancePositionEDT, advancePositionOEVT, advancePositionOEVTS;

    dt = 1.0*cmd_p->dt; //TODO

    ocrEdtTemplateCreate( &advancePositionTML, advancePositionEdt, 1, 3 );

    ocrEdtCreate( &advancePositionEDT, advancePositionTML,
                  EDT_PARAM_DEF, (u64*) &dt, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &pbPTR->myAffinityHNT, &advancePositionOEVT );

    ocrEventCreate( &advancePositionOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( advancePositionOEVT, advancePositionOEVTS, 0, DB_MODE_NULL );

    for( _idep=0; _idep <= 26; _idep++ )
        ocrAddDependence( advancePositionOEVTS, pbPTR->positionEVTs[_idep], 0, DB_MODE_NULL ); //Notify neighbors

    _idep = 0;
    ocrAddDependence( DBK_atomData, advancePositionEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_mass, advancePositionEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( advanceVelocityOEVTS, advancePositionEDT, _idep++, DB_MODE_NULL );

    //redistribute atoms

    ocrGuid_t redistributeAtomsTML, redistributeAtomsEDT, redistributeAtomsOEVT, redistributeAtomsOEVTS;

    //redistributeAtomsEdtParamv_t redistributeAtomsEdtParamv;
    //redistributeAtomsEdtParamv.delta = cmd_p->delta;

    ocrEdtTemplateCreate( &redistributeAtomsTML, redistributeAtomsEdt, 0, 29 );

    ocrEdtCreate( &redistributeAtomsEDT, redistributeAtomsTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &pbPTR->myAffinityHNT, &redistributeAtomsOEVT );

    ocrEventCreate( &redistributeAtomsOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( redistributeAtomsOEVT, redistributeAtomsOEVTS, 0, DB_MODE_NULL );

    for( _idep=0; _idep <= 26; _idep++ )
        ocrAddDependence( redistributeAtomsOEVTS, pbPTR->redistributeEVTs[_idep], 0, DB_MODE_NULL ); //Notify the neighbors

    _idep = 0;
    ocrAddDependence( DBK_linkCell, redistributeAtomsEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_atomData, redistributeAtomsEDT, _idep++, DB_MODE_RO );

    for( _idep=0; _idep <= 26; _idep++ )
    {
        //PRINTF("%lx %d %d\n", pbPTR->nbr_positionEVTs[_idep], iBox, _idep);
        ocrAddDependence( pbPTR->nbr_positionEVTs[_idep], redistributeAtomsEDT, _idep+2, DB_MODE_NULL ); //depend on the neighbors
    }

    //compute force
    //
    ocrGuid_t computeForceTML, computeForceEDT, computeForceOEVT, computeForceOEVTS;

    computeForceEdtParamv_t computeForceEdtParamv;
    computeForceEdtParamv.lj.sigma = 2.315; //lj.sigma
    computeForceEdtParamv.lj.epsilon = 0.167*4; //lj.epsilon
    computeForceEdtParamv.cutoff = 2.5*2.315; //j.cutoff
    computeForceEdtParamv.domain[0] = pbPTR->domain[0];
    computeForceEdtParamv.domain[1] = pbPTR->domain[1];
    computeForceEdtParamv.domain[2] = pbPTR->domain[2];

    ocrEdtTemplateCreate( &computeForceTML, ljforce_edt, sizeof(computeForceEdtParamv_t)/sizeof(u64), 54 );

    ocrEdtCreate( &computeForceEDT, computeForceTML,
                  EDT_PARAM_DEF, (u64*)&computeForceEdtParamv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &pbPTR->myAffinityHNT, &computeForceOEVT );

    ocrEventCreate( &computeForceOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( computeForceOEVT, computeForceOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    //ocrAddDependence( DBK_atomData, computeForceEDT, _idep++, DB_MODE_RW );

    for( _inbr=0; _inbr <= 26; _inbr++ )
    {
        //PRINTF("%lx %d %d\n", nbr_atomDataGuids[_inbr], iBox, _inbr);
        ocrAddDependence( nbr_atomDataGuids[_inbr], computeForceEDT, _inbr, _inbr!=13?DB_MODE_RO:DB_MODE_RW );
        ocrAddDependence( pbPTR->nbr_redistributeEVTs[_inbr], computeForceEDT, 27+_inbr, DB_MODE_NULL ); //depend on the neighbors
    }

    ocrGuid_t advanceVelocityTML1, advanceVelocityEDT1, advanceVelocityOEVT1, advanceVelocityOEVTS1;

    dt = 0.5*cmd_p->dt; //TODO

    ocrEdtTemplateCreate( &advanceVelocityTML1, advanceVelocityEdt, 1, 2 );

    ocrEdtCreate( &advanceVelocityEDT1, advanceVelocityTML1,
                  EDT_PARAM_DEF, (u64*) &dt, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &pbPTR->myAffinityHNT, &advanceVelocityOEVT1 );

    ocrEventCreate( &advanceVelocityOEVTS1, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( advanceVelocityOEVT1, advanceVelocityOEVTS1, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_atomData, advanceVelocityEDT1, _idep++, DB_MODE_RW );
    ocrAddDependence( computeForceOEVTS, advanceVelocityEDT1, _idep++, DB_MODE_NULL );

    s32 nSteps = cmd_p->period;
    s32 ntimesteps = cmd_p->steps;

    itimestep++;

    if( itimestep%nSteps == 0 )
    {
        ////Compute Kinetic energy of the system
        ocrGuid_t kineticEnergyTML, kineticEnergyEDT1, kineticEnergyOEVT1, kineticEnergyOEVTS1;

        ocrEdtTemplateCreate( &kineticEnergyTML, kineticEnergyEdt, 0, 5 );

        ocrEdtCreate( &kineticEnergyEDT1, kineticEnergyTML,
                      EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &pbPTR->myAffinityHNT, &kineticEnergyOEVT1 );

        ocrEventCreate( &kineticEnergyOEVTS1, OCR_EVENT_STICKY_T, false );
        ocrAddDependence( kineticEnergyOEVT1, kineticEnergyOEVTS1, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_linkCell, kineticEnergyEDT1, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_atomData, kineticEnergyEDT1, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_mass, kineticEnergyEDT1, _idep++, DB_MODE_RO );
        ocrAddDependence( rpKeDBK, kineticEnergyEDT1, _idep++, DB_MODE_RW );
        ocrAddDependence( advanceVelocityOEVTS1, kineticEnergyEDT1, _idep++, DB_MODE_NULL );

        ////Print things
        if(iBox == 0)
        {
            ocrGuid_t printThingsTML, printThingsEDT, printThingsOEVT, printThingsOEVTS;

            ocrEdtTemplateCreate( &printThingsTML, printThingsEdt, 1, 2 );

            ocrEdtCreate( &printThingsEDT, printThingsTML,
                          EDT_PARAM_DEF, (u64*) &itimestep, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, &pbPTR->myAffinityHNT, &printThingsOEVT );

            ocrEventCreate( &printThingsOEVTS, OCR_EVENT_STICKY_T, false );
            ocrAddDependence( printThingsOEVT, printThingsOEVTS, 0, DB_MODE_NULL );

            _idep = 0;
            ocrAddDependence( rpKeEVT, printThingsEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( kineticEnergyOEVTS1, printThingsEDT, _idep++, DB_MODE_RW );
        }


    }

    //Trigger wrapup EVT
    if( itimestep == ntimesteps )
    {

    }

#endif

    return NULL_GUID;
}

ocrGuid_t timestepLoopEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    s32 itimestep = paramv[0];

    ocrGuid_t cmd_g = depv[0].guid;
    ocrGuid_t sbDBK = depv[1].guid;
    ocrGuid_t pbDBK = depv[2].guid;
    ocrGuid_t rpKeDBK = depv[3].guid;
    ocrGuid_t DBK_linkCell = depv[4].guid;
    ocrGuid_t DBK_atomData = depv[5].guid;
    ocrGuid_t DBK_mass = depv[6].guid;

    command_t* cmd_p = depv[0].ptr;
    sharedBlock_t* sbPTR = depv[1].ptr;
    privateBlock_t* pbPTR = depv[2].ptr;
    reductionPrivate_t* rpKePTR = depv[3].ptr;
    linkCellH_t* PTR_linkCell = depv[4].ptr;
    atomData_t* PTR_atomData = depv[5].ptr;
    mass_t* mass_p = depv[6].ptr;

    // Do one timestep
    ocrGuid_t timestepTML, timestepEDT, timestepOEVT, timestepOEVTS;

    ocrEdtTemplateCreate( &timestepTML, timestepEdt, 1, 7 );

    ocrEdtCreate( &timestepEDT, timestepTML,
                  EDT_PARAM_DEF, (u64*) &itimestep, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &pbPTR->myAffinityHNT, &timestepOEVT );

    ocrEventCreate( &timestepOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( timestepOEVT, timestepOEVTS, 0, DB_MODE_NULL );

    s32 _idep = 0;
    ocrAddDependence(cmd_g, timestepEDT, 0, DB_MODE_RO);
    ocrAddDependence(sbDBK, timestepEDT, 1, DB_MODE_RO);
    ocrAddDependence(pbDBK, timestepEDT, 2, DB_MODE_RW);
    ocrAddDependence(rpKeDBK, timestepEDT, 3, DB_MODE_RW);
    ocrAddDependence(DBK_linkCell, timestepEDT, 4, DB_MODE_RW);
    ocrAddDependence(DBK_atomData, timestepEDT, 5, DB_MODE_RW);
    ocrAddDependence(DBK_mass, timestepEDT, 6, DB_MODE_RO);

    itimestep += 1;

    s32 ntimesteps = cmd_p->steps;

    if( itimestep < ntimesteps )
    {
    //start next timestep
        ocrGuid_t timestepLoopTML, timestepLoopEDT, timestepLoopOEVT, timestepLoopOEVTS;

        ocrEdtTemplateCreate( &timestepLoopTML, timestepLoopEdt, 1, 8 );

        ocrEdtCreate( &timestepLoopEDT, timestepLoopTML,
                      EDT_PARAM_DEF, (u64*)&itimestep, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &pbPTR->myAffinityHNT, &timestepLoopOEVT );

        ocrEventCreate( &timestepLoopOEVTS, OCR_EVENT_STICKY_T, false );
        ocrAddDependence( timestepLoopOEVT, timestepLoopOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( cmd_g, timestepLoopEDT, 0, DB_MODE_RO );
        ocrAddDependence( sbDBK, timestepLoopEDT, 1, DB_MODE_RO );
        ocrAddDependence( pbDBK, timestepLoopEDT, 2, DB_MODE_RW );
        ocrAddDependence( rpKeDBK, timestepLoopEDT, 3, DB_MODE_RW );
        ocrAddDependence( DBK_linkCell, timestepLoopEDT, 4, DB_MODE_RW );
        ocrAddDependence( DBK_atomData, timestepLoopEDT, 5, DB_MODE_RW );
        ocrAddDependence( DBK_mass, timestepLoopEDT, 6, DB_MODE_RO );
        ocrAddDependence( timestepOEVTS, timestepLoopEDT, 7, DB_MODE_NULL );
    }
    else
    {
        rpKePTR->returnEVT = pbPTR->finalOnceEVT;
    }

    return NULL_GUID;
}

//comdTML
ocrGuid_t comdEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t cmd_g = depv[0].guid;
    ocrGuid_t sbDBK = depv[1].guid;
    ocrGuid_t pbDBK = depv[2].guid;
    ocrGuid_t rpVcmDBK = depv[3].guid;
    ocrGuid_t rpKeDBK = depv[4].guid;
    ocrGuid_t DBK_linkCell = depv[5].guid;
    ocrGuid_t DBK_atomData = depv[6].guid;
    ocrGuid_t DBK_mass = depv[7].guid;

    command_t* cmd_p = depv[0].ptr;
    sharedBlock_t* sbPTR = depv[1].ptr;
    privateBlock_t* pbPTR = depv[2].ptr;
    reductionPrivate_t* rpVcmPTR = depv[3].ptr;
    reductionPrivate_t* rpKePTR = depv[4].ptr;
    linkCellH_t* PTR_linkCell = depv[5].ptr;
    atomData_t* PTR_atomData = depv[6].ptr;
    mass_t* mass_p = depv[7].ptr;

    SimFlat_t s;
    s.atoms = PTR_atomData;
    s.boxes = PTR_linkCell;
    s.species = mass_p;

    u32 nx, ny, nz;
    nx = cmd_p->nx;
    ny = cmd_p->ny;
    nz = cmd_p->nz;

    real_t latticeConstant = sbPTR->pot.lat;
    int nGlobal = sbPTR->nAtoms; //TODO

    u32 iBox = pbPTR->myrank;

    initLinkCells(iBox, PTR_linkCell, pbPTR, sbPTR);
    initAtoms(PTR_linkCell, PTR_atomData, sbPTR);

    createFccLattice( nx, ny, nz, latticeConstant, &s ); //set positions
    PTR_linkCell->max_occupancy = PTR_atomData->nAtoms;

    setMomentum( &s, cmd_p->temperature ); //assign initial momentum; Must be updated later to match the input temperature

    //compute Vcm and reset the momementum
    real_t localVcm[4];
    computeLocalVcm( &s, localVcm );

    ocrGuid_t rpVcmEVT = rpVcmPTR->returnEVT;
    ocrGuid_t rpKeEVT = rpKePTR->returnEVT;

    reductionLaunch(rpVcmPTR, rpVcmDBK, localVcm);

    //ocrDbRelease(rpKeDBK); //TODO

    ocrGuid_t nbr_atomDataGuids[27];
    u32 _inbr;
    for( _inbr = 0; _inbr <= 26; ++_inbr )
    {
        nbr_atomDataGuids[_inbr] = PTR_linkCell->nbr_atomDataGuids[_inbr];
        //PRINTF("H %d, nbr_atomDataGuids %lx %p\n", _inbr, nbr_atomDataGuids[_inbr], PTR_linkCell->nbr_atomDataGuids);
    }

    //ocrDbRelease(rpVcmDBK); not needed?
    ocrDbRelease(DBK_linkCell);
    ocrDbRelease(DBK_atomData);

    ////rpVcmPTR->returnEVT //triggers reset momentum task

    ////Create setVcm task --> now the Vcm is set to zero
    //{DBK_atomData, DBK_linkCell, DBK_mass, rpVcmPTR->returnEVT}
    //
    ocrGuid_t setVcmTML, setVcmEDT, setVcmOEVT, setVcmOEVTS;

    ocrEdtTemplateCreate( &setVcmTML, setVcm, 0, 4 );

    ocrEdtCreate( &setVcmEDT, setVcmTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &pbPTR->myAffinityHNT, &setVcmOEVT );

    ocrEventCreate( &setVcmOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( setVcmOEVT, setVcmOEVTS, 0, DB_MODE_NULL );

    u32 _idep = 0;
    ocrAddDependence( DBK_linkCell, setVcmEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_atomData, setVcmEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_mass, setVcmEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( rpVcmEVT, setVcmEDT, _idep++, DB_MODE_RO );

    ////Compute Kinetic energy of the system to compute temperature
    ocrGuid_t kineticEnergyTML, kineticEnergyEDT, kineticEnergyOEVT, kineticEnergyOEVTS;

    ocrEdtTemplateCreate( &kineticEnergyTML, kineticEnergyEdt, 0, 5 );

    ocrEdtCreate( &kineticEnergyEDT, kineticEnergyTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &pbPTR->myAffinityHNT, &kineticEnergyOEVT );

    ocrEventCreate( &kineticEnergyOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( kineticEnergyOEVT, kineticEnergyOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_linkCell, kineticEnergyEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_atomData, kineticEnergyEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_mass, kineticEnergyEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( rpKeDBK, kineticEnergyEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( setVcmOEVTS, kineticEnergyEDT, _idep++, DB_MODE_RO );

    ////Adjust momentum again: adjustTemperature()

    ocrGuid_t adjustTemperatureTML, adjustTemperatureEDT, adjustTemperatureOEVT, adjustTemperatureOEVTS;

    adjustTemperatureEdtParamv_t adjustTemperatureEdtParamv;
    adjustTemperatureEdtParamv.temperature = cmd_p->temperature;
    adjustTemperatureEdtParamv.nGlobal = nGlobal;

    ocrEdtTemplateCreate( &adjustTemperatureTML, adjustTemperatureEdt, sizeof(adjustTemperatureEdtParamv_t)/sizeof(u64), 4 );

    ocrEdtCreate( &adjustTemperatureEDT, adjustTemperatureTML,
                  EDT_PARAM_DEF, (u64*) &adjustTemperatureEdtParamv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &pbPTR->myAffinityHNT, &adjustTemperatureOEVT );

    ocrEventCreate( &adjustTemperatureOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( adjustTemperatureOEVT, adjustTemperatureOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_linkCell, adjustTemperatureEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_atomData, adjustTemperatureEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_mass, adjustTemperatureEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( rpKeEVT, adjustTemperatureEDT, _idep++, DB_MODE_RO );


    //randomDisplacements( s, delta );

    ocrGuid_t randomDisplacementsTML, randomDisplacementsEDT, randomDisplacementsOEVT, randomDisplacementsOEVTS;

    randomDisplacementsEdtParamv_t randomDisplacementsEdtParamv;
    randomDisplacementsEdtParamv.delta = cmd_p->delta;

    ocrEdtTemplateCreate( &randomDisplacementsTML, randomDisplacementsEdt, sizeof(randomDisplacementsEdtParamv_t)/sizeof(u64), 2 );

    ocrEdtCreate( &randomDisplacementsEDT, randomDisplacementsTML,
                  EDT_PARAM_DEF, (u64*) &randomDisplacementsEdtParamv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &pbPTR->myAffinityHNT, &randomDisplacementsOEVT );

    ocrEventCreate( &randomDisplacementsOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( randomDisplacementsOEVT, randomDisplacementsOEVTS, 0, DB_MODE_NULL );

    for( _idep=0; _idep <= 26; _idep++ )
        ocrAddDependence( randomDisplacementsOEVTS, pbPTR->positionEVTs[_idep], 0, DB_MODE_NULL ); //Notify neighbors

    _idep = 0;
    ocrAddDependence( DBK_atomData, randomDisplacementsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( adjustTemperatureOEVTS, randomDisplacementsEDT, _idep++, DB_MODE_RO );

    //redistribute atoms

    ocrGuid_t redistributeAtomsTML, redistributeAtomsEDT, redistributeAtomsOEVT, redistributeAtomsOEVTS;

    //redistributeAtomsEdtParamv_t redistributeAtomsEdtParamv;
    //redistributeAtomsEdtParamv.delta = cmd_p->delta;

    ocrEdtTemplateCreate( &redistributeAtomsTML, redistributeAtomsEdt, 0, 29 );

    ocrEdtCreate( &redistributeAtomsEDT, redistributeAtomsTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &pbPTR->myAffinityHNT, &redistributeAtomsOEVT );

    ocrEventCreate( &redistributeAtomsOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( redistributeAtomsOEVT, redistributeAtomsOEVTS, 0, DB_MODE_NULL );

    for( _idep=0; _idep <= 26; _idep++ )
        ocrAddDependence( redistributeAtomsOEVTS, pbPTR->redistributeEVTs[_idep], 0, DB_MODE_NULL ); //Notify the neighbors

    _idep = 0;
    ocrAddDependence( DBK_linkCell, redistributeAtomsEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_atomData, redistributeAtomsEDT, _idep++, DB_MODE_RO );

    for( _idep=0; _idep <= 26; _idep++ )
    {
        //PRINTF("%lx %d %d\n", pbPTR->nbr_positionEVTs[_idep], iBox, _idep);
        ocrAddDependence( pbPTR->nbr_positionEVTs[_idep], redistributeAtomsEDT, _idep+2, DB_MODE_NULL ); //depend on the neighbors
    }

    //compute force
    //
    ocrGuid_t computeForceTML, computeForceEDT, computeForceOEVT, computeForceOEVTS;

    computeForceEdtParamv_t computeForceEdtParamv;
    computeForceEdtParamv.lj.sigma = 2.315; //lj.sigma
    computeForceEdtParamv.lj.epsilon = 0.167*4; //lj.epsilon
    computeForceEdtParamv.cutoff = 2.5*2.315; //j.cutoff
    computeForceEdtParamv.domain[0] = pbPTR->domain[0];
    computeForceEdtParamv.domain[1] = pbPTR->domain[1];
    computeForceEdtParamv.domain[2] = pbPTR->domain[2];

    ocrEdtTemplateCreate( &computeForceTML, ljforce_edt, sizeof(computeForceEdtParamv_t)/sizeof(u64), 54 );

    ocrEdtCreate( &computeForceEDT, computeForceTML,
                  EDT_PARAM_DEF, (u64*)&computeForceEdtParamv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &pbPTR->myAffinityHNT, &computeForceOEVT );

    ocrEventCreate( &computeForceOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( computeForceOEVT, computeForceOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    //ocrAddDependence( DBK_atomData, computeForceEDT, _idep++, DB_MODE_RW );

    for( _inbr=0; _inbr <= 26; _inbr++ )
    {
        //PRINTF("%lx %d %d\n", nbr_atomDataGuids[_inbr], iBox, _inbr);
        ocrAddDependence( nbr_atomDataGuids[_inbr], computeForceEDT, _inbr, _inbr!=13?DB_MODE_RO:DB_MODE_RW );
        ocrAddDependence( pbPTR->nbr_redistributeEVTs[_inbr], computeForceEDT, 27+_inbr, DB_MODE_NULL ); //depend on the neighbors
    }

    #if 1
    ////Compute Kinetic energy of the system
    ocrGuid_t kineticEnergyEDT1, kineticEnergyOEVT1, kineticEnergyOEVTS1;

    //ocrEdtTemplateCreate( &kineticEnergyTML, kineticEnergyEdt, 0, 5 );

    ocrEdtCreate( &kineticEnergyEDT1, kineticEnergyTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &pbPTR->myAffinityHNT, &kineticEnergyOEVT1 );

    ocrEventCreate( &kineticEnergyOEVTS1, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( kineticEnergyOEVT1, kineticEnergyOEVTS1, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_linkCell, kineticEnergyEDT1, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_atomData, kineticEnergyEDT1, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_mass, kineticEnergyEDT1, _idep++, DB_MODE_RO );
    ocrAddDependence( rpKeDBK, kineticEnergyEDT1, _idep++, DB_MODE_RW );
    ocrAddDependence( computeForceOEVTS, kineticEnergyEDT1, _idep++, DB_MODE_NULL );

    //sumAtoms
    //ocrGuid_t sumAtomsEDT, sumAtomsOEVT, sumAtomsOEVTS;

    //ocrEdtTemplateCreate( &sumAtomsTML, sumAtomsEdt, 0, 4 );

    //ocrEdtCreate( &sumAtomsEDT, sumAtomsTML,
    //              EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
    //              EDT_PROP_NONE, &pbPTR->myAffinityHNT, &sumAtomsOEVT );

    //ocrEventCreate( &sumAtomsOEVTS, OCR_EVENT_STICKY_T, false );
    //ocrAddDependence( sumAtomsOEVT, sumAtomsOEVTS, 0, DB_MODE_NULL );

    //_idep = 0;
    //ocrAddDependence( DBK_atomData, sumAtomsEDT, _idep++, DB_MODE_RW );
    //ocrAddDependence( rpKeEVT, sumAtomsEDT, _idep++, DB_MODE_RW );

    ////Print things
    s64 itimestep = 0;
    if(iBox == 0)
    {
    ocrGuid_t printThingsTML, printThingsEDT, printThingsOEVT, printThingsOEVTS;

    ocrEdtTemplateCreate( &printThingsTML, printThingsEdt, 1, 1 );

    ocrEdtCreate( &printThingsEDT, printThingsTML,
                  EDT_PARAM_DEF, (u64*) &itimestep, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &pbPTR->myAffinityHNT, &printThingsOEVT );

    ocrEventCreate( &printThingsOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( printThingsOEVT, printThingsOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    //ocrAddDependence( DBK_linkCell, printThingsEDT, _idep++, DB_MODE_RO );
    //ocrAddDependence( DBK_atomData, printThingsEDT, _idep++, DB_MODE_RW );
    //ocrAddDependence( DBK_mass, printThingsEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( rpKeEVT, printThingsEDT, _idep++, DB_MODE_RW );
    }

    #endif

    //start timestep loop
    #if 1
    if( cmd_p->steps > 0 )
    {
    ocrGuid_t timestepLoopTML, timestepLoopEDT, timestepLoopOEVT, timestepLoopOEVTS;

    ocrEdtTemplateCreate( &timestepLoopTML, timestepLoopEdt, 1, 8 );

    ocrEdtCreate( &timestepLoopEDT, timestepLoopTML,
                  EDT_PARAM_DEF, (u64*) &itimestep, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &pbPTR->myAffinityHNT, &timestepLoopOEVT );

    ocrEventCreate( &timestepLoopOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( timestepLoopOEVT, timestepLoopOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence(cmd_g, timestepLoopEDT, 0, DB_MODE_RO);
    ocrAddDependence(sbDBK, timestepLoopEDT, 1, DB_MODE_RO);
    ocrAddDependence(pbDBK, timestepLoopEDT, 2, DB_MODE_RW);
    ocrAddDependence(rpKeDBK, timestepLoopEDT, 3, DB_MODE_RW);
    ocrAddDependence(DBK_linkCell, timestepLoopEDT, 4, DB_MODE_RW);
    ocrAddDependence(DBK_atomData, timestepLoopEDT, 5, DB_MODE_RW);
    ocrAddDependence(DBK_mass, timestepLoopEDT, 6, DB_MODE_RO);
    ocrAddDependence(kineticEnergyOEVTS1, timestepLoopEDT, 7, DB_MODE_NULL);
    }
    #endif

    return NULL_GUID;
}




ocrGuid_t channelInitEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t cmd_g = depv[0].guid;
    ocrGuid_t sbDBK = depv[1].guid;
    ocrGuid_t pbDBK = depv[2].guid;
    ocrGuid_t rpVcmDBK = depv[3].guid;
    ocrGuid_t rpKeDBK = depv[4].guid;
    ocrGuid_t DBK_linkCell = depv[5].guid;
    ocrGuid_t DBK_atomData = depv[6].guid;
    ocrGuid_t DBK_mass = depv[7].guid;

    command_t* cmd_p = depv[0].ptr;
    sharedBlock_t* sbPTR = depv[1].ptr;
    privateBlock_t* pbPTR = depv[2].ptr;
    reductionPrivate_t* rpVcmPTR = depv[3].ptr;
    reductionPrivate_t* rpKePTR = depv[4].ptr;
    linkCellH_t* PTR_linkCell = depv[5].ptr;
    atomData_t* PTR_atomData = depv[6].ptr;
    mass_t* mass_p = depv[7].ptr;

    //PRINTF("myrank %d\n", pbPTR->myrank);

    u64 i, dummy;

//record received channel events

    for(i=0;i<=26;i++)
    {
        ocrGuid_t* eventsPTR = depv[8+i].ptr;
        pbPTR->nbr_positionEVTs[i] = eventsPTR[0]; //positionEVTS[nbr]
        pbPTR->nbr_redistributeEVTs[i] = eventsPTR[1];
        PTR_linkCell->nbr_linkCellGuids[i] = eventsPTR[2];
        PTR_linkCell->nbr_atomDataGuids[i] = eventsPTR[3];
        //pbPTR->nbr_massGuids[i] = eventsPTR[4];
        //PRINTF("Atomg guid %lx %lx\n", PTR_linkCell->nbr_linkCellGuids[i], PTR_linkCell->nbr_atomDataGuids[i]);
    }

    //Capture all the events needed for inter-cell depedencies

//launch comd

    //comdEdt
    ocrGuid_t comd;
    ocrEdtCreate(&comd, pbPTR->comdTML, EDT_PARAM_DEF, NULL, 8, NULL,
      EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);

    ocrGuid_t dataBlock;
    ocrDbCreate(&dataBlock, (void**) &dummy, sizeof(double), 0, NULL_HINT, NO_ALLOC);

    ocrDbRelease(pbDBK);
    ocrDbRelease(rpVcmDBK);  //TODO: not needed?

    ocrDbRelease(DBK_linkCell);

    ocrAddDependence(cmd_g, comd, 0, DB_MODE_RO);
    ocrAddDependence(sbDBK, comd, 1, DB_MODE_RO);
    ocrAddDependence(pbDBK, comd, 2, DB_MODE_RW);
    ocrAddDependence(rpVcmDBK, comd, 3, DB_MODE_RW);
    ocrAddDependence(rpKeDBK, comd, 4, DB_MODE_RW);
    ocrAddDependence(DBK_linkCell, comd, 5, DB_MODE_RW);
    ocrAddDependence(DBK_atomData, comd, 6, DB_MODE_RW);
    ocrAddDependence(DBK_mass, comd, 7, DB_MODE_RO);

    //ocrAddDependence(dataBlock, comd, 0, DB_MODE_RW);

    return NULL_GUID;
}

#define MOD(a,b) ((((a)%(b))+(b))%(b))

ocrGuid_t comdInitEdt( u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrGuid_t cmd_g = depv[0].guid;
    ocrGuid_t sbDBK = depv[1].guid;
    ocrGuid_t pbDBK = depv[2].guid;
    ocrGuid_t DBK_linkCell = depv[3].guid;
    ocrGuid_t DBK_atomData = depv[4].guid;
    ocrGuid_t DBK_mass = depv[5].guid;
    ocrGuid_t rpVcmDBK = depv[6].guid;
    ocrGuid_t rpKeDBK = depv[7].guid;

    command_t* cmd_p = depv[0].ptr;
    sharedBlock_t* sbPTR = depv[1].ptr;
    privateBlock_t* pbPTR = depv[2].ptr;
    linkCellH_t* PTR_linkCell = depv[3].ptr;
    atomData_t* PTR_atomData = depv[4].ptr;
    mass_t* PTR_mass = depv[5].ptr;
    reductionPrivate_t* rpVcmPTR = depv[6].ptr;
    reductionPrivate_t* rpKePTR = depv[7].ptr;

    u32 myrank = paramv[0];

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = 3;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;

    rpVcmPTR->nrank = sbPTR->boxDataStH.b_num;
    rpVcmPTR->myrank = myrank;
    rpVcmPTR->ndata = 4;
    rpVcmPTR->reductionOperator = REDUCTION_F8_ADD;
    rpVcmPTR->rangeGUID = sbPTR->vcmReductionRangeGUID;
    rpVcmPTR->reductionTML = NULL_GUID;
    rpVcmPTR->new = 1;  //first time
    rpVcmPTR->all = 1;  //go up and down (ALL_REDUCE)

    ocrEventCreateParams(&(rpVcmPTR->returnEVT), OCR_EVENT_CHANNEL_T, true, &params);//TODO

    rpKePTR->nrank = sbPTR->boxDataStH.b_num;
    rpKePTR->myrank = myrank;
    rpKePTR->ndata = 3; //TODO
    rpKePTR->reductionOperator = REDUCTION_F8_ADD;
    rpKePTR->rangeGUID = sbPTR->KeReductionRangeGUID;
    rpKePTR->reductionTML = NULL_GUID;
    rpKePTR->new = 1;  //first time
    rpKePTR->all = 1;  //go up and down (ALL_REDUCE)

    ocrEventCreateParams(&(rpKePTR->returnEVT), OCR_EVENT_CHANNEL_T, true, &params);

    pbPTR->nrank = sbPTR->boxDataStH.b_num;
    pbPTR->myrank = myrank;
    pbPTR->grid[0] = sbPTR->boxDataStH.grid[0];
    pbPTR->grid[1] = sbPTR->boxDataStH.grid[1];
    pbPTR->grid[2] = sbPTR->boxDataStH.grid[2];
    pbPTR->domain[0] = sbPTR->boxDataStH.domain[0];
    pbPTR->domain[1] = sbPTR->boxDataStH.domain[1];
    pbPTR->domain[2] = sbPTR->boxDataStH.domain[2];
    pbPTR->timestep = 0;
    //TODO: Add more

    pbPTR->finalOnceEVT = sbPTR->finalOnceEVT;
    //pbPTR->comdPhase = 0;

    ocrGuid_t myAffinity;
    ocrHintInit(&pbPTR->myAffinityHNT,OCR_HINT_EDT_T);
    ocrAffinityGetCurrent(&myAffinity);
    ocrSetHintValue(&pbPTR->myAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(myAffinity));

    ocrEdtTemplateCreate(&(pbPTR->comdTML), comdEdt, 0, 8);
    //ocrEdtTemplateCreate(&(pbPTR->mgTML), mgEdt, PRMNUM(mg), DEPVNUM(mg));
    //ocrEdtTemplateCreate(&(pbPTR->haloExchangeTML), haloExchangeEdt, PRMNUM(haloExchange), DEPVNUM(haloExchange));
    //ocrEdtTemplateCreate(&(pbPTR->packTML), packEdt, PRMNUM(pack), DEPVNUM(pack));
    //ocrEdtTemplateCreate(&(pbPTR->unpackTML), unpackEdt, PRMNUM(unpack), DEPVNUM(unpack));
    //ocrEdtTemplateCreate(&(pbPTR->spmvTML), spmvEdt, PRMNUM(spmv), DEPVNUM(spmv));
    //ocrEdtTemplateCreate(&(pbPTR->smoothTML), smoothEdt, PRMNUM(smooth), DEPVNUM(smooth));

    u32 nrank = pbPTR->nrank;

    s32 gx = pbPTR->grid[0];
    s32 gy = pbPTR->grid[1];
    s32 gz = pbPTR->grid[2];
    s32 i, j, k;

//initialize sendBlock, sendEVT, and recvEVT

    for(i=0;i<=26;i++) {
        //pbPTR->haloSendEVT[i] = UNINITIALIZED_GUID;
        pbPTR->positionEVTs[i] = UNINITIALIZED_GUID;
        pbPTR->redistributeEVTs[i] = UNINITIALIZED_GUID;
    }

    ocrGuid_t comdEDT;
    ocrGuid_t cbDBK;
    ocrGuid_t cbPTR;
    ocrGuid_t channelEVT;
    ocrGuid_t channelInitTML;
    ocrGuid_t channelInitEDT;
    ocrGuid_t stickyEVT;

    u32 iBox = myrank;
    s32 ix0, iy0, iz0, ix, iy, iz;
    ix0 = iBox % gx;
    iBox /= gx;
    iy0 = iBox % gy;
    iz0 = iBox / gy;

    ocrGuid_t haloRangeGUID = sbPTR->haloRangeGUID;

    ocrEdtTemplateCreate(&channelInitTML, channelInitEdt, 0, 35 );
    ocrEdtCreate(&channelInitEDT, channelInitTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);

    u32 errno;
    u32 ind = 0;
    u32 partner;
    for(k=-1;k<2;k++)
        for(j=-1;j<2;j++)
            for(i=-1;i<2;i++)
            {
                ix = MOD(ix0+i, gx); //periodic index
                iy = MOD(iy0+j, gy);
                iz = MOD(iz0+k, gz);
                u32 nbrRank = ( iz*gy + iy ) * gx + ix;

                ocrGuidFromIndex(&(stickyEVT), sbPTR->haloRangeGUID, 27*myrank + ind);//send
                ocrEventCreate( &stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );

                ocrEventCreateParams( &(pbPTR->positionEVTs[ind]), OCR_EVENT_CHANNEL_T, false, &params );
                ocrEventCreateParams( &(pbPTR->redistributeEVTs[ind]), OCR_EVENT_CHANNEL_T, false, &params );

                ocrGuid_t* eventsPTR;
                ocrGuid_t eventsDBK;
                ocrDbCreate( &eventsDBK, (void**) &eventsPTR, 5*sizeof(ocrGuid_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

                eventsPTR[0] = pbPTR->positionEVTs[ind]; //channel event for box-i
                eventsPTR[1] = pbPTR->redistributeEVTs[ind]; //channel event for box-i
                eventsPTR[2] = DBK_linkCell;
                eventsPTR[3] = DBK_atomData;
                eventsPTR[4] = DBK_mass;

                ocrDbRelease( eventsDBK );

                ocrEventSatisfy( stickyEVT, eventsDBK );

                //receive
                ocrGuidFromIndex( &(stickyEVT), sbPTR->haloRangeGUID, 27*nbrRank + 26-ind );
                ocrEventCreate( &stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );

                //PRINTF("s %d r %d se %d re %d s(%d %d %d) r(%d %d %d)\n", myrank, nbrRank, ind, 26-ind, ix0, iy0, iz0, ix, iy, iz);
                //Collective event create for the neighbors

                ocrAddDependence( stickyEVT, channelInitEDT, 8+ind, DB_MODE_RW ); //TODO

                ind++;
            }

    ocrDbRelease(pbDBK);
    ocrDbRelease(rpVcmDBK);
    ocrDbRelease(rpKeDBK);

    ocrAddDependence( cmd_g, channelInitEDT, 0, DB_MODE_RO );
    ocrAddDependence( sbDBK, channelInitEDT, 1, DB_MODE_RO );
    ocrAddDependence( pbDBK, channelInitEDT, 2, DB_MODE_RW );
    ocrAddDependence( rpVcmDBK, channelInitEDT, 3, DB_MODE_RW );
    ocrAddDependence( rpKeDBK, channelInitEDT, 4, DB_MODE_RW );
    ocrAddDependence( DBK_linkCell, channelInitEDT, 5, DB_MODE_RW );
    ocrAddDependence( DBK_atomData, channelInitEDT, 6, DB_MODE_RW );
    ocrAddDependence( DBK_mass, channelInitEDT, 7, DB_MODE_RO );


    return NULL_GUID;
}

ocrGuid_t initEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t sbDBK = depv[0].guid;
    ocrGuid_t cmd_g = depv[1].guid;

    u64 id = (u64) paramv[0];

    sharedBlock_t * sbPTR = (sharedBlock_t *) depv[0].ptr;

    ocrGuid_t comdInitEDT, comdInitTML;

    ocrHint_t myHNT;
    ocrHintInit( &myHNT, OCR_HINT_EDT_T );
    ocrGuid_t myAffinity;
    ocrAffinityGetCurrent( &myAffinity );
    ocrSetHintValue(&myHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(myAffinity));

    ocrEdtTemplateCreate( &comdInitTML, comdInitEdt, 1, 8 );

    ocrEdtCreate( &comdInitEDT, comdInitTML, EDT_PARAM_DEF, (u64 *) &id, EDT_PARAM_DEF, NULL,
                    EDT_PROP_NONE, &myHNT, NULL );
    ocrEdtTemplateDestroy( comdInitTML );

    ocrGuid_t pbDBK, DBK_linkCell, DBK_atomData;
    u64 dummy;

    //DB creation calls
    ocrDbCreate( &pbDBK, (void**) &dummy, sizeof(privateBlock_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
    ocrDbCreate( &DBK_linkCell, (void**) &dummy, sizeof(linkCellH_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
    ocrDbCreate( &DBK_atomData, (void**) &dummy, sizeof(atomData_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
    //ocrDbCreate( &DBK_mass, (void**) &dummy, sizeof(mass_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
    ocrGuid_t DBK_mass = sbPTR->pot.mass;
//reduction block
    ocrGuid_t rpVcmDBK;
    ocrDbCreate( &rpVcmDBK, (void**) &dummy, sizeof(reductionPrivate_t), 0, NULL_HINT, NO_ALLOC );
    ocrGuid_t rpKeDBK;
    ocrDbCreate( &rpKeDBK, (void**) &dummy, sizeof(reductionPrivate_t), 0, NULL_HINT, NO_ALLOC );

    ocrAddDependence( cmd_g, comdInitEDT, 0, DB_MODE_RO );
    ocrAddDependence( sbDBK, comdInitEDT, 1, DB_MODE_RO );
    ocrAddDependence( pbDBK, comdInitEDT, 2, DB_MODE_RW );
    ocrAddDependence( DBK_linkCell, comdInitEDT, 3, DB_MODE_RW );
    ocrAddDependence( DBK_atomData, comdInitEDT, 4, DB_MODE_RW );
    ocrAddDependence( DBK_mass, comdInitEDT, 5, DB_MODE_RO );
    ocrAddDependence( rpVcmDBK, comdInitEDT, 6, DB_MODE_RW );
    ocrAddDependence( rpKeDBK, comdInitEDT, 7, DB_MODE_RW );

    //u64 i;

    //for(i=0;i<26;i++)
    //{
    //    ocrDbCreate( &tempDb, (void**) &dummy, sizeof(ocrGuid_t), 0, NULL_HINT, NO_ALLOC );
    //    ocrAddDependence( tempDb, comdInitEDT, 6+i, DB_MODE_RW );
    //}

    return NULL_GUID;
}

ocrGuid_t wrapUpEdt( u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[] )
{
    //prints out the sum of deviations from all 1s

    //DEPVDEF(wrapUp);
    //PRMDEF(wrapUp);
    //double * returnPTR = (double *) DEPV(wrapUp,returnBlock,ptr);
    //PRINTF("final deviation: %f \n", returnPTR[0]);
    //double stop =  wtime();
    //double elapsed = stop - PRM(wrapUp,startTime);
    //PRINTF("elapsed time: %f \n", elapsed);
    PRINTF("Shutting down\n");
    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t realMainEdt( u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrGuid_t cmd_g = depv[0].guid;
    ocrGuid_t sbDBK = depv[1].guid;

    command_t *cmd_p = depv[0].ptr;
    sharedBlock_t *sbPTR = depv[1].ptr;

    u8 rcode = init_simulation( cmd_p, sbPTR );
    //sbPTR is updated above

    ocrGuid_t wrapUpTML, wrapUpEDT;
    ocrEdtTemplateCreate( &wrapUpTML, wrapUpEdt, 0, 1 );
    ocrEdtCreate( &wrapUpEDT, wrapUpTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
    ocrEventCreate( &(sbPTR->finalOnceEVT), OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG );
    ocrEdtTemplateDestroy( wrapUpTML );

    ocrAddDependence(sbPTR->finalOnceEVT, wrapUpEDT, 0, DB_MODE_RO);

    u32* grid = sbPTR->boxDataStH.grid;

    ocrGuid_t initTML, EDT_init;
    ocrEdtTemplateCreate( &initTML, initEdt, 1, 2 );

    ocrGuid_t PDaffinityGuid = NULL_GUID;

    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );

#ifdef ENABLE_EXTENSION_AFFINITY
    s64 affinityCount;
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
    s64 PD_X, PD_Y, PD_Z;
    splitDimension( affinityCount, &PD_X, &PD_Y, &PD_Z ); //Split available PDs into a 3-D grid
#else
    //PRINTF("NOT Using affinity API\n");
#endif

    u64 i;

    u64 b_num = sbPTR->boxDataStH.b_num;
    ocrDbRelease(sbDBK);
    ocrDbRelease(cmd_g);

    for( i = 0; i < b_num; ++i )
    {
#ifdef ENABLE_EXTENSION_AFFINITY
        int pd = getPoliyDomainID( i, grid, PD_X, PD_Y, PD_Z );
        ocrAffinityGetAt( AFFINITY_PD, pd, &(PDaffinityGuid) );
        ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif
        ocrEdtCreate( &EDT_init, initTML, EDT_PARAM_DEF, (u64*)&i, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &HNT_edt, NULL );
        ocrAddDependence( sbDBK, EDT_init, 0, DB_MODE_RO );
        ocrAddDependence( cmd_g, EDT_init, 1, DB_MODE_RO );
    }

    return NULL_GUID;
}

ocrGuid_t mainEdt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    u32 _paramc, _depc, _idep;

    //BEGIN: Command line argument processing
    void * programArgv = depv[0].ptr;
    u32 argc = getArgc( programArgv );

    ocrGuid_t argv_g;
    char** argv;
    ocrDbCreate( &argv_g, (void**)&argv, sizeof(char*)*argc, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    for( u32 a = 0; a < argc; ++a )
      argv[a] = getArgv( programArgv, a );

    ocrGuid_t cmd_g;
    command_t* cmd_p;
    ocrDbCreate( &cmd_g, (void**)&cmd_p, sizeof(command_t), DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    parse_command( argc, argv, cmd_p );

    ocrDbRelease( cmd_g );
    ocrDbDestroy( argv_g );
    //END

    ocrGuid_t sharedDBK;
    sharedBlock_t* sim_p;
    ocrDbCreate( &sharedDBK, (void**) &sim_p, sizeof(sharedBlock_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

    ocrGuid_t realMainEDT, realMainTML;

    ocrEdtTemplateCreate( &realMainTML, realMainEdt, 0, 2 );

    ocrEdtCreate( &realMainEDT, realMainTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL );

    ocrAddDependence( cmd_g, realMainEDT, 0, DB_MODE_RO );
    ocrAddDependence( sharedDBK, realMainEDT, 1, DB_MODE_RW );

    ocrEdtTemplateDestroy( realMainTML );

    return NULL_GUID;
}


#include <math.h>
#include <ocr.h>

#include "extensions/ocr-affinity.h"

#include "comd.h"
#include "command.h"
#include "timers.h"
#include "potentials.h"
#include "simulation.h"
#include "reductions.h"

u8 init_simulation( command_t* cmd_p, simulationH_t* simH_p, mdtimer_t* timer_p )
{
  simH_p->step = 0;
  simH_p->steps = cmd_p->steps;
  simH_p->period = cmd_p->period;
  simH_p->dt = cmd_p->dt;
  simH_p->e_potential = 0.0;
  simH_p->e_kinetic = 0.0;
  simH_p->boxDataStH.DBK_linkCellGuidsH = simH_p->boxDataStH.DBK_atomDataGuidsH = NULL_GUID;
  u8 insane = 0;
  if( cmd_p->doeam )
    insane = init_eam( cmd_p->pot_dir, cmd_p->pot_name, cmd_p->pot_type, &simH_p->pot, simH_p->dt );
  else
    init_lj( &simH_p->pot, simH_p->dt );
  if( insane ) return insane;

  real_t lattice_const = cmd_p->lat < 0 ? simH_p->pot.lat : cmd_p->lat;
  double minx, miny, minz;
  minx = miny = minz = 2*simH_p->pot.cutoff;
  double sizex = cmd_p->nx*lattice_const;
  double sizey = cmd_p->ny*lattice_const;
  double sizez = cmd_p->nz*lattice_const;
  if( sizex < minx || sizey < miny || sizez < minz ) {
    insane |= 1;
    ocrPrintf( "\nSimulation too small.\n"
           "  Increase the number of unit cells to make the simulation\n"
           "  at least (%3.2f, %3.2f. %3.2f) Ansgstroms in size\n", minx, miny, minz );
  }
  if( simH_p->pot.lattice_type!=FCC ) {
    insane |= 2;
    ocrPrintf( "\nOnly FCC Lattice type supported. Fatal Error.\n");
  }
  if( insane ) return insane;

  simH_p->boxDataStH.domain[0] = cmd_p->nx*lattice_const;
  simH_p->boxDataStH.domain[1] = cmd_p->ny*lattice_const;
  simH_p->boxDataStH.domain[2] = cmd_p->nz*lattice_const;
  simH_p->boxDataStH.grid[0] = simH_p->boxDataStH.domain[0]/simH_p->pot.cutoff;
  simH_p->boxDataStH.grid[1] = simH_p->boxDataStH.domain[1]/simH_p->pot.cutoff;
  simH_p->boxDataStH.grid[2] = simH_p->boxDataStH.domain[2]/simH_p->pot.cutoff;
  simH_p->boxDataStH.b_num = simH_p->boxDataStH.grid[0]*simH_p->boxDataStH.grid[1]*simH_p->boxDataStH.grid[2];
  simH_p->boxDataStH.b_size[0] = simH_p->boxDataStH.domain[0]/simH_p->boxDataStH.grid[0];
  simH_p->boxDataStH.b_size[1] = simH_p->boxDataStH.domain[1]/simH_p->boxDataStH.grid[1];
  simH_p->boxDataStH.b_size[2] = simH_p->boxDataStH.domain[2]/simH_p->boxDataStH.grid[2];
  simH_p->boxDataStH.inv_b_size[0] = 1/simH_p->boxDataStH.b_size[0];
  simH_p->boxDataStH.inv_b_size[1] = 1/simH_p->boxDataStH.b_size[1];
  simH_p->boxDataStH.inv_b_size[2] = 1/simH_p->boxDataStH.b_size[2];

  simH_p->atoms = 4*(cmd_p->nx*cmd_p->ny*cmd_p->nz); //TODO: Hard-coded to copper BCC linkcells

  ocrPrintf( "Simulation data: \n");
  ocrPrintf( "  Total atoms        : %d\n", simH_p->atoms );
  ocrPrintf( "  Min bounds  : [ %14.10f, %14.10f, %14.10f ]\n", 0.0,0.0,0.0 );
  ocrPrintf( "  Max bounds  : [ %14.10f, %14.10f, %14.10f ]\n\n",
         simH_p->boxDataStH.domain[0], simH_p->boxDataStH.domain[1], simH_p->boxDataStH.domain[2] );
  ocrPrintf( "  Boxes        : %6d,%6d,%6d = %8d\n",
         simH_p->boxDataStH.grid[0], simH_p->boxDataStH.grid[1], simH_p->boxDataStH.grid[2], simH_p->boxDataStH.b_num );
  ocrPrintf( "  Box size           : [ %14.10f, %14.10f, %14.10f ]\n",
         simH_p->boxDataStH.b_size[0], simH_p->boxDataStH.b_size[1], simH_p->boxDataStH.b_size[2] );
  ocrPrintf( "  Box factor         : [ %14.10f, %14.10f, %14.10f ] \n",
         simH_p->boxDataStH.b_size[0]/simH_p->pot.cutoff,
         simH_p->boxDataStH.b_size[1]/simH_p->pot.cutoff,
         simH_p->boxDataStH.b_size[2]/simH_p->pot.cutoff );
  //ocrPrintf( "  Max Link Cell Occupancy: %d of %d\n\n", simH_p->boxDataStH.max_occupancy, MAXATOMS );
  ocrPrintf( "\nPotential data: \n");
  simH_p->pot.print( &simH_p->pot );

   // Memory footprint diagnostics
   int perAtomSize = 10*sizeof(real_t)+2*sizeof(int);
   float atomMemGlobal = (float)(perAtomSize*simH_p->atoms)/1024/1024;

   int nTotalBoxes = simH_p->boxDataStH.b_num;
   float linkCellMemTotal = (float) nTotalBoxes*(perAtomSize*MAXATOMS)/1024/1024;

   ocrPrintf( "\n" );
   ocrPrintf("Memory data: \n");
   ocrPrintf( "  Intrinsic atom footprint = %4d B/atom \n", perAtomSize);
   ocrPrintf( "  Total atom footprint     = %7.3f MB\n", atomMemGlobal);
   ocrPrintf( "  Link cell atom footprint = %7.3f MB\n\n", linkCellMemTotal);

  ocrGuid_t* PTR_linkCellGuidsH;
  ocrGuid_t* PTR_atomDataGuidsH;
  ocrDbCreate( &simH_p->boxDataStH.DBK_linkCellGuidsH, (void**)&PTR_linkCellGuidsH, sizeof(ocrGuid_t)*simH_p->boxDataStH.b_num, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
  ocrDbCreate( &simH_p->boxDataStH.DBK_atomDataGuidsH, (void**)&PTR_atomDataGuidsH, sizeof(ocrGuid_t)*simH_p->boxDataStH.b_num, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    ocrGuid_t PDaffinityGuid = NULL_GUID;

    ocrHint_t HNT_db;
    ocrHintInit( &HNT_db, OCR_HINT_DB_T );

#ifdef ENABLE_EXTENSION_AFFINITY
    s64 affinityCount;
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
    ocrGuid_t DBK_affinityGuids;
    ocrGuid_t* PTR_affinityGuids;
    ocrDbCreate( &DBK_affinityGuids, (void**) &PTR_affinityGuids, sizeof(ocrGuid_t)*affinityCount,
                 DB_PROP_SINGLE_ASSIGNMENT, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    ocrAffinityGet( AFFINITY_PD, &affinityCount, PTR_affinityGuids ); //Get all the available Policy Domain affinity guids
    ocrAssert( affinityCount >= 1 );
    ocrPrintf("Using affinity API\n");
    s64 PD_X, PD_Y, PD_Z;
    splitDimension(affinityCount, &PD_X, &PD_Y, &PD_Z); //Split available PDs into a 3-D grid
#else
    ocrPrintf("NOT Using affinity API\n");
#endif

  u32 b;
  u32* grid = simH_p->boxDataStH.grid;
  for( b=0; b<simH_p->boxDataStH.b_num; ++b )
  {
#ifdef ENABLE_EXTENSION_AFFINITY
        int pd = getPoliyDomainID( b, grid, PD_X, PD_Y, PD_Z );
        PDaffinityGuid = PTR_affinityGuids[pd];
        ocrSetHintValue( &HNT_db, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif
    void* ptr;
    //ocrDbCreate( PTR_linkCellGuidsH+b, &ptr, sizeof(linkCellH_t), DB_PROP_NO_ACQUIRE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );
    //ocrDbCreate( PTR_atomDataGuidsH+b, &ptr, sizeof(atomData_t), DB_PROP_NO_ACQUIRE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );
    ocrDbCreate( PTR_linkCellGuidsH+b, &ptr, sizeof(linkCellH_t),
                    DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC ); //TODO:DB_PROP_NO_ACQUIRE results in a hang with affinity hints
    ocrDbCreate( PTR_atomDataGuidsH+b, &ptr, sizeof(atomData_t),
                    DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );
  }

  ocrDbRelease( simH_p->boxDataStH.DBK_linkCellGuidsH );
  ocrDbRelease( simH_p->boxDataStH.DBK_atomDataGuidsH );

#ifdef ENABLE_EXTENSION_AFFINITY
  ocrDbDestroy( DBK_affinityGuids );
#endif

  return 0;
}

real_t lcg61( u64* seed )
{
  *seed *= 437799614237992725;
  *seed %= 2305843009213693951;
  return *seed*(1.0/2305843009213693951);
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

u64 mk_seed( u32 i, u32 c )
{
  u64 s1 = (i*2654435761)&0xFFFFFFFF;
  u64 s2 = ((i+c)*2654435761)&0xFFFFFFFF;
  u64 seed = (s1<<32)+s2;
  lcg61(&seed); lcg61(&seed);
  lcg61(&seed); lcg61(&seed);
  lcg61(&seed); lcg61(&seed);
  lcg61(&seed); lcg61(&seed);
  lcg61(&seed); lcg61(&seed);
  return seed;
}

//params: dt, continuation
//depv: DBK_atomDataH, mass
ocrGuid_t ukvel_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrGuid_t PDaffinityGuid = NULL_GUID;
    ocrHint_t HNT_db;
    ocrHintInit( &HNT_db, OCR_HINT_DB_T );
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&PDaffinityGuid);
    ocrSetHintValue( &HNT_db, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif

  PRM_ukvel_edt_t* PTR_PRM_ukvel_edt = (PRM_ukvel_edt_t*) paramv;

  real_t dt = PTR_PRM_ukvel_edt->dt;
  ocrGuid_t uleaf = PTR_PRM_ukvel_edt->uleaf;

  ocrGuid_t uk_g; real_t* uk_p;
  ocrDbCreate( &uk_g, (void**)&uk_p, sizeof(real_t)*2, DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );
  uk_p[0] = uk_p[1] = 0;

  atomData_t* PTR_atomData = (atomData_t*)depv[0].ptr;
  mass_t* mass = (mass_t*)depv[1].ptr;

  u8 a;
  for( a=0; a<PTR_atomData->atoms; ++a ) {
    uk_p[0] += PTR_atomData->u[a];
    uk_p[1] += (PTR_atomData->p[a][0]*PTR_atomData->p[a][0]+PTR_atomData->p[a][1]*PTR_atomData->p[a][1]+PTR_atomData->p[a][2]*PTR_atomData->p[a][2])*
               mass->inv_mass_2[PTR_atomData->s[a]];
    PTR_atomData->p[a][0] += dt*PTR_atomData->f[a][0];
    PTR_atomData->p[a][1] += dt*PTR_atomData->f[a][1];
    PTR_atomData->p[a][2] += dt*PTR_atomData->f[a][2];
  }

  ocrDbRelease( uk_g ); //uk_g was created and updated in the same EDT. Must Release the changes before EventSatisfy.
  ocrEventSatisfy( uleaf,uk_g );

  return NULL_GUID;
}

//params: dt, continuation
//depv: DBK_atomDataH, mass
ocrGuid_t veluk_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrGuid_t PDaffinityGuid = NULL_GUID;
    ocrHint_t HNT_db;
    ocrHintInit( &HNT_db, OCR_HINT_DB_T );
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&PDaffinityGuid);
    ocrSetHintValue( &HNT_db, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif

  PRM_veluk_edt_t* PTR_veluk_edt = (PRM_veluk_edt_t*) paramv;

  real_t dt = PTR_veluk_edt->dt;
  ocrGuid_t leaves_g = PTR_veluk_edt->leaves_g;

  atomData_t* PTR_atomData = (atomData_t*)depv[0].ptr;
  mass_t* mass = (mass_t*)depv[1].ptr;
  ocrGuid_t uk_g; real_t* uk_p;
  ocrDbCreate( &uk_g, (void**)&uk_p, sizeof(real_t)*2, DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC);
  uk_p[0] = uk_p[1]= 0;

  u8 a;
  for( a=0; a<PTR_atomData->atoms; ++a ) {
    PTR_atomData->p[a][0] += dt*PTR_atomData->f[a][0];
    PTR_atomData->p[a][1] += dt*PTR_atomData->f[a][1];
    PTR_atomData->p[a][2] += dt*PTR_atomData->f[a][2];
    uk_p[0] += PTR_atomData->u[a];
    uk_p[1] += (PTR_atomData->p[a][0]*PTR_atomData->p[a][0]+PTR_atomData->p[a][1]*PTR_atomData->p[a][1]+PTR_atomData->p[a][2]*PTR_atomData->p[a][2])*
               mass->inv_mass_2[PTR_atomData->s[a]];
  }

  ocrDbRelease( uk_g );
  ocrEventSatisfy( *(ocrGuid_t*)(paramv+1),uk_g );

  return NULL_GUID;
}

//depv: scale, DBK_atomDataH, schedule
ocrGuid_t FNC_sched( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
  ocrGuid_t DBK_atomDataH = depv[1].guid;
  ocrGuid_t sched26_g = depv[2].guid;

  reductionH_t* reductionH_p = (reductionH_t*)depv[0].ptr;
  atomData_t* PTR_atomData = (atomData_t*)depv[1].ptr;
  ocrGuid_t* schedule = (ocrGuid_t*)depv[2].ptr;

  u8 a;
  for( a=0; a<PTR_atomData->atoms; ++a ) {
    PTR_atomData->p[a][0] *= reductionH_p->value[0];
    PTR_atomData->p[a][1] *= reductionH_p->value[0];
    PTR_atomData->p[a][2] *= reductionH_p->value[0];
  }

  ocrDbRelease(DBK_atomDataH);

  u8 n;
  for( n=0; n<26; ++n )
    ocrAddDependence( DBK_atomDataH, schedule[n], n+1, DB_MODE_RO );
  ocrDbDestroy( sched26_g );

  return DBK_atomDataH;
}

//depv: reduction, DBK_atomDataH, mass
ocrGuid_t vcm_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrGuid_t PDaffinityGuid = NULL_GUID;
    ocrHint_t HNT_db;
    ocrHintInit( &HNT_db, OCR_HINT_DB_T );
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&PDaffinityGuid);
    ocrSetHintValue( &HNT_db, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif

  PRM_vcm_edt_t* PTR_PRM_vcm_edt = (PRM_vcm_edt_t*) paramv;

  ocrGuid_t tleaf = PTR_PRM_vcm_edt->tleaf;
  ocrGuid_t EDT_sched = PTR_PRM_vcm_edt->EDT_sched;

  reductionH_t* reductionH_p = (reductionH_t*)depv[0].ptr;
  atomData_t* PTR_atomData = (atomData_t*)depv[1].ptr;
  mass_t* mass = (mass_t*)depv[2].ptr;
  ocrGuid_t ek_g; real_t* ek_p;
  ocrDbCreate( &ek_g, (void**)&ek_p, sizeof(real_t), DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );
  *ek_p = 0;

  u32 a;
  for( a=0; a<PTR_atomData->atoms; ++a ) {
    real_t m = mass->mass[PTR_atomData->s[a]];
    PTR_atomData->p[a][0] += m*reductionH_p->value[0];
    PTR_atomData->p[a][1] += m*reductionH_p->value[1];
    PTR_atomData->p[a][2] += m*reductionH_p->value[2];
    *ek_p += (PTR_atomData->p[a][0]*PTR_atomData->p[a][0]+PTR_atomData->p[a][1]*PTR_atomData->p[a][1]+PTR_atomData->p[a][2]*PTR_atomData->p[a][2])*
              mass->inv_mass_2[PTR_atomData->s[a]];
  }

  ocrDbRelease( ek_g );

  ocrAddDependence( reductionH_p->OEVT_reduction, EDT_sched, 0, DB_MODE_RO ); //tred_out
  ocrAddDependence( ek_g, tleaf, 0, DB_MODE_RO );

  return NULL_GUID;
}

//params: grid[0]-grid[1], grid[2]-lattice[0], lattice[1]-lattice[2], delta, temperature, vleaf, tleaf, uleaf
//{ simH_g<RO>, mass_g<RW>, reduction_g<RW>, DBK_linkCellH<RW>, DBK_atomDataH<RW>, sched0<RW>, sched1<RW>, ..., sched25<RW>, sched26<RO>, DBK_nbr_dataGuids<RO> }
ocrGuid_t FNC_init( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrGuid_t PDaffinityGuid = NULL_GUID;
    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );
    ocrHint_t HNT_db;
    ocrHintInit( &HNT_db, OCR_HINT_DB_T );
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&PDaffinityGuid);
    ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
    ocrSetHintValue( &HNT_db, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif
    PRM_FNC_init_t* PTR_PRM_FNC_init = (PRM_FNC_init_t*) paramv;

    u32* coords = PTR_PRM_FNC_init->coords;
    u32* lattice = PTR_PRM_FNC_init->lattice;
    real_t delta = PTR_PRM_FNC_init->delta;
    real_t temperature = PTR_PRM_FNC_init->temperature;
    ocrGuid_t vleaf = PTR_PRM_FNC_init->vleaf;
    ocrGuid_t tleaf = PTR_PRM_FNC_init->tleaf;
    ocrGuid_t uleaf = PTR_PRM_FNC_init->uleaf;

    ocrGuid_t simH_g = depv[0].guid;
    ocrGuid_t mass_g = depv[1].guid;
    ocrGuid_t reduction_g = depv[2].guid;
    ocrGuid_t DBK_linkCellH = depv[3].guid;
    ocrGuid_t DBK_atomDataH = depv[4].guid;

    ocrGuid_t sched26_g = depv[31].guid;

    ocrGuid_t DBK_nbr_dataGuids = depv[32].guid;

    simulationH_t* simH_p = (simulationH_t*)depv[0].ptr;
    mass_t* mass_p = (mass_t*)depv[1].ptr;
    reductionH_t* reductionH_p = (reductionH_t*)depv[2].ptr;
    linkCellH_t* linkCellH_p = (linkCellH_t*)depv[3].ptr;
    atomData_t* PTR_atomData = (atomData_t*)depv[4].ptr;

    ocrGuid_t* nbr_dataGuids = (ocrGuid_t*)depv[32].ptr;

    PTR_atomData->linkCellGuid = DBK_linkCellH;
    //ocrPrintf("%s %d %d %d\n", __func__, coords[0], coords[1], coords[2]);

#ifndef TG_ARCH
  memset( linkCellH_p,0,sizeof(linkCellH_t)); memset(PTR_atomData,0,sizeof(atomData_t));
#else
  for( u32 m=0; m<sizeof(linkCellH_t); ++m ) ((char*)linkCellH_p)[m]=0;
  for( u32 m=0; m<sizeof(atomData_t); ++m ) ((char*)PTR_atomData)[m]=0;
#endif

  linkCellH_p->coordinates[0] = PTR_PRM_FNC_init->coords[0];
  linkCellH_p->coordinates[1] = PTR_PRM_FNC_init->coords[1];
  linkCellH_p->coordinates[2] = PTR_PRM_FNC_init->coords[2];

  linkCellH_p->min[0] = linkCellH_p->coordinates[0]*simH_p->boxDataStH.b_size[0];
  linkCellH_p->min[1] = linkCellH_p->coordinates[1]*simH_p->boxDataStH.b_size[1];
  linkCellH_p->min[2] = linkCellH_p->coordinates[2]*simH_p->boxDataStH.b_size[2];
  linkCellH_p->max[0] = linkCellH_p->min[0]+simH_p->boxDataStH.b_size[0];
  linkCellH_p->max[1] = linkCellH_p->min[1]+simH_p->boxDataStH.b_size[1];
  linkCellH_p->max[2] = linkCellH_p->min[2]+simH_p->boxDataStH.b_size[2];
  s32 begin[3] = {floor(linkCellH_p->min[0]/simH_p->pot.lat)-1,
                  floor(linkCellH_p->min[1]/simH_p->pot.lat)-1,
                  floor(linkCellH_p->min[2]/simH_p->pot.lat)-1};
  s32 end[3] = {ceil((linkCellH_p->max[0])/simH_p->pot.lat)+1,
                ceil((linkCellH_p->max[1])/simH_p->pot.lat)+1,
                ceil((linkCellH_p->max[2])/simH_p->pot.lat)+1};
  real3_t basis[4] = {{0.25, 0.25, 0.25},
                      {0.25, 0.75, 0.75},
                      {0.75, 0.25, 0.75},
                      {0.75, 0.75, 0.25}};

  if( linkCellH_p->coordinates[0]==0 ) PTR_atomData->nmask|=1;
  if( linkCellH_p->coordinates[0]==simH_p->boxDataStH.grid[0]-1 ) PTR_atomData->nmask|=2;
  if( linkCellH_p->coordinates[1]==0 ) PTR_atomData->nmask|=4;
  if( linkCellH_p->coordinates[1]==simH_p->boxDataStH.grid[1]-1 ) PTR_atomData->nmask|=8;
  if( linkCellH_p->coordinates[2]==0 ) PTR_atomData->nmask|=16;
  if( linkCellH_p->coordinates[2]==simH_p->boxDataStH.grid[2]-1 ) PTR_atomData->nmask|=32;

  u8 inbr;
  for( inbr=0; inbr < 26; ++inbr ) {
    linkCellH_p->nbr_atomDataGuids[inbr] = nbr_dataGuids[inbr];
    linkCellH_p->nbr_linkCellGuids[inbr] = nbr_dataGuids[26+inbr];
  }
  ocrDbDestroy( DBK_nbr_dataGuids );

  ocrGuid_t vcm_g;
  real_t* vcm_p;
  ocrDbCreate( &vcm_g, (void**)&vcm_p, sizeof(real_t)*4+sizeof(u64), DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );
#ifndef TG_ARCH
  memset( vcm_p,0,sizeof(real_t)*4+sizeof(u64));
#else
  for( u32 m=0; m<sizeof(real_t)*4+sizeof(u64); ++m ) ((char*)vcm_p)[m]=0;
#endif
  u64* atoms = (u64*)(vcm_p+4);

  s32 i,j,k,b;
  for( i = begin[0]; i<end[0]; ++i )
  for( j = begin[1]; j<end[1]; ++j )
  for( k = begin[2]; k<end[2]; ++k )
  for( b=0; b<4; ++b ) {
    real_t rx = (i+basis[b][0])*simH_p->pot.lat;
    real_t ry = (j+basis[b][1])*simH_p->pot.lat;
    real_t rz = (k+basis[b][2])*simH_p->pot.lat;
    linkCellH_p->gid[PTR_atomData->atoms] = b+((k+lattice[2]*(j+lattice[1]*i))<<2);
    u64 seed = mk_seed(linkCellH_p->gid[PTR_atomData->atoms], 457);
    PTR_atomData->r[PTR_atomData->atoms][0] = rx+(2.0*lcg61(&seed)-1.0)*delta;
    PTR_atomData->r[PTR_atomData->atoms][1] = ry+(2.0*lcg61(&seed)-1.0)*delta;
    PTR_atomData->r[PTR_atomData->atoms][2] = rz+(2.0*lcg61(&seed)-1.0)*delta;
    if (rx < linkCellH_p->min[0] || rx >= linkCellH_p->max[0]) continue;
    if (ry < linkCellH_p->min[1] || ry >= linkCellH_p->max[1]) continue;
    if (rz < linkCellH_p->min[2] || rz >= linkCellH_p->max[2]) continue;

    PTR_atomData->s[PTR_atomData->atoms] = 0;
    real_t ms = mass_p->mass[PTR_atomData->s[PTR_atomData->atoms]]*sqrt(kB_eV*temperature/mass_p->mass[PTR_atomData->s[PTR_atomData->atoms]]);
    seed = mk_seed(linkCellH_p->gid[PTR_atomData->atoms], 123);
    vcm_p[0] += PTR_atomData->p[PTR_atomData->atoms][0] = ms*gasdev(&seed);
    vcm_p[1] += PTR_atomData->p[PTR_atomData->atoms][1] = ms*gasdev(&seed);
    vcm_p[2] += PTR_atomData->p[PTR_atomData->atoms][2] = ms*gasdev(&seed);
    vcm_p[3] += mass_p->mass[PTR_atomData->s[PTR_atomData->atoms]];
    ++PTR_atomData->atoms;
  }

  *atoms = linkCellH_p->max_occupancy = PTR_atomData->atoms;

  ocrDbRelease(DBK_linkCellH);
  ocrDbRelease(DBK_atomDataH);

  ocrGuid_t tmp,edt;

  ocrGuid_t fin;
  ocrEdtTemplateCreate( &tmp,FNC_sched,0,3 );
  ocrEdtCreate( &edt, tmp, 0, NULL, 3, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), &fin );
  ocrAddDependence( DBK_atomDataH, edt, 1, DB_MODE_RW );
  ocrAddDependence( sched26_g, edt, 2, DB_MODE_RO );
  ocrEdtTemplateDestroy( tmp );

  PRM_vcm_edt_t PRM_vcm_edt;

  PRM_vcm_edt.tleaf = tleaf;
  PRM_vcm_edt.EDT_sched = edt;

  ocrEdtTemplateCreate( &tmp,vcm_edt,sizeof(PRM_vcm_edt_t)/sizeof(u64),3 );
  ocrEdtCreate( &edt, tmp, EDT_PARAM_DEF, (u64*)&PRM_vcm_edt, 3, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), NULL );
  ocrAddDependence( reductionH_p->OEVT_reduction, edt, 0, DB_MODE_RO ); //vred_out
  ocrAddDependence( DBK_atomDataH, edt, 1, DB_MODE_RW );
  ocrAddDependence( mass_g, edt, 2, DB_MODE_RO );
  ocrEdtTemplateDestroy( tmp );

  if( simH_p->pot.potential&LJ)
  {
    PRM_force_edt_t PRM_force_edt;
    PRM_force_edt.lj.sigma = simH_p->pot.lj.sigma;
    PRM_force_edt.lj.epsilon = simH_p->pot.lj.epsilon;
    PRM_force_edt.cutoff = simH_p->pot.cutoff;
    PRM_force_edt.domain[0] = simH_p->boxDataStH.domain[0];
    PRM_force_edt.domain[1] = simH_p->boxDataStH.domain[1];
    PRM_force_edt.domain[2] = simH_p->boxDataStH.domain[2];

    ocrGuid_t tfin=fin;
    ocrEdtTemplateCreate( &tmp,simH_p->pot.force_edt,sizeof(PRM_force_edt_t)/sizeof(u64),27 );
    ocrEdtCreate( &edt, tmp, EDT_PARAM_DEF, (u64*)&PRM_force_edt, 27, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), &fin );
    ocrAddDependence( tfin, edt, 0, DB_MODE_RW );
    ocrEdtTemplateDestroy( tmp );
  }
  else {
    ocrAssert(0);
  }

  u8 n;
  for( n=0; n<26; ++n ) {
    ocrGuid_t* PTR_linkCell_scheduleGuids = (ocrGuid_t*)depv[5+n].ptr;
    PTR_linkCell_scheduleGuids[n] = edt;
  }

  PRM_ukvel_edt_t PRM_ukvel_edt;

  PRM_ukvel_edt.dt = 0.5;
  PRM_ukvel_edt.uleaf = uleaf;

  ocrEdtTemplateCreate( &tmp,ukvel_edt,sizeof(PRM_ukvel_edt_t)/sizeof(u64),2 );
  ocrEdtCreate( &edt, tmp, EDT_PARAM_DEF, (u64*)&PRM_ukvel_edt, 2, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), NULL );
  ocrEdtTemplateDestroy( tmp );
  ocrAddDependence( fin, edt, 0, DB_MODE_RW );
  ocrAddDependence( mass_g, edt, 1, DB_MODE_RO );

  ocrDbRelease( vcm_g );
  ocrEventSatisfy( vleaf, vcm_g ); //vred_out

  return NULL_GUID;
}

//params: grid[0]-grid[1], grid[2]-lattice[0], lattice[1]-lattice[2], delta, temperature
// { simH_g<RW>, box<RO>, DBK_atomDataGuidsH<RO> }
ocrGuid_t EDT_init_fork( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
  PRM_EDT_init_fork_t* PTR_PRM_EDT_init_fork = (PRM_EDT_init_fork_t*) paramv;

  ocrGuid_t simH_g = depv[0].guid;
  ocrGuid_t DBK_linkCellGuidsH = depv[1].guid;
  ocrGuid_t DBK_atomDataGuidsH = depv[2].guid;

  simulationH_t* simH_p = (simulationH_t*)depv[0].ptr;
  ocrGuid_t* PTR_linkCellGuidsH = (ocrGuid_t*)depv[1].ptr;
  ocrGuid_t* PTR_atomDataGuidsH = (ocrGuid_t*)depv[2].ptr;

  u32* grid = PTR_PRM_EDT_init_fork->grid;
  u32* lattice = PTR_PRM_EDT_init_fork->lattice;
  real_t delta = PTR_PRM_EDT_init_fork->delta;
  real_t temperature = PTR_PRM_EDT_init_fork->temperature;

  ocrGuid_t DBK_scheduleGuids, *PTR_scheduleGuids;
  ocrDbCreate( &DBK_scheduleGuids, (void**)&PTR_scheduleGuids, sizeof( ocrGuid_t)*simH_p->boxDataStH.b_num, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    ocrGuid_t PDaffinityGuid = NULL_GUID;

    ocrHint_t HNT_db;
    ocrHintInit( &HNT_db, OCR_HINT_DB_T );

#ifdef ENABLE_EXTENSION_AFFINITY
    s64 affinityCount;
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
    ocrGuid_t DBK_affinityGuids;
    ocrGuid_t* PTR_affinityGuids;
    ocrDbCreate( &DBK_affinityGuids, (void**) &PTR_affinityGuids, sizeof(ocrGuid_t)*affinityCount,
                 DB_PROP_SINGLE_ASSIGNMENT, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    ocrAffinityGet( AFFINITY_PD, &affinityCount, PTR_affinityGuids ); //Get all the available Policy Domain affinity guids;
    ocrAssert( affinityCount >= 1 );
    //ocrPrintf("Using affinity API\n");
    s64 PD_X, PD_Y, PD_Z;
    splitDimension(affinityCount, &PD_X, &PD_Y, &PD_Z); //Split available PDs into a 3-D grid
#else
    //ocrPrintf("NOT Using affinity API\n");
#endif

  u32 b; void* PTR_linkCell_scheduleGuids;
  for( b=0; b<simH_p->boxDataStH.b_num; ++b )
  {
#ifdef ENABLE_EXTENSION_AFFINITY
        int pd = getPoliyDomainID( b, grid, PD_X, PD_Y, PD_Z );
        PDaffinityGuid = PTR_affinityGuids[pd];
        ocrSetHintValue( &HNT_db, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif
    //ocrDbCreate( PTR_scheduleGuids+b, &PTR_linkCell_scheduleGuids, sizeof(ocrGuid_t)*26, DB_PROP_NO_ACQUIRE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC ); //TODO: DB_PROP_NO_ACQUIRE issue
    ocrDbCreate( PTR_scheduleGuids+b, &PTR_linkCell_scheduleGuids, sizeof(ocrGuid_t)*26, DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );
  }

  reductionH_t* reductionH_p;
  ocrDbCreate( &simH_p->reductionH_g, (void**)&reductionH_p, sizeof(reductionH_t), DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

  ocrGuid_t* leaves_p; ocrGuid_t leaves_g;
  ocrDbCreate( &leaves_g, (void**)&leaves_p, sizeof(ocrGuid_t)*simH_p->boxDataStH.b_num*3, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

  PRM_red_t PRM_red;

  PRM_red.n = 0; //Update later
  PRM_red.temperature = temperature;
  PRM_red.epsilon = simH_p->pot.potential&LJ ? simH_p->pot.lj.epsilon : 1;
  PRM_red.guid = NULL_GUID;

  reductionH_p->OEVT_reduction = build_reduction( simH_g, simH_p->reductionH_g, simH_p->boxDataStH.b_num, leaves_p+simH_p->boxDataStH.b_num*2,
                                            sizeof(PRM_ured_edt_t)/sizeof(u64), &PRM_red, ured_edt, grid, 0 );

  PRM_red.guid = reductionH_p->OEVT_reduction;

  reductionH_p->OEVT_reduction = build_reduction( simH_g, simH_p->reductionH_g, simH_p->boxDataStH.b_num, leaves_p+simH_p->boxDataStH.b_num,
                                            sizeof(PRM_tred_edt_t)/sizeof(u64), &PRM_red, tred_edt, grid, 1 );

  PRM_red.guid = reductionH_p->OEVT_reduction;

  reductionH_p->OEVT_reduction = build_reduction( simH_g, simH_p->reductionH_g, simH_p->boxDataStH.b_num, leaves_p,
                                            sizeof(PRM_vred_edt_t)/sizeof(u64), &PRM_red, vred_edt, grid, 2 );

  ocrDbRelease( simH_p->reductionH_g );

  ocrGuid_t tmp, EDT_init;
  ocrEdtTemplateCreate( &tmp,FNC_init,sizeof(PRM_FNC_init_t)/sizeof(u64),33 );

  PRM_FNC_init_t PRM_FNC_init;
  PRM_FNC_init.lattice[0] = lattice[0];
  PRM_FNC_init.lattice[1] = lattice[1];
  PRM_FNC_init.lattice[2] = lattice[2];
  PRM_FNC_init.delta = delta;
  PRM_FNC_init.temperature = temperature;

    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );

  u32 ijk[3];
  PDaffinityGuid = NULL_GUID;

  u32 leaf = 0;
  for( ijk[0]=0; ijk[0]<grid[0]; ++ijk[0] )
  for( ijk[1]=0; ijk[1]<grid[1]; ++ijk[1] )
  for( ijk[2]=0; ijk[2]<grid[2]; ++ijk[2] ) {
    PRM_FNC_init.coords[0] = ijk[0];
    PRM_FNC_init.coords[1] = ijk[1];
    PRM_FNC_init.coords[2] = ijk[2];
    PRM_FNC_init.vleaf = leaves_p[leaf]; //vred_leaf
    PRM_FNC_init.tleaf = leaves_p[leaf+simH_p->boxDataStH.b_num]; //tred_leaf
    PRM_FNC_init.uleaf = leaves_p[leaf+simH_p->boxDataStH.b_num*2]; //ured_leaf

#ifdef ENABLE_EXTENSION_AFFINITY
        u64 count = 1;
        ocrAffinityQuery(PTR_linkCellGuidsH[leaf], &count, &PDaffinityGuid);
        ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
        ocrSetHintValue( &HNT_db, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif

    ocrEdtCreate( &EDT_init, tmp, EDT_PARAM_DEF, (u64*)&PRM_FNC_init, 33, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), NULL );

    ocrAddDependence( simH_g, EDT_init, 0, DB_MODE_RO );
    ocrAddDependence( simH_p->pot.mass, EDT_init, 1, DB_MODE_RO );
    ocrAddDependence( simH_p->reductionH_g, EDT_init, 2, DB_MODE_RO );
    ocrAddDependence( PTR_linkCellGuidsH[leaf], EDT_init, 3, DB_MODE_RW );
    ocrAddDependence( PTR_atomDataGuidsH[leaf], EDT_init, 4, DB_MODE_RW );

    ocrGuid_t DBK_nbr_dataGuids; ocrGuid_t* nbr_dataGuids;
    ocrDbCreate( &DBK_nbr_dataGuids, (void**)&nbr_dataGuids, sizeof(ocrGuid_t)*52, DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );

    for( b=0; b < 26; ++b ) {
      u32 n = neighbor_id(b,ijk,grid);
      nbr_dataGuids[b] = PTR_atomDataGuidsH[n]; nbr_dataGuids[26+b] = PTR_linkCellGuidsH[n];

      ocrAddDependence( PTR_scheduleGuids[n], EDT_init, 5+b, DB_MODE_RW ); //RW is OK here because the writes from the EDT_init 's are non-overlapping
    }

    ocrAddDependence( PTR_scheduleGuids[leaf], EDT_init, 31, DB_MODE_RO );

    ocrDbRelease(DBK_nbr_dataGuids);

    ocrAddDependence( DBK_nbr_dataGuids, EDT_init, 32, DB_MODE_RO );
    ++leaf;
  }

  ocrEdtTemplateDestroy( tmp );

  //ocrDbDestroy( leaves_g );
  //ocrDbDestroy( DBK_scheduleGuids );
#ifdef ENABLE_EXTENSION_AFFINITY
  ocrDbDestroy( DBK_affinityGuids );
#endif

  return NULL_GUID;
}

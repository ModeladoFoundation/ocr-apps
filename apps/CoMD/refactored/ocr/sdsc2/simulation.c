#include <math.h>
#include <ocr.h>

#include "extensions/ocr-affinity.h"

#include "comd.h"
#include "command.h"
#include "timers.h"
#include "potentials.h"
#include "simulation.h"
#include "reductions.h"

u8 init_simulation( command_t* cmd_p, simulation_t* sim_p, mdtimer_t* timer_p )
{
  sim_p->step = 0;
  sim_p->steps = cmd_p->steps;
  sim_p->period = cmd_p->period;
  sim_p->dt = cmd_p->dt;
  sim_p->e_potential = 0.0;
  sim_p->e_kinetic = 0.0;
  sim_p->boxes.box = sim_p->boxes.rpf = NULL_GUID;
  u8 insane = 0;
  if( cmd_p->doeam )
    insane = init_eam( cmd_p->pot_dir, cmd_p->pot_name, cmd_p->pot_type, &sim_p->pot, sim_p->dt );
  else
    init_lj( &sim_p->pot, sim_p->dt );
  if( insane ) return insane;

  real_t lattice_const = cmd_p->lat < 0 ? sim_p->pot.lat : cmd_p->lat;
  double minx, miny, minz;
  minx = miny = minz = 2*sim_p->pot.cutoff;
  double sizex = cmd_p->nx*lattice_const;
  double sizey = cmd_p->ny*lattice_const;
  double sizez = cmd_p->nz*lattice_const;
  if( sizex < minx || sizey < miny || sizez < minz ) {
    insane |= 1;
    PRINTF( "\nSimulation too small.\n"
           "  Increase the number of unit cells to make the simulation\n"
           "  at least (%3.2f, %3.2f. %3.2f) Ansgstroms in size\n", minx, miny, minz );
  }
  if( sim_p->pot.lattice_type!=FCC ) {
    insane |= 2;
    PRINTF( "\nOnly FCC Lattice type supported. Fatal Error.\n");
  }
  if( insane ) return insane;

  sim_p->boxes.domain[0] = cmd_p->nx*lattice_const;
  sim_p->boxes.domain[1] = cmd_p->ny*lattice_const;
  sim_p->boxes.domain[2] = cmd_p->nz*lattice_const;
  sim_p->boxes.grid[0] = sim_p->boxes.domain[0]/sim_p->pot.cutoff;
  sim_p->boxes.grid[1] = sim_p->boxes.domain[1]/sim_p->pot.cutoff;
  sim_p->boxes.grid[2] = sim_p->boxes.domain[2]/sim_p->pot.cutoff;
  sim_p->boxes.boxes_num = sim_p->boxes.grid[0]*sim_p->boxes.grid[1]*sim_p->boxes.grid[2];
  sim_p->boxes.box_size[0] = sim_p->boxes.domain[0]/sim_p->boxes.grid[0];
  sim_p->boxes.box_size[1] = sim_p->boxes.domain[1]/sim_p->boxes.grid[1];
  sim_p->boxes.box_size[2] = sim_p->boxes.domain[2]/sim_p->boxes.grid[2];
  sim_p->boxes.inv_box_size[0] = 1/sim_p->boxes.box_size[0];
  sim_p->boxes.inv_box_size[1] = 1/sim_p->boxes.box_size[1];
  sim_p->boxes.inv_box_size[2] = 1/sim_p->boxes.box_size[2];

  //TODO: sim_p->atoms uninitialized here

  PRINTF( "Simulation data: \n");
  PRINTF( "  Total atoms        : %d\n", sim_p->atoms );
  PRINTF( "  Min bounds  : [ %14.10f, %14.10f, %14.10f ]\n", 0.0,0.0,0.0 );
  PRINTF( "  Max bounds  : [ %14.10f, %14.10f, %14.10f ]\n\n",
         sim_p->boxes.domain[0], sim_p->boxes.domain[1], sim_p->boxes.domain[2] );
  PRINTF( "  Boxes        : %6d,%6d,%6d = %8d\n",
         sim_p->boxes.grid[0], sim_p->boxes.grid[1], sim_p->boxes.grid[2], sim_p->boxes.boxes_num );
  PRINTF( "  Box size           : [ %14.10f, %14.10f, %14.10f ]\n",
         sim_p->boxes.box_size[0], sim_p->boxes.box_size[1], sim_p->boxes.box_size[2] );
  PRINTF( "  Box factor         : [ %14.10f, %14.10f, %14.10f ] \n",
         sim_p->boxes.box_size[0]/sim_p->pot.cutoff,
         sim_p->boxes.box_size[1]/sim_p->pot.cutoff,
         sim_p->boxes.box_size[2]/sim_p->pot.cutoff );
  //PRINTF( "  Max Link Cell Occupancy: %d of %d\n\n", sim_p->boxes.max_occupancy, MAXATOMS );
  PRINTF( "\nPotential data: \n");
  sim_p->pot.print( &sim_p->pot );

  ocrGuid_t* box_p;
  ocrGuid_t* rpf_p;
  ocrDbCreate( &sim_p->boxes.box, (void**)&box_p, sizeof(ocrGuid_t)*sim_p->boxes.boxes_num, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
  ocrDbCreate( &sim_p->boxes.rpf, (void**)&rpf_p, sizeof(ocrGuid_t)*sim_p->boxes.boxes_num, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

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
    ASSERT( affinityCount >= 1 );
    PRINTF("Using affinity API\n");
    s64 PD_X, PD_Y, PD_Z;
    splitDimension(affinityCount, &PD_X, &PD_Y, &PD_Z); //Split available PDs into a 3-D grid
#else
    PRINTF("NOT Using affinity API\n");
#endif

  u32 b;
  u32* grid = sim_p->boxes.grid;
  for( b=0; b<sim_p->boxes.boxes_num; ++b )
  {
#ifdef ENABLE_EXTENSION_AFFINITY
        int id_x = (b/grid[0])/grid[1];
        s64 pd_x; getPartitionID(id_x, 0, grid[0]-1, PD_X, &pd_x);

        int id_y = (b/grid[0])%grid[1];
        s64 pd_y; getPartitionID(id_y, 0, grid[1]-1, PD_Y, &pd_y);

        int id_z = b%grid[0];
        s64 pd_z; getPartitionID(id_z, 0, grid[2]-1, PD_Z, &pd_z);

        //Each linkcell, with id=b, is mapped to a PD. The mapping similar to how the link cells map to
        //MPI ranks. In other words, all the PDs are arranged as a 3-D grid.
        //And, a 3-D subgrid of linkcells is mapped to a PD preserving "locality" within a PD.

        int pd = globalRankFromCoords(pd_z, pd_y, pd_x, PD_X, PD_Y, PD_Z);
        //PRINTF("box %d %d %d, policy domain %d: %d %d %d\n", id_x, id_y, id_z, pd, PD_X, PD_Y, PD_Z);
        PDaffinityGuid = PTR_affinityGuids[pd];
        //PDaffinityGuid = NULL_GUID;
        ocrSetHintValue( &HNT_db, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif
    void* ptr;
    //ocrDbCreate( box_p+b, &ptr, sizeof(box_t), DB_PROP_NO_ACQUIRE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );
    //ocrDbCreate( rpf_p+b, &ptr, sizeof(rpf_t), DB_PROP_NO_ACQUIRE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );
    ocrDbCreate( box_p+b, &ptr, sizeof(box_t), DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC ); //TODO:DB_PROP_NO_ACQUIRE results in a hang with affinity hints
    ocrDbCreate( rpf_p+b, &ptr, sizeof(rpf_t), DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );
  }

  ocrDbRelease( sim_p->boxes.box );
  ocrDbRelease( sim_p->boxes.rpf );

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
//depv: rpf, mass
ocrGuid_t ukvel_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
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

  PRM_ukvel_edt_t* PTR_PRM_ukvel_edt = (PRM_ukvel_edt_t*) paramv;

  real_t dt = PTR_PRM_ukvel_edt->dt;
  ocrGuid_t uleaf = PTR_PRM_ukvel_edt->uleaf;

  ocrGuid_t uk_g; real_t* uk_p;
  ocrDbCreate( &uk_g, (void**)&uk_p, sizeof(real_t)*2, DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );
  uk_p[0] = uk_p[1] = 0;

  rpf_t* rpf = (rpf_t*)depv[0].ptr;
  mass_t* mass = (mass_t*)depv[1].ptr;

  u8 a;
  for( a=0; a<rpf->atoms; ++a ) {
    uk_p[0] += rpf->u[a];
    uk_p[1] += (rpf->p[a][0]*rpf->p[a][0]+rpf->p[a][1]*rpf->p[a][1]+rpf->p[a][2]*rpf->p[a][2])*
               mass->inv_mass_2[rpf->s[a]];
    rpf->p[a][0] += dt*rpf->f[a][0];
    rpf->p[a][1] += dt*rpf->f[a][1];
    rpf->p[a][2] += dt*rpf->f[a][2];
  }

  ocrDbRelease( uk_g ); //uk_g was created and updated in the same EDT. Must Release the changes before EventSatisfy.
  ocrEventSatisfy( uleaf,uk_g );

  return NULL_GUID;
}

//params: dt, continuation
//depv: rpf, mass
ocrGuid_t veluk_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
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

  PRM_veluk_edt_t* PTR_veluk_edt = (PRM_veluk_edt_t*) paramv;

  real_t dt = PTR_veluk_edt->dt;
  ocrGuid_t leaves_g = PTR_veluk_edt->leaves_g;

  rpf_t* rpf = (rpf_t*)depv[0].ptr;
  mass_t* mass = (mass_t*)depv[1].ptr;
  ocrGuid_t uk_g; real_t* uk_p;
  ocrDbCreate( &uk_g, (void**)&uk_p, sizeof(real_t)*2, DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC);
  uk_p[0] = uk_p[1]= 0;

  u8 a;
  for( a=0; a<rpf->atoms; ++a ) {
    rpf->p[a][0] += dt*rpf->f[a][0];
    rpf->p[a][1] += dt*rpf->f[a][1];
    rpf->p[a][2] += dt*rpf->f[a][2];
    uk_p[0] += rpf->u[a];
    uk_p[1] += (rpf->p[a][0]*rpf->p[a][0]+rpf->p[a][1]*rpf->p[a][1]+rpf->p[a][2]*rpf->p[a][2])*
               mass->inv_mass_2[rpf->s[a]];
  }

  ocrEventSatisfy( *(ocrGuid_t*)(paramv+1),uk_g );

  return NULL_GUID;
}

//depv: scale, rpf, schedule
ocrGuid_t temp_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
  ocrGuid_t rpf_g = depv[1].guid;
  ocrGuid_t sched26_g = depv[2].guid;

  reductionH_t* reductionH_p = (reductionH_t*)depv[0].ptr;
  rpf_t* rpf = (rpf_t*)depv[1].ptr;
  ocrGuid_t* schedule = (ocrGuid_t*)depv[2].ptr;

  u8 a;
  for( a=0; a<rpf->atoms; ++a ) {
    rpf->p[a][0] *= reductionH_p->value[0];
    rpf->p[a][1] *= reductionH_p->value[0];
    rpf->p[a][2] *= reductionH_p->value[0];
  }

  u8 n;
  for( n=0; n<26; ++n )
    ocrAddDependence( rpf_g, schedule[n], n+1, DB_MODE_RO );
  ocrDbDestroy( sched26_g );

  return depv[1].guid;
}

//depv: reduction, rpf, mass
ocrGuid_t vcm_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
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

  PRM_vcm_edt_t* PTR_PRM_vcm_edt = (PRM_vcm_edt_t*) paramv;

  ocrGuid_t tleaf = PTR_PRM_vcm_edt->tleaf;
  ocrGuid_t EDT_sched = PTR_PRM_vcm_edt->EDT_sched;

  reductionH_t* reductionH_p = (reductionH_t*)depv[0].ptr;
  rpf_t* rpf = (rpf_t*)depv[1].ptr;
  mass_t* mass = (mass_t*)depv[2].ptr;
  ocrGuid_t ek_g; real_t* ek_p;
  ocrDbCreate( &ek_g, (void**)&ek_p, sizeof(real_t), DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );
  *ek_p = 0;

  u32 a;
  for( a=0; a<rpf->atoms; ++a ) {
    real_t m = mass->mass[rpf->s[a]];
    rpf->p[a][0] += m*reductionH_p->value[0];
    rpf->p[a][1] += m*reductionH_p->value[1];
    rpf->p[a][2] += m*reductionH_p->value[2];
    *ek_p += (rpf->p[a][0]*rpf->p[a][0]+rpf->p[a][1]*rpf->p[a][1]+rpf->p[a][2]*rpf->p[a][2])*
              mass->inv_mass_2[rpf->s[a]];
  }

  ocrAddDependence( reductionH_p->OEVT_reduction, EDT_sched, 0, DB_MODE_RO );
  ocrAddDependence( ek_g, tleaf, 0, DB_MODE_RO );

  return NULL_GUID;
}

//params: grid[0]-grid[1], grid[2]-lattice[0], lattice[1]-lattice[2], delta, temperature, vleaf, tleaf, uleaf
//{ sim_g<RO>, mass_g<RW>, reduction_g<RW>, ibox_g<RW>, irpf_g<RW>, sched0<RW>, sched1<RW>, ..., sched25<RW>, sched26<RO>, rb_g<RO> }
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

    ocrGuid_t sim_g = depv[0].guid;
    ocrGuid_t mass_g = depv[1].guid;
    ocrGuid_t reduction_g = depv[2].guid;
    ocrGuid_t ibox_g = depv[3].guid;
    ocrGuid_t irpf_g = depv[4].guid;

    ocrGuid_t sched26_g = depv[31].guid;

    ocrGuid_t rb_g = depv[32].guid;

    simulation_t* sim_p = (simulation_t*)depv[0].ptr;
    mass_t* mass_p = (mass_t*)depv[1].ptr;
    reductionH_t* reductionH_p = (reductionH_t*)depv[2].ptr;
    box_t* box_p = (box_t*)depv[3].ptr;
    rpf_t* rpf_p = (rpf_t*)depv[4].ptr;

    ocrGuid_t* rbp = (ocrGuid_t*)depv[32].ptr;

    rpf_p->box = ibox_g;
    //PRINTF("%s %d %d %d\n", __func__, coords[0], coords[1], coords[2]);

#ifndef TG_ARCH
  memset( box_p,0,sizeof(box_t)); memset(rpf_p,0,sizeof(rpf_t));
#else
  for( u32 m=0; m<sizeof(box_t); ++m ) ((char*)box_p)[m]=0;
  for( u32 m=0; m<sizeof(rpf_t); ++m ) ((char*)rpf_p)[m]=0;
#endif

  box_p->coordinates[0] = PTR_PRM_FNC_init->coords[0];
  box_p->coordinates[1] = PTR_PRM_FNC_init->coords[1];
  box_p->coordinates[2] = PTR_PRM_FNC_init->coords[2];

  box_p->min[0] = box_p->coordinates[0]*sim_p->boxes.box_size[0];
  box_p->min[1] = box_p->coordinates[1]*sim_p->boxes.box_size[1];
  box_p->min[2] = box_p->coordinates[2]*sim_p->boxes.box_size[2];
  box_p->max[0] = box_p->min[0]+sim_p->boxes.box_size[0];
  box_p->max[1] = box_p->min[1]+sim_p->boxes.box_size[1];
  box_p->max[2] = box_p->min[2]+sim_p->boxes.box_size[2];
  s32 begin[3] = {floor(box_p->min[0]/sim_p->pot.lat)-1,
                  floor(box_p->min[1]/sim_p->pot.lat)-1,
                  floor(box_p->min[2]/sim_p->pot.lat)-1};
  s32 end[3] = {ceil((box_p->max[0])/sim_p->pot.lat)+1,
                ceil((box_p->max[1])/sim_p->pot.lat)+1,
                ceil((box_p->max[2])/sim_p->pot.lat)+1};
  real3_t basis[4] = {{0.25, 0.25, 0.25},
                      {0.25, 0.75, 0.75},
                      {0.75, 0.25, 0.75},
                      {0.75, 0.75, 0.25}};

  if( box_p->coordinates[0]==0 ) rpf_p->nmask|=1;
  if( box_p->coordinates[0]==sim_p->boxes.grid[0]-1 ) rpf_p->nmask|=2;
  if( box_p->coordinates[1]==0 ) rpf_p->nmask|=4;
  if( box_p->coordinates[1]==sim_p->boxes.grid[1]-1 ) rpf_p->nmask|=8;
  if( box_p->coordinates[2]==0 ) rpf_p->nmask|=16;
  if( box_p->coordinates[2]==sim_p->boxes.grid[2]-1 ) rpf_p->nmask|=32;

  u8 rb;
  for( rb=0; rb < 26; ++rb ) {
    box_p->rneighbors[rb] = rbp[rb];
    box_p->bneighbors[rb] = rbp[26+rb];
  }
  ocrDbDestroy( rb_g );

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
    real_t rx = (i+basis[b][0])*sim_p->pot.lat;
    real_t ry = (j+basis[b][1])*sim_p->pot.lat;
    real_t rz = (k+basis[b][2])*sim_p->pot.lat;
    box_p->gid[rpf_p->atoms] = b+((k+lattice[2]*(j+lattice[1]*i))<<2);
    u64 seed = mk_seed(box_p->gid[rpf_p->atoms], 457);
    rpf_p->r[rpf_p->atoms][0] = rx+(2.0*lcg61(&seed)-1.0)*delta;
    rpf_p->r[rpf_p->atoms][1] = ry+(2.0*lcg61(&seed)-1.0)*delta;
    rpf_p->r[rpf_p->atoms][2] = rz+(2.0*lcg61(&seed)-1.0)*delta;
    if (rx < box_p->min[0] || rx >= box_p->max[0]) continue;
    if (ry < box_p->min[1] || ry >= box_p->max[1]) continue;
    if (rz < box_p->min[2] || rz >= box_p->max[2]) continue;

    rpf_p->s[rpf_p->atoms] = 0;
    real_t ms = mass_p->mass[rpf_p->s[rpf_p->atoms]]*sqrt(kB_eV*temperature/mass_p->mass[rpf_p->s[rpf_p->atoms]]);
    seed = mk_seed(box_p->gid[rpf_p->atoms], 123);
    vcm_p[0] += rpf_p->p[rpf_p->atoms][0] = ms*gasdev(&seed);
    vcm_p[1] += rpf_p->p[rpf_p->atoms][1] = ms*gasdev(&seed);
    vcm_p[2] += rpf_p->p[rpf_p->atoms][2] = ms*gasdev(&seed);
    vcm_p[3] += mass_p->mass[rpf_p->s[rpf_p->atoms]];
    ++rpf_p->atoms;
  }

  *atoms = box_p->max_occupancy = rpf_p->atoms;

  ocrGuid_t tmp,edt;

  ocrGuid_t fin;
  ocrEdtTemplateCreate( &tmp,temp_edt,0,3 );
  ocrEdtCreate( &edt, tmp, 0, NULL, 3, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), &fin );
  ocrAddDependence( irpf_g, edt, 1, DB_MODE_RW );
  ocrAddDependence( sched26_g, edt, 2, DB_MODE_RO );
  ocrEdtTemplateDestroy( tmp );

  PRM_vcm_edt_t PRM_vcm_edt;

  PRM_vcm_edt.tleaf = tleaf;
  PRM_vcm_edt.EDT_sched = edt;

  ocrEdtTemplateCreate( &tmp,vcm_edt,sizeof(PRM_vcm_edt_t)/sizeof(u64),3 );
  ocrEdtCreate( &edt, tmp, EDT_PARAM_DEF, (u64*)&PRM_vcm_edt, 3, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), NULL );
  ocrAddDependence( reductionH_p->OEVT_reduction, edt, 0, DB_MODE_RO );
  ocrAddDependence( irpf_g, edt, 1, DB_MODE_RW );
  ocrAddDependence( mass_g, edt, 2, DB_MODE_RO );
  ocrEdtTemplateDestroy( tmp );

  if( sim_p->pot.potential&LJ)
  {
    PRM_force_edt_t PRM_force_edt;
    PRM_force_edt.lj.sigma = sim_p->pot.lj.sigma;
    PRM_force_edt.lj.epsilon = sim_p->pot.lj.epsilon;
    PRM_force_edt.cutoff = sim_p->pot.cutoff;
    PRM_force_edt.domain[0] = sim_p->boxes.domain[0];
    PRM_force_edt.domain[1] = sim_p->boxes.domain[1];
    PRM_force_edt.domain[2] = sim_p->boxes.domain[2];

    ocrGuid_t tfin=fin;
    ocrEdtTemplateCreate( &tmp,sim_p->pot.force_edt,sizeof(PRM_force_edt_t)/sizeof(u64),27 );
    ocrEdtCreate( &edt, tmp, EDT_PARAM_DEF, (u64*)&PRM_force_edt, 27, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), &fin );
    ocrAddDependence( tfin, edt, 0, DB_MODE_RW );
    ocrEdtTemplateDestroy( tmp );
  }
  else {
    ASSERT(0);
  }

  u8 n;
  for( n=0; n<26; ++n ) {
    ocrGuid_t* schedule = (ocrGuid_t*)depv[5+n].ptr;
    schedule[n] = edt;
  }

  PRM_ukvel_edt_t PRM_ukvel_edt;

  PRM_ukvel_edt.dt = 0.5;
  PRM_ukvel_edt.uleaf = uleaf;

  ocrEdtTemplateCreate( &tmp,ukvel_edt,sizeof(PRM_ukvel_edt_t)/sizeof(u64),2 );
  ocrEdtCreate( &edt, tmp, EDT_PARAM_DEF, (u64*)&PRM_ukvel_edt, 2, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), NULL );
  ocrEdtTemplateDestroy( tmp );
  ocrAddDependence( fin, edt, 0, DB_MODE_RW );
  ocrAddDependence( mass_g, edt, 1, DB_MODE_RO );

  ocrEventSatisfy( vleaf, vcm_g );

  return NULL_GUID;
}

//params: grid[0]-grid[1], grid[2]-lattice[0], lattice[1]-lattice[2], delta, temperature
// { sim_g<RW>, box<RO>, rpf<RO> }
ocrGuid_t fork_init_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
  PRM_fork_init_edt_t* PTR_PRM_fork_init_edt = (PRM_fork_init_edt_t*) paramv;

  ocrGuid_t sim_g = depv[0].guid;
  ocrGuid_t box_g = depv[1].guid;
  ocrGuid_t rpf_g = depv[2].guid;

  simulation_t* sim_p = (simulation_t*)depv[0].ptr;
  ocrGuid_t* box_p = (ocrGuid_t*)depv[1].ptr;
  ocrGuid_t* rpf_p = (ocrGuid_t*)depv[2].ptr;

  u32* grid = PTR_PRM_fork_init_edt->grid;
  u32* lattice = PTR_PRM_fork_init_edt->lattice;
  real_t delta = PTR_PRM_fork_init_edt->delta;
  real_t temperature = PTR_PRM_fork_init_edt->temperature;

  ocrGuid_t sch_g, *sch_p;
  ocrDbCreate( &sch_g, (void**)&sch_p, sizeof( ocrGuid_t)*sim_p->boxes.boxes_num, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

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
    ocrAffinityGet( AFFINITY_PD, &affinityCount, PTR_affinityGuids ) //Get all the available Policy Domain affinity guids;
    ASSERT( affinityCount >= 1 );
    //PRINTF("Using affinity API\n");
    s64 PD_X, PD_Y, PD_Z;
    splitDimension(affinityCount, &PD_X, &PD_Y, &PD_Z); //Split available PDs into a 3-D grid
#else
    //PRINTF("NOT Using affinity API\n");
#endif

  u32 b; void* p;
  for( b=0; b<sim_p->boxes.boxes_num; ++b )
  {
#ifdef ENABLE_EXTENSION_AFFINITY
        int id_x = (b/grid[0])/grid[1];
        s64 pd_x; getPartitionID(id_x, 0, grid[0]-1, PD_X, &pd_x);

        int id_y = (b/grid[0])%grid[1];
        s64 pd_y; getPartitionID(id_y, 0, grid[1]-1, PD_Y, &pd_y);

        int id_z = b%grid[0];
        s64 pd_z; getPartitionID(id_z, 0, grid[2]-1, PD_Z, &pd_z);

        //Each linkcell, with id=b, is mapped to a PD. The mapping similar to how the link cells map to
        //MPI ranks. In other words, all the PDs are arranged as a 3-D grid.
        //And, a 3-D subgrid of linkcells is mapped to a PD preserving "locality" within a PD.
        //
        int pd = globalRankFromCoords(pd_z, pd_y, pd_x, PD_X, PD_Y, PD_Z);
        //PRINTF("box %d %d %d, policy domain %d: %d %d %d\n", id_x, id_y, id_z, pd, PD_X, PD_Y, PD_Z);
        PDaffinityGuid = PTR_affinityGuids[pd];
        ocrSetHintValue( &HNT_db, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif
    //ocrDbCreate( sch_p+b, &p, sizeof(ocrGuid_t)*26, DB_PROP_NO_ACQUIRE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC ); //TODO: DB_PROP_NO_ACQUIRE issue
    ocrDbCreate( sch_p+b, &p, sizeof(ocrGuid_t)*26, DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );
  }

  reductionH_t* reductionH_p;
  ocrDbCreate( &sim_p->reductionH_g, (void**)&reductionH_p, sizeof(reductionH_t), DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

  ocrGuid_t* leaves_p; ocrGuid_t leaves_g;
  ocrDbCreate( &leaves_g, (void**)&leaves_p, sizeof(ocrGuid_t)*sim_p->boxes.boxes_num*3, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

  PRM_red_t PRM_red;

  PRM_red.n = 0; //Update later
  PRM_red.temperature = temperature;
  PRM_red.epsilon = sim_p->pot.potential&LJ ? sim_p->pot.lj.epsilon : 1;
  PRM_red.guid = NULL_GUID;

  reductionH_p->OEVT_reduction = build_reduction( sim_g, sim_p->reductionH_g, sim_p->boxes.boxes_num, leaves_p+sim_p->boxes.boxes_num*2,
                                            sizeof(PRM_ured_edt_t)/sizeof(u64), &PRM_red, ured_edt, 0 );

  PRM_red.guid = reductionH_p->OEVT_reduction;

  reductionH_p->OEVT_reduction = build_reduction( sim_g, sim_p->reductionH_g, sim_p->boxes.boxes_num, leaves_p+sim_p->boxes.boxes_num,
                                            sizeof(PRM_tred_edt_t)/sizeof(u64), &PRM_red, tred_edt, 1 );

  PRM_red.guid = reductionH_p->OEVT_reduction;

  reductionH_p->OEVT_reduction = build_reduction( sim_g, sim_p->reductionH_g, sim_p->boxes.boxes_num, leaves_p,
                                            sizeof(PRM_vred_edt_t)/sizeof(u64), &PRM_red, vred_edt, 2 );

  ocrDbRelease( sim_p->reductionH_g );

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
    PRM_FNC_init.tleaf = leaves_p[leaf+sim_p->boxes.boxes_num]; //tred_leaf
    PRM_FNC_init.uleaf = leaves_p[leaf+sim_p->boxes.boxes_num*2]; //ured_leaf

#ifdef ENABLE_EXTENSION_AFFINITY
        u64 count = 1;
        ocrAffinityQuery(box_p[leaf], &count, &PDaffinityGuid);
        ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
        ocrSetHintValue( &HNT_db, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif

    ocrEdtCreate( &EDT_init, tmp, EDT_PARAM_DEF, (u64*)&PRM_FNC_init, 33, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), NULL );

    ocrAddDependence( sim_g, EDT_init, 0, DB_MODE_RO );
    ocrAddDependence( sim_p->pot.mass, EDT_init, 1, DB_MODE_RO );
    ocrAddDependence( sim_p->reductionH_g, EDT_init, 2, DB_MODE_RO );
    ocrAddDependence( box_p[leaf], EDT_init, 3, DB_MODE_RW );
    ocrAddDependence( rpf_p[leaf], EDT_init, 4, DB_MODE_RW );

    ocrGuid_t rb; ocrGuid_t* rbp;
    ocrDbCreate( &rb, (void**)&rbp, sizeof(ocrGuid_t)*52, DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC );

    for( b=0; b < 26; ++b ) {
      u32 n = neighbor_id(b,ijk,grid);
      rbp[b] = rpf_p[n]; rbp[26+b] = box_p[n];

      ocrAddDependence( sch_p[n], EDT_init, 5+b, DB_MODE_RW );
    }

    ocrAddDependence( sch_p[leaf], EDT_init, 31, DB_MODE_RO );

    ocrDbRelease(rb);

    ocrAddDependence( rb, EDT_init, 32, DB_MODE_RO );
    ++leaf;
  }

  ocrEdtTemplateDestroy( tmp );

  ocrDbDestroy( leaves_g );
  ocrDbDestroy( sch_g );
#ifdef ENABLE_EXTENSION_AFFINITY
  ocrDbDestroy( DBK_affinityGuids );
#endif

  return NULL_GUID;
}

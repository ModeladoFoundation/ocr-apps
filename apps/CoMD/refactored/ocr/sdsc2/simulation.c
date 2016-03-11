#include <math.h>
#include <ocr.h>

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
  ocrDbCreate( &sim_p->boxes.box, (void**)&box_p, sizeof(ocrGuid_t)*sim_p->boxes.boxes_num, DB_PROP_NONE, NULL_GUID, NO_ALLOC );
  ocrDbCreate( &sim_p->boxes.rpf, (void**)&rpf_p, sizeof(ocrGuid_t)*sim_p->boxes.boxes_num, DB_PROP_NONE, NULL_GUID, NO_ALLOC );

  //TODO: Use affinity hints here to distribute the datablocks across policy domains
  u32 b;
  for( b=0; b<sim_p->boxes.boxes_num; ++b ) {
    void* ptr;
    ocrDbCreate( box_p+b, &ptr, sizeof(box_t), DB_PROP_NO_ACQUIRE, NULL_GUID, NO_ALLOC );
    ocrDbCreate( rpf_p+b, &ptr, sizeof(rpf_t), DB_PROP_NO_ACQUIRE, NULL_GUID, NO_ALLOC );
  }

  ocrDbRelease( sim_p->boxes.box );
  ocrDbRelease( sim_p->boxes.rpf );

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
  ocrGuid_t uk_g; real_t* uk_p;
  ocrDbCreate( &uk_g, (void**)&uk_p, sizeof(real_t)*2, DB_PROP_NONE, NULL_GUID, NO_ALLOC );
  uk_p[0] = uk_p[1] = 0;

  real_t* dt = (real_t*)paramv;
  rpf_t* rpf = (rpf_t*)depv[0].ptr;
  mass_t* mass = (mass_t*)depv[1].ptr;

  u8 a;
  for( a=0; a<rpf->atoms; ++a ) {
    uk_p[0] += rpf->u[a];
    uk_p[1] += (rpf->p[a][0]*rpf->p[a][0]+rpf->p[a][1]*rpf->p[a][1]+rpf->p[a][2]*rpf->p[a][2])*
               mass->inv_mass_2[rpf->s[a]];
    rpf->p[a][0] += *dt*rpf->f[a][0];
    rpf->p[a][1] += *dt*rpf->f[a][1];
    rpf->p[a][2] += *dt*rpf->f[a][2];
  }

  ocrEventSatisfy( *(ocrGuid_t*)(paramv+1),uk_g );

  return NULL_GUID;
}

//params: dt, continuation
//depv: rpf, mass
ocrGuid_t veluk_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
  real_t* dt = (real_t*)paramv;
  rpf_t* rpf = (rpf_t*)depv[0].ptr;
  mass_t* mass = (mass_t*)depv[1].ptr;
  ocrGuid_t uk_g; real_t* uk_p;
  ocrDbCreate( &uk_g, (void**)&uk_p, sizeof(real_t)*2, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
  uk_p[0] = uk_p[1]= 0;

  u8 a;
  for( a=0; a<rpf->atoms; ++a ) {
    rpf->p[a][0] += *dt*rpf->f[a][0];
    rpf->p[a][1] += *dt*rpf->f[a][1];
    rpf->p[a][2] += *dt*rpf->f[a][2];
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
  reductionH_t* reductionH_p = (reductionH_t*)depv[0].ptr;
  rpf_t* rpf = (rpf_t*)depv[1].ptr;
  mass_t* mass = (mass_t*)depv[2].ptr;
  ocrGuid_t ek_g; real_t* ek_p;
  ocrDbCreate( &ek_g, (void**)&ek_p, sizeof(real_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC );
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

  ocrAddDependence( reductionH_p->OEVT_reduction, *(ocrGuid_t*)(paramv+1), 0, DB_MODE_RO );
  ocrAddDependence( ek_g, *(ocrGuid_t*)paramv, 0, DB_MODE_RO );

  return NULL_GUID;
}

//params: grid[0]-grid[1], grid[2]-lattice[0], lattice[1]-lattice[2], delta, temperature, vleaf, tleaf, uleaf
//{ sim_g<RO>, mass_g<RW>, reduction_g<RW>, ibox_g<RW>, irpf_g<RW>, sched0<RW>, sched1<RW>, ..., sched25<RW>, sched26<RO>, rb_g<RO> }
ocrGuid_t FNC_init( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{

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

#ifndef TG_ARCH
  memset( box_p,0,sizeof(box_t)); memset(rpf_p,0,sizeof(rpf_t));
#else
  for( u32 m=0; m<sizeof(box_t); ++m ) ((char*)box_p)[m]=0;
  for( u32 m=0; m<sizeof(rpf_t); ++m ) ((char*)rpf_p)[m]=0;
#endif

  box_p->coordinates[0] = ((u32*)paramv)[0];
  box_p->coordinates[1] = ((u32*)paramv)[1];
  box_p->coordinates[2] = ((u32*)paramv)[2];

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
  u32* lattice = ((u32*)paramv)+3;

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
  ocrDbCreate( &vcm_g, (void**)&vcm_p, sizeof(real_t)*4+sizeof(u64), DB_PROP_NONE, NULL_GUID, NO_ALLOC );
#ifndef TG_ARCH
  memset( vcm_p,0,sizeof(real_t)*4+sizeof(u64));
#else
  for( u32 m=0; m<sizeof(real_t)*4+sizeof(u64); ++m ) ((char*)vcm_p)[m]=0;
#endif
  u64* atoms = (u64*)(vcm_p+4);

  real_t delta = *(real_t*)(paramv+3);
  real_t temperature = *(real_t*)(paramv+4);

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
  ocrEdtCreate( &edt, tmp, 0, NULL, 3, NULL, EDT_PROP_NONE, NULL_GUID, &fin );
  ocrAddDependence( irpf_g, edt, 1, DB_MODE_RW );
  ocrAddDependence( sched26_g, edt, 2, DB_MODE_RO );
  ocrEdtTemplateDestroy( tmp );

  u64 pv[6];
  pv[0] = paramv[6]; pv[1] = (u64)edt;

  ocrEdtTemplateCreate( &tmp,vcm_edt,2,3 );
  ocrEdtCreate( &edt, tmp, 2, pv, 3, NULL, EDT_PROP_NONE, NULL_GUID, NULL );
  ocrAddDependence( reductionH_p->OEVT_reduction, edt, 0, DB_MODE_RO );
  ocrAddDependence( irpf_g, edt, 1, DB_MODE_RW );
  ocrAddDependence( mass_g, edt, 2, DB_MODE_RO );
  ocrEdtTemplateDestroy( tmp );

  if( sim_p->pot.potential&LJ) {
    pv[0] = *(u64*)&sim_p->pot.lj.sigma; pv[1] = *(u64*)&sim_p->pot.lj.epsilon;
    pv[2] = *(u64*)&sim_p->pot.cutoff; pv[3] = *(u64*)&sim_p->boxes.domain[0];
    pv[4] = *(u64*)&sim_p->boxes.domain[1]; pv[5] = *(u64*)&sim_p->boxes.domain[2];

    ocrGuid_t tfin=fin;
    ocrEdtTemplateCreate( &tmp,sim_p->pot.force_edt,6,27 );
    ocrEdtCreate( &edt, tmp, 6, pv, 27, NULL, EDT_PROP_NONE, NULL_GUID, &fin );
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

  *(real_t*)pv = 0.5; pv[1] = paramv[7];
  ocrEdtTemplateCreate( &tmp,ukvel_edt,2,2 );
  ocrEdtCreate( &edt, tmp, 2, pv, 2, NULL, EDT_PROP_NONE, NULL_GUID, NULL );
  ocrEdtTemplateDestroy( tmp );
  ocrAddDependence( fin, edt, 0, DB_MODE_RW );
  ocrAddDependence( mass_g, edt, 1, DB_MODE_RO );

  ocrEventSatisfy( *(ocrGuid_t*)(paramv+5), vcm_g );

  return NULL_GUID;
}

//params: grid[0]-grid[1], grid[2]-lattice[0], lattice[1]-lattice[2], delta, temperature
// { sim_g<RW>, box<RO>, rpf<RO> }
ocrGuid_t fork_init_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
  ocrGuid_t sim_g = depv[0].guid;
  ocrGuid_t box_g = depv[1].guid;
  ocrGuid_t rpf_g = depv[2].guid;

  simulation_t* sim_p = (simulation_t*)depv[0].ptr;
  ocrGuid_t* box_p = (ocrGuid_t*)depv[1].ptr;
  ocrGuid_t* rpf_p = (ocrGuid_t*)depv[2].ptr;

  u32* grid = (u32*)paramv;
  u32* lattice = grid+3;
  u32 ijk[16] = {0,0,0,lattice[0],lattice[1],lattice[2],0,0,0,0,0,0,0,0,0,0};
  ((u64*)ijk)[3] = paramv[3];
  ((u64*)ijk)[4] = paramv[4];

  ocrGuid_t sch_g, *sch_p;
  ocrDbCreate( &sch_g, (void**)&sch_p, sizeof( ocrGuid_t)*sim_p->boxes.boxes_num, DB_PROP_NONE, NULL_GUID, NO_ALLOC );

  u32 b; void* p;
  //TODO: affinity hints here
  for( b=0; b < sim_p->boxes.boxes_num; ++b )
    ocrDbCreate( sch_p+b, &p, sizeof(ocrGuid_t)*26, DB_PROP_NO_ACQUIRE, NULL_GUID, NO_ALLOC );

  reductionH_t* reductionH_p;
  ocrDbCreate( &sim_p->reductionH_g, (void**)&reductionH_p, sizeof(reductionH_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC );

  ocrGuid_t* leaves_p; ocrGuid_t leaves_g;
  ocrDbCreate( &leaves_g, (void**)&leaves_p, sizeof(ocrGuid_t)*sim_p->boxes.boxes_num*3, DB_PROP_NONE, NULL_GUID, NO_ALLOC );

  u64 red_paramv[3];
  red_paramv[1] = sim_p->pot.potential&LJ ? *(u64*)&sim_p->pot.lj.epsilon : 1;

  reductionH_p->OEVT_reduction = build_reduction( sim_g, sim_p->reductionH_g, sim_p->boxes.boxes_num, leaves_p+sim_p->boxes.boxes_num*2,
                                            2, red_paramv, ured_edt );

  red_paramv[1] = paramv[4]; red_paramv[2] = reductionH_p->OEVT_reduction;

  reductionH_p->OEVT_reduction = build_reduction( sim_g, sim_p->reductionH_g, sim_p->boxes.boxes_num, leaves_p+sim_p->boxes.boxes_num,
                                            3, red_paramv, tred_edt );

  red_paramv[1] = reductionH_p->OEVT_reduction;

  reductionH_p->OEVT_reduction = build_reduction( sim_g, sim_p->reductionH_g, sim_p->boxes.boxes_num, leaves_p,
                                            2, red_paramv, vred_edt );

  ocrDbRelease( sim_p->reductionH_g );

  ocrGuid_t tmp, EDT_init;
  ocrEdtTemplateCreate( &tmp,FNC_init,8,33 );

  u32 leaf = 0;
  for( ijk[0]=0; ijk[0]<grid[0]; ++ijk[0] )
  for( ijk[1]=0; ijk[1]<grid[1]; ++ijk[1] )
  for( ijk[2]=0; ijk[2]<grid[2]; ++ijk[2] ) {
    ((u64*)ijk)[5] = leaves_p[leaf]; //vred_leaf
    ((u64*)ijk)[6] = leaves_p[leaf+sim_p->boxes.boxes_num]; //tred_leaf
    ((u64*)ijk)[7] = leaves_p[leaf+sim_p->boxes.boxes_num*2]; //ured_leaf

    ocrEdtCreate( &EDT_init, tmp, 8, (u64*)ijk, 33, NULL, EDT_PROP_NONE, NULL_GUID, NULL );

    ocrAddDependence( sim_g, EDT_init, 0, DB_MODE_RO );
    ocrAddDependence( sim_p->pot.mass, EDT_init, 1, DB_MODE_RW );
    ocrAddDependence( sim_p->reductionH_g, EDT_init, 2, DB_MODE_RW );
    ocrAddDependence( box_p[leaf], EDT_init, 3, DB_MODE_RW );
    ocrAddDependence( rpf_p[leaf], EDT_init, 4, DB_MODE_RW );

    ocrGuid_t rb; ocrGuid_t* rbp;
    //TODO: affinity hints here?
    ocrDbCreate( &rb, (void**)&rbp, sizeof(ocrGuid_t)*52, DB_PROP_NONE, NULL_GUID, NO_ALLOC );

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

  return NULL_GUID;
}

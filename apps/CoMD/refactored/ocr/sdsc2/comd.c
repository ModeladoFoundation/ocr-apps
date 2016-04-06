#ifndef TG_ARCH
#include <time.h>
#endif
#include <string.h>
#include <ocr.h>

#include "comd.h"
#include "constants.h"
#include "timers.h"
#include "command.h"
#include "simulation.h"
#include "potentials.h"

static inline void timestamp( const char* msg )
{
#ifndef TG_ARCH
  time_t t = time( NULL );
  char* time_string = ctime( &t );
  time_string[24] = '\0';
  PRINTF( "%s: ", time_string );
#endif
  PRINTF( "%s\n", msg );
}

ocrGuid_t period_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t end_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_preInit( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t FNC_setUpGraph( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

// ( );
// { DBK_cmdLineArgs<RO>, timer_g<RW>, cmd_g<RW>, sim_g<RW> }
ocrGuid_t FNC_preInit( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;
    ocrGuid_t timer_g = depv[1].guid;
    ocrGuid_t cmd_g = depv[2].guid;
    ocrGuid_t sim_g = depv[3].guid;

    void* PTR_cmdLineArgs = depv[0].ptr;
    mdtimer_t* timers_p = depv[1].ptr;
    command_t* cmd_p = depv[2].ptr;
    simulation_t* sim_p = depv[3].ptr;

    u64 argc = getArgc( PTR_cmdLineArgs );
    ocrGuid_t argv_g;
    char** argv;
    ocrDbCreate( &argv_g, (void**)&argv, sizeof(char*)*argc, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    for( u32 a = 0; a < argc; ++a )
      argv[a] = getArgv( PTR_cmdLineArgs,a );

#ifndef TG_ARCH
    memset( timers_p, 0, sizeof(mdtimer_t)*number_of_timers );
#else
    for( u32 a = 0; a < sizeof(mdtimer_t)*number_of_timers; ++a ) ((char*)timers_p)[a]='\0';
#endif
    timestamp( "Starting Initialization\n");
    profile_start( total_timer, timers_p );

    parse_command( argc, argv, cmd_p );
    ocrDbDestroy( argv_g );

    u8 insane = init_simulation( cmd_p, sim_p, timers_p );
    ocrDbDestroy( DBK_cmdLineArgs );

    if( insane ) {
      ocrDbDestroy( timer_g );
      ocrDbDestroy( sim_g );
      timestamp( "CoMD ending with initialization error\n");
      ocrShutdown( );
    }

    return NULL_GUID;
}

// ( )
// { timer_g<RW>, cmd_g<RO>, sim_g<RW>, EVENT<RW> }
ocrGuid_t FNC_setUpGraph( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t timer_g = depv[0].guid;
    ocrGuid_t cmd_g = depv[1].guid;
    ocrGuid_t sim_g = depv[2].guid;

    mdtimer_t* timer_p = (mdtimer_t*)depv[0].ptr;
    command_t* cmd_p = depv[1].ptr;
    simulation_t* sim_p = (simulation_t*)depv[2].ptr;

    ocrGuid_t tmp_g;
    ocrGuid_t EDT_init, EDT_compute, EDT_finalize;

    PRM_fork_init_edt_t PRM_fork_init_edt;
    PRM_fork_init_edt.grid[0] = sim_p->boxes.grid[0];
    PRM_fork_init_edt.grid[1] = sim_p->boxes.grid[1];
    PRM_fork_init_edt.grid[2] = sim_p->boxes.grid[2];
    PRM_fork_init_edt.lattice[0] = cmd_p->nx;
    PRM_fork_init_edt.lattice[1] = cmd_p->ny;
    PRM_fork_init_edt.lattice[2] = cmd_p->nz;
    PRM_fork_init_edt.delta = cmd_p->delta;
    PRM_fork_init_edt.temperature = cmd_p->temperature;

    ocrGuid_t OET_init, OETS_init;
    ocrEventCreate( &OETS_init, OCR_EVENT_STICKY_T, false );

    ocrGuid_t TML_fork_init;
    ocrEdtTemplateCreate( &TML_fork_init, fork_init_edt, sizeof(PRM_fork_init_edt_t)/sizeof(u64), 3 );
    ocrEdtCreate( &EDT_init, TML_fork_init, 5, (u64*)&PRM_fork_init_edt, 3, NULL, EDT_PROP_FINISH, PICK_1_1(NULL_HINT,NULL_GUID), &OET_init );
    ocrEdtTemplateDestroy( TML_fork_init );

    ocrAddDependence( OET_init, OETS_init, 0, DB_MODE_NULL );

    ocrAddDependence( sim_g, EDT_init, 0, DB_MODE_RW );
    ocrAddDependence( sim_p->boxes.box, EDT_init, 1, DB_MODE_RO );
    ocrAddDependence( sim_p->boxes.rpf, EDT_init, 2, DB_MODE_RO );

    ocrGuid_t TML_end;
    ocrEdtTemplateCreate( &TML_end, end_edt, 0, 6 );
    ocrEdtCreate( &EDT_finalize, TML_end, 0, NULL, 6, NULL, EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );
    ocrEdtTemplateDestroy( TML_end );

    if( sim_p->steps ) {
        ocrGuid_t TML_period;
        PRM_period_edt_t PRM_period_edt = { .EDT_finalize = EDT_finalize };
        ocrEdtTemplateCreate( &TML_period, period_edt, sizeof(PRM_period_edt_t)/sizeof(u64), 6 );
        ocrEdtCreate( &EDT_compute, TML_period, 1, (u64*)&PRM_period_edt, 6, NULL, EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );
        //EDT_compute sets up the 6th dependency for EDT_finalize
        ocrEdtTemplateDestroy( TML_period );

        ocrAddDependence( timer_g, EDT_compute, 0, DB_MODE_RW );
        ocrAddDependence( sim_g, EDT_compute, 1, DB_MODE_RW );
        ocrAddDependence( sim_p->pot.mass, EDT_compute, 2, DB_MODE_RO );
        ocrAddDependence( sim_p->boxes.box, EDT_compute, 3, DB_MODE_RO );
        ocrAddDependence( sim_p->boxes.rpf, EDT_compute, 4, DB_MODE_RO );
        ocrAddDependence( OETS_init, EDT_compute, 5, DB_MODE_NULL );
    }
    else
        ocrAddDependence( OETS_init, EDT_finalize, 5, DB_MODE_NULL );

    ocrAddDependence( timer_g, EDT_finalize, 0, DB_MODE_RW );
    ocrAddDependence( sim_g, EDT_finalize, 1, DB_MODE_RW );
    ocrAddDependence( sim_p->boxes.box, EDT_finalize, 2, DB_MODE_RO );
    ocrAddDependence( sim_p->boxes.rpf, EDT_finalize, 3, DB_MODE_RO );
    ocrAddDependence( sim_p->pot.mass, EDT_finalize, 4, DB_MODE_RO );

    ocrDbDestroy( cmd_g );

    return NULL_GUID;
}

ocrGuid_t mainEdt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;

    ocrGuid_t timer_g;
    mdtimer_t* timers_p;
    ocrDbCreate( &timer_g, (void**)&timers_p, sizeof(mdtimer_t)*number_of_timers, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    ocrGuid_t cmd_g;
    command_t* cmd_p;
    ocrDbCreate( &cmd_g, (void**)&cmd_p, sizeof(command_t), DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    ocrGuid_t sim_g;
    simulation_t* sim_p;
    ocrDbCreate( &sim_g, (void**)&sim_p, sizeof(simulation_t), DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

    ocrGuid_t TML_preInit;
    ocrGuid_t EDT_preInit;

    ocrGuid_t OET_preInit;
    ocrGuid_t OET_preInit_Sticky;
    ocrEventCreate( &OET_preInit_Sticky, OCR_EVENT_STICKY_T, false );

    ocrEdtTemplateCreate( &TML_preInit, FNC_preInit, 0, 4 );
    ocrEdtCreate( &EDT_preInit, TML_preInit, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), &OET_preInit );

    ocrAddDependence( OET_preInit, OET_preInit_Sticky, 0, DB_MODE_NULL );

    ocrAddDependence( DBK_cmdLineArgs, EDT_preInit, 0, DB_MODE_RO );
    ocrAddDependence( timer_g, EDT_preInit, 1, DB_MODE_RW );
    ocrAddDependence( cmd_g, EDT_preInit, 2, DB_MODE_RW );
    ocrAddDependence( sim_g, EDT_preInit, 3, DB_MODE_RW );

    ocrGuid_t TML_setUpGraph;
    ocrGuid_t EDT_setUpGraph;

    ocrEdtTemplateCreate( &TML_setUpGraph, FNC_setUpGraph, 0, 4 );
    ocrEdtCreate( &EDT_setUpGraph, TML_setUpGraph, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );

    ocrAddDependence( timer_g, EDT_setUpGraph, 0, DB_MODE_RW );
    ocrAddDependence( cmd_g, EDT_setUpGraph, 1, DB_MODE_RO );
    ocrAddDependence( sim_g, EDT_setUpGraph, 2, DB_MODE_RW );
    ocrAddDependence( OET_preInit_Sticky, EDT_setUpGraph, 3, DB_MODE_NULL );

    return NULL_GUID;
}

void print_status( simulation_t* sim_p, mass_t* mass, double time )
{
  real_t energy = (sim_p->e_potential + sim_p->e_kinetic) / sim_p->atoms;
  if( sim_p->step==0 ) {
    sim_p->energy0 = energy;
    PRINTF( "\nInitial energy : %f, atom count : %u\n\n", energy, sim_p->atoms0 );
    timestamp( "Starting simulation\n");
    PRINTF( "#                                                                                         Performance\n");
    PRINTF( "#  Loop   Time(fs)       Total Energy   Potential Energy     Kinetic Energy  Temperature   (us/atom)     # Atoms\n");
  }

  real_t e_k = sim_p->e_kinetic / sim_p->atoms;
  real_t e_u = sim_p->e_potential / sim_p->atoms;
  real_t temp = e_k / kB_eV_1_5;

  int nEval = (sim_p->step % sim_p->period == 0) ? sim_p->period : (sim_p->step % sim_p->period);
  if( sim_p->step == 0 ) nEval = 1;

  double time_per_atom = 1.0e6*time/(double)(nEval*sim_p->atoms);
  PRINTF( "%7d %10.2f %18.12f %18.12f %18.12f %12.4f %10.4f %12d\n",
         sim_p->step, sim_p->step*sim_p->dt, energy, e_u, e_k, temp, time_per_atom, sim_p->atoms );
}

//depv: timer, sim, box, rpf, mass
ocrGuid_t end_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation_t* sim_p = (simulation_t*)depv[1].ptr;
  mdtimer_t* timer_p = (mdtimer_t*)depv[0].ptr;

  profile_stop( total_timer, timer_p );
  print_status( sim_p, (mass_t*)depv[4].ptr, get_elapsed_time( total_timer, timer_p ));
  timestamp( "Ending simulation\n");

  real_t energy = (sim_p->e_potential + sim_p->e_kinetic) / sim_p->atoms;
  PRINTF( "\nSimulation Validation:\n");
  PRINTF( "  Initial energy  : %14.12f\n", sim_p->energy0 );
  PRINTF( "  Final energy    : %14.12f\n", energy );
  PRINTF( "  eFinal/eInitial : %f\n", energy/sim_p->energy0 );

  int atoms_diff = sim_p->atoms - sim_p->atoms0;
  if( atoms_diff == 0 )
    PRINTF( "  Final atom count : %d, no atoms lost\n", sim_p->atoms );
  else {
    PRINTF( "#############################\n");
    PRINTF( "# WARNING: %6d atoms lost #\n", atoms_diff );
    PRINTF( "#############################\n");
  }

  print_timers( timer_p, sim_p->atoms, sim_p->steps );

  ocrGuid_t* box = (ocrGuid_t*) depv[2].ptr;
  ocrGuid_t* rpf = (ocrGuid_t*) depv[3].ptr;
  u32 n = sim_p->boxes.boxes_num;
  while( n ) {
    --n;
    ocrDbDestroy( box[n] );
    ocrDbDestroy( rpf[n] );
  }
  ocrDbDestroy( depv[2].guid );
  ocrDbDestroy( depv[3].guid );
  ocrDbDestroy( sim_p->reductionH_g );
  ocrDbDestroy( sim_p->pot.mass );

  if( sim_p->pot.potential&EAM ) {
    ocrDbDestroy( sim_p->pot.eam.phi.values );
    ocrDbDestroy( sim_p->pot.eam.rho.values );
    ocrDbDestroy( sim_p->pot.eam.f.values );
  }
  ocrDbDestroy( depv[0].guid );
  ocrDbDestroy( depv[1].guid );

  timestamp( "CoMD Ending\n");
  ocrShutdown( );

  return NULL_GUID;
}

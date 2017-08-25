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
  ocrPrintf( "%s: ", time_string );
#endif
  ocrPrintf( "%s\n", msg );
}

ocrGuid_t period_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t end_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t FNC_preInit( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t FNC_setUpGraph( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

// ( );
// { DBK_cmdLineArgs<RO>, timer_g<RW>, cmd_g<RW>, simH_g<RW> }
ocrGuid_t FNC_preInit( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;
    ocrGuid_t timer_g = depv[1].guid;
    ocrGuid_t cmd_g = depv[2].guid;
    ocrGuid_t simH_g = depv[3].guid;

    void* PTR_cmdLineArgs = depv[0].ptr;
    mdtimer_t* timers_p = depv[1].ptr;
    command_t* cmd_p = depv[2].ptr;
    simulationH_t* simH_p = depv[3].ptr;

    u64 argc = ocrGetArgc( PTR_cmdLineArgs );
    ocrGuid_t argv_g;
    char** argv;
    ocrDbCreate( &argv_g, (void**)&argv, sizeof(char*)*argc, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    for( u32 a = 0; a < argc; ++a )
      argv[a] = ocrGetArgv( PTR_cmdLineArgs,a );

#ifndef TG_ARCH
    memset( timers_p, 0, sizeof(mdtimer_t)*number_of_timers );
#else
    for( u32 a = 0; a < sizeof(mdtimer_t)*number_of_timers; ++a ) ((char*)timers_p)[a]='\0';
#endif
    timestamp( "Starting Initialization\n");
    profile_start( total_timer, timers_p );

    parse_command( argc, argv, cmd_p );
    ocrDbDestroy( argv_g );

    u8 insane = init_simulation( cmd_p, simH_p, timers_p );
    ocrDbDestroy( DBK_cmdLineArgs );

    if( insane ) {
      ocrDbDestroy( timer_g );
      ocrDbDestroy( simH_g );
      timestamp( "CoMD ending with initialization error\n");
      ocrShutdown( );
    }

    return NULL_GUID;
}

// ( )
// { timer_g<RW>, cmd_g<RO>, simH_g<RW>, EVENT<RW> }
ocrGuid_t FNC_setUpGraph( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t timer_g = depv[0].guid;
    ocrGuid_t cmd_g = depv[1].guid;
    ocrGuid_t simH_g = depv[2].guid;

    mdtimer_t* timer_p = (mdtimer_t*)depv[0].ptr;
    command_t* cmd_p = depv[1].ptr;
    simulationH_t* simH_p = (simulationH_t*)depv[2].ptr;

    ocrGuid_t tmp_g;
    ocrGuid_t EDT_init, EDT_compute, EDT_finalize;

    PRM_EDT_init_fork_t PRM_EDT_init_fork;
    PRM_EDT_init_fork.grid[0] = simH_p->boxDataStH.grid[0];
    PRM_EDT_init_fork.grid[1] = simH_p->boxDataStH.grid[1];
    PRM_EDT_init_fork.grid[2] = simH_p->boxDataStH.grid[2];
    PRM_EDT_init_fork.lattice[0] = cmd_p->nx;
    PRM_EDT_init_fork.lattice[1] = cmd_p->ny;
    PRM_EDT_init_fork.lattice[2] = cmd_p->nz;
    PRM_EDT_init_fork.delta = cmd_p->delta;
    PRM_EDT_init_fork.temperature = cmd_p->temperature;

    ocrGuid_t OET_init, OETS_init;
    ocrEventCreate( &OETS_init, OCR_EVENT_STICKY_T, false );

    ocrGuid_t TML_fork_init;
    ocrEdtTemplateCreate( &TML_fork_init, EDT_init_fork, sizeof(PRM_EDT_init_fork_t)/sizeof(u64), 3 );
    ocrEdtCreate( &EDT_init, TML_fork_init, 5, (u64*)&PRM_EDT_init_fork, 3, NULL, EDT_PROP_FINISH, PICK_1_1(NULL_HINT,NULL_GUID), &OET_init );
    ocrEdtTemplateDestroy( TML_fork_init );

    ocrAddDependence( OET_init, OETS_init, 0, DB_MODE_NULL );

    ocrAddDependence( simH_g, EDT_init, 0, DB_MODE_RW );
    ocrAddDependence( simH_p->boxDataStH.DBK_linkCellGuidsH, EDT_init, 1, DB_MODE_RO );
    ocrAddDependence( simH_p->boxDataStH.DBK_atomDataGuidsH, EDT_init, 2, DB_MODE_RO );

    ocrGuid_t TML_end;
    ocrEdtTemplateCreate( &TML_end, end_edt, 0, 6 );
    ocrEdtCreate( &EDT_finalize, TML_end, 0, NULL, 6, NULL, EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );
    ocrEdtTemplateDestroy( TML_end );

    if( simH_p->steps ) {
        ocrGuid_t TML_period;
        PRM_period_edt_t PRM_period_edt = { .EDT_finalize = EDT_finalize };
        ocrEdtTemplateCreate( &TML_period, period_edt, sizeof(PRM_period_edt_t)/sizeof(u64), 6 );
        ocrEdtCreate( &EDT_compute, TML_period, 1, (u64*)&PRM_period_edt, 6, NULL, EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );
        //EDT_compute sets up the 6th dependency for EDT_finalize
        ocrEdtTemplateDestroy( TML_period );

        ocrAddDependence( timer_g, EDT_compute, 0, DB_MODE_RW );
        ocrAddDependence( simH_g, EDT_compute, 1, DB_MODE_RW );
        ocrAddDependence( simH_p->pot.mass, EDT_compute, 2, DB_MODE_RO );
        ocrAddDependence( simH_p->boxDataStH.DBK_linkCellGuidsH, EDT_compute, 3, DB_MODE_RO );
        ocrAddDependence( simH_p->boxDataStH.DBK_atomDataGuidsH, EDT_compute, 4, DB_MODE_RO );
        ocrAddDependence( OETS_init, EDT_compute, 5, DB_MODE_NULL );
    }
    else
        ocrAddDependence( OETS_init, EDT_finalize, 5, DB_MODE_NULL );

    ocrAddDependence( timer_g, EDT_finalize, 0, DB_MODE_RW );
    ocrAddDependence( simH_g, EDT_finalize, 1, DB_MODE_RW );
    ocrAddDependence( simH_p->boxDataStH.DBK_linkCellGuidsH, EDT_finalize, 2, DB_MODE_RO );
    ocrAddDependence( simH_p->boxDataStH.DBK_atomDataGuidsH, EDT_finalize, 3, DB_MODE_RO );
    ocrAddDependence( simH_p->pot.mass, EDT_finalize, 4, DB_MODE_RO );

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

    ocrGuid_t simH_g;
    simulationH_t* simH_p;
    ocrDbCreate( &simH_g, (void**)&simH_p, sizeof(simulationH_t), DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );

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
    ocrAddDependence( simH_g, EDT_preInit, 3, DB_MODE_RW );

    ocrGuid_t TML_setUpGraph;
    ocrGuid_t EDT_setUpGraph;

    ocrEdtTemplateCreate( &TML_setUpGraph, FNC_setUpGraph, 0, 4 );
    ocrEdtCreate( &EDT_setUpGraph, TML_setUpGraph, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL );

    ocrAddDependence( timer_g, EDT_setUpGraph, 0, DB_MODE_RW );
    ocrAddDependence( cmd_g, EDT_setUpGraph, 1, DB_MODE_RO );
    ocrAddDependence( simH_g, EDT_setUpGraph, 2, DB_MODE_RW );
    ocrAddDependence( OET_preInit_Sticky, EDT_setUpGraph, 3, DB_MODE_NULL );

    return NULL_GUID;
}

void print_status( simulationH_t* simH_p, mass_t* mass, double time )
{
  real_t energy = (simH_p->e_potential + simH_p->e_kinetic) / simH_p->atoms;
  if( simH_p->step==0 ) {
    simH_p->energy0 = energy;
    ocrPrintf( "\nInitial energy : %f, atom count : %u\n\n", energy, simH_p->atoms0 );
    timestamp( "Starting simulation\n");
    ocrPrintf( "#                                                                                         Performance\n");
    ocrPrintf( "#  Loop   Time(fs)       Total Energy   Potential Energy     Kinetic Energy  Temperature   (us/atom)     # Atoms\n");
  }

  real_t e_k = simH_p->e_kinetic / simH_p->atoms;
  real_t e_u = simH_p->e_potential / simH_p->atoms;
  real_t temp = e_k / kB_eV_1_5;

  int nEval = (simH_p->step % simH_p->period == 0) ? simH_p->period : (simH_p->step % simH_p->period);
  if( simH_p->step == 0 ) nEval = 1;

  double time_per_atom = 1.0e6*time/(double)(nEval*simH_p->atoms);
  ocrPrintf( "%7d %10.2f %18.12f %18.12f %18.12f %12.4f %10.4f %12d\n",
         simH_p->step, simH_p->step*simH_p->dt, energy, e_u, e_k, temp, time_per_atom, simH_p->atoms );
}

//depv: timer, sim, DBK_linkCellGuidsH, DBK_atomDataGuidsH, mass
ocrGuid_t end_edt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulationH_t* simH_p = (simulationH_t*)depv[1].ptr;
  mdtimer_t* timer_p = (mdtimer_t*)depv[0].ptr;

  profile_stop( total_timer, timer_p );
  print_status( simH_p, (mass_t*)depv[4].ptr, get_elapsed_time( total_timer, timer_p ));
  timestamp( "Ending simulation\n");

  real_t energy = (simH_p->e_potential + simH_p->e_kinetic) / simH_p->atoms;
  ocrPrintf( "\nSimulation Validation:\n");
  ocrPrintf( "  Initial energy  : %14.12f\n", simH_p->energy0 );
  ocrPrintf( "  Final energy    : %14.12f\n", energy );
  ocrPrintf( "  eFinal/eInitial : %f\n", energy/simH_p->energy0 );

  int atoms_diff = simH_p->atoms - simH_p->atoms0;
  if( atoms_diff == 0 )
    ocrPrintf( "  Final atom count : %d, no atoms lost\n", simH_p->atoms );
  else {
    ocrPrintf( "#############################\n");
    ocrPrintf( "# WARNING: %6d atoms lost #\n", atoms_diff );
    ocrPrintf( "#############################\n");
  }

  print_timers( timer_p, simH_p->atoms, simH_p->steps );

  ocrGuid_t* PTR_linkCellGuids = (ocrGuid_t*) depv[2].ptr;
  ocrGuid_t* PTR_atomDataGuids = (ocrGuid_t*) depv[3].ptr;
  u32 n = simH_p->boxDataStH.b_num;
  while( n ) {
    --n;
    ocrDbDestroy( PTR_linkCellGuids[n] );
    ocrDbDestroy( PTR_atomDataGuids[n] );
  }
  ocrDbDestroy( depv[2].guid );
  ocrDbDestroy( depv[3].guid );
  ocrDbDestroy( simH_p->reductionH_g );
  ocrDbDestroy( simH_p->pot.mass );

  if( simH_p->pot.potential&EAM ) {
    ocrDbDestroy( simH_p->pot.eam.phi.values );
    ocrDbDestroy( simH_p->pot.eam.rho.values );
    ocrDbDestroy( simH_p->pot.eam.f.values );
  }
  ocrDbDestroy( depv[0].guid );
  ocrDbDestroy( depv[1].guid );

  timestamp( "CoMD Ending\n");
  ocrShutdown( );

  return NULL_GUID;
}

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

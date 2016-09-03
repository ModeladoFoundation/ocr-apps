/// \file
/// CoMD data structures.

#ifndef __COMDTYPES_H_
#define __COMDTYPES_H_

#include <stdio.h>

#ifndef TG_ARCH
#include <time.h>
#endif

#include "mytype.h"
#include "mycommand.h" //TODO
#include "haloExchange.h"
#include "linkCells.h"
#include "decomposition.h"
#include "initAtoms.h"
#include "performanceTimers.h"

#include "reduction.h"

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

struct SimFlatSt;

/// The base struct from which all potentials derive.  Think of this as an
/// abstract base class.
///
/// CoMD uses the following units:
///  - distance is in Angstroms
///  - energy is in eV
///  - time is in fs
///  - force in in eV/Angstrom
///
///  The choice of distance, energy, and time units means that the unit
///  of mass is eV*fs^2/Angstrom^2.  Hence, we must convert masses that
///  are input in AMU (atomic mass units) into internal mass units.
typedef struct BasePotentialSt
{
   real_t cutoff;          //!< potential cutoff distance in Angstroms
   real_t mass;            //!< mass of atoms in intenal units
   real_t lat;             //!< lattice spacing (angs) of unit cell
   char latticeType[8];    //!< lattice type, e.g. FCC, BCC, etc.
   char  name[3];	   //!< element name
   int	 atomicNo;	   //!< atomic number
   //int  (*force)(struct SimFlatSt* s); //!< function pointer to force routine
   ocrGuid_t (*force_edt)(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
   void (*print)(struct BasePotentialSt* pot);
   //void (*destroy)(struct BasePotentialSt** pot); //!< destruction of the potential
} BasePotential;


/// species data: chosen to match the data found in the setfl/funcfl files
typedef struct SpeciesDataSt
{
   char  name[3];   //!< element name
   int	 atomicNo;  //!< atomic number
   real_t mass;     //!< mass in internal units
} SpeciesData;

/// Simple struct to store the initial energy and number of atoms.
/// Used to check energy conservation and atom conservation.
typedef struct ValidateSt
{
   double eTot0; //<! Initial total energy
   int nAtoms0;  //<! Initial global number of atoms
} Validate;


typedef struct
{
    ocrGuid_t haloRangeGUID;
    ocrGuid_t KeReductionRangeGUID, VcmReductionRangeGUID, maxOccupancyReductionRangeGUID;
    ocrGuid_t perfTimerReductionRangeGUID;
    //ocrGuid_t EVT_OUT_norm_reduction, EVT_OUT_timer_reduction;
    ocrEVT_t finalOnceEVT;
} globalOcrParamH_t;

typedef struct
{
    Command cmdParamH;
    globalOcrParamH_t ocrParamH;
} globalParamH_t;

typedef struct
{
    ocrGuid_t TML_FNC_Lsend;
    ocrGuid_t TML_FNC_Rsend;
    ocrGuid_t TML_FNC_Lrecv;
    ocrGuid_t TML_FNC_Rrecv;
    ocrGuid_t TML_FNC_Bsend;
    ocrGuid_t TML_FNC_Tsend;
    ocrGuid_t TML_FNC_Brecv;
    ocrGuid_t TML_FNC_Trecv;
    ocrGuid_t TML_FNC_update;
} rankTemplateH_t;

typedef struct
{
    s64 nRanks, myRank;
    globalParamH_t globalParamH;
    //rankParamH_t rankParamH;
    Command cmd;
    rankTemplateH_t rankTemplateH;

    //ocrDBK_t DBK_sim;

    ocrGuid_t DBK_xIn, DBK_xOut, DBK_weight;
    ocrGuid_t DBK_LsendBufs[2], DBK_RsendBufs[2];
    ocrGuid_t DBK_TsendBufs[2], DBK_BsendBufs[2];
    ocrGuid_t DBK_refNorm;

    ocrGuid_t DBK_timers;

    ocrDBK_t rpKeDBK, rpVcmDBK, rpmaxOccupancyDBK, rpPerfTimerDBK;
    reductionPrivate_t *rpKePTR, *rpVcmPTR, *rpmaxOccupancyPTR, *rpPerfTimerPTR;
    ocrEVT_t rpKeEVT, rpVcmEVT, rpmaxOccupancyEVT, rpPerfTimerEVT;

    ocrHint_t myEdtAffinityHNT;
    ocrHint_t myDbkAffinityHNT;
    ocrGuid_t haloRangeGUID;
    ocrGuid_t haloSendEVTs[6];
    ocrGuid_t haloRecvEVTs[6];
    ocrGuid_t haloTagSendEVTs[6];
    ocrGuid_t haloTagRecvEVTs[6];
} rankH_t;

///
/// The fundamental simulation data structure with MAXATOMS in every
/// link cell.
///
typedef struct SimFlatSt
{
   int nSteps;            //<! number of time steps to run
   int printRate;         //<! number of steps between output
   double dt;             //<! time step

   Domain* domain;        //<! domain decomposition data
   Domain domain_INST;

   LinkCell* boxes;       //<! link-cell data
   LinkCell boxes_INST;

   Atoms* atoms;          //<! atom data (positions, momenta, ...)
   Atoms atoms_INST;

   SpeciesData* species;  //<! species data (per species, not per atom)
   SpeciesData species_INST;

   real_t ePotential;     //!< the total potential energy of the system
   real_t eKinetic;       //!< the total kinetic energy of the system

   Validate* validate;
   Validate validate_INST;

   BasePotential *pot;	  //!< the potential
   ocrDBK_t DBK_pot;

   HaloExchange* atomExchange;
   HaloExchange atomExchange_INST;

   Timers perfTimer[numberOfTimers];
   TimerGlobal perfGlobal;

   rankH_t* PTR_rankH; //TODO

} SimFlat;


#endif

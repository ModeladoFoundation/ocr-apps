/// \file
/// CoMD data structures.

#ifndef __COMDTYPES_H_
#define __COMDTYPES_H_

#include <stdio.h>
#include "mytype.h"
#include "mycommand.h" //TODO
#include "haloExchange.h"
#include "linkCells.h"
#include "decomposition.h"
#include "initAtoms.h"

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
   int  (*force)(struct SimFlatSt* s); //!< function pointer to force routine
   void (*print)(FILE* file, struct BasePotentialSt* pot);
   void (*destroy)(struct BasePotentialSt** pot); //!< destruction of the potential
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

   LinkCell* boxes;       //<! link-cell data

   Atoms* atoms;          //<! atom data (positions, momenta, ...)

   SpeciesData* species;  //<! species data (per species, not per atom)

   real_t ePotential;     //!< the total potential energy of the system
   real_t eKinetic;       //!< the total kinetic energy of the system

   BasePotential *pot;	  //!< the potential

   HaloExchange* atomExchange;

} SimFlat;

typedef struct
{
    ocrGuid_t haloRangeGUID, normReductionRangeGUID, timerReductionRangeGUID;
    ocrGuid_t EVT_OUT_norm_reduction, EVT_OUT_timer_reduction;
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
    globalParamH_t globalParamH;
    //rankParamH_t rankParamH;
    Command cmd;
    rankTemplateH_t rankTemplateH;

    ocrGuid_t DBK_xIn, DBK_xOut, DBK_weight;
    ocrGuid_t DBK_LsendBufs[2], DBK_RsendBufs[2];
    ocrGuid_t DBK_TsendBufs[2], DBK_BsendBufs[2];
    ocrGuid_t DBK_refNorm;

    ocrGuid_t DBK_timers;

    ocrGuid_t DBK_norm_reductionH; //->reductionPrivate_t
    ocrGuid_t DBK_timer_reductionH;

    ocrHint_t myEdtAffinityHNT;
    ocrHint_t myDbkAffinityHNT;
    ocrGuid_t haloRangeGUID;
    ocrGuid_t haloSendEVTs[4];
    ocrGuid_t haloRecvEVTs[4];
} rankH_t;


#endif

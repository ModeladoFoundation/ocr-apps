/// \file
/// Compute forces for the Embedded Atom Model (EAM).

#ifndef __EAM_H
#define __EAM_H

#include "mytype.h"
#include "comd.h"

#ifdef CNCOCR_TG
static inline int fprintf(FILE *s, const char *f, ...) {
  return 0;
}

static inline int fscanf(FILE *stream, const char *format, ...) {
  return 0;
}

static inline int sscanf(const char *str, const char *format, ...) {
  return 0;
}

static inline int sprintf(char *str, const char *format, ...) {
  return 0;
}
#endif

struct BasePotentialSt;
struct LinkCellSt;

/// Pointers to the data that is needed in the load and unload functions
/// for the force halo exchange.
/// \see loadForceBuffer
/// \see unloadForceBuffer
typedef struct ForceExchangeDataSt
{
   real_t* dfEmbed; //<! derivative of embedding energy
   struct LinkCellSt* boxes;
}ForceExchangeData;


/// Handles interpolation of tabular data.
///
/// \see initInterpolationObject
/// \see interpolate
typedef struct InterpolationObjectSt
{
   int n;          //!< the number of values in the table
   real_t x0;      //!< the starting ordinate range
   real_t invDx;   //!< the inverse of the table spacing
   real_t* values; //!< the abscissa values
} InterpolationObject;

/// Derived struct for an EAM potential.
/// Uses table lookups for function evaluation.
/// Polymorphic with BasePotential.
/// \see BasePotential
typedef struct EamPotentialSt
{
   real_t cutoff;          //!< potential cutoff distance in Angstroms
   real_t mass;            //!< mass of atoms in intenal units
   real_t lat;             //!< lattice spacing (angs) of unit cell
   char latticeType[8];    //!< lattice type, e.g. FCC, BCC, etc.
   char  name[3];      //!< element name
   int   atomicNo;     //!< atomic number
   InterpolationObject* phi;  //!< Pair energy
   InterpolationObject* rho;  //!< Electron Density
   InterpolationObject* f;    //!< Embedding Energy

   real_t* rhobar;        //!< per atom storage for rhobar
   real_t* dfEmbed;       //!< per atom storage for derivative of Embedding
//   HaloExchange* forceExchange;
//   ForceExchangeData* forceExchangeData;
} EamPotential;

struct BasePotentialSt* initEamPot(const char* dir, const char* file, const char* type);
#endif

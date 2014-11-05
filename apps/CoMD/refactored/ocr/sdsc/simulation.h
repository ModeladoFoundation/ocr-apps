#ifndef COMD_H
#define COMD_H

#include <ocr.h>

#include "timers.h"
#include "command.h"

#define MAXATOMS 64
#define MAXSAMPLES 64
#define CONT_DEPC 3

// The units for this code are:
//     - Time in femtoseconds (fs)
//     - Length in Angstroms (Angs)
//     - Energy in electron Volts (eV)
//     - Force in eV/Angstrom
//     - Mass read in as Atomic Mass Units (amu) and then converted for
//       consistency (energy*time^2/length^2)
// Values are taken from NIST, http://physics.nist.gov/cuu/Constants/

/// 1 amu in kilograms
#define amu2Kilograms  1.660538921e-27

/// 1 fs in seconds
#define fs2seconds     1.0e-15

/// 1 Ang in meters
#define Angs2meters    1.0e-10

/// 1 eV in Joules
#define eV2Joules      1.602176565e-19

/// Internal mass units are eV * fs^2 / Ang^2
static const double amu2internal_mass =
         amu2Kilograms * Angs2meters * Angs2meters
         / (fs2seconds * fs2seconds  * eV2Joules);

/// Boltmann constant in eV's
static const double kB_eV = 8.6173324e-5;  // eV/K

/// Conversion to Temperature
static const double kB_eV_1_5 = 7736.34618837;  // K/eV

/// Hartrees to eVs
static const double hartree2Ev = 27.21138505;

/// Bohrs to Angstroms
static const double bohr2Angs = 0.52917721092;

#ifdef SINGLE
typedef float real_t;
  #define FMT1 "%f"
  #define EMT1 "%f"
#else
typedef double real_t;
  #define FMT1 "%f"
  #define EMT1 "%f"
#endif

typedef real_t real3[3];

static inline void zeroReal3(real3 a)
{
  a[0] = a[1] = a[2] = 0.0;
}

typedef struct
{
  real_t sigma;
  real_t epsilon;
} lj_potential;

typedef struct
{
  u32 n;                      //!< the number of values in the table
  real_t x0;                  //!< the starting ordinate range
  real_t xn;                  //!< ending of ordinate range
  real_t inv_dx;              //!< the inverse of the table spacing
  ocrGuid_t values;           //!< the abscissa values
} interpolator;

typedef struct
{
  interpolator phi;  //!< Pair energy
  interpolator rho;  //!< Electron Density
  interpolator f;    //!< Embedding Energy
} eam_potential;

typedef struct simulation simulation;

typedef struct potential
{
  real_t cutoff;                       //!< potential cutoff distance in Angstroms
  real_t mass;                         //!< mass of atoms in intenal units
  real_t inv_mass_2;                   //!< mass of atoms in intenal units
  real_t inv_mass_dt;                  //!< mass of atoms in intenal units
  real_t lat;                          //!< lattice spacing (angs) of unit cell
  char lattice_type[8];                //!< lattice type, e.g. FCC, BCC, etc.
  char name[3];	                       //!< element name
  u32	atomic_no;	               //!< atomic number
  void (*fork_force_computation)
       (ocrGuid_t sim, simulation* sim_ptr, ocrGuid_t cont, ocrGuid_t* list, u32 grid[3]);
                                       //!< function pointer to force routine
  void (*print)(struct potential* p);  //!< function pointer to print routine
  union {
    lj_potential lj;
    eam_potential eam;
  };
} potential;

typedef struct
{
  u32 grid[3];         //!< number of boxes in each dimension on processor
  u32 boxes_num;       //!< total number of local boxes on processor
  real3 local_min;     //!< minimum local bounds on processor
  real3 local_max;     //!< maximum local bounds on processor
  real3 box_size;      //!< size of box in each dimension
  real3 inv_box_size;  //!< inverse size of box in each dimension
  real3 shift[27];     //!< shift for periodic boundaries
  u32 atoms;           //!< total number of atoms on this processor
  u32 max_occupancy;   //!< max number of atoms in a cell
  ocrGuid_t list;      //!< guid of the list of guids of the boxes
} boxes;

typedef struct
{
  u32 gid[MAXATOMS];  //!< A globally unique id for each atom
  real3 r[MAXATOMS];  //!< positions
  real3 p[MAXATOMS];  //!< momenta of atoms
  real3 f[MAXATOMS];  //!< forces
  real_t U[MAXATOMS]; //!< potential energy per atom
  real_t a[MAXATOMS]; //!< accumulator and temporary value, used in EAM
  u32 atoms;          //!< total number of atoms in the box
} box;

struct simulation
{
  u32 step;             //<! current step
  u32 steps;            //<! number of time steps to run
  u32 period;           //<! number of steps between output
  real_t dt;            //<! time step

  potential pot;	 //!< the potential

  boxes bxs;            //<! link-cell data

  real_t e_potential;   //!< the total potential energy of the system
  real_t e_kinetic;     //!< the total kinetic energy of the system

  double energy0;       //<! Initial total energy
  u32 atoms0;           //<! Initial global number of atoms
};

u8 init_simulation(command* c, simulation* s, timer* t, ocrGuid_t** list);
void destroy_simulation(void* s, void* l);

u8 init_eam(const char* d, char* n, const char* t, potential* p, real_t dt);
void init_lj(potential* p, real_t dt);

void print_status_header();
void print_status(simulation* s, double time);
void print_simulation(simulation* s);
void validate_result(simulation* sim);

void redistribute_atoms(simulation* sim, box** bxs_ptr, u32 boxes_num);
void kinetic_energy(simulation* sim, box** bxs_ptr, u32 boxes_num);
void compute_force(simulation* s);

void fork_kinetic_energy(ocrGuid_t sim, ocrGuid_t cont, u32 depc, ocrGuid_t* list, u32 boxes_num);
void fork_advance_position(ocrGuid_t sim, ocrGuid_t cont, u32 depc, ocrGuid_t* list, u32 boxes_num);
void fork_advance_velocity(ocrGuid_t sim, ocrGuid_t cont, u32 depc, ocrGuid_t* list, u32 boxes_num, real_t step);

void timestep(ocrGuid_t timer, void* timer_ptr, ocrGuid_t sim, ocrGuid_t cont,
              ocrGuid_t list, ocrGuid_t* list_ptr, u32 boxes_num);

#endif

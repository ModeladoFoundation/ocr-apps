#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>
#include <getopt.h>
#include <ocr.h>

#include "comd.h"

#define MAXOPTIONS 17
#define MAXSTRINGLEN 1024

typedef struct
{
  char help[MAXSTRINGLEN];
  char long_arg[MAXSTRINGLEN];
  char short_arg;
  char type;
  void* ptr;
  u32 sz;
} option_t;

typedef struct
{
  char pot_dir[MAXSTRINGLEN];  //!< the directory where EAM potentials reside
  char pot_name[MAXSTRINGLEN]; //!< the name of the potential
  char pot_type[MAXSTRINGLEN]; //!< the type of the potential (funcfl or setfl)
  u32 doeam;                   //!< a flag to determine whether we're running EAM potentials
  u32 nx;                      //!< number of unit cells in x
  u32 ny;                      //!< number of unit cells in y
  u32 nz;                      //!< number of unit cells in z
  u32 steps;                   //!< number of time steps to run
  u32 period;                  //!< number of steps between output
  real_t dt;                   //!< time step (in femtoseconds)
  real_t lat;                  //!< lattice constant (in Angstroms)
  real_t temperature;          //!< simulation initial temperature (in Kelvin)
  real_t delta;                //!< magnitude of initial displacement from lattice (in Angstroms)

  u32 longest;
  u32 options_num;
  u32 args_num;
  option_t options[MAXOPTIONS];
  char args[MAXOPTIONS*2+1];
  struct option long_opt[MAXOPTIONS+1];
} command_t;

void parse_command(int argc, char** argv, command_t* cmd);

#endif

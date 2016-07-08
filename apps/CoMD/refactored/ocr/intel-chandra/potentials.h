#ifndef POTENTIALS_H
#define POTENTIALS_H

#include "comd.h"

enum potentials {LJ=1,EAM=2,SETFL=4,FUNCFL=8,Cu=16};
enum lattices {CUBIC=0, FCC=1, BCC=2};

#define FSHIFT_X(m,n) (((n%3==0)&&(m&1))?-shift[0] :(((n%3==2)&&(m&2))?shift[0]:0))
#define FSHIFT_Y(m,n) (((n%9<=2)&&(m&4))?-shift[1] :(((n%9>=6)&&(m&8))?shift[1]:0))
#define FSHIFT_Z(m,n) (( n<9&&(m&16)   )?-shift[2] :(( n>17&&(m&32))?shift[2]:0))

#define LATTICE_NAME(l) (l==FCC?"FCC":(l==BCC?"BCC":"CUBIC"))

void init_lj(potential_t* p, real_t dt);

u8 init_eam(const char* pot_dir, const char* pot_name, const char* pot_type, potential_t* p, real_t dt);

#endif

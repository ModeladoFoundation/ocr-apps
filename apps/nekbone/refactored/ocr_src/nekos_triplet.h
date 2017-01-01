#ifndef NEKOS_TRIPLET_H
#define NEKOS_TRIPLET_H

typedef long Coord_t;
typedef long Idz;  //An accumulation of Coord_t is an Idz.

#define NOTA_IDZ -1L

typedef struct Triplet_proto  //Keep it POD
{
    Idz a,b,c;

}Triplet;

void assignT(Triplet * io_destination, Triplet * in_source);

//Returns 1 if in_point is in the lattice described by in_lattice.
int isinLattice(Triplet in_point, Triplet in_lattice);

Idz abcT(Triplet in); //Returns (a*b*c)
Idz abT(Triplet in); //Returns (a*b)

void addlongT(Triplet * io, long value);
void addT(Triplet * io, Triplet in_src); //io += in_src
void subT(Triplet * io, Triplet in_src); //io -= in_src
void mulT(Triplet * io, Triplet in_src); //io *= in_src

int is_equalT(Triplet lhs, Triplet rhs); //Returns 1 if true
int lessthanT(Triplet lhs, Triplet rhs); //Returns 1 if lhs < rhs

char * triplet2text(char * buf, Triplet in);

#endif // NEKOS_TRIPLET_H

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

Idz Coords_to_index(Triplet in_location, Triplet in_lattice);
Triplet index_to_coords(Idz in_id, Triplet in_lattice);

Triplet build_latticeRE(Triplet in_E, Triplet in_R);
Triplet build_coordinateRE(Triplet in_Elattice, Triplet in_e, Triplet in_r);
void unbuild_coordinateRE(Triplet in_re, Triplet in_Elattice, Triplet * o_e, Triplet * o_r);

//Here bigCoord & biglattice refer to the RE lattice made up from the Rank lattice and the Element lattice.
//gdid is a global ID on the overall system, unique for each disconnect coordinates r,e,d.
Idz disconect_to_gdid(Triplet in_d, Triplet in_bigCoord, Triplet in_Dlattice, Triplet in_biglattice);
void gdid_to_disconect(Idz in_gdid, Triplet in_Dlattice, Triplet in_biglattice, Triplet * o_d, Triplet * o_bigCoord);

//These 3 functions attempt to move a DOF on a boundary of an element to the next
//element.  DOF internal to an element are left intact.
int elementMoveX(Triplet in_refDlattice, Triplet * io_d, Triplet * io_re);
int elementMoveY(Triplet in_refDlattice, Triplet * io_d, Triplet * io_re);
int elementMoveZ(Triplet in_refDlattice, Triplet * io_d, Triplet * io_re);

typedef struct Candidate
{
    Triplet d;
    Triplet re;  //The is the coordinate on the RE lattice, where all elements
                 //are laid out flat.  RElattice = E .* R
    Idz gdid; //global disconnected index.  This involves the R,E,&D lattices.

    Triplet e; //The e coordinate extracted from this->re.
    Triplet r; //The r coordinate extracted from this->re.
    Idz rid; //This is the rid build from this->r;
    Idz de;  //This is the de build from this->e and this->d;

} Candidate_t;

void candidate2text(char * io_buf, Candidate_t in_can);


#endif // NEKOS_TRIPLET_H

#ifndef NEKOS_TRIPLET_H
#include "nekos_triplet.h"
#endif

void assignT(Triplet * io_destination, Triplet * in_source)
{
    io_destination->a = in_source->a;
    io_destination->b = in_source->b;
    io_destination->c = in_source->c;
}

//Returns 1 if in_point is in the lattice described by in_lattice.
int isinLattice(Triplet in_point, Triplet in_lattice)
{
    if(in_point.a < 0 || in_point.a >= in_lattice.a) return 0;
    if(in_point.b < 0 || in_point.b >= in_lattice.b) return 0;
    if(in_point.c < 0 || in_point.c >= in_lattice.c) return 0;
    return 1;
}

Idz abcT(Triplet in)
{
    return (in.a * in.b * in.c);
}
Idz abT(Triplet in)
{
    return (in.a * in.b);
}

void addlongT(Triplet * io, long value)
{
    io->a += value;     io->b += value;     io->c += value;
}

void addT(Triplet * io, Triplet in_src)
{
    io->a += in_src.a;     io->b += in_src.b;     io->c += in_src.c;
}

void subT(Triplet * io, Triplet in_src)
{
    io->a -= in_src.a;     io->b -= in_src.b;     io->c -= in_src.c;
}

void mulT(Triplet * io, Triplet in_src)
{
    io->a *= in_src.a;     io->b *= in_src.b;     io->c *= in_src.c;
}

int is_equalT(Triplet lhs, Triplet rhs)
{
    if (lhs.a==rhs.a && lhs.b==rhs.b && lhs.c==rhs.c) return 1;
    return 0;
}

int lessthanT(Triplet lhs, Triplet rhs)
{
    if(lhs.a == rhs.a){
        if(lhs.b == rhs.b){
            if( lhs.c < rhs.c ) return 1;
            return 0;
        } else{
            if( lhs.b < rhs.b ) return 1;
            return 0;
        }
    } else{
        if( lhs.a < rhs.a ) return 1;
        return 0;
    }
}






























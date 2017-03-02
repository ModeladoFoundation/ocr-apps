#ifndef NEKOS_TRIPLET_H
#include "nekos_triplet.h"
#endif

#include <stdio.h>//sprintf

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

char * triplet2text(char * buf, Triplet in)
{
    sprintf(buf,"(%ld,%ld,%ld)", in.a, in.b, in.c);
    return buf;
}

Idz Coords_to_index(Triplet in_location, Triplet in_lattice)
{
    Idz x = in_location.a;
    x += in_lattice.a * in_location.b;
    x += in_lattice.a * in_lattice.b * in_location.c;
    return x;
}

Triplet index_to_coords(Idz in_id, Triplet in_lattice)
{
    Triplet p = {0};
    p.c = in_id /(in_lattice.a * in_lattice.b);
    in_id = in_id - p.c * in_lattice.a * in_lattice.b;
    p.b = in_id / in_lattice.a;
    p.a = in_id - p.b * in_lattice.a;
    return p;
}

Triplet build_latticeRE(Triplet in_E, Triplet in_R)
{
    Triplet t;
    t.a = in_R.a * in_E.a;
    t.b = in_R.b * in_E.b;
    t.c = in_R.c * in_E.c;  //The flatten total element count across all ranks.
    return t;
}

Triplet build_coordinateRE(Triplet in_Elattice, Triplet in_e, Triplet in_r)
{
    Triplet re;
    re.a = in_e.a + in_Elattice.a * in_r.a;
    re.b = in_e.b + in_Elattice.b * in_r.b;
    re.c = in_e.c + in_Elattice.c * in_r.c;
    return re;
}

void unbuild_coordinateRE(Triplet in_re, Triplet in_Elattice, Triplet * o_e, Triplet * o_r)
{
    o_r->a = in_re.a / in_Elattice.a;
    o_e->a = in_re.a - o_r->a * in_Elattice.a;

    o_r->b = in_re.b / in_Elattice.b;
    o_e->b = in_re.b - o_r->b * in_Elattice.b;

    o_r->c = in_re.c / in_Elattice.c;
    o_e->c = in_re.c - o_r->c * in_Elattice.c;
}

Idz disconect_to_gdid(Triplet in_d, Triplet in_bigCoord, Triplet in_Dlattice, Triplet in_biglattice)
{
    const Idz d_id  = Coords_to_index(in_d, in_Dlattice);
    const Idz big_id = Coords_to_index(in_bigCoord, in_biglattice);
    const Idz gdid = d_id + abcT(in_Dlattice) * big_id;
    return gdid;
}
void gdid_to_disconect(Idz in_gdid, Triplet in_Dlattice, Triplet in_biglattice, Triplet * o_d, Triplet * o_bigCoord)
{
    Idz big_id = in_gdid / abcT(in_Dlattice);
    Idz d_id = in_gdid - abcT(in_Dlattice) * big_id;

    *o_d  = index_to_coords(d_id, in_Dlattice);
    *o_bigCoord = index_to_coords(big_id, in_biglattice);
}

int elementMoveX(Triplet in_refDlattice, Triplet * io_d, Triplet * io_re)
{
    int check = 0;
    if(io_d->a == 0){
        io_d->a = in_refDlattice.a - 1;
        --io_re->a;
        check = -1;
    } else if(io_d->a == in_refDlattice.a -1){
        io_d->a = 0;
        ++io_re->a;
        check = 1;
    } else {
        check = 0; //Nothing changes.
    }

    return check;
}

int elementMoveY(Triplet in_refDlattice, Triplet * io_d, Triplet * io_re)
{
    int check = 0;
    if(io_d->b == 0){
        io_d->b = in_refDlattice.b - 1;
        --io_re->b;
        check = -1;
    } else if(io_d->b == in_refDlattice.b -1){
        io_d->b = 0;
        ++io_re->b;
        check = 1;
    } else {
        check = 0; //Nothing changes.
    }

    return check;
}

int elementMoveZ(Triplet in_refDlattice, Triplet * io_d, Triplet * io_re)
{
    int check = 0;
    if(io_d->c == 0){
        io_d->c = in_refDlattice.c - 1;
        --io_re->c;
        check = -1;
    } else if(io_d->c == in_refDlattice.c -1){
        io_d->c = 0;
        ++io_re->c;
        check = 1;
    } else {
        check = 0; //Nothing changes.
    }

    return check;
}

void candidate2text(char * io_buf, Candidate_t in_can)
{
    char buff_d[128];
    char buff_re[128];
    char buff_e[128];
    char buff_r[128];
    sprintf(io_buf, "d=%s\tre=%s\tgdid=%ld\te=%s\tr=%s %ld de=%ld",
            triplet2text(buff_d, in_can.d),
            triplet2text(buff_re, in_can.re),
            in_can.gdid,
            triplet2text(buff_e, in_can.e),
            triplet2text(buff_r, in_can.r),
            in_can.rid, in_can.de
            );
}



























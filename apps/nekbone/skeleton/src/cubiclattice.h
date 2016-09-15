#ifndef CUBIC_LATTICE_H
#define CUBIC_LATTICE_H

#include "triplet.h"

namespace CUBICLAT
{
    static Triplet id2tripletZ(Triplet in_partition, Idz in_idz)
    {
        //in_partition : the full count of blocks in the (a|b|c)-axis direction
        //in_id :  an offset of the current block in the lattice
        //          0 <= in_id < in_partition.abc()
        // Returns the tuplet (x,y,z) of the block specified by in_id
        //      0<= x < in_partition.a
        //      0<= y < in_partition.b
        //      0<= z < in_partition.c
        // The returned triplet will in the Z-ordering.
        Triplet t;
        const Idz ab = in_partition.ab();
        t.c = in_idz / ab;
        in_idz -= t.c * ab;
        t.b = in_idz / in_partition.a;
        t.a = in_idz - t.b * in_partition.a;
        return t;
    }

    static Idz tripletZ2id(Triplet in_partition, Triplet in_locationZ)
    {
        //in_partition : the full count of blocks in the (a|b|c)-axis direction
        //in_location : the location (x,y,z) of the current block within the lattice
        //              ordered in the Z-ordering.
        // Returns the id associate with in_location
        Idz idz =0;
        idz = in_locationZ.a + in_locationZ.b * in_partition.a + in_locationZ.c * in_partition.ab();
        return idz;
    }

    //=Assume that the cell of interest in the center of a 3 by 3 by 3 arrangements
    // of cells.
    //=Then the cell of interest has 26 neighbors (3^3-1).
    //=Given the cell of interest Triplet Tcoi, then each neighbors can be represented
    // by displacement from Tcoi as follows:
    //      Tneighbor = Tcoi +Triplet(u,v,t)
    // where (u,v,t) are chosen to correspond unity move in the direction
    //                   neighbor cell.
    //=Thus one can establish a one-to-one relation between a neighbor ID
    // and these (u,v,t) direction
    //=The following function helps keeping track of this, using the following
    // grid numbering
    //
    //      Level Z=-1          Level Z=0           Level Z=+1
    //    Y                    Y                   Y
    //    ^ 6   7   8          ^15  16  17         ^24  25  26
    //    | 3   4   5          |12  13  14         |21  22  23
    //    | 0   1   2          |9   10  11         |18  19  20
    //      -----> X            -----> X            -----> X
    //  The center node is at DIRid=13 and location (0,0,0).
    //  DIRid=0  is at location (x,y,z)=(-1,-1,-1)
    //  DIRid=2  is at location (x,y,z)=(1,-1,-1)
    //  DIRid=17 is at location (x,y,z)=(1,1,0)
    //  DIRid=25 is at location (x,y,z)=(0,1,1)

    typedef long DIRid; //NB for neighbor
    enum {
         NOTA_DIRid = -1
        ,COI_DIRid  = 13  //The DIRid of the cell of interest, i.e. the middle one.
        ,CELLCOUNT_IN_NEIGHBORHOOD = 27
    };

    static DIRid tripletNB2dirId(Triplet in_tripletNB)
    {
        Triplet B = in_tripletNB;
        if( B.a <-1 || 1 <B.a ) return NOTA_DIRid;
        if( B.b <-1 || 1 <B.b ) return NOTA_DIRid;
        if( B.c <-1 || 1 <B.c ) return NOTA_DIRid;

        B += Triplet(1,1,1);

        DIRid q = B.a  + 3*B.b+ 9*B.c;
        return q;
    }

    static Triplet dirId2tripletNB(DIRid did)
    {
        Triplet t;
        t.c = did /9;
        did -= t.c *9;
        t.b = did / 3;
        t.a = did - t.b *3;
        t -= Triplet(1,1,1);
        return t;
    }

    enum Interaction{
        //These following enum defines how two neighboring cells interact:
          actSELF = -1 // The 2 cells are the same cell!
        , actZERO = 0 //  zero --> the two cells share nothing  //Must be equal to zero.
        , actCORNER = 1 //  corner --> the 2 cells share a common corner
        , actEDGE = 2 //  edge --> the 2 cells share a common edge, and 2 corners
        , actFACE = 3 //  face --> the 2 cells share a common face, and 4 edges and 4 corners
    };

    static Interaction dirId2interaction(DIRid did)
    {
        if(did==COI_DIRid) return actSELF;
        if(did<0 || CELLCOUNT_IN_NEIGHBORHOOD<= did ) return actZERO;
        Triplet t = dirId2tripletNB(did);
        int zeroCount = 0;
        if(t.a ==0) ++zeroCount;
        if(t.b ==0) ++zeroCount;
        if(t.c ==0) ++zeroCount;

        switch(zeroCount){
            case 0:  return actCORNER;  break;
            case 1:  return actEDGE;    break;
            case 2:  return actFACE;    break;
            case 3:  return actSELF;    break;
            default: return actZERO;    break; //Weird?
        }
        return actZERO; //One should never get here.
    }

    static Interaction t2tAction(Triplet in_tripletZ_A, Triplet in_tripletZ_B)
    {
        Triplet A = in_tripletZ_A;
        Triplet B = in_tripletZ_B;
        if( B < A ){
            Triplet T = B;
            B = A;
            A = T;
        }

        B -= A;

            //Check the range.  Keep only immediate neighbors.
        if( B.a <-1 || 1 < B.a ) return actZERO;
        if( B.b <-1 || 1 < B.b ) return actZERO;
        if( B.c <-1 || 1 < B.c ) return actZERO;

        int zeroCount = 0;
        if(B.a ==0) ++zeroCount;
        if(B.b ==0) ++zeroCount;
        if(B.c ==0) ++zeroCount;

        switch(zeroCount){
            case 0:  return actCORNER;  break;
            case 1:  return actEDGE;    break;
            case 2:  return actFACE;    break;
            case 3:  return actSELF;    break;
            default: return actZERO;    break; //Weird?
        }
        return actZERO; //One should never get here.
    }

    static Interaction id2idAction(Triplet in_partition, Idz in_idz_A, Idz in_idz_B)
    {
        Triplet A = id2tripletZ(in_partition, in_idz_A);
        Triplet B = id2tripletZ(in_partition, in_idz_B);
        return t2tAction(A,B);
    }

}//namespace CUBICLAT
#endif //CUBIC_LATTICE_H

#ifndef NEKOS_CUBIC_LATTICE_H
#define NEKOS_CUBIC_LATTICE_H

#include "nekos_triplet.h"

//NOTE: The documentation for the function is in the .c file right nest to the function.

Triplet id2tripletZ(Triplet in_partition, Idz in_idz);
Idz tripletZ2id(Triplet in_partition, Triplet in_locationZ);

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
#define NOTA_DIRid -1
#define COI_DIRid  13  //The DIRid of the cell of interest, i.e. the middle one.
#define CELLCOUNT_IN_NEIGHBORHOOD 27

DIRid tripletNB2dirId(Triplet in_tripletNB);
Triplet dirId2tripletNB(DIRid did);

typedef enum Interaction_proto{
    //These following enum defines how two neighboring cells interact:
      IAactSELF = -1 // The 2 cells are the same cell!
    , IAactZERO = 0 //  zero --> the two cells share nothing  //Must be equal to zero.
    , IAactCORNER = 1 //  corner --> the 2 cells share a common corner
    , IAactEDGE = 2 //  edge --> the 2 cells share a common edge, and 2 corners
    , IAactFACE = 3 //  face --> the 2 cells share a common face, and 4 edges and 4 corners
} Interaction;

Interaction dirId2interaction(DIRid did);
Interaction t2tAction(Triplet in_tripletZ_A, Triplet in_tripletZ_B);
Interaction id2idAction(Triplet in_partition, Idz in_idz_A, Idz in_idz_B);
#endif //NEKOS_CUBIC_LATTICE_H

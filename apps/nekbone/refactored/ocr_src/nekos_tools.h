#ifndef NEKOS_TOOLS_H
#define NEKOS_TOOLS_H

#include "app_ocr_err_util.h"
#include "nekos_cubiclattice.h"
#include "neko_globals.h"

//struct neighborLoad does the following
//  For the current rank ID,  it has to send "load" bytes
//  to the neighboring rank indicated by "rid".
//Each rank can have at most (27-1) neighbors.
//Note that construction DIRid = 13 is not possible.

typedef struct NeighborLoad
{
    Idz rid; //ID of the neighboring rank
    Interaction interaction; //How the neighbor interact with the current rank.
    DIRid did; //Direction of the neighbor with respect to the current rank.

    unsigned long disconnectedDOF; //The count of disconnected DOF
                                   // in common with that other rank
    unsigned long ownedConnectDOF; //=The count of owned connected DOF.
                                   //=A connected DOF is a DOF on the global grid
                                   // for the entire geometry, i.e. see NekCGsim::globPartP
                                   //=A DOF is owned by the current rank if
                                   // the current rank number is bigger than this-->rid.
                                   // Ownership is a way to avoid multiple counting
                                   // for DOF shared among many ranks.
} NeighborLoad_t;

//In the original Nekbone skeleton code, the class NekCGsim had many member data.
//These were all put in this NekosTools struct.
typedef struct NekosTools
{
    int mpiRank; //The rank of the current process
                 //It is also the rank ID number for the current process inside this->globPartR.

    //PARTITIONS:
    //
    //The local partitions tell how the components are split within one set:
    //      partR   --> rank distribution within the main object
    //      partE   --> element distribution within one rank
    //      partP   --> DOF distribution within one element
    //
    //The global partitions tell how a component are split over the entire object
    //      globpartR --> How all the ranks split the entire object
    //      globpartE --> How all the elements split the entire object
    //      globpartP --> How all the DOFs split the entire object
    //
    //  so   globpartR < globpartE < globpartP

    Triplet partR; //The partition along the 3 main axis for the ranks
    Triplet partE; //The partition along the 3 main axis for the element within a single rank.
    Triplet partP; //The DOF count along the 3 main axis within a single element

    Triplet globPartR; //Partition over the entire mesh for only the ranks, i.e. = this->partR.
                       // This gives the number of ranks along each of the 3 main axis.
    Triplet globPartE; //Partition over the entire mesh for all the elements
                       // This gives the number of elements along each of the 3 main axis.
    Triplet globPartP; //Partition over the entire mesh for all the DOFS.
                       // This gives the number of DOF along each of the 3 main axis.

    int sz_nloads;  //The number of entry in nloads.
    NeighborLoad_t nloads[NEKbone_neighborCount]; //In a cubic lattice each rank can have at most 26 neighbors.

    char dir_present[NEKbone_regionCount]; //Set to 1 if this->nloads.did is present; otherwise zero.

    unsigned int largest_countDOFdisco; //The largest number of DOF in any given (send|receive) in this->nloads().

    unsigned long total_shared_nodes;
} NEKOtools_t;

Err_t init_NEKOtools(NEKOtools_t * io, NEKOstatics_t in_nstatics, unsigned int in_rankID,
                     unsigned int in_pDOF);

Err_t destroy_NEKOtools(NEKOtools_t * io);
Err_t copy_NEKOtools(NEKOtools_t * in_from, NEKOtools_t * o_target);
void  print_NEKOtools(NEKOtools_t * in, int in_output_neighborhoods); // In Nekbone skeleton, this is NekCGsim::debug_PrintNeighbor(...)
                                                                       //=Set in_output_neighborhoods to 1 to get debug output
                                                                       // Otherwise set it to zero.

Err_t makeMesh(NEKOtools_t * io, Triplet in_R, Triplet in_E, unsigned int in_pDOF);

Err_t make_neighbors_loads2(NEKOtools_t * io);
Err_t make_connected_neighbors_loads(NEKOtools_t * io, int in_polyOrder);


#endif // NEKOS_TOOLS_H

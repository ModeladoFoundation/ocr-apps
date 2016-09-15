#ifndef NEKCGSIM_H
#define NEKCGSIM_H
#include <mpi.h>

#include "cubiclattice.h"
#include "sizedata.h"
#include "app_ocr_err_util.h"

void debug_PrintSize(int in_rank=0, bool in_outputOnlyForRank0=false);

class NekCGsim
{
public:
    NekCGsim();
    ~NekCGsim();

    Err_t create(int argc, char * argv[]);
    Err_t destroy();
    Err_t run();

private:
    //struct neighborLoad does the following
    //  For the current rank this->mpiRank, it has to send "load" bytes
    //  to the neighboring rank indicated by "rid".
    //Each rank can have at most (27-1) neighbors.
    struct NeighborLoad
    {
        Idz rid; //ID of the neighboring rank
        CUBICLAT::Interaction interaction; //How the neighbor interact with the current rank.
        CUBICLAT::DIRid did; //Direction of the neighbor with respect to the current rank.

        unsigned long disconnectedDOF; //The count of disconnected DOF
                                       // in common with that other rank
        unsigned long ownedConnectDOF; //=The count of owned connected DOF.
                                       //=A connected DOF is a DOF on the global grid
                                       // for the entire geometry, i.e. see NekCGsim::globPartP
                                       //=A DOF is owned by the current rank if
                                       // the current rank number is bigger than this-->rid.
                                       // Ownership is a way to avoid multiple counting
                                       // for DOF shared among many ranks.
    };

    int check_finalize; //Only do MPI_Finalize if zero.
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
    NeighborLoad nloads[SD::NeighborCount]; //In a cubic lattice each rank can have at most 26 neighbors.
    unsigned int largest_countDOFdisco; //The largest number of DOF in any given (send|receive) in this->nloads().

    unsigned long total_shared_nodes;

    //===== Methods
    Err_t Initial_checks();
    Err_t makeMesh(int in_polyOrder, bool in_output_debug);
    Err_t make_neighbors_loads();
    Err_t make_neighbors_loads2();
    Err_t make_connected_neighbors_loads(int in_polyOrder);

    Err_t clearMesh();

    Err_t conjugateGradient();
    Err_t nek_gsop(const char * in_text); //Note that in_text is not use.  It is just for documentation.
    Err_t nek_glsc3();

    void debug_PrintNeighbor(bool in_output_neighborhoods);

    //===== Disabled Methods
    NekCGsim( const NekCGsim & rhs);
    NekCGsim & operator=(const NekCGsim & rhs);
};

#endif // NEKCGSIM_H

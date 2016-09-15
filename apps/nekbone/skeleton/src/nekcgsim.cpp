#   ifndef NEKCGSIM_H
#       include "nekcgsim.h"
#   endif

#include <stdio.h>
#include <string.h> //memset

#include "cubiclattice.h"

#include <vector> //Neede only for NekCGsim::nek_gsop(), during initial implementation.

#define CHECKMPIERR if(mpierr!=MPI_SUCCESS){err=__LINE__; IFEB;}

void debug_PrintSize(int in_rank, bool in_outputOnlyForRank0)
{
    if(in_outputOnlyForRank0 && in_rank!=0){
        return;
    }
    printf("DBGps>%d-%d> (RX, RY, RZ) = (%d, %d, %d)\tRtotal = %d\n", in_rank, __LINE__, (int)SD::Rx,(int)SD::Ry,(int)SD::Rz, (int)SD::Rtotal);
    printf("DBGps>%d-%d> (EX, EY, EZ) = (%d, %d, %d)\tEtotal = %d\n", in_rank, __LINE__, (int)SD::Ex,(int)SD::Ey,(int)SD::Ez, (int)SD::Etotal);
    printf("DBGps>%d-%d> (Pb, Pe, Ps) = (%d, %d, %d)\n",              in_rank, __LINE__, (int)SD::Pbegin,(int)SD::Pend,(int)SD::Pstep);
    fflush(stdout);
}

void NekCGsim::debug_PrintNeighbor(bool in_output_neighborhoods)
{
    printf("DBGpn>N>%d RANK = %d\t", this->mpiRank,this->mpiRank);

    printf("DBGpn>R(%d, %d, %d)", this->partR.a, this->partR.b, this->partR.c);
    printf("DBGpn>E(%d, %d, %d)", this->partE.a, this->partE.b, this->partE.c);
    printf("DBGpn>P(%d, %d, %d)\t", this->partP.a, this->partP.b, this->partP.c);

    printf("DBGpn>gR(%d, %d, %d)", this->globPartR.a, this->globPartR.b, this->globPartR.c);
    printf("DBGpn>gE(%d, %d, %d)", this->globPartE.a, this->globPartE.b, this->globPartE.c);
    printf("DBGpn>gP(%d, %d, %d)\n", this->globPartP.a, this->globPartP.b, this->globPartP.c);

    printf("DBGpn>N>%d total_shared_nodes = %lu\n", this->mpiRank, this->total_shared_nodes);

    if(in_output_neighborhoods)
    for(int i=0; i<this->sz_nloads; ++i){
        printf("DBGpn>N>%-8d  -->  R=%-8d\tDIRid=%-8ld  %2d\tddof=%-6lu\tocDof=%-6lu\n"
                                     , this->mpiRank
                                     , this->nloads[i].rid
                                     , this->nloads[i].did
                                     , int(this->nloads[i].interaction)
                                     , this->nloads[i].disconnectedDOF
                                     , this->nloads[i].ownedConnectDOF
                                     );
    }
    fflush(stdout);
}

NekCGsim::NekCGsim( const NekCGsim & rhs)
:check_finalize(0), mpiRank(rhs.mpiRank),
 partR(), partE(), partP(), globPartR(), globPartE(), globPartP(),
 sz_nloads(0), //nloads
 largest_countDOFdisco(0), total_shared_nodes(0)
{}

NekCGsim & NekCGsim::operator=(const NekCGsim & rhs) { if(this!=&rhs){} return *this; }

NekCGsim::NekCGsim()
:check_finalize(0), mpiRank(0),
 partR(), partE(), partP(), globPartR(), globPartE(), globPartP(),
 sz_nloads(0), //nloads
 largest_countDOFdisco(0), total_shared_nodes(0)
{}

NekCGsim::~NekCGsim()
{
    Err_t err=this->destroy();
    if(err){
        return; //Did somebody say throw?
    }
    return;
}

Err_t NekCGsim::create(int argc, char * argv[])
{
    Err_t err=0;
    int mpierr = MPI_SUCCESS;

    while(!err){
        if( sizeof(long) != 8){
            //triplet.h::Idz assumes an 8-byte long.
            //If that is not the case, one run the risk of an overflow
            //when calculating IDs in cubiclattice.h
            //So flag this as an error for now.
            err=__LINE__; IFEB;
        }

        {//Keep this code block together, i.e. lookout for check_finalize.
            err=this->destroy(); if(err) break;
            mpierr = MPI_Init(&argc, &argv); CHECKMPIERR;
            ++this->check_finalize;
        }

        err = this->Initial_checks(); IFEB;
        break;
    }
    return err;
}

Err_t NekCGsim::destroy()
{
    Err_t err=0;
    while(!err){
        if(this->check_finalize>0){
            MPI_Finalize();
            this->check_finalize=0;
        }

        this->mpiRank = 0;

        err = this->clearMesh(); IFEB;

        //Do not delete/reset these
        //process_id;
        //request_vc;
        //response_vc

        break;
    }
    return err;
}

Err_t NekCGsim::run()
{
    Err_t err=0;
    int mpierr = MPI_SUCCESS;

    while(!err){

        for(int polyO=SD::Pbegin; polyO<SD::Pend; polyO+=SD::Pstep)
        {
            if("debug") if(this->mpiRank==0) {printf("RUN> %d> polyO=%d\n", __LINE__, polyO); fflush(stdout);}

            //NEKbone loop over element/rank removed-->for(G->nelt = G->iel0; G->nelt <= G->ielN; G->nelt += G->ielD){
            //Use sizedata.h::E(x|y|z) to change the element distribution within 1 rank.

                const bool output_debug=true;

                err = this->makeMesh(polyO,output_debug); IFEB;

                err = this->nek_gsop("on c"); IFEB;

                err = this->nek_gsop("on f"); IFEB;

                err = this->conjugateGradient(); IFEB;

                mpierr = MPI_Barrier(MPI_COMM_WORLD); CHECKMPIERR;

                err = this->conjugateGradient(); IFEB;

                err = this->clearMesh(); IFEB;

            //NEKbone loop over element/rank removed-->}

        }//for(int polyO=
        if(err) break;

        break;
    }
    return err;
}

Err_t  NekCGsim::Initial_checks()
{
    Err_t err=0;
    int mpierr = MPI_SUCCESS;
    while(!err){

        if(SD::Rx <=0 || SD::Ry<=0 || SD::Rz<=0) err=__LINE__; IFEB;
        if(SD::Ex <=0 || SD::Ey<=0 || SD::Ez<=0) err=__LINE__; IFEB;
        if(SD::Pbegin >= SD::Pend || SD::Pstep<=0) err=__LINE__; IFEB;

        int numtasks=0;
        mpierr = MPI_Comm_size(MPI_COMM_WORLD, &numtasks); CHECKMPIERR;
        if(numtasks != SD::Rtotal) {
                printf("Rank requested=%d\t Rank given=%d\n", SD::Rtotal, numtasks); fflush(stdout);
                err=__LINE__; IFEB;
        }

        mpierr = MPI_Comm_rank(MPI_COMM_WORLD, &this->mpiRank); CHECKMPIERR;

        if( this->mpiRank <0 || SD::Rtotal <= this->mpiRank ) err=__LINE__; IFEB;

        if(1) debug_PrintSize(this->mpiRank, true);
        break;
    }
    return err;
}

Err_t NekCGsim::makeMesh(int in_polyOrder, bool in_output_debug)
{
    Err_t err=0;
    while(!err){
        this->partR = Triplet(SD::Rx, SD::Ry, SD::Rz);
        this->partE = Triplet(SD::Ex, SD::Ey, SD::Ez);
        this->partP = Triplet(in_polyOrder+1, in_polyOrder+1, in_polyOrder+1);

        //The formula for a regular grid on a line segment with T partition
        //and P dof per partition is
        //  Total DOF = (T*P) - T +1

        // So for this->globPartR --> The same as this->partR
        this->globPartR = this->partR;

        // So for this->globPartE --> T is this->globPartR, and P is this->partE
        this->globPartE = this->partE;
        this->globPartE *= this->globPartR;

        // So for this->globPartP --> T is this->globPartE, and P is this->partP
        this->globPartP = this->partP;
        this->globPartP *= this->globPartE;
        this->globPartP -= this->globPartE;
        this->globPartP += 1L;

        //old stuff--> err = this->make_neighbors_loads(); IFEB;
        err = this->make_neighbors_loads2(); IFEB;
        err = this->make_connected_neighbors_loads(in_polyOrder); IFEB;

        if(in_output_debug) this->debug_PrintNeighbor(true);

        break;
    }
    return err;
}

Err_t NekCGsim::make_connected_neighbors_loads(int in_polyOrder)
{
    Err_t err=0;
    while(!err){
        //The total amount of nodes shared by more
        // than one rank is calculated as follows:

        unsigned int dofCorner = 1; //Number of DOF per Corner on the RANK-to-RANK grid
        Triplet dofEdge; //Number of DOF per Edge on the RANK-to-RANK grid, without the 2 corners.
        Triplet dofFace; //Number of DOF per Face on the RANK-to-RANK grid.

        dofEdge.a = (this->partE.a *(in_polyOrder+1) - this->partE.a +1 ) - 2; //-2 to remove the two corners
        dofEdge.b = (this->partE.b *(in_polyOrder+1) - this->partE.b +1 ) - 2; //-2 to remove the two corners
        dofEdge.c = (this->partE.c *(in_polyOrder+1) - this->partE.c +1 ) - 2; //-2 to remove the two corners

        dofFace.a = dofEdge.a * dofEdge.a;
        dofFace.b = dofEdge.b * dofEdge.b;
        dofFace.c = dofEdge.c * dofEdge.c;

        const int rA = this->partR.a;
        const int rB = this->partR.b;
        const int rC = this->partR.c;

        unsigned int sharedCorners = 0;
        Triplet sharedEdges;  //The number of shared edges in a convex cubic lattice without holes.
        Triplet sharedFaces;  //The number of shared faces in a convex cubic lattice without holes.

        sharedCorners = (rA+1)*(rB+1)*(rC+1) - 8;

        sharedEdges.a = (rA  )*(rB+1)*(rC+1) - 4*rA;
        sharedEdges.b = (rA+1)*(rB  )*(rC+1) - 4*rB;
        sharedEdges.c = (rA+1)*(rB+1)*(rC  ) - 4*rC;

        sharedFaces.a = (rB-1)*rA*rC;
        sharedFaces.b = (rA-1)*rB*rC;
        sharedFaces.c = (rC-1)*rA*rB;

        this->total_shared_nodes = 0;
        this->total_shared_nodes += sharedCorners * dofCorner;
        this->total_shared_nodes += dofEdge.a * sharedEdges.a;
        this->total_shared_nodes += dofEdge.b * sharedEdges.b;
        this->total_shared_nodes += dofEdge.c * sharedEdges.c;
        this->total_shared_nodes += dofFace.a * sharedFaces.a;
        this->total_shared_nodes += dofFace.b * sharedFaces.b;
        this->total_shared_nodes += dofFace.c * sharedFaces.c;

        break;
    }
    return err;
}

Err_t NekCGsim::make_neighbors_loads2()
{
    Err_t err=0;
    while(!err){
        Triplet rankplet = CUBICLAT::id2tripletZ(this->globPartR, this->mpiRank);

        this->sz_nloads=0;
        this->largest_countDOFdisco=0;
        int t=-1; //offset in this->nloads

        //=The number of DOF on an edge of a Rank does not include the 2 corner DOF.
        //=Then The number of DOF on a face is simply dofOnEdgeU * dofOnEdgeV for proper U,V in {X,Y,Z}
        long dofOnEdgeX = 0;
        long dofOnEdgeY = 0;
        long dofOnEdgeZ = 0;

        dofOnEdgeX = this->partE.a * (this->partP.a) - this->partE.a +1; //This has the 2 corner DOF.
        dofOnEdgeY = this->partE.b * (this->partP.b) - this->partE.b +1; //This has the 2 corner DOF.
        dofOnEdgeZ = this->partE.c * (this->partP.c) - this->partE.c +1; //This has the 2 corner DOF.

        dofOnEdgeX -= 2; //Removing the two corner DOF.
        dofOnEdgeY -= 2; //Removing the two corner DOF.
        dofOnEdgeZ -= 2; //Removing the two corner DOF.

        if(0) {printf("%d>DOFonEdge(X|Y|Z)=(%ld|%ld|%ld)\n",__LINE__,dofOnEdgeX,dofOnEdgeY,dofOnEdgeZ); fflush(stdout);}

        //dofOnEdge(X|Y|Z) can be zero but never negative.
        if(dofOnEdgeX<0 || dofOnEdgeY<0 || dofOnEdgeZ<0) {err=__LINE__; break;} //Sanity check.

        for(CUBICLAT::DIRid did=0; did<CUBICLAT::CELLCOUNT_IN_NEIGHBORHOOD; ++did)
        {
            if(did==CUBICLAT::COI_DIRid) continue; //Skip current rank.

            Triplet direction = CUBICLAT::dirId2tripletNB(did);
            CUBICLAT::Interaction interac = CUBICLAT::dirId2interaction(did);

            Triplet neighbor;
            neighbor = rankplet;
            neighbor += direction;

            if(neighbor.isinLattice(this->globPartR)){
                Idz rid = CUBICLAT::tripletZ2id(this->globPartR, neighbor);

                ++t;
                this->nloads[t].did = did;
                this->nloads[t].rid = rid;
                this->nloads[t].interaction = interac;

                unsigned long ddof = 0; //Disconnected DOF count
                switch(interac){
                default:
                    err=__LINE__;
                    break;
                case CUBICLAT::actFACE:
                    {
                        if(direction.a!=0){ //Y-Z plane
                            ddof += dofOnEdgeY * dofOnEdgeZ;    //For the face
                            ddof += 2*(dofOnEdgeY +dofOnEdgeZ );//For the 4 edges
                        }else if(direction.b!=0){ //X-Z plane
                            ddof += dofOnEdgeX * dofOnEdgeZ;    //For the face
                            ddof += 2*(dofOnEdgeX +dofOnEdgeZ );//For the 4 edges
                        }else if(direction.c!=0){ //X-Y plane
                            ddof += dofOnEdgeY * dofOnEdgeX;    //For the face
                            ddof += 2*(dofOnEdgeY +dofOnEdgeX );//For the 4 edges
                        }else{
                            err=__LINE__; break;
                        }
                        ddof += 4;                              //For the 4 corners

                        if(0) {printf("%d> ddof = %lu\n",__LINE__,ddof); fflush(stdout);}
                    }
                    break;
                case CUBICLAT::actEDGE:
                    {
                        if(direction.a==0){ //along X-axis
                            ddof += dofOnEdgeX;
                        }else if(direction.b==0){ //along Y-axis
                            ddof += dofOnEdgeY;
                        }else if(direction.c==0){ //along Z-axis
                            ddof += dofOnEdgeZ;
                        }else{
                            err=__LINE__; break;
                        }
                        ddof += 2; //Adding the 2 coners
                    }
                    break;
                case CUBICLAT::actCORNER:
                    ddof =1;
                    break;
                }
                if(err) break;

                this->nloads[t].disconnectedDOF = ddof;
                if(this->largest_countDOFdisco<ddof) this->largest_countDOFdisco=ddof;

            }else{ //Neighbor not in lattice
                //Do nothing
            }
        }//for(CUBICLAT::DIRid did
        if(err) break;

        this->sz_nloads = t+1;

        break;
    }
    return err;
}

Err_t NekCGsim::make_neighbors_loads()
{
//#   define DEBUG_PRINT printf("DBG>\t%d\t%d\t%ld\t%lu\n", this->mpiRank, t, (long)d, dof)
#   define DEBUG_PRINT
    Err_t err=0;
    while(!err){
        Triplet rankplet = CUBICLAT::id2tripletZ(this->globPartR, this->mpiRank);

        this->sz_nloads=0;
        this->largest_countDOFdisco=0;
        int t=-1; //offset in this->nloads
        Triplet neighbor;

        //=====The first 6 direction --> face-to-face
        {
            neighbor = rankplet;
            neighbor += Triplet(1,0,0);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.c * this->partP.c + 1;
                unsigned long v = this->partE.b * this->partP.b + 1;

                unsigned long dof = u * v;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }

            neighbor = rankplet;
            neighbor += Triplet(-1,0,0);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.c * this->partP.c + 1;
                unsigned long v = this->partE.b * this->partP.b + 1;

                unsigned long dof = u * v;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }

            neighbor = rankplet;
            neighbor += Triplet(0,1,0);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.c * this->partP.c + 1;
                unsigned long v = this->partE.a * this->partP.a + 1;

                unsigned long dof = u * v;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }

            neighbor = rankplet;
            neighbor += Triplet(0,-1,0);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.c * this->partP.c + 1;
                unsigned long v = this->partE.a * this->partP.a + 1;

                unsigned long dof = u * v;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }

            neighbor = rankplet;
            neighbor += Triplet(0,0,1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.a * this->partP.a + 1;
                unsigned long v = this->partE.b * this->partP.b + 1;

                unsigned long dof = u * v;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }

            neighbor = rankplet;
            neighbor += Triplet(0,0,-1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.a * this->partP.a + 1;
                unsigned long v = this->partE.b * this->partP.b + 1;

                unsigned long dof = u * v;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }
        }
        //===== The 8 directions connected corner-to-corner --> 1 DOF shared
        {
            neighbor = rankplet;
            neighbor += Triplet(1,1,-1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long dof = 1;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }

            neighbor = rankplet;
            neighbor += Triplet(1,-1,-1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long dof = 1;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }

            neighbor = rankplet;
            neighbor += Triplet(-1,-1,-1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long dof = 1;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }

            neighbor = rankplet;
            neighbor += Triplet(-1,1,-1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long dof = 1;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }

            neighbor = rankplet;
            neighbor += Triplet(1,1,1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long dof = 1;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }

            neighbor = rankplet;
            neighbor += Triplet(1,-1,1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long dof = 1;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }

            neighbor = rankplet;
            neighbor += Triplet(-1,-1,1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long dof = 1;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }

            neighbor = rankplet;
            neighbor += Triplet(-1,1,1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long dof = 1;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }
        }
        //===== The 12 neighbors on a edge
        {
            neighbor = rankplet;
            neighbor += Triplet(0,-1,-1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.a * this->partP.a + 1;

                unsigned long dof = u;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }
            neighbor = rankplet;
            neighbor += Triplet(0,1,-1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.a * this->partP.a + 1;

                unsigned long dof = u;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }
            neighbor = rankplet;
            neighbor += Triplet(0,-1,1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.a * this->partP.a + 1;

                unsigned long dof = u;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }
            neighbor = rankplet;
            neighbor += Triplet(0,1,1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.a * this->partP.a + 1;

                unsigned long dof = u;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }

            neighbor = rankplet;
            neighbor += Triplet(1,0,-1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.b * this->partP.b + 1;

                unsigned long dof = u;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }
            neighbor = rankplet;
            neighbor += Triplet(1,0,1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.b * this->partP.b + 1;

                unsigned long dof = u;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }
            neighbor = rankplet;
            neighbor += Triplet(-1,0,1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.b * this->partP.b + 1;

                unsigned long dof = u;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }
            neighbor = rankplet;
            neighbor += Triplet(-1,0,-1);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.b * this->partP.b + 1;

                unsigned long dof = u;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }

            neighbor = rankplet;
            neighbor += Triplet(-1,-1,0);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.c * this->partP.c + 1;

                unsigned long dof = u;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }
            neighbor = rankplet;
            neighbor += Triplet(-1,1,0);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.c * this->partP.c + 1;

                unsigned long dof = u;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }
            neighbor = rankplet;
            neighbor += Triplet(1,-1,0);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.c * this->partP.c + 1;

                unsigned long dof = u;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }
            neighbor = rankplet;
            neighbor += Triplet(1,1,0);
            if(neighbor.isinLattice(this->globPartR)){
                unsigned long u = this->partE.c * this->partP.c + 1;

                unsigned long dof = u;
                if(this->largest_countDOFdisco<dof) this->largest_countDOFdisco=dof;
                Idz d = CUBICLAT::tripletZ2id(this->globPartR, neighbor);
                ++t;
                this->nloads[t].rid = d;
                this->nloads[t].disconnectedDOF = dof;
                DEBUG_PRINT;
            }
        }

        //===== Finishing up
        this->sz_nloads = t+1;

        break;
    }//while(!err){
    return err;
#   undef DEBUG_PRINT
}

Err_t NekCGsim::clearMesh()
{
    Err_t err=0;
    while(!err){
        memset(    (void*)&this->partR,0,sizeof(Triplet));
        memset(    (void*)&this->partE,0,sizeof(Triplet));
        memset(    (void*)&this->partP,0,sizeof(Triplet));
        memset((void*)&this->globPartR,0,sizeof(Triplet));
        memset((void*)&this->globPartE,0,sizeof(Triplet));
        memset((void*)&this->globPartP,0,sizeof(Triplet));

        for(int i=0; i!=(int)SD::NeighborCount; ++i){
            NeighborLoad * lo = &this->nloads[i];
            memset( (void*)lo, 0, sizeof(NeighborLoad));
        }

        this->sz_nloads=0;
        this->largest_countDOFdisco=0;

        break;
    }
    return err;
}

Err_t NekCGsim::conjugateGradient()
{
    Err_t err=0;
    int mpierr = MPI_SUCCESS;
    while(!err){

            err = this->nek_glsc3(); IFEB;

            for(int iter = 0; iter <SD::CGcount; ++iter){

                err = this->nek_glsc3(); IFEB;

                err = this->nek_gsop("on w"); IFEB;

                err = this->nek_glsc3(); IFEB;

                err = this->nek_glsc3(); IFEB;
            }
            if(err) break;

        break;
    }
    return err;
}

Err_t NekCGsim::nek_gsop(const char * in_text)
{
    Err_t err=0;

    int mpierr = MPI_SUCCESS;
    while(!err){
        if(this->largest_countDOFdisco==0) err=__LINE__; IFEB;
        const size_t szbuf = (this->largest_countDOFdisco +1)* SD::ByteSizeOf1DOF; //+1 for good luck

        std::vector<char > recvBuffer(szbuf, 0);
        char * rbuf = &recvBuffer[0];

        MPI_Request emptyRequest;
        std::vector<MPI_Request > rrequest(this->sz_nloads, emptyRequest);

        //In NEKbone, the MPI_Irecv are done first
        for(int i=0; i!=this->sz_nloads; ++i)
        {
            NeighborLoad b = this->nloads[i];

            int source = b.rid;

            int tag = 1;  //Each send->recv pair are unique.  So use simplpest tag.

            MPI_Request * rreq = &rrequest[i];

            mpierr = MPI_Irecv( (void*)rbuf
                              , b.disconnectedDOF * SD::ByteSizeOf1DOF
                              , MPI_CHAR
                              , source
                              , tag
                              , MPI_COMM_WORLD
                              , rreq
                              ); CHECKMPIERR;
        }
        if(err) break;

        std::vector<char > sendBuffer(szbuf, 0);
        char * sbuf = &sendBuffer[0];

        MPI_Request srequest; //I'm not going to bother with tracking send requests.
                             //I hope that is OK.

        //In NEKbone, the MPI_Isend are done second
        for(int i=0; i!=this->sz_nloads; ++i)
        {
            NeighborLoad b = this->nloads[i];

            int dest = b.rid;

            int tag = 1; //Each send->recv pair are unique.  So use simplpest tag.

            mpierr = MPI_Isend( (void*)sbuf
                              , b.disconnectedDOF * SD::ByteSizeOf1DOF
                              , MPI_CHAR
                              , dest
                              , tag
                              , MPI_COMM_WORLD
                              , &srequest
                              ); CHECKMPIERR;
        }
        if(err) break;

        {//Waiting for all MPI_Recv to be done
            MPI_Request * rreqall = &rrequest[0];
            int sz = rrequest.size();
            MPI_Waitall(sz, rreqall, MPI_STATUSES_IGNORE);
        }

        break;
    }
    return err;
}

Err_t NekCGsim::nek_glsc3()
{
    Err_t err=0;
    int mpierr = MPI_SUCCESS;
    while(!err){

        double qx[1];
        double qres[1];
        qx[0]=1;
        qres[0]=0.0;

        mpierr = MPI_Allreduce( qx, qres, 1, MPI_DOUBLE_PRECISION, MPI_SUM, MPI_COMM_WORLD );

        double result = qres[0];

        break;
    }
    return err;
}

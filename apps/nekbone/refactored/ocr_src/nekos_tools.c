#ifndef NEKOS_TOOLS_H
#include "nekos_tools.h"
#endif

#define XMEMSET(SRC, CHARC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)SRC+xmIT)=CHARC;}
#define XMEMCPY(DEST, SRC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)DEST+xmIT)=*((char*)SRC+xmIT);}

#include "ocr.h" //PRINTF

Err_t init_NEKOtools(NEKOtools_t * io, NEKOstatics_t in_nstatics,
                     NEKOglobals_t in_nglobals)
{
    Err_t err=0;
    while(!err){
        err = destroy_NEKOtools(io); IFEB;

        const bool output_debug = false;

        if(output_debug){
            //print_NEKOstatics(&in_nstatics);
            //print_NEKOglobals(&in_nglobals);
        }

        io->mpiRank = in_nglobals.rankID;

        err = makeMesh(io, in_nstatics, in_nglobals, output_debug); IFEB;

        break;
    }
    return err;
}

Err_t destroy_NEKOtools(NEKOtools_t * io)
{
    Err_t err=0;
    while(!err){
        XMEMSET(io, 0, sizeof(NEKOtools_t));
        break;
    }
    return err;
}

Err_t copy_NEKOtools(NEKOtools_t * in_from, NEKOtools_t * o_target)
{
    Err_t err=0;
    while(!err){
        XMEMCPY(o_target, in_from, sizeof(NEKOtools_t));
        break;
    }
    return err;
}

void  print_NEKOtools(NEKOtools_t * in, int in_output_neighborhoods)
{
    PRINTF("NekTools>%d> mpiRank=%d\t", in->mpiRank, in->mpiRank);
    PRINTF("R(%d, %d, %d)", in->partR.a, in->partR.b, in->partR.c);
    PRINTF("E(%d, %d, %d)", in->partE.a, in->partE.b, in->partE.c);
    PRINTF("P(%d, %d, %d)\n", in->partP.a, in->partP.b, in->partP.c);

    PRINTF("NekTools>%d> \t", in->mpiRank);
    PRINTF("gR(%d, %d, %d)", in->globPartR.a, in->globPartR.b, in->globPartR.c);
    PRINTF("gE(%d, %d, %d)", in->globPartE.a, in->globPartE.b, in->globPartE.c);
    PRINTF("gP(%d, %d, %d)\n", in->globPartP.a, in->globPartP.b, in->globPartP.c);

    PRINTF("NekTools>%d> sz_nloads=%d\ttotal_shared_nodes=%lu\tlargest_countDOFdisco=%u\n",
           in->mpiRank, in->sz_nloads, in->total_shared_nodes, in->largest_countDOFdisco);

    int i;
    if(in_output_neighborhoods)
    for(i=0; i<in->sz_nloads; ++i){
        NeighborLoad_t *b = in->nloads + i;
        PRINTF("NekTools>%d> -->  R=%d\tDIRid=%ld  %d\tddof=%lu\tocDof=%lu\n",
               in->mpiRank,
               b->rid, b->did, (int)(b->interaction), b->disconnectedDOF, b->ownedConnectDOF
               );
    }
}

Err_t makeMesh(NEKOtools_t * io, NEKOstatics_t in_nstatics,
               NEKOglobals_t in_nglobals, int in_output_debug)
{
    Err_t err=0;
    while(!err){
        io->partR.a = in_nstatics.Rx; io->partR.b = in_nstatics.Ry; io->partR.c = in_nstatics.Rz;
        io->partE.a = in_nstatics.Ex; io->partE.b = in_nstatics.Ey; io->partE.c = in_nstatics.Ez;
        io->partP.a = in_nglobals.pDOF; io->partP.b = in_nglobals.pDOF; io->partP.c = in_nglobals.pDOF;

        //The formula for a regular grid on a line segment with T partition
        //and P dof per partition is
        //  Total DOF = (T*P) - T +1

        // So for this->globPartR --> The same as this->partR
        assignT(& io->globPartR, & io->partR);

        // So for this->globPartE --> T is this->globPartR, and P is this->partE
        io->globPartE.a = io->partE.a * io->globPartR.a;
        io->globPartE.b = io->partE.b * io->globPartR.b;
        io->globPartE.c = io->partE.c * io->globPartR.c;

        // So for this->globPartP --> T is this->globPartE, and P is this->partP
        io->globPartP.a = io->partP.a * io->globPartE.a - io->globPartE.a + 1;
        io->globPartP.b = io->partP.b * io->globPartE.b - io->globPartE.b + 1;
        io->globPartP.c = io->partP.c * io->globPartE.c - io->globPartE.c + 1;

        //old stuff--> err = this->make_neighbors_loads(); IFEB;
        err = make_neighbors_loads2(io); IFEB;
        err = make_connected_neighbors_loads(io, in_nglobals.pDOF); IFEB;

        if(in_output_debug){
            bool output_neighborhoods = true;
            print_NEKOtools(io, output_neighborhoods);
        }

        break;
    }
    return err;
}

Err_t make_connected_neighbors_loads(NEKOtools_t * io, int in_polyOrder)
{
    Err_t err=0;
    while(!err){
        //The total amount of nodes shared by more
        // than one rank is calculated as follows:

        unsigned int dofCorner = 1; //Number of DOF per Corner on the RANK-to-RANK grid
        Triplet dofEdge; //Number of DOF per Edge on the RANK-to-RANK grid, without the 2 corners.
        Triplet dofFace; //Number of DOF per Face on the RANK-to-RANK grid.

        dofEdge.a = (io->partE.a * in_polyOrder - io->partE.a +1 ) - 2; //-2 to remove the two corners
        dofEdge.b = (io->partE.b * in_polyOrder - io->partE.b +1 ) - 2; //-2 to remove the two corners
        dofEdge.c = (io->partE.c * in_polyOrder - io->partE.c +1 ) - 2; //-2 to remove the two corners

        dofFace.a = dofEdge.a * dofEdge.a;
        dofFace.b = dofEdge.b * dofEdge.b;
        dofFace.c = dofEdge.c * dofEdge.c;

        const int rA = io->partR.a;
        const int rB = io->partR.b;
        const int rC = io->partR.c;

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

        io->total_shared_nodes = 0;
        io->total_shared_nodes += sharedCorners * dofCorner;
        io->total_shared_nodes += dofEdge.a * sharedEdges.a;
        io->total_shared_nodes += dofEdge.b * sharedEdges.b;
        io->total_shared_nodes += dofEdge.c * sharedEdges.c;
        io->total_shared_nodes += dofFace.a * sharedFaces.a;
        io->total_shared_nodes += dofFace.b * sharedFaces.b;
        io->total_shared_nodes += dofFace.c * sharedFaces.c;

        break;
    }
    return err;
}

Err_t make_neighbors_loads2(NEKOtools_t * io)
{
    Err_t err=0;
    while(!err){
        Triplet rankplet = id2tripletZ(io->globPartR, io->mpiRank);

        io->sz_nloads=0;
        io->largest_countDOFdisco=0;
        int t=-1; //offset in io->nloads

        //=The number of DOF on an edge of a Rank does not include the 2 corner DOF.
        //=Then The number of DOF on a face is simply dofOnEdgeU * dofOnEdgeV for proper U,V in {X,Y,Z}
        long dofOnEdgeX = 0;
        long dofOnEdgeY = 0;
        long dofOnEdgeZ = 0;

        dofOnEdgeX = io->partE.a * io->partP.a - io->partE.a +1; //This has the 2 corner DOF.
        dofOnEdgeY = io->partE.b * io->partP.b - io->partE.b +1; //This has the 2 corner DOF.
        dofOnEdgeZ = io->partE.c * io->partP.c - io->partE.c +1; //This has the 2 corner DOF.

        dofOnEdgeX -= 2; //Removing the two corner DOF.
        dofOnEdgeY -= 2; //Removing the two corner DOF.
        dofOnEdgeZ -= 2; //Removing the two corner DOF.

        if(0) PRINTF("%d>DOFonEdge(X|Y|Z)=(%ld|%ld|%ld)\n",__LINE__,dofOnEdgeX,dofOnEdgeY,dofOnEdgeZ);

        //dofOnEdge(X|Y|Z) can be zero but never negative.
        if(dofOnEdgeX<0 || dofOnEdgeY<0 || dofOnEdgeZ<0) {err=__LINE__; break;} //Sanity check.

        DIRid did;
        for(did=0; did<CELLCOUNT_IN_NEIGHBORHOOD; ++did)
        {
            if(did==COI_DIRid) continue; //Skip current rank.

            Triplet direction = dirId2tripletNB(did);
            Interaction interac = dirId2interaction(did);

            Triplet neighbor;
            assignT(&neighbor, &rankplet);
            addT(&neighbor, direction);

            if(isinLattice(neighbor, io->globPartR)){
                Idz rid = tripletZ2id(io->globPartR, neighbor);

                ++t;
                io->nloads[t].did = did;
                io->nloads[t].rid = rid;
                io->nloads[t].interaction = interac;

                unsigned long ddof = 0; //Disconnected DOF count
                switch(interac){
                default:
                    err=__LINE__;
                    break;
                case IAactFACE:
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

                        if(0) {PRINTF("%d> ddof = %lu\n",__LINE__,ddof); }
                    }
                    break;
                case IAactEDGE:
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
                case IAactCORNER:
                    ddof =1;
                    break;
                }
                if(err) break;

                io->nloads[t].disconnectedDOF = ddof;
                if(io->largest_countDOFdisco<ddof) io->largest_countDOFdisco=ddof;

            }else{ //Neighbor not in lattice
                //Do nothing
            }
        }//for(DIRid did
        if(err) break;

        io->sz_nloads = t+1;

        break;
    }
    return err;
}















































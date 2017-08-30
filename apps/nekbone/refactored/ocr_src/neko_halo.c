#ifndef NEKO_HALO_H
#include "neko_halo.h"
#endif

#ifdef NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
#   define ENABLE_EXTENSION_LABELING // For labeled GUIDs
#   include "extensions/ocr-labeling.h" // For labeled GUIDs
#endif

#include "nekos_tools.h"

//#define OUTPUT_HALO_DEBUG_PRINTS
#ifdef OUTPUT_HALO_DEBUG_PRINTS
#   define DBG_PRINTF0(txt)                         printf(txt)
#   define DBG_PRINTF1(txt,a)                       printf(txt,a)
#   define DBG_PRINTF3(txt,a,b,c)                   printf(txt,a,b,c)
#   define DBG_PRINTF6(txt,a,b,c, d,e,f)            printf(txt,a,b,c, d,e,f)
#   define DBG_PRINTF7(txt,a,b,c, d,e,f, g)         printf(txt,a,b,c, d,e,f, g)
#   define DBG_PRINTF8(txt,a,b,c, d,e,f, g,h)       printf(txt,a,b,c, d,e,f, g,h)
#   define DBG_PRINTF9(txt,a,b,c, d,e,f, g,h,i)     printf(txt,a,b,c, d,e,f, g,h,i)
#   define DBG_PRINTF10(txt,a,b,c, d,e,f, g,h,i, j) printf(txt,a,b,c, d,e,f, g,h,i, j)
#else
#   define DBG_PRINTF0(txt)                         do{}while(0)
#   define DBG_PRINTF1(txt,a)                       do{}while(0)
#   define DBG_PRINTF3(txt,a,b,c)                   do{}while(0)
#   define DBG_PRINTF6(txt,a,b,c,d,e,f)             do{}while(0)
#   define DBG_PRINTF7(txt,a,b,c, d,e,f, g)         do{}while(0)
#   define DBG_PRINTF8(txt,a,b,c, d,e,f, g,h)       do{}while(0)
#   define DBG_PRINTF9(txt,a,b,c, d,e,f, g,h,i)     do{}while(0)
#   define DBG_PRINTF10(txt,a,b,c, d,e,f, g,h,i, j) do{}while(0)
#endif

Err_t make_labeledEvents(OA_DEBUG_ARGUMENT, Idz in_did, ocrGuid_t * in_haloLabeledGuids,
                          unsigned int in_myRank, ocrGuid_t * io_myEvent,
                          unsigned int in_neighborRank, ocrGuid_t * io_neighborEvent)
{
   Err_t err=0;
    while(!err){
#       ifdef NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
        err = ocrGuidFromIndex( io_myEvent, in_haloLabeledGuids[in_did], in_myRank );
        if(err){
            ocrPrintf("ERROR> TaskTYPE=%d TaskID="GUIDF" rank=%u: make_labeledEvents> Creation of my labeled Guid failed: err=%u.\n",
                   in_edtType, GUIDA(in_thisEDT), in_myRank, err);
            IFEB;
        }
        err = ocrEventCreate( io_myEvent, OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );
        if(err){
            if( err == OCR_EGUIDEXISTS ){
                //Someone has already created an object associated with this o_guid.
                //This is usually the expected behavior. --> Just ignore it.
                err = 0;
            }else{
                ocrPrintf("ERROR> TaskTYPE=%d TaskID="GUIDF" rank=%u make_labeledEvents> Creation of io_myEvent EVT failed: %d\n",
                       in_edtType, GUIDA(in_thisEDT), in_myRank, err);
                IFEB;
            }
        }

        err = ocrGuidFromIndex( io_neighborEvent, in_haloLabeledGuids[NEKbone_neighborCount - in_did], in_neighborRank );
        if(err){
            ocrPrintf("ERROR> TaskTYPE=%d TaskID="GUIDF" rank=%u: make_labeledEvents> Creation of the neighbbor labeled Guid failed: err=%u.\n",
                   in_edtType, GUIDA(in_thisEDT), in_myRank, err);

            IFEB;
        }
        err = ocrEventCreate( io_neighborEvent, OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );
        if(err){
            if( err == OCR_EGUIDEXISTS ){
                //Someone has already created an object associated with this o_guid.
                //This is usually the expected behavior. --> Just ignore it.
                err = 0;
            }else{
                ocrPrintf("ERROR> TaskTYPE=%d TaskID="GUIDF" rank=%u make_labeledEvents> Creation of io_neighborEvent EVT (rank=%u) failed: %d\n",
                       in_edtType, GUIDA(in_thisEDT), in_myRank, in_neighborRank, err);
                IFEB;
            }
        }
#       endif // NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
        break;  //  while(!err)
    }
    return err;
}

Err_t start_channelExchange(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools,
                            ocrGuid_t * in_haloLabeledGuids, ocrGuid_t * io_futureselfEDT,
                            NEKOglobals_t * in_globals, NEKOglobals_t * o_globals)
{
    Err_t err=0;
    while(!err){
        err = copy_NEKOglobals(in_globals, o_globals);
        NEKOglobals_t * G = o_globals;

        //ocrPrintf("DBG> TaskTYPE=%d TaskID="GUIDF" start_channelExchange channelCount=%d neighborEDT="GUIDF"\n",
        //       in_edtType, GUIDA(in_thisEDT), (int)SLOTCNT_total_channelExchange, GUIDA(*io_futureselfEDT));

        unsigned int bor;
#       ifdef NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
        for(bor=0; bor < in_nekoTools->sz_nloads; ++bor){

            unsigned int neighborRank = in_nekoTools->nloads[bor].rid;
            DIRid did = in_nekoTools->nloads[bor].did;

            if( ! in_nekoTools->dir_present[did]){
                err = __LINE__; IFEB;
            }

            //===== Create labeled EVTs
            ocrGuid_t myLabelEvt = NULL_GUID;
            ocrGuid_t neighborLabelEvt = NULL_GUID;

            err = make_labeledEvents(OA_DEBUG_INVARS, did, in_haloLabeledGuids, G->rankID,
                                     &myLabelEvt, neighborRank, &neighborLabelEvt); IFEB;

            //===== Make my channel event for this pairing
            ocrEventParams_t params;
            params.EVENT_CHANNEL.maxGen = 2;
            params.EVENT_CHANNEL.nbSat  = 1;
            params.EVENT_CHANNEL.nbDeps = 1;

            ocrGuid_t myChannel = NULL_GUID;
            err = ocrEventCreateParams( &myChannel, OCR_EVENT_CHANNEL_T, EVT_PROP_NONE, &params ); IFEB;
            GUID_ASSIGN_VALUE(G->myChannels[did].c4multi, myChannel);      //Record myChannel for future use.
            //TODO: Record all labeled events for later destruction.

            myChannel = NULL_GUID;
            err = ocrEventCreateParams( &myChannel, OCR_EVENT_CHANNEL_T, EVT_PROP_NONE, &params ); IFEB;
            GUID_ASSIGN_VALUE(G->myChannels[did].c4setf, myChannel);      //Record myChannel for future use.
            //TODO: Record all labeled events for later destruction.

            myChannel = NULL_GUID;
            err = ocrEventCreateParams( &myChannel, OCR_EVENT_CHANNEL_T, EVT_PROP_NONE, &params ); IFEB;
            GUID_ASSIGN_VALUE(G->myChannels[did].c4axi, myChannel);      //Record myChannel for future use.
            //TODO: Record all labeled events for later destruction.

            //===== Package the channel event for sending
            ocrGuid_t envelopGuid  = NULL_GUID;
            ChannelStruct_t * envelopPtr = NULL;
            err = ocrDbCreate( &envelopGuid, (void**)&envelopPtr, sizeof(ChannelStruct_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC); IFEB;

            //ocrPrintf("DBG140> TaskTYPE=%d TaskID="GUIDF" start_channelExchange myrank=%u did=%ld borRank=%u envelop="GUIDF"\n",
            //       in_edtType, GUIDA(in_thisEDT), G->rankID, did, neighborRank, GUIDA(envelopGuid));

            err = copy_ChannelStruct(&G->myChannels[did], envelopPtr); IFEB;
            err = ocrDbRelease( envelopGuid );

            //===== Connect the labeled events
            err = ocrEventSatisfy( neighborLabelEvt, envelopGuid); IFEB;  //This sends myChannel to the neighbors.

            //This sends the neighbor's Channel to our future self.
                // Taking the complement makes sure the neighbor gets the info
                // in a slot it knows comes from here(i.e. did). This works due
                // to the complementarity of the canonical representation imposed by did.
            unsigned int destinationSlot = did;

            unsigned int actualSlot = SLOTCNT_offset0_channelExchange + destinationSlot;
            err = ocrAddDependence( myLabelEvt, *io_futureselfEDT, actualSlot, DB_MODE_RO); IFEB;

            //ocrPrintf("DBG157> TaskTYPE=%d TaskID="GUIDF" start_channelExchange myrank=%u did=%ld borRank=%u destSlot=%u actualSlot=%u\n",
            //       in_edtType, GUIDA(in_thisEDT), G->rankID, did, neighborRank, destinationSlot, actualSlot);

        } IFEB;//for(bor=0;
#       endif // NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES

        //This part is to send to our future self one NULL_GUID for each slot
        //which will not receive a labeled EDT.
        unsigned int dir;
        for(dir=0; dir < NEKbone_regionCount; ++dir){
#           ifdef NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
            if(in_nekoTools->dir_present[dir]){
                continue;
            }
#           endif // NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES

            unsigned int destinationSlot = dir;
            unsigned int actualSlot = SLOTCNT_offset0_channelExchange + destinationSlot;
            err = ocrAddDependence( NULL_GUID, *io_futureselfEDT, actualSlot, DB_MODE_RO); IFEB;

            //ocrPrintf("DBG175> TaskTYPE=%d TaskID="GUIDF" start_channelExchange myrank=%u dir=%u destSlot=%u actualSlot=%u NULL_GUID\n",
            //       in_edtType, GUIDA(in_thisEDT), G->rankID, dir, destinationSlot, actualSlot);

        }IFEB;

        break;
    }
    return err;
}

Err_t stop_channelExchange(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools, ocrEdtDep_t * in_depv,
                           NEKOglobals_t * in_globals, NEKOglobals_t * o_globals)
{
    Err_t err=0;
    while(!err){
        err = copy_NEKOglobals(in_globals, o_globals);

#       ifdef NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
        NEKOglobals_t * G = o_globals;

        ocrEdtDep_t * depvNeighborChannels = in_depv + SLOTCNT_offset0_channelExchange;

        unsigned int bor;
        for(bor=0; bor < in_nekoTools->sz_nloads; ++bor){
            unsigned int neighborRank = in_nekoTools->nloads[bor].rid;
            DIRid did = in_nekoTools->nloads[bor].did;

            err = copy_ChannelStruct((ChannelStruct_t*)depvNeighborChannels[did].ptr, &G->neighborChannels[did]); IFEB;

            //ocrPrintf("DBG204> TaskTYPE=%d TaskID="GUIDF" stop_channelExchange myrank=%d did=%ld borRank=%u envelop="GUIDF"\n",
            //       in_edtType, GUIDA(in_thisEDT), in_nekoTools->mpiRank, did, neighborRank, GUIDA(depvNeighborChannels[did].guid) );

            err = ocrDbDestroy(depvNeighborChannels[did].guid); IFEB; // Clean up used envelop DBK.

        }//for(bor=0

#       endif // NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
        break;
    }
    return err;
}

//==========================
Idz triplet2index(Triplet in_position, Triplet in_lattice)
{
    const Triplet P = in_position;
    const Triplet L = in_lattice;

    Idz x = P.a + L.a*P.b + L.a*L.b*P.c;
    return x;
}
void index2triplet(Idz in_index, Triplet in_lattice, Triplet * o_x)
{
    const Triplet L = in_lattice;
    o_x->c = in_index / (L.a*L.b);
    in_index -= (L.a*L.b) * o_x->c;
    o_x->b = in_index / L.a;
    o_x->a = in_index - o_x->b * L.a;
}

Idz elementLocalTriplets2index(Triplet in_dofLattice, Triplet in_Elattice, Triplet in_dof, Triplet in_element)
{
    Idz new_d = triplet2index(in_dof, in_dofLattice);
    Idz new_e = triplet2index(in_element, in_Elattice);
    Idz new_i = new_d + abcT(in_dofLattice) * new_e;
    return new_i;
}

void posOnNewRank2indices(Triplet in_dofLattice, Triplet in_Elattice, Triplet in_rankLattice,
                     Triplet in_dof, Triplet in_element, Triplet in_rank, Idz * o_dofEleIndex, Idz * o_rankID)
{
    *o_dofEleIndex = elementLocalTriplets2index(in_dofLattice, in_Elattice, in_dof, in_element);
    *o_rankID = triplet2index(in_rank, in_rankLattice);
}

void set_rankIndexedValue(rankIndexedValue_t * io_riv, unsigned int in_offset,
                           Idz in_rankID, Idz in_eleDofID, NBN_REAL in_value)
{
    io_riv[in_offset].rankID = in_rankID;
    io_riv[in_offset].eleDofID = in_eleDofID;
    io_riv[in_offset].value = in_value;
}

//==========================
int halo_exchanges(Triplet in_Rlattice, Triplet in_Elattice, unsigned int in_pDOF,
                           unsigned int in_rankID, NBN_REAL * io_V,
                           Idz * io_sz_values, rankIndexedValue_t * io_rankedValues)
{
    const unsigned int really_big_number = 4000000000;
    int err=0;
    while(!err){
        //The maximum number of neighbors a pDOF can have
        // face   --> 1
        // edge   --> 3
        // corner --> 7

        const Triplet DOFlattice = {in_pDOF, in_pDOF, in_pDOF};
        const Idz pDOF3D = abcT(DOFlattice);
        unsigned int pDOF1 = in_pDOF - 1;

        unsigned int E = abcT(in_Elattice);

        Triplet rT = {0};
        index2triplet(in_rankID, in_Rlattice, &rT);

        unsigned int sz_surfacers=0;
        Idz surfacersIDs[8] = {0};

        *io_sz_values = 0;
        Idz sz_values = 0;
        Idz old_sz_values = 0;

        unsigned int eid;
        for(eid=0; eid<E; ++eid){
            Triplet eT = {0};
            index2triplet(eid, in_Elattice, &eT);

            Idz d;
            for(d=0; d<pDOF3D; ++d){
                Triplet dofT = {0};
                index2triplet(d, DOFlattice, &dofT);
                //Skip all non internal nodes

                const int X0 = dofT.a == 0;
                const int XN = dofT.a == pDOF1;
                const int Y0 = dofT.b == 0;
                const int YN = dofT.b == pDOF1;
                const int Z0 = dofT.c == 0;
                const int ZN = dofT.c == pDOF1;
                const int onX = X0 || XN;
                const int onY = Y0 || YN;
                const int onZ = Z0 || ZN;

                if( !(onX || onY || onZ) ) {
                    DBG_PRINTF3("Internal --> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                    continue;
                }

                Idz old_i = d + pDOF3D * eid;
                Triplet te, tdof, trank;

                if(  onX && onY && onZ  )
                {
                    //We are on a corner
                    if(X0){
                        if(Y0){
                            if(Z0) { //We are on corner X0 Y0 Z0
                                old_sz_values = sz_values;

                                DBG_PRINTF3("CornerX0Y0Z0--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                                sz_surfacers = 0;
                                surfacersIDs[sz_surfacers++] = old_i;

                                tdof.a = dofT.a;    tdof.b = dofT.b;    tdof.c = DOFlattice.c-1;
                                te.a   = eT.a;      te.b   = eT.b;      te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0Y0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = eT.b;     te.c    = in_Elattice.c-1;
                                    trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = DOFlattice.b-1;    tdof.c = dofT.c;
                                te.a   = eT.a;      te.b   = eT.b-1;            te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0Y0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = in_Elattice.b-1;  te.c    = eT.c;
                                    trank.a = rT.a;     trank.b = rT.b-1;           trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = DOFlattice.b-1;    tdof.c = DOFlattice.c-1;
                                te.a   = eT.a;      te.b   = eT.b-1;            te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0Y0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = in_Elattice.b-1;  te.c    = in_Elattice.c-1;
                                    trank.a = rT.a;     trank.b = rT.b-1;           trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = dofT.b;    tdof.c = dofT.c;
                                te.a   = eT.a-1;            te.b   = eT.b;      te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0Y0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = eT.b;     te.c    = eT.c;
                                    trank.a = rT.a-1;           trank.b = rT.b;     trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = DOFlattice.b-1;    tdof.c = dofT.c;
                                te.a   = eT.a-1;            te.b   = eT.b-1;            te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0Y0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = in_Elattice.b-1;  te.c    = eT.c;
                                    trank.a = rT.a-1;           trank.b = rT.b-1;           trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = dofT.b;    tdof.c = DOFlattice.c-1;
                                te.a   = eT.a-1;            te.b   = eT.b;      te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0Y0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = eT.b;     te.c    = in_Elattice.c-1;
                                    trank.a = rT.a-1;           trank.b = rT.b;     trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = DOFlattice.b-1;    tdof.c = DOFlattice.c-1;
                                te.a   = eT.a-1;            te.b   = eT.b-1;            te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0Y0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = in_Elattice.b-1;  te.c    = in_Elattice.c-1;
                                    trank.a = rT.a-1;           trank.b = rT.b-1;           trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                unsigned int s;
                                unsigned int mini = really_big_number;
                                for(s=0; s<sz_surfacers; ++s){
                                    if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                                }

                                if(sz_surfacers > 0 && mini == old_i){
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[old_i] += io_V[ surfacersIDs[s] ];
                                    }
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[ surfacersIDs[s] ] = io_V[old_i];
                                    }
                                    if(sz_values > old_sz_values){
                                        for(s=old_sz_values; s < sz_values; ++s){
                                            io_rankedValues[s].value = io_V[old_i];
                                        }
                                    }
                                    DBG_PRINTF6("CornerX0Y0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                                   ifdef OUTPUT_HALO_DEBUG_PRINTS
                                        unsigned int ss;
                                        for(ss=0; ss<sz_surfacers; ++ss){
                                            DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                        }
                                        DBG_PRINTF0("\n");
#                                   endif
                                }
                                else{
                                        sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                                }
                                //End of //We are on corner X0 Y0 Z0
                            }
                            else   { //We are on corner X0 Y0 ZN
                                old_sz_values = sz_values;
                                DBG_PRINTF3("CornerX0Y0ZN--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                                sz_surfacers = 0;
                                surfacersIDs[sz_surfacers++] = old_i;

                                tdof.a = dofT.a;    tdof.b = dofT.b;    tdof.c = 0;
                                te.a   = eT.a;      te.b   = eT.b;      te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0Y0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = eT.b;     te.c    = 0;
                                    trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = dofT.b;    tdof.c = dofT.c;
                                te.a   = eT.a-1;            te.b   = eT.b;      te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0Y0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = eT.b;     te.c    = eT.c;
                                    trank.a = rT.a-1;           trank.b = rT.b;     trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = dofT.b;    tdof.c = 0;
                                te.a   = eT.a-1;            te.b   = eT.b;      te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0Y0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = eT.b;     te.c    = 0;
                                    trank.a = rT.a-1;           trank.b = rT.b;     trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = DOFlattice.b-1;    tdof.c = dofT.c;
                                te.a   = eT.a;      te.b   = eT.b-1;            te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0Y0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = in_Elattice.b-1;  te.c    = eT.c;
                                    trank.a = rT.a;     trank.b = rT.b-1;           trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = DOFlattice.b-1;    tdof.c = dofT.c;
                                te.a   = eT.a-1;            te.b   = eT.b-1;            te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0Y0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;     te.b    = in_Elattice.b-1;  te.c    = eT.c;
                                    trank.a = rT.a-1;               trank.b = rT.b-1;          trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = DOFlattice.b-1;    tdof.c = 0;
                                te.a   = eT.a;      te.b   = eT.b-1;            te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0Y0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = in_Elattice.b-1;  te.c    = 0;
                                    trank.a = rT.a;     trank.b = rT.b-1;           trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = DOFlattice.b-1;    tdof.c = 0;
                                te.a   = eT.a-1;            te.b   = eT.b-1;            te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0Y0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = in_Elattice.b-1;  te.c    = 0;
                                    trank.a = rT.a-1;           trank.b = rT.b-1;           trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                unsigned int s;
                                unsigned int mini = really_big_number; //Some really big number
                                for(s=0; s<sz_surfacers; ++s){
                                    if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                                }

                                if(sz_surfacers > 0 && mini == old_i){
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[old_i] += io_V[ surfacersIDs[s] ];
                                    }
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[ surfacersIDs[s] ] = io_V[old_i];
                                    }
                                    if(sz_values > old_sz_values){
                                        for(s=old_sz_values; s < sz_values; ++s){
                                            io_rankedValues[s].value = io_V[old_i];
                                        }
                                    }

                                    DBG_PRINTF6("CornerX0Y0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                                   ifdef OUTPUT_HALO_DEBUG_PRINTS
                                        unsigned int ss;
                                        for(ss=0; ss<sz_surfacers; ++ss){
                                            DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                        }
                                        DBG_PRINTF0("\n");
#                                   endif
                                }
                                else{
                                        sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                                }

                                //End of //We are on corner X0 Y0 ZN
                            }
                        }
                        else{
                            if(Z0) { //We are on corner X0 YN Z0
                                old_sz_values = sz_values;
                                DBG_PRINTF3("CornerX0YNZ0--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                                sz_surfacers = 0;
                                surfacersIDs[sz_surfacers++] = old_i;

                                tdof.a = dofT.a;    tdof.b = 0;         tdof.c = dofT.c;
                                te.a   = eT.a;      te.b   = eT.b+1;    te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0YNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = 0;        te.c    = eT.c;
                                    trank.a = rT.a;     trank.b = rT.b+1;   trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = dofT.b;    tdof.c = dofT.c;
                                te.a   = eT.a-1;            te.b   = eT.b;      te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0YNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = eT.b;     te.c    = eT.c;
                                    trank.a = rT.a-1;           trank.b = rT.b;     trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = 0;         tdof.c = dofT.c;
                                te.a   = eT.a-1;            te.b   = eT.b+1;    te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0YNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = 0;        te.c    = eT.c;
                                    trank.a = rT.a-1;           trank.b = rT.b+1;   trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = dofT.b;    tdof.c = DOFlattice.c-1;
                                te.a   = eT.a;      te.b   = eT.b;      te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0YNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = eT.b;     te.c    = in_Elattice.c-1;
                                    trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = 0;    tdof.c = DOFlattice.c-1;
                                te.a   = eT.a;      te.b   = eT.b+1;      te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0YNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = 0;        te.c    = in_Elattice.c-1;
                                    trank.a = rT.a;     trank.b = rT.b+1;   trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = dofT.b;    tdof.c = DOFlattice.c-1;
                                te.a   = eT.a-1;            te.b   = eT.b;      te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0YNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = eT.b;     te.c    = in_Elattice.c-1;
                                    trank.a = rT.a-1;           trank.b = rT.b;     trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = 0;         tdof.c = DOFlattice.c-1;
                                te.a   = eT.a-1;            te.b   = eT.b+1;    te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0YNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = 0;        te.c    = in_Elattice.c-1;
                                    trank.a = rT.a-1;           trank.b = rT.b+1;   trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                unsigned int s;
                                unsigned int mini = really_big_number; //Some really big number
                                for(s=0; s<sz_surfacers; ++s){
                                    if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                                }

                                if(sz_surfacers > 0 && mini == old_i){
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[old_i] += io_V[ surfacersIDs[s] ];
                                    }
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[ surfacersIDs[s] ] = io_V[old_i];
                                    }

                                    //DBG> printf("DBG>CornerX0YNZ0> old=%ld  sz=%ld\n", old_sz_values, sz_values);
                                    if(sz_values > old_sz_values){
                                        for(s=old_sz_values; s < sz_values; ++s){
                                            io_rankedValues[s].value = io_V[old_i];
                                            //DBG> printf("DBG>CornerX0YNZ0> s=%u rankedValue=%ld %ld %g\n", s, io_rankedValues[s].rankID, io_rankedValues[s].eleDofID, io_rankedValues[s].value);
                                        }
                                    }

                                    DBG_PRINTF6("CornerX0YNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                                   ifdef OUTPUT_HALO_DEBUG_PRINTS
                                        unsigned int ss;
                                        for(ss=0; ss<sz_surfacers; ++ss){
                                            DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                        }
                                        DBG_PRINTF0("\n");
#                                   endif
                                }
                                else{
                                        sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                                }

                                //End of //We are on corner X0 YN Z0
                            }
                            else   { //We are on corner X0 YN ZN
                                old_sz_values = sz_values;
                                DBG_PRINTF3("CornerX0YNZN--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                                sz_surfacers = 0;
                                surfacersIDs[sz_surfacers++] = old_i;

                                tdof.a = dofT.a;    tdof.b = 0;         tdof.c = dofT.c;
                                te.a   = eT.a;      te.b   = eT.b+1;    te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = 0;        te.c    = eT.c;
                                    trank.a = rT.a;     trank.b = rT.b+1;   trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = dofT.b;    tdof.c = dofT.c;
                                te.a   = eT.a-1;            te.b   = eT.b;      te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = eT.b;     te.c    = eT.c;
                                    trank.a = rT.a-1;           trank.b = rT.b;     trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = 0;         tdof.c = dofT.c;
                                te.a   = eT.a-1;            te.b   = eT.b+1;    te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = 0;        te.c    = eT.c;
                                    trank.a = rT.a-1;           trank.b = rT.b+1;   trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = dofT.b;    tdof.c = 0;
                                te.a   = eT.a;      te.b   = eT.b;      te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = eT.b;     te.c    = 0;
                                    trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = 0;         tdof.c = 0;
                                te.a   = eT.a;      te.b   = eT.b+1;    te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = 0;        te.c    = 0;
                                    trank.a = rT.a;     trank.b = rT.b+1;   trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = dofT.b;    tdof.c = 0;
                                te.a   = eT.a-1;            te.b   = eT.b;      te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = eT.b;     te.c    = 0;
                                    trank.a = rT.a-1;           trank.b = rT.b;     trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = DOFlattice.a-1;    tdof.b = 0;         tdof.c = 0;
                                te.a   = eT.a-1;            te.b   = eT.b+1;    te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerX0YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = in_Elattice.a-1;  te.b    = 0;        te.c    = 0;
                                    trank.a = rT.a-1;           trank.b = rT.b+1;   trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerX0YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerX0YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                unsigned int s;
                                unsigned int mini = really_big_number; //Some really big number
                                for(s=0; s<sz_surfacers; ++s){
                                    if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                                }

                                if(sz_surfacers > 0 && mini == old_i){
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[old_i] += io_V[ surfacersIDs[s] ];
                                    }
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[ surfacersIDs[s] ] = io_V[old_i];
                                    }
                                    if(sz_values > old_sz_values){
                                        for(s=old_sz_values; s < sz_values; ++s){
                                            io_rankedValues[s].value = io_V[old_i];
                                        }
                                    }

                                    DBG_PRINTF6("CornerX0YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                                   ifdef OUTPUT_HALO_DEBUG_PRINTS
                                        unsigned int ss;
                                        for(ss=0; ss<sz_surfacers; ++ss){
                                            DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                        }
                                        DBG_PRINTF0("\n");
#                                   endif
                                }
                                else{
                                    sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                                }

                                //End of //We are on corner X0 YN ZN
                            }
                        }
                    }
                    else{
                        if(Y0){
                            if(Z0) { //We are on corner XN Y0 Z0
                                old_sz_values = sz_values;
                                DBG_PRINTF3("CornerXNY0Z0--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                                sz_surfacers = 0;
                                surfacersIDs[sz_surfacers++] = old_i;

                                tdof.a = 0;         tdof.b = dofT.b;    tdof.c = dofT.c;
                                te.a   = eT.a+1;    te.b   = eT.b;      te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNY0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = eT.b;     te.c    = eT.c;
                                    trank.a = rT.a+1;   trank.b = rT.b;     trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNY0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNY0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = DOFlattice.b-1;    tdof.c = dofT.c;
                                te.a   = eT.a;      te.b   = eT.b-1;            te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNY0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = in_Elattice.b-1;  te.c    = eT.c;
                                    trank.a = rT.a;     trank.b = rT.b-1;           trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNY0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNY0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = 0;         tdof.b = DOFlattice.b-1;    tdof.c = dofT.c;
                                te.a   = eT.a+1;    te.b   = eT.b-1;            te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNY0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = in_Elattice.b-1;  te.c    = eT.c;
                                    trank.a = rT.a+1;   trank.b = rT.b-1;           trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNY0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNY0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = dofT.b;    tdof.c = DOFlattice.c-1;
                                te.a   = eT.a;      te.b   = eT.b;      te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNY0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = eT.b;     te.c    = in_Elattice.c-1;
                                    trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNY0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNY0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = DOFlattice.b-1;    tdof.c = DOFlattice.c-1;
                                te.a   = eT.a;      te.b   = eT.b-1;            te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNY0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = in_Elattice.b-1;  te.c    = in_Elattice.c-1;
                                    trank.a = rT.a;     trank.b = rT.b-1;           trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNY0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNY0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = 0;         tdof.b = dofT.b;    tdof.c = DOFlattice.c-1;
                                te.a   = eT.a+1;    te.b   = eT.b;      te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNY0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = eT.b;     te.c    = in_Elattice.c-1;
                                    trank.a = rT.a+1;   trank.b = rT.b;     trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNY0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNY0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = 0;         tdof.b = DOFlattice.b-1;    tdof.c = DOFlattice.c-1;
                                te.a   = eT.a+1;    te.b   = eT.b-1;            te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNY0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = in_Elattice.b-1;  te.c    = in_Elattice.c-1;
                                    trank.a = rT.a+1;   trank.b = rT.b-1;           trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNY0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNY0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                unsigned int s;
                                unsigned int mini = really_big_number; //Some really big number
                                for(s=0; s<sz_surfacers; ++s){
                                    if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                                }

                                if(sz_surfacers > 0 && mini == old_i){
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[old_i] += io_V[ surfacersIDs[s] ];
                                    }
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[ surfacersIDs[s] ] = io_V[old_i];
                                    }
                                    if(sz_values > old_sz_values){
                                        for(s=old_sz_values; s < sz_values; ++s){
                                            io_rankedValues[s].value = io_V[old_i];
                                        }
                                    }

                                    DBG_PRINTF6("CornerXNY0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                                   ifdef OUTPUT_HALO_DEBUG_PRINTS
                                        unsigned int ss;
                                        for(ss=0; ss<sz_surfacers; ++ss){
                                            DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                        }
                                        DBG_PRINTF0("\n");
#                                   endif
                                }
                                else{
                                    sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                                }

                                //End of //We are on corner XN Y0 Z0
                            }
                            else   { //We are on corner XN Y0 ZN
                                old_sz_values = sz_values;
                                DBG_PRINTF3("CornerXNY0ZN--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                                sz_surfacers = 0;
                                surfacersIDs[sz_surfacers++] = old_i;

                                tdof.a = 0;         tdof.b = dofT.b;    tdof.c = dofT.c;
                                te.a   = eT.a+1;    te.b   = eT.b;      te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNY0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = eT.b;     te.c    = eT.c;
                                    trank.a = rT.a+1;   trank.b = rT.b;     trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNY0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNY0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = DOFlattice.b-1;    tdof.c = dofT.c;
                                te.a   = eT.a;      te.b   = eT.b-1;            te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNY0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = in_Elattice.b-1;  te.c    = eT.c;
                                    trank.a = rT.a;     trank.b = rT.b-1;           trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNY0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNY0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = 0;         tdof.b = DOFlattice.b-1;    tdof.c = dofT.c;
                                te.a   = eT.a+1;    te.b   = eT.b-1;            te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNY0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = in_Elattice.b-1;  te.c    = eT.c;
                                    trank.a = rT.a+1;   trank.b = rT.b-1;           trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNY0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNY0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = dofT.b;    tdof.c = 0;
                                te.a   = eT.a;      te.b   = eT.b;      te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNY0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = eT.b;     te.c    = 0;
                                    trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNY0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNY0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = 0;         tdof.b = dofT.b;    tdof.c = 0;
                                te.a   = eT.a+1;    te.b   = eT.b;      te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNY0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = eT.b;     te.c    = 0;
                                    trank.a = rT.a+1;   trank.b = rT.b;     trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNY0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNY0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = DOFlattice.b-1;    tdof.c = 0;
                                te.a   = eT.a;      te.b   = eT.b-1;            te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNY0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = in_Elattice.b-1;  te.c    = 0;
                                    trank.a = rT.a;     trank.b = rT.b-1;           trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNY0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNY0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = 0;         tdof.b = DOFlattice.b-1;    tdof.c = 0;
                                te.a   = eT.a+1;    te.b   = eT.b-1;            te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNY0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = in_Elattice.b-1;  te.c    = 0;
                                    trank.a = rT.a+1;   trank.b = rT.b-1;           trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNY0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNY0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                unsigned int s;
                                unsigned int mini = really_big_number; //Some really big number
                                for(s=0; s<sz_surfacers; ++s){
                                    if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                                }

                                if(sz_surfacers > 0 && mini == old_i){
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[old_i] += io_V[ surfacersIDs[s] ];
                                    }
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[ surfacersIDs[s] ] = io_V[old_i];
                                    }
                                    if(sz_values > old_sz_values){
                                        for(s=old_sz_values; s < sz_values; ++s){
                                            io_rankedValues[s].value = io_V[old_i];
                                        }
                                    }

                                    DBG_PRINTF6("CornerXNY0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                                   ifdef OUTPUT_HALO_DEBUG_PRINTS
                                        unsigned int ss;
                                        for(ss=0; ss<sz_surfacers; ++ss){
                                            DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                        }
                                        DBG_PRINTF0("\n");
#                                   endif
                                }
                                else{
                                    sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                                }

                                //End of //We are on corner XN Y0 ZN
                            }
                        }
                        else{
                            if(Z0) { //We are on corner XN YN Z0
                                old_sz_values = sz_values;
                                DBG_PRINTF3("CornerXNYNZ0--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                                sz_surfacers = 0;
                                surfacersIDs[sz_surfacers++] = old_i;

                                tdof.a = 0;         tdof.b = dofT.b;    tdof.c = dofT.c;
                                te.a   = eT.a+1;    te.b   = eT.b;      te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNYNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = eT.b;     te.c    = eT.c;
                                    trank.a = rT.a+1;   trank.b = rT.b;     trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNYNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNYNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = 0;         tdof.c = dofT.c;
                                te.a   = eT.a;      te.b   = eT.b+1;    te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNYNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = 0;        te.c    = eT.c;
                                    trank.a = rT.a;     trank.b = rT.b+1;   trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNYNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNYNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = 0;         tdof.b = 0;         tdof.c = dofT.c;
                                te.a   = eT.a+1;    te.b   = eT.b+1;    te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNYNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = 0;        te.c    = eT.c;
                                    trank.a = rT.a+1;   trank.b = rT.b+1;   trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNYNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNYNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = dofT.b;  tdof.c = DOFlattice.c-1;
                                te.a   = eT.a;      te.b   = eT.b;    te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNYNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = eT.b;     te.c    = in_Elattice.c-1;
                                    trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNYNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNYNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = 0;         tdof.b = dofT.b;  tdof.c = DOFlattice.c-1;
                                te.a   = eT.a+1;    te.b   = eT.b;    te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNYNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = eT.b;     te.c    = in_Elattice.c-1;
                                    trank.a = rT.a+1;   trank.b = rT.b;     trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNYNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNYNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = 0;         tdof.c = DOFlattice.c-1;
                                te.a   = eT.a;      te.b   = eT.b+1;    te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNYNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = 0;        te.c    = in_Elattice.c-1;
                                    trank.a = rT.a;     trank.b = rT.b+1;   trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNYNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNYNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = 0;         tdof.b = 0;         tdof.c = DOFlattice.c-1;
                                te.a   = eT.a+1;    te.b   = eT.b+1;    te.c   = eT.c-1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNYNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = 0;        te.c    = in_Elattice.c-1;
                                    trank.a = rT.a+1;   trank.b = rT.b+1;   trank.c = rT.c-1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNYNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNYNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                unsigned int s;
                                unsigned int mini = really_big_number; //Some really big number
                                for(s=0; s<sz_surfacers; ++s){
                                    if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                                }

                                if(sz_surfacers > 0 && mini == old_i){
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[old_i] += io_V[ surfacersIDs[s] ];
                                    }
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[ surfacersIDs[s] ] = io_V[old_i];
                                    }
                                    if(sz_values > old_sz_values){
                                        for(s=old_sz_values; s < sz_values; ++s){
                                            io_rankedValues[s].value = io_V[old_i];
                                        }
                                    }

                                    DBG_PRINTF6("CornerXNYNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                                   ifdef OUTPUT_HALO_DEBUG_PRINTS
                                        unsigned int ss;
                                        for(ss=0; ss<sz_surfacers; ++ss){
                                            DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                        }
                                        DBG_PRINTF0("\n");
#                                   endif
                                }
                                else{
                                    sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                                }

                                //End of //We are on corner XN YN Z0
                            }
                            else   { //We are on corner XN YN ZN
                                old_sz_values = sz_values;
                                DBG_PRINTF3("CornerXNYNZN--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                                sz_surfacers = 0;
                                surfacersIDs[sz_surfacers++] = old_i;

                                tdof.a = 0;         tdof.b = dofT.b;    tdof.c = dofT.c;
                                te.a   = eT.a+1;    te.b   = eT.b;      te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNYNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = eT.b;     te.c    = eT.c;
                                    trank.a = rT.a+1;   trank.b = rT.b;     trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNYNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNYNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = 0;         tdof.c = dofT.c;
                                te.a   = eT.a;      te.b   = eT.b+1;    te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNYNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = 0;        te.c    = eT.c;
                                    trank.a = rT.a;     trank.b = rT.b+1;   trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNYNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNYNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = 0;         tdof.b = 0;         tdof.c = dofT.c;
                                te.a   = eT.a+1;    te.b   = eT.b+1;    te.c   = eT.c;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNYNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = 0;        te.c    = eT.c;
                                    trank.a = rT.a+1;   trank.b = rT.b+1;   trank.c = rT.c;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNYNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNYNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = dofT.b;    tdof.c = 0;
                                te.a   = eT.a;      te.b   = eT.b;      te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNYNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = eT.b;     te.c    = 0;
                                    trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNYNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNYNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = 0;         tdof.b = dofT.b;    tdof.c = 0;
                                te.a   = eT.a+1;    te.b   = eT.b;      te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNYNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = eT.b;     te.c    = 0;
                                    trank.a = rT.a+1;   trank.b = rT.b;     trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNYNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNYNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = dofT.a;    tdof.b = 0;         tdof.c = 0;
                                te.a   = eT.a;      te.b   = eT.b+1;    te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNYNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = eT.a;     te.b    = 0;        te.c    = 0;
                                    trank.a = rT.a;     trank.b = rT.b+1;   trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNYNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNYNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                tdof.a = 0;         tdof.b = 0;         tdof.c = 0;
                                te.a   = eT.a+1;    te.b   = eT.b+1;    te.c   = eT.c+1;
                                if( isinLattice(te, in_Elattice) ){
                                    Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                    surfacersIDs[sz_surfacers++] = new_i;
                                    DBG_PRINTF8("CornerXNYNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                                } else {
                                    te.a    = 0;        te.b    = 0;        te.c    = 0;
                                    trank.a = rT.a+1;   trank.b = rT.b+1;   trank.c = rT.c+1;
                                    if( isinLattice(trank, in_Rlattice)){
                                        DBG_PRINTF9("NewRank CornerXNYNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                        Idz rid, dofeleID;
                                        posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                        set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                    } else {
                                        DBG_PRINTF9("Rejected CornerXNYNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    }
                                }

                                unsigned int s;
                                unsigned int mini = really_big_number; //Some really big number
                                for(s=0; s<sz_surfacers; ++s){
                                    if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                                }

                                if(sz_surfacers > 0 && mini == old_i){
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[old_i] += io_V[ surfacersIDs[s] ];
                                    }
                                    for(s=1; s<sz_surfacers; ++s){
                                        io_V[ surfacersIDs[s] ] = io_V[old_i];
                                    }
                                    if(sz_values > old_sz_values){
                                        for(s=old_sz_values; s < sz_values; ++s){
                                            io_rankedValues[s].value = io_V[old_i];
                                        }
                                    }

                                    DBG_PRINTF6("CornerXNYNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                                   ifdef OUTPUT_HALO_DEBUG_PRINTS
                                        unsigned int ss;
                                        for(ss=0; ss<sz_surfacers; ++ss){
                                            DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                        }
                                        DBG_PRINTF0("\n");
#                                   endif
                                }
                                else{
                                    sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                                }

                                //End of //We are on corner XN YN ZN
                            }
                        }
                    }
                }
                else if(  onX && !onY && !onZ) { //We are on FaceX
                    if(X0){
                        old_sz_values = sz_values;
                        DBG_PRINTF3("FaceX0--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                        tdof.a = in_pDOF - 1;   tdof.b = dofT.b;    tdof.c = dofT.c;
                        te.a   = eT.a-1;        te.b   = eT.b;      te.c   = eT.c;
                        if( isinLattice(te, in_Elattice) ){
                            DBG_PRINTF6("FaceX0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                            Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                            if(old_i > new_i){
                                io_V[old_i] += io_V[new_i];
                                io_V[new_i] = io_V[old_i];
                            }
                            DBG_PRINTF8("FaceX0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i, new_i);

                        } else{
                            trank.a = rT.a - 1;     trank.b = rT.b;         trank.c = rT.c;
                            te.a    = in_Elattice.a-1; te.b   = eT.b;      te.c   = eT.c;
                            if( isinLattice(trank, in_Rlattice)){
                                DBG_PRINTF9("NewRank FaceX0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                Idz rid, dofeleID;
                                posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                            } else {
                                DBG_PRINTF9("Rejected FaceX0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                            }
                        }
                    } else {
                        old_sz_values = sz_values;
                        DBG_PRINTF3("FaceXN--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                        tdof.a = 0;         tdof.b = dofT.b;    tdof.c = dofT.c;
                        te.a   = eT.a+1;    te.b   = eT.b;      te.c   = eT.c;
                        if( isinLattice(te, in_Elattice) ){
                            Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                            if(old_i > new_i){
                                io_V[old_i] += io_V[new_i];
                                io_V[new_i] = io_V[old_i];
                            }
                            DBG_PRINTF8("FaceXN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i, new_i);
                        } else{
                            trank.a = rT.a + 1;     trank.b = rT.b;        trank.c = rT.c;
                            te.a    = 0;            te.b   = eT.b;         te.c   = eT.c;
                            if( isinLattice(trank, in_Rlattice)){
                                DBG_PRINTF9("NewRank FaceXN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                Idz rid, dofeleID;
                                posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                            } else {
                                DBG_PRINTF9("Rejected FaceXN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                            }
                        }
                    }

                }
                else if( !onX &&  onY && !onZ) { //We are on FaceY
                    if(Y0){
                        old_sz_values = sz_values;
                        DBG_PRINTF3("FaceY0--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                        tdof.a = dofT.a;   tdof.b = in_pDOF - 1;    tdof.c = dofT.c;
                        te.a   = eT.a;     te.b   = eT.b-1;         te.c   = eT.c;
                        if( isinLattice(te, in_Elattice) ){
                            DBG_PRINTF6("FaceY0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                            Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                            if(old_i > new_i){
                                io_V[old_i] += io_V[new_i];
                                io_V[new_i] = io_V[old_i];
                            }
                            DBG_PRINTF8("FaceY0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i, new_i);

                        } else{
                            trank.a = rT.a;     trank.b = rT.b-1;              trank.c = rT.c;
                            te.a    = eT.b;      te.b   = in_Elattice.b-1;      te.c   = eT.c;
                            if( isinLattice(trank, in_Rlattice)){
                                DBG_PRINTF9("NewRank FaceY0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                Idz rid, dofeleID;
                                posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                            } else {
                                DBG_PRINTF9("Rejected FaceY0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                            }
                        }
                    } else {
                        old_sz_values = sz_values;
                        DBG_PRINTF3("FaceYN--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                        tdof.a = dofT.a;    tdof.b = 0;         tdof.c = dofT.c;
                        te.a   = eT.a;      te.b   = eT.b+1;    te.c   = eT.c;
                        if( isinLattice(te, in_Elattice) ){
                            Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                            if(old_i > new_i){
                                io_V[old_i] += io_V[new_i];
                                io_V[new_i] = io_V[old_i];
                            }
                            DBG_PRINTF8("FaceYN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i, new_i);
                        } else{
                            trank.a = rT.a;     trank.b = rT.b + 1;   trank.c = rT.c;
                            te.a    = eT.a;     te.b    = 0;          te.c    = eT.c;
                            if( isinLattice(trank, in_Rlattice)){
                                DBG_PRINTF9("NewRank FaceYN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                Idz rid, dofeleID;
                                posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                            } else {
                                DBG_PRINTF9("Rejected FaceYN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                            }
                        }
                    }
                }
                else if( !onX && !onY &&  onZ) { //We are on FaceZ
                    if(Z0){
                        old_sz_values = sz_values;
                        DBG_PRINTF3("FaceZ0--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                        tdof.a = dofT.a;   tdof.b = dofT.b;    tdof.c = in_pDOF - 1;
                        te.a   = eT.a;     te.b   = eT.b;      te.c   = eT.c - 1;
                        if( isinLattice(te, in_Elattice) ){
                            DBG_PRINTF6("FaceZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                            Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                            if(old_i > new_i){
                                io_V[old_i] += io_V[new_i];
                                io_V[new_i] = io_V[old_i];
                            }
                            DBG_PRINTF8("FaceZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);

                        } else{
                            trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c-1;
                            te.a    = eT.b;      te.b   = eT.b;     te.c    = in_Elattice.c-1;
                            if( isinLattice(trank, in_Rlattice)){
                                DBG_PRINTF9("NewRank FaceZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                Idz rid, dofeleID;
                                posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                            } else {
                                DBG_PRINTF9("Rejected FaceZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                            }
                        }
                    } else {
                        old_sz_values = sz_values;
                        DBG_PRINTF3("FaceZN--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                        tdof.a = dofT.a;    tdof.b = dofT.b;    tdof.c = 0;
                        te.a   = eT.a;      te.b   = eT.b;      te.c   = eT.c+1;
                        if( isinLattice(te, in_Elattice) ){
                            Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                            if(old_i > new_i){
                                io_V[old_i] += io_V[new_i];
                                io_V[new_i] = io_V[old_i];
                            }
                            DBG_PRINTF8("FaceZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                        } else{
                            trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c+1;
                            te.a    = eT.a;     te.b    = eT.b;     te.c    = 0;
                            if( isinLattice(trank, in_Rlattice)){
                                DBG_PRINTF9("NewRank FaceZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                Idz rid, dofeleID;
                                posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                            } else {
                                DBG_PRINTF9("Rejected FaceZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                            }
                        }
                    }
                }
                else if( !onX &&  onY &&  onZ){
                    if(Y0){
                        if(Z0) { //We are on constant-X Edge Y0 Z0
                            old_sz_values = sz_values;
                            DBG_PRINTF3("EdgeX_Y0Z0--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                            sz_surfacers = 0;
                            surfacersIDs[sz_surfacers++] = old_i;

                            tdof.a = dofT.a;    tdof.b = dofT.b;    tdof.c = DOFlattice.c-1;
                            te.a   = eT.a;      te.b   = eT.b;      te.c   = eT.c-1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_Y0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = eT.b;     te.c    = in_Elattice.c-1;
                                trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c-1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = dofT.a;    tdof.b = DOFlattice.b-1;    tdof.c = dofT.c;
                            te.a   = eT.a;      te.b   = eT.b-1;            te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_Y0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = in_Elattice.b-1;  te.c    = eT.c;
                                trank.a = rT.a;     trank.b = rT.b-1;           trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = dofT.a;    tdof.b = DOFlattice.b-1;    tdof.c = DOFlattice.c-1;
                            te.a   = eT.a;      te.b   = eT.b-1;            te.c   = eT.c-1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_Y0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = in_Elattice.b-1;  te.c    = in_Elattice.c-1;
                                trank.a = rT.a;     trank.b = rT.b-1;           trank.c = rT.c-1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_Y0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            unsigned int s;
                            unsigned int mini = really_big_number; //Some really big number
                            for(s=0; s<sz_surfacers; ++s){
                                if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                            }

                            if(sz_surfacers > 0 && mini == old_i){
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[old_i] += io_V[ surfacersIDs[s] ];
                                }
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[ surfacersIDs[s] ] = io_V[old_i];
                                }
                                if(sz_values > old_sz_values){
                                    for(s=old_sz_values; s < sz_values; ++s){
                                        io_rankedValues[s].value = io_V[old_i];
                                    }
                                }

                                DBG_PRINTF6("EdgeX_Y0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                               ifdef OUTPUT_HALO_DEBUG_PRINTS
                                    unsigned int ss;
                                    for(ss=0; ss<sz_surfacers; ++ss){
                                        DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                    }
                                    DBG_PRINTF0("\n");
#                               endif
                            }
                            else{
                                sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                            }

                            //End of //We are on constant-X Edge Y0 Z0
                        }
                        else   {  //We are on constant-X Edge Y0 ZN
                            old_sz_values = sz_values;
                            DBG_PRINTF3("EdgeX_Y0ZN--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                            sz_surfacers = 0;
                            surfacersIDs[sz_surfacers++] = old_i;

                            tdof.a = dofT.a;    tdof.b = dofT.b;    tdof.c = 0;
                            te.a   = eT.a;      te.b   = eT.b;      te.c   = eT.c+1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_Y0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = eT.b;     te.c    = 0;
                                trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c+1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = dofT.a;    tdof.b = DOFlattice.b-1;    tdof.c = dofT.c;
                            te.a   = eT.a;      te.b   = eT.b-1;            te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_Y0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = in_Elattice.b-1;  te.c    = eT.c;
                                trank.a = rT.a;     trank.b = rT.b-1;           trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = dofT.a;    tdof.b = DOFlattice.b-1;    tdof.c = 0;
                            te.a   = eT.a;      te.b   = eT.b-1;            te.c   = eT.c+1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_Y0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = in_Elattice.b-1;  te.c    = 0;
                                trank.a = rT.a;     trank.b = rT.b-1;           trank.c = rT.c+1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_Y0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            unsigned int s;
                            unsigned int mini = really_big_number; //Some really big number
                            for(s=0; s<sz_surfacers; ++s){
                                if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                            }

                            if(sz_surfacers > 0 && mini == old_i){
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[old_i] += io_V[ surfacersIDs[s] ];
                                }
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[ surfacersIDs[s] ] = io_V[old_i];
                                }
                                if(sz_values > old_sz_values){
                                    for(s=old_sz_values; s < sz_values; ++s){
                                        io_rankedValues[s].value = io_V[old_i];
                                    }
                                }

                                DBG_PRINTF6("EdgeX_Y0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                               ifdef OUTPUT_HALO_DEBUG_PRINTS
                                    unsigned int ss;
                                    for(ss=0; ss<sz_surfacers; ++ss){
                                        DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                    }
                                    DBG_PRINTF0("\n");
#                               endif
                            }
                            else{
                                sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                            }

                            //End of //We are on constant-X Edge Y0 ZN
                        }
                    }
                    else{
                        if(Z0) { //We are on constant-X Edge YN Z0
                            old_sz_values = sz_values;
                            DBG_PRINTF3("EdgeX_YNZ0--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                            sz_surfacers = 0;
                            surfacersIDs[sz_surfacers++] = old_i;

                            tdof.a = dofT.a;    tdof.b = 0;           tdof.c = dofT.c;
                            te.a   = eT.a;      te.b   = eT.b+1;      te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_YNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = 0   ;     te.c    = eT.c;
                                trank.a = rT.a;     trank.b = rT.b+1;   trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = dofT.a;    tdof.b = dofT.b;      tdof.c = DOFlattice.c-1;
                            te.a   = eT.a;      te.b   = eT.b;        te.c   = eT.c-1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_YNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = eT.b;     te.c    = in_Elattice.c-1;
                                trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c-1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = dofT.a;    tdof.b = 0;         tdof.c = DOFlattice.c-1;
                            te.a   = eT.a;      te.b   = eT.b+1;    te.c   = eT.c-1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_YNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = 0;        te.c    = in_Elattice.c-1;
                                trank.a = rT.a;     trank.b = rT.b+1;   trank.c = rT.c-1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_YNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            unsigned int s;
                            unsigned int mini = really_big_number; //Some really big number
                            for(s=0; s<sz_surfacers; ++s){
                                if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                            }

                            if(sz_surfacers > 0 && mini == old_i){
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[old_i] += io_V[ surfacersIDs[s] ];
                                }
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[ surfacersIDs[s] ] = io_V[old_i];
                                }
                                if(sz_values > old_sz_values){
                                    for(s=old_sz_values; s < sz_values; ++s){
                                        io_rankedValues[s].value = io_V[old_i];
                                    }
                                }

                                DBG_PRINTF6("EdgeX_YNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                               ifdef OUTPUT_HALO_DEBUG_PRINTS
                                    unsigned int ss;
                                    for(ss=0; ss<sz_surfacers; ++ss){
                                        DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                    }
                                    DBG_PRINTF0("\n");
#                               endif
                            }
                            else{
                                sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                            }

                            //End of //We are on constant-X Edge YN Z0
                        }
                        else   { //We are on constant-X Edge YN ZN
                            old_sz_values = sz_values;
                            DBG_PRINTF3("EdgeX_YNZN--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                            sz_surfacers = 0;
                            surfacersIDs[sz_surfacers++] = old_i;

                            tdof.a = dofT.a;    tdof.b = 0;           tdof.c = dofT.c;
                            te.a   = eT.a;      te.b   = eT.b+1;      te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = 0   ;     te.c    = eT.c;
                                trank.a = rT.a;     trank.b = rT.b+1;   trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = dofT.a;    tdof.b = dofT.b;      tdof.c = 0;
                            te.a   = eT.a;      te.b   = eT.b;        te.c   = eT.c+1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = eT.b;     te.c    = 0;
                                trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c+1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = dofT.a;    tdof.b = 0;         tdof.c = 0;
                            te.a   = eT.a;      te.b   = eT.b+1;    te.c   = eT.c+1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = 0;        te.c    = 0;
                                trank.a = rT.a;     trank.b = rT.b+1;   trank.c = rT.c+1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            unsigned int s;
                            unsigned int mini = really_big_number; //Some really big number
                            for(s=0; s<sz_surfacers; ++s){
                                if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                            }

                            if(sz_surfacers > 0 && mini == old_i){
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[old_i] += io_V[ surfacersIDs[s] ];
                                }
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[ surfacersIDs[s] ] = io_V[old_i];
                                }
                                if(sz_values > old_sz_values){
                                    for(s=old_sz_values; s < sz_values; ++s){
                                        io_rankedValues[s].value = io_V[old_i];
                                    }
                                }

                                DBG_PRINTF6("EdgeX_YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                               ifdef OUTPUT_HALO_DEBUG_PRINTS
                                    unsigned int ss;
                                    for(ss=0; ss<sz_surfacers; ++ss){
                                        DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                    }
                                    DBG_PRINTF0("\n");
#                               endif
                            }
                            else{
                                sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                            }

                            //End of //We are on constant-X Edge YN ZN
                        }//else off Z0
                    }//else off Y0
                }
                else if(  onX && !onY &&  onZ) { //We are on constant-Y Edge
                    if(X0){
                        if(Z0) { //We are on constant-Y Edge X0 Z0
                            old_sz_values = sz_values;
                            DBG_PRINTF3("EdgeY_X0Z0--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                            sz_surfacers = 0;
                            surfacersIDs[sz_surfacers++] = old_i;

                            tdof.a = dofT.a;    tdof.b = dofT.b;    tdof.c = DOFlattice.c-1;
                            te.a   = eT.a;      te.b   = eT.b;      te.c   = eT.c-1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeY_X0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = eT.b;     te.c    = in_Elattice.c-1;
                                trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c-1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeY_X0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeY_X0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = DOFlattice.a-1;    tdof.b = dofT.b;    tdof.c = dofT.c;
                            te.a   = eT.a-1;            te.b   = eT.b;      te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeY_X0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = in_Elattice.a-1;  te.b    = eT.b;     te.c    = eT.c;
                                trank.a = rT.a-1;           trank.b = rT.b;     trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeY_X0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeY_X0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = DOFlattice.a-1;    tdof.b = dofT.b;    tdof.c = DOFlattice.c-1;
                            te.a   = eT.a-1;            te.b   = eT.b;      te.c   = eT.c-1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeY_X0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = in_Elattice.a-1;  te.b    = eT.b;  te.c    = in_Elattice.c-1;
                                trank.a = rT.a-1;           trank.b = rT.b;  trank.c = rT.c-1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeY_X0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeY_X0Z0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            unsigned int s;
                            unsigned int mini = really_big_number; //Some really big number
                            for(s=0; s<sz_surfacers; ++s){
                                if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                            }

                            if(sz_surfacers > 0 && mini == old_i){
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[old_i] += io_V[ surfacersIDs[s] ];
                                }
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[ surfacersIDs[s] ] = io_V[old_i];
                                }
                                if(sz_values > old_sz_values){
                                    for(s=old_sz_values; s < sz_values; ++s){
                                        io_rankedValues[s].value = io_V[old_i];
                                    }
                                }

                                DBG_PRINTF6("EdgeY_X0Z0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                               ifdef OUTPUT_HALO_DEBUG_PRINTS
                                    unsigned int ss;
                                    for(ss=0; ss<sz_surfacers; ++ss){
                                        DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                    }
                                    DBG_PRINTF0("\n");
#                               endif
                            }
                            else{
                                sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                            }

                            //End of //We are on constant-Y Edge X0 Z0
                        }
                        else   { //We are on constant-Y Edge X0 ZN
                            old_sz_values = sz_values;
                            DBG_PRINTF3("EdgeY_X0ZN--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                            sz_surfacers = 0;
                            surfacersIDs[sz_surfacers++] = old_i;

                            tdof.a = dofT.a;    tdof.b = dofT.b;    tdof.c = 0;
                            te.a   = eT.a;      te.b   = eT.b;      te.c   = eT.c+1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeY_X0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = eT.b;     te.c    = 0;
                                trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c+1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeY_X0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeY_X0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = DOFlattice.a-1;    tdof.b = dofT.b;    tdof.c = dofT.c;
                            te.a   = eT.a-1;            te.b   = eT.b;      te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeY_X0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = in_Elattice.a-1;  te.b    = eT.b;     te.c    = eT.c;
                                trank.a = rT.a-1;           trank.b = rT.b;     trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeY_X0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeY_X0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = DOFlattice.a-1;    tdof.b = dofT.b;    tdof.c = 0;
                            te.a   = eT.a-1;            te.b   = eT.b;      te.c   = eT.c+1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeY_X0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = in_Elattice.a-1;  te.b    = eT.b;     te.c    = 0;
                                trank.a = rT.a-1;           trank.b = rT.b;     trank.c = rT.c+1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeY_X0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeY_X0ZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            unsigned int s;
                            unsigned int mini = really_big_number; //Some really big number
                            for(s=0; s<sz_surfacers; ++s){
                                if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                            }

                            if(sz_surfacers > 0 && mini == old_i){
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[old_i] += io_V[ surfacersIDs[s] ];
                                }
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[ surfacersIDs[s] ] = io_V[old_i];
                                }
                                if(sz_values > old_sz_values){
                                    for(s=old_sz_values; s < sz_values; ++s){
                                        io_rankedValues[s].value = io_V[old_i];
                                    }
                                }

                                DBG_PRINTF6("EdgeY_X0ZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                               ifdef OUTPUT_HALO_DEBUG_PRINTS
                                    unsigned int ss;
                                    for(ss=0; ss<sz_surfacers; ++ss){
                                        DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                    }
                                    DBG_PRINTF0("\n");
#                               endif
                            }
                            else{
                                sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                            }

                            //End of //We are on constant-Y Edge X0 ZN
                        }
                    }else{
                        if(Z0) { //We are on constant-Y Edge XN Z0
                            old_sz_values = sz_values;
                            DBG_PRINTF3("EdgeY_XNZ0--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                            sz_surfacers = 0;
                            surfacersIDs[sz_surfacers++] = old_i;

                            tdof.a = 0;         tdof.b = dofT.b;    tdof.c = dofT.c;
                            te.a   = eT.a+1;    te.b   = eT.b;      te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeY_XNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = 0;        te.b    = eT.b;     te.c    = eT.c;
                                trank.a = rT.a+1;   trank.b = rT.b;     trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeY_XNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeY_XNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = dofT.a;    tdof.b = dofT.b;      tdof.c = DOFlattice.c-1;
                            te.a   = eT.a;      te.b   = eT.b;        te.c   = eT.c-1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeY_XNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = eT.b;     te.c    = in_Elattice.c-1;
                                trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c-1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeY_XNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeY_XNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = 0;         tdof.b = dofT.b;    tdof.c = DOFlattice.c-1;
                            te.a   = eT.a+1;    te.b   = eT.b;      te.c   = eT.c-1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeY_XNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = 0;        te.b    = eT.b;     te.c    = in_Elattice.c-1;
                                trank.a = rT.a+1;   trank.b = rT.b;     trank.c = rT.c-1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeY_XNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeY_XNZ0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            unsigned int s;
                            unsigned int mini = really_big_number; //Some really big number
                            for(s=0; s<sz_surfacers; ++s){
                                if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                            }

                            if(sz_surfacers > 0 && mini == old_i){
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[old_i] += io_V[ surfacersIDs[s] ];
                                }
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[ surfacersIDs[s] ] = io_V[old_i];
                                }
                                if(sz_values > old_sz_values){
                                    for(s=old_sz_values; s < sz_values; ++s){
                                        io_rankedValues[s].value = io_V[old_i];
                                    }
                                }

                                DBG_PRINTF6("EdgeY_XNZ0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                               ifdef OUTPUT_HALO_DEBUG_PRINTS
                                    unsigned int ss;
                                    for(ss=0; ss<sz_surfacers; ++ss){
                                        DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                    }
                                    DBG_PRINTF0("\n");
#                               endif
                            }
                            else{
                                sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                            }

                            //End of //We are on constant-Y Edge XN Z0
                        }
                        else   { //We are on constant-Y Edge XN ZN
                            old_sz_values = sz_values;
                            DBG_PRINTF3("EdgeY_XNZN--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                            sz_surfacers = 0;
                            surfacersIDs[sz_surfacers++] = old_i;

                            tdof.a = 0;         tdof.b = dofT.b;    tdof.c = dofT.c;
                            te.a   = eT.a+1;    te.b   = eT.b;      te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = 0;        te.b    = eT.b;     te.c    = eT.c;
                                trank.a = rT.a+1;   trank.b = rT.b;     trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = dofT.a;    tdof.b = dofT.b;      tdof.c = 0;
                            te.a   = eT.a;      te.b   = eT.b;        te.c   = eT.c+1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = eT.b;     te.c    = 0;
                                trank.a = rT.a;     trank.b = rT.b;     trank.c = rT.c+1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = 0;         tdof.b = dofT.b;    tdof.c = 0;
                            te.a   = eT.a+1;    te.b   = eT.b;      te.c   = eT.c+1;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeX_YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = 0;        te.b    = eT.b;     te.c    = 0;
                                trank.a = rT.a+1;   trank.b = rT.b;     trank.c = rT.c+1;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeX_YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeX_YNZN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            unsigned int s;
                            unsigned int mini = really_big_number; //Some really big number
                            for(s=0; s<sz_surfacers; ++s){
                                if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                            }

                            if(sz_surfacers > 0 && mini == old_i){
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[old_i] += io_V[ surfacersIDs[s] ];
                                }
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[ surfacersIDs[s] ] = io_V[old_i];
                                }
                                if(sz_values > old_sz_values){
                                    for(s=old_sz_values; s < sz_values; ++s){
                                        io_rankedValues[s].value = io_V[old_i];
                                    }
                                }

                                DBG_PRINTF6("EdgeX_YNZN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                               ifdef OUTPUT_HALO_DEBUG_PRINTS
                                    unsigned int ss;
                                    for(ss=0; ss<sz_surfacers; ++ss){
                                        DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                    }
                                    DBG_PRINTF0("\n");
#                               endif
                            }
                            else{
                                sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                            }

                            //End of //We are on constant-Y Edge XN ZN
                        } //End of ZN
                    }//End of XN
                } //End of //We are on constant-Y Edge
                else if(  onX &&  onY && !onZ) { //We are on constant-Z Edge
                    if(X0){
                        if(Y0) { //We are on constant-Z Edge X0 Y0
                            old_sz_values = sz_values;
                            DBG_PRINTF3("EdgeZ_X0Y0--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                            sz_surfacers = 0;
                            surfacersIDs[sz_surfacers++] = old_i;

                            tdof.a = dofT.a;    tdof.b = DOFlattice.b-1;    tdof.c = dofT.c;
                            te.a   = eT.a;      te.b   = eT.b-1;            te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeZ_X0Y0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = in_Elattice.b-1;  te.c    = eT.c;
                                trank.a = rT.a;     trank.b = rT.b-1;           trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeZ_X0Y0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeZ_X0Y0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = DOFlattice.a-1;    tdof.b = dofT.b;    tdof.c = dofT.c;
                            te.a   = eT.a-1;            te.b   = eT.b;      te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeZ_X0Y0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = in_Elattice.a-1;  te.b    = eT.b;     te.c    = eT.c;
                                trank.a = rT.a-1;           trank.b = rT.b;     trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeZ_X0Y0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeZ_X0Y0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = DOFlattice.a-1;    tdof.b = DOFlattice.b-1;    tdof.c = dofT.c;
                            te.a   = eT.a-1;            te.b   = eT.b-1;            te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeZ_X0Y0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = in_Elattice.a-1;  te.b    = in_Elattice.b-1;  te.c    = eT.c;
                                trank.a = rT.a-1;           trank.b = rT.b-1;           trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeZ_X0Y0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeZ_X0Y0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            unsigned int s;
                            unsigned int mini = really_big_number; //Some really big number
                            for(s=0; s<sz_surfacers; ++s){
                                if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                            }

                            if(sz_surfacers > 0 && mini == old_i){
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[old_i] += io_V[ surfacersIDs[s] ];
                                }
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[ surfacersIDs[s] ] = io_V[old_i];
                                }
                                if(sz_values > old_sz_values){
                                    for(s=old_sz_values; s < sz_values; ++s){
                                        io_rankedValues[s].value = io_V[old_i];
                                    }
                                }

                                DBG_PRINTF6("EdgeZ_X0Y0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                               ifdef OUTPUT_HALO_DEBUG_PRINTS
                                    unsigned int ss;
                                    for(ss=0; ss<sz_surfacers; ++ss){
                                        DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                    }
                                    DBG_PRINTF0("\n");
#                               endif
                            }
                            else{
                                sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                            }

                            //End of //We are on constant-Z Edge X0 Y0
                        }
                        else   { //We are on constant-Z Edge X0 YN
                            old_sz_values = sz_values;
                            DBG_PRINTF3("EdgeZ_X0YN--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                            sz_surfacers = 0;
                            surfacersIDs[sz_surfacers++] = old_i;

                            tdof.a = dofT.a;    tdof.b = 0;         tdof.c = dofT.c;
                            te.a   = eT.a;      te.b   = eT.b+1;    te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeZ_X0YN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = 0;        te.c    = eT.c;
                                trank.a = rT.a;     trank.b = rT.b+1;   trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeZ_X0YN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeZ_X0YN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = DOFlattice.a-1;    tdof.b = dofT.b;    tdof.c = dofT.c;
                            te.a   = eT.a-1;            te.b   = eT.b;      te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeZ_X0YN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = in_Elattice.a-1;  te.b    = eT.b;     te.c    = eT.c;
                                trank.a = rT.a-1;           trank.b = rT.b;     trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeZ_X0YN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeZ_X0YN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = DOFlattice.a-1;    tdof.b = 0;         tdof.c = dofT.c;
                            te.a   = eT.a-1;            te.b   = eT.b+1;    te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeZ_X0YN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = in_Elattice.a-1;  te.b    = 0;        te.c    = eT.c;
                                trank.a = rT.a-1;           trank.b = rT.b+1;   trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeZ_X0YN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeZ_X0YN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            unsigned int s;
                            unsigned int mini = really_big_number; //Some really big number
                            for(s=0; s<sz_surfacers; ++s){
                                if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                            }

                            if(sz_surfacers > 0 && mini == old_i){
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[old_i] += io_V[ surfacersIDs[s] ];
                                }
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[ surfacersIDs[s] ] = io_V[old_i];
                                }
                                if(sz_values > old_sz_values){
                                    for(s=old_sz_values; s < sz_values; ++s){
                                        io_rankedValues[s].value = io_V[old_i];
                                    }
                                }

                                DBG_PRINTF6("EdgeZ_X0YN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                               ifdef OUTPUT_HALO_DEBUG_PRINTS
                                    unsigned int ss;
                                    for(ss=0; ss<sz_surfacers; ++ss){
                                        DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                    }
                                    DBG_PRINTF0("\n");
#                               endif
                            }
                            else{
                                sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                            }

                            //End of //We are on constant-Z Edge X0 YN
                        }
                    }
                    else{
                        if(Y0) { //We are on constant-Z Edge XN Y0
                            old_sz_values = sz_values;
                            DBG_PRINTF3("EdgeZ_XNY0--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                            sz_surfacers = 0;
                            surfacersIDs[sz_surfacers++] = old_i;

                            tdof.a = 0;         tdof.b = dofT.b;    tdof.c = dofT.c;
                            te.a   = eT.a+1;    te.b   = eT.b;      te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeZ_XNY0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = 0;        te.b    = eT.b;     te.c    = eT.c;
                                trank.a = rT.a+1;   trank.b = rT.b;     trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeZ_XNY0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeZ_XNY0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = dofT.a;    tdof.b = DOFlattice.b-1;    tdof.c = dofT.c;
                            te.a   = eT.a;      te.b   = eT.b-1;            te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeZ_XNY0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = in_Elattice.b-1;  te.c    = eT.c;
                                trank.a = rT.a;     trank.b = rT.b-1;           trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeZ_XNY0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeZ_XNY0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = 0;         tdof.b = DOFlattice.b-1;    tdof.c = dofT.c;
                            te.a   = eT.a+1;    te.b   = eT.b-1;            te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeZ_XNY0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = 0;        te.b    = in_Elattice.b-1;  te.c    = eT.c;
                                trank.a = rT.a+1;   trank.b = rT.b-1;           trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeZ_XNY0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeZ_XNY0--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            unsigned int s;
                            unsigned int mini = really_big_number; //Some really big number
                            for(s=0; s<sz_surfacers; ++s){
                                if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                            }

                            if(sz_surfacers > 0 && mini == old_i){
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[old_i] += io_V[ surfacersIDs[s] ];
                                }
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[ surfacersIDs[s] ] = io_V[old_i];
                                }
                                if(sz_values > old_sz_values){
                                    for(s=old_sz_values; s < sz_values; ++s){
                                        io_rankedValues[s].value = io_V[old_i];
                                    }
                                }

                                DBG_PRINTF6("EdgeZ_XNY0--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                               ifdef OUTPUT_HALO_DEBUG_PRINTS
                                    unsigned int ss;
                                    for(ss=0; ss<sz_surfacers; ++ss){
                                        DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                    }
                                    DBG_PRINTF0("\n");
#                               endif
                            }
                            else{
                                sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                            }

                            //End of //We are on constant-Z Edge XN Y0
                        }
                        else   { //We are on constant-Z Edge XN YN
                            old_sz_values = sz_values;
                            DBG_PRINTF3("EdgeZ_XNYN--> R=%u\tE=%u\td=%ld\n", in_rankID, eid, d);
                            sz_surfacers = 0;
                            surfacersIDs[sz_surfacers++] = old_i;

                            tdof.a = 0;         tdof.b = dofT.b;    tdof.c = dofT.c;
                            te.a   = eT.a+1;    te.b   = eT.b;      te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeZ_XNYN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = 0;        te.b    = eT.b;     te.c    = eT.c;
                                trank.a = rT.a+1;   trank.b = rT.b;     trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeZ_XNYN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeZ_XNYN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = dofT.a;    tdof.b = 0;         tdof.c = dofT.c;
                            te.a   = eT.a;      te.b   = eT.b+1;    te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeZ_XNYN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = eT.a;     te.b    = 0;        te.c    = eT.c;
                                trank.a = rT.a;     trank.b = rT.b+1;   trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeZ_XNYN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeZ_XNYN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            tdof.a = 0;         tdof.b = 0;         tdof.c = dofT.c;
                            te.a   = eT.a+1;    te.b   = eT.b+1;    te.c   = eT.c;
                            if( isinLattice(te, in_Elattice) ){
                                Idz new_i = elementLocalTriplets2index(DOFlattice, in_Elattice, tdof, te);
                                surfacersIDs[sz_surfacers++] = new_i;
                                DBG_PRINTF8("EdgeZ_XNYN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) old_i=%ld new_i=%ld\n", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c, old_i,new_i);
                            } else {
                                te.a    = 0;        te.b    = 0;        te.c    = eT.c;
                                trank.a = rT.a+1;   trank.b = rT.b+1;   trank.c = rT.c;
                                if( isinLattice(trank, in_Rlattice)){
                                    DBG_PRINTF9("NewRank EdgeZ_XNYN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                    Idz rid, dofeleID;
                                    posOnNewRank2indices(DOFlattice, in_Elattice, in_Rlattice, tdof, te, trank, &dofeleID, &rid);
                                    set_rankIndexedValue(io_rankedValues, sz_values++, rid, dofeleID, io_V[dofeleID]);
                                } else {
                                    DBG_PRINTF9("Rejected EdgeZ_XNYN--> newR=(%ld,%ld,%ld) newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld)\n", trank.a,trank.b,trank.c, te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
                                }
                            }

                            unsigned int s;
                            unsigned int mini = really_big_number; //Some really big number
                            for(s=0; s<sz_surfacers; ++s){
                                if( mini > surfacersIDs[s]) mini = surfacersIDs[s];
                            }

                            if(sz_surfacers > 0 && mini == old_i){
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[old_i] += io_V[ surfacersIDs[s] ];
                                }
                                for(s=1; s<sz_surfacers; ++s){
                                    io_V[ surfacersIDs[s] ] = io_V[old_i];
                                }
                                if(sz_values > old_sz_values){
                                    for(s=old_sz_values; s < sz_values; ++s){
                                        io_rankedValues[s].value = io_V[old_i];
                                    }
                                }

                                DBG_PRINTF6("EdgeZ_XNYN--> newE=(%ld,%ld,%ld) newDOF=(%ld,%ld,%ld) all_i=", te.a,te.b,te.c, tdof.a,tdof.b,tdof.c);
#                               ifdef OUTPUT_HALO_DEBUG_PRINTS
                                    unsigned int ss;
                                    for(ss=0; ss<sz_surfacers; ++ss){
                                        DBG_PRINTF1("%ld ", surfacersIDs[ss]);
                                    }
                                    DBG_PRINTF0("\n");
#                               endif
                            }
                            else{
                                sz_values = old_sz_values;  //Reject found newRank indexed values as they are not the mini.
                            }

                            //End of //We are on constant-Z Edge XN YN
                        }
                    }
                }
                else {
                    err=__LINE__; IFEB; //We should never get here.
                }IFEB;
            }IFEB; // for(d=0; d<pDOF3D; ++d)

        }IFEB; //for(e=0; e<E; ++e)

        sort_rankIndexedValue(sz_values, io_rankedValues);

        *io_sz_values = sz_values;

        break;
    }
    if(err){
        ocrPrintf("ERROR: halo_exchanges: %d\n", err);
    }
    return err;
}

Err_t find_in_nekoTools(Idz in_rank, NEKOtools_t * in_nekoTools, DIRid * o_did)
{
    *o_did = NOTA_DIRid;
    int i;
    for(i=0; i < in_nekoTools->sz_nloads; ++i){
        if( in_nekoTools->nloads[i].rid == in_rank){
            *o_did = in_nekoTools->nloads[i].did;
            break;
        }
    }
    if(*o_did == NOTA_DIRid){ return __LINE__;}
    return 0;
}

Err_t start_halo_multiplicity(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools,
                              Triplet in_Rlattice, Triplet in_Elattice,
                              NEKOglobals_t * in_globals, Idz in_sz_V, NBN_REAL * io_V,
                              rankIndexedValue_t * io_riValues, ocrGuid_t * io_destEDT)
{
    Err_t err=0;
    while(!err){
        if(!io_V || !io_riValues){ err=__LINE__; IFEB;}

#       ifdef NKEBONE_USE_CHANNEL_FOR_HALO_MULTIPLICITY
        Idz sz_riValues = 0;

//        err = halo_exchanges(in_Rlattice, in_Elattice, in_globals->pDOF, in_globals->rankID, io_V, &sz_riValues, io_riValues); IFEB;

        Triplet DOFlattice = {in_globals->pDOF, in_globals->pDOF, in_globals->pDOF};
        Idz sz_V = in_sz_V;
        err = halo_exchanges2(in_Rlattice,in_Elattice, DOFlattice, in_globals->rankID,
                              sz_V, io_V, &sz_riValues, io_riValues); IFEB;

        if(sz_riValues == 0) break; //Nothing more to do.

        char dir_found[NEKbone_regionCount]; //For sanity checking
        Idz i;
        for(i=0; i<NEKbone_regionCount; ++i){
            dir_found[i]=-1; //-1 for unknown
        }

        for(i=0; i < sz_riValues; ++i){
            //Find the sub-block with the same rank
            const Idz neighborRank = io_riValues[i].rankID;

            Idz j = i;
            while(j < sz_riValues && io_riValues[j].rankID == neighborRank){
                ++j;
            }
            Idz ii = i;
            i = j -1;

            //So the interval [ii,j[ has the rank neighborRank, in order to get its "direction" value.
            DIRid direction = NOTA_DIRid;
            err = find_in_nekoTools(neighborRank, in_nekoTools, &direction); IFEB;  //TODO: Optimize the search for did when doing halo exchange.
            dir_found[direction] = 1;

            //===== Package the value
            unsigned int length = j - ii;

            ocrGuid_t envelopGuid  = NULL_GUID;
            IndexedValue_t * envelopPtr = NULL;
            err = ocrDbCreate( &envelopGuid, (void**)&envelopPtr, (length+1) * sizeof(IndexedValue_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC); IFEB;

            //NOTE: Length of the array is put in offset zero
            envelopPtr[0].eleDofID = length;
            envelopPtr[0].value = length;  //Just in case for debugging.

            unsigned int k;
            for(k=0; k < length; ++k){
                envelopPtr[k+1].eleDofID = io_riValues[ii+k].eleDofID;
                envelopPtr[k+1].value    = io_riValues[ii+k].value;
            }

            //ocrPrintf("DBG3155> TaskTYPE=%d TaskID="GUIDF" start_halo_multiplicity myrank=%d dir=%ld borRank=%ld envelop="GUIDF"\n",
            //       in_edtType, GUIDA(in_thisEDT), in_nekoTools->mpiRank, direction, neighborRank, GUIDA(envelopGuid) );

            err = ocrDbRelease( envelopGuid ); IFEB;

            //===== Connect the channel events
            err = ocrEventSatisfy( in_globals->neighborChannels[direction].c4multi, envelopGuid); IFEB;  //This sends value's envelop to the neighbors

            //This sends the neighbor's value to our future self.
            unsigned int destinationSlot = direction;
            unsigned int actualSlot = SLOTCNT_offset0_channels4multiplicity + destinationSlot;
            err = ocrAddDependence( in_globals->myChannels[direction].c4multi, *io_destEDT, actualSlot, DB_MODE_RO); IFEB;
        }IFEB;

        //sanity checking
        for(i=0; i<NEKbone_regionCount; ++i){
            if(in_nekoTools->dir_present[i]){
                if(dir_found[i] != 1){
                    err = __LINE__;
                }
            } else{
                if(dir_found[i] != -1){
                    err = __LINE__;
                }
            }
        }IFEB;
#       endif // NKEBONE_USE_CHANNEL_FOR_HALO_MULTIPLICITY

        //Fill up the empty channel slots
        unsigned int dir;
        for(dir=0; dir < NEKbone_regionCount; ++dir){
#           ifdef NKEBONE_USE_CHANNEL_FOR_HALO_MULTIPLICITY
            if(in_nekoTools->dir_present[dir]){
                continue;
            }
#           endif // NKEBONE_USE_CHANNEL_FOR_HALO_MULTIPLICITY

            unsigned int destinationSlot = dir;
            unsigned int actualSlot = SLOTCNT_offset0_channels4multiplicity + destinationSlot;
            err = ocrAddDependence( NULL_GUID, *io_destEDT, actualSlot, DB_MODE_RO); IFEB;

            //ocrPrintf("DBG3196> TaskTYPE=%d TaskID="GUIDF" start_halo_multiplicity myrank=%d dir=%u actualSlot=%u NULL_GUID\n",
            //       in_edtType, GUIDA(in_thisEDT), in_nekoTools->mpiRank, dir, actualSlot);

        }IFEB;

        break;
    }
    if(err){
        ocrPrintf("ERROR: start_halo_multiplicity: %d\n", err);
    }
    return err;
}

Err_t stop_halo_multiplicity(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools, ocrEdtDep_t * in_depv,
                             NBN_REAL * io_V)
{
    Err_t err=0;
    while(!err){
#       ifdef NKEBONE_USE_CHANNEL_FOR_HALO_MULTIPLICITY
        ocrEdtDep_t * depvNeighborChannels = in_depv + SLOTCNT_offset0_channels4multiplicity;

        unsigned int dir;
        for(dir=0; dir!=NEKbone_regionCount; ++dir){
            if(in_nekoTools->dir_present[dir]){
                //We do get stuff on this depv.
                if(depvNeighborChannels[dir].ptr){
                    //ocrPrintf("DBG3222> TaskTYPE=%d TaskID="GUIDF" stop_halo_multiplicity myrank=%d dir=%u envelop="GUIDF"  %p\n",
                    //       in_edtType, GUIDA(in_thisEDT), in_nekoTools->mpiRank, dir, GUIDA(depvNeighborChannels[dir].guid), depvNeighborChannels[dir].ptr);
                    IndexedValue_t * ivals = (IndexedValue_t *) depvNeighborChannels[dir].ptr;

                    Idz length = ivals[0].eleDofID;
                    NBN_REAL dlength = ivals[0].value; //Just to check.

                    ++ivals; //Skip the first entry, the one at offset zero.

                    Idz i;
                    for(i=0; i<length; ++i){
                        Idz offset = ivals[i].eleDofID;
                        NBN_REAL x = ivals[i].value;
                        io_V[offset] += x;
                    }

                    err = ocrDbDestroy(depvNeighborChannels[dir].guid); IFEB; // Clean up used envelop DBK.
                }else{
                    err = __LINE__; break;
                }
            }else{
                //We are not supposed to receive something on this depv.
                if(depvNeighborChannels[dir].ptr){
                    err = __LINE__; break;
                }
            }
        }IFEB;
#       endif // NKEBONE_USE_CHANNEL_FOR_HALO_MULTIPLICITY
        break;
    }
    if(err){
        ocrPrintf("ERROR: stop_halo_multiplicity: %d\n", err);
    }
    return err;
}

Err_t start_halo_setf(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools,
                      Triplet in_Rlattice, Triplet in_Elattice,
                      NEKOglobals_t * in_globals, Idz in_sz_V, NBN_REAL * io_V,
                      rankIndexedValue_t * io_riValues, ocrGuid_t * io_destEDT)
{
    Err_t err=0;
    while(!err){
        if(!io_V || !io_riValues){ err=__LINE__; IFEB;}

#       ifdef NKEBONE_USE_CHANNEL_FOR_HALO_SETF
        Idz sz_riValues = 0;
//        err = halo_exchanges(in_Rlattice, in_Elattice, in_globals->pDOF, in_globals->rankID, io_V, &sz_riValues, io_riValues); IFEB;

        Triplet DOFlattice = {in_globals->pDOF, in_globals->pDOF, in_globals->pDOF};
        Idz sz_V = in_sz_V;
        err = halo_exchanges2(in_Rlattice,in_Elattice, DOFlattice, in_globals->rankID,
                              sz_V, io_V, &sz_riValues, io_riValues); IFEB;


        if(sz_riValues == 0) break; //Nothing more to do.

        char dir_found[NEKbone_regionCount]; //For sanity checking
        Idz i;
        for(i=0; i<NEKbone_regionCount; ++i){
            dir_found[i]=-1; //-1 for unknown
        }

        for(i=0; i < sz_riValues; ++i){
            //Find the sub-block with the same rank
            const Idz neighborRank = io_riValues[i].rankID;

            Idz j = i;
            while(j < sz_riValues && io_riValues[j].rankID == neighborRank){
                ++j;
            }
            Idz ii = i;
            i = j -1;

            //So the interval [ii,j[ has the rank neighborRank, in order to get its "direction" value.
            DIRid direction = NOTA_DIRid;
            err = find_in_nekoTools(neighborRank, in_nekoTools, &direction); IFEB;  //TODO: Optimize the search for did when doing halo exchange.
            dir_found[direction] = 1;

            //===== Package the value
            unsigned int length = j - ii;

            ocrGuid_t envelopGuid  = NULL_GUID;
            IndexedValue_t * envelopPtr = NULL;
            err = ocrDbCreate( &envelopGuid, (void**)&envelopPtr, (length+1) * sizeof(IndexedValue_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC); IFEB;

            //NOTE: Length of the array is put in offset zero
            envelopPtr[0].eleDofID = length;
            envelopPtr[0].value = length;  //Just in case for debugging.

            unsigned int k;
            for(k=0; k < length; ++k){
                envelopPtr[k+1].eleDofID = io_riValues[ii+k].eleDofID;
                envelopPtr[k+1].value    = io_riValues[ii+k].value;
            }
            err = ocrDbRelease( envelopGuid ); IFEB;

            //===== Connect the channel events
            err = ocrEventSatisfy( in_globals->neighborChannels[direction].c4setf, envelopGuid); IFEB;  //This sends value's envelop to the neighbors

            //This sends the neighbor's value to our future self.
            unsigned int destinationSlot = direction;
            unsigned int actualSlot = SLOTCNT_offset0_channels4setf + destinationSlot;
            err = ocrAddDependence( in_globals->myChannels[direction].c4setf, *io_destEDT, actualSlot, DB_MODE_RO); IFEB;
        }IFEB;

        //sanity checking
        for(i=0; i<NEKbone_regionCount; ++i){
            if(in_nekoTools->dir_present[i]){
                if(dir_found[i] != 1){
                    err = __LINE__;
                }
            } else{
                if(dir_found[i] != -1){
                    err = __LINE__;
                }
            }
        }IFEB;
#       endif // NKEBONE_USE_CHANNEL_FOR_HALO_SETF

        //Fill up the empty channel slots
        unsigned int dir;
        for(dir=0; dir < NEKbone_regionCount; ++dir){
#           ifdef NKEBONE_USE_CHANNEL_FOR_HALO_SETF
            if(in_nekoTools->dir_present[dir]){
                continue;
            }
#           endif // NKEBONE_USE_CHANNEL_FOR_HALO_SETF

            unsigned int destinationSlot = dir;
            unsigned int actualSlot = SLOTCNT_offset0_channels4setf + destinationSlot;
            err = ocrAddDependence( NULL_GUID, *io_destEDT, actualSlot, DB_MODE_RO); IFEB;

            //ocrPrintf("DBG> TaskTYPE=%d TaskID="GUIDF" start_halo_setf myrank=%u dir=%u destSlot=%u actualSlot=%u NULL_GUID\n",
            //       in_edtType, GUIDA(in_thisEDT), G->rankID, dir, destinationSlot, actualSlot);

        }IFEB;

        break;
    }
    if(err){
        ocrPrintf("ERROR: start_halo_setf: %d\n", err);
    }
    return err;
}

Err_t stop_halo_setf(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools, ocrEdtDep_t * in_depv,
                     NBN_REAL * io_V)
{
    Err_t err=0;
    while(!err){
#       ifdef NKEBONE_USE_CHANNEL_FOR_HALO_SETF
        ocrEdtDep_t * depvNeighborChannels = in_depv + SLOTCNT_offset0_channels4setf;

        unsigned int dir;
        for(dir=0; dir!=NEKbone_regionCount; ++dir){
            if(in_nekoTools->dir_present[dir]){
                //We do get stuff on this depv.
                if(depvNeighborChannels[dir].ptr){
                    IndexedValue_t * ivals = (IndexedValue_t *) depvNeighborChannels[dir].ptr;

                    Idz length = ivals[0].eleDofID;
                    NBN_REAL dlength = ivals[0].value; //Just to check.

                    ++ivals; //Skip the first entry, the one at offset zero.

                    Idz i;
                    for(i=0; i<length; ++i){
                        Idz offset = ivals[i].eleDofID;
                        NBN_REAL x = ivals[i].value;
                        io_V[offset] += x;
                    }

                    err = ocrDbDestroy(depvNeighborChannels[dir].guid); IFEB; // Clean up used envelop DBK.
                }else{
                    err = __LINE__; break;
                }
            }else{
                //We are not supposed to receive something on this depv.
                if(depvNeighborChannels[dir].ptr){
                    err = __LINE__; break;
                }
            }
        }IFEB;
#       endif // NKEBONE_USE_CHANNEL_FOR_HALO_SETF
        break;
    }
    if(err){
        ocrPrintf("ERROR: stop_halo_setf: %d\n", err);
    }
    return err;
}

Err_t start_halo_ai(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools,
                    Triplet in_Rlattice, Triplet in_Elattice,
                    NEKOglobals_t * in_globals, Idz in_sz_V, NBN_REAL * io_V,
                    rankIndexedValue_t * io_riValues, ocrGuid_t * io_destEDT)
{
    Err_t err=0;
    while(!err){
        if(!io_V || !io_riValues){ err=__LINE__; IFEB;}

#       ifdef NKEBONE_USE_CHANNEL_FOR_HALO_AI
        Idz sz_riValues = 0;
//        err = halo_exchanges(in_Rlattice, in_Elattice, in_globals->pDOF, in_globals->rankID, io_V, &sz_riValues, io_riValues); IFEB;

        Triplet DOFlattice = {in_globals->pDOF, in_globals->pDOF, in_globals->pDOF};
        Idz sz_V = in_sz_V;
        err = halo_exchanges2(in_Rlattice,in_Elattice, DOFlattice, in_globals->rankID,
                              sz_V, io_V, &sz_riValues, io_riValues); IFEB;

        if(sz_riValues == 0) break; //Nothing more to do.

        char dir_found[NEKbone_regionCount]; //For sanity checking
        Idz i;
        for(i=0; i<NEKbone_regionCount; ++i){
            dir_found[i]=-1; //-1 for unknown
        }
        //ocrPrintf("DBG3428> rank=%d sz_riValues=%ld\n", in_nekoTools->mpiRank, sz_riValues);
        for(i=0; i < sz_riValues; ++i){
            //Find the sub-block with the same rank
            const Idz neighborRank = io_riValues[i].rankID;

            Idz j = i;
            while(j < sz_riValues && io_riValues[j].rankID == neighborRank){
                ++j;
            }
            Idz ii = i;
            i = j -1;

            //So the interval [ii,j[ has the rank neighborRank, in order to get its "direction" value.
            DIRid direction = NOTA_DIRid;
            err = find_in_nekoTools(neighborRank, in_nekoTools, &direction); IFEB;  //TODO: Optimize the search for did when doing halo exchange.
            dir_found[direction] = 1;

            //===== Package the value
            unsigned int length = j - ii;

            ocrGuid_t envelopGuid  = NULL_GUID;
            IndexedValue_t * envelopPtr = NULL;
            err = ocrDbCreate( &envelopGuid, (void**)&envelopPtr, (length+1) * sizeof(IndexedValue_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC); IFEB; //+1 to store length

            //NOTE: Length of the array is put in offset zero
            envelopPtr[0].eleDofID = length;
            envelopPtr[0].value = length;  //Just in case for debugging.

            unsigned int k;
            for(k=0; k < length; ++k){
                envelopPtr[k+1].eleDofID = io_riValues[ii+k].eleDofID;
                envelopPtr[k+1].value    = io_riValues[ii+k].value;
            }

            //ocrPrintf("DBG3474> TaskTYPE=%d TaskID="GUIDF" start_halo_ai myrank=%d dir=%ld borRank=%ld envelop="GUIDF" content=%p count=%lu\n",
            //       in_edtType, GUIDA(in_thisEDT), in_nekoTools->mpiRank, direction, neighborRank, GUIDA(envelopGuid), envelopPtr, length);

            err = ocrDbRelease( envelopGuid ); IFEB;

            //ocrPrintf("DBG3428> rank=%d i=%ld length=%u direction=%ld\n", in_nekoTools->mpiRank, i, length, direction);

            //===== Connect the channel events
            err = ocrEventSatisfy( in_globals->neighborChannels[direction].c4axi, envelopGuid); IFEB;  //This sends value's envelop to the neighbors

            //This sends the neighbor's value to our future self.
            unsigned int destinationSlot = direction;
            unsigned int actualSlot = SLOTCNT_offset0_channels4ax + destinationSlot;
            err = ocrAddDependence( in_globals->myChannels[direction].c4axi, *io_destEDT, actualSlot, DB_MODE_RO); IFEB;
        }IFEB;

        //sanity checking
        for(i=0; i<NEKbone_regionCount; ++i){
            if(in_nekoTools->dir_present[i]){
                if(dir_found[i] != 1){
                    err = __LINE__;
                }
            } else{
                if(dir_found[i] != -1){
                    err = __LINE__;
                }
            }
        }IFEB;
#       endif // NKEBONE_USE_CHANNEL_FOR_HALO_AI

        //Fill up the empty channel slots
        unsigned int dir;
        for(dir=0; dir < NEKbone_regionCount; ++dir){
#           ifdef NKEBONE_USE_CHANNEL_FOR_HALO_AI
            if(in_nekoTools->dir_present[dir]){
                continue;
            }
#           endif // NKEBONE_USE_CHANNEL_FOR_HALO_AI

            unsigned int destinationSlot = dir;
            unsigned int actualSlot = SLOTCNT_offset0_channels4ax + destinationSlot;
            err = ocrAddDependence( NULL_GUID, *io_destEDT, actualSlot, DB_MODE_RO); IFEB;

            //ocrPrintf("DBG3517> TaskTYPE=%d TaskID="GUIDF" start_halo_ai myrank=%d dir=%u actualSlot=%u NULL_GUID\n",
            //       in_edtType, GUIDA(in_thisEDT), in_nekoTools->mpiRank, dir, actualSlot);

        }IFEB;

        break;
    }
    if(err){
        ocrPrintf("ERROR: start_halo_ai: %d\n", err);
    }
    return err;
}

Err_t stop_halo_ai(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools, ocrEdtDep_t * in_depv,
                   NBN_REAL * io_V)
{
    Err_t err=0;
    while(!err){
#       ifdef NKEBONE_USE_CHANNEL_FOR_HALO_AI
        ocrEdtDep_t * depvNeighborChannels = in_depv + SLOTCNT_offset0_channels4ax;

        unsigned int dir;
        for(dir=0; dir!=NEKbone_regionCount; ++dir){
            if(in_nekoTools->dir_present[dir]){
                //We do get stuff on this depv.
                if(depvNeighborChannels[dir].ptr){
                    IndexedValue_t * ivals = (IndexedValue_t *) depvNeighborChannels[dir].ptr;

                    Idz length = ivals[0].eleDofID;
                    NBN_REAL dlength = ivals[0].value; //Just to check.
                    //ocrPrintf("DBG3543> TaskTYPE=%d TaskID="GUIDF" stop_halo_ai myrank=%d dir=%u envelop="GUIDF"  content=%p count=%ld\n",
                    //         in_edtType, GUIDA(in_thisEDT), in_nekoTools->mpiRank, dir, GUIDA(depvNeighborChannels[dir].guid), depvNeighborChannels[dir].ptr, length);
                    if("debug"){
                        Idz d = -1;
                        d = (Idz)(dlength + 0.5);
                        if( length != d){
                            ocrPrintf("ERROR: stop_halo_ai: dir=%u length=%ld dlength=%24.14e\n", dir, length, dlength);
                            err=__LINE__; break;
                        }
                    }

                    ++ivals; //Skip the first entry, the one at offset zero.

                    Idz i;
                    for(i=0; i<length; ++i){
                        //ocrPrintf("DBG3562> stop_halo_ai myrank=%d dir=%u envelop="GUIDF" i=%ld offset=%ld x=%24.14e\n",
                        //       in_nekoTools->mpiRank,dir, GUIDA(depvNeighborChannels[dir].guid), i, ivals[i].eleDofID, ivals[i].value);
                        Idz offset = ivals[i].eleDofID;
                        NBN_REAL x = ivals[i].value;
                        io_V[offset] += x;
                    }

                    err = ocrDbDestroy(depvNeighborChannels[dir].guid); IFEB; // Clean up used envelop DBK.
                }else{
                    err = __LINE__; break;
                }
            }else{
                //We are not supposed to receive something on this depv.
                if(depvNeighborChannels[dir].ptr){
                    err = __LINE__; break;
                }
            }
        }IFEB;
#       endif // NKEBONE_USE_CHANNEL_FOR_HALO_AI
        break;
    }
    if(err){
        ocrPrintf("ERROR: stop_halo_ax: %d\n", err);
    }
    return err;
}

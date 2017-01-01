#ifndef INCLUSION_UTEST_CHANNELG_H
#include "utest_channelG.h"
#endif

#define ENABLE_EXTENSION_LABELING
#include<extensions/ocr-labeling.h>

#define UTEST_CHANNEL_VERBOSE //Define this macro in order to get many PRINTF activated.
                              //Otherwise, to keep silent, undefine the macro.

Err_t create_utcLedger(OA_DEBUG_ARGUMENT, utcLedger_t * io_ledger, ocrGuid_t * in_whenDone)
{
    Err_t err=0;
    while(!err){
        io_ledger->rankid = NOTA_RANK;

        GUID_ASSIGN_VALUE(io_ledger->whenDone, *in_whenDone);  //No default constructor.  So this will have to do.
        GUID_ASSIGN_VALUE(io_ledger->labeledGuids, NULL_GUID); //No default constructor.  So this will have to do.

        GUID_ASSIGN_VALUE(io_ledger->myChannel, NULL_GUID);       //No default constructor.  So this will have to do.
        GUID_ASSIGN_VALUE(io_ledger->neighborChannel, NULL_GUID); //No default constructor.  So this will have to do.

        err = ocrGuidRangeCreate( &io_ledger->labeledGuids, RANK_COUNT, GUID_USER_EVENT_STICKY); IFEB;

        if( ocrGuidIsNull(io_ledger->labeledGuids) ){
            PRINTF("ERROR: Ledger creation in create_utcLedger failed: labeledGuids is NULL.\n");
            err = __LINE__;
            IFEB;
        }

        GUID_ASSIGN_VALUE(io_ledger->temp_myLabelEvt, NULL_GUID);

        print_utcLedger(OA_DEBUG_INVARS, io_ledger, "create_utcLedger");
        break;  //  while(!err)
    }
    return err;
}

Err_t destroy_utcLedger(OA_DEBUG_ARGUMENT, utcLedger_t * io_ledger)
{
    Err_t err=0;
    while(!err){
        //print_utcLedger(OA_DEBUG_INVARS, io_ledger, "destroy_utcLedger");
        if( ! ocrGuidIsEq(io_ledger->labeledGuids, NULL_GUID) ){
            err = ocrGuidMapDestroy(io_ledger->labeledGuids); IFEB;
            GUID_ASSIGN_VALUE(io_ledger->labeledGuids, NULL_GUID);
        }

        if( ! ocrGuidIsNull(io_ledger->myChannel) ){
            err = ocrEventDestroy(io_ledger->myChannel); IFEB;
            GUID_ASSIGN_VALUE(io_ledger->myChannel, NULL_GUID);
        }

        if( ! ocrGuidIsNull(io_ledger->neighborChannel) ){
            err = ocrEventDestroy(io_ledger->neighborChannel); IFEB;
            GUID_ASSIGN_VALUE(io_ledger->neighborChannel, NULL_GUID);
        }

        if( ! ocrGuidIsNull(io_ledger->temp_myLabelEvt) ){
            err = ocrEventDestroy(io_ledger->temp_myLabelEvt); IFEB;
            GUID_ASSIGN_VALUE(io_ledger->temp_myLabelEvt, NULL_GUID);
        }

        err = clear_utcLedger(OA_DEBUG_INVARS, io_ledger); IFEB;
        break;  //  while(!err)
    }
    return err;
}

Err_t clear_utcLedger(OA_DEBUG_ARGUMENT, utcLedger_t * io_ledger)
{
    Err_t err=0;
    while(!err){
        io_ledger->rankid = NOTA_RANK;

        GUID_ASSIGN_VALUE(io_ledger->whenDone, NULL_GUID);
        GUID_ASSIGN_VALUE(io_ledger->labeledGuids, NULL_GUID);
        GUID_ASSIGN_VALUE(io_ledger->myChannel, NULL_GUID);
        GUID_ASSIGN_VALUE(io_ledger->neighborChannel, NULL_GUID);
        GUID_ASSIGN_VALUE(io_ledger->temp_myLabelEvt, NULL_GUID);
        break;  //  while(!err)
    }
    return err;
}

void copy_utcLedger(OA_DEBUG_ARGUMENT, utcLedger_t * in_source, utcLedger_t * io_target)
{
    //print_utcLedger(OA_DEBUG_INVARS, in_source, "copy_utcLedger Source");

    io_target->rankid = in_source->rankid;

    GUID_ASSIGN_VALUE(io_target->whenDone, in_source->whenDone);
    GUID_ASSIGN_VALUE(io_target->labeledGuids, in_source->labeledGuids);
    GUID_ASSIGN_VALUE(io_target->myChannel, in_source->myChannel);
    GUID_ASSIGN_VALUE(io_target->neighborChannel, in_source->neighborChannel);
    GUID_ASSIGN_VALUE(io_target->temp_myLabelEvt, in_source->temp_myLabelEvt);

    //print_utcLedger(OA_DEBUG_INVARS, io_target, "copy_utcLedger Target");
}

void print_utcLedger(OA_DEBUG_ARGUMENT, utcLedger_t * in_ledger, const char * in_text)
{
#   ifdef UTEST_CHANNEL_VERBOSE
        const char * defaultt = "";
        const char * t = in_text;
        if(!in_text){
            t = defaultt;
        }

        ocrGuid_t labeled;
        ocrGuid_t wdone;
        ocrGuid_t myChan;
        ocrGuid_t borChan;
        ocrGuid_t tempEvt;

        GUID_ASSIGN_VALUE(labeled, NULL_GUID);
        GUID_ASSIGN_VALUE(wdone,   NULL_GUID);
        GUID_ASSIGN_VALUE(myChan,  NULL_GUID);
        GUID_ASSIGN_VALUE(borChan, NULL_GUID);
        GUID_ASSIGN_VALUE(tempEvt, NULL_GUID);

        if( ! ocrGuidIsNull(in_thisEDT)){
            GUID_ASSIGN_VALUE(labeled, in_ledger->labeledGuids);
            GUID_ASSIGN_VALUE(wdone,   in_ledger->whenDone);
            GUID_ASSIGN_VALUE(myChan,  in_ledger->myChannel);
            GUID_ASSIGN_VALUE(borChan, in_ledger->neighborChannel);
            GUID_ASSIGN_VALUE(tempEvt, in_ledger->temp_myLabelEvt);
        }

        PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" UTEST CHANNEL LEDGER Rankid=%u LabeledEVTs="GUIDF" whenDone="GUIDF" myChannel="GUIDF" BorChannel="GUIDF" tempEvt="GUIDF" %s\n",
                in_edtType, GUIDA(in_thisEDT),
                in_ledger->rankid,
                GUIDA(labeled), GUIDA(wdone),
                GUIDA(myChan), GUIDA(borChan),
                GUIDA(tempEvt),
                t);
#   endif
}

Err_t Userfcn_MainEdt(OA_DEBUG_ARGUMENT, ocrGuid_t * in_finalEdt, utcLedger_t * io_leftLedger,
                      utcLedger_t * io_rightLedger, utcLedger_t * io_refLedger)
{
    Err_t err=0;
    while(!err){
#       ifdef UTEST_CHANNEL_VERBOSE
            PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF"  In Main\n", in_edtType, GUIDA(in_thisEDT));
#       endif

        err = create_utcLedger(OA_DEBUG_INVARS, io_refLedger, in_finalEdt); IFEB;
        io_refLedger->rankid = NOTA_RANK;

        copy_utcLedger(OA_DEBUG_INVARS, io_refLedger, io_leftLedger);
        io_leftLedger->rankid = RANK_LEFT;

        copy_utcLedger(OA_DEBUG_INVARS, io_refLedger, io_rightLedger);
        io_rightLedger->rankid = RANK_RIGHT;

        break;  // while(!err)
    }
    return err;
}

Err_t Userfcn_FinalEdt(OA_DEBUG_ARGUMENT, utcLedger_t * io_refLedger, utcLedger_t * io_leftLedger,
                       utcLedger_t * io_rightLedger)
{
    Err_t err=0;
    while(!err){
#       ifdef UTEST_CHANNEL_VERBOSE
            PRINTF("TESTIO> In Final\n");
#       endif
        err = clear_utcLedger(OA_DEBUG_INVARS, io_leftLedger); IFEB;
        err = clear_utcLedger(OA_DEBUG_INVARS, io_rightLedger); IFEB;
        err = destroy_utcLedger(OA_DEBUG_INVARS, io_refLedger); IFEB;
        break;  // while(!err)
    }
    return err;
}

unsigned int utcGet_neighbor_rank(unsigned int in_rankid)
{
    if(in_rankid == RANK_LEFT) return RANK_RIGHT;
    if(in_rankid == RANK_RIGHT) return RANK_LEFT;
    return NOTA_RANK;
}

unsigned int utcGet_next_slot(unsigned int in_rankid)
{
    return 0;
}

unsigned int utcGet_neighbor_slot(unsigned int in_rankid)
{
    return 1;
}

Err_t make_labeledEvents(OA_DEBUG_ARGUMENT, utcLedger_t * io_ledger, ocrGuid_t * io_myEvent, unsigned int in_myRank,
                         ocrGuid_t * io_neighborEvent, unsigned int in_neighborRank)
{
   Err_t err=0;
    while(!err){
        err = ocrGuidFromIndex( io_myEvent, io_ledger->labeledGuids, in_myRank );
        if(err){
            PRINTF("ERROR: Userfcn_BeginInit[%u]: Creation of the Send labeled EVT failed: err=%u.\n", in_myRank, err);
            IFEB;
        }
        err = ocrEventCreate( io_myEvent, OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );
        if(err){
            if( err == OCR_EGUIDEXISTS ){
                //Someone has already created an object associated with this o_guid.
                //This is usually the expected behavior. --> Just ignore it.
                err = 0;
            }else{
                PRINTF("ERROR> TaskTYPE=%d TaskID="GUIDF" Userfcn_BeginInit[rank=%u]> Creation of io_myEvent EVT failed: %d\n",
                       in_edtType, GUIDA(in_thisEDT), in_myRank, err);
                IFEB;
            }
        }

        err = ocrGuidFromIndex( io_neighborEvent, io_ledger->labeledGuids, in_neighborRank );
        if(err){
            PRINTF("ERROR: Userfcn_BeginInit[%u]: Creation of the Receiving labeled EVT failed: err=%u.\n", in_myRank, err);
            IFEB;
        }
        err = ocrEventCreate( io_neighborEvent, OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );
        if(err){
            if( err == OCR_EGUIDEXISTS ){
                //Someone has already created an object associated with this o_guid.
                //This is usually the expected behavior. --> Just ignore it.
                err = 0;
            }else{
                PRINTF("ERROR> TaskTYPE=%d TaskID="GUIDF" Userfcn_BeginInit[rank=%u]> Creation of io_neighborEvent EVT (rank=%u) failed: %d\n",
                       in_edtType, GUIDA(in_thisEDT), in_myRank, in_neighborRank, err);
                IFEB;
            }
        }
        break;  //  while(!err)
    }
    return err;
}

Err_t Userfcn_BeginInit(OA_DEBUG_ARGUMENT, utcLedger_t * in_ledger, utcLedger_t * o_ledger,
                        ocrGuid_t * io_destEDT, unsigned int in_destSlot)
{
    Err_t err=0;
    while(!err){
        const unsigned int rankID = in_ledger->rankid;
        const unsigned int neighborRank = utcGet_neighbor_rank(rankID);

#       ifdef UTEST_CHANNEL_VERBOSE
            PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" Userfcn_BeginInit[%u] neighborID=%u\n", in_edtType, GUIDA(in_thisEDT), rankID, neighborRank);
#       endif

        copy_utcLedger(OA_DEBUG_INVARS, in_ledger, o_ledger);

        //===== Create labeled EVTs
        ocrGuid_t myLabelEvt = NULL_GUID;
        ocrGuid_t neighborLabelEvt = NULL_GUID;
        err = make_labeledEvents(OA_DEBUG_INVARS, o_ledger, &myLabelEvt, rankID, &neighborLabelEvt, neighborRank); IFEB;

        //===== Make my channel event for this pairing
        ocrEventParams_t params;
        params.EVENT_CHANNEL.maxGen = 2;
        params.EVENT_CHANNEL.nbSat  = 1;
        params.EVENT_CHANNEL.nbDeps = 1;

        ocrGuid_t myChannel = NULL_GUID;
        err = ocrEventCreateParams( &myChannel, OCR_EVENT_CHANNEL_T, EVT_PROP_NONE, &params ); IFEB;
        GUID_ASSIGN_VALUE(o_ledger->myChannel, myChannel);      //Record myChannel for future use.
        GUID_ASSIGN_VALUE(o_ledger->temp_myLabelEvt, myLabelEvt);  //Record for later destruction.

        //DBG> print_utcLedger(OA_DEBUG_INVARS, o_ledger, "BeginInit o_ledger");

        //===== Package the channel event for sending
        ocrGuid_t envelopGuid  = NULL_GUID;
        ocrGuid_t * envelopPtr = NULL;
        err = ocrDbCreate( &envelopGuid, (void**)&envelopPtr, sizeof(ocrGuid_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC); IFEB;

        GUID_ASSIGN_VALUE(*envelopPtr, myChannel);
        err = ocrDbRelease( envelopGuid );

        //===== Connect the labeled events
        err = ocrEventSatisfy( neighborLabelEvt, envelopGuid); IFEB;  //This sends myChannel to the neighbors

        //This sends the neighbor's Channel to our future self.
        err = ocrAddDependence( myLabelEvt, *io_destEDT, in_destSlot, DB_MODE_RO); IFEB;

        break;  // while(!err)
    }
    return err;
}

Err_t Userfcn_EndInit(OA_DEBUG_ARGUMENT, utcLedger_t * in_ledger, utcLedger_t * o_ledger,
                      ocrEdtDep_t in_borLabelEVT)
{
    Err_t err=0;
    while(!err){
        const unsigned int rankID = in_ledger->rankid;

#       ifdef UTEST_CHANNEL_VERBOSE
            PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" Userfcn_EndInit[%u]\n", in_edtType, GUIDA(in_thisEDT), rankID);
#       endif
        //DBG> print_utcLedger(OA_DEBUG_INVARS, in_ledger, "EndInit in_ledger");

        copy_utcLedger(OA_DEBUG_INVARS, in_ledger, o_ledger);

        GUID_ASSIGN_VALUE( o_ledger->neighborChannel, *(ocrGuid_t*)in_borLabelEVT.ptr);

        err = ocrDbDestroy(in_borLabelEVT.guid); IFEB; // Clean up used DBK.

        if( ! ocrGuidIsNull(o_ledger->temp_myLabelEvt) ){
            err = ocrEventDestroy(o_ledger->temp_myLabelEvt); IFEB;
            GUID_ASSIGN_VALUE(o_ledger->temp_myLabelEvt, NULL_GUID);
        }

        //DBG> print_utcLedger(OA_DEBUG_INVARS, o_ledger, "EndInit o_ledger");
        break;  // while(!err)
    }
    return err;
}

Err_t work_start(OA_DEBUG_ARGUMENT, utcLedger_t * io_ledger,
                 ocrGuid_t * io_destEDT, unsigned int in_destSlot)
{
    Err_t err=0;
    while(!err){
        const unsigned int rankID = io_ledger->rankid;

        unsigned int value=123;

        const char * left  = "Work_start_Left";
        const char * right = "Work_start_Right";
        const char * t = left;
        if(rankID == RANK_RIGHT) {
                t = right;
                value = 456;
        }

        print_utcLedger(OA_DEBUG_INVARS, io_ledger, t);

        PRINTF("TESTIO> %s has the value %u to send.\n", t, value);

        //===== Package the value
        ocrGuid_t envelopGuid  = NULL_GUID;
        unsigned int * envelopPtr = NULL;
        err = ocrDbCreate( &envelopGuid, (void**)&envelopPtr, sizeof(unsigned int), DB_PROP_NONE, NULL_HINT, NO_ALLOC); IFEB;

        *envelopPtr = value;
        err = ocrDbRelease( envelopGuid );

        //===== Connect the channel events
        err = ocrEventSatisfy( io_ledger->neighborChannel, envelopGuid); IFEB;  //This sends value's envelop to the neighbors

        //This sends the neighbor's value to our future self.
        err = ocrAddDependence( io_ledger->myChannel, *io_destEDT, in_destSlot, DB_MODE_RO); IFEB;

        break;  // while(!err)
    }
    return err;
}

Err_t work_stop(OA_DEBUG_ARGUMENT, utcLedger_t * io_ledger, ocrGuid_t * io_finalEdt,
                ocrEdtDep_t in_borChanlEVT)
{
    Err_t err=0;
    while(!err){
        const unsigned int rankID = io_ledger->rankid;

        const char * left  = "Work_stop_Left";
        const char * right = "Work_stop_Right";
        const char * t = left;
        const char * bor = right;
        if(rankID == RANK_RIGHT) {
                t = right;
                bor = left;
        }
        print_utcLedger(OA_DEBUG_INVARS, io_ledger, t);

        unsigned int value=0;
        value = *(unsigned int*)in_borChanlEVT.ptr;
        err = ocrDbDestroy(in_borChanlEVT.guid); IFEB; // Clean up used DBK.

        PRINTF("TESTIO> %s got the value %u from %s.\n", t, value, bor);

        GUID_ASSIGN_VALUE(*io_finalEdt, io_ledger->whenDone);
        break;  // while(!err)
    }
    return err;
}


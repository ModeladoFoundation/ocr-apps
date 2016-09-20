#define MAY_REFINE  0
#define WILL_REFINE 1
#define WONT_REFINE 2

typedef struct{
    u8 disposition;
    s8 neighborDisps[24];
} refineState_t;

typedef struct{
    u8 intent;
}intent_t;

ocrGuid_t intentLoopEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] );

ocrGuid_t haloNewChannelsSend( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

return NULL_GUID;

}

ocrGuid_t haloNewChannelsRcv( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

return NULL_GUID;

}

ocrGuid_t haloRefineSend( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
        ocrGuid_t *sendChannel = (ocrGuid_t *)paramv;
        //PRINTF("SENDING!\n");
        if( paramc == 1 )
            ocrEventSatisfy( *sendChannel, depv[0].guid );
        if( /*sendCount==*/ 4) {
            //create 4 new datablocks
            //send refinement message to each block.
        }

    return NULL_GUID;
}

ocrGuid_t haloRefineRcv( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    if( depc == 1 ){
        //PRINTF("INTENT RECEIVED.\n");
        ocrGuid_t newDBK;
        intent_t *tmp;
        ocrDbCreate( &newDBK, (void **)&tmp, sizeof(intent_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
        memcpy( tmp, depv[0].ptr, sizeof(intent_t) );
        return newDBK;
    }
    else if( depc == 4 ) {
        //do the things
    }
    return NULL_GUID;
}

ocrGuid_t refineEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    //
    //if I'm not refining, check if I can remain the same.

    //if(PRM_block->id == 0)
        //force a refine, as long as the ref lvl is not out of bounds.

    //we have a few cases we need to account for:
    //
    // 1:1
    //  - send refinement message to neighbor.
    //  - set up new channels in each direction. we will have a total of 4 * 6 non-local channels.
    //  -
    if( /*refine*/ 1 ){

        //refine.
        //communicate refinement halos.
        //create new channels to send to neighbors. if 1:1, send all new channels across existing channel.
    } else if( /*coarsen*/ 1 ) {
        //coarsen.
    }

    return NULL_GUID;
}

ocrGuid_t establishNewConnections( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    return NULL_GUID;
}

ocrGuid_t updateIntentEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    //PRINTF("updateIntent!\n");
    bool stable = true;

    block_t *PRM_block = (block_t *)paramv;
    refineState_t *rState = (refineState_t *)depv[0].ptr;

    u32 i;

    for( i = 1; i < depc; i++ )
    {
        intent_t *tmp = (intent_t *)depv[i].ptr;
        s32 difference = PRM_block->comms.neighborRefineLvls[i] - PRM_block->refLvl;

        if( !ocrGuidIsNull(depv[i].guid) ) rState->neighborDisps[i-1] = tmp->intent;
        else continue; //this neighbor is already stable from a previous communicate step.

        switch( difference ) {
            case  1: //I am less refined.
                break;
            case  0:
            case -1:

            break;
        }
        if( tmp->intent != MAY_REFINE ) continue;
        stable = false;
    }

    if( stable && rState->disposition != MAY_REFINE )
    {
        PRINTF("Block %ld All states stable.\n", PRM_block->id);
    } else {
        //decide if neighbors have decided to refine, and if I should refine, because of it.
        PRINTF("Block %ld isn't stable.\n", PRM_block->id);
        if( rState->disposition == MAY_REFINE ){
            u8 newDecision = WONT_REFINE; //placeholder for now.
            rState->disposition = newDecision;
        }

        ocrGuid_t commIntentGUID;
        ocrEdtCreate( &commIntentGUID, PRM_block->communicateIntentTML, EDT_PARAM_DEF, (u64 *)PRM_block,
                                EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NO_ALLOC );
        ocrDbRelease( depv[0].guid );
        ocrAddDependence( depv[0].guid, commIntentGUID, 0, DB_MODE_RW );
    }

    return NULL_GUID;

}

ocrGuid_t communicateIntentEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    //stay inside of communicate intent until "stable."
    //This means that if any neighbor has identified itself as a block that "could be" refining, all of its
    //neighbors must await its decision.

    block_t *PRM_block = (block_t *)paramv;
    refineState_t *rState = (refineState_t *)depv[0].ptr;
    //u64 pCount;


    u64 i;

    u64 updateDepc = 0;
    for( i = 0; i < 6; i++ )
    {
        s32 difference = PRM_block->comms.neighborRefineLvls[i] - PRM_block->refLvl;
        switch(difference) {
            case 1: //I am less refined.
                updateDepc += 4;
                break;
            case 0:
            case -1:
                updateDepc++;
                break;
            default:
                PRINTF("EDT %ld has a relation that is %ld. this is not allowed!\n", PRM_block->id, difference);
                ocrShutdown();
                return NULL_GUID;
                break;
        }
    }

    //if we made it to this part of the loop, we must have an updateDepc of at least 6.
    ASSERT( updateDepc >= 6 );

    ocrGuid_t updateIntentGUID;
    ocrEdtCreate( &updateIntentGUID, PRM_block->updateIntentTML, EDT_PARAM_DEF, (u64 *)PRM_block,
                            updateDepc + 1, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
    u32 pCount = sizeof(ocrGuid_t)/sizeof(u64);

    u64 iDep = 1;
    for( i = 0; i < 6; i++ )
    {
        s32 difference = PRM_block->comms.neighborRefineLvls[i] - PRM_block->refLvl;

        ASSERT( iDep <= updateDepc );

        ocrGuid_t rcvGUID;
        ocrGuid_t rcvOUT;
        //pCount = sizeof(ocrGuid_t)/sizeof(u64);
        switch( difference ) {
            case 1: //I am less refined.
                //set to receive from 4 receive channels (1 Edt, 4 rcv Evts).
                break;
            case 0: //I am at the same level.
            case -1: //I am more refined.
                //set to receive from 1 receive channel (1 Edt, 1 receive Evt).
                if( rState->neighborDisps[i] == MAY_REFINE)
                {
                    ocrEdtCreate( &rcvGUID, PRM_block->refineRcvTML, 0, NULL, 1, &PRM_block->comms.rcv[i*5], EDT_PROP_NONE,
                        NULL_HINT, &rcvOUT );
                    ocrAddDependence( rcvOUT, updateIntentGUID, iDep++, DB_MODE_RW );
                }
                else ocrAddDependence( NULL_GUID, updateIntentGUID, iDep++, DB_MODE_RW );
                break;
            default:
                PRINTF("EDT %ld has a relation that is %ld. This is not allowed!\n", PRM_block->id, difference );
                ocrShutdown();
                return NULL_GUID;
        }
    }


    for( i = 0; i < 6; i++ )
    {
        s32 difference = PRM_block->comms.neighborRefineLvls[i] - PRM_block->refLvl;

        ocrGuid_t sndGUID;
        switch(difference) {
            //set to send down 1 send channel( 1 Edt, 1 send Evt );
            case  1: //I am less refined.
                //set to send down the 4 send channels. (4 Edts, 1 send Evt).
                break;
            case  0:
            case -1:
                ocrEdtCreate( &sndGUID, PRM_block->refineSndTML, pCount, (u64 *)&PRM_block->comms.snd[i*5],
                                                                1, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
                ocrGuid_t intentDBK;
                intent_t *tmp;

                ocrDbCreate( &intentDBK, (void **)&tmp, sizeof(intent_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                tmp->intent = rState->disposition;

                ocrDbRelease( intentDBK );
                ocrAddDependence( intentDBK, sndGUID, 0, DB_MODE_RW );
                break;
        }
    }

    ocrDbRelease( depv[0].guid );
    ocrAddDependence( depv[0].guid, updateIntentGUID, 0, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t intentLoopEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    PRINTF("intentLoopEdt!\n");
    return NULL_GUID;
}

ocrGuid_t willRefineEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    //find out, based on the object state,
    block_t *PRM_block = (block_t *) paramv;
    refineState_t *rState = (refineState_t *)depv[0].ptr;
    u64 i;
    for( i = 0; i < 24; i++ ) rState->neighborDisps[i] = MAY_REFINE; //set all neighbor states to MAY_REFINE.
    if( 0/*I will refine*/ ){
        rState->disposition = WILL_REFINE;
    } else if( 1/*all neighbors are at a point where I won't need to refine, regardless*/ ){
        rState->disposition = WONT_REFINE;
    } else { //we have neighbors at a lower refinement level than us.
        rState->disposition = MAY_REFINE;
    }

    if( PRM_block->id == 13 ) rState->disposition = MAY_REFINE;

    return depv[0].guid;
}

ocrGuid_t refineControlEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    block_t *PRM_block = (block_t *) paramv;
    //PRINTF( "refine! %ld\n", PRM_block->id );

    ocrGuid_t intentGUID, rStateDBK;
    ocrGuid_t willRefineGUID, willRefineEVT;
    refineState_t *myState;

    ocrDbCreate( &rStateDBK, (void **)&myState, sizeof(refineState_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
    ocrEdtCreate( &willRefineGUID, PRM_block->willRefineTML, EDT_PARAM_DEF, (u64 *)PRM_block, EDT_PARAM_DEF, NULL,
                    EDT_PROP_NONE, NULL_HINT, &willRefineEVT ); //we care about the returnEVT of this Edt.
    ocrEdtCreate( &intentGUID, PRM_block->communicateIntentTML, EDT_PARAM_DEF, (u64 *)PRM_block, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, NULL_HINT, NULL );

    //ocrGuid_t intentLoopGUID, updateIntentGUID;

    //ocrEdt

    ocrAddDependence( willRefineEVT, intentGUID, 0, DB_MODE_RW );

    ocrAddDependence( rStateDBK, willRefineGUID, 0, DB_MODE_RW );

    return NULL_GUID;
}


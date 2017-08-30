#define MAY_COARSEN  0
#define WILL_COARSEN 1
#define WONT_COARSEN 2

#define MORE_REFINED -1
#define SAME_REFINED  0
#define LESS_REFINED  1

typedef struct{
    ocrGuid_t channels[4];
} bundle_t;

typedef struct{
    bool channelsNeeded[6];
    s64 disposition;
}catalog_t;

typedef struct{
    u32 cCount;
    u8  disposition;
    u8  prevDisposition;
    s8  neighborDisps[24]; //directions in order of: left, right, up, down, front, back.
} coarsenState_t;

typedef struct{
    u8 intent;
}intent_t;

ocrGuid_t blockEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] );

ocrGuid_t haloCoarsenSend( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
        ocrGuid_t *sendChannel = (ocrGuid_t *)paramv;
        ocrAssert( !ocrGuidIsNull( depv[0].guid ) );
        if( paramc == 1 )
            ocrEventSatisfy( *sendChannel, depv[0].guid );
        if( /*sendCount==*/ 4) {
            //create 4 new datablocks
            //send refinement message to each block.
        }

    return NULL_GUID;
}

ocrGuid_t haloCoarsenRcv( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    if( depc == 1 ){
        ocrGuid_t newDBK;
        intent_t *tmp;
        ocrDbCreate( &newDBK, (void **)&tmp, sizeof(intent_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
        //ocrAssert( !ocrGuidIsNull(depv[0].guid) );
        memcpy( tmp, depv[0].ptr, sizeof(intent_t) );
        //ocrDbDestroy( depv[0].guid );
        return newDBK;
    }
    else if( depc == 4 ) {
        //do the things
    }
    return NULL_GUID;
}

ocrGuid_t coarsenEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    //
    //if I'm not coarsening, check if I can remain the same.
    //block_t * PRM_block = (block_t *) paramv;
    block_t PRM_block;
    memcpy( &PRM_block, paramv, sizeof( block_t ) );
    ocrPrintf("COARSEN %ld\n", PRM_block.id);
    catalog_t *catalog = depv[0].ptr;
    ocrGuid_t blockDriverGUID;

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen =   2;
    params.EVENT_CHANNEL.nbSat  =   1;
    params.EVENT_CHANNEL.nbDeps =   1;


    //
    // 1:1
    //  - send refinement message to neighbor.
    //  - set up new channels in each direction. we will have a total of 4 * 6 non-local channels.
    //  -
    u64 i, j;
    bool newChannels = false;
    ocrGuid_t internalChannels[6][4];
    //s64 difference = PRM_block.my

    //connect all new channels, if needed.
    //
    u64 ddx = 1;
    if( catalog->disposition == WILL_REFINE ) PRM_block.refLvl++;
    for(i = 0; i < 6; i++) //posterity check for detecting whether new events were needed.
    {
        if( catalog->channelsNeeded[i] )
        {
            //ocrPrintf("%ld is setting new send/rcv channels in %ld direction.\n", PRM_block.id, i);
            bundle_t *tBundle = depv[ddx++].ptr;
            for( j = 0; j < 4; j++ )
            {
                u64 slot = j+1;
                memcpy( &PRM_block.comms.snd[(i*5)+slot], &tBundle->channels[j], sizeof(ocrGuid_t) ); // <--- can serialize this into one single memcpy, did this for increased readability.
            }
        }
    }

    /*
     * At this point, the new channels are connected to the parent's PRM_block. This is exactly what we want, for a couple of reasons:
     *  Reason 1:
     *      we have a way to keep track of the channels once this block joins (coarsens) again, and just in case the neighbors have not coarsened.
     *  Reason 2:
     *      once those neighbors are also coarsened, we can destroy our send/rcvs (based on ownership).
     */


    switch( catalog->disposition ) {

        case WILL_COARSEN:
            break;
        case WONT_COARSEN:
            for( i = 0; i < 6; i++ )
            {
                if( catalog->channelsNeeded[i] == WILL_REFINE ) newChannels = true;
            }
            if( newChannels ) ocrPrintf("%ld is not refining, but has new channels incoming.\n", PRM_block.id);
            else
            {
                //ocrPrintf("no new connections to be made; falling back to driverEdt\n");
                ocrEdtCreate( &blockDriverGUID, PRM_block.blockTML, EDT_PARAM_DEF, (u64 *)&PRM_block, 0,
                    NULL, EDT_PROP_NONE, NULL_HINT, NULL );
                    return NULL_GUID;
            }
            break;
        case MAY_COARSEN:
            ocrPrintf("block fell to refine while still in the MAY_REFINE state.\n");
            ocrAssert(0);
            break;
        default:
            break;
    }

    return NULL_GUID;
}

ocrGuid_t coarsenUpdateIntentEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    bool stable = true;

    block_t PRM_block;
    memcpy( &PRM_block, paramv, sizeof( block_t ) );
    coarsenState_t *rState = (coarsenState_t *)depv[0].ptr;

    u32 i;
    u32 nDir = 0;
    u32 subIdx = 0;

    for( i = 1; i < depc; i++ )
    {
        intent_t *tmp = (intent_t *)depv[i].ptr;
        s32 difference = PRM_block.comms.neighborRefineLvls[nDir] - PRM_block.refLvl;

        if( !ocrGuidIsNull(depv[i].guid) )
        {
            switch( difference )
            {
                case  1: //I am less refined.
                    rState->neighborDisps[(i-1) * 4] = tmp->intent;
                    nDir++;
                    break;
                case  0:
                case -1:
                    rState->neighborDisps[(i-1) * 4] = tmp->intent;
                    nDir++;
                    subIdx = 0;
                    break;
            }
        } else continue; //this neighbor is already stable from a previous communicate step.

        if( tmp->intent != MAY_REFINE )
        {
            subIdx++;
            if( subIdx == 4 )
            {
                subIdx = 0;
                nDir++;
            }
            continue;
        }
        stable = false;
    }

    if( stable && rState->disposition != MAY_REFINE )
    {
        //ocrPrintf("%ld stable and establishing new connections.\n", PRM_block.id);
        ocrGuid_t newConnectionsGUID, newConnectionsTML;
        u32 pCount = sizeof(block_t)/sizeof(u64);
        ocrEdtTemplateCreate( &newConnectionsTML, establishNewConnections, pCount, 1 );
        ocrEdtCreate( &newConnectionsGUID, newConnectionsTML, EDT_PARAM_DEF, (u64 *)&PRM_block, EDT_PARAM_DEF,
                                                                &depv[0].guid, EDT_PROP_NONE, NULL_HINT, NO_ALLOC );
    } else {
        //decide if neighbors have decided to refine, and if I should refine, because of it.
        //ocrPrintf("Block %ld isn't stable.\n", PRM_block.id);
        if( rState->disposition == MAY_REFINE ){
            u8 newDecision = WILL_REFINE; //placeholder for now.
            rState->prevDisposition = rState->disposition;
            rState->disposition = newDecision;
        } else
        {
            rState->prevDisposition = rState->disposition;
        }
        ocrGuid_t commIntentGUID;
        ocrEdtCreate( &commIntentGUID, PRM_block.communicateIntentTML, EDT_PARAM_DEF, (u64 *)&PRM_block,
                                EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NO_ALLOC );

        rState->cCount++;
        ocrDbRelease( depv[0].guid );
        ocrAddDependence( depv[0].guid, commIntentGUID, 0, DB_MODE_RW );
    }

    return NULL_GUID;
}

ocrGuid_t coarsenCommunicateIntentEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    //stay inside of communicate intent until "stable."
    //This means that if any neighbor has identified itself as a block that "could be" refining, all of its
    //neighbors must await its decision.

    block_t PRM_block;
    memcpy( &PRM_block, paramv, sizeof( block_t ) );
    coarsenState_t *rState = (coarsenState_t *)depv[0].ptr;

    u64 i;

    u64 updateDepc = 0;
    for( i = 0; i < 6; i++ ) //decide how many neighbors we may need to potential hear from.
    {
        s32 difference = PRM_block.comms.neighborRefineLvls[i] - PRM_block.refLvl;
        switch(difference) {
            case 1: //I am less refined.
                updateDepc += 4;
                break;
            case 0:
            case -1:
                updateDepc++;
                break;
            default:
                ocrPrintf("EDT %ld has a relation that is %ld. this is not allowed!\n", PRM_block.id, difference);
                ocrAssert(0);
                return NULL_GUID;
                break;
        }
    }

    ocrAssert( updateDepc >= 6 );

    ocrGuid_t updateIntentGUID, updateIntentTML;
    u32 pSize = ( sizeof( block_t ) / sizeof( u64 ) ) + 1;

    ocrEdtTemplateCreate( &updateIntentTML, coarsenUpdateIntentEdt,
    ocrEdtCreate( &updateIntentGUID, PRM_block.updateIntentTML, EDT_PARAM_DEF, (u64 *)&PRM_block,
                            updateDepc + 1, NULL, EDT_PROP_NONE, NULL_HINT, NULL );

    u64 iDep = 1;
    for( i = 0; i < 6; i++ )
    {
        s32 difference = PRM_block.comms.neighborRefineLvls[i] - PRM_block.refLvl;

        ocrAssert( iDep <= updateDepc );

        ocrGuid_t rcvGUID;
        ocrGuid_t rcvOUT;
        switch( difference ) {
            case 1: //I am less refined.
                //set to receive from 4 receive channels (1 Edt, 4 rcv Evts).
                break;
            case 0: //I am at the same level.
            case -1: //I am more refined.
                //set to receive from 1 receive channel (1 Edt, 1 receive Evt).
                if( rState->neighborDisps[i*4] == MAY_COARSEN )
                {
                    //if( rState->cCount > 0 ) ocrPrintf("%ld's %ld neighbor may refine.\n", PRM_block.id, i );
                    ocrEdtCreate( &rcvGUID, PRM_block.refineRcvTML, 0, NULL, 1, NULL, EDT_PROP_NONE,
                            NULL_HINT, &rcvOUT );
                    ocrAddDependence( rcvOUT, updateIntentGUID, iDep++, DB_MODE_RW );
                    ocrAddDependence( PRM_block.comms.rcv[i*5], rcvGUID, 0, DB_MODE_RO );
                }
                else if( rState->prevDisposition == MAY_COARSEN && rState->cCount > 0 )
                {
                   ocrEdtCreate( &rcvGUID, PRM_block.refineRcvTML, 0, NULL, 1, NULL, EDT_PROP_NONE,
                        NULL_HINT, &rcvOUT );
                    ocrAddDependence( rcvOUT, updateIntentGUID, iDep++, DB_MODE_RW );
                    ocrAddDependence( PRM_block.comms.rcv[i*5], rcvGUID, 0, DB_MODE_RO );
                }
                else ocrAddDependence( NULL_GUID, updateIntentGUID, iDep++, DB_MODE_RW );
                break;
            default:
                ocrPrintf("EDT %ld has a relation that is %ld. This is not allowed!\n", PRM_block.id, difference );
                ocrAssert(0);
                return NULL_GUID;
        }
    }


    u32 pCount = sizeof(ocrGuid_t)/sizeof(u64);
    for( i = 0; i < 6; i++ )
    {
        s32 difference = PRM_block.comms.neighborRefineLvls[i] - PRM_block.refLvl;

        ocrGuid_t sndGUID;
        switch(difference) {
            case  1: //I am less refined.
                break;
            case  0:
            case -1:
                if( rState->prevDisposition == MAY_COARSEN )
                {
                    ocrEdtCreate( &sndGUID, PRM_block.refineSndTML, pCount, (u64 *)&PRM_block.comms.snd[i*5],
                                                                    1, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
                    ocrGuid_t intentDBK;
                    intent_t *tmp;

                    ocrDbCreate( &intentDBK, (void **)&tmp, sizeof(intent_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                    tmp->intent = rState->disposition;

                    ocrDbRelease( intentDBK );
                    ocrAddDependence( intentDBK, sndGUID, 0, DB_MODE_RW );
                }
                else if( rState->cCount > 0 && rState->neighborDisps[i*4] == MAY_COARSEN )
                {
                    ocrEdtCreate( &sndGUID, PRM_block.refineSndTML, pCount, (u64 *)&PRM_block.comms.snd[i*5],
                                                                    1, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
                    ocrGuid_t intentDBK;
                    intent_t *tmp;

                    ocrDbCreate( &intentDBK, (void **)&tmp, sizeof(intent_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                    tmp->intent = rState->disposition;

                    ocrDbRelease( intentDBK );
                    ocrAddDependence( intentDBK, sndGUID, 0, DB_MODE_RW );
                }

                break;
        }
    }
    ocrDbRelease( depv[0].guid );
    ocrAddDependence( depv[0].guid, updateIntentGUID, 0, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t willCoarsenEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    block_t PRM_block;
    memcpy( &PRM_block, paramv, sizeof(block_t) );
    coarsenState_t *rState = (coarsenState_t *)depv[0].ptr;
    u64 i;
    for( i = 0; i < 24; i++ ) rState->neighborDisps[i] = MAY_COARSEN; //set all neighbor states to MAY_REFINE.
    if( 0/*I will refine*/ ){
        rState->disposition = WILL_COARSEN;
    } else if( 1/*all neighbors are at a point where I won't need to refine, regardless*/ ){
        rState->disposition = WONT_COARSEN;
    } else { //we have neighbors at a lower refinement level than us.
        rState->disposition = MAY_COARSEN;
    }

    if( PRM_block.id == 13 ) rState->disposition = MAY_COARSEN;
    rState->prevDisposition = MAY_COARSEN;
    rState->cCount = 0;
    return depv[0].guid;
}

ocrGuid_t coarsenControlEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    block_t PRM_block;
    memcpy( &PRM_block, paramv, sizeof(block_t) );

    ocrGuid_t intentGUID, rStateDBK;
    ocrGuid_t willCoarsenGUID, willCoarsenEVT, willCoarsenTML;
    ocrGuid_t intentGUID, intentTML;
    coarsenState_t *myState;

    u32 pSize = ( sizeof(block_t) / sizeof(u64) ) + 1;

    ocrEdtTemplateCreate( &willCoarsenTML, willCoarsenEdt, pSize, 1 );
    ocrEdtTemplateCreate( &intentTML, coarsenUpdateIntentEdt, pSize, 1 );

    ocrDbCreate( &rStateDBK, (void **)&myState, sizeof(coarsenState_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
    ocrEdtCreate( &willCoarsenGUID, willCoarsenTML, EDT_PARAM_DEF, (u64 *)&PRM_block, EDT_PARAM_DEF, NULL,
                    EDT_PROP_NONE, NULL_HINT, &willCoarsenEVT ); //we care about the returnEVT of this Edt.
    ocrEdtCreate( &intentGUID, , EDT_PARAM_DEF, (u64 *)&PRM_block, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, NULL_HINT, NULL );

    ocrAddDependence( willCoarsenEVT, intentGUID, 0, DB_MODE_RW );

    ocrAddDependence( rStateDBK, willCoarsenGUID, 0, DB_MODE_RW );

    return NULL_GUID;
}

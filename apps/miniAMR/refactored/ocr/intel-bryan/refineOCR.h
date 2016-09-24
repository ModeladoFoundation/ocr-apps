#define MAY_REFINE  0
#define WILL_REFINE 1
#define WONT_REFINE 2

#define MORE_REFINED -1
#define SAME_REFINED  0
#define LESS_REFINED  1

typedef struct{
    ocrGuid_t channels[4];
} bundle_t;

typedef struct{
    u32 cCount;
    u8  disposition;
    u8  prevDisposition;
    s8  neighborDisps[24];
} refineState_t;

typedef struct{
    u8 intent;
}intent_t;

ocrGuid_t intentLoopEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] );

ocrGuid_t haloNewChannelsSend( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrGuid_t *sendChannel = (ocrGuid_t *)paramv;
    //PRINTF("SENDING CHANNELS\n");
    ocrEventSatisfy( *sendChannel, depv[0].guid );

return NULL_GUID;
}

ocrGuid_t haloNewChannelsRcv( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
        //PRINTF("CHANNELS RECEIVED.\n");
        ocrGuid_t newDBK;
        ocrGuid_t *tmp;
        ocrDbCreate( &newDBK, (void **)&tmp, sizeof( ocrGuid_t ) * 4, DB_PROP_SINGLE_ASSIGNMENT,
                                                                                    NULL_HINT, NO_ALLOC );
        memcpy( tmp, depv[0].ptr, sizeof( ocrGuid_t ) * 4 );
        return newDBK;
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
        ocrGuid_t newDBK;
        intent_t *tmp;
        ocrDbCreate( &newDBK, (void **)&tmp, sizeof(intent_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
        memcpy( tmp, depv[0].ptr, sizeof(intent_t) );
        ocrDbDestroy( depv[0].guid );
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
    block_t * PRM_block = (block_t *) paramv;
    PRINTF("REFINE %ld\n", PRM_block->id);
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


    block_t *PRM_block = (block_t *)paramv;
    refineState_t *rState = depv[0].ptr;

    /*ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen =   2;
    params.EVENT_CHANNEL.nbSat =    1;
    params.EVENT_CHANNEL.nbDeps =   1;*/


    u64 i;
    bool channelsNeeded[6];
    ocrGuid_t channelListDBK;

    ocrDbCreate( &channelListDBK, (void **)&channelsNeeded, sizeof(bool) * 6, DB_PROP_SINGLE_ASSIGNMENT,
                            NULL_HINT, NO_ALLOC );

    u32 sets = 0;
    for( i = 0; i < 6; i++ ) channelsNeeded[i] = false;
    switch( rState->disposition )
    {
        case WILL_REFINE:
            //PRINTF("%ld will refine.\n", PRM_block->id);
            for( i = 0; i < 6; i++ )
            {
                s64 difference = PRM_block->comms.neighborRefineLvls[i] - PRM_block->refLvl;
                ASSERT( difference >= -1 );
                ASSERT( difference <=  1 );
                switch(difference)
                {
                    case LESS_REFINED://no new channels to create.
                        break;
                    case SAME_REFINED://need to create more channels.
                    case MORE_REFINED://create new channels and send them along.
                        sets++;
                        channelsNeeded[i] = true;
                        break;
                }
            }
        case WONT_REFINE: //check to see which neighbors, if any, we must set up channels with.

            for( i = 0; i < 6; i++ )
            {
                s64 difference = PRM_block->comms.neighborRefineLvls[i] - PRM_block->refLvl;
                ASSERT( difference >= -1 );
                ASSERT( difference <=  1 );
                if( rState->neighborDisps[i*4] == WILL_REFINE )
                {
                    //PRINTF("%ld's %ld neighbor refining.\n", PRM_block->id, i);
                    PRM_block->comms.neighborRefineLvls[i]++; //increase the neighbor's refinement level.
                    switch(difference)
                    {
                        case  LESS_REFINED:
                            /*Finer neighbor attempting to refine while current block wishes to stay the same!*/
                            ASSERT(0);
                            break;
                        case  SAME_REFINED:
                            channelsNeeded[i] = true;
                            sets++;
                            break;
                        case MORE_REFINED:
                            break;
                    }
                }
            }
            break;
        default:
            PRINTF("this block is broken.\n");
            ASSERT(0);
            return NULL_GUID;
    }


    ocrGuid_t refineGUID;


    ocrEdtCreate( &refineGUID, PRM_block->refineTML, EDT_PARAM_DEF, (u64 *)PRM_block, sets+1, NULL,
            EDT_PROP_NONE, NULL_HINT, NULL );


    //PRINTF("%ld block, %ld sets\n", PRM_block->id, sets );
    u32 pCount = sizeof( ocrGuid_t )/sizeof( u64 );

    ocrGuid_t channelRcvTML, channelSndTML;
    ocrEdtTemplateCreate( &channelRcvTML, haloNewChannelsRcv, 0, 1 );
    ocrEdtTemplateCreate( &channelSndTML, haloNewChannelsSend, pCount, 1 );

    if( sets )
    {
        //PRINTF("%ld must create %ld sets of channels\n", PRM_block->id, sets);
        u32 idx = 1;
        for( i = 0; i < 6; i++ )
        {

            if( channelsNeeded[i] == true )
            {
                //PRINTF("%ld receiving in %ld direction\n", PRM_block->id, i);
                ocrGuid_t rcvGUID, rcvOUT;

                ocrEdtCreate( &rcvGUID, channelRcvTML, EDT_PARAM_DEF, NULL,  EDT_PARAM_DEF, &PRM_block->comms.rcv[i*5],
                                    EDT_PROP_NONE, NULL_HINT, &rcvOUT );

                ocrAddDependence( rcvOUT, refineGUID, idx++, DB_MODE_RO );

            }
        }

        for( i = 0; i < 6; i++ )
        {
            if( channelsNeeded[i] == true )
            {
               // if(PRM_block->id == 13) PRINTF("sending in %ld direction\n", i);
                ocrGuid_t sndGUID;
                //bundle_t *newRcvChannels;
                ocrGuid_t newRcvChannels[4];
                ocrGuid_t bundleGUID;
                //ocrDbCreate( &bundleGUID, (void **)&newRcvChannels, sizeof(bundle_t), DB_PROP_SINGLE_ASSIGNMENT,
                //                NULL_HINT, NO_ALLOC );
                ocrDbCreate( &bundleGUID, (void **)&newRcvChannels, sizeof(ocrGuid_t) * 4, DB_PROP_SINGLE_ASSIGNMENT,
                                NULL_HINT, NO_ALLOC );


                u64 j;
                for( j = 0; j < 4; j++ )
                {

                   // ocrEventCreateParams( &newRcvChannels->channels[i], OCR_EVENT_CHANNEL_T, false, &params );
                   //ocrEventCreateParams( &newRcvChannels[i], OCR_EVENT_CHANNEL_T, false, &params );
                }

                ocrDbRelease( bundleGUID );
                ocrEdtCreate( &sndGUID, channelSndTML, EDT_PARAM_DEF, (u64 *)&PRM_block->comms.snd[i*5],
                                EDT_PARAM_DEF, &bundleGUID, EDT_PROP_NONE, NULL_HINT, NULL );

                //ocrEventSatisfy( PRM_block->comms.snd[i*5], bundleGUID );
            }

        }
    }
    ocrDbRelease( channelListDBK );
    ocrAddDependence( channelListDBK, refineGUID, 0, DB_MODE_RO );

    //PRINTF("RefineEDT dep[0] satisfied.\n");

    return NULL_GUID;
}

ocrGuid_t updateIntentEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    bool stable = true;

    block_t *PRM_block = (block_t *)paramv;
    refineState_t *rState = (refineState_t *)depv[0].ptr;


    u32 i;
    u32 nDir = 0;
    u32 subIdx = 0;

    //if( rState->cCount > 0 ) PRINTF("%ld next iteration of updateIntent.\n", PRM_block->id );

    for( i = 1; i < depc; i++ )
    {
        intent_t *tmp = (intent_t *)depv[i].ptr;
        s32 difference = PRM_block->comms.neighborRefineLvls[nDir] - PRM_block->refLvl;

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
        //if(rState->cCount > 0)PRINTF("%ld stable and establishing new connections.\n", PRM_block->id);
        ocrGuid_t newConnectionsGUID, newConnectionsTML;
        u32 pCount = sizeof(block_t)/sizeof(u64);
        ocrEdtTemplateCreate( &newConnectionsTML, establishNewConnections, pCount, 1 );
        ocrEdtCreate( &newConnectionsGUID, newConnectionsTML, EDT_PARAM_DEF, (u64 *)PRM_block, EDT_PARAM_DEF,
                            &depv[0].guid, EDT_PROP_NONE, NULL_HINT, NO_ALLOC );
    } else {
        //decide if neighbors have decided to refine, and if I should refine, because of it.
        //PRINTF("Block %ld isn't stable.\n", PRM_block->id);
        if( rState->disposition == MAY_REFINE ){
            u8 newDecision = WILL_REFINE; //placeholder for now.
            rState->prevDisposition = rState->disposition;
            rState->disposition = newDecision;
        } else
        {
            rState->prevDisposition = rState->disposition;
        }
        ocrGuid_t commIntentGUID;
        ocrEdtCreate( &commIntentGUID, PRM_block->communicateIntentTML, EDT_PARAM_DEF, (u64 *)PRM_block,
                                EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NO_ALLOC );

        rState->cCount++;
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

    //if( rState->cCount > 0 ) PRINTF("%ld communicating new intent.\n", PRM_block->id );

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
                if( rState->neighborDisps[i*4] == MAY_REFINE )
                {
                    //if( rState->cCount > 0 ) PRINTF("%ld's %ld neighbor may refine.\n", PRM_block->id, i );
                    ocrEdtCreate( &rcvGUID, PRM_block->refineRcvTML, 0, NULL, 1, &PRM_block->comms.rcv[i*5], EDT_PROP_NONE,
                        NULL_HINT, &rcvOUT );
                    ocrAddDependence( rcvOUT, updateIntentGUID, iDep++, DB_MODE_RW );
                }
                else if( rState->prevDisposition == MAY_REFINE && rState->cCount > 0 )
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
                if( rState->prevDisposition == MAY_REFINE )
                {
                    ocrEdtCreate( &sndGUID, PRM_block->refineSndTML, pCount, (u64 *)&PRM_block->comms.snd[i*5],
                                                                    1, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
                    ocrGuid_t intentDBK;
                    intent_t *tmp;

                    ocrDbCreate( &intentDBK, (void **)&tmp, sizeof(intent_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                    tmp->intent = rState->disposition;

                    ocrDbRelease( intentDBK );
                    ocrAddDependence( intentDBK, sndGUID, 0, DB_MODE_RW );
                }
                else if( rState->cCount > 0 && rState->neighborDisps[i*4] == MAY_REFINE )
                {
     //               if( rState->prevDisposition == rState->disposition )
     //                   PRINTF("%ld sending intent again.\n", PRM_block->id );
                    ocrEdtCreate( &sndGUID, PRM_block->refineSndTML, pCount, (u64 *)&PRM_block->comms.snd[i*5],
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
    rState->prevDisposition = MAY_REFINE;
    rState->cCount = 0;

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


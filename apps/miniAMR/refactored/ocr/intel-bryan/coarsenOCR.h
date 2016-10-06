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
} refineState_t;

typedef struct{
    u8 intent;
}intent_t;

ocrGuid_t blockEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] );

ocrGuid_t haloNewChannelsSend( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrGuid_t *sendChannel = (ocrGuid_t *)paramv;
    ocrEventSatisfy( *sendChannel, depv[0].guid );

return NULL_GUID;
}

ocrGuid_t haloNewChannelsRcv( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
        ocrGuid_t newDBK;
        bundle_t *tmp;
        ocrDbCreate( &newDBK, (void **)&tmp, sizeof( bundle_t ), DB_PROP_SINGLE_ASSIGNMENT, NULL_HINT, NO_ALLOC );
        memcpy( tmp, depv[0].ptr, sizeof( bundle_t ) );

        ASSERT( !ocrGuidIsNull( tmp->channels[0] ) );
        ASSERT( !ocrGuidIsNull( tmp->channels[1] ) );
        ASSERT( !ocrGuidIsNull( tmp->channels[2] ) );
        ASSERT( !ocrGuidIsNull( tmp->channels[3] ) );//this shall guarantee that all channels have been initialized.

        return newDBK;
}

ocrGuid_t haloRefineSend( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
        ocrGuid_t *sendChannel = (ocrGuid_t *)paramv;
        ASSERT( !ocrGuidIsNull( depv[0].guid ) );
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
        //ASSERT( !ocrGuidIsNull(depv[0].guid) );
        memcpy( tmp, depv[0].ptr, sizeof(intent_t) );
        //ocrDbDestroy( depv[0].guid );
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
    //block_t * PRM_block = (block_t *) paramv;
    block_t PRM_block;
    memcpy( &PRM_block, paramv, sizeof( block_t ) );
   // PRINTF("REFINE %ld\n", PRM_block.id);
    catalog_t *catalog = depv[0].ptr;
    ocrGuid_t blockDriverGUID;

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen =   2;
    params.EVENT_CHANNEL.nbSat  =   1;
    params.EVENT_CHANNEL.nbDeps =   1;

    //u32 dCnt = depc-1;

    //we have a few cases we need to account for:
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
            //PRINTF("%ld is setting new send/rcv channels in %ld direction.\n", PRM_block.id, i);
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

        case WILL_REFINE:

            for( i = 0; i < 6; i++ )
            {
              if( catalog->channelsNeeded[i] ) //this is for the internal channels.
              {
                PRINTF("%ld needs new channels in %ld direction\n", PRM_block.id, i );
                for( j = 0; j < 4; j++ )
                {
                    ocrEventCreateParams( &internalChannels[i][j], OCR_EVENT_CHANNEL_T, false, &params );
                }
              }
            }

            PRINTF("%ld is refining!\n", PRM_block.id);

            for( i = 0; i < 8; i++ ) //create new child block.
            {
                //ocrGuid_t childGUID;
                block_t childBlock = PRM_block;
                childBlock.parent = PRM_block.id;
                childBlock.id = PRM_block.id + PRM_block.rootId + i;
                childBlock.refLvl++;


                //set channels.
                switch(i){
                    case 0:
                        {
                        u64 idx = 0;
                        //set left direction
                        //send
                        memcpy( &childBlock.comms.snd[idx], &PRM_block.comms.snd[(idx)+1], sizeof(ocrGuid_t) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx], &PRM_block.comms.rcv[(idx)+1], sizeof(ocrGuid_t) );
                        idx++;

                        /*---------------------------------------------------------------------------------------*/

                        //set right direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][0], sizeof(ocrGuid_t) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx], &internalChannels[idx-1][0], sizeof(ocrGuid_t) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5) + 1], sizeof(ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5) + 1], sizeof( ocrGuid_t ) );
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][0], sizeof(ocrGuid_t) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx-1][0], sizeof(ocrGuid_t) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][0], sizeof(ocrGuid_t) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx+1][0], sizeof(ocrGuid_t) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock.comms.snd[(idx)*5], &PRM_block.comms.snd[(idx*5)+1], sizeof( ocrGuid_t ) ); //the channels had to be created in this direction, use the channels sent by this bundle.
                        //rcv
                        memcpy( &childBlock.comms.rcv[(idx)*5], &PRM_block.comms.rcv[(idx*5)+1], sizeof( ocrGuid_t ) );
                        }
                        PRINTF("block for %ld.%ld.%ld\n", childBlock.rootId, childBlock.parent, childBlock.id );
                        break;

                    case 1:
                        {
                        u64 idx = 0;

                        //set left direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][0], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx+1][0], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set right direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+1], sizeof( ocrGuid_t ) );
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx-1][1], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx+1][1], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        }
                        PRINTF("block for %ld.%ld.%ld\n", childBlock.rootId, childBlock.parent, childBlock.id );
                        break;

                    case 2:
                        {
                        u64 idx = 0;

                        //set left direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        idx++;

                        //set right direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx-1][1], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx-1][2], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+1], sizeof( ocrGuid_t ) );
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][0], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx-1][0], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        }
                        PRINTF("block for %ld.%ld.%ld\n", childBlock.rootId, childBlock.parent, childBlock.id );
                        break;
                    case 3:
                        {
                        u64 idx = 0;

                        //set left direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx+1][1], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set right direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.snd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx-1][3], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx-1][1], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        }
                        PRINTF("block for %ld.%ld.%ld\n", childBlock.rootId, childBlock.parent, childBlock.id );
                        break;
                    case 4:
                        {
                        u64 idx = 0;

                        //set left direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        idx++;

                        //set right direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx-1][2], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][0], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx+1][0], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+1], sizeof( ocrGuid_t ) );
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx+1][2], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        }
                        PRINTF("block for %ld.%ld.%ld\n", childBlock.rootId, childBlock.parent, childBlock.id );
                        break;
                    case 5:
                        {
                        u64 idx = 0;

                        //set left direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx+1][2], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set right direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][1], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx-1][1], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+2], sizeof( ocrGuid_t ) );
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx+1][3], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        }
                        PRINTF("block for %ld.%ld.%ld\n", childBlock.rootId, childBlock.parent, childBlock.id );
                        break;
                    case 6:
                        {
                        u64 idx = 0;

                        //set left direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        idx++;

                        //set right direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx-1][3], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx+1][2], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+3], sizeof( ocrGuid_t ) );
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][2], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx-1][2], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        }
                        PRINTF("block for %ld.%ld.%ld\n", childBlock.rootId, childBlock.parent, childBlock.id );
                        break;
                    case 7:
                        {
                        u64 idx = 0;

                        //set left direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx+1][3], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set right direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        idx++;

                        //set up direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx+1][3], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        idx++;

                        //set down direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        idx++;

                        //set front direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &PRM_block.comms.snd[(idx*5)+4], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &PRM_block.comms.rcv[(idx*5)+4], sizeof( ocrGuid_t ) );
                        idx++;

                        //set back direction
                        //send
                        memcpy( &childBlock.comms.snd[idx*5], &internalChannels[idx][3], sizeof( ocrGuid_t ) );
                        //rcv
                        memcpy( &childBlock.comms.rcv[idx*5], &internalChannels[idx-1][3], sizeof( ocrGuid_t ) );
                        childBlock.comms.neighborRefineLvls[idx] = childBlock.refLvl;
                        }
                        PRINTF("block for %ld.%ld.%ld\n", childBlock.rootId, childBlock.parent, childBlock.id );
                        break;
                }

            ocrEdtCreate( &blockDriverGUID, PRM_block.blockTML, EDT_PARAM_DEF, (u64 *)&childBlock, 0, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
            }
            break;
        case WONT_REFINE:
            for( i = 0; i < 6; i++ )
            {
                if( catalog->channelsNeeded[i] == WILL_REFINE ) newChannels = true;
            }
            if( newChannels ) PRINTF("%ld is not refining, but has new channels incoming.\n", PRM_block.id);
            else
            {
                //PRINTF("no new connections to be made; falling back to driverEdt\n");
                ocrEdtCreate( &blockDriverGUID, PRM_block.blockTML, EDT_PARAM_DEF, (u64 *)&PRM_block, 0,
                    NULL, EDT_PROP_NONE, NULL_HINT, NULL );
                    return NULL_GUID;
            }
            break;
        case MAY_REFINE:
            PRINTF("block fell to refine while still in the MAY_REFINE state.\n");
            ASSERT(0);
            break;
        default:
            break;
    }


    return NULL_GUID;
}

ocrGuid_t establishNewConnections( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    block_t PRM_block;
    memcpy( &PRM_block, paramv, sizeof(block_t) );
    refineState_t *rState = depv[0].ptr;

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen =   2;
    params.EVENT_CHANNEL.nbSat =    1;
    params.EVENT_CHANNEL.nbDeps =   1;

    ocrGuid_t channelListDBK;

    u64 i;
    catalog_t *catalog;

    ocrDbCreate( &channelListDBK, (void **)&catalog, sizeof(catalog_t), DB_PROP_SINGLE_ASSIGNMENT, NULL_HINT, NO_ALLOC );

    u32 sets = 0;
    for( i = 0; i < 6; i++ ) catalog->channelsNeeded[i] = false;
    switch( rState->disposition )
    {
        case WILL_REFINE:
            for( i = 0; i < 6; i++ )
            {
                s64 difference = PRM_block.comms.neighborRefineLvls[i] - PRM_block.refLvl;
                ASSERT( difference >= -1 );
                ASSERT( difference <=  1 );
                switch(difference)
                {
                    case LESS_REFINED://no new channels to create.
                        break;
                    case SAME_REFINED://need to create more channels.
                    case MORE_REFINED://create new channels and send them along.
                        sets++;
                        catalog->channelsNeeded[i] = true;
                        break;
                }
            }
        case WONT_REFINE: //check to see which neighbors, if any, we must set up channels with.

            for( i = 0; i < 6; i++ )
            {
                s64 difference = PRM_block.comms.neighborRefineLvls[i] - PRM_block.refLvl;
                ASSERT( difference >= -1 );
                ASSERT( difference <=  1 );
                if( rState->neighborDisps[i*4] == WILL_REFINE )
                {
                    PRM_block.comms.neighborRefineLvls[i]++; //update the refinement level of your neighbor.
                    switch(difference)
                    {
                        case  LESS_REFINED:
                            /*Finer neighbor attempting to refine while current block wishes to stay the same!*/
                            ASSERT(0);
                            break;
                        case  SAME_REFINED:
                            catalog->channelsNeeded[i] = true;
                            sets++;
                            break;
                        case MORE_REFINED: //no new channels to create.
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


    u32 pCount = sizeof( ocrGuid_t )/sizeof( u64 );

    ocrGuid_t channelRcvTML, channelSndTML;
    ocrEdtTemplateCreate( &channelRcvTML, haloNewChannelsRcv, 0, 1 );
    ocrEdtTemplateCreate( &channelSndTML, haloNewChannelsSend, pCount, 1 );

    if( sets )
    {
        u32 idx = 1;

        for( i = 0; i < 6; i++ )
        {
            if( catalog->channelsNeeded[i] == true )
            {
                ocrGuid_t sndGUID;
                bundle_t *newRcvChannels;
                ocrGuid_t bundleGUID;
                ocrDbCreate( &bundleGUID, (void **)&newRcvChannels, sizeof(bundle_t), DB_PROP_SINGLE_ASSIGNMENT,
                                NULL_HINT, NO_ALLOC );


                u64 j;
                for( j = 0; j < 4; j++ )
                {
                   ocrEventCreateParams( &newRcvChannels->channels[j], OCR_EVENT_CHANNEL_T, false, &params );
                   memcpy( &PRM_block.comms.rcv[(i*5) + (j + 1)], &newRcvChannels->channels[j], sizeof(ocrGuid_t) );
                   ASSERT( ocrGuidIsEq( PRM_block.comms.rcv[i*5 + (j + 1)], newRcvChannels->channels[j] ) );
                   //set the listening guids.
                }

                ocrDbRelease( bundleGUID );
                ocrEdtCreate( &sndGUID, channelSndTML, EDT_PARAM_DEF, (u64 *)&PRM_block.comms.snd[i*5],
                                EDT_PARAM_DEF, &bundleGUID, EDT_PROP_NONE, NULL_HINT, NULL );

            }

        }

        ocrEdtCreate( &refineGUID, PRM_block.refineTML, EDT_PARAM_DEF, (u64 *)&PRM_block, sets+1, NULL,
            EDT_PROP_NONE, NULL_HINT, NULL );

        for( i = 0; i < 6; i++ )
        {

            if( catalog->channelsNeeded[i] == true )
            {
                ocrGuid_t rcvGUID, rcvOUT;

                ocrEdtCreate( &rcvGUID, channelRcvTML, EDT_PARAM_DEF, NULL,  EDT_PARAM_DEF, NULL,
                                    EDT_PROP_NONE, NULL_HINT, &rcvOUT );

                ocrAddDependence( rcvOUT, refineGUID, idx++, DB_MODE_RO );
                ocrAddDependence( PRM_block.comms.rcv[i*5], rcvGUID, 0, DB_MODE_RO );
            }
        }

    } else {
        ocrEdtCreate( &refineGUID, PRM_block.refineTML, EDT_PARAM_DEF, (u64 *)&PRM_block, sets+1, NULL,
            EDT_PROP_NONE, NULL_HINT, NULL );
    }
    catalog->disposition = rState->disposition;
    ocrDbRelease( channelListDBK );
    ocrAddDependence( channelListDBK, refineGUID, 0, DB_MODE_RO );

    return NULL_GUID;
}

ocrGuid_t updateIntentEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    bool stable = true;

    block_t PRM_block;
    memcpy( &PRM_block, paramv, sizeof( block_t ) );
    refineState_t *rState = (refineState_t *)depv[0].ptr;


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
        //PRINTF("%ld stable and establishing new connections.\n", PRM_block.id);
        ocrGuid_t newConnectionsGUID, newConnectionsTML;
        u32 pCount = sizeof(block_t)/sizeof(u64);
        ocrEdtTemplateCreate( &newConnectionsTML, establishNewConnections, pCount, 1 );
        ocrEdtCreate( &newConnectionsGUID, newConnectionsTML, EDT_PARAM_DEF, (u64 *)&PRM_block, EDT_PARAM_DEF,
                            &depv[0].guid, EDT_PROP_NONE, NULL_HINT, NO_ALLOC );
    } else {
        //decide if neighbors have decided to refine, and if I should refine, because of it.
        //PRINTF("Block %ld isn't stable.\n", PRM_block.id);
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

ocrGuid_t communicateIntentEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    //stay inside of communicate intent until "stable."
    //This means that if any neighbor has identified itself as a block that "could be" refining, all of its
    //neighbors must await its decision.

    block_t PRM_block;
    memcpy( &PRM_block, paramv, sizeof( block_t ) );
    refineState_t *rState = (refineState_t *)depv[0].ptr;

    u64 i;

    u64 updateDepc = 0;
    for( i = 0; i < 6; i++ )
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
                PRINTF("EDT %ld has a relation that is %ld. this is not allowed!\n", PRM_block.id, difference);
                ocrShutdown();
                return NULL_GUID;
                break;
        }
    }

    ASSERT( updateDepc >= 6 );

    ocrGuid_t updateIntentGUID;
    ocrEdtCreate( &updateIntentGUID, PRM_block.updateIntentTML, EDT_PARAM_DEF, (u64 *)&PRM_block,
                            updateDepc + 1, NULL, EDT_PROP_NONE, NULL_HINT, NULL );

    u64 iDep = 1;
    for( i = 0; i < 6; i++ )
    {
        s32 difference = PRM_block.comms.neighborRefineLvls[i] - PRM_block.refLvl;

        ASSERT( iDep <= updateDepc );

        ocrGuid_t rcvGUID;
        ocrGuid_t rcvOUT;
        switch( difference ) {
            case 1: //I am less refined.
                //set to receive from 4 receive channels (1 Edt, 4 rcv Evts).
                break;
            case 0: //I am at the same level.
            case -1: //I am more refined.
                //set to receive from 1 receive channel (1 Edt, 1 receive Evt).
                if( rState->neighborDisps[i*4] == MAY_REFINE )
                {
                    //if( rState->cCount > 0 ) PRINTF("%ld's %ld neighbor may refine.\n", PRM_block.id, i );
                    ocrEdtCreate( &rcvGUID, PRM_block.refineRcvTML, 0, NULL, 1, NULL, EDT_PROP_NONE,
                            NULL_HINT, &rcvOUT );
                    ocrAddDependence( rcvOUT, updateIntentGUID, iDep++, DB_MODE_RW );
                    ocrAddDependence( PRM_block.comms.rcv[i*5], rcvGUID, 0, DB_MODE_RO );
                }
                else if( rState->prevDisposition == MAY_REFINE && rState->cCount > 0 )
                {
                   ocrEdtCreate( &rcvGUID, PRM_block.refineRcvTML, 0, NULL, 1, NULL, EDT_PROP_NONE,
                        NULL_HINT, &rcvOUT );
                    ocrAddDependence( rcvOUT, updateIntentGUID, iDep++, DB_MODE_RW );
                    ocrAddDependence( PRM_block.comms.rcv[i*5], rcvGUID, 0, DB_MODE_RO );
                }
                else ocrAddDependence( NULL_GUID, updateIntentGUID, iDep++, DB_MODE_RW );
                break;
            default:
                PRINTF("EDT %ld has a relation that is %ld. This is not allowed!\n", PRM_block.id, difference );
                ocrShutdown();
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
                if( rState->prevDisposition == MAY_REFINE )
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
                else if( rState->cCount > 0 && rState->neighborDisps[i*4] == MAY_REFINE )
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
    refineState_t *rState = (refineState_t *)depv[0].ptr;
    u64 i;
    for( i = 0; i < 24; i++ ) rState->neighborDisps[i] = MAY_COARSEN; //set all neighbor states to MAY_REFINE.
    if( 0/*I will refine*/ ){
        rState->disposition = WILL_COARSEN;
    } else if( 1/*all neighbors are at a point where I won't need to refine, regardless*/ ){
        rState->disposition = WONT_COARSEN;
    } else { //we have neighbors at a lower refinement level than us.
        rState->disposition = MAY_COARSEN;
    }

    if( PRM_block.id == 13 ) rState->disposition = MAY_REFINE;
    rState->prevDisposition = MAY_REFINE;
    rState->cCount = 0;
    return depv[0].guid;
}

ocrGuid_t coarsenControlEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    block_t PRM_block;
    memcpy( &PRM_block, paramv, sizeof(block_t) );

    ocrGuid_t intentGUID, rStateDBK;
    ocrGuid_t willRefineGUID, willRefineEVT;
    refineState_t *myState;

    ocrDbCreate( &rStateDBK, (void **)&myState, sizeof(refineState_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
    ocrEdtCreate( &willRefineGUID, PRM_block.willRefineTML, EDT_PARAM_DEF, (u64 *)&PRM_block, EDT_PARAM_DEF, NULL,
                    EDT_PROP_NONE, NULL_HINT, &willRefineEVT ); //we care about the returnEVT of this Edt.
    ocrEdtCreate( &intentGUID, PRM_block.communicateIntentTML, EDT_PARAM_DEF, (u64 *)&PRM_block, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, NULL_HINT, NULL );

    ocrAddDependence( willRefineEVT, intentGUID, 0, DB_MODE_RW );

    ocrAddDependence( rStateDBK, willRefineGUID, 0, DB_MODE_RW );

    return NULL_GUID;
}


#define WILL_COARSEN    2
#define CAN_COARSEN     1
#define CANT_COARSEN    0

typedef struct{
    u32 canCoarsen;
    u64 parentId;
    u64 rootId;         //now we know who our siblings are for this generation.
}coarsenInfo_t;

typedef struct{
    u64 disposition;
    s64 neighborDisps;
}coarsenState_t;

ocrGuid_t haloRestartRcv( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t * depv )
{
    return NULL_GUID;
}

ocrGuid_t coalesceEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t * depv )
{
    block_t * PRM_block = depv[depc-1].ptr;
    block_t * childBlock[8];
    //block_t * childBlock = depv[0].ptr;
    u64 i;

    if( ocrGuidIsNull( depv[1].guid ) )
    {
        if( !ocrGuidIsNull( PRM_block->parentEVT ) )
        {
            ocrEventSatisfy( PRM_block->parentEVT, NULL_GUID );
        }
        else
        {
        ocrEdtTemplateDestroy( PRM_block->blockTML);
        ocrEdtTemplateDestroy( PRM_block->refineTML );
        ocrEdtTemplateDestroy( PRM_block->refineRcvTML );
        ocrEdtTemplateDestroy( PRM_block->refineCtrlTML);
        ocrEdtTemplateDestroy( PRM_block->communicateIntentTML );
        ocrEdtTemplateDestroy( PRM_block->willRefineTML );
        ocrEdtTemplateDestroy( PRM_block->updateIntentTML );
        ocrEdtTemplateDestroy( PRM_block->stencilTML );
        ocrEdtTemplateDestroy( PRM_block->haloRcvTML);
        }

        ocrDbDestroy( depv[depc-1].guid );
        return NULL_GUID;
    }

    for( i = 0; i < 8; i++ ) childBlock[i] = depv[i].ptr;
    //PRINTF("%ld is coarsening!\n", PRM_block->id);

    //set neighborRefinement Levels.
    //
    PRM_block->neighborRefineLvls[0] = childBlock[0]->neighborRefineLvls[0];
    PRM_block->neighborRefineLvls[2] = childBlock[0]->neighborRefineLvls[2];
    PRM_block->neighborRefineLvls[5] = childBlock[0]->neighborRefineLvls[5];

    PRM_block->neighborRefineLvls[1] = childBlock[7]->neighborRefineLvls[1];
    PRM_block->neighborRefineLvls[3] = childBlock[7]->neighborRefineLvls[3];
    PRM_block->neighborRefineLvls[4] = childBlock[7]->neighborRefineLvls[4];


    PRM_block->timestep = childBlock[0]->timestep;
    memcpy( PRM_block->objects, childBlock[0]->objects, sizeof( object ) * childBlock[0]->numObjects );

    ocrGuid_t driverGUID;

    ocrEdtCreate( &driverGUID, PRM_block->blockTML, EDT_PARAM_DEF, NULL, 7, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
    for( i = 0; i < 6; i++ )
    {
        u64 base = i*5;

        if( PRM_block->neighborRefineLvls[i] > PRM_block->refLvl )
        {
            u64 j;
            for( j = 0; j < 4; j++ )
            {
                    ocrEventSatisfy( PRM_block->rSnd[base+(j+1)], NULL_GUID );
            }
        }
        else
        {
            ocrEventSatisfy( PRM_block->rSnd[base], NULL_GUID );
        }
    }

    ocrGuid_t rcvTML;
    ocrEdtTemplateCreate( &rcvTML, haloRestartRcv, EDT_PARAM_UNK, EDT_PARAM_UNK );

    for( i = 0; i < 6; i++ )
    {
        u64 base = i*5;
        ocrGuid_t rcvOUT, rcvGUID;

        if( PRM_block->neighborRefineLvls[i] > PRM_block->refLvl )
        {
            ocrEdtCreate( &rcvGUID, rcvTML, 0, NULL, 4, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
            ocrAddDependence( rcvOUT, driverGUID, i+1, DB_MODE_RW );
            u64 j;
            for( j = 0; j < 4; j++ )
            {
                 ocrAddDependence( PRM_block->rRcv[base+(j+1)], rcvGUID, j, DB_MODE_RW );
            }
        }
        else
        {
            ocrEdtCreate( &rcvGUID, rcvTML, 0, NULL, 1, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
            ocrAddDependence( rcvOUT, driverGUID, i+1, DB_MODE_RW );
            ocrAddDependence( PRM_block->rRcv[base], rcvGUID, 0, DB_MODE_RW );
        }
    }
    ocrEdtTemplateDestroy( rcvTML );

    ocrDbRelease( depv[depc-1].guid );
    ocrAddDependence( depv[depc-1].guid, driverGUID, 0, DB_MODE_RW );

    for( i = 0; i < 8; i++ ) ocrDbDestroy( depv[i].guid );

    return NULL_GUID;
}

ocrGuid_t concensusRcvEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t * depv )
{
    if( depc == 1 )
    {
        //PRINTF("recvd 1\n");
        return depv[0].guid;
    }
    else if( depc == 4 )
    {
        u64 i;
        coarsenInfo_t * inf;
        ocrGuid_t infDBK;
        //PRINTF("rcvd 4\n");

        ocrDbCreate( &infDBK, (void **)&inf, sizeof(coarsenInfo_t) * 4, DB_PROP_NONE, NULL_HINT, NO_ALLOC );
        for( i = 0; i < 4; i++ )
        {
            memcpy( &inf[i], depv[i].ptr, sizeof(coarsenInfo_t) );
            ocrDbDestroy( depv[i].guid );
        }

        return infDBK;
    }
    else PRINTF("wtf?\n");
    return NULL_GUID;
}


ocrGuid_t finalConsensus( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t * depv )
{
    block_t * PRM_block = depv[depc-1].ptr;
    coarsenInfo_t * myInfo = depv[0].ptr;

    u64 i;

    for( i = 0; i < 6; i++ )
    {
        coarsenInfo_t * tmp = depv[i+1].ptr;
        if( tmp->parentId == PRM_block->parent )
        {
            if( tmp->rootId == PRM_block->rootId )
            {
                if( tmp->canCoarsen == CANT_COARSEN )
                    myInfo->canCoarsen = CANT_COARSEN;
            }
        }
    }

    if( myInfo->canCoarsen == CAN_COARSEN )
    {
        ocrGuid_t coalesceEVT;
        //PRINTF("%ld child of %ld root %ld will coarsen.\n", PRM_block->id, PRM_block->parent, PRM_block->rootId);
        memcpy( &coalesceEVT, &PRM_block->parentEVT, sizeof(ocrGuid_t) );

        for( i = 0; i < 6; i++ )
        {
            if( ocrGuidIsNull( depv[i+1].guid ) ) continue;
            coarsenInfo_t * tmp = depv[i+1].ptr;

            if( PRM_block->neighborRefineLvls[i] > PRM_block->refLvl )
            {
                bool isCoarsening = true;
                u64 j;
                for( j = 0; j < 4; j++ )
                {
                    if( tmp[j].canCoarsen == CAN_COARSEN ) continue;

                    isCoarsening = false;
                    break;
                }
                if( isCoarsening )
                {
                    //PRINTF("%ld neighbor in %ld direction is coarsening into parent %ld.\n", PRM_block->id, i, tmp->parentId );

                    PRM_block->neighborRefineLvls[i]--;
                    ASSERT( PRM_block->neighborRefineLvls[i] >= 0 );
                }
            }
            else
            {
                if( tmp->canCoarsen == CAN_COARSEN )
                {
                    //PRINTF("%ld neighbor in %ld direction is coarsening into parent %ld.\n", PRM_block->id, i, tmp->parentId );
                    PRM_block->neighborRefineLvls[i]--;
                    ASSERT( PRM_block->neighborRefineLvls[i] >= 0 );
                }
            }
            ocrDbDestroy( depv[i+1].guid );
        }

        ocrDbRelease( depv[depc-1].guid );
        ocrEventSatisfy( coalesceEVT, depv[depc-1].guid );
    }
    else
    {
        for( i = 0; i < 6; i++ )
        {
            if( ocrGuidIsNull( depv[i+1].guid ) ) continue;

            coarsenInfo_t * tmp = depv[i+1].ptr;

            if( PRM_block->neighborRefineLvls[i] > PRM_block->refLvl ) //if I have a finer neighbor...
            {
                bool isCoarsening = true;
                u64 j;
                for(j = 0; j < 4; j++)
                {
                    if( tmp[j].canCoarsen == CAN_COARSEN ) continue;

                    isCoarsening = false;
                    break;
                }
                if( isCoarsening )
                {
                    //PRINTF( "%ld neighbor in %ld direction is coarsening into parent %ld.\n", PRM_block->id, i, tmp->parentId );

                    PRM_block->neighborRefineLvls[i]--;
                    ASSERT( PRM_block->neighborRefineLvls[i] >= 0 );
                }
                //TODO sever ties, if need be.
            }
            else
            {
                if( tmp->canCoarsen == CAN_COARSEN )
                {
                    //PRINTF( "%ld neighbor in %ld direction is coarsening into parent %ld.\n", PRM_block->id, i, tmp->parentId );
                    PRM_block->neighborRefineLvls[i]--;
                    ASSERT( PRM_block->neighborRefineLvls[i] >= 0 );
                }
            }
            ocrDbDestroy( depv[i+1].guid );
        }
        ocrGuid_t driverGUID;
        ocrEdtCreate( &driverGUID, PRM_block->blockTML, EDT_PARAM_DEF, NULL, 7, NULL, EDT_PROP_NONE, NULL_HINT, NULL );

        for( i = 0; i < 6; i++ )
        {
            u64 base = i*5;

            if( PRM_block->neighborRefineLvls[i] > PRM_block->refLvl )
            {
                u64 j;
                for( j = 0; j < 4; j++ )
                {
                    ocrEventSatisfy( PRM_block->rSnd[base+(j+1)], NULL_GUID );
                }
            }
            else
            {

                ocrEventSatisfy( PRM_block->rSnd[base], NULL_GUID );
            }
        }

        ocrGuid_t rcvTML;
        ocrEdtTemplateCreate( &rcvTML, haloRestartRcv, EDT_PARAM_UNK, EDT_PARAM_UNK );

        for( i = 0; i < 6; i++ )
        {
            u64 base = i*5;
            ocrGuid_t rcvOUT, rcvGUID;

            if( PRM_block->neighborRefineLvls[i] > PRM_block->refLvl )
            {
                ocrEdtCreate( &rcvGUID, rcvTML, 0, NULL, 4, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
                ocrAddDependence( rcvOUT, driverGUID, i+1, DB_MODE_RW );
                u64 j;
                for( j = 0; j < 4; j++ )
                {
                     ocrAddDependence( PRM_block->rRcv[base+(j+1)], rcvGUID, j, DB_MODE_RW );
                }
            }
            else
            {
                ocrEdtCreate( &rcvGUID, rcvTML, 0, NULL, 1, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
                ocrAddDependence( rcvOUT, driverGUID, i+1, DB_MODE_RW );
                ocrAddDependence( PRM_block->rRcv[base], rcvGUID, 0, DB_MODE_RW );
            }
        }

        ocrEdtTemplateDestroy( rcvTML );

        ocrDbRelease( depv[depc-1].guid );
        ocrAddDependence( depv[depc-1].guid, driverGUID, 0, DB_MODE_RW );
    }
    ocrDbDestroy(depv[0].guid);
    return NULL_GUID;
}

ocrGuid_t confirmConsensus( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t * depv )
{
    block_t * PRM_block = depv[depc-1].ptr;
    coarsenInfo_t * myInfo = depv[0].ptr;

    u64 i; //if I can't coarsen, I still need to see if any of my finer neighbors will coarsen.

    for( i = 0; i < 6; i++ )
    {
        if( ocrGuidIsNull( depv[i+1].guid ) ) continue;
        coarsenInfo_t * tmp = depv[i+1].ptr;
        if( tmp->parentId == PRM_block->parent )
        {
            if( tmp->rootId == PRM_block->rootId )
            {
                if( tmp->canCoarsen == CANT_COARSEN )
                    myInfo->canCoarsen = CANT_COARSEN;
            }
        }
        ocrDbDestroy( depv[i+1].guid );
    }


    ocrGuid_t confConGUID, confConTML;
    ocrEdtTemplateCreate( &confConTML, finalConsensus, 0, 8 );

    ocrEdtCreate( &confConGUID, confConTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
    ocrEdtTemplateDestroy( confConTML );


    ocrGuid_t rcvTML;
    ocrEdtTemplateCreate( &rcvTML, concensusRcvEdt, EDT_PARAM_UNK, EDT_PARAM_UNK );

    for( i = 0; i < 6; i++ ) //send confirmation that I can/can't coarsen.
    {
        u64 j;
        if( PRM_block->neighborRefineLvls[i] <= PRM_block->refLvl )
        {
                coarsenInfo_t * tmp;
                ocrGuid_t tmpDBK;
                ocrDbCreate( &tmpDBK, (void **)&tmp, sizeof( coarsenInfo_t ), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                memcpy( tmp, myInfo, sizeof( coarsenInfo_t ) );
                ocrEventSatisfy( PRM_block->rSnd[i*5], tmpDBK );
        }
        else
        {
            for( j = 0; j < 4; j++ )
            {
                ocrGuid_t tmpDBK;
                coarsenInfo_t * tmp;
                ocrDbCreate( &tmpDBK, (void **)&tmp, sizeof( coarsenInfo_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                memcpy( tmp, myInfo, sizeof( coarsenInfo_t ) );
                ocrEventSatisfy( PRM_block->rSnd[(i*5)+(j+1)], tmpDBK );
            }
        }
    }

    for( i = 0; i < 6; i++ )
    {
        ocrGuid_t rcvGUID, rcvOUT;
        if( PRM_block->neighborRefineLvls[i] <= PRM_block->refLvl )
        {
                ocrEdtCreate( &rcvGUID, rcvTML, 0, NULL, 1, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
                ocrAddDependence( rcvOUT, confConGUID, i+1, DB_MODE_RW );
                ocrAddDependence( PRM_block->rRcv[i*5], rcvGUID, 0, DB_MODE_RW );

        }
        else
        {
            ocrEdtCreate( &rcvGUID, rcvTML, 0, NULL, 4, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
            ocrAddDependence( rcvOUT, confConGUID, i+1, DB_MODE_RW );
            u64 base = i*5, offs;
            for( offs = 0; offs < 4; offs++ )
            {
                ocrAddDependence( PRM_block->rRcv[base+(offs+1)], rcvGUID, offs, DB_MODE_RW );
            }
        }
    }
    ocrEdtTemplateDestroy( rcvTML );

    ocrDbRelease( depv[0].guid );
    ocrDbRelease( depv[depc-1].guid );
    ocrAddDependence( depv[0].guid, confConGUID, 0, DB_MODE_RW );
    ocrAddDependence( depv[depc-1].guid, confConGUID, 7, DB_MODE_RW );

    return NULL_GUID;

}

ocrGuid_t doubleCheckConsensus( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t * depv )
{
    block_t * PRM_block = depv[depc-1].ptr;
    coarsenInfo_t * myInfo = depv[0].ptr;

    u64 i; //if I can't coarsen, I still need to see if any of my finer neighbors will coarsen.

    for( i = 0; i < 6; i++ )
    {
        if( ocrGuidIsNull( depv[i+1].guid ) ) continue;
        coarsenInfo_t * tmp = depv[i+1].ptr;
        if( tmp->parentId == PRM_block->parent )
        {
            if( tmp->rootId == PRM_block->rootId )
            {
                if( tmp->canCoarsen == CANT_COARSEN )
                    myInfo->canCoarsen = CANT_COARSEN;
            }
        }
        ocrDbDestroy( depv[i+1].guid );
    }


    ocrGuid_t confConGUID, confConTML;
    ocrEdtTemplateCreate( &confConTML, confirmConsensus, 0, 8 );

    ocrEdtCreate( &confConGUID, confConTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
    ocrEdtTemplateDestroy( confConTML );


    ocrGuid_t rcvTML;
    ocrEdtTemplateCreate( &rcvTML, concensusRcvEdt, EDT_PARAM_UNK, EDT_PARAM_UNK );

    for( i = 0; i < 6; i++ ) //send confirmation that I can/can't coarsen.
    {
        u64 j;
        if( PRM_block->neighborRefineLvls[i] <= PRM_block->refLvl )
        {
                coarsenInfo_t * tmp;
                ocrGuid_t tmpDBK;
                ocrDbCreate( &tmpDBK, (void **)&tmp, sizeof( coarsenInfo_t ), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                memcpy( tmp, myInfo, sizeof( coarsenInfo_t ) );
                ocrEventSatisfy( PRM_block->rSnd[i*5], tmpDBK );
        }
        else
        {
            for( j = 0; j < 4; j++ )
            {
                ocrGuid_t tmpDBK;
                coarsenInfo_t * tmp;
                ocrDbCreate( &tmpDBK, (void **)&tmp, sizeof( coarsenInfo_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                memcpy( tmp, myInfo, sizeof( coarsenInfo_t ) );
                ocrEventSatisfy( PRM_block->rSnd[(i*5)+(j+1)], tmpDBK );
            }
        }
    }

    for( i = 0; i < 6; i++ )
    {
        ocrGuid_t rcvGUID, rcvOUT;
        if( PRM_block->neighborRefineLvls[i] <= PRM_block->refLvl )
        {
                ocrEdtCreate( &rcvGUID, rcvTML, 0, NULL, 1, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
                ocrAddDependence( rcvOUT, confConGUID, i+1, DB_MODE_RW );
                ocrAddDependence( PRM_block->rRcv[i*5], rcvGUID, 0, DB_MODE_RW );
        }
        else
        {
            ocrEdtCreate( &rcvGUID, rcvTML, 0, NULL, 4, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
            ocrAddDependence( rcvOUT, confConGUID, i+1, DB_MODE_RW );
            u64 base = i*5, offs;
            for( offs = 0; offs < 4; offs++ )
            {
                ocrAddDependence( PRM_block->rRcv[base+(offs+1)], rcvGUID, offs, DB_MODE_RW );
            }
        }
    }
    ocrEdtTemplateDestroy( rcvTML );

    ocrDbRelease( depv[0].guid );
    ocrDbRelease( depv[depc-1].guid );
    ocrAddDependence( depv[0].guid, confConGUID, 0, DB_MODE_RW );
    ocrAddDependence( depv[depc-1].guid, confConGUID, 7, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t checkConsensus( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t * depv)
{
    block_t * PRM_block = depv[depc-1].ptr;
    coarsenInfo_t * myInfo = depv[0].ptr;

    u64 i; //if I can't coarsen, I still need to see if any of my finer neighbors will coarsen.

    for( i = 0; i < 6; i++ )
    {
        if( ocrGuidIsNull( depv[i+1].guid ) ) continue;
        coarsenInfo_t * tmp = depv[i+1].ptr;
        if( tmp->parentId == PRM_block->parent )
        {
            if( tmp->rootId == PRM_block->rootId )
            {
                if( tmp->canCoarsen == CANT_COARSEN )
                    myInfo->canCoarsen = CANT_COARSEN;
            }
        }
        ocrDbDestroy( depv[i+1].guid );
    }


    ocrGuid_t confConGUID, confConTML;
    ocrEdtTemplateCreate( &confConTML, doubleCheckConsensus, 0, 8 );

    ocrEdtCreate( &confConGUID, confConTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
    ocrEdtTemplateDestroy( confConTML );


    ocrGuid_t rcvTML;
    ocrEdtTemplateCreate( &rcvTML, concensusRcvEdt, EDT_PARAM_UNK, EDT_PARAM_UNK );

    for( i = 0; i < 6; i++ ) //send confirmation that I can/can't coarsen.
    {
        u64 j;
        if( PRM_block->neighborRefineLvls[i] <= PRM_block->refLvl )
        {
                coarsenInfo_t * tmp;
                ocrGuid_t tmpDBK;
                ocrDbCreate( &tmpDBK, (void **)&tmp, sizeof( coarsenInfo_t ), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                memcpy( tmp, myInfo, sizeof( coarsenInfo_t ) );
                ocrEventSatisfy( PRM_block->rSnd[i*5], tmpDBK );
        }
        else
        {
            for( j = 0; j < 4; j++ )
            {
                ocrGuid_t tmpDBK;
                coarsenInfo_t * tmp;
                ocrDbCreate( &tmpDBK, (void **)&tmp, sizeof( coarsenInfo_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                memcpy( tmp, myInfo, sizeof( coarsenInfo_t ) );
                ocrEventSatisfy( PRM_block->rSnd[(i*5)+(j+1)], tmpDBK );
            }
        }
    }

    for( i = 0; i < 6; i++ )
    {
        ocrGuid_t rcvGUID, rcvOUT;
        if( PRM_block->neighborRefineLvls[i] <= PRM_block->refLvl )
        {
                ocrEdtCreate( &rcvGUID, rcvTML, 0, NULL, 1, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
                ocrAddDependence( rcvOUT, confConGUID, i+1, DB_MODE_RW );
                ocrAddDependence( PRM_block->rRcv[i*5], rcvGUID, 0, DB_MODE_RW );

        }
        else
        {
            ocrEdtCreate( &rcvGUID, rcvTML, 0, NULL, 4, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
            ocrAddDependence( rcvOUT, confConGUID, i+1, DB_MODE_RW );
            u64 base = i*5, offs;
            for( offs = 0; offs < 4; offs++ )
            {
                ocrAddDependence( PRM_block->rRcv[base+(offs+1)], rcvGUID, offs, DB_MODE_RW );
            }
        }
    }
    ocrEdtTemplateDestroy( rcvTML );

    ocrDbRelease( depv[0].guid );
    ocrDbRelease( depv[depc-1].guid );
    ocrAddDependence( depv[0].guid, confConGUID, 0, DB_MODE_RW );
    ocrAddDependence( depv[depc-1].guid, confConGUID, 7, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t coarsenIntent( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t * depv )
{
    coarsenInfo_t * intent = depv[0].ptr;
    block_t * PRM_block = depv[1].ptr;

    ocrGuid_t checkConcensusGUID, checkConsensusTML;
    ocrGuid_t rcvTML;

   // PRINTF("%ld coarsenIntent!\n", PRM_block->id);

    ocrEdtTemplateCreate( &checkConsensusTML, checkConsensus, 0, 8 );
    ocrEdtCreate( &checkConcensusGUID, checkConsensusTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NO_ALLOC );
    ocrEdtTemplateDestroy( checkConsensusTML );
    ocrEdtTemplateCreate( &rcvTML, concensusRcvEdt, EDT_PARAM_UNK, EDT_PARAM_UNK );

    u32 i;
    for( i = 0; i < 6; i++ ) //set up the sends.
    {
        u64 j;
        if( PRM_block->neighborRefineLvls[i] <= PRM_block->refLvl )
        {
                coarsenInfo_t * tmp;
                ocrGuid_t tmpDBK;
                ocrDbCreate( &tmpDBK, (void **)&tmp, sizeof( coarsenInfo_t ), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                memcpy( tmp, depv[0].ptr, sizeof( coarsenInfo_t ) );
                ocrEventSatisfy( PRM_block->rSnd[i*5], tmpDBK );
        }
        else
        {
            for( j = 0; j < 4; j++ ) //we do not need the same guard as the single send case above.
            {
                ocrGuid_t tmpDBK;
                coarsenInfo_t * tmp;
                ocrDbCreate( &tmpDBK, (void **)&tmp, sizeof( coarsenInfo_t ), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                memcpy( tmp, intent, sizeof( coarsenInfo_t ) );
                ocrEventSatisfy( PRM_block->rSnd[(i*5)+(j+1)], tmpDBK );
            }
        }
    }

    for( i = 0; i < 6; i++ ) //set up the receives.
    {
        ocrGuid_t rcvGUID, rcvOUT;
        if( PRM_block->neighborRefineLvls[i] <= PRM_block->refLvl )
        {
            //PRINTF("%d difference\n", difference);
                ocrEdtCreate( &rcvGUID, rcvTML, 0, NULL, 1, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
                ocrAddDependence( rcvOUT, checkConcensusGUID, i+1, DB_MODE_RW );
                ocrAddDependence( PRM_block->rRcv[i*5], rcvGUID, 0, DB_MODE_RW );
        }
        else
        {
           // PRINTF("%d difference\n", difference);
            ocrEdtCreate( &rcvGUID, rcvTML, 0, NULL, 4, NULL, EDT_PROP_NONE, NULL_HINT, &rcvOUT );
            ocrAddDependence( rcvOUT, checkConcensusGUID, i+1, DB_MODE_RW );
            u64 base = i*5, offs;
            for( offs = 0; offs < 4; offs++ )
                ocrAddDependence( PRM_block->rRcv[base+(offs+1)], rcvGUID, offs, DB_MODE_RW );
        }
    }
    ocrEdtTemplateDestroy( rcvTML );


    ocrDbRelease( depv[0].guid );
    ocrDbRelease( depv[1].guid );
    ocrAddDependence( depv[0].guid, checkConcensusGUID, 0, DB_MODE_RW );
    ocrAddDependence( depv[1].guid, checkConcensusGUID, 7, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t canCoarsenEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t * depv )
{

    block_t * PRM_block = depv[0].ptr;

    coarsenInfo_t * intent;
    ocrGuid_t intentDBK;

    ocrDbCreate( &intentDBK, (void **)&intent, sizeof( coarsenInfo_t ), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

    intent->canCoarsen = CAN_COARSEN;
    intent->parentId = PRM_block->parent;
    intent->rootId = PRM_block->rootId;

    if( PRM_block->refLvl == 0 ) intent->canCoarsen = CANT_COARSEN;

    u64 i;

    for( i = 0; i < 6; i++ )
    {
        if( PRM_block->neighborRefineLvls[i] <= PRM_block->refLvl ) continue;

        intent->canCoarsen = CANT_COARSEN;
        break;
    }

    if( intent->canCoarsen != CANT_COARSEN )
    {
        #ifdef OBJECT_DRIVEN
        bool decision = false;
        for( i = 0; i < PRM_block->numObjects; i++ )
        {
            if(!decision)
            {
                double xMin = PRM_block->objects[i].cen[0]-(PRM_block->objects[i].size[0]/2.0f), xMax = PRM_block->objects[i].cen[0] + (PRM_block->objects[i].size[0]/2.0f);
                double yMin = PRM_block->objects[i].cen[1]-(PRM_block->objects[i].size[1]/2.0f), yMax = PRM_block->objects[i].cen[1] + (PRM_block->objects[i].size[1]/2.0f);
                double zMin = PRM_block->objects[i].cen[2]-(PRM_block->objects[i].size[2]/2.0f), zMax = PRM_block->objects[i].cen[2] + (PRM_block->objects[i].size[2]/2.0f);

                bool dX, dY, dZ;

                dX = ((xMin >= PRM_block->pos.x - PRM_block->halfSize.x) && (xMin < PRM_block->pos.x + PRM_block->halfSize.x)) ||
                                ((xMax >=PRM_block->pos.x - PRM_block->halfSize.x) && (xMax < PRM_block->pos.x + PRM_block->halfSize.x) );
                dY = ((yMin >= PRM_block->pos.y - PRM_block->halfSize.y) && (yMin < PRM_block->pos.y + PRM_block->halfSize.y)) ||
                                    ((yMax >= PRM_block->pos.y - PRM_block->halfSize.y) && (yMax < PRM_block->pos.y + PRM_block->halfSize.y) );
                dZ = ((zMin >= PRM_block->pos.z - PRM_block->halfSize.z) && (zMin < PRM_block->pos.z + PRM_block->halfSize.z)) ||
                                    ((zMax >= PRM_block->pos.z - PRM_block->halfSize.z) && (zMax < PRM_block->pos.z + PRM_block->halfSize.z) );

                decision = dX && dY && dZ;
                continue;
            }
            break;
        }
        if (decision) intent->canCoarsen = CANT_COARSEN;
        #endif
    }

    return intentDBK;
}

ocrGuid_t coarsenControlEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ocrGuid_t canCoarsenGUID, canCoarsenTML, canCoarsenOUT;
    ocrGuid_t coarsenIntentGUID, coarsenIntentTML;

    ocrEdtTemplateCreate( &canCoarsenTML, canCoarsenEdt, 0, 1 );
    ocrEdtTemplateCreate( &coarsenIntentTML, coarsenIntent, 0, 2 );

    ocrEdtCreate( &canCoarsenGUID, canCoarsenTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, &canCoarsenOUT );
    ocrEdtCreate( &coarsenIntentGUID, coarsenIntentTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
    ocrEdtTemplateDestroy( canCoarsenTML );
    ocrEdtTemplateDestroy( coarsenIntentTML );

    ocrAddDependence( canCoarsenOUT, coarsenIntentGUID, 0, DB_MODE_RW );

    ocrDbRelease(depv[0].guid);
    ocrAddDependence( depv[0].guid, coarsenIntentGUID, 1, DB_MODE_RW );

    ocrAddDependence( depv[0].guid, canCoarsenGUID, 0, DB_MODE_RW );

    return NULL_GUID;
}

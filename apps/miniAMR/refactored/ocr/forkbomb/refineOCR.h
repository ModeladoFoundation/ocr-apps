#define MAY_REFINE  0
#define WILL_REFINE 1
#define WONT_REFINE 2

#define MORE_REFINED -1
#define SAME_REFINED  0
#define LESS_REFINED  1

#include<stdlib.h>
#include<time.h>

ocrGuid_t blockEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] );

ocrGuid_t refineControlEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    block_t PRM_block;
    memcpy( &PRM_block, paramv, sizeof(block_t) );
    u32 pCount = (sizeof( block_t ) / sizeof( u64 )) +1;

    srand(time(NULL) * PRM_block.id);
    ocrGuid_t blockDriverGUID, blockTML;
    ocrEdtTemplateCreate( &blockTML, blockEdt, pCount, 1 );

    s64 willRef = rand();

    if( (willRef % 20 == 0) && PRM_block.refLvl < PRM_block.maxRefLvl )
    {
        u64 i;
        block_t childBlock = PRM_block;
        childBlock.refLvl++;
        if( PRM_block.refLvl == 0 ) PRM_block.parent = PRM_block.id;
        childBlock.id = PRM_block.id * 8 * childBlock.refLvl;

        for( i = 0; i < 8; i++ )
        {
            childBlock.id += i;
            ocrEdtCreate( &blockDriverGUID, blockTML, EDT_PARAM_DEF, (u64 *)&childBlock, 1, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
            ocrAddDependence( NULL_GUID, blockDriverGUID, 0, DB_MODE_RW );
        }

    }
    else if( ( willRef % 20 == 0) && PRM_block.refLvl == PRM_block.maxRefLvl )
    {
        PRINTF("%ld cannot refine more!\n", PRM_block.id);
    }
    else
    {
        ocrEdtCreate( &blockDriverGUID, blockTML, EDT_PARAM_DEF, (u64 *)&PRM_block, 1, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
        ocrAddDependence( NULL_GUID, blockDriverGUID, 0, DB_MODE_RW );
    }

    ocrEdtTemplateDestroy( blockTML );

    return NULL_GUID;
}

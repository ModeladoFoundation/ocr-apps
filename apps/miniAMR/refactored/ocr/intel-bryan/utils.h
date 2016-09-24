#define DEFAULT_LG_PROPS GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG

typedef struct{
    bool willRefine;
} refineMessage_t;

typedef struct{
    ocrGuid_t newChannels[4];
} refineConnections_t;

typedef struct{
    ocrGuid_t snd[30];          //multiply the number of indices by 4 (you either have 1:1 or 1:4
    ocrGuid_t rcv[30];          //neighbor relationship. Stride across this as you see fit.
    u32 neighborRefineLvls[6];
} block_comm_t;

typedef struct{
    ocrGuid_t rcv[6];
} comm_t;

typedef struct{
    ocrGuid_t neighbors[4];
    u32 n;
} halo_t;

typedef struct{
    ocrGuid_t EVT;
} connect_t;

typedef struct{
    u64 id;
    u64 timestep;
    ocrGuid_t TML;
    block_comm_t comms;

    //more to be packed in here.
} PRM_block_t;

typedef struct{
    u64 id;
    u64 timestep;
} PRM_stencilEdt_t;

typedef struct{
    ocrGuid_t range[6];
}range_t;

typedef struct{
    u32 refLvl;
    u32 maxRefLvl;
    block_comm_t comms;
    u64 timestep;
    u64 id;
    s64 parent;
    u64 rootId;
    ocrGuid_t blockTML;
    ocrGuid_t refineTML;
    ocrGuid_t refineSndTML;
    ocrGuid_t refineRcvTML;
    ocrGuid_t refineCtrlTML;
    ocrGuid_t intentLoopTML;
    ocrGuid_t communicateIntentTML;
    ocrGuid_t willRefineTML;
    ocrGuid_t updateIntentTML;
    ocrGuid_t stencilTML;
    ocrGuid_t haloSndTML;
    ocrGuid_t haloRcvTML; //we need to account for 1 or 4 neighbor values inbound.
}block_t;


void addDepsAndSatisfy( ocrGuid_t childGUID, ocrGuid_t * rcvs, ocrGuid_t * snds )
{
    u64 i;

    for( i = 1; i < 7; i++ ){
        ocrAddDependence( rcvs[i-1], childGUID, i, DB_MODE_RW );
    }

    for( i = 0; i < 6; i++ ){
        ocrEventSatisfy( snds[i], NULL_GUID );
    }

}

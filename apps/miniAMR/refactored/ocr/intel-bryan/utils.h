#define DEFAULT_LG_PROPS GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG

typedef struct{
    bool willRefine;
} refineMessage_t;

typedef struct{
    ocrGuid_t newChannels[4];
} refineConnections_t;

typedef struct{
} block_comm_t;

typedef struct{
    double data;
    u64 id;
}data_t;

typedef struct{
    data_t data[4];
    u64 num;
}dataBundle_t;

typedef struct{
    ocrGuid_t rcv[6];
    ocrGuid_t rRcv[6];
} comm_t;

typedef struct{
    ocrGuid_t sEVT;
    ocrGuid_t rEVT;
} connect_t;

typedef struct{
    u64 id;
    u64 timestep;
    ocrGuid_t TML;
    block_comm_t comms;
    //more to be packed in here.
} PRM_block_t;

typedef struct{
    double x, y, z;
}vec3;

typedef struct{
    vec3    pos;
    vec3    vel;
    double  size;
}object_t;

typedef struct{
    u64 id;
    u64 timestep;
} PRM_stencilEdt_t;

typedef struct{
    ocrGuid_t   range[6];
    ocrGuid_t   DBKObjects;
    ocrGuid_t   *reductionChannels;
    u32         numRedChannels;
}range_t;

typedef struct{
    ocrGuid_t snd[30];          //multiply the number of indices by 4 (you either have 1:1 or 1:4
    ocrGuid_t rcv[30];          //neighbor relationship. Stride across this as you see fit.
    ocrGuid_t rSnd[30];
    ocrGuid_t rRcv[30];
    ocrGuid_t blockTML;
    ocrGuid_t refineTML;
    ocrGuid_t refineSndTML;
    ocrGuid_t refineRcvTML;
    ocrGuid_t refineCtrlTML;
    ocrGuid_t communicateIntentTML;
    ocrGuid_t willRefineTML;
    ocrGuid_t updateIntentTML;
    ocrGuid_t stencilTML;
    ocrGuid_t haloSndTML;
    ocrGuid_t haloRcvTML; //we need to account for 1 or 4 neighbor values inbound.
    ocrGuid_t parentEVT;
    u64 id;
    s64 self;
    u64 numObjects;
    u64 rootId;
    u64 neighborIds[24];
    s64 neighborRefineLvls[6];
    s64 refLvl;
    u64 timestep;
    s64 parent;
    u32 maxRefLvl;
    u32 refineFreq;
    vec3 pos;
    vec3 size;
    vec3 halfSize;
    ocrGuid_t reductionChannel;
    object objects[100];
    double data;
}block_t;


double stencil_calcOCR( double * arr, u64 points )
{
    u64 i;
    double work, sum = 0.0f;

    for( i = 0; i < points; i++ ) sum += arr[i];

    work = sum / (double)points;

    return work;
}

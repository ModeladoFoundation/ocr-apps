#define ARITY 2

typedef struct{
    double data;
    ocrGuid_t event[ARITY+1];
/*
the events are sticky events for passing data blocks
event[0] is where I send
all the other events are receives all done with one cloning

*/
    } gsBlock_t;

ocrGuid_t GSiEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t GSxEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t computeInitEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]);

typedef struct{
    ocrGuid_t GSiTemplate;
    ocrGuid_t GSxTemplate;
    ocrGuid_t rootEvent;
    ocrGuid_t shimEvent;
    ocrGuid_t computeInitTemplate;
    double sum;
    double rtr0;
    double rtr;
    double rtrold;
    double rtz;
    double rtzold;
    double rtz0;
    u64 numnodes;
    ocrGuid_t userBlock;
    } GSsharedBlock_t;
#define M 300  //dimension of each slice
#define N 20  //number of participants
#define T 100  //number of iterations


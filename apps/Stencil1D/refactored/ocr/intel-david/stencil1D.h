
// Written by David S Scott

#define N 10  //number of workers (EDTs)
#define M 50   // size of local block
#define T 10000  //number of iterations

typedef struct{
    double buffer; //halo value
        ocrGuid_t control;
//used for exchanging data:
//Sticky: sticky event to be satisfied
//Once: once event to be safely satisfied
//Guid: the GUID of the nephew
    } buffer_t;

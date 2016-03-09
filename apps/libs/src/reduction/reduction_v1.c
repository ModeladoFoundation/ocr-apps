#include <stdio.h>
#include <stdlib.h>

#include "ocr.h"
#include "ocr-std.h" //TODO: Does this need to be included?

#include "reduction_v1.h"

//TODO: Garbage collection; The reduction tree seems to fail/hang when once events are used to set up the tree.
//Sticky events on the other hand result in a stable tree. However, the sticky events need to be destroyed
//when they are no longer needed. One suggestion is to pass them as parameters to the reduction EDTs to destroy them.
//This is not done yet.

//scalar double reduction EDT function. Pass this function pointer to reduction tree set up call to define the reduction operation.
//ocrGuid_t FNC_reduction_double(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

//The functions below are used in setting up the reduction tree
int mypow( int base, int exponent);
int getMaxDepth( int maxleaves );
void getNodesAtDepth( int querydepth, int maxleaves, int* leaves, int* nodes );

//nParticipants:    IN
//      Number of participants in the overall reduction
//PTR_EVT_inputs:   IN
//      The user provides a pointer to "N" events of the reduction input events
//      The function sets up the appropriate "ARITY"-ary reduction tree in the whole process
//EVT_reduction_out: IN
//      Output event for the reduction operation
//      Any EDT that depends on this event will be able to access the final reduction result
//FNC_reduction:    IN
//      Pointer to EDT function implementing the reduction for atmost "ARITY" input datablocks

void ocrLibRed_setup_tree_serial( u32 nParticipants, ocrGuid_t* PTR_EVT_inputs, ocrGuid_t EVT_reduction_out, ocrGuid_t (*FNC_reduction) (u32, u64*, u32, ocrEdtDep_t*) )
{
    ocrGuid_t DBK_reductionTreeHs;
    ocrGuid_t *PTR_reductionTreeHs;
    ocrDbCreate( &DBK_reductionTreeHs, (void **) &PTR_reductionTreeHs, nParticipants*sizeof(ocrGuid_t),
                 DB_PROP_NONE, NULL_GUID, NO_ALLOC );

    ocrGuid_t TML_reduction;

    int _paramc, _depc;
    _paramc = 0; _depc = EDT_PARAM_UNK; //A reduction EDT may have atmost ARITY dependencies depending on the leaves of the tree at each depth level
    ocrEdtTemplateCreate(&TML_reduction, FNC_reduction, _paramc, _depc );

    int maxleaves = nParticipants;
    int maxdepth = getMaxDepth( maxleaves ); //The tree's depth is at least 1

    int idepth = maxdepth;
    int leaves, nodes;
    int leafGap, nodeGap, rem;
    int leaves_lowerLevel, nodes_lowerLevel, rem_lowerLevel;
    getNodesAtDepth( idepth, maxleaves, &leaves, &nodes );

    ocrGuid_t EVT_reduction_out_once; //This is the output reduction event; This event will be satisfied by the final reduced result datablock.

    ocrEdtCreate( &PTR_reductionTreeHs[0], TML_reduction, EDT_PARAM_DEF, NULL, leaves, NULL, EDT_PROP_NONE, NULL_GUID, &EVT_reduction_out_once );
    ocrAddDependence( EVT_reduction_out_once, EVT_reduction_out, 0, DB_MODE_RW );

    //PRINTF("maxleaves = %d maxdepth = %d ARITY = %d\n", maxleaves, maxdepth, ARITY);
    //PRINTF("Creating EDT %d(level=%d) with depc %d\n", 0, idepth, leaves);

    for( idepth = maxdepth; idepth > 1; idepth-- )
    {
        leafGap = mypow( ARITY, idepth-1);
        nodeGap  = leafGap*ARITY;

        getNodesAtDepth( idepth, maxleaves, &leaves, &nodes );
        rem = leaves&(ARITY-1);

        getNodesAtDepth( idepth-1, maxleaves, &leaves_lowerLevel, &nodes_lowerLevel );
        rem_lowerLevel = leaves_lowerLevel&(ARITY-1);

        int inode_lowerLevel = 0;

        for( int inode=0; inode < nodes; inode++ )
        {
            ocrGuid_t EDT_node = PTR_reductionTreeHs[inode*nodeGap]; //Already created before

            //Create leaf EDTs and set up dependencies for the pre-existing node EDT
            int nleaves_node = ( inode != nodes-1 ) ? (ARITY) : ( (rem==0) ? ARITY : rem ); //Only the last node's reduction EDT will have "reminder" deps

            for( int ileaf = 0; ileaf < nleaves_node; ileaf++, inode_lowerLevel++ )
            {
                int ndep_leaf = ( inode_lowerLevel != nodes_lowerLevel-1 ) ? (ARITY) : ( (rem_lowerLevel==0) ? ARITY : rem_lowerLevel );
                //Only the last leaf reduction EDT participating in the last "node" reduction EDT will have "reminder" deps

                ocrGuid_t event, EVT_out;
                ocrEventCreate( &EVT_out, OCR_EVENT_STICKY_T, true ); //TODO: How to destroy these later when no longer needed?
                _depc = ndep_leaf;
                ocrEdtCreate( &(PTR_reductionTreeHs[inode*nodeGap+ileaf*leafGap]), TML_reduction, EDT_PARAM_DEF, NULL, _depc, NULL, EDT_PROP_NONE, NULL_GUID, &event);
                ocrAddDependence( event, EVT_out, 0, ileaf ? DB_MODE_RO : DB_MODE_RW );
                ocrAddDependence( EVT_out, EDT_node, ileaf, ileaf ? DB_MODE_RO : DB_MODE_RW );
                //PRINTF("Creating EDT %d(level=%d) with depc %d with output slot triggering edt %d(level=%d) on slot %d\n",
                //        inode*nodeGap+ileaf*leafGap, idepth-1, ndep_leaf, inode*nodeGap, idepth, ileaf);
            }
        }
    }

    //Create the reduction input events and set up the dependencies for the previously created reduction EDTs.
    //The input array is filled in here with reduction events.
    idepth = 1;
    leafGap = mypow( ARITY, idepth-1);
    nodeGap  = leafGap*ARITY;

    getNodesAtDepth( idepth, maxleaves, &leaves, &nodes );
    rem = leaves&(ARITY-1);

    for( int inode=0; inode < nodes; inode++ )
    {
        ocrGuid_t EDT_node = PTR_reductionTreeHs[inode*nodeGap];//Already created before

        //Create child EDTs and set up dependencies for the pre-existing node
        int nleaves_node = ( inode != nodes-1 ) ? (ARITY) : ( (rem==0) ? ARITY : rem ); //Only the last node reduction EDT will have "reminder" deps

        for( int ileaf = 0; ileaf < nleaves_node; ileaf++ )
        {
            ocrGuid_t event = PTR_EVT_inputs[inode*nodeGap+ileaf*leafGap];
            ocrAddDependence( event, EDT_node, ileaf, ileaf ? DB_MODE_RO : DB_MODE_RW );
            //PRINTF("Creating event guid %d which triggers EDT guid %d(level=%d) on slot %d\n", inode*nodeGap+ileaf*leafGap, inode*nodeGap, idepth, ileaf);
        }
    }

    ocrEdtTemplateDestroy( TML_reduction );

    ocrDbDestroy( DBK_reductionTreeHs );

    return;
}

ocrGuid_t FNC_reduction_double(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //PRINTF("%s\n", __func__);
    s32 _paramc, _depc, _idep;
    s32 ielem, NELEM = 1;
    double* PTR_reduction_out = depv[0].ptr;

    for( _idep = 1; _idep < depc; _idep++ )
    {
        double* PTR_reduction_in = depv[_idep].ptr;
        for( ielem = 0; ielem < NELEM; ielem++ )
            PTR_reduction_out[ielem] += PTR_reduction_in[ielem];
    }

    return depv[0].guid;
}

int mypow( int base, int exponent)
{
    // Does not work for negative exponents.
    if (exponent == 0) return 1;  // base case;
    int temp = mypow(base, exponent/2);
    if (exponent % 2 == 0)
        return temp * temp;
    else
        return (base * temp * temp);
}

int getMaxDepth( int maxleaves )
{
    int maxdepth = 0; //The tree's depth is at least 1

    int leaves;
    int nodes = maxleaves;

    do
    {
        ++maxdepth;
        leaves = nodes;
        nodes = leaves/ARITY + ( leaves&(ARITY-1) ? 1 : 0 );
    }
    while( nodes != 1 );

    return maxdepth;
}

void getNodesAtDepth( int querydepth, int maxleaves, int* leaves, int* nodes )
{
    *nodes = maxleaves;

    for( int idepth = 1; idepth <= querydepth; idepth++ )
    {
        *leaves = *nodes;
        *nodes = *leaves/ARITY + ( *leaves&(ARITY-1) ? 1 : 0 );
    }

    return;
}

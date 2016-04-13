#include <math.h>
#include <ocr.h>

#include "extensions/ocr-affinity.h"

#include "comd.h"
#include "reductions.h"

int mypow( int base, int exponent);
int getMaxDepth( int maxleaves );
void getNodesAtDepth( int querydepth, int maxleaves, int* leaves, int* nodes );

ocrGuid_t build_reduction(ocrGuid_t sim_g, ocrGuid_t reductionH_g, u32 nParticipants, ocrGuid_t* leaves_p,
                          u32 paramc, PRM_red_t* PTR_PRM_red, ocrGuid_t (*red_edt)(u32, u64*, u32, ocrEdtDep_t*), u32* grid, int key)
{
  ocrGuid_t root;
  ocrGuid_t TML_reduction;

  int _paramc, _depc;
  _paramc = EDT_PARAM_UNK; _depc = EDT_PARAM_UNK; //A reduction EDT may have at most FANIN dependencies depending on the leaves of the tree at each depth level
  ocrEdtTemplateCreate(&TML_reduction, red_edt, _paramc, _depc );

    int maxleaves = nParticipants;
    int maxdepth = getMaxDepth( maxleaves ); //The tree's depth is at least 1

    int idepth = maxdepth;
    int leaves, nodes;
    int leafGap, nodeGap, rem;
    int leaves_lowerLevel, nodes_lowerLevel, rem_lowerLevel;
    getNodesAtDepth( idepth, maxleaves, &leaves, &nodes );

  PRM_ured_edt_t PRM_ured_edt;
  PRM_tred_edt_t PRM_tred_edt;
  PRM_vred_edt_t PRM_vred_edt;
  _depc = leaves+2;

    ocrGuid_t PDaffinityGuid = NULL_GUID;

    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );

#ifdef ENABLE_EXTENSION_AFFINITY
    s64 affinityCount;
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
    ocrGuid_t DBK_affinityGuids;
    ocrGuid_t* PTR_affinityGuids;
    ocrDbCreate( &DBK_affinityGuids, (void**) &PTR_affinityGuids, sizeof(ocrGuid_t)*affinityCount,
                 DB_PROP_SINGLE_ASSIGNMENT, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    ocrAffinityGet( AFFINITY_PD, &affinityCount, PTR_affinityGuids ) //Get all the available Policy Domain affinity guids;
    ASSERT( affinityCount >= 1 );
    //PRINTF("Using affinity API\n");
    s64 PD_X, PD_Y, PD_Z;
    splitDimension(affinityCount, &PD_X, &PD_Y, &PD_Z); //Split available PDs into a 3-D grid

    int pd = globalRankFromCoords(0, 0, 0, PD_X, PD_Y, PD_Z);
    //PRINTF("box %d %d %d, policy domain %d: %d %d %d\n", id_x, id_y, id_z, pd, PD_X, PD_Y, PD_Z);
    PDaffinityGuid = PTR_affinityGuids[pd];
    ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#else
    //PRINTF("NOT Using affinity API\n");
#endif

    switch(key)
    {
        case 0: //u_red_edt
            PRM_ured_edt.n = leaves;
            PRM_ured_edt.epsilon = PTR_PRM_red->epsilon;
            ocrEdtCreate(&leaves_p[0], TML_reduction, sizeof(PRM_ured_edt_t)/sizeof(u64), (u64*)&PRM_ured_edt, _depc, NULL,
                            EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), &root);
            break;
        case 1:
            PRM_tred_edt.n = leaves;
            PRM_tred_edt.temperature = PTR_PRM_red->temperature;
            PRM_tred_edt.guid = PTR_PRM_red->guid;
            ocrEdtCreate(&leaves_p[0], TML_reduction, sizeof(PRM_tred_edt_t)/sizeof(u64), (u64*)&PRM_tred_edt, _depc, NULL,
                            EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), &root);
            break;
        case 2:
            PRM_vred_edt.n = leaves;
            PRM_vred_edt.guid = PTR_PRM_red->guid;
            ocrEdtCreate(&leaves_p[0], TML_reduction, sizeof(PRM_vred_edt_t)/sizeof(u64), (u64*)&PRM_vred_edt, _depc, NULL,
                            EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), &root);
            break;
    }

  ocrAddDependence(sim_g,leaves_p[0],leaves,DB_MODE_RW);
  ocrAddDependence(reductionH_g,leaves_p[0],leaves+1,DB_MODE_RW);

  u64 paramv[1];

    for( idepth = maxdepth; idepth > 1; idepth-- )
    {
        leafGap = mypow( FANIN, idepth-1);
        nodeGap  = leafGap*FANIN;

        getNodesAtDepth( idepth, maxleaves, &leaves, &nodes );
        rem = leaves&(FANIN-1);

        getNodesAtDepth( idepth-1, maxleaves, &leaves_lowerLevel, &nodes_lowerLevel );
        rem_lowerLevel = leaves_lowerLevel&(FANIN-1);

        int inode_lowerLevel = 0;

        for( int inode=0; inode < nodes; inode++ )
        {
            ocrGuid_t EDT_node = leaves_p[inode*nodeGap]; //Already created before

            //Create leaf EDTs and set up dependencies for the pre-existing node EDT
            int nleaves_node = ( inode != nodes-1 ) ? (FANIN) : ( (rem==0) ? FANIN : rem ); //Only the last node's reduction EDT will have "reminder" deps

            for( int ileaf = 0; ileaf < nleaves_node; ileaf++, inode_lowerLevel++ )
            {
                int b = inode*nodeGap+ileaf*leafGap;
                PDaffinityGuid = NULL_GUID;
#ifdef ENABLE_EXTENSION_AFFINITY
                int pd = getPoliyDomainID( b, grid, PD_X, PD_Y, PD_Z );
                PDaffinityGuid = PTR_affinityGuids[pd];
                ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif
                int ndep_leaf = ( inode_lowerLevel != nodes_lowerLevel-1 ) ? (FANIN) : ( (rem_lowerLevel==0) ? FANIN : rem_lowerLevel );
                //Only the last leaf reduction EDT participating in the last "node" reduction EDT will have "reminder" deps

                ocrGuid_t event;
                _paramc = 1; _depc = ndep_leaf;
                paramv[0] = _depc;
                ocrEdtCreate( &(leaves_p[inode*nodeGap+ileaf*leafGap]), TML_reduction, _paramc, paramv, _depc, NULL,
                                EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), &event);
                ocrAddDependence( event, EDT_node, ileaf, ileaf ? DB_MODE_RO : DB_MODE_RW );
                //PRINTF("Creating EDT %d(level=%d) with depc %d with output slot triggering edt %d(level=%d) on slot %d\n",
                //        inode*nodeGap+ileaf*leafGap, idepth-1, ndep_leaf, inode*nodeGap, idepth, ileaf);
            }
        }
    }

    //Create the reduction input events and set up the dependencies for the previously created reduction EDTs.
    //The input array is filled in here with reduction events.
    idepth = 1;
    leafGap = mypow( FANIN, idepth-1);
    nodeGap  = leafGap*FANIN;

    getNodesAtDepth( idepth, maxleaves, &leaves, &nodes );
    rem = leaves&(FANIN-1);

    for( int inode=0; inode < nodes; inode++ )
    {
        ocrGuid_t EDT_node = leaves_p[inode*nodeGap];//Already created before

        //Create child EDTs and set up dependencies for the pre-existing node
        int nleaves_node = ( inode != nodes-1 ) ? (FANIN) : ( (rem==0) ? FANIN : rem ); //Only the last node reduction EDT will have "reminder" deps

        for( int ileaf = 0; ileaf < nleaves_node; ileaf++ )
        {
            ocrGuid_t event;
            ocrEventCreate(&event,OCR_EVENT_ONCE_T,1);
            ocrAddDependence( event, EDT_node, ileaf, ileaf ? DB_MODE_RO : DB_MODE_RW );
            leaves_p[inode*nodeGap+ileaf*leafGap] = event;
            //PRINTF("Creating event guid %d which triggers EDT guid %d(level=%d) on slot %d\n", inode*nodeGap+ileaf*leafGap, inode*nodeGap, idepth, ileaf);
        }
    }

#ifdef ENABLE_EXTENSION_AFFINITY
    ocrDbDestroy( DBK_affinityGuids );

    ocrEdtTemplateDestroy( TML_reduction );
#endif

  return root;
}

//params: n, epsilon
//depv: uk0..ukn-1, [sim_g, reductionH_g]
ocrGuid_t ured_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  ocrGuid_t uk0_g = depv[0].guid;
  real_t* u = (real_t*)depv[0].ptr;

  PRM_ured_edt_t* PTR_PRM_ured_edt = (PRM_ured_edt_t*)paramv;

  u32 d; u64 n = PTR_PRM_ured_edt->n;

  for(d=1; d<n; ++d) {
    u[0] += ((real_t*)depv[d].ptr)[0];
    u[1] += ((real_t*)depv[d].ptr)[1];
    ocrDbDestroy(depv[d].guid);
  }

  if(paramc>1) {
    ocrGuid_t sim_g = depv[n].guid;
    simulation_t* sim_p = depv[n].ptr;
    sim_p->e_potential = u[0]*PTR_PRM_ured_edt->epsilon;
    sim_p->e_kinetic = u[1];

    reductionH_t* reductionH_p = (reductionH_t*)depv[depc-1].ptr;
    reductionH_p->OEVT_reduction = NULL_GUID;

    ocrDbDestroy(uk0_g);

    return sim_g;

  }

  return uk0_g;
}

//params: n, OEVT_tred
//depv: vcm0..vcmn-1, [sim_g,reductionH_g]
ocrGuid_t vred_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  ocrGuid_t vcm0_g = depv[0].guid;
  real_t* vcm0 = (real_t*)depv[0].ptr;

  PRM_vred_edt_t* PTR_PRM_vred_edt = (PRM_vred_edt_t*)paramv;

  u32 d; u64 n = PTR_PRM_vred_edt->n;

  for(d=1; d<n; ++d) {
    real_t* vcm1 = (real_t*)depv[d].ptr;
    vcm0[0] += vcm1[0];
    vcm0[1] += vcm1[1];
    vcm0[2] += vcm1[2];
    vcm0[3] += vcm1[3];
    ((u64*)vcm0)[4] += ((u64*)vcm1)[4];
    ocrDbDestroy(depv[d].guid);
  }

  if(paramc>1) {
    ocrGuid_t sim_g = depv[n].guid;
    simulation_t* sim_p = (simulation_t*)depv[n].ptr;
    sim_p->atoms0 = sim_p->atoms = ((u64*)vcm0)[4];

    ocrGuid_t reductionH_g = depv[depc-1].guid;
    reductionH_t* reductionH_p = (reductionH_t*)depv[depc-1].ptr;
    reductionH_p->value[0] = -vcm0[0]/vcm0[3];
    reductionH_p->value[1] = -vcm0[1]/vcm0[3];
    reductionH_p->value[2] = -vcm0[2]/vcm0[3];
    reductionH_p->OEVT_reduction = PTR_PRM_vred_edt->guid; //OEVT_tred

    ocrDbDestroy(vcm0_g);
    return reductionH_g;
  }

  return vcm0_g;
}

//params: n, temperature, OEVT_ured
//depv: ek0..ekn-1, [sim_g,reductionH_g]
ocrGuid_t tred_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  ocrGuid_t ek0_g = depv[0].guid;
  real_t* ek = (real_t*)depv[0].ptr;

  PRM_tred_edt_t* PTR_PRM_tred_edt = (PRM_tred_edt_t*)paramv;

  u32 d; u64 n = PTR_PRM_tred_edt->n;

  for(d=1; d<n; ++d) {
    *ek += *(real_t*)depv[d].ptr;
    ocrDbDestroy(depv[d].guid);
  }

  if(paramc>1) {
    ocrGuid_t sim_g = depv[n].guid;
    simulation_t* sim_p = depv[n].ptr;
    *ek /= (sim_p->atoms*kB_eV_1_5);

    ocrGuid_t reductionH_g = depv[depc-1].guid;
    reductionH_t* reductionH_p = (reductionH_t*)depv[depc-1].ptr;
    reductionH_p->value[0] = sqrt((double)PTR_PRM_tred_edt->temperature/ *ek);
    reductionH_p->OEVT_reduction = PTR_PRM_tred_edt->guid; //OEVT_ured

    ocrDbDestroy(ek0_g);
    return reductionH_g;
  }

  return ek0_g;
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
        nodes = leaves/FANIN + ( leaves&(FANIN-1) ? 1 : 0 );
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
        *nodes = *leaves/FANIN + ( *leaves&(FANIN-1) ? 1 : 0 );
    }

    return;
}

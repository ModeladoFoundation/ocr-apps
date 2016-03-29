///////////////////////////////////////////////////////////////////////////////
///
///	\file    TaskTestOCR.cpp
///	\author  Paul Ullrich, Bryan Pawlowski
///	\version February 29, 2016
///
///	<remarks>
///		Copyright 2000-2010 Paul Ullrich
///
///		This file is distributed as part of the Tempest source code package.
///		Permission is granted to use, copy, modify and distribute this
///		source code and its documentation under the terms of the GNU General
///		Public License.  This software is provided "as is" without express
///		or implied warranty.
///	</remarks>


/*
 * This code is written without the db-alloc library written by Nick Vrvilo. This is basically a proof of
 * concept for Paul Ullrich to show what needs to happen to OCR-ify the current Tempest library.
 */


#include "Tempest.h"
#include "GridPatchCartesianGLL.h"

#include <ocr.h>
#include <ocr-std.h>
#include <extensions/ocr-labeling.h>
#include <iostream>

static const int ParamHorizontalOrder = 4;
static const int ParamVerticalOrder = 4;

static const int ParamTotalPatchCount = 2;
static const int ParamNeighborsPerPatch = 8;

static const int ParamTotalSteps = 2;

using namespace std;

typedef struct {
    ocrEdtDep_t modelDep;
    ocrEdtDep_t gridDep;
    ocrEdtDep_t gridPatchDep;
    ocrEdtDep_t timestepDep;
    ocrEdtDep_t horizDynDep;
    ocrEdtDep_t vertDynDep;
} metaDeps;

typedef struct realMainPRM {
    u64 patchNum;
    ocrGuid_t RangeToLeft, RangeToRight;
} realMainPRM;

typedef struct control {
    ocrGuid_t in, out;
} control;

typedef struct privateBlock {
    control left, right;
} privateBlock;

#define T_MAX 10
#define DEFAULT_LG_PROPS GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG

ocrGuid_t gridPatchExecuteEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] ) {

   /* paramv[3]:
    *  0: ixPatch
    *  1: ixStep
    *  2: ixSubStep
    *
    * depv[7]:
    *  0: model (RO)
    *  1: grid (RO)
    *  2: gridPatch
    *  3: pGeometricData
    *  4: pActivestateData
    *  5: pBufferStateData
    *  6: pAuxiliaryData
    *
    *
    * [UNIMPLEMENTED INPUT TODO]
    *   nExchangeBuffers
    *   rgExchangeBufferIds
    *   rgRecvBufferPtrs
    *   rgSendBufferPtrs
    *
    */

    metaDeps * meta = (metaDeps *)depv;

    Model * pModel = (Model *)meta->modelDep.ptr;

    GridCartesianGLL *pGrid = (GridCartesianGLL *)meta->gridDep.ptr;

    ExchangeBufferRegistry & ebr = pGrid->GetExchangeBufferRegistry();

    std::vector<int> vecExchangeBufferIndices;

    /*------------CHECK AND REATTACH INTERNAL POINTERS-----------------*/


    if( pModel != &(pGrid->GetModel()) ) pGrid->ResetModel( *pModel );

    if( pGrid != pModel->GetGrid() ) pModel->ResetGrid( pGrid );

    TimestepScheme * tssPTR = (TimestepScheme *)meta->timestepDep.ptr;
    if( pModel->GetTimestepScheme() != tssPTR ) pModel->ResetTimestepScheme( tssPTR );

    HorizontalDynamics * hdPTR = (HorizontalDynamics *)meta->horizDynDep.ptr;
    if( pModel->GetHorizontalDynamics() != hdPTR ) pModel->ResetHorizontalDynamics( hdPTR );

    VerticalDynamics * vdPTR = (VerticalDynamics *)meta->vertDynDep.ptr;
    if( pModel->GetVerticalDynamics() != vdPTR ) pModel->ResetVerticalDynamics( vdPTR );

    /*----------END CHECK AND REATTACH INTERNAL POINTERS---------------*/

    u64 * stateInfo = paramv;

    u64 ixPatch = stateInfo[0];
    u64 ixStep = stateInfo[1];
    u64 ixSubStep = stateInfo[2];
    u64 subStepCt = stateInfo[3];

    ebr.GetExchangeBuffersBySourcePatchIx( ixPatch, vecExchangeBufferIndices );

    if( ixStep < T_MAX ){

        if( ixPatch == 0 ) {
            PRINTF("timestep %ld.%ld\n", ixStep, ixSubStep);
            fflush( stdout );
        }

        if( ixSubStep < subStepCt - 1 ) paramv[2]++;
        else{
            paramv[2] = 0;
            paramv[1]++;
        }
    } else {
        if( ixPatch == 0 )PRINTF("number of neighbors: %ld\n", vecExchangeBufferIndices.size());
        return NULL_GUID;
    }

    /*---------------OCR Calls----------------*/

    privateBlock * priv = (privateBlock *)depv[12].ptr;

    if( ixStep == 0 && ixSubStep == 0) {

        if( depv[10].ptr != NULL ){
            ocrGuid_t * leftOut = (ocrGuid_t *)depv[10].ptr;
            priv->left.out = *leftOut;
        }
        if( depv[11].ptr != NULL ){
            ocrGuid_t * rightOut = (ocrGuid_t *)depv[11].ptr;
            priv->right.out = *rightOut;
        }
    }

    ocrGuid_t childTML, childGUID;

    ocrEdtTemplateCreate( &childTML, gridPatchExecuteEdt, 4, 13 );
    ocrEdtCreate( &childGUID, childTML, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                    EDT_PROP_NONE, NULL_GUID, NULL );

    u64 _idep;

    for( _idep = 0; _idep < 10; _idep++ ){

        ocrDbRelease( depv[_idep].guid );
        ocrAddDependence( depv[_idep].guid, childGUID, _idep, DB_MODE_RW );

    }

    ocrAddDependence( priv->left.in, childGUID, 10, DB_MODE_RW );
    ocrAddDependence( priv->right.in, childGUID, 11, DB_MODE_RW );

    if( ixPatch != 0 ) ocrEventSatisfy( priv->left.out, NULL_GUID );
    if( ixPatch != ParamTotalPatchCount - 1 ) ocrEventSatisfy( priv->right.out, NULL_GUID );


    ocrDbRelease( depv[12].guid );
    ocrAddDependence( depv[12].guid, childGUID, 12, DB_MODE_RW );


    return NULL_GUID;

}

ocrGuid_t gridPatchAttachEdt( u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[] ) {

    Model * pModel = (Model *)depv[0].ptr;
    privateBlock * priv = (privateBlock *) depv[12].ptr;

    priv->left.in = NULL_GUID;
    priv->left.out = NULL_GUID;
    priv->right.in = NULL_GUID;
    priv->right.out = NULL_GUID;

    u64 numSubs = pModel->GetTimestepScheme()->GetSubStepCount();

    PRINTF("GridPatchAttachEdt!\n");

    u64 params[4] = { paramv[0], 0, 0, numSubs};

    ocrEdtDep_t gridPatch = depv[2];

    GridPatch * pPatch = (GridPatch *)gridPatch.ptr;

    pPatch->GetDataContainerGeometric().Detach();
    pPatch->GetDataContainerActiveState().Detach();
    pPatch->GetDataContainerBufferState().Detach();
    pPatch->GetDataContainerAuxiliary().Detach();

    pPatch->GetDataContainerGeometric().AttachTo( (unsigned char *)depv[3].ptr );
    pPatch->GetDataContainerActiveState().AttachTo( (unsigned char *)depv[4].ptr );
    pPatch->GetDataContainerBufferState().AttachTo( (unsigned char *)depv[5].ptr );
    pPatch->GetDataContainerAuxiliary().AttachTo( (unsigned char *)depv[6].ptr );


    /*---------------OCR SPECIFIC CALLS--------------*/

    ocrEventParams_t prms;

    prms.EVENT_CHANNEL.maxGen = 2;
    prms.EVENT_CHANNEL.nbSat = 1;
    prms.EVENT_CHANNEL.nbDeps = 1;



    ocrGuid_t patchExecuteTML, patchExecuteGUID;

    ocrEdtTemplateCreate( &patchExecuteTML, gridPatchExecuteEdt, 4, 13 );
    ocrEdtCreate( &patchExecuteGUID, patchExecuteTML, EDT_PARAM_DEF, params, EDT_PARAM_DEF, NULL,
                    EDT_PROP_NONE, NULL_GUID, NULL );

    u64 i;

    for( i = 0; i < 10; i++ ){
        ocrDbRelease( depv[i].guid );
        ocrAddDependence( depv[i].guid, patchExecuteGUID, i, DB_MODE_RW );
    }

    realMainPRM * prm = (realMainPRM *)paramv;


    ocrGuid_t outLeft, inLeft, outRight, inRight;

    if( prm->patchNum == 0 ){
        outLeft = NULL_GUID;
        inLeft = NULL_GUID;
    }else{
        ocrGuidFromIndex( &outLeft, prm->RangeToLeft, prm->patchNum );
        ocrGuidFromIndex( &inLeft, prm->RangeToRight, prm->patchNum - 1 );

        ocrEventCreate( &outLeft, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
        ocrEventCreate( &inLeft, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );

        ocrEventCreateParams( &(priv->left.in), OCR_EVENT_CHANNEL_T, false, &prms );
    }

    if( prm->patchNum == ParamTotalPatchCount - 1 ){
        outRight = NULL_GUID;
        inRight = NULL_GUID;
    }else{
        ocrGuidFromIndex( &outRight, prm->RangeToRight, prm->patchNum );
        ocrGuidFromIndex( &inRight, prm->RangeToLeft, prm->patchNum + 1 );

        ocrEventCreate( &outRight, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );
        ocrEventCreate( &inRight, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS );

        ocrEventCreateParams( &(priv->right.in), OCR_EVENT_CHANNEL_T, false, &prms );
    }

    ocrAddDependence( inLeft, patchExecuteGUID, 10, DB_MODE_RW );
    ocrAddDependence( inRight, patchExecuteGUID, 11, DB_MODE_RW );

    ocrGuid_t   *lChannelOut = (ocrGuid_t *)depv[10].ptr,
                *rChannelOut = (ocrGuid_t *)depv[11].ptr;

    if( lChannelOut != NULL ) *lChannelOut = priv->left.in;
    if( rChannelOut != NULL ) *rChannelOut = priv->right.in;

    if( outLeft != NULL_GUID ) ocrEventSatisfy( outLeft, depv[10].guid );
    if( outRight != NULL_GUID ) ocrEventSatisfy( outRight, depv[11].guid );

    ocrDbRelease( depv[12].guid );
    ocrAddDependence( depv[12].guid, patchExecuteGUID, 12, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t gridPatchInitEdt( u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[] ) {

    u64 ixPatch = paramv[0];        //this will change with a higher number of patches.

    GridCartesianGLL * pGrid = (GridCartesianGLL *)depv[1].ptr;

    ocrEdtDep_t gridPatch = depv[2];


    const PatchBox & box = pGrid->GetPatchBox( ixPatch );

    GridPatch * pPatch = new(gridPatch.ptr) GridPatchCartesianGLL(
                                *pGrid,
                                ixPatch,
                                box,
                                ParamHorizontalOrder,
                                ParamVerticalOrder );

    pPatch->InitializeDataLocal();

    ocrGuid_t gDBK, asDBK, bsDBK, aDBK,
                leftDBK = NULL_GUID, rightDBK = NULL_GUID, privateDBK;

    ocrGuid_t gridPatchAttachTML, gridPatchAttachGUID;
    double * dummy;

    ocrDbCreate( &gDBK, (void **)&dummy, pPatch->GetDataContainerGeometric().GetTotalByteSize(),
                    0, NULL_GUID, NO_ALLOC );
    ocrDbCreate( &asDBK, (void **)&dummy, pPatch->GetDataContainerActiveState().GetTotalByteSize(),
                    0, NULL_GUID, NO_ALLOC );
    ocrDbCreate( &bsDBK, (void **)&dummy, pPatch->GetDataContainerBufferState().GetTotalByteSize(),
                    0, NULL_GUID, NO_ALLOC );
    ocrDbCreate( &aDBK, (void **)&dummy, pPatch->GetDataContainerAuxiliary().GetTotalByteSize(),
                    0, NULL_GUID, NO_ALLOC );

    if( ixPatch != 0 ) ocrDbCreate( &leftDBK, (void **)&dummy, sizeof(control),
                        0, NULL_GUID, NO_ALLOC );
    if( ixPatch != ParamTotalSteps - 1 ) ocrDbCreate( &rightDBK, (void **)&dummy, sizeof(control),
                        0, NULL_GUID, NO_ALLOC );

    ocrDbCreate( &privateDBK, (void **)&dummy, sizeof(privateBlock),
                        0, NULL_GUID, NO_ALLOC );

    ocrEdtTemplateCreate( &gridPatchAttachTML, gridPatchAttachEdt, paramc, 13 );

    ocrEdtCreate( &gridPatchAttachGUID, gridPatchAttachTML, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                    EDT_PROP_NONE, NULL_GUID, NULL );

    u64 _idep = 0;

    ocrAddDependence( depv[0].guid, gridPatchAttachGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( depv[1].guid, gridPatchAttachGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( gridPatch.guid, gridPatchAttachGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( depv[3].guid, gridPatchAttachGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( depv[4].guid, gridPatchAttachGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( depv[5].guid, gridPatchAttachGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( gDBK, gridPatchAttachGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( asDBK, gridPatchAttachGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( bsDBK, gridPatchAttachGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( aDBK, gridPatchAttachGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( leftDBK, gridPatchAttachGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( rightDBK, gridPatchAttachGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( privateDBK, gridPatchAttachGUID, _idep++, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t tempestInitEdt( u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[] ) {

	const int nResolutionX = 10;
	const int nResolutionY = 10;
	const int nLevels = 4;
	const int nMaxPatchCount = ParamTotalPatchCount;

	double dGDim[6];
	dGDim[0] = 0.0;
	dGDim[1] = 1000.0;
	dGDim[2] = -1000.0;
	dGDim[3] = 1000.0;
	dGDim[4] = 0.0;
	dGDim[5] = 1000.0;

    int iLatBC[4];

    iLatBC[0] = Grid::BoundaryCondition_Periodic;
    iLatBC[1] = Grid::BoundaryCondition_NoFlux;
    iLatBC[2] = Grid::BoundaryCondition_Periodic;
    iLatBC[3] = Grid::BoundaryCondition_NoFlux;

	const double dRefLat = 0.0;

    double * dummy;

    ocrEdtDep_t modelDep = depv[0];
    ocrEdtDep_t gridDep = depv[1];
    ocrEdtDep_t timestepDep = depv[2];
    ocrEdtDep_t horizDynDep = depv[3];
    ocrEdtDep_t vertDynDep = depv[4];

    PRINTF("Model GUID: %lx, PTR: %ld\n", modelDep.guid, modelDep.ptr);
    PRINTF("Grid GUID: %lx, PTR: %ld\n", gridDep.guid, gridDep.ptr);

    if( gridDep.guid == NULL_GUID ) return NULL_GUID;

    ocrGuid_t gridDBK, tssDBK, hdDBK, vdDBK;
    ocrGuid_t patchInitTML, patchInitGUID;

    Grid::VerticalDiscretization eVerticalDiscretization = Grid::VerticalDiscretization_FiniteElement;
	const Grid::VerticalStaggering eVerticalStaggering =
		Grid::VerticalStaggering_Levels;

    cout << "Allocating Model" << endl;

    u64 nCommSize = 2;
    u64 i;

    cout << "Initializing Model" << endl;

    Model * pModel = new(modelDep.ptr) Model(EquationSet::PrimitiveNonhydrostaticEquations);
    if( pModel == NULL ) return NULL_GUID;

    cout << "Model constructed and placed into datablock." << endl;

    pModel->SetTimestepScheme( new(timestepDep.ptr) TimestepSchemeStrang(*pModel) );
    pModel->SetHorizontalDynamics( new(horizDynDep.ptr) HorizontalDynamicsStub( *pModel ) );
    pModel->SetVerticalDynamics( new(vertDynDep.ptr) VerticalDynamicsStub( *pModel ) );

    cout << "Model Initialized!" << endl;

    cout << "Initializing Grid" << endl;

    GridCartesianGLL * pGrid = new(gridDep.ptr) GridCartesianGLL( *pModel );
    //GridCartesianGLL * pGrid = new GridCartesianGLL( *pModel );

    if( pGrid == NULL ) return NULL_GUID;

    cout << "Grid constructed and placed into datablock" << endl;

    pGrid->DefineParameters();

    pGrid->SetParameters(
        nLevels,
        nCommSize,
        nResolutionX,
        nResolutionY,
        4,
        ParamHorizontalOrder,
        ParamVerticalOrder,
        dGDim,
        dRefLat,
        iLatBC,
        eVerticalDiscretization,
        eVerticalStaggering );

    pGrid->InitializeDataLocal();

    pModel->SetGrid(pGrid, ParamTotalPatchCount, false);

    pGrid->InitializeAllExchangeBuffers();

    pGrid->GetExchangeBufferRegistry().SetNoDataOwnership();

    std::vector<int> vecExchangeBufferIndices;
    ExchangeBufferRegistry & ebr = pGrid->GetExchangeBufferRegistry();

    ebr.GetExchangeBuffersBySourcePatchIx( 0, vecExchangeBufferIndices );

    /*---------------OCR SPECIFICS---------------*/

    ocrDbRelease( modelDep.guid );
    ocrDbRelease( gridDep.guid );
    ocrDbRelease( timestepDep.guid );
    ocrDbRelease( horizDynDep.guid );
    ocrDbRelease( vertDynDep.guid );

    ocrEdtTemplateCreate( &patchInitTML, gridPatchInitEdt, paramc, 6 ); //pass the params down the line

    ocrGuid_t gridPatchDBK;
    ocrEdtCreate( &patchInitGUID, patchInitTML, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                    EDT_PROP_NONE, NULL_GUID, NULL );

    ocrDbCreate( &gridPatchDBK, (void **)&dummy, sizeof( GridPatchCartesianGLL ), 0, NULL_GUID, NO_ALLOC );

    u64 _idep = 0;

    ocrAddDependence( modelDep.guid, patchInitGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( gridDep.guid, patchInitGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( gridPatchDBK, patchInitGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( timestepDep.guid, patchInitGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( horizDynDep.guid, patchInitGUID, _idep++, DB_MODE_RW );
    ocrAddDependence( vertDynDep.guid, patchInitGUID, _idep++, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t wrapupEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] ) {

    ocrShutdown();

    return NULL_GUID;

}


ocrGuid_t realMainEdt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] ) {

/*
 * This is where we will split the running. It is now clear that each edt 'rank' must have its own copy of
 * Model, Grid and GridPatch.
 *
 */

ocrGuid_t ModelDBK, GridDBK, TimestepSchemeDBK, HorizontalDynamicsDBK, VerticalDynamicsDBK;
ocrGuid_t tempestInitTML, tempestInitGUID;



double * dummy;

realMainPRM prm;

PRINTF("Hello! realMainEdt\n");

u64 paramSize = sizeof( realMainPRM ) / sizeof( u64 );

cout << "paramSize: " << paramSize << endl;

ocrEdtTemplateCreate( &tempestInitTML, tempestInitEdt, paramSize, 5 );


ocrGuidRangeCreate( &prm.RangeToLeft, ParamTotalPatchCount, GUID_USER_EVENT_STICKY );
ocrGuidRangeCreate( &prm.RangeToRight, ParamTotalPatchCount, GUID_USER_EVENT_STICKY );

    u64 &i = prm.patchNum;


    for( i = 0; i < ParamTotalPatchCount; i++ ) {

        u64 _idep = 0;

        ocrEdtCreate( &tempestInitGUID, tempestInitTML, EDT_PARAM_DEF, (u64 *)&prm, EDT_PARAM_DEF, NULL,
                        EDT_PROP_NONE, NULL_GUID, NULL );

        ocrDbCreate( &ModelDBK, (void **)&dummy, sizeof(Model), 0, NULL_GUID, NO_ALLOC );
        ocrAddDependence( ModelDBK, tempestInitGUID, _idep++, DB_MODE_RW );

        ocrDbCreate( &GridDBK, (void **)&dummy, sizeof(GridCartesianGLL), 0, NULL_GUID, NO_ALLOC );
        ocrAddDependence( GridDBK, tempestInitGUID, _idep++, DB_MODE_RW );

        ocrDbCreate( &TimestepSchemeDBK, (void **)&dummy, sizeof(TimestepSchemeStrang), 0, NULL_GUID, NO_ALLOC );
        ocrAddDependence( TimestepSchemeDBK, tempestInitGUID, _idep++, DB_MODE_RW );

        ocrDbCreate( &HorizontalDynamicsDBK, (void **)&dummy, sizeof(HorizontalDynamicsStub), 0, NULL_GUID, NO_ALLOC );
        ocrAddDependence( HorizontalDynamicsDBK, tempestInitGUID, _idep++, DB_MODE_RW );

        ocrDbCreate( &VerticalDynamicsDBK, (void **)&dummy, sizeof(VerticalDynamicsStub), 0, NULL_GUID, NO_ALLOC );
        ocrAddDependence( VerticalDynamicsDBK, tempestInitGUID, _idep, DB_MODE_RW );

    }

return NULL_GUID;

}

extern "C" ocrGuid_t mainEdt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] ) {

    /*
     * We need realMainEdt for wrapupEdt.
     */

    PRINTF("Hello, from TaskTestOCR!\n");

    ocrGuid_t tempestInitTML, tempestInitGUID;
    ocrGuid_t modelDBK, gridDBK;
    ocrGuid_t wrapupTML, wrapupGUID, finishEVT;
    ocrGuid_t realMainTML, realMainGUID;

    /*double * dummy;

    u8 errno;

    errno = ocrDbCreate( &modelDBK, (void **)&dummy, sizeof(Model), 0, NULL_GUID, NO_ALLOC );

    PRINTF("guid = %lx\n", modelDBK );

    errno = ocrDbCreate( &gridDBK, (void **)&dummy, sizeof(GridCartesianGLL), 0, NULL_GUID, NO_ALLOC );

    PRINTF("guid = %lx\n", gridDBK );

    ocrEdtTemplateCreate( &tempestInitTML, tempestInitEdt, 0, 2 );

    ocrEdtCreate( &tempestInitGUID, tempestInitTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                    EDT_PROP_FINISH, NULL_GUID, &finishEVT );

    ocrEdtTemplateCreate( &wrapupTML, wrapupEdt, 0, 1 );

    ocrEdtCreate( &wrapupGUID, wrapupTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, &finishEVT,
                    EDT_PROP_NONE, NULL_GUID, NULL );

    ocrDbRelease( modelDBK );
    ocrAddDependence( modelDBK, tempestInitGUID, 0, DB_MODE_RW );
    ocrDbRelease( gridDBK );
    ocrAddDependence( gridDBK, tempestInitGUID, 1, DB_MODE_RW );
    */
    ocrEdtTemplateCreate( &realMainTML, realMainEdt, 0, 0 );

    ocrEdtCreate( &realMainGUID, realMainTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH,
                    NULL_GUID, &finishEVT );

    ocrEdtTemplateCreate( &wrapupTML, wrapupEdt, 0, 1 );

    ocrEdtCreate( &wrapupGUID, wrapupTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, &finishEVT,
                    EDT_PROP_NONE, NULL_GUID, NULL );
    return NULL_GUID;
}

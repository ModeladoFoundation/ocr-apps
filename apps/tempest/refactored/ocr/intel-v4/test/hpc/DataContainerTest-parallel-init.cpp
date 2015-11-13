///////////////////////////////////////////////////////////////////////////////
///
///	\file    DataContainerTest.cpp
///	\author  Paul Ullrich, Jorge Guerra
///	\version December 18, 2013
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

#include "Tempest.h"
#include "GridPatchCartesianGLL.h"

#include "ocr.h"
#include "ocr-std.h"
#include <extensions/ocr-labeling.h>
#define DEFAULT_LG_PROPS GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG

typedef struct
{
    ocrEdt_t FNC;
    ocrGuid_t TML;
    ocrGuid_t EDT;
    ocrGuid_t OET;
} TempestOcrTask_t;

typedef struct
{
    u64 rank;
    u64 lenGeometric;
    u64 lenActive;
    u64 lenBuffer;
    u64 lenAux;
    u64 thisStep;
    u64 nSteps;
    u64 nRanks;
    ocrEdt_t FNC;
    ocrGuid_t TML;
    ocrGuid_t EDT;
    ocrGuid_t OET;
    ocrGuid_t DONE;
    ocrGuid_t RANGE;
    ocrGuid_t SEND_GUID[4];
    ocrGuid_t RECV_GUID[4];
} updateStateInfo_t;

ocrGuid_t updatePatch(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    int i, rank, len, dlen, step, maxStep, nRanks;

    double * localDataGeometric = (double *) depv[1].ptr;
    double * haloLeft = (double *) depv[2].ptr;
    double * haloRight = (double *) depv[3].ptr;
    updateStateInfo_t * stateInfo = (updateStateInfo_t *) depv[0].ptr;

    rank = stateInfo->rank;
    len = stateInfo->lenGeometric;
    step = stateInfo->thisStep;
    maxStep = stateInfo->nSteps;
    nRanks = stateInfo->nRanks;

    u64 phase = step%2;

    // TODO: Create a single guid with list of neighbor guids; double buffer events for each worker

    dlen = len/8;
    if (len == 0) {
        PRINTF ("ERROR!\n");
        return 1;
    }
    PRINTF ("Rank %d in updatePatch will update %d bytes! (%d)\n", rank, len, dlen);
    double *newData;
    newData = (double *) localDataGeometric;

    // Get a pointer to the global grid: NOTE this is illegal OCR; it is used to demonstrate the need for globally accessible RO object;

    Grid * pGrid =  (Grid *) paramv [0];
    //PRINTF("GJDEBUG: rank= %d grid pointer in updatePatch %lx\n", rank, pGrid);

    // activate the patch
    GridPatch * pPatch = pGrid->ActivateEmptyPatch(rank);


    // Get DataContainers associated with GridPatch

    DataContainer & dataGeometric = pPatch->GetDataContainerGeometric();

    unsigned char * pDataGeometric = (unsigned char*)(depv[1].ptr);



    if (step > 0) {
        if (rank != 0) ocrEventDestroy(stateInfo->RECV_GUID[!phase]);
        if (rank != nRanks-1) ocrEventDestroy(stateInfo->RECV_GUID[!phase+2]);
    }

    if (step == 0) {
	//Initialize data to zero
	memset(localDataGeometric, 0, len);
        //memset(pDataActiveState, 0, dataActiveState.GetTotalByteSize());
	//memset(pDataBufferState, 0, dataBufferState.GetTotalByteSize());
	//memset(pDataAuxiliary, 0, dataAuxiliary.GetTotalByteSize());
        haloLeft [0] = 0.;
        haloRight [0] = 0.;
        if (rank < nRanks - 1) {
            // send right double buffered
            ocrGuidFromIndex(&stateInfo->SEND_GUID[0], stateInfo->RANGE, 4*rank);
            ocrGuidFromIndex(&stateInfo->SEND_GUID[1], stateInfo->RANGE, 4*rank+1);
            // recv right double buffered
            ocrGuidFromIndex(&stateInfo->RECV_GUID[2], stateInfo->RANGE, 4*(rank+1)+2);
            ocrGuidFromIndex(&stateInfo->RECV_GUID[3], stateInfo->RANGE, 4*(rank+1)+3);
        } else {
            stateInfo->SEND_GUID[0] = NULL_GUID;
            stateInfo->SEND_GUID[1] = NULL_GUID;
            stateInfo->RECV_GUID[2] = NULL_GUID;
            stateInfo->RECV_GUID[3] = NULL_GUID;
        }
        if (rank > 0) {
            // send left double buffered
            ocrGuidFromIndex(&stateInfo->SEND_GUID[2], stateInfo->RANGE, 4*rank+2);
            ocrGuidFromIndex(&stateInfo->SEND_GUID[3], stateInfo->RANGE, 4*rank+3);
            // recv left double buffered
            ocrGuidFromIndex(&stateInfo->RECV_GUID[0], stateInfo->RANGE, 4*(rank-1)+0);
            ocrGuidFromIndex(&stateInfo->RECV_GUID[1], stateInfo->RANGE, 4*(rank-1)+1);
        } else {
            stateInfo->SEND_GUID[2] = NULL_GUID;
            stateInfo->SEND_GUID[3] = NULL_GUID;
            stateInfo->RECV_GUID[0] = NULL_GUID;
            stateInfo->RECV_GUID[1] = NULL_GUID;
        }
    }


// Proof of concept to update data in a GridPatch
    // unpack halo data from dependences

    if (rank > 0 ) {
        newData [0] = haloLeft[0];
    } else {
        if (step == 0) {
            newData [0] = 0.;
        } else {
            newData [0] = (double) (rank-1);
        }
    }
    if (rank < nRanks-1) {
        newData [dlen] = haloRight[0];
    } else {
        if (step == 0) {
            newData [dlen] = 0.;
        } else {
            newData [dlen] = (double) (rank+1);
        }
    }
    PRINTF("GJDEBUG1: rank = %d, step = %d left halo = %f, right halo = %f update = %f\n", rank, step, newData[0], newData[dlen], 0.5 * (newData [0] + newData[dlen]));
    double left, right;
    left = newData[0];
    right = newData[dlen];
    // update data
    for (i=0; i<dlen; i++) {
        //newData  = newData  + 0.5 * (newData [i-1] + newData[i+1]);
        newData [i]  = newData [i] + 0.5 * (left + right);
    }

    //
    //deactivate the GridPatch
    PRINTF ("Rank %d deactivates its patch in step %d \n", rank, step);
    pGrid->DeactivatePatch(rank);

    // pack data into updated halo data block
    //if (rank > 0) haloLeft [0] = newData [0];
    if (rank > 0) haloLeft [0] = (double) rank;
    //if (rank < nRanks-1) haloRight [0] = newData [dlen-1];
    if (rank < nRanks-1) haloRight [0] = (double) rank;


// create clone for next timestep or trigger output
    if (step < maxStep) {
        stateInfo->thisStep++;
        phase=step%2;
        // change to local guid for EDT
        ocrEdtCreate(&stateInfo->EDT, stateInfo->TML, EDT_PARAM_DEF, paramv,
                      EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        // Create neighbor events for next steps
        //
        if (rank != 0) ocrEventCreate(&stateInfo->RECV_GUID[phase],
                                      OCR_EVENT_STICKY_T,DEFAULT_LG_PROPS);
        ocrAddDependence(stateInfo->RECV_GUID[phase], stateInfo->EDT, 2, DB_MODE_RW );
        if (rank != nRanks-1) ocrEventCreate(&stateInfo->RECV_GUID[phase+2],
                                             OCR_EVENT_STICKY_T,DEFAULT_LG_PROPS);
        ocrAddDependence(stateInfo->RECV_GUID[phase+2], stateInfo->EDT, 3, DB_MODE_RW );

        if (rank != 0) {
            ocrEventCreate(&stateInfo->SEND_GUID[phase+2],
                           OCR_EVENT_STICKY_T,DEFAULT_LG_PROPS);
            ocrDbRelease(depv[2].guid);
            ocrEventSatisfy(stateInfo->SEND_GUID[phase+2], depv[2].guid);
        }

        if (rank != nRanks-1) {
            ocrEventCreate(&stateInfo->SEND_GUID[phase+0],
                           OCR_EVENT_STICKY_T,DEFAULT_LG_PROPS);
            ocrDbRelease(depv[3].guid);
            ocrEventSatisfy(stateInfo->SEND_GUID[phase+0], depv[3].guid);
        }

        // release data blocks and add dependences for next step
        ocrDbRelease(depv[0].guid);
        ocrAddDependence(depv[0].guid, stateInfo->EDT, 0, DB_MODE_RW );
        ocrDbRelease(depv[1].guid);
        ocrAddDependence(depv[1].guid, stateInfo->EDT, 1, DB_MODE_RW );

    } else {
        // TODO:  Release the data block which has been updated and is printed by outputEDT; important for x86-mpi;
        ocrDbRelease (depv[0].guid);
        ocrDbRelease (depv[1].guid);
        ocrEventSatisfy (stateInfo->DONE, NULL_GUID);
        PRINTF ("Good-by from Rank %d in updatePatch\n", rank);
    }


    fflush (stdout);
    return NULL_GUID;
}

ocrGuid_t outputEdt (u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    int nWorkers = (int) paramv [0];
    int lastStep = (int) paramv[1];
    int i, j;
    for (i = 0; i<nWorkers; i++) {
        double* newData = (double *) depv[i].ptr;
        PRINTF ("Output of rank %d\n", i);
        for (j = 0; j<10; j++) {
            PRINTF( "%f ", newData [j]);
        }
        if (newData [5] ==  (i * lastStep)) {
          PRINTF ("\n\n");
          PRINTF ("Test passed!\n");
        } else {
          PRINTF ("\n\n");
          PRINTF ("Test FAILED! data [5] = %f should be %f\n", newData [5],  (double) (i + i * lastStep));
        }
        PRINTF ("\n\n");
    }
    PRINTF ("Good-by from outputEdt!\n");
    PRINTF ("***********************\n");
    fflush (stdout);
    ocrShutdown ();
    return NULL_GUID;
}

ocrGuid_t initializeStep (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("Hello from initilaizeStepEdt!\n");
    fflush (stdout);

    try {
        TempestOcrTask_t updatePatch_t;
        int nWorkers = paramv [0];
        int nSteps = paramv [1];
        int rank = paramv [2];
        int nHorizontalOrder = paramv [3];
        int nVerticalOrder = paramv [4];
        GridCartesianGLL * pGrid =  (GridCartesianGLL *) paramv [8];

	//////////////////////////////////////////////////////////////////
	// BEGIN MAIN PROGRAM BLOCK
        //PRINTF("GJDEBUG: grid pointer in mainEdt %lx\n", pGrid);

	// Create a GridPatch
        GridPatch *pPatchFirst;
       	const PatchBox & box = pGrid->GetPatchBox (rank);
	pPatchFirst =
	    new GridPatchCartesianGLL(
			(*pGrid),
			0,
			box,
			nHorizontalOrder,
			nVerticalOrder);

	// Build the DataContainer object for the GridPatch objects

        ocrGuid_t dataGuidGeometric;
        ocrGuid_t dataGuidActiveState;
        ocrGuid_t dataGuidBufferState;
        ocrGuid_t dataGuidAuxiliary;
        ocrGuid_t dataGuidState;
        ocrGuid_t dataGuidHaloLeft;
        ocrGuid_t dataGuidHaloRight;
        ocrGuid_t dataGuidGrid [1];

        ocrGuid_t updateEdtTemplate;
        ocrGuid_t outputEventGuid;
        ocrGuid_t updatePatch_DONE;
        updateStateInfo_t stateInfo;

        u64 output_paramv [2];
        output_paramv [0] = (u64) nWorkers;
        output_paramv [1] = (u64) nSteps;

        //
        PRINTF("GJDEBUG: %d active patches for rank = %d \n", pGrid->GetActivePatchCount(), rank);
        fflush (stdout);
        PRINTF("GJDEBUG: done with initializeStep rank = %d\n", rank);
        fflush (stdout);
        pPatchFirst->InitializeDataLocal(false, false, false, false);
	    // Get DataContainers associated with GridPatch
	DataContainer & dataGeometric = pPatchFirst->GetDataContainerGeometric();
	DataContainer & dataActiveState = pPatchFirst->GetDataContainerActiveState();
	DataContainer & dataBufferState = pPatchFirst->GetDataContainerBufferState();
	DataContainer & dataAuxiliary = pPatchFirst->GetDataContainerAuxiliary();

	// Output the size requirements (in bytes) of each DataContainer
	std::cout << "GridPatch.Geometric Size:   " << dataGeometric.GetTotalByteSize() << " bytes" << std::endl;
	std::cout << "GridPatch.ActiveState Size: " << dataActiveState.GetTotalByteSize() << " bytes" << std::endl;
	std::cout << "GridPatch.BufferState Size: " << dataBufferState.GetTotalByteSize() << " bytes" << std::endl;
	std::cout << "GridPatch.Auxiliary Size:   " << dataAuxiliary.GetTotalByteSize() << " bytes" << std::endl;

	// Allocate data
	std::cout << "Allocating data ... " << std::endl;
        unsigned char * pDataGeometric;
        unsigned char * pDataActiveState;
        unsigned char * pDataBufferState;
        unsigned char * pDataAuxiliary;
        unsigned char * pHaloLeft;
        unsigned char * pHaloRight;
        updateStateInfo_t * pStateInfo;
        updatePatch_t.FNC = updatePatch;

        ocrDbCreate (&dataGuidState, (void **)&pStateInfo, (u64) (sizeof(updateStateInfo_t)), 0, NULL_GUID, NO_ALLOC);
        ocrDbCreate (&dataGuidGeometric, (void **)&pDataGeometric, (u64) dataGeometric.GetTotalByteSize(), 0, NULL_GUID, NO_ALLOC);
        //
        // Create Halo-Data
        ocrDbCreate (&dataGuidHaloLeft, (void **)&pHaloLeft, sizeof (double), 0, NULL_GUID, NO_ALLOC);
        ocrDbCreate (&dataGuidHaloRight, (void **)&pHaloRight, sizeof (double), 0, NULL_GUID, NO_ALLOC);

        // These containers are currently not being used for the update since we are only performing a proof of concept update

        ocrDbCreate (&dataGuidActiveState, (void **)&pDataActiveState, (u64) dataActiveState.GetTotalByteSize(), 0, NULL_GUID, NO_ALLOC);
        ocrDbCreate (&dataGuidBufferState, (void **)&pDataBufferState, (u64) dataBufferState.GetTotalByteSize(), 0, NULL_GUID, NO_ALLOC);
        ocrDbCreate (&dataGuidAuxiliary, (void **)&pDataAuxiliary, (u64) dataAuxiliary.GetTotalByteSize(), 0, NULL_GUID, NO_ALLOC);

        // Each updatePatch EDT gets stateInfo: rank, sizes, time step information, information how to create next Edt
        int thisStep = 0;
        pStateInfo->rank = rank;
        pStateInfo->lenGeometric = (u64) dataGeometric.GetTotalByteSize();
        pStateInfo->lenActive = (u64) dataActiveState.GetTotalByteSize();
        pStateInfo->lenBuffer = (u64) dataBufferState.GetTotalByteSize();
        pStateInfo->lenAux = (u64) dataAuxiliary.GetTotalByteSize();
        pStateInfo->thisStep = (u64) thisStep;
        pStateInfo->nSteps = (u64) nSteps;
        pStateInfo->nRanks = (u64) nWorkers;
        pStateInfo->EDT =  updatePatch_t.EDT;
        pStateInfo->TML =  paramv [5];
        pStateInfo->FNC =  updatePatch_t.FNC;

        ocrEventCreate( &updatePatch_DONE, OCR_EVENT_STICKY_T, false );
        pStateInfo->DONE =  updatePatch_DONE;

        // Add pointer to global grid as parameter to update EDT:
        // NOTE: is currently illegal OCR. It will break on x86-mpi. It is a temporary workaround to demonstrate the need for
        // worker thread local globally accessible objects

        u64 update_paramv [1] = {(u64) pGrid};
        ocrEdtCreate(&updatePatch_t.EDT, paramv [6], EDT_PARAM_DEF, update_paramv,
                              EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        // add data dependences for updatePatch EDT

        s32 _idep;
        _idep = 0;
        ocrAddDependence(dataGuidState, updatePatch_t.EDT, _idep++, DB_MODE_RW );
        ocrAddDependence(dataGuidGeometric, updatePatch_t.EDT, _idep++, DB_MODE_RW );
        ocrAddDependence(dataGuidHaloLeft, updatePatch_t.EDT, _idep++, DB_MODE_RW );
        ocrAddDependence(dataGuidHaloRight, updatePatch_t.EDT, _idep++, DB_MODE_RW );

        // chain output EDT with the updatePatch EDTs
        _idep = rank;
        ocrAddDependence( dataGuidGeometric , paramv [6], _idep, DB_MODE_RW );
        ocrAddDependence( updatePatch_DONE , paramv [6], _idep+rank, DB_MODE_NULL );
    } catch(Exception & e) {
	std::cout << e.ToString() << std::endl;
    }

	// Deinitialize Tempest
}

ocrGuid_t initializeEdt (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[])
{
	std::cout << "Initializing Model and Grid ... " << std::endl;
    try {

	// Model parameters
	const int nResolutionX = 12;
	const int nResolutionY = 10;
	const int nHorizontalOrder = 4;
	const int nVerticalOrder = 4;
	const int nLevels = 40;
        const int nWorkers = paramv [0];
        const int nSteps = paramv [1];
        const int rank = paramv [1];
        PRINTF("initilaizeEdt nWorkers = %d nSteps = %d!\n", nWorkers, nSteps);
        fflush (stdout);

	double dGDim[6];
	dGDim[0] = 0.0;
	dGDim[1] = 1000.0;
	dGDim[2] = -1000.0;
	dGDim[3] = 1000.0;
	dGDim[4] = 0.0;
	dGDim[5] = 1000.0;

	const double dRefLat = 0.0;
	const double dTopoHeight = 0.0;

        TempestOcrTask_t updatePatch_t;
        TempestOcrTask_t output_t;
        TempestOcrTask_t initializeStep_t;

        Grid::VerticalStaggering eVerticalStaggering =
		Grid::VerticalStaggering_Levels;

	// Setup the Model
	Model model(EquationSet::PrimitiveNonhydrostaticEquations);

	// Set the parameters
	ModelParameters param;

	model.SetParameters(param);

	model.SetTimestepScheme(new TimestepSchemeStrang(model));

	model.SetHorizontalDynamics(
		new HorizontalDynamicsStub(model));

	model.SetVerticalDynamics(
		new VerticalDynamicsStub(model));

        // Set the model grid (one Cartesian grid per rank)
        GridCartesianGLL * pGrid [nWorkers];
        for (int i =0; i<nWorkers; i++) {
	     pGrid [i] =
	        	new GridCartesianGLL(
	        		model,
                                nWorkers,
			        nResolutionX,
			        nResolutionY,
			        4,
			        nHorizontalOrder,
			        nVerticalOrder,
			        nLevels,
			        dGDim,
			        dRefLat,
			        eVerticalStaggering);
        }
        // Apply the default patch layout
        for (int i = 0; i<nWorkers; i++) {
             pGrid[i]->ApplyDefaultPatchLayout(nWorkers);
        }
            //
        // create a range of labled Guids
        //
        int nNeighbors = 2;
        ocrGuid_t GuidRange;
        ocrGuidRangeCreate( &(GuidRange), 2 * nNeighbors * nWorkers, GUID_USER_EVENT_STICKY);

        // Create Template for output Edt
        // Parameters:
        //      # workers
        //      # steps
        // Dependences:
        //      # workers * updated data blocks
        //      # workers * DONE events
        output_t.FNC = outputEdt;
        initializeStep_t.FNC = initializeStep;
        //initializeStep_t.FNC = outputEdt;
        ocrEdtTemplateCreate(&output_t.TML, output_t.FNC, 2, 2*nWorkers);
        u64 output_paramv [2];
        output_paramv [0] = (u64) nWorkers;
        output_paramv [1] = (u64) nSteps;

        /// start the output Edt
        ocrEdtCreate(&output_t.EDT, output_t.TML, EDT_PARAM_DEF, output_paramv, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL );

        // Create Template for initializeStep EDT
        // Parameters:
        //           # workers
        //           # steps
        //           # rank
        //           nHorizontalOrder
        //           nVerticalOrder
        //           GUID for TML updatePatch
        //           GUID for EDT outputPatch
        //           GUID for GUID_RANGE
        //           GUID for global Grid NOOOOOOOOT
        // Dependences: none
        u64 init_paramv [9];

        ocrEdtTemplateCreate(&initializeStep_t.TML, initializeStep_t.FNC, 9, 0);

        // Create Template for updatePatch EDT
        // Parameters:
        //           pointer to Global Grid NOOOOOOOOT!
        // Dependences:
        //           stateInfo
        //           patch
        //           haloLeft
        //           haloRight
        ocrEdtTemplateCreate(&updatePatch_t.TML, updatePatch_t.FNC, 1, 4);
        init_paramv [0] = nWorkers;
        init_paramv [1] = nSteps;
        init_paramv [3] = nHorizontalOrder;
        init_paramv [4] = nVerticalOrder;
        init_paramv [5] = updatePatch_t.TML;
        init_paramv [6] = output_t.EDT;
        init_paramv [7] = GuidRange;


        for (int i = 0; i<nWorkers; i++) {
            init_paramv [2] = i;
            init_paramv [8] = (u64) pGrid [i]; ///NOOOOooooooOOOOooooOoooooo.... but can't be helped at the moment
            ocrEdtCreate(&initializeStep_t.EDT, initializeStep_t.TML, EDT_PARAM_DEF, init_paramv, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL );
            PRINTF("initilaizeEdt start initializeStep for rank = %d!\n", i);
            fflush (stdout);
        }

    } catch(Exception & e) {
	std::cout << e.ToString() << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////

extern "C" ocrGuid_t mainEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("Hello from DataContainerTest-OCR!\n");
    fflush (stdout);
    void *programArgv = depv[0].ptr;
    u32 argc=getArgc(programArgv);
    PRINTF("argc = %d\n", argc);
    ocrGuid_t initialize;
    TempestOcrTask_t initialize_t;
    u64 initialize_paramv [2];
    u32 nWorkers;
    u32 nSteps;
    if (argc != 3) {
        nWorkers = 4;
        nSteps = 4;
    } else {
        u32 i = 1;
        nWorkers = (u32) atoi(getArgv(programArgv, i++));
        nSteps = (u32) atoi(getArgv(programArgv, i++));
    }
    PRINTF("Using %d workers, %d steps.\n", nWorkers, nSteps);
    fflush (stdout);

    // Create the InitializationTask
    initialize_paramv [0] = nWorkers;
    initialize_paramv [1] = nSteps;
    initialize_t.FNC = initializeEdt;
    ocrEdtTemplateCreate(&initialize_t.TML, initialize_t.FNC, 2, 0);
    ocrEdtCreate(&initialize_t.EDT, initialize_t.TML, EDT_PARAM_DEF, initialize_paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    PRINTF("mainEdt started Initialize\n");
    fflush (stdout);
    return NULL_GUID;
}

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

//#include <mpi.h>
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
    double *newData, *newHaloLeft, *newHaloRight;
    newData = (double *) localDataGeometric;
    newHaloLeft = (double *) haloLeft;
    newHaloRight = (double *) haloRight;

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
        if (rank != 0) PRINTF ("GJDEBUG: rank = %d step = %d halo left = %f \n", rank, step,  newHaloLeft[0]);
        if (rank != nRanks-1) PRINTF ("GJDEBUG: rank = %d step = %d halo right = %f \n", rank, step, newHaloRight[0]);
    }

    if (step == 0) {
        newHaloLeft [0] = 1.;
        newHaloRight [0] = 1.;
        if (rank == 0) newHaloLeft [0] = 1.;
        if (rank == (nRanks-1)) newHaloRight [0] = 1.0;
        PRINTF ("GJDEBUG: rank = %d step = %d halo left = %f \n", rank, step,  newHaloLeft[0]);
        PRINTF ("GJDEBUG: rank = %d step = %d halo right = %f \n", rank, step, newHaloRight[0]);
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
        newData [0] = newHaloLeft[0];
    } else {
        newData [0] = 1.;
    }
    if (rank < nRanks-1) {
        newData [dlen] = newHaloRight[0];
    } else {
        newData [dlen] = 1.;
    }
    for (i=1; i<dlen-1; i++) {
        newData [i] = newData [i] + 0.5 * (newData [i-1] + newData[i+1]);
    }
    //newData [i] = newData [i] + 0.5 * hLeft;

    //
    //deactivate the GridPatch
    PRINTF ("Rank %d deactivates its patch in step %d \n", rank, step);
    pGrid->DeactivatePatch(rank);

    // pack data into dependences
    if (rank > 0) newHaloLeft [0] = newData [1];
    if (rank < nRanks-1) newHaloRight [0] = newData [dlen-1];


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
    char * localDataGeometric = (char *) depv[0].ptr;
    int nWorkers = (int) paramv [0];
    int lastStep = (int) paramv[1];
    double sum = 0.0;
    double* newData = (double *) localDataGeometric;
    int i, j;
    for (i = 0; i<nWorkers; i++) {
        double* newData = (double *) depv[i].ptr;
        PRINTF ("Output of rank %d\n", i);
        for (j = 0; j<10; j++) {
            PRINTF( "%f ", newData [j]);
        }
        if (newData [5] ==  (i + i * lastStep)) {
          PRINTF ("\n\n");
          PRINTF ("Test passed!\n");
        } else {
          PRINTF ("\n\n");
          PRINTF ("Test FAILED! %f\n", sum);
        }
        PRINTF ("\n\n");
    }
    PRINTF ("Good-by from outputEdt!\n");
    PRINTF ("***********************\n");
    fflush (stdout);
    ocrShutdown ();
    return NULL_GUID;
}

ocrGuid_t initializeEdt (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[])
{
    PRINTF("Hello from initilaizeEdt!\n");
    fflush (stdout);
    try {
	std::cout << "Initializing Model and Grid ... " << std::endl;

	// Model parameters
	const int nResolutionX = 12;
	const int nResolutionY = 10;
	const int nHorizontalOrder = 4;
	const int nVerticalOrder = 4;
	const int nLevels = 40;
        const int nWorkers = paramv [0];
        const int nSteps = paramv [1];
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

        //
        // create a range of labled Guids
        //
        int nNeighbors = 2;
        ocrGuid_t GuidRange;
        ocrGuidRangeCreate( &(GuidRange), 2 * nNeighbors * nWorkers, GUID_USER_EVENT_STICKY);
    PRINTF("initilaizeEdt after Model Setup\n");
    fflush (stdout);
	// Set the model grid (one patch Cartesian grid)
        GridCartesianGLL * pGrid [nWorkers];
        for (int i = 0; i<nWorkers; i++) {
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
    PRINTF("initilaizeEdt after Grid Setup\n");
    fflush (stdout);

	// Apply the default patch layout
        for (int i = 0; i<nWorkers; i++) {
	    pGrid[i]->ApplyDefaultPatchLayout(nWorkers);
        }

	//////////////////////////////////////////////////////////////////
	// BEGIN MAIN PROGRAM BLOCK
        //PRINTF("GJDEBUG: grid pointer in mainEdt %lx\n", pGrid);

	// Create a GridPatch
        GridPatch *pPatchFirst [nWorkers];
        for (int i = 0; i<nWorkers; i++) {
       	    const PatchBox & box = pGrid[i]->GetPatchBox(i);
	    pPatchFirst [i] =
	    new GridPatchCartesianGLL(
			(*pGrid [i]),
			0,
			box,
			nHorizontalOrder,
			nVerticalOrder);
        }

	// Build the DataContainer object for the GridPatch objects

        ocrGuid_t dataGuidGeometric [nWorkers];
        ocrGuid_t dataGuidActiveState [nWorkers];
        ocrGuid_t dataGuidBufferState [nWorkers];
        ocrGuid_t dataGuidAuxiliary [nWorkers];
        ocrGuid_t dataGuidState [nWorkers];
        ocrGuid_t dataGuidHaloLeft [nWorkers];
        ocrGuid_t dataGuidHaloRight [nWorkers];
        ocrGuid_t dataGuidGrid [1];

        ocrGuid_t updateEdtTemplate;
        ocrGuid_t outputEventGuid [nWorkers];
        ocrGuid_t updatePatch_DONE [nWorkers];
        updateStateInfo_t stateInfo [nWorkers];
        TempestOcrTask_t updatePatch_t [nWorkers];
        TempestOcrTask_t output_t;

        // Craete Template for output Edt
        // Parameters:
        //      # workers
        //      # steps
        // Dependences:
        //      # workers * updated data blocks
        //      # workers * DONE events
        output_t.FNC = outputEdt;
        ocrEdtTemplateCreate(&output_t.TML, output_t.FNC, 2, 2*nWorkers);
        u64 output_paramv [2];
        output_paramv [0] = (u64) nWorkers;
        output_paramv [1] = (u64) nSteps;

        // Create the output EDT:
        ocrEdtCreate(&output_t.EDT, output_t.TML, EDT_PARAM_DEF, output_paramv, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL );


        for (int i = 0; i<nWorkers; i++) {
	    pPatchFirst[i]->InitializeDataLocal(false, false, false, false);
	    // Get DataContainers associated with GridPatch
	    DataContainer & dataGeometric = pPatchFirst[i]->GetDataContainerGeometric();
	    DataContainer & dataActiveState = pPatchFirst[i]->GetDataContainerActiveState();
	    DataContainer & dataBufferState = pPatchFirst[i]->GetDataContainerBufferState();
	    DataContainer & dataAuxiliary = pPatchFirst[i]->GetDataContainerAuxiliary();

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
            updatePatch_t[i].FNC = updatePatch;

            // Craete Template for updatePatch Edt
            // Parameters:
            //       1 pointer to global grid
            // Dependences:2
            //      updated geometric data block
            //      state information
            //      TODO: Add dependences on "magic" neighbor events to handle the data exchange; 1 event per neighbor

            ocrEdtTemplateCreate(&updatePatch_t[i].TML, updatePatch_t[i].FNC, 1, 4);

            ocrDbCreate (&dataGuidState [i], (void **)&pStateInfo, (u64) (sizeof(updateStateInfo_t)), 0, NULL_GUID, NO_ALLOC);
            ocrDbCreate (&dataGuidGeometric[i], (void **)&pDataGeometric, (u64) dataGeometric.GetTotalByteSize(), 0, NULL_GUID, NO_ALLOC);
            //
            // Create Halo-Data
            ocrDbCreate (&dataGuidHaloLeft[i], (void **)&pHaloLeft, sizeof (double), 0, NULL_GUID, NO_ALLOC);
            ocrDbCreate (&dataGuidHaloRight[i], (void **)&pHaloRight, sizeof (double), 0, NULL_GUID, NO_ALLOC);

            // These containers are currently not being used for the update since we are only performing a proof of concept update

            ocrDbCreate (&dataGuidActiveState[i], (void **)&pDataActiveState, (u64) dataActiveState.GetTotalByteSize(), 0, NULL_GUID, NO_ALLOC);
            ocrDbCreate (&dataGuidBufferState[i], (void **)&pDataBufferState, (u64) dataBufferState.GetTotalByteSize(), 0, NULL_GUID, NO_ALLOC);
            ocrDbCreate (&dataGuidAuxiliary[i], (void **)&pDataAuxiliary, (u64) dataAuxiliary.GetTotalByteSize(), 0, NULL_GUID, NO_ALLOC);

	    // Initialize data to zero
	    memset(pDataGeometric, 0, dataGeometric.GetTotalByteSize());
	    memset(pDataActiveState, 0, dataActiveState.GetTotalByteSize());
	    memset(pDataBufferState, 0, dataBufferState.GetTotalByteSize());
	    memset(pDataAuxiliary, 0, dataAuxiliary.GetTotalByteSize());

            // Each updatePatch EDT gets stateInfo: rank, sizes, time step information, information how to create next Edt
            int thisStep = 0;
            pStateInfo->rank = i;
            pStateInfo->lenGeometric = (u64) dataGeometric.GetTotalByteSize();
            pStateInfo->lenActive = (u64) dataActiveState.GetTotalByteSize();
            pStateInfo->lenBuffer = (u64) dataBufferState.GetTotalByteSize();
            pStateInfo->lenAux = (u64) dataAuxiliary.GetTotalByteSize();
            pStateInfo->thisStep = (u64) thisStep;
            pStateInfo->nSteps = (u64) nSteps;
            pStateInfo->nRanks = (u64) nWorkers;
            pStateInfo->EDT =  updatePatch_t[i].EDT;
            pStateInfo->TML =  updatePatch_t[i].TML;
            pStateInfo->FNC =  updatePatch_t[i].FNC;
            pStateInfo->RANGE = GuidRange;


            ocrEventCreate( &updatePatch_DONE [i], OCR_EVENT_STICKY_T, false );
            pStateInfo->DONE =  updatePatch_DONE [i];

        // Add pointer to global grid as parameter to update EDT:
        // NOTE: is currently illegal OCR. It will break on x86-mpi. It is a temporary workaround to demonstrate the need for
        // worker thread local globally accessible objects

            u64 update_paramv [1] = {(u64) pGrid [i] };
            ocrEdtCreate(&updatePatch_t[i].EDT, updatePatch_t[i].TML, EDT_PARAM_DEF, update_paramv,
                              EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        }
        // add data dependences for updatePatch EDT
        //
        s32 _idep;
        for (int i=0; i<nWorkers; i++) {
            _idep = 0;
            ocrAddDependence(dataGuidState[i], updatePatch_t[i].EDT, _idep++, DB_MODE_RW );
            ocrAddDependence(dataGuidGeometric[i], updatePatch_t[i].EDT, _idep++, DB_MODE_RW );
            ocrAddDependence(dataGuidHaloLeft[i], updatePatch_t[i].EDT, _idep++, DB_MODE_RW );
            ocrAddDependence(dataGuidHaloRight[i], updatePatch_t[i].EDT, _idep++, DB_MODE_RW );
            // This data is currently not being used, since our update is just a proof of concept
            //ocrAddDependence(dataGuidActiveState[i], updatePatch_t[i].EDT, _idep++, DB_MODE_RW );
            //ocrAddDependence(dataGuidBufferState[i], updatePatch_t[i].EDT, _idep++, DB_MODE_RW );
            //ocrAddDependence(dataGuidAuxiliary[i], updatePatch_t[i].EDT, _idep++, DB_MODE_RW );
        }

        _idep = 0;
        for (int i=0; i<nWorkers; i++) {
            ocrAddDependence( dataGuidGeometric [i], output_t.EDT, _idep++, DB_MODE_RW );
        }
        // chain output EDT with all of the updatePatch EDTs
        for (int i=0; i<nWorkers; i++) {
            ocrAddDependence( updatePatch_DONE [i], output_t.EDT, _idep++, DB_MODE_NULL );
        }


    } catch(Exception & e) {
	std::cout << e.ToString() << std::endl;
    }

	// Deinitialize Tempest
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

#include <ocxxr-main.hpp>
#include <iostream>
#include <cstdlib>
#include <Tempest.h>
#include <GridPatchCartesianGLL.h>

using namespace ocxxr;
using namespace std;


struct initParams{
    u32 nWorkers;
    u32 nSteps;
};

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

void initializeEdt( initParams &params )
{
   cout << "hello, from initializeEdt!" << endl;

   cout <<"Initializing Model and Grid..." << endl;

    const int nResolutionX = 12;
    const int nResolutionY = 10;
    const int nHorizontalOrder = 4;
	const int nVerticalOrder = 4;
	const int nLevels = 40;
        const int nWorkers = params.nWorkers;
        const int nSteps = params.nSteps;

    cout << "initializeEdt nWorkers = " << params.nWorkers << " nSteps = " << params.nSteps << "!" << endl;

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
    cout << "initilaizeEdt after Model Setup" << endl;


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
    cout << "initilaizeEdt after Grid Setup" << endl;

	// Apply the default patch layout
        for (int i = 0; i<nWorkers; i++) {
	    pGrid[i]->ApplyDefaultPatchLayout(nWorkers);
        }

	//////////////////////////////////////////////////////////////////
	// BEGIN MAIN PROGRAM BLOCK

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

        /*ocrGuid_t dataGuidGeometric [nWorkers];
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
*/

    for( int i = 0; i < nWorkers; i++ )
    {
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
            auto pDataGeometric = ocxxr::Datablock<unsigned char>::Create(dataGeometric.GetTotalByteSize());
            auto pDataActiveState = ocxxr::Datablock<unsigned char>::Create(dataActiveState.GetTotalByteSize());
            auto pDataBufferState = ocxxr::Datablock<unsigned char>::Create(dataBufferState.GetTotalByteSize());
            auto pDataAuxiliary = ocxxr::Datablock<unsigned char>::Create(dataAuxiliary.GetTotalByteSize());
            auto pHaloLeft = ocxxr::Datablock<double>::Create();
            auto pHaloRight = ocxxr::Datablock<double>::Create();
            auto pStateInfo = ocxxr::Datablock<updateStateInfo_t>::Create();
            //updatePatch_t[i].FNC = updatePatch;

            // Craete Template for updatePatch Edt
            // Parameters:
            //       1 pointer to global grid
            // Dependences:2
            //      updated geometric data block
            //      state information

            //ocrEdtTemplateCreate(&updatePatch_t[i].TML, updatePatch_t[i].FNC, 1, 4);

            //ocrDbCreate (&dataGuidState [i], (void **)&pStateInfo, (u64) (sizeof(updateStateInfo_t)), 0, NULL_GUID, NO_ALLOC);
            //ocrDbCreate (&dataGuidGeometric[i], (void **)&pDataGeometric, (u64) dataGeometric.GetTotalByteSize(), 0, NULL_GUID, NO_ALLOC);
            //
            // Create Halo-Data
            //ocrDbCreate (&dataGuidHaloLeft[i], (void **)&pHaloLeft, sizeof (double), 0, NULL_GUID, NO_ALLOC);
            //ocrDbCreate (&dataGuidHaloRight[i], (void **)&pHaloRight, sizeof (double), 0, NULL_GUID, NO_ALLOC);

            // These containers are currently not being used for the update since we are only performing a proof of concept update

            //ocrDbCreate (&dataGuidActiveState[i], (void **)&pDataActiveState, (u64) dataActiveState.GetTotalByteSize(), 0, NULL_GUID, NO_ALLOC);
            //ocrDbCreate (&dataGuidBufferState[i], (void **)&pDataBufferState, (u64) dataBufferState.GetTotalByteSize(), 0, NULL_GUID, NO_ALLOC);
            //ocrDbCreate (&dataGuidAuxiliary[i], (void **)&pDataAuxiliary, (u64) dataAuxiliary.GetTotalByteSize(), 0, NULL_GUID, NO_ALLOC);

	    // Initialize data to zero
	    //memset(pDataGeometric, 0, dataGeometric.GetTotalByteSize());
	    //memset(pDataActiveState, 0, dataActiveState.GetTotalByteSize());
	    //memset(pDataBufferState, 0, dataBufferState.GetTotalByteSize());
	    //memset(pDataAuxiliary, 0, dataAuxiliary.GetTotalByteSize());

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
            //pStateInfo->EDT =  updatePatch_t[i].EDT;
            //pStateInfo->TML =  updatePatch_t[i].TML;
            //pStateInfo->FNC =  updatePatch_t[i].FNC;
            //pStateInfo->RANGE = GuidRange;


            //ocrEventCreate( &updatePatch_DONE [i], OCR_EVENT_STICKY_T, false );
            //pStateInfo->DONE =  updatePatch_DONE [i];

        // Add pointer to global grid as parameter to update EDT:
        // NOTE: is currently illegal OCR. It will break on x86-mpi. It is a temporary workaround to demonstrate the need for
        // worker thread local globally accessible objects

            //u64 update_paramv [1] = {(u64) pGrid [i] };
//            ocrEdtCreate(&updatePatch_t[i].EDT, updatePatch_t[i].TML, EDT_PARAM_DEF, update_paramv,
//                              EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        }

    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs> args)
{
    ocrPrintf("hello, world! ocxxr successfully included!\n");


    u32 nWorkers;
    u32 nSteps;
    if( args->argc() != 3 ){
        nWorkers = 4;
        nSteps = 1;
    }
    else
    {
        nWorkers = atoi(args->argv(1));
        nSteps = atoi(args->argv(2));
    }

    auto init_template = OCXXR_TEMPLATE_FOR(initializeEdt);

    initParams params = {nWorkers, nSteps};

    init_template().CreateTask(params);

}

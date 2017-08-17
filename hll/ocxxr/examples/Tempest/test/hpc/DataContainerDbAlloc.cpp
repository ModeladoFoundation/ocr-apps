///////////////////////////////////////////////////////////////////////////////
///
///    \file    DataContainerTest.cpp
///    \author  Paul Ullrich, Jorge Guerra
///    \version December 18, 2013
///
///    <remarks>
///        Copyright 2000-2010 Paul Ullrich
///
///        This file is distributed as part of the Tempest source code package.
///        Permission is granted to use, copy, modify and distribute this
///        source code and its documentation under the terms of the GNU General
///        Public License.  This software is provided "as is" without express
///        or implied warranty.
///    </remarks>

#include "GridPatchCartesianGLL.h"
#include "Tempest.h"
#include "ocxxr-main.hpp"

//#include <mpi.h>
//#include "ocr.h"
//#include "ocr-std.h"
//#include "ocr_relative_ptr.hpp"
//#include "ocr_db_alloc.hpp"
#include <cstring>

#define ARENA_SIZE (1 << 26)  // 64MB

#ifndef NDEBUG
#define DBG_PRINTF PRINTF
#define DBG_ONLY(x) (x)
#else
#define DBG_ONLY(x) \
    do {            \
        if (0) {    \
            (x);    \
        }           \
    } while (0)
#define DBG_PRINTF(...) DBG_ONLY(PRINTF(__VA_ARGS__))
#endif


std::map<u64, int> guidHandle;

struct MG {
    ocxxr::RelPtr<Model> m;
    ocxxr::RelPtr<GridCartesianGLL> g;
};

struct updateStateInfo_t;

void UpdatePatch(ocxxr::Datablock<updateStateInfo_t> stateInfo,
                 ocxxr::Arena<void> geometricArena,
                 ocxxr::Arena<void> localArena);

struct updateStateInfo_t {
    u64 rank;
    u64 lenGeometric;
    u64 lenActive;
    u64 lenBuffer;
    u64 lenAux;
    u64 thisStep;
    u64 nSteps;
    u64 nRanks;
    ocxxr::BasedPtr<Grid> grid;
    ocxxr::BasedPtr<double> geometric;
    // ocrEdt_t FNC;    // function ptr
    ocxxr::TaskTemplate<decltype(UpdatePatch)> TML;  // template
    // ocrGuid_t EDT;   // edt guid
    ocxxr::Event<void> DONE;  // sync event
};

// XXX - running extra iterations makes the test fail, obviously
#define LOOP_ITERS 1

void UpdatePatch(ocxxr::Datablock<updateStateInfo_t> stateInfo,
                 ocxxr::Arena<void> geometricArena,
                 ocxxr::Arena<void> localArena) {
    int i, rank, len, dlen, step, maxStep, nRanks;

    // updateStateInfo_t *stateInfo = (updateStateInfo_t *)depv[1].ptr;

    // BEGIN LOOP
    for (int iter = 0; iter < LOOP_ITERS; iter++) {
        rank = stateInfo->rank;
        len = stateInfo->lenGeometric;
        step = stateInfo->thisStep;
        maxStep = stateInfo->nSteps;
        nRanks = stateInfo->nRanks;

        // TODO: Create a single guid with list of neighbor guids; double buffer
        // events for each worker

        dlen = len / 8;
        if (len == 0) {
            PRINTF("ERROR!\n");
            ocxxr::Abort(-1);
        }
        DBG_PRINTF("Rank %d in updatePatch will update %d bytes! (%d)\n", rank,
                   len, dlen);
        double *newData = stateInfo->geometric;

        ocxxr::SetImplicitArena(localArena);
        Grid *grid = stateInfo->grid;
        DBG_PRINTF("GJDEBUG: rank= %d grid pointer in updatePatch %lx\n", rank,
                   grid);

        // save pre-patch allocator state
        auto alloc_state = localArena.SaveState();
        DBG_PRINTF("NV: [%d] offset  = %ld\n", rank, alloc_state.offset);

        // activate the patch
        GridPatch *pPatch = grid->ActivateEmptyPatch(rank);
        DBG_PRINTF("GJDEBUG: rank= %d active patches %d\n", rank,
                   grid->GetActivePatchCount());

        // Get DataContainers associated with GridPatch

        DataContainer &dataGeometric = pPatch->GetDataContainerGeometric();

        // Proof of concept to update data in a GridPatch
        // TODO: Perform a functional update
        // TODO: Destroy neighbor events
        // TODO: unpack neighbor data
        for (i = 0; i < dlen; i++) {
            newData[i] = newData[i] + (double)rank;
        }
        // TODO: pack data for neighbor
        // TODO: satisfy events for neighbors: note we need to double the
        // events; alternate even/odd during iterations
        //
        // deactivate the GridPatch
        DBG_PRINTF("Rank %d deactivates its patch in step %d \n", rank, step);
        grid->DeactivatePatch(rank);
        // TODO:create neighbor events; use the guids from list created in main

        // DEBUG print current allocator state
        auto alloc_state2 = localArena.SaveState();
        DBG_PRINTF("NV: [%d] offset' = %ld\n", rank, alloc_state2.offset);

        // restore pre-patch allocator state
        localArena.RestoreState(alloc_state);

    }  // END LOOP

    // create clone for next timestep or trigger output
    if (step < maxStep) {
        stateInfo->thisStep++;
        geometricArena.Release();
        localArena.Release();
        // change to local guid for EDT
        auto update_template = stateInfo->TML;
        auto update_task = update_template().CreateTask(
                stateInfo, geometricArena, localArena);
        // release data blocks

        //    This data is currently not being used
        //    ocrAddDependence(depv[2].guid, stateInfo->EDT, 2, DB_MODE_RW );
        //    ocrAddDependence(depv[3].guid, stateInfo->EDT, 3, DB_MODE_RW );
        //    ocrAddDependence(depv[4].guid, stateInfo->EDT, 4, DB_MODE_RW );

        // TODO: Create "magic" neighbor events
    } else {
        // TODO:  Release the data block which has been updated and is printed
        // by outputEDT; important for x86-mpi;
        geometricArena.Release();
        localArena.Release();
        stateInfo->DONE.Satisfy();
        DBG_PRINTF("Good-by from Rank %d in updatePatch\n", rank);
    }

    fflush(stdout);
}

struct OutputParams {
    u32 nWorkers;
    u32 lastStep;
};

void OutputTask(OutputParams &params, ocxxr::Datablock<void>,
                ocxxr::ArenaList<double> arenas) {
    double sum = 0.0;
    int i, j;
    for (i = 0; i < params.nWorkers; i++) {
        double *newData = arenas[i].data_ptr();
        PRINTF("Output of rank %d\n", i);
        for (j = 0; j < 10; j++) {
            PRINTF("%f ", newData[j]);
        }
        if (newData[5] == (i + i * params.lastStep)) {
            PRINTF("\n\n");
            PRINTF("Test passed!\n");
        } else {
            PRINTF("\n\n");
            PRINTF("Test FAILED! %f\n", sum);
        }
        PRINTF("\n\n");
    }
    DBG_PRINTF("Good-by from outputEdt!\n");
    DBG_PRINTF("***********************\n");
    fflush(stdout);
    ocxxr::Shutdown();
}

///////////////////////////////////////////////////////////////////////////////

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs> args) {
    DBG_PRINTF("Hello from DataContainerTest-OCR!\n");
    u32 argc = args->argc();
    DBG_PRINTF("argc = %d.\n", argc);
    u32 nWorkers;
    u32 nSteps;
    if (argc != 3) {
        nWorkers = 4;
        nSteps = 500;
    } else {
        u32 i = 1;
        nWorkers = (u32)atoi(args->argv(i++));
        nSteps = (u32)atoi(args->argv(i++));
    }
    DBG_PRINTF("Using %d workers, %d steps.\n", nWorkers, nSteps);
    try {
        DBG_ONLY(std::cout << "Initializing Model and Grid ... " << std::endl);

        // Model parameters
        const int nResolutionX = 12;
        const int nResolutionY = 10;
        const int nHorizontalOrder = 4;
        const int nVerticalOrder = 4;
        const int nLevels = 40;

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
        // Model model(EquationSet::PrimitiveNonhydrostaticEquations);

        // Setup the Model

        // pointer to the memories serving as backups for ocrDblock to hold
        // Model and Grids
        // FIXME using new...
        ocxxr::Arena<MG> *arena = new ocxxr::Arena<MG>[nWorkers];
        // void *arenaPtr[nWorkers];
        // MG *myMG[nWorkers];
        // ocrGuid_t arenaGuid[nWorkers];
        for (int i = 0; i < nWorkers; i++) {
            arena[i] = ocxxr::Arena<MG>::Create(ARENA_SIZE);
        }

        ////////// test test test ////////////////////////
        // FIXME: I don't know why the linker fails if I don't set up this
        // constant
        const int EQUATION_TYPE = EquationSet::PrimitiveNonhydrostaticEquations;
        //        MG testMG;
        //        void *testArenaPtr;
        //        ocrGuid_t testArenaGuid;
        //        ocrDbCreate(&testArenaGuid, &testArenaPtr, ARENA_SIZE,
        //        DB_PROP_NONE, NULL_HINT, NO_ALLOC);
        //        ocrAllocatorSetDb(testArenaPtr, (size_t) ARENA_SIZE,
        //        true);
        //        Model * testModel;
        //        testMG.m = ocrNew (Model, EQUATION_TYPE);
        //        testModel = testMG.m;
        //        assert((void*)testModel == testArenaPtr);

        ModelParameters param;  // XXX - what does this do?
        // Model * model [nWorkers];
        for (int i = 0; i < nWorkers; i++) {
            ocxxr::SetImplicitArena(arena[i]);
            MG *root = ocxxr::New<MG>();
            assert(arena[i].data_ptr() == root);
            arena[i]->m = ocxxr::New<Model>(EQUATION_TYPE);
            Model &m = *arena[i]->m;
            // Set the parameters
            m.SetParameters(param);

            // FIXME - why are these allocated like this?
            m.SetHorizontalDynamics(new HorizontalDynamicsStub(m));
            m.SetTimestepScheme(new TimestepSchemeStrang(m));
            m.SetVerticalDynamics(new VerticalDynamicsStub(m));
        }

        // Set the model grid (one patch Cartesian grid)
        // GridCartesianGLL * pGrid[nWorkers];
        for (int i = 0; i < nWorkers; i++) {
            ocxxr::SetImplicitArena(arena[i]);
            arena[i]->g = ocxxr::New<GridCartesianGLL>(
                    *arena[i]->m, nWorkers, nResolutionX, nResolutionY, 4,
                    nHorizontalOrder, nVerticalOrder, nLevels, dGDim, dRefLat,
                    eVerticalStaggering);
        }
        // Create testGrid in a data block

        // GridCartesianGLL * testGrid;
        // myMG.g = ocrNew (GridCartesianGLL, *testModel, nWorkers,
        // nResolutionX, nResolutionY, 4,
        //                  nHorizontalOrder, nVerticalOrder, nLevels,
        //                  dGDim,
        //                  dRefLat, eVerticalStaggering);
        // testGrid = myMG.g;
        // testGrid->ApplyDefaultPatchLayout(nWorkers);
        // const PatchBox & testBox = testGrid->GetPatchBox (0);
        // auto testPatch = GridPatchCartesianGLL(*testGrid, 0, testBox,
        // nHorizontalOrder, nVerticalOrder);
        // int testIndex = testPatch.GetPatchIndex ();
        // testPatch.InitializeDataLocal(false, false, false, false);
        // DataContainer & testGeometric =
        // testPatch.GetDataContainerGeometric();
        // std::cout << "testPatch.Geometric Size:   " <<
        // testGeometric.GetTotalByteSize() << " bytes" << std::endl;
        // Apply the default patch layout
        for (int i = 0; i < nWorkers; i++) {
            ocxxr::SetImplicitArena(arena[i]);
            arena[i]->g->ApplyDefaultPatchLayout(nWorkers);
        }

        //////////////////////////////////////////////////////////////////
        // BEGIN MAIN PROGRAM BLOCK
        // DBG_PRINTF("GJDEBUG: grid pointer in mainEdt %lx\n", pGrid);

        // Build the DataContainer object for the GridPatch objects

        // Last update task for each worker should sync on this event
        auto sync_event = ocxxr::LatchEvent<void>::Create((u64)nWorkers);

        // Output Task
        OutputParams out_params = {nWorkers, nSteps};
        auto out_template = OCXXR_TEMPLATE_FOR(OutputTask);
        auto out_task = out_template().CreateTaskPartial(out_params, nWorkers);
        out_task.DependOn<0>(sync_event);
        out_template.Destroy();

        auto update_template = OCXXR_TEMPLATE_FOR(UpdatePatch);

        for (int i = 0; i < nWorkers; i++) {
            // Create a GridPatch
            GridPatch *pPatchFirst;
            {
                ocxxr::SetImplicitArena(arena[i]);
                const PatchBox &box = arena[i]->g->GetPatchBox(i);
                // assert(&box != nullptr);
                pPatchFirst = ocxxr::New<GridPatchCartesianGLL>(
                        *arena[i]->g, 0, box, nHorizontalOrder, nVerticalOrder);
            }

            pPatchFirst->InitializeDataLocal(false, false, false, false);

            // Get DataContainers associated with GridPatch
            DataContainer &dataGeometric =
                    pPatchFirst->GetDataContainerGeometric();
#if 0
            DataContainer &dataActiveState =
                    pPatchFirst->GetDataContainerActiveState();
            DataContainer &dataBufferState =
                    pPatchFirst->GetDataContainerBufferState();
            DataContainer &dataAuxiliary =
                    pPatchFirst->GetDataContainerAuxiliary();
#endif

            // Output the size requirements (in bytes) of each DataContainer
            DBG_ONLY(std::cout << "GridPatch.Geometric Size:   "
                               << dataGeometric.GetTotalByteSize() << " bytes"
                               << std::endl);
#if 0
            DBG_ONLY(std::cout << "GridPatch.ActiveState Size: "
                               << dataActiveState.GetTotalByteSize() << " bytes"
                               << std::endl);
            DBG_ONLY(std::cout << "GridPatch.BufferState Size: "
                               << dataBufferState.GetTotalByteSize() << " bytes"
                               << std::endl);
            DBG_ONLY(std::cout << "GridPatch.Auxiliary Size:   "
                               << dataAuxiliary.GetTotalByteSize() << " bytes"
                               << std::endl);
#endif

            // Allocate data
            DBG_ONLY(std::cout << "Allocating data ... " << std::endl);
            unsigned char *pDataGeometric;
            unsigned char *pDataActiveState;
            unsigned char *pDataBufferState;
            unsigned char *pDataAuxiliary;

            // Craete Template for updatePatch Edt
            // Parameters:
            //       1 pointer to global grid
            // Dependences:3
            //      updated geometric data block
            //      state information
            //      arena holding grid+model data
            //      TODO: Add dependences on "magic" neighbor events to
            //      handle
            //      the data exchange; 1 event per neighbor

            auto state_info = ocxxr::Datablock<updateStateInfo_t>::Create();
            // ocrDbCreate(&dataGuidState[i], (void **)&state_info,
            //            (u64)(sizeof(updateStateInfo_t)), 0, NULL_HINT,
            //            NO_ALLOC);

            auto geometric = ocxxr::Arena<double>::Create(
                    dataGeometric.GetTotalByteSize());
            memset(geometric.data_ptr(), 0, dataGeometric.GetTotalByteSize());

#if 0
        ocrGuid_t dataGuidActiveState[nWorkers];
        ocrGuid_t dataGuidBufferState[nWorkers];
        ocrGuid_t dataGuidAuxiliary[nWorkers];

            // These containers are currently not being used for the update
            // since we are only performing a proof of concept update
            ocrDbCreate(&dataGuidActiveState[i], (void **)&pDataActiveState,
                        (u64)dataActiveState.GetTotalByteSize(), 0, NULL_HINT,
                        NO_ALLOC);
            ocrDbCreate(&dataGuidBufferState[i], (void **)&pDataBufferState,
                        (u64)dataBufferState.GetTotalByteSize(), 0, NULL_HINT,
                        NO_ALLOC);
            ocrDbCreate(&dataGuidAuxiliary[i], (void **)&pDataAuxiliary,
                        (u64)dataAuxiliary.GetTotalByteSize(), 0, NULL_HINT,
                        NO_ALLOC);

            // Initialize data to zero
            memset(pDataActiveState, 0, dataActiveState.GetTotalByteSize());
            memset(pDataBufferState, 0, dataBufferState.GetTotalByteSize());
            memset(pDataAuxiliary, 0, dataAuxiliary.GetTotalByteSize());
#endif

            // Each updatePatch EDT gets stateInfo: rank, sizes, time step
            // information, information how to create next Edt
            state_info->rank = i;
            state_info->lenGeometric = (u64)dataGeometric.GetTotalByteSize();
#if 0
            state_info->lenActive = (u64)dataActiveState.GetTotalByteSize();
            state_info->lenBuffer = (u64)dataBufferState.GetTotalByteSize();
            state_info->lenAux = (u64)dataAuxiliary.GetTotalByteSize();
#endif
            state_info->thisStep = 0;
            state_info->nSteps = (u64)nSteps;
            state_info->nRanks = (u64)nWorkers;
            state_info->geometric = geometric.data_ptr(),
            state_info->grid = (Grid *)arena[i]->g;
            state_info->TML = update_template;
            state_info->DONE = sync_event;

            auto update_task = update_template().CreateTask(
                    state_info, geometric.Untyped(), arena[i].Untyped());
            out_task.DependOnWithinList(i, geometric);
        }

        // FIXME
        delete[] arena;

    } catch (Exception &e) {
        std::cout << e.ToString() << std::endl;
    }

    // Deinitialize Tempest
    // MPI_Finalize();
}

///////////////////////////////////////////////////////////////////////////////

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

#include <mpi.h>

///////////////////////////////////////////////////////////////////////////////

static const int ParamHorizontalOrder = 4;
static const int ParamVerticalOrder = 4;

static const int ParamTotalPatchCount = 1;
static const int ParamNeighborsPerPatch = 8;

static const int ParamTotalSteps = 1;

///////////////////////////////////////////////////////////////////////////////

///	<summary>
///		Pointer to thread-local Model object (infrastructure for performing
///		the simulation).
///	</summary>
Model * g_pModel;

///////////////////////////////////////////////////////////////////////////////

void InitializeModel(
	Model ** pModel,
    int  &argc,
    char ** &argv
) {

    TempestInitialize( &argc, &argv );

	// Model parameters
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

	const Grid::VerticalStaggering eVerticalStaggering =
		Grid::VerticalStaggering_Levels;

	// Set the parameters

    /*double  dU0,
            dH0,
            dNbar,
            dTheta0,
            dhC,
            daC,
            dxC,
            dpiC,
            dAlpha;
    bool fNoRayleighFriction;

    BeginTempestCommandLine("TaskTest")
        SetDefaultResolutionX( nResolutionX );
        SetDefaultResolutionY( nResolutionY );
        SetDefaultLevels( nLevels );
        SetDefaultOutputDeltaT("200s");
        SetDefaultDeltaT("200s");
        SetDefaultEndTime("200s");
        SetDefaultHorizontalOrder( ParamHorizontalOrder );
        SetDefaultVerticalOrder( ParamVerticalOrder );

        CommandLineDouble( dU0, "u0", 38.61068277 );
        CommandLineDouble( dNbar, "Nbar", 0.01 );
        CommandLineDouble(dTheta0, "Theta0", 280.0);
        CommandLineDouble(dhC, "hC", 1.0);
        CommandLineDouble(daC, "aC", 1000.0);
        CommandLineDouble(dxC, "xC", 7.2E+4);
        CommandLineDouble(dpiC, "piC", 3.14159265);
        CommandLineBool(fNoRayleighFriction, "norayleigh");
        ParseCommandLine(argc, argv);
    EndTempestCommandLine(argv)

    std::cout << "Create Model" << std::endl;
    */

	(*pModel) = new Model(EquationSet::PrimitiveNonhydrostaticEquations);

    //TempestSetupCartesianModel(**pModel, dGDim, dRefLat, iLatBC);

    std::cout << "Model Allocated!" << std::endl;

    //TempestSetupCubedSphereModel(**pModel);

	(*pModel)->SetTimestepScheme(new TimestepSchemeStrang(**pModel));

	(*pModel)->SetHorizontalDynamics(
		new HorizontalDynamicsStub(**pModel));

	(*pModel)->SetVerticalDynamics(
		new VerticalDynamicsStub(**pModel));

	// Set the model grid (one patch Cartesian grid)
    //
    //

    Grid::VerticalDiscretization eVerticalDiscretization = Grid::VerticalDiscretization_FiniteElement;

    GridCartesianGLL * pGrid = new GridCartesianGLL ( **pModel );

    pGrid->DefineParameters();

    int nCommSize;
    MPI_Comm_size(MPI_COMM_WORLD, &nCommSize);

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
        eVerticalStaggering);

    pGrid->InitializeDataLocal();

	/*GridCartesianGLL * pGrid = new GridCartesianGLL(
			**pModel,
			nMaxPatchCount,
			nResolutionX,
			nResolutionY,
			4,
			ParamHorizontalOrder,
			ParamVerticalOrder,
			nLevels,
			dGDim,
			dRefLat,
			eVerticalStaggering);*/

	(*pModel)->SetGrid(pGrid, ParamTotalPatchCount, false);

	pGrid->InitializeAllExchangeBuffers();

	pGrid->GetExchangeBufferRegistry().SetNoDataOwnership();
}

///////////////////////////////////////////////////////////////////////////////

void InitializeTask(
	int ixPatch,
	unsigned char ** pGeometricData,
	unsigned char ** pActiveStateData,
	unsigned char ** pBufferStateData,
	unsigned char ** pAuxiliaryData,
	int * nExchangeBuffers,
	int * rgExchangeBuffers,
	unsigned char ** rgRecvBuffers,
	unsigned char ** rgSendBuffers
) {

	const int nResolutionX = 10;
	const int nResolutionY = 10;
	// Activate an empty GridPatch
	GridCartesianGLL * pGrid = (GridCartesianGLL *)g_pModel->GetGrid();

    std::cout << "Trying to make new patch!" << std::endl;

	GridPatch * pPatch = pGrid->NewPatch(ixPatch);

    /* PatchBox box(
        0, 0, g_pModel->GetHaloElements(),
        0, ParamHorizontalOrder * nResolutionX,
        0, ParamHorizontalOrder * nResolutionY );

    std::cout << "PatchBox made!" << std::endl;

    GridPatch * pPatch = new GridPatchCartesianGLL(
                                (*pGrid),
                                0,
                                box,
                                (int)ParamHorizontalOrder,
                                (int)ParamVerticalOrder );
    */
    std::cout << "GridPatch made!" << std::endl;

	pPatch->InitializeDataLocal();

    std::cout << "Geometric DC: " << pPatch->GetDataContainerGeometric().GetTotalByteSize() << std::endl;
    std::cout << "ActiveState DC: " << pPatch->GetDataContainerActiveState().GetTotalByteSize() << std::endl;
    std::cout << "BufferState DC: " << pPatch->GetDataContainerBufferState().GetTotalByteSize() << std::endl;
    std::cout << "Auxiliary DC: " << pPatch->GetDataContainerAuxiliary().GetTotalByteSize() << std::endl;

	// Allocate patch data
	*pGeometricData = (unsigned char *)
		malloc(pPatch->GetDataContainerGeometric().GetTotalByteSize());
	*pActiveStateData = (unsigned char *)
		malloc(pPatch->GetDataContainerActiveState().GetTotalByteSize());
	*pBufferStateData = (unsigned char *)
		malloc(pPatch->GetDataContainerBufferState().GetTotalByteSize());
	*pAuxiliaryData = (unsigned char *)
		malloc(pPatch->GetDataContainerAuxiliary().GetTotalByteSize());

	// Allocate exchange buffer data using exchange buffer metadata
	ExchangeBufferRegistry & ebr = pGrid->GetExchangeBufferRegistry();

	std::vector<int> vecExchangeBufferIndices;
	ebr.GetExchangeBuffersBySourcePatchIx(ixPatch, vecExchangeBufferIndices);

	if (vecExchangeBufferIndices.size() > ParamNeighborsPerPatch) {
		_EXCEPTION1("Insufficient ParamNeighborsPerPatch (>= %lu)",
			vecExchangeBufferIndices.size());
	}

	(*nExchangeBuffers) = vecExchangeBufferIndices.size();
	for (int m = 0; m < (*nExchangeBuffers); m++) {
		int ixExchangeBuffer = vecExchangeBufferIndices[m];

		const ExchangeBufferInfo & ebinfo =
			ebr.GetExchangeBufferInfo(ixExchangeBuffer);

		rgExchangeBuffers[m] = ixExchangeBuffer;
		rgRecvBuffers[m] = (unsigned char *) malloc(ebinfo.sByteSize);
		rgSendBuffers[m] = (unsigned char *) malloc(ebinfo.sByteSize);
	}

	pGrid->DeactivatePatch(ixPatch);

    std::cout << "Grid patches made!" << std::endl;

	// TODO: Create a new ExecuteTask with Patch ixPatch, Step 0, SubStep 0
}

///////////////////////////////////////////////////////////////////////////////

void ExecuteTask(
	int ixPatch,
	int ixStep,
	int ixSubStep,
	unsigned char * pGeometricData,
	unsigned char * pActiveStateData,
	unsigned char * pBufferStateData,
	unsigned char * pAuxiliaryData,
	int nExchangeBuffers,
	int * rgExchangeBufferIds,
	unsigned char ** rgRecvBufferPtrs,
	unsigned char ** rgSendBufferPtrs
) {
	// Get a pointer to the Grid
	Grid * pGrid = g_pModel->GetGrid();

	// Set the relevant exchange buffer pointers
	ExchangeBufferRegistry & ebr = pGrid->GetExchangeBufferRegistry();
	for (int m = 0; m < nExchangeBuffers; m++) {
		ebr.Assign(
			rgExchangeBufferIds[m],
			rgRecvBufferPtrs[m],
			rgSendBufferPtrs[m]);
	}

	// Set the data for the GridPatch
	GridPatch * pPatch = pGrid->ActivateEmptyPatch(ixPatch);

	pGrid->InitializeConnectivity(false);

	pPatch->InitializeDataLocal();

    pPatch->GetDataContainerGeometric().Detach();
    pPatch->GetDataContainerActiveState().Detach();
    pPatch->GetDataContainerBufferState().Detach();
    pPatch->GetDataContainerAuxiliary().Detach();

	pPatch->GetDataContainerGeometric().AttachTo(pGeometricData);
	pPatch->GetDataContainerActiveState().AttachTo(pActiveStateData);
	pPatch->GetDataContainerBufferState().AttachTo(pBufferStateData);
	pPatch->GetDataContainerAuxiliary().AttachTo(pAuxiliaryData);

	// Perform sub-step
	printf("CURR:  GridPatch %i performing step %i.%i\n",
		ixPatch, ixStep, ixSubStep);

/*
	model.SubStep(
		(ixStep == 0),
		(ixStep == ParamTotalSteps-1),
		ixSubStep);
*/
	// TODO: Pack data from Patch into exchange buffers and tag as complete

	// TODO: Create a new ExecuteTask on Patch ixPatch, ixNextStep, ixNextSubStep
	int nSubStepsPerStep = g_pModel->GetTimestepScheme()->GetSubStepCount();

	int ixNextSubStep;
	int ixNextStep;

	if (ixSubStep == nSubStepsPerStep-1) {
		ixNextSubStep = 0;
		ixNextStep = ixStep + 1;
	} else {
		ixNextSubStep = ixSubStep + 1;
		ixNextStep = ixStep;
	}

	// Cleanup
	pGrid->DeactivatePatch(ixPatch);

	for (int m = 0; m < nExchangeBuffers; m++) {
		ebr.Unassign(rgExchangeBufferIds[m]);
	}

	printf("NEXT:  GridPatch %i to perform step %i.%i (new EDT)\n",
		ixPatch, ixNextStep, ixNextSubStep);
}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {

	// Initialize MPI
	//MPI_Init(&argc, &argv);

try {

	//////////////////////////////////////////////////////////////////
	// BEGIN INITIALIZATION BLOCK
	std::cout << "BEGIN Thread Startup" << std::endl;
	InitializeModel(&g_pModel, argc, argv);
	std::cout << "END Thread Startup" << std::endl;

	// END INITIALIZATION BLOCK
	//////////////////////////////////////////////////////////////////

	// TODO: Create DataBlocks for all GridPatch data
	unsigned char * pGeometricData[ParamTotalPatchCount];
	unsigned char * pActiveStateData[ParamTotalPatchCount];
	unsigned char * pBufferStateData[ParamTotalPatchCount];
	unsigned char * pAuxiliaryData[ParamTotalPatchCount];

	int nExchangeBuffers[ParamTotalPatchCount];
	int rgExchangeBuffers[ParamTotalPatchCount][ParamNeighborsPerPatch];
	unsigned char * rgRecvBuffers[ParamTotalPatchCount][ParamNeighborsPerPatch];
	unsigned char * rgSendBuffers[ParamTotalPatchCount][ParamNeighborsPerPatch];

	//////////////////////////////////////////////////////////////////
	// BEGIN INITIALIZATION BLOCK

	std::cout << "BEGIN Initialization" << std::endl;

	for (int ixPatch = 0; ixPatch < ParamTotalPatchCount; ixPatch++) {
		InitializeTask(
			ixPatch,
			&(pGeometricData[ixPatch]),
			&(pActiveStateData[ixPatch]),
			&(pBufferStateData[ixPatch]),
			&(pAuxiliaryData[ixPatch]),
			&(nExchangeBuffers[ixPatch]),
			&(rgExchangeBuffers[ixPatch][0]),
			&(rgRecvBuffers[ixPatch][0]),
			&(rgSendBuffers[ixPatch][0]));
	}

	std::cout << "DONE Initialization" << std::endl;

	// END INITIALIZATION BLOCK
	//////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////
	// BEGIN MAIN PROGRAM BLOCK

	std::cout << "BEGIN Execution" << std::endl;

	for (int ixPatch = 0; ixPatch < ParamTotalPatchCount; ixPatch++) {
	for (int ixStep = 0; ixStep < ParamTotalSteps; ixStep++) {
	for (int ixSubStep = 0; ixSubStep < 1; ixSubStep++) {
		ExecuteTask(
			ixPatch,
			ixStep,
			ixSubStep,
			pGeometricData[ixPatch],
			pActiveStateData[ixPatch],
			pBufferStateData[ixPatch],
			pAuxiliaryData[ixPatch],
			nExchangeBuffers[ixPatch],
			rgExchangeBuffers[ixPatch],
			rgRecvBuffers[ixPatch],
			rgSendBuffers[ixPatch]);
	}
	}
	}

	std::cout << "DONE Execution" << std::endl;

	// END MAIN PROGRAM BLOCK
	//////////////////////////////////////////////////////////////////

	delete g_pModel;

} catch(Exception & e) {
	std::cout << e.ToString() << std::endl;
}

	// Deinitialize Tempest
	//MPI_Finalize();
    //
    TempestDeinitialize();
}

///////////////////////////////////////////////////////////////////////////////


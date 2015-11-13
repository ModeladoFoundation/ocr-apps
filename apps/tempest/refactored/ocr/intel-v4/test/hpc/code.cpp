try {
	std::cout << "Initializing Model and Grid ... " << std::endl;

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
	Model model(EquationSet::PrimitiveNonhydrostaticEquations);

	// Set the parameters
	ModelParameters param;

	model.SetParameters(param);

	model.SetTimestepScheme(new TimestepSchemeStrang(model));

	model.SetHorizontalDynamics(
		new HorizontalDynamicsStub(model));

	model.SetVerticalDynamics(
		new VerticalDynamicsStub(model));

	// Set the model grid (one patch Cartesian grid)
        GridCartesianGLL * pGrid [nWorkers];
        for (int i = 0; i<nWorkers; i++) {
	    std::cout << "GJDEBUG creating the Grid " << i << std::endl;
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
	        pGrid[i]->ApplyDefaultPatchLayout(nWorkers);
                model.SetGrid(pGrid [i], false);
	        std::cout << "GJDEBUG Set Grid " << i << "to zero" << std::endl;
                pGrid [i]->InitializeAllExchangeBuffers();
	        std::cout << "GJDEBUG Initialize exchange buffer for Grid " << i << std::endl;
                pGrid [i]->GetExchangeBufferRegistry().SetNoDataOwnership();
	        std::cout << "GJDEBUG set exchange buffer registry to no data ownership " << i << std::endl;
        }

#include "rsbench.h"

// JRT
void generate_n_poles( Inputs input, int *R )
{
    int total_resonances = input.avg_n_poles * input.n_nuclides;

    for( int i = 0; i < input.n_nuclides; i++ )
        R[i] = 0;

    for( int i = 0; i < total_resonances; i++ )
        R[rand() % input.n_nuclides]++;

    // Ensure all nuclides have at least 1 resonance
    for( int i = 0; i < input.n_nuclides; i++ )
        if( R[i] == 0 )
            R[i] = 1;

    //Debug
    //for( int i = 0; i < input.n_nuclides; i++ )
    //  printf("R[%d] = %d\n", i, R[i]);


    return;
}

void generate_n_windows( Inputs input, int *R )
{
    int total_resonances = input.avg_n_windows * input.n_nuclides;

    for( int i = 0; i < input.n_nuclides; i++ )
        R[i] = 0;

    for( int i = 0; i < total_resonances; i++ )
        R[rand() % input.n_nuclides]++;

    // Ensure all nuclides have at least 1 resonance
    for( int i = 0; i < input.n_nuclides; i++ )
        if( R[i] == 0 )
            R[i] = 1;

    /* Debug
    for( int i = 0; i < input.n_nuclides; i++ )
        printf("R[%d] = %d\n", i, R[i]);
    */

    return;
}

//
void generate_poles( Inputs input, int * n_poles, ocrGuid_t* PTR_pole_DBguids_nuclide )
{
    // Pole Scaling Factor -- Used to bias hitting of the fast Faddeeva
    // region to approximately 99.5% (i.e., only 0.5% of lookups should
    // require the full eval).
    double f = 152.5;

    // fill with data
    for( int i = 0; i < input.n_nuclides; i++ )
    {
        Pole* R_i;
        //ocrPrintf( " i %d poles %d\n", i, n_poles[i] );
        ocrDbCreate( &(PTR_pole_DBguids_nuclide[i]), (void **) &R_i, sizeof(Pole)*(n_poles[i]),
                     DB_PROP_NONE, NULL_HINT, NO_ALLOC );

        for( int j = 0; j < n_poles[i]; j++ )
        {
            //ocrPrintf( "j = %d %d\n", j, input.numL );
            R_i[j].MP_EA = f*((double) rand() / RAND_MAX + (double) rand() / RAND_MAX * I);
            R_i[j].MP_RT = f*(double) rand() / RAND_MAX + (double) rand() / RAND_MAX * I;
            R_i[j].MP_RA = f*(double) rand() / RAND_MAX + (double) rand() / RAND_MAX * I;
            R_i[j].MP_RF = f*(double) rand() / RAND_MAX + (double) rand() / RAND_MAX * I;
            R_i[j].l_value = rand() % input.numL;
        }
    }

    /* Debug
    for( int i = 0; i < input.n_nuclides; i++ )
        for( int j = 0; j < n_poles[i]; j++ )
            printf("R[%d][%d]: Eo = %lf lambda_o = %lf Tn = %lf Tg = %lf Tf = %lf\n", i, j, R[i][j].Eo, R[i][j].lambda_o, R[i][j].Tn, R[i][j].Tg, R[i][j].Tf);
    */

    return;
}

void generate_window_params( Inputs input, int * n_windows, int * n_poles, ocrGuid_t* PTR_window_DBguids_nuclide )
{
    // fill with data
    for( int i = 0; i < input.n_nuclides; i++ )
    {
        Window* R_i;
        //ocrPrintf( " i %d windows %d\n", i, n_windows[i] );
        ocrDbCreate( &(PTR_window_DBguids_nuclide[i]), (void **) &R_i, sizeof(Window)*(n_windows[i]),
                     DB_PROP_NONE, NULL_HINT, NO_ALLOC );

        int space = n_poles[i] / n_windows[i];
        int remainder = n_poles[i] - space * n_windows[i];
        int ctr = 0;
        for( int j = 0; j < n_windows[i]; j++ )
        {
            R_i[j].T = (double) rand() / RAND_MAX;
            R_i[j].A = (double) rand() / RAND_MAX;
            R_i[j].F = (double) rand() / RAND_MAX;
            R_i[j].start = ctr;
            R_i[j].end = ctr + space - 1;

            ctr += space;

            if ( j < remainder )
            {
                ctr++;
                R_i[j].end++;
            }
        }
    }

    return;
}

void generate_pseudo_K0RS( Inputs input, ocrGuid_t* PTR_pseudoK0RS_DBguids_nuclide )
{
    // fill with data
    for( int i = 0; i < input.n_nuclides; i++ )
    {
        double* R_i;
        ocrDbCreate( &(PTR_pseudoK0RS_DBguids_nuclide[i]), (void **) &R_i, sizeof(double)*(input.numL),
                     DB_PROP_NONE, NULL_HINT, NO_ALLOC );
        for( int j = 0; j < input.numL; j++ )
            R_i[j] = (double) rand() / RAND_MAX;
    }

    return;
}

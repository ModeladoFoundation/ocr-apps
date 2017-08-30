#include "rsbench.h"
#include "ocr.h"

// Prints program logo
void logo(int version)
{
    border_print();
    ocrPrintf(
"                    _____   _____ ____                  _     \n"
"                   |  __ \\ / ____|  _ \\                | |    \n"
"                   | |__) | (___ | |_) | ___ _ __   ___| |__  \n"
"                   |  _  / \\___ \\|  _ < / _ \\ '_ \\ / __| '_ \\ \n"
"                   | | \\ \\ ____) | |_) |  __/ | | | (__| | | |\n"
"                   |_|  \\_\\_____/|____/ \\___|_| |_|\\___|_| |_|\n"
    );
    border_print();
    center_print("Ported to OCR at Intel", 79);
    //char v[100];
    //sprintf(v, "Version: %d", version);
    //center_print(v, 79);
    center_print("Version: 1.0", 79); //Hard-coded
    border_print();
}

// Prints Section titles in center of 80 char terminal
void center_print(const char *s, int width)
{
    int length = strlen(s);
    int i;
    for (i=0; i<=(width-length)/2; i++) {
        ocrPrintf(" ");
    }
    ocrPrintf("%s", s);
    ocrPrintf("\n");
}

void border_print(void)
{
    ocrPrintf(
    "==================================================================="
    "=============\n");
}

// Prints comma separated integers - for ease of reading
void fancy_int( int a )
{
#if 0
    if( a < 1000 )
        ocrPrintf("%d\n",a);

    else if( a >= 1000 && a < 1000000 )
        ocrPrintf("%d,%03d\n", a / 1000, a % 1000);

    else if( a >= 1000000 && a < 1000000000 )
        ocrPrintf("%d,%03d,%03d\n", a / 1000000, (a % 1000000) / 1000, a % 1000 );

    else if( a >= 1000000000 )
        ocrPrintf("%d,%03d,%03d,%03d\n",
               a / 1000000000,
               (a % 1000000000) / 1000000,
               (a % 1000000) / 1000,
               a % 1000 );
    else
        ocrPrintf("%d\n",a);
#endif
    ocrPrintf("%d\n", a);
}

Inputs read_CLI( int argc, char * argv[] )
{
    Inputs input;

    input.nprocs = 1; //omp_get_num_procs();
    // defaults to max threads on the system
    input.nthreads = 1; //omp_get_num_procs();
    // defaults to 355 (corresponding to H-M Large benchmark)
    input.n_nuclides = 355;
    input.n_mats = 12;
    // defaults to 10,000,000
    input.lookups = 10000000;
    // defaults to H-M Large benchmark
    input.HM = LARGE;
    // defaults to 3000 resonancs (avg) per nuclide
    input.avg_n_poles = 1000;
    // defaults to 100
    input.avg_n_windows = 100;
    // defaults to 4;
    input.numL = 4;
    // defaults to temperature dependence (Doppler broadening)
    input.doppler = 1;

    // Collect Raw Input
    for( int i = 1; i < argc; i++ )
    {
        char * arg = argv[i];

        // nthreads (-t)
        if( strcmp(arg, "-t") == 0 )
        {
            if( ++i < argc )
                input.nthreads = atoi(argv[i]);
            else
                print_CLI_error();
        }
        // nprocs (-p)
        else if( strcmp(arg, "-p") == 0 )
        {
            if( ++i < argc )
                input.nprocs = atoi(argv[i]);
            else
                print_CLI_error();
        }
        // lookups (-l)
        else if( strcmp(arg, "-l") == 0 )
        {
            if( ++i < argc )
                input.lookups = atoi(argv[i]);
            else
                print_CLI_error();
        }
        // nuclides (-n)
        else if( strcmp(arg, "-n") == 0 )
        {
            if( ++i < argc )
                input.n_nuclides = atoi(argv[i]);
            else
                print_CLI_error();
        }
        // HM (-s)
        else if( strcmp(arg, "-s") == 0 )
        {
            if( ++i < argc )
            {
                if( strcmp(argv[i], "small") == 0 )
                {
                    input.HM = SMALL;
                    input.n_nuclides = 68;
                }
                else if ( strcmp(argv[i], "large") == 0 )
                    input.HM = LARGE;
                else
                    print_CLI_error();
            }
            else
                print_CLI_error();
        }
        // Doppler Broadening (Temperature Dependence)
        else if( strcmp(arg, "-d") == 0 )
        {
            input.doppler = 0;
        }
        // Avg number of windows per nuclide (-w)
        else if( strcmp(arg, "-w") == 0 )
        {
            if( ++i < argc )
                input.avg_n_windows = atoi(argv[i]);
            else
                print_CLI_error();
        }
        // Avg number of poles per nuclide (-p)
        else if( strcmp(arg, "-p") == 0 )
        {
            if( ++i < argc )
                input.avg_n_poles = atoi(argv[i]);
            else
                print_CLI_error();
        }
        else
            print_CLI_error();
    }

    // Validate Input

    // Validate nthreads
    if( input.nthreads < 1 )
        print_CLI_error();

    // Validate n_isotopes
    if( input.n_nuclides < 1 )
        print_CLI_error();

    // Validate lookups
    if( input.lookups < 1 )
        print_CLI_error();

    // Validate lookups
    if( input.avg_n_poles < 1 )
        print_CLI_error();

    // Validate lookups
    if( input.avg_n_windows < 1 )
        print_CLI_error();

    // Set HM size specific parameters
    // (defaults to large)

    // Return input struct
    return input;
}

void print_CLI_error(void)
{
    ocrPrintf("Usage: ./multibench <options>\n");
    ocrPrintf("Options include:\n");
    ocrPrintf("  -t <threads>     Number of OpenMP threads to run\n");
    ocrPrintf("  -s <size>        Size of H-M Benchmark to run (small, large)\n");
    ocrPrintf("  -l <lookups>     Number of Cross-section (XS) lookups\n");
    ocrPrintf("  -p <poles>       Average Number of Poles per Nuclide\n");
    ocrPrintf("  -w <poles>       Average Number of Windows per Nuclide\n");
    ocrPrintf("  -d               Disables Temperature Dependence (Doppler Broadening)\n");
    ocrPrintf("Default is equivalent to: -s large -l 10000000 -p 1000 -w 100\n");
    ocrPrintf("See readme for full description of default run values\n");
    exit(4);
}

void print_input_summary(Inputs input)
{
    // Calculate Estimate of Memory Usage
    int version = 9;
    logo(version);
    center_print("INPUT SUMMARY", 79);
    border_print();
    size_t mem = get_mem_estimate(input);

    ocrPrintf("Materials:                   %d\n", input.n_mats);
    ocrPrintf("H-M Benchmark Size:          ");
    if( input.HM == 0 )
        ocrPrintf("Small\n");
    else
        ocrPrintf("Large\n");
    if( input.doppler == 1 )
        ocrPrintf("Temperature Dependence:      ON\n");
    else
        ocrPrintf("Temperature Dependence:      OFF\n");
    ocrPrintf("Total Nuclides:              %d\n", input.n_nuclides);
    ocrPrintf("Avg Poles per Nuclide:       "); fancy_int(input.avg_n_poles);
    ocrPrintf("Avg Windows per Nuclide:     "); fancy_int(input.avg_n_windows);
    ocrPrintf("XS Lookups:                  "); fancy_int(input.lookups);
    ocrPrintf("Threads:                     %d\n", input.nthreads);
    ocrPrintf("Est. Memory Usage (MB):      %.1f\n", mem / 1024.0 / 1024.0);
    #ifdef PAPI
    ocrPrintf("PAPI Performance Counters:   ON\n");
    #endif

}

void print_results(Inputs input, int mype, double runtime, int nprocs, u64 g_abrarov, u64 g_alls)
{
    if( mype == 0 )
    {
    border_print();
    center_print("RESULTS", 79);
    border_print();

    ocrPrintf("Threads:     %d\n", input.nthreads);
    ocrPrintf("Ranks:     %d\n", input.nprocs);
    if( input.doppler)
    ocrPrintf("Slow Faddeeva: %.2f%%\n", (double) g_abrarov/g_alls * 100.f);
    ocrPrintf("Runtime:     %.3f seconds\n", runtime);
    ocrPrintf("Lookups:     "); fancy_int(input.lookups);
    ocrPrintf("Lookups/s:   "); fancy_int((double) input.lookups / (runtime));

    border_print();
    }

}

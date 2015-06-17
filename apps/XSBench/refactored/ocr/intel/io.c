#include "XSbench_header.h"
#include "ocr.h"

#ifdef MPI
#include<mpi.h>
#endif

// Prints program logo
void logo(int version)
{
    border_print();
    PRINTF(
    "                   __   __ ___________                 _                        \n"
    "                   \\ \\ / //  ___| ___ \\               | |                       \n"
    "                    \\ V / \\ `--.| |_/ / ___ _ __   ___| |__                     \n"
    "                    /   \\  `--. \\ ___ \\/ _ \\ '_ \\ / __| '_ \\                    \n"
    "                   / /^\\ \\/\\__/ / |_/ /  __/ | | | (__| | | |                   \n"
    "                   \\/   \\/\\____/\\____/ \\___|_| |_|\\___|_| |_|                   \n\n"
    );
    border_print();
    center_print("Ported to OCR at Intel", 79);
    char v[100];
    center_print("Version: 1.0", 79); //Hard-coded
    border_print();
}

// Prints Section titles in center of 80 char terminal
void center_print(const char *s, int width)
{
    int length = strlen(s);
    int i;
    for (i=0; i<=(width-length)/2; i++) {
        PRINTF(" ");

    }
    PRINTF("%s", s);
    PRINTF("\n");
}

void print_results( Inputs in, int mype, double runtime, int nprocs )
//    unsigned long long vhash )
{
    // Calculate Lookups per sec
    int lookups_per_sec = (int) ((double) in.lookups / runtime);

    // Print output
    if( mype == 0 )
    {
        border_print();
        center_print("RESULTS", 79);
        border_print();

        // Print the results
        PRINTF("Threads:     %d\n", in.nthreads);
        #ifdef MPI
        PRINTF("MPI ranks:   %d\n", nprocs);
        #endif
        #ifdef MPI
        PRINTF("Lookups:     "); fancy_int(in.lookups);
        PRINTF("Total Lookups/s:            ");
        fancy_int(total_lookups);
        PRINTF("Avg Lookups/s per MPI rank: ");
        fancy_int(total_lookups / nprocs);
        #else
        PRINTF("Runtime:     %.3f seconds\n", runtime);
        PRINTF("Lookups:     "); fancy_int(in.lookups);
        PRINTF("Lookups/s:   ");
        fancy_int(lookups_per_sec);
        #endif
        //#ifdef VERIFICATION
        //PRINTF("Verification checksum: %llu\n", vhash);
        //#endif
        border_print();

    }
}

void print_inputs(Inputs in, int nprocs, int version )
{
    // Calculate Estimate of Memory Usage
    int mem_tot = estimate_mem_usage( in );
    logo(version);
    center_print("INPUT SUMMARY", 79);
    border_print();
    #ifdef VERIFICATION
    PRINTF("Verification Mode:            on\n");
    #endif
    PRINTF("Materials:                    %d\n", in.n_mats);
    PRINTF("H-M Benchmark Size:           %s\n", in.HM);
    PRINTF("Total Nuclides:               %ld\n", in.n_isotopes);
    PRINTF("Gridpoints (per Nuclide):     ");
    fancy_int(in.n_gridpoints);
    PRINTF("Unionized Energy Gridpoints:  ");
    fancy_int(in.n_isotopes*in.n_gridpoints);
    PRINTF("XS Lookups:                   "); fancy_int(in.lookups);
    #ifdef MPI
    PRINTF("MPI Ranks:                    %d\n", nprocs);
    PRINTF("OMP Threads per MPI Rank:     %d\n", in.nthreads);
    PRINTF("Mem Usage per MPI Rank (MB):  "); fancy_int(mem_tot);
    #else
    PRINTF("Threads:                      %d\n", in.nthreads);
    PRINTF("Est. Memory Usage (MB):       "); fancy_int(mem_tot);
    #endif
    border_print();
    center_print("INITIALIZATION", 79);
    border_print();
}

void border_print(void)
{
    PRINTF(
    "==================================================================="
    "=============\n");
}

// Prints comma separated integers - for ease of reading
void fancy_int( long a )
{
    #if 0   //PRINTF doesn't support fancy format specifiers (e.g., %03ld for leading zeros)
    if( a < 1000 )
        PRINTF("%ld\n",a);

    else if( a >= 1000 && a < 1000000 )
        PRINTF("%ld,%03ld\n", a / 1000, a % 1000);

    else if( a >= 1000000 && a < 1000000000 )
        PRINTF("%ld,%03ld,%03ld\n",a / 1000000,(a % 1000000) / 1000,a % 1000 );

    else if( a >= 1000000000 )
        PRINTF("%ld,%03ld,%03ld,%03ld\n",
               a / 1000000000,
               (a % 1000000000) / 1000000,
               (a % 1000000) / 1000,
               a % 1000 );
    else
        PRINTF("%ld\n",a);
    #endif

    PRINTF("%ld\n",a);
}

void print_CLI_error(void)
{
    PRINTF("Usage: ./XSBench <options>\n");
    PRINTF("Options include:\n");
    PRINTF("  -t <threads>     Number of OpenMP threads to run\n");
    PRINTF("  -s <size>        Size of H-M Benchmark to run (small, large, XL, XXL)\n");
    PRINTF("  -g <gridpoints>  Number of gridpoints per nuclide (overrides -s defaults)\n");
    PRINTF("  -l <lookups>     Number of Cross-section (XS) lookups\n");
    PRINTF("Default is equivalent to: -s large -l 15000000\n");
    PRINTF("See readme for full description of default run values\n");
    ocrShutdown();
    //exit(4);
}

Inputs read_CLI( int argc, char * argv[] )
{
    Inputs input;

    input.nprocs = 1;

    // defaults to max threads on the system
    input.nthreads = 1; //omp_get_num_procs();

    // defaults to 355 (corresponding to H-M Large benchmark)
    input.n_isotopes = 355;

    input.n_mats = 12;

    // defaults to 11303 (corresponding to H-M Large benchmark)
    input.n_gridpoints = 11303;

    // defaults to 15,000,000
    input.lookups = 15000;

    // defaults to H-M Large benchmark
    //input.HM = (char *) malloc( 6 * sizeof(char) );
    strcpy( input.HM, "small" );

    // Check if user sets these
    int user_g = 0;

    // Collect Raw Input
    for( int i = 1; i < argc; i++ )
    {
        char * arg = getArgv(argv, i);

        // nthreads (-t)
        if( strcmp(arg, "-t") == 0 )
        {
            if( ++i < argc )
                input.nthreads = atoi(getArgv(argv, i));
            else
                print_CLI_error();
        }
        // n_gridpoints (-g)
        else if( strcmp(arg, "-g") == 0 )
        {
            if( ++i < argc )
            {
                user_g = 1;
                input.n_gridpoints = atoi(getArgv(argv, i)); //TODO: we need atol here but we don't have it yet
            }
            else
                print_CLI_error();
        }
        // lookups (-l)
        else if( strcmp(arg, "-l") == 0 )
        {
            if( ++i < argc )
                input.lookups = atoi(getArgv(argv,i));
            else
                print_CLI_error();
        }
        // HM (-s)
        else if( strcmp(arg, "-s") == 0 )
        {
            if( ++i < argc )
                strcpy(input.HM, getArgv(argv,i));
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
    if( input.n_isotopes < 1 )
        print_CLI_error();

    // Validate n_gridpoints
    if( input.n_gridpoints < 1 )
        print_CLI_error();

    // Validate lookups
    if( input.lookups < 1 )
        print_CLI_error();

    // Validate HM size
    if( strcasecmp(input.HM, "small") != 0 &&
        strcasecmp(input.HM, "large") != 0 &&
        strcasecmp(input.HM, "XL") != 0 &&
        strcasecmp(input.HM, "XXL") != 0 )
        print_CLI_error();

    // Set HM size specific parameters
    // (defaults to large)
    if( strcasecmp(input.HM, "small") == 0 )
        input.n_isotopes = 68;
    else if( strcasecmp(input.HM, "XL") == 0 && user_g == 0 )
        input.n_gridpoints = 238847; // sized to make 120 GB XS data
    else if( strcasecmp(input.HM, "XXL") == 0 && user_g == 0 )
        input.n_gridpoints = 238847 * 2.1; // 252 GB XS data

    // Return input struct
    return input;
}

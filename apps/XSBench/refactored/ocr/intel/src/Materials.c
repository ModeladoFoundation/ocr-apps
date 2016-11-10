// Material data is hard coded into the functions in this file.
// Note that there are 12 materials present in H-M (large or small)

#include "XSbench_header.h"

// picks a material based on a probabilistic distribution
int pick_mat( unsigned long * seed )
{
    // I have a nice spreadsheet supporting these numbers. They are
    // the fractions (by volume) of material in the core. Not a
    // *perfect* approximation of where XS lookups are going to occur,
    // but this will do a good job of biasing the system nonetheless.

    // Also could be argued that doing fractions by weight would be
    // a better approximation, but volume does a good enough job for now.

    double dist[12];
    dist[0]  = 0.140;   // fuel
    dist[1]  = 0.052;   // cladding
    dist[2]  = 0.275;   // cold, borated water
    dist[3]  = 0.134;   // hot, borated water
    dist[4]  = 0.154;   // RPV
    dist[5]  = 0.064;   // Lower, radial reflector
    dist[6]  = 0.066;   // Upper reflector / top plate
    dist[7]  = 0.055;   // bottom plate
    dist[8]  = 0.008;   // bottom nozzle
    dist[9]  = 0.015;   // top nozzle
    dist[10] = 0.025;   // top of fuel assemblies
    dist[11] = 0.013;   // bottom of fuel assemblies

    //double roll = (double) rand() / (double) RAND_MAX;
    #ifdef VERIFICATION
    double roll = rn_v();
    #else
    double roll = rn(seed);
    #endif

    // makes a pick based on the distro
    for( int i = 0; i < 12; i++ )
    {
        double running = 0;
        for( int j = i; j > 0; j-- )
            running += dist[j];
        if( roll < running )
            return i;
    }

    return 0;
}

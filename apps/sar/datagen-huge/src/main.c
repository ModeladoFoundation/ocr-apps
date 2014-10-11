/*****************************************************************
 * Synthetic Aperture Radar (SAR) point target simulator.
 *
 * USAGE: datagen.exe <Data output file> <Platform position output
 *        file> <Pulse transmission time output file> <Output file
 *        containing SSCP relevant parameters>
 *
 * NOTES: The code expects Targets.txt and Parameters.txt to be
 * located in the working directory.

 * Each line of Targets.txt represents a bounding box whose
 * location, size, density, and reflectivity are defined within
 * brackets as follows:
 * [UpperLeftCornerX, UpperLeftCornerY, Width, Height,
 * ReflectorSpacing, ReflectorReflectivity]. The two values
 * following the brackets define the first and last image(s) the
 * bounding box appears in. For example, the line "[-15, 10, 15,
 * 20, 0.5, 1] 1 2" generates a 15m by 20m box where the upper left
 * hand corner is located at (-15m, 10m) and populated with
 * reflectors that are spaced 0.5m apart and have a reflectivity
 * of 1. This box appears in images 1 and 2.

 * The Parameters.txt file is described below where the value in
 * parenthesis denotes the corresponding variable used in the code.
 *
 * Fs: Sampling frequency, Hz (fs)
 * Fc: Carrier frequency, Hz (fc)
 * PRF: Pulse Repetition Frequency, Hz (prf)
 * SlantRange: Range from platform to scene center, m (r0)
 * NumberImages: Number of images to simulate (numImages)
 * CrossRangeRes: Cross-range resolution, m (cr)
 * ElevationAngle: Platform elevation angle relative to the ground,
 *    degrees (theta_el)
 * GroundEdgeSize: Ground square edge size, m (edgeSize)
 * PlatformVelocity: Velocity of platform, m/s (v)
 * RangeBinDeviation: Amount of range bin deviation induced in
 *    platform flight path (rngBinDev)
 * BackgroundReflectorSpacingMeters: Spacing of the grid of
 *    background reflectors that appear in all images, m
 *    (backgroundReflSpacing)
 *
 * Written by: Brian Mulvaney, <brian.mulvaney@gtri.gatech.edu>
 *			   Georgia Tech Research Institute
 *             Thomas Benson, <thomas.benson@gtri.gatech.edu>
 *             Georgia Tech Research Institute
 ****************************************************************/

#include "common.h"
#include <assert.h>
#include <stdlib.h>

#define PULSE_GROUP_SIZE 128
#define MASTER_RANK 0

// Radar parameters
float prf;  // Pulse repetition frequency (Hz)
float BW;   // Bandwidth (Hz)
float fs;   // Sampling Frequency (Sa/s)
float fc;   // Carrier frequency (Hz)

// Simulation parameters
double backgroundReflSpacing;
int rngBinDev;      // Range bin deviation (determines the amount of perturbation in platform trajectory)
int numImages;      // Number of images to simulate
float v;            // Platform velocity (m/s)
float r0;           // Range from platform to scene center (m)
float cr;           // Cross-Range resolution (m)
float theta_el;     // Elevation angle (degrees)
float edgeSize;     // Ground square edge size (m)

// Target info
int *target_last;   // Last image target appears in
int *target_first;  // First image target appears in
float *target_x;    // X location of target relative to origin (m)
float *target_y;    // Y location of target relative to origin (m)
float *target_z;    // Z location of target relative to ground plane (m)
float *target_b;    // Amplitude of target

// Malloc variables
int *Xt;        // Ideal spectrum of transmitted LFM signal (after downconversion)
float *y2;      // Pulse compressed signal (output to file)
float *Ux;      // Uniform distribution used for x coordinate
float *Uy;      // Uniform distribution used for y coordinate
float *Uz;      // Uniform distribution used for z coordinate
float *Ux_filt; // Filtered uniform distribution used for x coordinate
float *Uy_filt; // Filtered uniform distribution used for y coordinate
float *Uz_filt; // Filtered uniform distribution used for z coordinate
float *freqVec; // Frequency vector (Hz)

void CleanupAndExit(int exitCode);
void AddBackgroundReflectors(
    Reflector **reflectors,
    int *numBackgroundReflectors,
    double spacing,
    double dr,
    double edgeSize);
void AddTargetReflectors(
    Reflector **reflectors,
    int *numTargetReflectors,
    TargetBoundingBox *bboxes,
    int numBoundingBoxes);

#ifdef USE_MPI
    #include <mpi.h>
    #define MPI_TAG 0
    // Error checking wrapper for debugging.  All MPI errors are considered fatal;
    // MPI does not guarantee that the process can continue after an error.
    #define MPI_ERROR_CHECK(x) \
        do { \
            int mpiReturnCode = x; \
            if (mpiReturnCode != MPI_SUCCESS) \
            { \
                char mpiErrBuf[MPI_MAX_ERROR_STRING]; \
                int mpiErrLen; \
                MPI_Error_string(mpiReturnCode, mpiErrBuf, &mpiErrLen); \
                fprintf(stderr, "MPI Error: %s.\n", mpiErrBuf); \
                CleanupAndExit(1); \
            } \
        } while (0);
#endif // USE_MPI

int main(int argc, char *argv[])
{
    FILE *pFile1;               // Pointer to file containing output data
    FILE *pFile2;               // Pointer to file containing platform positions
    FILE *pFile3;               // Pointer to file containing pulse transmission timestamps
    FILE *pFile4;               // Pointer to file containing SSCP parameters
    fftwf_complex **y;          // Pulse compressed signal (output from IFFT)
    fftwf_complex **Xr;         // Received spectrum after downconversion (input to IFFT)
    fftwf_plan plan_backward[PULSE_GROUP_SIZE]; // FFTW plan variable
    int i, k, l, m, n, g;       // Loop variables
    int Nx;                     // Number of range samples
    int numPulses;              // Number of pulses per image
    int numGroups;              // Number of pulse groups (each group is up to PULSE_GROUP_SIZE)
    double R;                    // Range from platform to target (m)
    float R0;                   // Range of the zeroth range bin (m)
    float w;                    // Platform angular velocity (rad/s)
    float dr;                   // Down-Range resolution (m)
    float *Tp;                  // Pulse transmission timestamp
    float el;                   // Platform eleveation (m)
    float pri;                  // Pulse rate interval (s)
    double t_d;                  // Round trip time delay (s)
    double arg;                  // Argument in complex exponential term
    float h[Nh];                // FIR filter coefficients
    float Rswath;               // Range swath (m)
    float theta, phi;           // Azimuth angle, Polar angle
    float *px, *py, *pz;        // Platform position relative to origin
    Reflector *backgroundReflectors = NULL;
    Reflector *targetReflectors = NULL;
    int numBoundingBoxes;
    int numBackgroundReflectors;
    int numTargetReflectors;

#ifdef USE_MPI
    int mpiRank, mpiNumProcs;
    MPI_ERROR_CHECK(MPI_Init(&argc, &argv));
    MPI_ERROR_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &mpiNumProcs));
    MPI_ERROR_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank));
#else // USE_MPI
    // Even when not using MPI, we assign the process the MPI MASTER_RANK
    // to reduce code duplication later.
    const int mpiRank = MASTER_RANK;
    const int mpiNumProcs = 1;
#endif // USE_MPI

    if(argc != 5) {
        printf("Requisite number of arguments (4) not met.\n"
            "USAGE: datagen.exe <Data output file> <Platform position output file> "
            "<Pulse transmission time output file> <Output file "
            "containing SSCP releveant parameters>\n");
        CleanupAndExit(1);
    }

    // Read parameters
    switch(ReadParams()) {
      case 1:
        printf("Parameters.txt not found. Exiting.\n");
        CleanupAndExit(1);
      case 2:
        printf("Parameters.txt does not adhere to expected format. Exiting.\n");
        CleanupAndExit(1);
    }

    // Read target bounding box information
    TargetBoundingBox *target_bboxes = NULL;
    numBoundingBoxes = ReadTargetBoundingBoxes("Targets.txt", &target_bboxes);

    // Open files for writing
    if (mpiRank == MASTER_RANK)
    {
        pFile1 = fopen(argv[1], "wb");
        assert(pFile1 != NULL);
        pFile2 = fopen(argv[2], "wb");
        assert(pFile2 != NULL);
        pFile3 = fopen(argv[3], "wb");
        assert(pFile3 != NULL);
        pFile4 = fopen(argv[4], "w");
        assert(pFile4 != NULL);
    }

    // Calculate dependent variables
    w = v/r0;
    dr = c/2/fs;
    pri = 1/prf;
    BW = fs;
    Rswath = sqrtf(2.0)*edgeSize*cos(theta_el*M_PI/180.0);
    R0 = r0 - Rswath/2;
    Nx = (int)(fs*2*Rswath/c + 0.5);
    numPulses = (int)ceilf(r0*c*prf/(2*v*cr*fc));
    el = r0*sin(theta_el*M_PI/180.0);
    phi = acos(el/r0);
    numGroups = numPulses / PULSE_GROUP_SIZE;
    if (numPulses % PULSE_GROUP_SIZE != 0) { ++numGroups; }

    srand48(0);

#if 0
numBackgroundReflectors=0;
printf("backgroundReflSpacing = %f\n",backgroundReflSpacing);
#else
    AddBackgroundReflectors(&backgroundReflectors, &numBackgroundReflectors,
        backgroundReflSpacing, dr, edgeSize);
#endif
    // Use half of the background reflector spacing for the target spacing.
    // This means that sparse background reflector spacing will likely correspond
    // to targets with only a few reflectors.
    AddTargetReflectors(&targetReflectors, &numTargetReflectors,
        target_bboxes, numBoundingBoxes);

    Xt = (int*)malloc(Nx*sizeof(int));
    if(Xt == NULL) {
        printf("Error allocating memory for transmitted spectrum (Xt).\n");
        CleanupAndExit(1);
    }

    y2 = (float *)malloc(sizeof(float)*PULSE_GROUP_SIZE*Nx*2);
    if(y2 == NULL) {
        printf("Error allocating memory for pulse compressed signal (y2).\n");
        CleanupAndExit(1);
    }

    freqVec = (float*)malloc(Nx*sizeof(float));
    if(freqVec == NULL) {
        printf("Error allocating memory for frequency vector (freqVec).\n");
        CleanupAndExit(1);
    }

    Ux = (float*)malloc((numPulses+Nh-1)*sizeof(float));
    Uy = (float*)malloc((numPulses+Nh-1)*sizeof(float));
    Uz = (float*)malloc((numPulses+Nh-1)*sizeof(float));
    if(Ux == NULL || Uy == NULL || Uz == NULL) {
        printf("Error allocating memory for uniform distributions.\n");
        CleanupAndExit(1);
    }

    Ux_filt = (float*)malloc(numPulses*sizeof(float));
    Uy_filt = (float*)malloc(numPulses*sizeof(float));
    Uz_filt = (float*)malloc(numPulses*sizeof(float));
    if(Ux_filt == NULL || Uy_filt == NULL || Uz_filt == NULL) {
        printf("Error allocating memory for filtered uniform distributions.\n");
        CleanupAndExit(1);
    }

    px = (float *)malloc(PULSE_GROUP_SIZE*sizeof(float));
    py = (float *)malloc(PULSE_GROUP_SIZE*sizeof(float));
    pz = (float *)malloc(PULSE_GROUP_SIZE*sizeof(float));
    if (px == NULL || py == NULL || pz == NULL) {
        printf("Error allocating memory for platform position.\n");
        CleanupAndExit(1);
    }

    Tp = (float *)malloc(PULSE_GROUP_SIZE*sizeof(float));
    if (Tp == NULL) {
        printf("Error allocating memory for transmission timestamp.\n");
        CleanupAndExit(1);
    }

    // FFTW initialization
    y =  (fftwf_complex **)malloc(PULSE_GROUP_SIZE*sizeof(fftwf_complex *));
    Xr = (fftwf_complex **)malloc(PULSE_GROUP_SIZE*sizeof(fftwf_complex *));
    for (i=0; i<PULSE_GROUP_SIZE; i++)
    {
        y[i]  = (fftwf_complex*)fftwf_malloc(Nx * sizeof(fftwf_complex));
        Xr[i] = (fftwf_complex*)fftwf_malloc(Nx * sizeof(fftwf_complex));
        plan_backward[i] = fftwf_plan_dft_1d(Nx, Xr[i], y[i], FFTW_BACKWARD, FFTW_ESTIMATE);
    }

    // Create moving average filter coefficients
    for(n=0; n<Nh; n++)
        h[n] = (float)1/Nh;

    // Create frequency vector (positive freqs followed by negative freqs)
    freqVec[0] = 0;
    if( !(Nx % 2) )
    {   // Nx even
        for(n=1; n<Nx/2; n++)
        {
            freqVec[n] = n*(fs/Nx);
            freqVec[Nx-n] = -n*(fs/Nx);
        }
        freqVec[Nx/2] = -fs/2;
    }
    else
    {   // Nx odd
        for(n=1; n<=Nx/2; n++)
        {
            freqVec[n] = n*(fs/Nx);
            freqVec[Nx-n] = -n*(fs/Nx);
        }
    }

    // Create ideal spectrum, Xt
    for(n=0; n<Nx; n++)
    {
        if(fabs(freqVec[n]) <= BW/2)
            Xt[n] = 1;
        else
            Xt[n] = 0;
    }

    // Initialize delay buffers
    memset(Ux, 0, (Nh-1)*sizeof(float));
    memset(Uy, 0, (Nh-1)*sizeof(float));
    memset(Uz, 0, (Nh-1)*sizeof(float));

    if (mpiRank == MASTER_RANK)
    {
        printf("Generating data for %d images with %d targets using the following parameters:\n"
            "PRF = %f Hz\n"
            "Slant Range = %f m\n"
            "Cross Range Resolution = %f m\n"
            "Elevation Angle = %f degrees\n"
            "Ground Edge Size = %f m\n"
            "Platform Velocity = %f m/s\n"
            "Range Bin Deviation = %d\n"
            "Pulses Per Image = %d\n"
            "Samples Per Pulse = %d\n",
            numImages, numBackgroundReflectors+numTargetReflectors, prf, r0, cr, theta_el, edgeSize, v, rngBinDev, numPulses, Nx);

#ifdef RAG
        fprintf(pFile4, "Fs %f\n"
            "Fc %f\n"
            "PRF %f\n"
            "P1 %d\n"
            "S1 %d\n"
            "r0 %f\n"
            "R0 %f\n"
            "NumberImages %d\n", fs, fc, prf, numPulses, Nx, r0, R0, numImages);
#else
        fprintf(pFile4, "Fs %f\r\n"
            "Fc %f\r\n"
            "PRF %f\r\n"
            "P1 %d\r\n"
            "S1 %d\r\n"
            "r0 %f\r\n"
            "R0 %f\r\n"
            "NumberImages %d", fs, fc, prf, numPulses, Nx, r0, R0, numImages);
#endif
    }
#ifndef RAG_PARAM_ONLY
    for(k=0; k<numImages; k++)
    {
        if (mpiRank == MASTER_RANK) printf ("Image #%i\n", k);
        // Generate uniform distribution over the interval [-0.5, 0.5]*rngBinDev*dr
        gen_rand(&Ux[Nh-1], 0.5, -0.5, (float)rngBinDev*dr, numPulses);
        gen_rand(&Uy[Nh-1], 0.5, -0.5, (float)rngBinDev*dr, numPulses);
        gen_rand(&Uz[Nh-1], 0.5, -0.5, (float)rngBinDev*dr, numPulses);

        // Filter distributions
        fir_filter(Ux, h, Ux_filt, Nh, numPulses);
        fir_filter(Uy, h, Uy_filt, Nh, numPulses);
        fir_filter(Uz, h, Uz_filt, Nh, numPulses);

        // Shift delay buffer samples from bottom to top
        memcpy(Ux, &Ux[numPulses], (Nh-1)*sizeof(float));
        memcpy(Uy, &Uy[numPulses], (Nh-1)*sizeof(float));
        memcpy(Uz, &Uz[numPulses], (Nh-1)*sizeof(float));

        for (g = mpiRank; g < numGroups; g += mpiNumProcs)
        {
            i = g * PULSE_GROUP_SIZE;
            if (mpiRank == MASTER_RANK)
            {
                printf("%d of %d (%d%%)\r", k*numPulses+i, numImages*numPulses, 100*(k*numPulses+i)/(numImages*numPulses));
                fflush(stdout);
            }

            #pragma omp parallel for private(theta,m,n,R,t_d,arg)
            for(l=i; l<i+PULSE_GROUP_SIZE; l++)
            {
                if (l>=numPulses)
                {
                    continue;
                }

                theta = w*pri*l;    // Calculate azimuth angle
                Tp[l-i] = pri*l;        // Pulse transmission timestamp
                px[l-i] = r0*cos(theta)*sin(phi); // Platform location, x
                py[l-i] = r0*sin(theta)*sin(phi); // Platform location, y
                pz[l-i] = r0*cos(phi);             // Platform location, z

                // Induce perturbation in flight trajectory
                px[l-i] += Ux_filt[l];
                py[l-i] += Uy_filt[l];
                pz[l-i] += Uz_filt[l];

                // Initialize received spectrum, Xr
                memset(Xr[l-i], 0, Nx*sizeof(fftwf_complex));

                for(m=0; m<numBackgroundReflectors+numTargetReflectors; m++)
                {
                    Reflector refl = (m < numBackgroundReflectors) ?
                        backgroundReflectors[m] : targetReflectors[m-numBackgroundReflectors];
                    int is_target_reflector = (m >= numBackgroundReflectors) ? 1 : 0;
                    int is_valid_image = ((k >= (refl.first_image-1)) && (k <= (refl.last_image-1))) ?
                        1 : 0;
                    // We always add target reflectors, but the phase differs depending on whether
                    // the target is currently "valid" or active
#if 0
                    int is_valid_reflector = is_target_reflector || is_valid_image;
                    double phase_offset = (is_target_reflector && is_valid_image) ?
                        refl.phase_offset : 0.0;
                    if(is_valid_reflector) {
#else
		    int is_rag = (k!=0) ? 1 : 0;
                    double phase_offset = (is_target_reflector && is_valid_image && is_rag) ?
                        refl.phase_offset : 0.0;
                    if(is_valid_image) {
#endif
                        // Calculate range from platform to target m
                        R = sqrt( ((double)px[l-i]-refl.x)*((double)px[l-i]-refl.x) +
                            ((double)py[l-i]-refl.y)*((double)py[l-i]-refl.y) +
                            ((double)pz[l-i]-refl.z)*((double)pz[l-i]-refl.z) );

                        // Calculate time delay relative to scene center
                        t_d = 2*(R-r0)/c;

                        // Accumulate received signal
                        for(n=0; n<Nx; n++)
                        {
                            arg = -2*M_PI*t_d*(fc+freqVec[n]);
                            Xr[l-i][n][0] += (float)Xt[n] * refl.refl * cos(arg+phase_offset);
                            Xr[l-i][n][1] += (float)Xt[n] * refl.refl * sin(arg+phase_offset);
                        }
                    }
                }

                // Perform IFFT
                fftwf_execute(plan_backward[l-i]);

                // Normalize IFFT output and swap first and second halves
                for(m=(int)ceilf(Nx/2), n=0; m<Nx; m++, n++) {
                    y2[2*((l-i)*Nx+n)+0] = y[l-i][m][0]/Nx;
                    y2[2*((l-i)*Nx+n)+1] = y[l-i][m][1]/Nx;
                }
                for(m=0; m<(int)ceilf(Nx/2); m++, n++) {
                    y2[2*((l-i)*Nx+n)+0] = y[l-i][m][0]/Nx;
                    y2[2*((l-i)*Nx+n)+1] = y[l-i][m][1]/Nx;
                }

                // Induce affine misregistration in second image
                if(k == 1) {
                    px[l-i] += 8.4 * dr;
                    py[l-i] += 8.4 * dr;
                }

            } // for l

            // Write the results to file for the master rank
            if (mpiRank == MASTER_RANK)
            {
                int nelem = (numPulses - i < PULSE_GROUP_SIZE) ? numPulses - i : PULSE_GROUP_SIZE;
                int p;
                fwrite(y2, sizeof(float), nelem * Nx * 2, pFile1);
                for (p = 0; p < nelem; ++p)
                {
                    fwrite(px+p, sizeof(float), 1, pFile2);
                    fwrite(py+p, sizeof(float), 1, pFile2);
                    fwrite(pz+p, sizeof(float), 1, pFile2);
                }
                fwrite(Tp, sizeof(float), nelem, pFile3);
            }

#ifdef USE_MPI
            // Communicate this pulse group back to the master node for writing
            if (mpiRank == MASTER_RANK)
            {
                int nelem, node, p;
                MPI_Status status;
                for (node = 1; node < mpiNumProcs; ++node)
                {
                    if (g + node >= numGroups) { continue; }

                    MPI_ERROR_CHECK(MPI_Recv(&nelem, 1, MPI_INT, node,
                        MPI_TAG, MPI_COMM_WORLD, &status));
                    MPI_ERROR_CHECK(MPI_Recv(y2, nelem*Nx*2, MPI_FLOAT, node,
                        MPI_TAG, MPI_COMM_WORLD, &status));
                    MPI_ERROR_CHECK(MPI_Recv(px, nelem, MPI_FLOAT, node,
                        MPI_TAG, MPI_COMM_WORLD, &status));
                    MPI_ERROR_CHECK(MPI_Recv(py, nelem, MPI_FLOAT, node,
                        MPI_TAG, MPI_COMM_WORLD, &status));
                    MPI_ERROR_CHECK(MPI_Recv(pz, nelem, MPI_FLOAT, node,
                        MPI_TAG, MPI_COMM_WORLD, &status));
                    MPI_ERROR_CHECK(MPI_Recv(Tp, nelem, MPI_FLOAT, node,
                        MPI_TAG, MPI_COMM_WORLD, &status));
                    //printf("Writing results for group %d\n", g+node);
                    fwrite(y2, sizeof(float), nelem * Nx * 2, pFile1);
                    for (p = 0; p < nelem; ++p)
                    {
                        fwrite(px+p, sizeof(float), 1, pFile2);
                        fwrite(py+p, sizeof(float), 1, pFile2);
                        fwrite(pz+p, sizeof(float), 1, pFile2);
                    }
                    fwrite(Tp, sizeof(float), nelem, pFile3);
                }
            }
            else // worker node
            {
                int nelem = (numPulses - i < PULSE_GROUP_SIZE) ? numPulses - i : PULSE_GROUP_SIZE;
                MPI_ERROR_CHECK(MPI_Send(&nelem, 1, MPI_INT, MASTER_RANK,
                    MPI_TAG, MPI_COMM_WORLD));
                MPI_ERROR_CHECK(MPI_Send(y2, nelem*Nx*2, MPI_FLOAT, MASTER_RANK,
                    MPI_TAG, MPI_COMM_WORLD));
                MPI_ERROR_CHECK(MPI_Send(px, nelem, MPI_FLOAT, MASTER_RANK,
                    MPI_TAG, MPI_COMM_WORLD));
                MPI_ERROR_CHECK(MPI_Send(py, nelem, MPI_FLOAT, MASTER_RANK,
                    MPI_TAG, MPI_COMM_WORLD));
                MPI_ERROR_CHECK(MPI_Send(pz, nelem, MPI_FLOAT, MASTER_RANK,
                    MPI_TAG, MPI_COMM_WORLD));
                MPI_ERROR_CHECK(MPI_Send(Tp, nelem, MPI_FLOAT, MASTER_RANK,
                    MPI_TAG, MPI_COMM_WORLD));
            }
#endif // USE_MPI

        } // for i
    } // for k
#endif
    // Close files
    if (mpiRank == MASTER_RANK)
    {
        fclose(pFile1);
        fclose(pFile2);
        fclose(pFile3);
        fclose(pFile4);
    }

    if (Xt) free(Xt);
    if (freqVec) free(freqVec);
    if (Ux) free(Ux);
    if (Uy) free(Uy);
    if (Uz) free(Uz);
    if (Ux_filt) free(Ux_filt);
    if (Uy_filt) free(Uy_filt);
    if (Uz_filt) free(Uz_filt);
    if (px) free(px);
    if (py) free(py);
    if (pz) free(pz);
    if (Tp) free(Tp);

    // Free memory and deallocate plan
    for (i=0; i<PULSE_GROUP_SIZE; i++)
    {
        if (y && y[i]) fftwf_free(y[i]);
        if (Xr && Xr[i]) fftwf_free(Xr[i]);
        if (plan_backward[i]) fftwf_destroy_plan(plan_backward[i]);
    }

    if (y) free(y);
    if (Xr) free(Xr);

    if (targetReflectors) { free(targetReflectors); }
    if (backgroundReflectors) { free(backgroundReflectors); }

    CleanupAndExit(0);

    return 0;
}

void CleanupAndExit(int exitCode)
{
#ifdef USE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
#endif // USE_MPI
    exit(exitCode);
}

int GetNumLinearBackgroundReflectorsBySpacing(
    double spacing,
    float edgeSize)
{
    return (int) ceil(edgeSize / spacing);
}

void GetNumReflectorsByBoundingBox(
    int *width,
    int *height,
    TargetBoundingBox bbox)
{
    assert(width != NULL);
    assert(height != NULL);

    *width = (int) ceil(bbox.width/bbox.spacing);
    *height = (int) ceil(bbox.height/bbox.spacing);
}

void AddTargetReflectors(
    Reflector **reflectors,
    int *numTargetReflectors,
    TargetBoundingBox *bboxes,
    int numBoundingBoxes)
{
    int width, height, i, w, h, n;
    assert(reflectors != NULL);
    assert(bboxes != NULL);

    *numTargetReflectors = 0;
    for (i = 0; i < numBoundingBoxes; ++i)
    {
        GetNumReflectorsByBoundingBox(&width, &height, bboxes[i]);
        *numTargetReflectors += width * height;
    }

    *reflectors = (Reflector *) malloc(sizeof(Reflector)*(*numTargetReflectors));
    if (*reflectors == NULL)
    {
        printf("Error allocating memory for target reflectors.\n");
        CleanupAndExit(1);
    }

    n = 0;
    for (i = 0; i < numBoundingBoxes; ++i)
    {
        Reflector *refl = *reflectors;
        const double spacing = bboxes[i].spacing;
        GetNumReflectorsByBoundingBox(&width, &height, bboxes[i]);
        for (w = 0; w < width; ++w)
        {
            for (h = 0; h < height; ++h)
            {
                refl[n].x = bboxes[i].tlhc_x + w * spacing +
                    drand48() * spacing - spacing/2.0;
                refl[n].y = bboxes[i].tlhc_y - h * spacing +
                    drand48() * spacing - spacing/2.0;
                refl[n].z = 0.0;
                refl[n].refl = bboxes[i].reflectivity;
                refl[n].phase_offset = drand48() * 2.0 * M_PI;
                refl[n].first_image = bboxes[i].firstImage;
                refl[n].last_image = bboxes[i].lastImage;
                ++n;
            }
        }
    }
}

void AddBackgroundReflectors(
    Reflector **reflectors,
    int *numBackgroundReflectors,
    double spacing,
    double dr,
    double edgeSize)
{
    int i, j;

    assert(reflectors != NULL);
    assert(numBackgroundReflectors != NULL);

    const int numLinearRefl = GetNumLinearBackgroundReflectorsBySpacing(
        spacing, edgeSize);
    *reflectors = (Reflector *) malloc(sizeof(Reflector) *
        numLinearRefl * numLinearRefl);
    if (*reflectors == NULL)
    {
        printf("Error allocating memory for background reflectors.\n");
        CleanupAndExit(1);
    }

    *numBackgroundReflectors = numLinearRefl * numLinearRefl;
    memset(*reflectors, 0, sizeof(Reflector) * numLinearRefl * numLinearRefl);

    for (i = 0; i < numLinearRefl; ++i)
    {
        Reflector *refl = *reflectors;
        const float y = -edgeSize/2.0 + i * spacing;;
        for (j = 0; j < numLinearRefl; ++j)
        {
            const float x = -edgeSize/2.0 + j * spacing;
            const float x_off = spacing * (drand48() - 0.5);
            const float y_off = spacing * (drand48() - 0.5);

            refl[i*numLinearRefl+j].x = x + x_off;
            refl[i*numLinearRefl+j].y = y + y_off;
            refl[i*numLinearRefl+j].z = 0.0;
            refl[i*numLinearRefl+j].refl = 1.0;
            refl[i*numLinearRefl+j].phase_offset = 0.0;
            refl[i*numLinearRefl+j].first_image = 1;
            refl[i*numLinearRefl+j].last_image = numImages;
        }
    }
}

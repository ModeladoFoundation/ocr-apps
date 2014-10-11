/*****************************************************************
 * Streaming Sensor Challenge Problem (SSCP) reference
 * implementation.
 *
 * USAGE: SSCP.exe <Data input file> <Platform position input
 *        file> <Pulse transmission time input file> <Output
 *        file containing detects>
 *
 * NOTES: The code expects Parameters.txt to be located in the
 * working directory. The Parameters.txt file is
 * described below.
 *
 * Fs: Sample frequency, Hz
 * Fc: Carrier frequency, Hz
 * PRF: Pulse Repetition Frequency, Hz
 * F: Oversampling factor in down-range
 * Ix: Number of pixels in x dimension of full image, pixels
 * Iy: Number of pixels in y dimension of full image, pixels
 * Sx: Number of pixels in x dimension of subimage, pixels
 * Sy: Number of pixels in y dimension of subimage, pixels
 * Fbp: Upsampling factor in backprojection
 * P1: Number of pulses per image
 * S1: Number of samples per pulse
 * r0: Range from platform to scene center, m
 * R0: Range of the zeroth range bin, m
 * Nc: Number of registration control points
 * Rc: Range of registration
 * Sc: Registration neighborhood size
 * Tc: Registration correlation threshold
 * Ncor: Coherent Change Detection (CCD) neighborhood size
 * Ncfar: Constant false alarm rate (CFAR) neighborhood size
 * Tcfar: CFAR threshold
 * Nguard: Number of guard cells for CFAR neighborhood
 * NumberImages: Number of images to processs
 *
 * Written by: Brian Mulvaney, <brian.mulvaney@gtri.gatech.edu>
 *             Georgia Tech Research Institute
 *             Thomas Benson, <thomas.benson@gtri.gatech.edu>
 *             Georgia Tech Research Institute
 ****************************************************************/

#include "common.h"
#include "assert.h"

#ifdef SSCP_DEBUG
#define WRITE_CORRELATION_MAP
#define WRITE_FORMED_IMAGES
#endif

struct Inputs in;                   // Inputs
struct detects *Y;                  // Detects list
struct point **corr_map;            // Correlation map
struct DigSpotVars dig_spot;        // Digital spotlight variables
struct complexData **curImage;      // Current image
struct complexData **refImage;      // Reference (previous) image
#ifdef RAG
struct complexData **regImage = NULL;      // Current registered image non-upsampled
#endif
struct complexData **curImage2;     // Current image upsampled by Fbp
struct complexData **regImage2;     // Current registered image upsampled by Fbp
struct complexData **refImage2;     // Reference (previous) image upsampled by Fbp
struct CfarParams cfar_params;      // CFAR parameters
struct RadarParams radar_params;    // Radar parameters
struct ImageParams image_params;    // Image parameters
struct RegistrationParams registration_params;  // Registration parameters

int main(int argc, char *argv[])
{
    int m, n;
    int Nd;
    FILE *pInFile, *pInFile2, *pInFile3;
    FILE *pOutFile;

    if (argc != 5)
    {
        printf("Usage: %s <phase-history-file> <platform-position-file> "
               "<pulse-timestamp-file> <detection-output-file>\n", argv[0]);
        exit(-1);
    }

    // SAR data
    if( (pInFile = fopen(argv[1], "rb")) == NULL ) {
        printf("Error opening %s\n", argv[1]);
        exit(1);
    }

    // Platform positions
    if( (pInFile2 = fopen(argv[2], "rb")) == NULL ) {
        printf("Error opening %s\n", argv[2]);
        exit(1);
    }

    // Pulse transmission timestamps
    if( (pInFile3 = fopen(argv[3], "rb")) == NULL ) {
        printf("Error opening %s\n", argv[3]);
        exit(1);
    }

    if( (pOutFile = fopen(argv[4], "wb")) == NULL ) {
        printf("Error opening %s\n", argv[4]);
        exit(1);
    }

    switch(ReadParams(&radar_params, &image_params, &registration_params, &cfar_params)) {
      case 1:
          printf("Unable to open Parameters.txt\n");
          exit(1);
      case 2:
          printf("Parameters.txt does not adhere to expected format.\n");
          exit(1);
    }

    // Ensure all window sizes are odd
    if( !(registration_params.Sc % 2) ) {
        printf("Sc must be odd. Exiting.\n");
        exit(-1);
    }

    if( !(image_params.Ncor % 2) ) {
        printf("Ncor must be odd. Exiting.\n");
        exit(-1);
    }

    if( !(cfar_params.Ncfar % 2) ) {
        printf("Ncfar must be odd. Exiting.\n");
        exit(-1);
    }

    if( !(cfar_params.Nguard % 2) ) {
        printf("Nguard must be odd. Exiting.\n");
        exit(-1);
    }

    // Calculate dependent variables
    image_params.TF = image_params.Ix/image_params.Sx;
#ifdef RAG_PURE_FLOAT
    image_params.dr = SPEED_OF_LIGHT/radar_params.fs/2.0f/image_params.F;
    image_params.dx = SPEED_OF_LIGHT/radar_params.fs/2.0f;
    image_params.dx2 = SPEED_OF_LIGHT/radar_params.fs/2.0f/image_params.Fbp;
    image_params.dy = SPEED_OF_LIGHT/radar_params.fs/2.0f;
    image_params.dy2 = SPEED_OF_LIGHT/radar_params.fs/2.0f/image_params.Fbp;
#else
    image_params.dr = SPEED_OF_LIGHT/radar_params.fs/2.0/image_params.F;
    image_params.dx = SPEED_OF_LIGHT/radar_params.fs/2.0;
    image_params.dx2 = SPEED_OF_LIGHT/radar_params.fs/2.0/image_params.Fbp;
    image_params.dy = SPEED_OF_LIGHT/radar_params.fs/2.0;
    image_params.dy2 = SPEED_OF_LIGHT/radar_params.fs/2.0/image_params.Fbp;
#endif
    image_params.P2 = image_params.P1/image_params.TF;
    image_params.S2 = image_params.S1/image_params.TF;
    image_params.Ix2 = image_params.Ix * image_params.Fbp;
    image_params.Iy2 = image_params.Iy * image_params.Fbp;
    image_params.Sx2 = image_params.Sx * image_params.Fbp;
    image_params.Sy2 = image_params.Sy * image_params.Fbp;

    // TF > 1 implies digital spotlighting, TF = 1 implies no digital spotlighting
    if(image_params.TF > 1) {
        image_params.P3 = image_params.P2;
        image_params.S3 = image_params.S2;

        // Calculate new zeroth range bin
        radar_params.R0_prime = radar_params.r0 - (radar_params.r0 - radar_params.R0)/image_params.TF;

        // Allocate memory for variables needed to perform digital spotlighting
        dig_spot.freqVec = (float*)MALLOC_CHECKED(image_params.S1*sizeof(float));
        dig_spot.filtOut = (struct complexData*)MALLOC_CHECKED(image_params.P2*sizeof(struct complexData));
        dig_spot.X2 = (struct complexData**)MALLOC_CHECKED(image_params.P1*sizeof(struct complexData*));
        for(n=0; n<image_params.P1; n++) {
            dig_spot.X2[n] = (struct complexData*)MALLOC_CHECKED(image_params.S1*sizeof(struct complexData));
        }

        dig_spot.X3 = (struct complexData**)MALLOC_CHECKED(image_params.P1*sizeof(struct complexData*));
        for(n=0; n<image_params.P1; n++) {
            dig_spot.X3[n] = (struct complexData*)MALLOC_CHECKED(image_params.S2*sizeof(struct complexData));
        }

        dig_spot.X4 = (struct complexData**)MALLOC_CHECKED(image_params.P2*sizeof(struct complexData*));
        for(n=0; n<image_params.P2; n++) {
            dig_spot.X4[n] = (struct complexData*)MALLOC_CHECKED(image_params.S2*sizeof(struct complexData));
        }

        dig_spot.tmpVector = (struct complexData*)MALLOC_CHECKED(image_params.P1*sizeof(struct complexData));

        dig_spot.Pt2 = (float**)MALLOC_CHECKED(image_params.P2*sizeof(float*));
        for(n=0; n<image_params.P2; n++) {
            dig_spot.Pt2[n] = (float*)MALLOC_CHECKED(3*sizeof(float));
        }

        // Create frequency vector (positive freqs followed by negative freqs)
        dig_spot.freqVec[0] = 0;
        if( !(image_params.S1 % 2) )
        {   // S1 even
            for(n=1; n<image_params.S1/2; n++)
            {
                dig_spot.freqVec[n] = n*(radar_params.fs/image_params.S1);
                dig_spot.freqVec[image_params.S1-n] = -n*(radar_params.fs/image_params.S1);
            }
            dig_spot.freqVec[image_params.S1/2] = -radar_params.fs/2;
        }
        else
        {   // S1 odd
            for(n=1; n<=image_params.S1/2; n++)
            {
                dig_spot.freqVec[n] = n*(radar_params.fs/image_params.S1);
                dig_spot.freqVec[image_params.S1-n] = -n*(radar_params.fs/image_params.S1);
            }
        }
    }
    else {
        image_params.P3 = image_params.P1;
        image_params.S3 = image_params.S1;
    }

    image_params.S4 = (int)ceilf(image_params.F*image_params.S3);

    // Allocate memory for axis vectors
    image_params.xr = (float*)MALLOC_CHECKED(image_params.Ix*sizeof(float));
    image_params.yr = (float*)MALLOC_CHECKED(image_params.Iy*sizeof(float));
    image_params.xr2 = (float*)MALLOC_CHECKED(image_params.Ix2*sizeof(float));
    image_params.yr2 = (float*)MALLOC_CHECKED(image_params.Iy2*sizeof(float));

    // Create axis vectors
    for(n=0; n<image_params.Ix; n++) {
        image_params.xr[n] = (n - floorf((float)image_params.Ix/2.0))*image_params.dx;
    }
    for(n=0; n<image_params.Iy; n++) {
        image_params.yr[n] = (n - floorf((float)image_params.Iy/2.0))*image_params.dy;
    }
    for(n=0; n<image_params.Ix2; n++) {
        image_params.xr2[n] = (n - floorf((float)image_params.Ix2/2.0))*image_params.dx2;
    }
    for(n=0; n<image_params.Iy2; n++) {
        image_params.yr2[n] = (n - floorf((float)image_params.Iy2/2.0))*image_params.dy2;
    }

    // Allocate memory for pulse compressed SAR data
    in.X = (struct complexData**)MALLOC_CHECKED(image_params.P1*sizeof(struct complexData*));
    for(n=0; n<image_params.P1; n++) {
        in.X[n] = (struct complexData*)MALLOC_CHECKED(image_params.S1*sizeof(struct complexData));
    }

    // Allocate memory for transmitter positions at each pulse
    in.Pt = (float**)MALLOC_CHECKED(image_params.P1*sizeof(float*));
    for(n=0; n<image_params.P1; n++) {
        in.Pt[n] = (float*)MALLOC_CHECKED(3*sizeof(float));
    }

    // Allocate memory for timestamp of pulse transmissions
    in.Tp = (float*)MALLOC_CHECKED(image_params.P1*sizeof(float));

    // Allocate memory for current image
    curImage = (struct complexData**)MALLOC_CHECKED(image_params.Iy*sizeof(struct complexData*));
    for(n=0; n<image_params.Iy; n++) {
        curImage[n] = (struct complexData*)MALLOC_CHECKED(image_params.Ix*sizeof(struct complexData));
    }

#ifdef RAG
    if( (image_params.F == 1) && (image_params.Fbp == 1) )  {
        curImage2 = curImage;
    } else {
        // Allocate memory for upsampled current image
        curImage2 = (struct complexData**)MALLOC_CHECKED(image_params.Iy2*sizeof(struct complexData*));
        for(n=0; n<image_params.Iy2; n++) {
            curImage2[n] = (struct complexData*)MALLOC_CHECKED(image_params.Ix2*sizeof(struct complexData));
        }
    }
#else
    // Allocate memory for upsampled current image
    curImage2 = (struct complexData**)MALLOC_CHECKED(image_params.Iy2*sizeof(struct complexData*));
    for(n=0; n<image_params.Iy2; n++) {
        curImage2[n] = (struct complexData*)MALLOC_CHECKED(image_params.Ix2*sizeof(struct complexData));
    }
#endif

#ifdef RAG
    if( (image_params.F == 1) && (image_params.Fbp == 1) )  {
        // Allocate memory for registered version of current non-upsampled image
        regImage = (struct complexData**)MALLOC_CHECKED(image_params.Iy*sizeof(struct complexData*));
        for(n=0; n<image_params.Iy; n++) {
            regImage[n] = (struct complexData*)MALLOC_CHECKED(image_params.Ix*sizeof(struct complexData));
        }
        regImage2 = regImage;
    } else {
        // Allocate memory for registered version of current upsampled image
        regImage2 = (struct complexData**)MALLOC_CHECKED(image_params.Iy2*sizeof(struct complexData*));
        for(n=0; n<image_params.Iy2; n++) {
            regImage2[n] = (struct complexData*)MALLOC_CHECKED(image_params.Ix2*sizeof(struct complexData));
        }
    }
#else
    // Allocate memory for registered version of current upsampled image
    regImage2 = (struct complexData**)MALLOC_CHECKED(image_params.Iy2*sizeof(struct complexData*));
    for(n=0; n<image_params.Iy2; n++) {
        regImage2[n] = (struct complexData*)MALLOC_CHECKED(image_params.Ix2*sizeof(struct complexData));
    }
#endif

    // Allocate memory for reference image
    refImage = (struct complexData**)MALLOC_CHECKED(image_params.Iy*sizeof(struct complexData*));
    for(n=0; n<image_params.Iy; n++) {
        refImage[n] = (struct complexData*)MALLOC_CHECKED(image_params.Ix*sizeof(struct complexData));
    }


#ifdef RAG
    if( (image_params.F == 1) && (image_params.Fbp == 1) )  {
        refImage2 = refImage;
    } else {
        // Allocate memory for upsampled reference image
        refImage2 = (struct complexData**)MALLOC_CHECKED(image_params.Iy2*sizeof(struct complexData*));
        for(n=0; n<image_params.Iy2; n++) {
            refImage2[n] = (struct complexData*)MALLOC_CHECKED(image_params.Ix2*sizeof(struct complexData));
        }
    }
#else
    // Allocate memory for upsampled reference image
    refImage2 = (struct complexData**)MALLOC_CHECKED(image_params.Iy2*sizeof(struct complexData*));
    for(n=0; n<image_params.Iy2; n++) {
        refImage2[n] = (struct complexData*)MALLOC_CHECKED(image_params.Ix2*sizeof(struct complexData));
    }
#endif

    // Allocate memory for correlation map
    corr_map = (struct point**)MALLOC_CHECKED((image_params.Iy-image_params.Ncor+1)*sizeof(struct point*));
    for(m=0; m<image_params.Iy-image_params.Ncor+1; m++)
    {
        corr_map[m] = (struct point*)MALLOC_CHECKED((image_params.Ix-image_params.Ncor+1)*sizeof(struct point));
    }

    // Allocate memory for detection list
    Y = (struct detects*)MALLOC_CHECKED((image_params.Iy-image_params.Ncor-cfar_params.Ncfar+2)*
        (image_params.Ix-image_params.Ncor-cfar_params.Ncfar+2)*sizeof(struct detects));

#ifdef RAG_TRACE
    printf("// Read input data\n");
#endif
    ReadData(pInFile, pInFile2, pInFile3, &in, &image_params);

#ifdef RAG_TRACE
    printf("// Form first image\n");
#endif
    FormImage(&dig_spot, &image_params, &in, curImage2, &radar_params);

    while(--image_params.numImages)
    {
#ifdef RAG_TRACE
    printf("// Read input data\n");
#endif
        ReadData(pInFile, pInFile2, pInFile3, &in, &image_params);

#ifdef RAG_TRACE
        printf("// Copy previous current image to reference image\n");
#endif
        for(m=0; m<image_params.Iy2; m++) {
            memcpy(&refImage2[m][0], &curImage2[m][0], image_params.Ix2*sizeof(struct complexData));
        }

#ifdef RAG_TRACE
        printf("// Form current image\n");
#endif
        FormImage(&dig_spot, &image_params, &in, curImage2, &radar_params);

        // Copy the current upsampled image into a temporary used for registration.  The
        // unregistered version of the current image will become the reference image
        // on the next pass.
#ifdef RAG
        if( ( image_params.numImages == 1 ) && ( image_params.F == 1 ) && ( image_params.Fbp == 1 ) ) {
#ifdef RAG_TRACE
            printf("retImage2 == curImage\n");
#endif
            regImage2 = curImage;
        } else {
#ifdef RAG_TRACE
            printf("retImage2 == curImage2\n");
#endif
            for(m=0; m<image_params.Iy2; m++) {
                memcpy(&regImage2[m][0], &curImage2[m][0], image_params.Ix2*sizeof(struct complexData));
            }
        }
#else
        for(m=0; m<image_params.Iy2; m++) {
            memcpy(&regImage2[m][0], &curImage2[m][0], image_params.Ix2*sizeof(struct complexData));
        }
#endif

#ifdef RAG_TRACE
        printf("// Registration\n");
#endif
        Registration(&registration_params, &image_params, regImage2, refImage2);

#ifdef RAG
        if( (image_params.numImages!=1) || (image_params.F != 1) || (image_params.Fbp != 1) ) {
#ifdef RAG_TRACE
            printf("// Downsample both images by the Fbp upsampling factor %d %d %d\n",image_params.numImages,image_params.F,image_params.Fbp);
#endif
            Downsample(&image_params, curImage, refImage, regImage2, refImage2);
        }
#else
#ifdef RAG_TRACE
        printf("// Downsample both images by the Fbp upsampling factor\n");
#endif
        Downsample(&image_params, curImage, refImage, regImage2, refImage2);
#endif

#ifdef RAG_TRACE
        printf("// Coherent Change Detection\n");
#endif
        CCD(image_params.Ncor, corr_map, &image_params, curImage, refImage);

#ifdef RAG_TRACE
        printf("// Constant False Alarm Rate\n");
#endif
        Nd = CFAR(&image_params, image_params.Ncor, &cfar_params, corr_map, Y);

        // Output to file
        for(m=0; m<Nd; m++) {
#ifdef RAG
            fprintf(pOutFile, "(x=%7.2f m, y=%7.2f m, p=%4.2f)\n", Y[m].x, Y[m].y, Y[m].p);
#else
            fprintf(pOutFile, "(x=%7.2f m, y=%7.2f m, p=%4.2f)\r\n", Y[m].x, Y[m].y, Y[m].p);
#endif
        }
    }

#ifdef WRITE_FORMED_IMAGES
    {
        FILE *pOutImg = fopen("images_debug.bin", "wb");
        assert(pOutImg != NULL);

        for(m=0; m<image_params.Iy; m++)
            fwrite(&curImage[m][0], sizeof(struct complexData), image_params.Ix, pOutImg);
        for(m=0; m<image_params.Iy; m++)
            fwrite(&refImage[m][0], sizeof(struct complexData), image_params.Ix, pOutImg);

        fclose(pOutImg);
    }
#endif

#ifdef WRITE_CORRELATION_MAP
    {
        FILE *pOutCorr = fopen("corr_debug.bin", "wb");
        assert(pOutCorr != NULL);
        for(m=0; m<image_params.Iy-image_params.Ncor+1; m++)
            fwrite(&corr_map[m][0], sizeof(struct point), image_params.Ix-image_params.Ncor+1, pOutCorr);
        fclose(pOutCorr);
    }
#endif

    fclose(pInFile);
    fclose(pInFile2);
    fclose(pInFile3);
    fclose(pOutFile);

    if(image_params.TF > 1) {
        FREE_AND_NULL(dig_spot.freqVec);
        FREE_AND_NULL(dig_spot.filtOut);
        for(n=0; n<image_params.P1; n++) {
            FREE_AND_NULL(dig_spot.X2[n]);
            FREE_AND_NULL(dig_spot.X3[n]);
        }
        FREE_AND_NULL(dig_spot.X2);
        FREE_AND_NULL(dig_spot.X3);
        for(n=0; n<image_params.P2; n++) {
            FREE_AND_NULL(dig_spot.X4[n]);
            FREE_AND_NULL(dig_spot.Pt2[n]);
        }
        FREE_AND_NULL(dig_spot.X4);
        FREE_AND_NULL(dig_spot.tmpVector);
        FREE_AND_NULL(dig_spot.Pt2);
    }

    FREE_AND_NULL(image_params.xr);
    FREE_AND_NULL(image_params.yr);
    FREE_AND_NULL(image_params.xr2);
    FREE_AND_NULL(image_params.yr2);

    for(n=0; n<image_params.P1; n++) {
        FREE_AND_NULL(in.X[n]);
        FREE_AND_NULL(in.Pt[n]);
    }
    FREE_AND_NULL(in.X);
    FREE_AND_NULL(in.Pt);
    FREE_AND_NULL(in.Tp);

#ifdef RAG
    for(n=0; n<image_params.Iy2; n++) {
        if ( ( curImage2 != NULL ) && ( curImage2 != curImage ) ) FREE_AND_NULL(curImage2[n]);
        if ( ( refImage2 != NULL ) && ( refImage2 != refImage ) ) FREE_AND_NULL(refImage2[n]);
        if ( ( regImage2 != NULL ) && ( ( regImage2 != regImage ) && ( regImage2 != curImage ) ) ) FREE_AND_NULL(regImage2[n]);
    }
    if ( ( regImage2 != NULL ) && ( ( regImage2 != regImage ) && ( regImage2 != curImage ) ) ) FREE_AND_NULL(regImage2);
    if ( ( refImage2 != NULL ) && ( refImage2 != refImage ) ) FREE_AND_NULL(refImage2);
    if ( ( curImage2 != NULL ) && ( curImage2 != curImage ) ) FREE_AND_NULL(curImage2);

    for(n=0; n<image_params.Iy; n++) {
        if ( curImage != NULL ) FREE_AND_NULL(curImage[n]);
        if ( refImage != NULL ) FREE_AND_NULL(refImage[n]);
        if ( ( regImage != NULL ) && ( regImage != curImage ) ) FREE_AND_NULL(regImage[n]);
    }
    if ( ( regImage != NULL ) && ( regImage != curImage) ) FREE_AND_NULL(regImage);
    if ( refImage != NULL ) FREE_AND_NULL(refImage);
    if ( curImage != NULL ) FREE_AND_NULL(curImage);
#else
    for(n=0; n<image_params.Iy; n++) {
        FREE_AND_NULL(curImage[n]);
        FREE_AND_NULL(refImage[n]);
    }
    FREE_AND_NULL(curImage);
    FREE_AND_NULL(refImage);

    for(n=0; n<image_params.Iy2; n++) {
        FREE_AND_NULL(curImage2[n]);
        FREE_AND_NULL(refImage2[n]);
        FREE_AND_NULL(regImage2[n]);
    }
    FREE_AND_NULL(curImage2);
    FREE_AND_NULL(refImage2);
    FREE_AND_NULL(regImage2);
#endif

    for(m=0; m<image_params.Iy-image_params.Ncor+1; m++) {
        FREE_AND_NULL(corr_map[m]);
    }
    FREE_AND_NULL(corr_map);
    FREE_AND_NULL(Y);

    return 0;
}

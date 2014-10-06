#ifndef _COMMON_H_
#define _COMMON_H_

#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <fftw3.h>
#include <stdlib.h>
#include <string.h>

typedef struct _TargetBoundingBox
{
    /* tlhc is the top-left hand corner of a bounding box.  width
     * and height correspond to the extents in x and y, respectively.
     */
    double tlhc_x;
    double tlhc_y;
    double width;
    double height;
    double spacing;
    double reflectivity;
    int firstImage;
    int lastImage;
} TargetBoundingBox;

typedef struct _Reflector
{
    double x;
    double y;
    double z;
    double refl;
    double phase_offset;
    /* [first_image, last_image] defines the range of images for which
     * this reflector is valid.
     */
    int first_image;
    int last_image;
} Reflector;

// Constants
#define c 3e8				// Speed of light (m/s)
#define Nh 33				// Number of FIR filter coefficients

// Function prototypes
int ReadParams();
//int ReadTargets();
void fir_filter(float *x, float *h, float *y, int nh, int nr);
void gen_rand(float *x, float max, float min, float scale, int nx);
int ReadTargetBoundingBoxes(
    const char *targets_filename,
    TargetBoundingBox **target_bboxes);

extern double backgroundReflSpacing;
extern int rngBinDev;
extern int numImages;
//extern int numTargets;
extern int *target_last;
extern int *target_first;
extern float v;
extern float r0;
extern float cr;
extern float fs;
extern float fc;
extern float prf;
extern float theta_el;
extern float edgeSize;
extern float *target_x;
extern float *target_y;
extern float *target_z;
extern float *target_b;

#endif // _COMMON_H_

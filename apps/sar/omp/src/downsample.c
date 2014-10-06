#include <assert.h>

#include "common.h"

void Downsample(
    const struct ImageParams *image_params,
    struct complexData **curImage,
    struct complexData **refImage,
    struct complexData ** curImage2,
    struct complexData ** refImage2)
{
    const int Ix = image_params->Ix;
    const int Iy = image_params->Iy;
    const int Fbp = image_params->Fbp;
    int i, j;

    for (i = 0; i < Iy; ++i)
    {
        for (j = 0; j < Ix; ++j)
        {
            refImage[i][j] = refImage2[i*Fbp][j*Fbp];
            curImage[i][j] = curImage2[i*Fbp][j*Fbp];
        }
    }
}

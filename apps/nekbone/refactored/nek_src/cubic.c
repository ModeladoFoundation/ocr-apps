#ifndef NEKBONE_CUBIC_H
#include "cubic.h"
#endif // NEKBONE_CUBIC_H

#include <math.h>

int nek_cubic(unsigned long * o_x,
              unsigned long * o_y,
              unsigned long * o_z,
              unsigned long in_value
              )
{
    int err=0;
    while(!err){
        *o_x = in_value;
        *o_y = 1;
        *o_z = 1;

        double ratio = in_value;

        unsigned long iroot3 = (unsigned long) (pow(in_value, ((double)1/3)) + 0.000001);
        unsigned long i;
        for(i=iroot3; i>0; --i){
            unsigned long ix, iy;
            unsigned long iz = i;
            unsigned long myx = in_value / iz;
            unsigned long nrem = in_value - myx * iz;

            if( nrem == 0 ){
                unsigned long iroot2 = (unsigned long)(pow(myx,((double)1/2)) + 0.000001);
                unsigned long j;
                for(j=iroot2; j>0; --j){
                    iy = j;
                    ix = myx / iy;
                    nrem = myx - ix * iy;
                    if(nrem == 0)
                        break;
                }

                if(ix < iy){
                    unsigned long it = ix;
                    ix = iy;
                    iy = it;
                }
                if(ix < iz){
                    unsigned long it = ix;
                    ix = iz;
                    iz = it;
                }
                if(iy < iz){
                    unsigned long it = iy;
                    iy = iz;
                    iz = it;
                }
                if(((double)ix)/iz < ratio){
                    ratio = ((double)ix)/iz;
                    *o_x = ix;
                    *o_y = iy;
                    *o_z = iz;
                }
            } //if( nrem == 0 )
        } //for(i=iroot3;
        break;
    }
    return err;
}


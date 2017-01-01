#ifndef SET_F_H
#include "set_f.h"
#endif

#include "blas.h" //NEK_USE_ADVANCED_FUNCTIONS

#ifdef NEK_USE_ADVANCED_FUNCTIONS
#include <math.h>
#endif
//DBG> #include <stdio.h>

int nbb_set_f(unsigned int in_length, NBN_REAL * io_f)
{
    int err = 0;
    unsigned int k;

#ifdef NEK_USE_ADVANCED_FUNCTIONS

    if( 4 == sizeof(NBN_REAL)){
        for(k=0; k < in_length; ++k){
            int i = (int) k;;
            float arg = 1.e9*(i*i);
            arg = 1.e9*cos(arg);
            io_f[k] = sin(arg);
        }
    } else {
        if( 8 != sizeof(NBN_REAL)){
            err = __LINE__; return err;
        }
        for(k=0; k < in_length; ++k){
            int i = (int) k+1;
            double arg = 1.e9*(i*i);
            //DBG> printf("i=  %u  %24.14E\n", i,arg);
            arg = 1.e9*cos(arg);
            //DBG> printf("arg= %24.14E\n", arg);
            io_f[k] = sin(arg);
            //DBG> printf("f= %24.14E\n", io_f[k]);
        }
    }
#else
    for(k=0; k < in_length; ++k){
        io_f[k] = 1;
    }
#endif

    return err;
}

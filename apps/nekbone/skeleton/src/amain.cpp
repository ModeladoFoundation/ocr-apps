#include "app_ocr_err_util.h"

#include "nekcgsim.h"

#include <stdio.h>

int main(int argc, char * argv[])
{
    Err_t err=0;
    while(!err){
        NekCGsim nekcg;
        err = nekcg.create(argc,argv); IFEB;

        err = nekcg.run(); IFEB;

        err = nekcg.destroy(); IFEB;
        break;
    }

    if(err){
        printf("ERROR:%d\n",err);
        return 1;
    }
    return 0;
}

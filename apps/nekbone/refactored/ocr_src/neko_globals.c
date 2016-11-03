#ifndef NEKO_GLOBALS_H
#include "neko_globals.h"
#endif

#include "ocr.h"
#include "spmd_global_data.h" //SPMD_GlobalData_t
#include "../nek_src/nbn_setup.h"

#define XMEMSET(SRC, CHARC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)SRC+xmIT)=CHARC;}
#define XMEMCPY(DEST, SRC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)DEST+xmIT)=*((char*)SRC+xmIT);}

Err_t init_NEKOstatics(NEKOstatics_t * io)
{
    Err_t err=0;
    while(!err){
        XMEMSET(io, 0, sizeof(NEKOstatics_t));

        //This specify the disribution of MPI ranks along the 3 main axis.
        //For example, (Rx,Ry,Rz) puts down
        //      Rx ranks along the X-axis
        //      Ry ranks along the Y-axis
        //      Rz ranks along the Z-axis
        // The total number of ranks in use will be Rtotal= Rx*Ry*Rz
        io->Rx=2;
        io->Ry=2;
        io->Rz=1;
        io->Rtotal = io->Rx * io->Ry * io->Rz;

        //This specify the distribution of elements within each and all ranks.
        //For example, (Ex,Ey,Ez) puts down
        //      Ex ranks along the X-axis
        //      Ey ranks along the Y-axis
        //      Ez ranks along the Z-axis
        // The total number of element in a single rank is Etotal= Ex*Ey*Ez
        io->Ex = 2;
        io->Ey = 2;
        io->Ez = 2;
        io->Etotal = io->Ex * io->Ey * io->Ez;

        //This gives the overall total element count in use:
        //GlobalElementCount = Rtotal * Etotal
        io->GlobalElementCount = io->Rtotal * io->Etotal;

        // For the workload specification, that ends up
        // setting (pDOF_begin, pDOF_end, pDOF_step) = (9, 13, 3) which corresponds
        // to NEKbone::(nx0,nxN,nxD)=(9,12,3)

        //2016Oct26: Here pDOF is set to 8 in the hope of getting, on TG, some
        //           support for a hardware assisted dgemm.
        io->pDOF_begin = 8;
        io->pDOF_end = io->pDOF_begin +1;
        io->pDOF_step = 1;

        //The maximum number of DOF along one dimension
        io->pDOF_max = 0;
        unsigned int i;
        for(i=io->pDOF_begin; i<io->pDOF_end; ++i) if(io->pDOF_max < i) io->pDOF_max = i;
        if(io->pDOF_max == 0){
            err = __LINE__;
            break;
        }

        io->pDOFmax2D = io->pDOF_max * io->pDOF_max;

        //The maximum number of DOF in one element, exactly pDOF_max^3
        io->pDOFmax3D = io->pDOF_max * io->pDOF_max * io->pDOF_max;

        //The maximum number of DOF in one rank, exactly pDOFmax3D*Etotal
        io->pDOF3DperRmax = io->pDOFmax3D * io->Etotal;

        //This specifies the maximum number of CG iteration to be done.
        //100 is the required number.
        io->CGcount = 4;

        io->ByteSizeOf1DOF = sizeof(NBN_REAL);

        //===== Basic checks
        if(io->Rtotal==0 || io->Rtotal != io->Rx * io->Ry * io->Rz ) { err=__LINE__; IFEB; }
        if(io->Etotal==0 || io->Etotal != io->Ex * io->Ey * io->Ez ) { err=__LINE__; IFEB; }
        if(io->pDOF_begin >= io->pDOF_end || io->pDOF_step==0) { err=__LINE__; IFEB; }

        if(NEKbone_neighborCount != 26) {err=__LINE__; IFEB;} //This should not change. This code only works with cubic lattices.
        if(io->pDOF_begin !=8 || io->pDOF_end !=8+1) {
            //2016OCt26: In the course of the initial development, it was felt
            //           that a pDOF of 8 would be best since there was the possibility
            //           to get hardware support on the TG/FSIM side for matrices
            //           size 8.
            err=__LINE__; IFEB;
        }

        break;
    }
    return err;
}

Err_t clear_NEKOstatics(NEKOstatics_t * io)
{
    Err_t err=0;
    while(!err){
        XMEMSET(io, 0, sizeof(NEKOstatics_t));
        break;
    }
    return err;
}

Err_t destroy_NEKOstatics(NEKOstatics_t * io)
{
    Err_t err=0;
    while(!err){
        err = clear_NEKOstatics(io);
        break;
    }
    return err;
}

Err_t copy_NEKOstatics(NEKOstatics_t * in_from, NEKOstatics_t * o_target)
{
    Err_t err=0;
    XMEMCPY(o_target, in_from, sizeof(NEKOstatics_t));
    return err;
}

void  print_NEKOstatics(NEKOstatics_t * in)
{
    PRINTF("NEKOStatics: Rx,Ry,Rz,Rtotal= %u,%u,%u,%u ", in->Rx, in->Ry, in->Rz, in->Rtotal);
    PRINTF("Ex,Ey,Ez,Etotal=%u,%u,%u,%u ", in->Ex, in->Ey, in->Ez, in->Etotal);
    PRINTF("pDOF_begin,pDOF_end,pDOF_step= %u,%u,%u\n", in->pDOF_begin, in->pDOF_end, in->pDOF_step);

    PRINTF("NEKOStatics: pDOF_max,pDOFmax2D,pDOFmax3D,pDOF3DperRmax = %u, %u, %u\n",
           in->pDOF_max, in->pDOFmax2D, in->pDOFmax3D, in->pDOF3DperRmax);

    PRINTF("NEKOStatics: GlobalElementCount= %u\n", in->GlobalElementCount);
    PRINTF("NEKOStatics: CGcount= %u\n", in->CGcount);
    PRINTF("NEKOStatics: NeighborCount= %u\n", NEKbone_neighborCount);
    PRINTF("NEKOStatics: ByteSizeOf1DOF= %u\n", in->ByteSizeOf1DOF);
}

Err_t setup_SPMD_using_NEKOstatics(NEKOstatics_t * in_NEKOstatics,
                                   SPMD_GlobalData_t * o_SPMDglobals)
{
   Err_t err=0;
    while(!err){
        o_SPMDglobals->overall_mpi_count = in_NEKOstatics->Rtotal;
        o_SPMDglobals->iterationCountOnEachRank = in_NEKOstatics->CGcount;
        break;
    }
    return err;
}

Err_t init_NEKOglobals(NEKOstatics_t * in_statics, unsigned int in_rankID, NEKOglobals_t * io)
{
    Err_t err=0;
    while(!err){
        XMEMSET(io, 0, sizeof(NEKOglobals_t));
        // The current number of degree of freedom for one polynomial Basis
        //      pDOF_begin <= pDOF < pDOF_end
        io->pDOF = in_statics->pDOF_begin;

        // The total number of disconnected DOF in one rank, exactly pDOF^3 * Etotal
        io->pDOF3DperR = io->pDOF * io->pDOF * io->pDOF * in_statics->Etotal;

        // The emulated rank ID of this EDT.
        io->rankID = in_rankID;

        if(io->pDOF != in_statics->pDOF_begin){
            PRINTF("ERROR: NEKOglobals.pDOF has to be equal to NEKOstatics.pDOF_begin for now: pDOF=%u  pDOF_begin=%u\n",
                   io->pDOF, in_statics->pDOF_begin);
            err = __LINE__;
            break;
        }

        if(io->pDOF <=1){
            PRINTF("ERROR: NEKOglobals.pDOF is too small.  It  must be >= 2: pDOF=%u\n",
                   io->pDOF);
            err = __LINE__;
            break;
        }

        if( io->rankID >= in_statics->Rtotal){
            PRINTF("ERROR: NEKOglobals.rankID is too large:  rankID=%u  Rtotal=%u\n",
                   io->pDOF, in_statics->Rtotal);
            err = __LINE__;
            break;
        }

        break;
    }
    return err;
}

Err_t clear_NEKOglobals(NEKOglobals_t * io)
{
    Err_t err=0;
    while(!err){
        XMEMSET(io, 0, sizeof(NEKOglobals_t));
        break;
    }
    return err;
}

Err_t destroy_NEKOglobals(NEKOglobals_t * io)
{
    Err_t err=0;
    while(!err){
        XMEMSET(io, 0, sizeof(NEKOglobals_t));
        break;
    }
    return err;
}

Err_t copy_NEKOglobals(NEKOglobals_t * in_from, NEKOglobals_t * o_target)
{
    Err_t err=0;
    while(!err){
        XMEMCPY(o_target, in_from, sizeof(NEKOglobals_t));
        break;
    }
    return err;
}

void  print_NEKOglobals(NEKOglobals_t * in)
{
    PRINTF("NEKOglobals: pDOF, pDOF3DperR, rankID, ExRx,EyRy,EzRz = %u, %u, %u, %u,%u,%u\n",
           in->pDOF, in->pDOF3DperR, in->rankID, in->ExRx, in->EyRy, in->EzRz);
}

































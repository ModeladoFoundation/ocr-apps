#ifndef NEKO_GLOBALS_H
#define NEKO_GLOBALS_H

#include "extensions/ocr-affinity.h"

#include "app_ocr_err_util.h"
#include "timing.h"
#include "spmd_global_data.h"

//Some values are set in Nekbone which cannot be changed without having
//to re-architect most of the Nekbone baseline.
//They are added here as macro constants in order to minimize the run-time
//burden of handling them.

#define NEKbone_ifbrick 1 //This indicates that a parallelepiped of Rx by Ry by Rz
                          //ranks is to be used.
                          //2016Oct26: I do not recommend using then other possible
                          // value (-0) as a brick with (Rx,Ry,Rz)=(Rx,1,1) does the
                          // same thing, and has been better tested.

//In a cubic lattice, each cube can have at most 26 neighbors.
#define NEKbone_neighborCount 26
#define NEKbone_regionCount 27   //Exactly NEKbone_neighborCount+1

#define NEKbone_ldim 3  //The number of dimension used, i.e. 3D, also = ndim

#define NEKbone_numthreads 1 //driver.f::numthreads  //The number of threads used.

//The number of OpenMP threads in use;
// obviously this has to be one for an OCR program.
//But the Nekbone variable cg.f::thread is, in this case, set to zero.
#define NEKbone_thread 0

#define NEKbone_numth 1  //cg.f::numth
#define NEKbone_tmt (NEKbone_thread +1)  //cg.f::tmt
#define NEKbone_fel 1   //cg.f::fel
#define NEKbone_find 1  //cg.f::find

//----- HALO Exchanges enablers
//Define one, or all, of the followings in order to use halo exchanges; otherwise comment out.
#define NKEBONE_USE_CHANNEL_FOR_HALO_MULTIPLICITY
#define NKEBONE_USE_CHANNEL_FOR_HALO_SETF
#define NKEBONE_USE_CHANNEL_FOR_HALO_AI

#ifdef NKEBONE_USE_CHANNEL_FOR_HALO_MULTIPLICITY
#   define NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
#else
#   ifdef NKEBONE_USE_CHANNEL_FOR_HALO_SETF
#       define NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
#   else
#       ifdef NKEBONE_USE_CHANNEL_FOR_HALO_AI
#           define NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
#       endif
#   endif
#endif
//^^^^^HALO Exchanges enablers Done

//There are two types of globals: NEKOstatics and NEKOglobals.
//The NEKOstatics were compile-time constants and could not be changed at run-time.
//The NEKOglobals are established once one arrives at the start OCR "rank" equivalent
// and remains constant for the rest of the execution.

typedef struct NEKOstatics {
    // In the original Nekbone code, these variables have the following correspondence:
    //      Rtotal      --> lp : The maximum number of ranks
    //                  --> np, np_ : actual number of ranks in use
    //      Rx,Ry,Rz    --> npx, npy, npz
    //
    //      Etotal      --> lelt : The maximum number of elements per ranks
    //                  --> nelt : The actual number of elements per ranks,
    //                             as NEkbone does not vary this often.
    //                             Thus iel0, ielN, ielD are skipped.
    //                  Also cg.f::lel
    //      Ex,Ey,Ez    --> mx, my, mz
    //
    //      pDOF_begin, pDOF_end, pDOF_step -->  roughly nx0, nxN, nxD.  See below for details.
    //      pDOF_max      --> lx1, ly1, lz1  as lx1 == ly1 == lz1 is always true
    //      pDOFmax3D     --> lxyz = lx1*ly1*lz1
    //      pDOF3DperRmax --> lt = lxyz*lelt
    //
    //      CGcount     --> niter = miter

    //This specify the disribution of MPI ranks along the 3 main axis.
    //For example, (Rx,Ry,Rz) puts down
    //      Rx ranks along the X-axis
    //      Ry ranks along the Y-axis
    //      Rz ranks along the Z-axis
    // The total number of ranks in use will be Rtotal= Rx*Ry*Rz
    unsigned int Rx,Ry,Rz;
    unsigned int Rtotal;

    //This specify the distribution of elements within each and all ranks.
    //For example, (Ex,Ey,Ez) puts down
    //      Ex ranks along the X-axis
    //      Ey ranks along the Y-axis
    //      Ez ranks along the Z-axis
    // The total number of element in a single rank is Etotal= Ex*Ey*Ez
    unsigned int Ex,Ey,Ez;
    unsigned int Etotal;

    //This gives the overall total element count in use:
    //GlobalElementCount = Rtotal * Etotal
    unsigned GlobalElementCount;

    //This specify the polynomial DOF sequence to use.
    //For example, for (pDOF_begin, pDOF_end, pDOF_step)=(9,13,3), then
    //  the sequence generated will be the same as
    //      for(int p=pDOF_begin; p!=pDOF_end; p+=pDOF_step){...}
    //  where p is the polynomial DOF in use
    //  So p will 9,then 12.
    //IMPORTANT Note:
    // DOF: degree of freedom, the number of unknowns along one dimension
    // For a given polynomial order P-1, NEKbone assumes that a given
    // element will be partitioned by putting P degree-of-freedom (DOF)
    // along each main axis of the element.
    // So each and all elements end up with a total of (P)^3 DOFs each,
    // distributed in the element.
    // For the workload specification, that ends up
    // setting (pDOF_begin, pDOF_end, pDOF_step) = (9, 13, 3) which corresponds
    // to NEKbone::(nx0,nxN,nxD)=(9,12,3)
    unsigned int pDOF_begin, pDOF_end, pDOF_step;

    //The maximum number of DOF along one dimension
    unsigned int pDOF_max;

    //The maximum number of DOF in one element, exactly pDOF_max^2
    unsigned int pDOFmax2D;

    //The maximum number of DOF in one element, exactly pDOF_max^3
    unsigned int pDOFmax3D;

    //The maximum number of DOF in one rank, exactly pDOFmax3D*Etotal
    unsigned int pDOF3DperRmax;

    //This specifies the maximum number of CG iteration to be done.
    //100 is the required number.
    unsigned int CGcount;

    unsigned int ByteSizeOf1DOF;

    //When the entire program started
    TimeMark_t startTimeMark;

    long OCR_affinityCount;

    //= The following is used to do the initial exchange of the channels events
    //  in order to facilitate halo exchanges between neighboring EDTs.
    //= For the numbering scheme used, see nekos_tool.h::NEKOtools_t::nloads[].did.
    //  It skips the number 13 because that is our self location.
    ocrGuid_t haloLabeledGuids[NEKbone_regionCount];

} NEKOstatics_t;

Err_t init_NEKOstatics(NEKOstatics_t * io, void * in_programArgv);
Err_t clear_NEKOstatics(NEKOstatics_t * io);
Err_t destroy_NEKOstatics(NEKOstatics_t * io);
Err_t copy_NEKOstatics(NEKOstatics_t * in_from, NEKOstatics_t * o_target);
void  print_NEKOstatics(NEKOstatics_t * in);

Err_t setup_SPMD_using_NEKOstatics(NEKOstatics_t * in_NEKOstatics,
                                   SPMD_GlobalData_t * o_SPMDglobals);

Err_t nekbone_finalEDTt(void);

//Because we cannot guarantee temporal separation of channels,
//one channel per operation is required.
typedef struct ChannelStruct
{
    ocrGuid_t c4multi; //This is to be used for nekMultiplicity.
    ocrGuid_t c4setf;  //This is to be used for nekSetF.
    ocrGuid_t c4axi;   //This is to be used for nekCG_axi. //It is hoped that temporal separation
                                                           //between CG iterations will be enough,
                                                           //even if reduction is not used.
} ChannelStruct_t;

Err_t copy_ChannelStruct(ChannelStruct_t * in_from, ChannelStruct_t * o_target);

// In the original Nekbone code, these variables have the following correspondence:
//      pDOF        --> nx1, ny1, nz1    Note that nx1 == ny1 == nz1 is always true.
//      pDOF3DperR  --> nx1*ny1*nz1*nelt
//      rankID      --> nid
//
typedef struct NEKOglobals {
    // The current number of degree of freedom for one polynomial Basis
    //      pDOF_begin <= pDOF < pDOF_end
    unsigned int pDOF;

    // The total number of disconnected DOF in one rank, exactly pDOF^3 * Etotal
    unsigned int pDOF3DperR; //Also cg.f::lind

    // The emulated rank ID of this EDT.
    unsigned int rankID;
    unsigned int OCR_affinityCount;

    // The number of elements along a global Rank direction
    unsigned int ExRx;
    unsigned int EyRy;
    unsigned int EzRz;

    //When an individual rank started
    TimeMark_t startTimeMark;

    //Channels
    ChannelStruct_t myChannels[NEKbone_regionCount]; //These are my channel events to be shared with the other ranks,
                                                     // and which the current rank will use ocrEventSatisfy with a DBK
                                                     // in order to pass to the neighbors.
    ChannelStruct_t neighborChannels[NEKbone_regionCount]; //These are the channel events, received from the neighbors,
                                                           //and which I'll pass to my next EDT in order to get
                                                           //their DBKs.
} NEKOglobals_t;

Err_t init_NEKOglobals(NEKOstatics_t * in_statics, unsigned int in_rankID,
                       NEKOglobals_t * io);
Err_t destroy_NEKOglobals(NEKOglobals_t * io);
Err_t copy_NEKOglobals(NEKOglobals_t * in_from, NEKOglobals_t * o_target);
void  print_NEKOglobals(NEKOglobals_t * in);

#define NEK_OCR_ENABLE_AFFINITIES
#ifndef ENABLE_EXTENSION_AFFINITY
#undef NEK_OCR_ENABLE_AFFINITIES
#endif

#define NEK_OCR_USE_CURRENT_PD ((unsigned long)-2)

//io_HNT can either be NULL or a pointer to a within scope concrete ocrHint_t;
//o_pHNT is fed in as pointer to ocrHint_t.
//o_pHNT will be returned either as NULL_HINT or a pointer to *io_HNT.
//So (*o_HNT) is the hint value one will use in the code.
//NOTE: The round abound way with o_HNT is because NULL_HINT is 0x0.
unsigned long calcPDid_G(NEKOglobals_t * in);
unsigned long calcPDid_S(unsigned int in_OCR_affinityCount, unsigned int in_rankID);
Err_t ocrXgetEdtHint(unsigned long in_pdID, ocrHint_t * io_HNT, ocrHint_t ** o_pHNT);
Err_t ocrXgetDbkHint(unsigned long in_pdID, ocrHint_t * io_HNT, ocrHint_t ** o_pHNT);

#endif // NEKO_GLOBALS_H

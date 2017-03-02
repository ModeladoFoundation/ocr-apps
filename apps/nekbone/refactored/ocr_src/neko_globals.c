#ifndef NEKO_GLOBALS_H
#include "neko_globals.h"
#endif

#include "ocr.h"

#include "spmd_global_data.h" //SPMD_GlobalData_t
#include "../nek_src/nbn_setup.h"

#include "blas.h"

#ifdef NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
#   define ENABLE_EXTENSION_LABELING // For labeled GUIDs
#   include "extensions/ocr-labeling.h" // For labeled GUIDs
#endif

#define XMEMSET(SRC, CHARC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)SRC+xmIT)=CHARC;}
#define XMEMCPY(DEST, SRC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)DEST+xmIT)=*((char*)SRC+xmIT);}

unsigned int myAtoU(const char *in)
{
    const long max_length = 2048;
    unsigned int res = 0;

    const char * p = in;
    while( *p != '\0' && (*p==' ' || *p=='\t') ) {
            if(p-in > max_length) return res; //Error: text too long
            ++p;
    }
    if(*p=='\0'){
        return res;
    }

    int i;
    for(i = 0; p[i] != '\0'; ++i){
        if( p[i] == ' ' || p[i] == '\t') {
            break;
        }
        res = res*10U + p[i] - '0';
    }

    return res;
}

Err_t init_NEKOstatics(NEKOstatics_t * io, void * in_programArgv)
{
    Err_t err=0;
    while(!err){

#       ifdef NEK_USE_ADVANCED_FUNCTIONS
            PRINTF("INFO: NEK_USE_ADVANCED_FUNCTIONS is active.\n");
#       else
            PRINTF("INFO: NEK_USE_ADVANCED_FUNCTIONS is off.\n");
#       endif

#       ifdef NEKO_USE_TIMING
            PRINTF("INFO: NEKO_USE_TIMING   is active.\n");
#       else
            PRINTF("INFO: NEKO_USE_TIMING   is off.\n");
#       endif
#       ifdef NEKO_PRINT_TIMING
            PRINTF("INFO: NEKO_PRINT_TIMING is active.\n");
#       else
            PRINTF("INFO: NEKO_PRINT_TIMING is off.\n");
#       endif
#       ifdef NEKO_CG_TIMING
            PRINTF("INFO: NEKO_CG_TIMING    is active.\n");
#       else
            PRINTF("INFO: NEKO_CG_TIMING    is off.\n");
#       endif

        XMEMSET(io, 0, sizeof(NEKOstatics_t));

        unsigned int argRx=0,argRy=0,argRz=0;
        unsigned int argEx=0,argEy=0,argEz=0;
        unsigned int argPDOF_begin=0;
        unsigned int argCGcount=0;

        if(!in_programArgv){
                err = __LINE__; IFEB;
        }else{
            unsigned int argc = getArgc(in_programArgv);
            if(8+1 != argc){ //+1 for argc at offset zero.
                PRINTF("ERROR: 8 cmd line arguments are needed: Rx Ry Rz Ex Ey Ez pDOF CGcount.  Received: %u\n", argc);
                err = __LINE__; IFEB;
            }

            unsigned int k = 1;
//            argRx = (unsigned int) atoi(getArgv(in_programArgv, k++));
//            argRy = (unsigned int) atoi(getArgv(in_programArgv, k++));
//            argRz = (unsigned int) atoi(getArgv(in_programArgv, k++));
//            argEx = (unsigned int) atoi(getArgv(in_programArgv, k++));
//            argEy = (unsigned int) atoi(getArgv(in_programArgv, k++));
//            argEz = (unsigned int) atoi(getArgv(in_programArgv, k++));
//            argPDOF_begin = (unsigned int) atoi(getArgv(in_programArgv, k++));
//            argCGcount = (unsigned int) atoi(getArgv(in_programArgv, k++));
            argRx = (unsigned int) myAtoU(getArgv(in_programArgv, k++));
            argRy = (unsigned int) myAtoU(getArgv(in_programArgv, k++));
            argRz = (unsigned int) myAtoU(getArgv(in_programArgv, k++));
            argEx = (unsigned int) myAtoU(getArgv(in_programArgv, k++));
            argEy = (unsigned int) myAtoU(getArgv(in_programArgv, k++));
            argEz = (unsigned int) myAtoU(getArgv(in_programArgv, k++));
            argPDOF_begin = (unsigned int) myAtoU(getArgv(in_programArgv, k++));
            argCGcount = (unsigned int) myAtoU(getArgv(in_programArgv, k++));

            //PRINTF("DBG> init_NEKOstatics> ARGV= %u %u %u  %u %u %u  %u %u \n",
            //       argRx, argRy, argRz,  argEx, argEy, argEz, argPDOF_begin, argCGcount);
        }

        io->startTimeMark = nekbone_getTime();
        TIMEPRINT1("NKTIME> init_NEKOstatics= "TIMEF"\n", io->startTimeMark);

        //This specify the disribution of MPI ranks along the 3 main axis.
        //For example, (Rx,Ry,Rz) puts down
        //      Rx ranks along the X-axis
        //      Ry ranks along the Y-axis
        //      Rz ranks along the Z-axis
        // The total number of ranks in use will be Rtotal= Rx*Ry*Rz
        io->Rx=argRx;
        io->Ry=argRy;
        io->Rz=argRz;
        io->Rtotal = io->Rx * io->Ry * io->Rz;

        //This specify the distribution of elements within each and all ranks.
        //For example, (Ex,Ey,Ez) puts down
        //      Ex ranks along the X-axis
        //      Ey ranks along the Y-axis
        //      Ez ranks along the Z-axis
        // The total number of element in a single rank is Etotal= Ex*Ey*Ez
        io->Ex = argEx;
        io->Ey = argEy;
        io->Ez = argEz;
        io->Etotal = io->Ex * io->Ey * io->Ez;

        //This gives the overall total element count in use:
        //GlobalElementCount = Rtotal * Etotal
        io->GlobalElementCount = io->Rtotal * io->Etotal;

        // For the workload specification, that ends up
        // setting (pDOF_begin, pDOF_end, pDOF_step) = (9, 13, 3) which corresponds
        // to NEKbone::(nx0,nxN,nxD)=(9,12,3)

        //2016Oct26: Here pDOF is set to 8 in the hope of getting, on TG, some
        //           support for a hardware assisted dgemm.
        io->pDOF_begin = argPDOF_begin;
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
        io->CGcount = argCGcount;

        io->ByteSizeOf1DOF = sizeof(NBN_REAL);

        //===== Basic checks
        if(io->Rtotal==0 || io->Rtotal != io->Rx * io->Ry * io->Rz ) { err=__LINE__; IFEB; }
        if(io->Etotal==0 || io->Etotal != io->Ex * io->Ey * io->Ez ) { err=__LINE__; IFEB; }
        if(io->pDOF_begin >= io->pDOF_end || io->pDOF_step==0) { err=__LINE__; IFEB; }

        if(NEKbone_neighborCount != 26) {err=__LINE__; IFEB;} //This should not change. This code only works with cubic lattices.

        //The hashing used to go from an ID to a triplet, e.g. rid <-> (rx,ry,rz),
        // is not rotational invariant.  So the hashing ra=(2,1,1) will not yield
        // the same numbering for rb=(1,2,1), nor rc=(1,1,2).
        //In order to alleviate this state of affair, the ordering of triplets
        // is forced to be ordered.
        //TODO: Remove the forced ordering of R,E,P triplets.
        if(io->Rx < io->Ry || io->Ry < io->Rz || io->Rx < io->Rz) { err=__LINE__; IFEB; }
        if(io->Ex < io->Ey || io->Ey < io->Ez || io->Ex < io->Ez) { err=__LINE__; IFEB; }

        //===== Setup OCR affinities
        io->OCR_affinityCount = 0;

#       ifdef NEK_OCR_ENABLE_AFFINITIES
            u64 affinityCount;
            err = ocrAffinityCount( AFFINITY_PD, &affinityCount ); IFEB;
            io->OCR_affinityCount = affinityCount;
            if(io->OCR_affinityCount < 0) {err = __LINE__; IFEB;}
            PRINTF("INFO: Affinities are in use.\n");
#       else
            PRINTF("INFO: Affinities are not used.\n");
#       endif // NEK_OCR_ENABLE_AFFINITIES

        //===== Setup for channel halo exchange
        {
            unsigned int i;
            for(i=0; i < NEKbone_regionCount; ++i){
                GUID_ASSIGN_VALUE(io->haloLabeledGuids[i], NULL_GUID);
            }

#           ifdef NKEBONE_USE_CHANNEL_FOR_HALO_MULTIPLICITY
                PRINTF("INFO: Channel Halo exchanges in Multiplicity are active.\n");
#           else
                PRINTF("INFO: Channel Halo exchanges in Multiplicity are off.\n");
#           endif
#           ifdef NKEBONE_USE_CHANNEL_FOR_HALO_SETF
                PRINTF("INFO: Channel Halo exchanges in SetF         are active.\n");
#           else
                PRINTF("INFO: Channel Halo exchanges in SetF         are off.\n");
#           endif
#           ifdef NKEBONE_USE_CHANNEL_FOR_HALO_AI
                PRINTF("INFO: Channel Halo exchanges in AI           are active.\n");
#           else
                PRINTF("INFO: Channel Halo exchanges in AI           are off.\n");
#           endif

#           ifdef NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
                for(i=0; i < NEKbone_regionCount; ++i){
                    err = ocrGuidRangeCreate( &io->haloLabeledGuids[i], io->Rtotal, GUID_USER_EVENT_STICKY); IFEB;
                    if( ocrGuidIsNull(io->haloLabeledGuids[i]) ){
                        PRINTF("ERROR: Ledger creation in init_NEKOstatics failed: labeledGuid %u is NULL.\n", i);
                        err = __LINE__;
                        IFEB;
                    }
                }IFEB;

                PRINTF("INFO> Use of Channels for halo exchanges is active.\n");
#           else
                PRINTF("INFO> Use of Channels for halo exchanges is off.\n");
#           endif
        }

        //===== Echo results obtained
        print_NEKOstatics(io);

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
        unsigned int i=0;
#       ifdef NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
            for(i=0; i<NEKbone_regionCount; ++i){
                if( ! ocrGuidIsNull(io->haloLabeledGuids[i]) ){
                    err = ocrGuidMapDestroy(io->haloLabeledGuids[i]); IFEB;
                    GUID_ASSIGN_VALUE(io->haloLabeledGuids[i], NULL_GUID);
                }
            } IFEB;
#       endif
        err = clear_NEKOstatics(io);IFEB;
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
    PRINTF("NEKOStatics: TimeMark= "TIMEF"\n", in->startTimeMark);

    PRINTF("NEKOStatics: Affinity counts= %ld\n", in->OCR_affinityCount);

    unsigned int i;
    for(i=0; i<NEKbone_regionCount; ++i){
        PRINTF("NEKOStatics: labeledGuids_for_halo[%u]="GUIDF"\n", i, GUIDA(in->haloLabeledGuids[i]) );
    }
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

Err_t nekbone_finalEDTt(void)
{
    TimeMark_t t = nekbone_getTime();
    TIMEPRINT1("NKTIME> FinalEDT="TIMEF"\n", t);
    return 0;
}

Err_t copy_ChannelStruct(ChannelStruct_t * in_from, ChannelStruct_t * o_target)
{
    Err_t err=0;
    while(!err){
        if(!in_from || !o_target) {err =__LINE__; IFEB;}
        GUID_ASSIGN_VALUE(o_target->c4multi , in_from->c4multi);
        GUID_ASSIGN_VALUE(o_target->c4setf , in_from->c4setf);
        GUID_ASSIGN_VALUE(o_target->c4axi , in_from->c4axi);
        if(ocrGuidIsNull(o_target->c4multi)||ocrGuidIsUninitialized(o_target->c4multi)||ocrGuidIsError(o_target->c4multi)) {err =__LINE__; IFEB;}
        if(ocrGuidIsNull(o_target->c4setf)||ocrGuidIsUninitialized(o_target->c4setf)||ocrGuidIsError(o_target->c4setf)) {err =__LINE__; IFEB;}
        if(ocrGuidIsNull(o_target->c4axi)||ocrGuidIsUninitialized(o_target->c4axi)||ocrGuidIsError(o_target->c4axi)) {err =__LINE__; IFEB;}
        break;
    }
    return err;
}

Err_t init_NEKOglobals(NEKOstatics_t * in_statics, unsigned int in_rankID, NEKOglobals_t * io)
{
    Err_t err=0;
    while(!err){
        XMEMSET(io, 0, sizeof(NEKOglobals_t));

        io->startTimeMark = nekbone_getTime();
        TIMEPRINT2("NKTIME> rank=%u SPMD_fork="TIMEF"\n",
                   in_rankID, io->startTimeMark - in_statics->startTimeMark);

        // The current number of degree of freedom for one polynomial Basis
        //      pDOF_begin <= pDOF < pDOF_end
        io->pDOF = in_statics->pDOF_begin;

        // The total number of disconnected DOF in one rank, exactly pDOF^3 * Etotal
        io->pDOF3DperR = io->pDOF * io->pDOF * io->pDOF * in_statics->Etotal;

        // The emulated rank ID of this EDT.
        io->rankID = in_rankID;
        io->OCR_affinityCount = in_statics->OCR_affinityCount;

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
                   io->rankID, in_statics->Rtotal);
            err = __LINE__;
            break;
        }

        if( in_statics->Rtotal < io->OCR_affinityCount){
            PRINTF("WARNING: NEKOglobals.OCR_affinityCount is bigger the total count of Ranks:  Rtotal=%u  OCR_affinityCount=%u\n",
                   in_statics->Rtotal, io->OCR_affinityCount);
            PRINTF("WARNING: The distribution of rankID will not fill all policy domains.\n");
        }

        io->ExRx = in_statics->Ex * in_statics->Rx;
        io->EyRy = in_statics->Ey * in_statics->Ry;
        io->EzRz = in_statics->Ez * in_statics->Rz;

        //===== Setup fro channel halo exchange
        unsigned int i;
        for(i=0; i < NEKbone_regionCount; ++i){
            GUID_ASSIGN_VALUE(io->myChannels[i].c4multi, NULL_GUID);
            GUID_ASSIGN_VALUE(io->myChannels[i].c4setf, NULL_GUID);
            GUID_ASSIGN_VALUE(io->myChannels[i].c4axi, NULL_GUID);
            GUID_ASSIGN_VALUE(io->neighborChannels[i].c4multi, NULL_GUID);
            GUID_ASSIGN_VALUE(io->neighborChannels[i].c4setf, NULL_GUID);
            GUID_ASSIGN_VALUE(io->neighborChannels[i].c4axi, NULL_GUID);
        }

        //===== Output what we got
        print_NEKOglobals(io);

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
    PRINTF("NEKOglobals: pDOF, pDOF3DperR, rankID,affCount ExRx,EyRy,EzRz = %u,%u, %u,%u, %u,%u,%u\n",
           in->pDOF, in->pDOF3DperR, in->rankID, in->OCR_affinityCount,
           in->ExRx, in->EyRy, in->EzRz);
}

unsigned long calcPDid_G(NEKOglobals_t * in)
{
    return NEK_OCR_USE_CURRENT_PD; //This will cause the PDid, the policy domain ID,
                                       //to be the same as the parent EDT.
                                       //See ocrXget(Edt|Dbk)Hint for details.
}
unsigned long calcPDid_S(unsigned int in_OCR_affinityCount, unsigned int in_rankID)
{
    const unsigned long x = in_rankID % in_OCR_affinityCount;
    return x;
}

Err_t ocrXgetEdtHint(unsigned long in_pdID, ocrHint_t * io_HNT, ocrHint_t ** o_pHNT)
{
    Err_t err = 0;
    while(!err){
        *o_pHNT = NULL_HINT;
#       ifdef NEK_OCR_ENABLE_AFFINITIES
            err = ocrHintInit( io_HNT, OCR_HINT_EDT_T ); IFEB;

            ocrGuid_t currentAffinity = NULL_GUID;
            if(in_pdID == NEK_OCR_USE_CURRENT_PD){
                ocrAffinityGetCurrent( &currentAffinity );
            }else{
                ocrAffinityGetAt( AFFINITY_PD, in_pdID, &currentAffinity );
            }

            ocrSetHintValue( io_HNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue( currentAffinity ) );
            *o_pHNT = io_HNT;
#       endif
        break;
    }
    return err;
}

Err_t ocrXgetDbkHint(unsigned long in_pdID, ocrHint_t * io_HNT, ocrHint_t ** o_pHNT)
{
    Err_t err = 0;
    while(!err){
        *o_pHNT = NULL_HINT;
#       ifdef NEK_OCR_ENABLE_AFFINITIES
            err = ocrHintInit( io_HNT, OCR_HINT_DB_T ); IFEB;

            ocrGuid_t currentAffinity = NULL_GUID;
            if(in_pdID == NEK_OCR_USE_CURRENT_PD){
                ocrAffinityGetCurrent( &currentAffinity );
            }else{
                ocrAffinityGetAt( AFFINITY_PD, in_pdID, &currentAffinity );
            }

            ocrSetHintValue( io_HNT, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue( currentAffinity ) );
            *o_pHNT = io_HNT;
#       endif
        break;
    }
    return err;
}


































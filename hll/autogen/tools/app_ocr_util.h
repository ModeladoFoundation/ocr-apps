#ifndef APP_OCR_UTIL_H
#define APP_OCR_UTIL_H

#include <ocr.h>
#include "../../tools/app_ocr_err_util.h"

//-----------------------------------------------------------------------------
#define EDT_PRINTEXIT(label, x,line,filename) if(!(x)){PRINTF("%s: %s : line=%d, file=%s\n",label, #x,line,filename); ocrShutdown();}
#define EDT_PRINTERROREXIT(label, x,line,filename) if(x){PRINTF("%s: %u : line=%d, file=%s\n",label, (unsigned int)x,line,filename); ocrShutdown();}
#define EDT_ASSERT(x) EDT_PRINTEXIT(     "ASSERT", x,__LINE__,__FILE__)
#define EDT_ERROR(x)  EDT_PRINTERROREXIT("ERROR", x,__LINE__,__FILE__)
#define EDT_ARGS u32 paramc, u64* paramv, u32 depc , ocrEdtDep_t depv[]
//-----------------------------------------------------------------------------
#define STRINGIZE(x) (#x)

//-----------------------------------------------------------------------------
//The input parameter x must be a sized array.
#define ARGsizedArray(x) x.c, x.v

typedef struct OcrParams
{
    u32 c;   //This is paramc.  //>=0
    u64 * v; //This is paramv.  //Valid values: NULL or an address to an array of u64
} OcrParams_t;

typedef struct OcrDeps_create  //The create is appended here because, only during creation,
                               // depv is of type ocrGuid_t.
                               //On recpetion, depv is of type ocrEdtDep_t.
{
    u32         c; //This is depc.
    ocrGuid_t * v; //This is depv.
} OcrDeps_t;

typedef struct OcrXEdtCreate_args
{
    ocrEdt_t funcPtr;           //IN
    OcrParams_t params;         //IN
    OcrDeps_t deps;             //IN
    u16 flags;                  //IN
    ocrHint_t * hint;           //IN
    ocrGuid_t * task;           //OUT
    ocrGuid_t * outputEvent;    //IO
} OcrXEdtCreate_args_t;

//The input parameter x must of type OcrXEdtCreate_args.
#define ARGSedtCreate(x) x.funcPtr, x.param.c, x.params.v, x.deps.c, x.deps.v, x.flags, x.hint, x.task, x.outputEvent

Err_t ocrEdtXCreate(ocrEdt_t in_funcPtr,
                    u32 in_paramc, u64 * in_paramv, u32 in_depc, ocrGuid_t * in_depv,
                    u16 in_flags, ocrHint_t * in_hint,
                    ocrGuid_t * o_guid, ocrGuid_t * io_outputEvent
                   );

Err_t ocrXHookup(ocrEventTypes_t in_eventType,
                 u16 in_eventFlags,
                 ocrGuid_t in_EDTguid,
                 u32 in_slotNumber,
                 ocrDbAccessMode_t in_accessMode,
                 ocrGuid_t in_DBKguid
                 );

#endif // APP_OCR_UTIL_H

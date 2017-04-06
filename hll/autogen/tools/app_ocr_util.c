#ifndef APP_OCR_UTIL_H
#include "app_ocr_util.h"
#endif

Err_t ocrEdtXCreate(ocrEdt_t in_funcPtr,
                    u32 in_paramc, u64 * in_paramv, u32 in_depc, ocrGuid_t * in_depv,
                    u16 in_flags, ocrHint_t * in_hint,
                    ocrGuid_t * o_guid, ocrGuid_t * io_outputEvent
                   )
{
    *o_guid = NULL_GUID;
    Err_t err=0;
    while(!err){
        ocrGuid_t ta_guid = NULL_GUID;
        err = ocrEdtTemplateCreate( &ta_guid, in_funcPtr, in_paramc, in_depc); IFEB;

        err = ocrEdtCreate( o_guid, ta_guid, EDT_PARAM_DEF, in_paramv, EDT_PARAM_DEF, in_depv,
                           in_flags, in_hint, io_outputEvent); IFEB;
        err = ocrEdtTemplateDestroy( ta_guid ); IFEB;
        break;
    }
    return err;
}

#undef OMIT_FOR_RUN_OB_TG
#ifdef TG_ARCH
#define OMIT_FOR_RUN_OB_TG
#endif // TG_ARCH
#ifndef OMIT_FOR_RUN_OB_TG
Err_t ocrXIndexedEdtCreate(ocrEdt_t in_funcPtr, u32 in_paramc, u64 * in_paramv, u32 in_depc,
                           u16 in_flags, ocrHint_t * in_hint, unsigned long in_index, ocrGuid_t * in_EDTmap,
                           ocrGuid_t * o_guid, ocrGuid_t * io_outputEvent
                          )
{
    GUID_ASSIGN_VALUE(*o_guid, NULL_GUID);
    Err_t err=0;
    while(!err){
        PRINTF("DEVDBG36> in_funcPtr=%p  in_flags=%u  in_hint=%p in_index=%lu in_EDTmap="GUIDF"\n",
               in_funcPtr, (unsigned int)in_flags, in_hint, in_index,
               GUIDA(*in_EDTmap)
               );

        ocrGuid_t ta_guid;
        GUID_ASSIGN_VALUE(ta_guid, NULL_GUID);
        err = ocrEdtTemplateCreate( &ta_guid, in_funcPtr, in_paramc, in_depc); IFEB;

//        PRINTF("DEVDBG45 before> o_guid="GUIDF"\n", GUIDA(*o_guid));
        err = ocrGuidFromIndex( o_guid, *in_EDTmap, in_index);
        if(err){
            PRINTF("ERROR: ocrGuidFromIndex for a labeled EDT, indexed %lu, failed: %u\n", in_index, err);
            IFEB;
        }
//        PRINTF("DEVDBG45 after> o_guid="GUIDF"\n", GUIDA(*o_guid));

        //With labeled EDTs, one cannot specify the depv at creation time.
        ocrGuid_t * depv = NULL;

        //GUID_PROP_IS_LABELED is required for labeled guids.
        //GUID_PROP_CHECK is just a safety measure.
        //  At some run-time cost, GUID_PROP_CHECK can be used.
        //  it will perform some checks. For example, it will check if the guid
        //  for a particular index has been destroyed before being re-assigned.
        //  GUID_PROP_CHECK is optional, if one, by design, can guarantee no infractions.
        //  On the other hand, currently it is no really doable to make that guarantee
        //  between two asynchronous processes.  --> Best practice for now: Use it.
        in_flags |= GUID_PROP_IS_LABELED | GUID_PROP_CHECK;

//        PRINTF("DEVDBG72 before EdtCreate> o_guid="GUIDF"\n", GUIDA(*o_guid));

        //2016Aug31
        //in_hint --> As usual, but, currently, the meta data for o_guid will be
        //            stored on the machine where where edtMap was created.
        //        --> So any interaction with this ga_Achild will require talking
        //            to the machine where edtMap was created!
        //        --> That is one of the performance bottleneck.
        //            Execution of o_guid will be where it was created as usual.
        err = ocrEdtCreate( o_guid, ta_guid, EDT_PARAM_DEF, in_paramv, EDT_PARAM_DEF, depv,
                           in_flags, in_hint, io_outputEvent);

//        PRINTF("DEVDBG72 after EdtCreate> o_guid="GUIDF"\n", GUIDA(*o_guid));

        if(err){
            if( err == OCR_EGUIDEXISTS ){
                //Someone has already created an object associated with this o_guid.
                //This is usually the expected behavior. --> Just ignore it.
                err = 0;
            }else{
                PRINTF("ERROR: ocrEdtCreate for a labeled EDT, indexed=%lu, failed: %u\n", in_index, err);
                IFEB;
            }
        }

        err = ocrEdtTemplateDestroy( ta_guid ); IFEB;
        break;
    }
    return err;
}
#endif //OMIT_FOR_RUN_OB_TG

Err_t ocrXHookup(ocrEventTypes_t in_eventType,
                 u16 in_eventFlags,
                 ocrGuid_t in_EDTguid,
                 u32 in_slotNumber,
                 ocrDbAccessMode_t in_accessMode,
                 ocrGuid_t in_DBKguid
                 )
{
    Err_t err=0;
    while(!err){
        err = ocrAddDependence( in_DBKguid, in_EDTguid, in_slotNumber, in_accessMode); IFEB;
        break;
    }
    return err;
}








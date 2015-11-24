#ifndef REDUCTIONS_H
#define REDUCTIONS_H

#include <ocr.h>

#define FANIN 8

ocrGuid_t build_reduction(ocrGuid_t sim, ocrGuid_t reduction, u32 leaves, ocrGuid_t* leaves_p,
                          u32 paramc, u64* paramv, ocrGuid_t (*red_edt)(u32, u64*, u32, ocrEdtDep_t*));

ocrGuid_t vred_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t tred_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t ured_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

#endif


#ifndef OUTLINE_H
#define OUTLINE_H

extern ocrGuid_t taskOutlineTemplate;
extern ocrGuid_t cleanupTemplate;

ocrGuid_t edtOutlineWrapper( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] );

ocrGuid_t edtCleanup( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] );

ocrGuid_t edtShutdown( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] );

#endif // OUTLINE_H


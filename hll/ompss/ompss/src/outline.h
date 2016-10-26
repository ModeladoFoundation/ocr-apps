
#ifndef OUTLINE_H
#define OUTLINE_H

#include "dependences.h"
#include "task.h"

namespace ompss {

extern ocrGuid_t taskOutlineTemplate;
extern ocrGuid_t cleanupTemplate;

ocrGuid_t edtOutlineWrapper( uint32_t paramc, uint64_t* paramv, uint32_t depc, ocrEdtDep_t depv[] );

ocrGuid_t edtCleanup( uint32_t paramc, uint64_t* paramv, uint32_t depc, ocrEdtDep_t depv[] );

ocrGuid_t edtShutdown( uint32_t paramc, uint64_t* paramv, uint32_t depc, ocrEdtDep_t depv[] );

} // namespace ompss

#endif // OUTLINE_H


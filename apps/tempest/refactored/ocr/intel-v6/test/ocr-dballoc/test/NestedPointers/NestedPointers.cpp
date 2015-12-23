#include "ocr.h"
#include "ocr_relative_ptr.hpp"

using namespace Ocr;

extern "C"
ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    int x = 7;
    RelPtr<int> p1 = &x;
    NestedRelPtr<int, 2> p2 = &p1;
    NestedRelPtr<int, 3> p3 = &p2;
    NestedRelPtr<int, 4> p4 = &p3;
    PRINTF("x = %d\n", ****p4);
    ocrShutdown();
    return 0;
}

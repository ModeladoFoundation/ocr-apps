#include "ocr_db_alloc.hpp"

#ifdef __APPLE__
// For some reason they don't support the standard C++ thread_local,
// but they *do* support the non-standard __thread extension for C.
#define thread_local __thread
#endif /* __APPLE__ */

namespace Ocr {
    namespace SimpleDbAllocator {
        static thread_local DatablockAllocator _localDbAllocator;

        DatablockAllocator &ocrAllocatorGet(void) {
            return _localDbAllocator;
        }

        void ocrAllocatorSetDb(void *dbPtr, size_t dbSize, bool needsInit) {
            new (&_localDbAllocator) DatablockAllocator(dbPtr, dbSize);
            if (needsInit) {
                _localDbAllocator.init();
            }
        }
    }
}

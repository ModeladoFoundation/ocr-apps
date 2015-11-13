#ifndef _OCR_DB_ALLOC_HPP_
#define _OCR_DB_ALLOC_HPP_

// Using placed-new syntax
#define ocrNewIn(A, T, ...) (new ((A).allocate(sizeof(T))) T(__VA_ARGS__))
#define ocrNew(T, ...) ocrNewIn(Ocr::SimpleDbAllocator::ocrAllocatorGet(), T, __VA_ARGS__)

// NOTE: No constructors are called here! Must call explicitly if needed.
#define ocrNewArrayIn(A, T, SZ) ((T*) (A).allocate(sizeof(T), SZ))
#define ocrNewArray(T, SZ) ocrNewArrayIn(Ocr::SimpleDbAllocator::ocrAllocatorGet(), T, SZ)

// Currently deleting is a no-op. All memory is freed with the datablock.
#define ocrDelete(ptr) /* NO-OP */
#define ocrDeleteArray(ptr) /* NO-OP */

#include <cstddef>
#include <cassert>
#include <memory>

namespace Ocr {
    namespace SimpleDbAllocator {
        class DatablockAllocator {
            private:
                char *const m_dbBuf;
                ptrdiff_t *const m_offset;

                inline void *allocateAligned(size_t size, int alignment) const {
                    assert(m_offset != nullptr && "Uninitialized allocator");
                    ptrdiff_t start = *m_offset & (-alignment);
                    *m_offset = start + size;
                    assert(&m_dbBuf[*m_offset] <= (char*)m_offset && "Datablock allocator overflow");
                    return (void*)&m_dbBuf[start];
                }

            public:
                constexpr DatablockAllocator(void):
                    m_dbBuf(nullptr), m_offset(nullptr) { }

                DatablockAllocator(void *dbPtr, size_t dbSize):
                    m_dbBuf((char*)dbPtr),
                    m_offset((ptrdiff_t*)&m_dbBuf[dbSize-sizeof(ptrdiff_t)])
                {
                    assert(dbPtr < (void*)m_offset && "Datablock is too small for allocator");
                }

                void init(void) const {
                    *m_offset = 0;
                }

                inline void *allocate(int size, int count) const {
                    assert(size > 0);
                    if (size == 1) {
                        return allocateAligned(1*count, 1);
                    }
                    else if (size <= 4) {
                        return allocateAligned(size*count, 4);
                    }
                    else if (size <= 8) {
                        return allocateAligned(size*count, 8);
                    }
                    else {
                        return allocateAligned(size*count, 16);
                    }
                }

                inline void *allocate(int size) const {
                    return allocate(size, 1);
                }

        };

        const DatablockAllocator &ocrAllocatorGet(void);
        void ocrAllocatorSetDb(void *dbPtr, size_t dbSize, bool needsInit);
    }
}

#endif /* _OCR_DB_ALLOC_HPP_ */

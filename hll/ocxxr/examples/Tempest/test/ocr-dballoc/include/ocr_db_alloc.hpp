#ifndef _OCR_DB_ALLOC_HPP_
#define _OCR_DB_ALLOC_HPP_

#define ocrNewIn(A, T, ...) (Ocr::NewIn<T>(A, __VA_ARGS__))
#define ocrNew(T, ...) (Ocr::New<T>(__VA_ARGS__))

#define ocrNewArrayIn(A, T, SZ) (Ocr::NewArrayIn<T>(A, SZ))
#define ocrNewArray(T, SZ) (Ocr::NewArray<T>(SZ))

// Currently deleting is a no-op. All memory is freed with the datablock.
#define ocrDelete(ptr) /* NO-OP */
#define ocrDeleteArray(ptr) /* NO-OP */

#include <cstddef>
#include <cassert>
#include <memory>

namespace Ocr {
    namespace SimpleDbAllocator {

        struct AllocatorState {
            ptrdiff_t offset;
        };

        class DatablockAllocator {
            private:
                char *const m_dbBuf;
                ptrdiff_t *const m_offset;

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

                AllocatorState saveState(void) {
                    return { *m_offset };
                }

                void restoreState(AllocatorState state) {
                    *m_offset = state.offset;
                }

                inline void *allocateAligned(size_t size, int alignment) const {
                    assert(m_offset != nullptr && "Uninitialized allocator");
                    ptrdiff_t start = (*m_offset + alignment - 1) & (-alignment);
                    *m_offset = start + size;
                    assert(&m_dbBuf[*m_offset] <= (char*)m_offset && "Datablock allocator overflow");
                    return (void*)&m_dbBuf[start];
                }

                // FIXME - I don't know if these alignment checks are sufficient,
                // especially if we do weird things like allocate a char[N]
                // so we have N bytes to store some struct or something.
                inline void *allocate(size_t size, size_t count) const {
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

                inline void *allocate(size_t size) const {
                    return allocate(size, 1);
                }

        };

        DatablockAllocator &ocrAllocatorGet(void);
        void ocrAllocatorSetDb(void *dbPtr, size_t dbSize, bool needsInit);
    }

    /////////////////////////////////////////////
    // Ocr::New

    template <typename T, typename ...Ts>
    static inline T* NewIn(SimpleDbAllocator::DatablockAllocator arena, Ts&&... args) {
        auto mem = arena.allocate(sizeof(T));
        return new (mem) T(std::forward<Ts>(args)...);
    }

    template <typename T, typename ...Ts>
    static inline T* New(Ts&&... args) {
        SimpleDbAllocator::DatablockAllocator arena = Ocr::SimpleDbAllocator::ocrAllocatorGet();
        return NewIn<T, Ts...>(arena, std::forward<Ts>(args)...);
    }

    /////////////////////////////////////////////
    // Constructor helper for Ocr::NewArray

    template <typename T, typename NoInit = void>
    struct TypeInitializer {
        static inline void init(T& target) {
            new (&target) T();
        }
    };

    template <typename T>
    struct TypeInitializer<T, typename std::enable_if<std::is_scalar<T>::value>::type> {
        static inline void init(T&) { }
    };

    /////////////////////////////////////////////
    // Ocr::NewArray

    template <typename T>
    static inline T* NewArrayIn(SimpleDbAllocator::DatablockAllocator arena, size_t count) {
        T* data = reinterpret_cast<T*>(arena.allocate(sizeof(T), count));
        for (size_t i=0; i<count; i++) {
            TypeInitializer<T>::init(data[i]);
        }
        return data;
    }

    template <typename T>
    static inline T* NewArray(size_t count) {
        SimpleDbAllocator::DatablockAllocator arena = Ocr::SimpleDbAllocator::ocrAllocatorGet();
        return NewArrayIn<T>(arena, count);
    }

}

#endif /* _OCR_DB_ALLOC_HPP_ */

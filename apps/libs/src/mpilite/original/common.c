/*
* This file is subject to the license agreement located in the file LICENSE
* and cannot be distributed without it. This notice cannot be
* removed or modified.
*/

#define ENABLE_ALLOCATOR_QUICK
#define ENABLE_ALLOCATOR_QUICK_STANDALONE

typedef unsigned long u64;
typedef long s64;
typedef unsigned int u32;
typedef int s32;
typedef unsigned char u8;
typedef char s8;



#if 0
#define COMPILE_ASSERT(a) assert(a)
#define ASSERT(a) assert(a)
//#define RESULT_ASSERT(a, op, b) do { assert((a) op (b), __FILE__, __LINE__); } while(0);
//#define RESULT_TRUE(a) do { assert((a) != 0, __FILE__, __LINE__); } while(0);
#define ASSERT_BLOCK_BEGIN(cond) if(!(cond)) {
#define ASSERT_BLOCK_END ASSERT(0 && "assert block failure"); }
#else
#define COMPILE_ASSERT(a)
#define ASSERT(a)
#define RESULT_ASSERT(a, op, b) do { a; } while(0);
#define RESULT_TRUE(a) do { a; } while(0);
#define ASSERT_BLOCK_BEGIN(cond) if(0) {
#define ASSERT_BLOCK_END }
#endif /* OCR_ASSERT */


#define POOL_HEADER_TYPE_MASK (7L)
#define allocatorQuick_id (0)

u32 fls64(u64 val) {
    return 63 - __builtin_clzl(val);
}

#define hal_xadd32(X, Y)    __sync_fetch_and_add_4 (X, Y)

#define hal_lock32(lock)                                    \
    do {                                                    \
        while(__sync_lock_test_and_set_4(lock, 1) != 0) ;     \
    } while(0);


#define hal_unlock32(lock)         \
    do {                           \
        __sync_lock_release(lock); \
    } while(0)


#define hal_trylock32(lock)                                             \
    ({                                                                  \
        u32 __tmp = __sync_lock_test_and_set_4(lock, 1);                  \
        __tmp;                                                          \
    })


#define DPRINTF(X, args...)             DPRINTF_##X(args)
//#define DPRINTF_DEBUG_LVL_VERB(args...)	printf("VERB " args)
#define DPRINTF_DEBUG_LVL_VERB(args...)
//#define DPRINTF_DEBUG_LVL_WARN(args...)
#define DPRINTF_DEBUG_LVL_WARN(args...)	printf("WARN " args)
#define DPRINTF_DEBUG_LVL_INFO(args...)
//#define DPRINTF_DEBUG_LVL_INFO(args...)	printf("INFO " args)

typedef struct _ocrPolicyDomain_t {
    int dummy;
} ocrPolicyDomain_t;


void *addrGlobalizeOnTG(void *result, ocrPolicyDomain_t *self) {return result;}


/*
 * Optimization PARSE[-I//usr/include, -I/home/pradelle/work/pca-ocr-db-autodec/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -I../include, -I/home/pradelle/work/pca-ocr-db-autodec/runtime/codelet/ocr/inc, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DOCR_TARGET, -D__RSTREAM_CC__, ../src/stencils/sor.c] (383ms)
 * Optimization SSA (37ms)
 * Optimization CCP (13ms)
 * Optimization DCE (8ms)
 * Optimization Raise[-liftnone, -stacktrace] (172ms)
 * Optimization PolyhedralMapperNew[-stacktrace, no-simplify-loop, no-spatial-layout, stacktrace=true, scal-dep, threadf=reduce_create=true, as=scopes, logfile=stencils/sor-mapper.log] (1858ms)
 * Optimization Lower[-stacktrace] (315ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization DCE (3ms)
 * Optimization SyntaxPrepass (12ms)
 * Optimization OSR (19ms)
 * Optimization DCE (8ms)
 * Optimization DeSSA (78ms)
 */
#include <rstream_ocr.h>
#include <ocr.h>
#define ENABLE_EXTENSION_LEGACY
#include <ocr-legacy.h>
typedef float real_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_21;
union __args_so4_22;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_23;
union __args_so_main_24;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct __anon_21 {
   real_t (* A)[10000];
   int IT0;
   int IT1;
};
union __args_so4_22 {
   struct __anon_21 data;
   double padding[2];
};
struct __anon_23 {
   real_t (* A)[10000];
};
union __args_so_main_24 {
   struct __anon_23 data;
   double padding[1];
};
/*
 * Forward declarations of functions
 */
void print_submatrix(char const*, void const*, int, int, int, int, int, int);
int check_matrices(char const*, char const*, void const*, void const*, int, int
   );
void initialize(void);
void initialize_once(void);
void show(void);
void kernel(void);
int check(void);
double flops_per_trial(void);
static void so(real_t (* A)[10000]);
static void so_1(real_t (* A)[10000]);
void so_2(real_t (* A)[10000]);
static ocrGuid_t so4(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
unsigned long so4_count_0(long* args);
ocrGuid_t rocrAlloc(void**, unsigned long);
static ocrGuid_t so_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv);
void so_async(void* event, real_t (* A)[10000]);
/*
 * Forward declarations of global variables
 */
extern int add_flop;
extern int mul_flop;
/* This can be redefined to be the appropriate 'inline' keyword */
#ifndef RSTREAM_INLINE
#define RSTREAM_INLINE
#endif
#ifdef GPU_TARGET
    #define __maxs_32(x,y) \
 max((int)(x),(int)(y))
#else
static RSTREAM_INLINE int __maxs_32(int x, int y) {
   return x > y ? x : y;
}
#endif
#ifdef GPU_TARGET
    #define __mins_32(x,y) \
 min((int)(x),(int)(y))
#else
static RSTREAM_INLINE int __mins_32(int x, int y) {
   return x < y ? x : y;
}
#endif
/*
 * Definitions of global variables
 */
extern int add_flop;
extern int mul_flop;
static real_t P[10000][10000];
static real_t Q[10000][10000];
int nb_samples = 100000000;
char const* function_name = "sor";
/*
 * Definitions of functions
 */
/*
 * ../src/stencils/sor.c:45.6
 */
void initialize(void)
{
   int i;
   int i_1;
   int i_2;
   for (i = 0, i_1 = 0, i_2 = 0; i < 10000; i++, i_1 += 0, i_2++) {
      int j;
      int j_1;
      for (j = 0, j_1 = i_1; j < 10000; j++, j_1 += i_2) {
         P[i][j] = (float)(0.3 + (double)j_1 * 0.5);
         Q[i][j] = (float)(0.3 + (double)j_1 * 0.5);
      }
   }
}

/*
 * ../src/stencils/sor.c:56.6
 */
void initialize_once(void)
{
   initialize();
}

/*
 * ../src/stencils/sor.c:62.6
 */
void show(void)
{
   print_submatrix("P", P, 10000, 10000, 0, 8, 0, 8);
   print_submatrix("P", P, 10000, 10000, 9992, 10000, 9992, 10000);
}

/*
 * ../src/stencils/sor.c:72.6
 */
void kernel(void)
{
   ocrGuid_t outEvt;
   union __args_so_main_24* allArgs;
   ocrGuid_t _t1;
   void* _t2;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, 0);
   _t2 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t2, so_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t2, so4, 0u, (unsigned long)395, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 8ul);
   allArgs->data.A = P;
   rocrExecute(_t2, so_main, _t1, &outEvt);
   rocrExit(_t2);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

/*
 * ../src/stencils/sor.c:76.5
 */
int check(void)
{
   int i;
   int i_1;
   int i_2;
   for (i = 1, i_1 = 0, i_2 = 2; i < 9999; i++, i_1++, i_2++) {
      int j;
      int j_1;
      int j_2;
      for (j = 1, j_1 = 0, j_2 = 2; j < 9999; j++, j_2++, j_1++) {
         Q[i][j] = (Q[i][j] + Q[i][j_1] + Q[i][j_2] + Q[i_1][j] + Q[i_2][j]) /
            5.0f;
      }
   }
   return check_matrices("P", "Q", P, Q, 10000, 10000);
}

/*
 * ../src/stencils/sor.c:84.8
 */
double flops_per_trial(void)
{
   return (double)(mul_flop + add_flop) * 10000.0 * 10000.0;
}

/*
 * ../src/stencils/sor.c:68.6
 */
static void so(real_t (* A)[10000])
{
   int i;
   int i_1;
   int i_2;
   for (i = 1, i_1 = 0, i_2 = 2; i < 9999; i++, i_1++, i_2++) {
      int j;
      int j_1;
      int j_2;
      for (j = 1, j_1 = 0, j_2 = 2; j < 9999; j_2++, j++, j_1++) {
         A[i][j] = (A[i][j] + A[i][j_1] + A[i][j_2] + A[i_1][j] + A[i_2][j]) /
            5.0f;
      }
   }
}

/*
 * ../src/stencils/sor.c:68.6
 */
static void so_1(real_t (* A)[10000])
{
   int i;
   int i_1;
   int i_2;
   for (i = 1, i_1 = 0, i_2 = 2; i < 9999; i++, i_1++, i_2++) {
      int j;
      int j_1;
      int j_2;
      for (j = 1, j_1 = 0, j_2 = 2; j < 9999; j_2++, j++, j_1++) {
         A[i][j] = (A[i][j] + A[i][j_1] + A[i][j_2] + A[i_1][j] + A[i_2][j]) /
            5.0f;
      }
   }
}

void so_2(real_t (* A)[10000])
{
   ocrGuid_t outEvt;
   union __args_so_main_24* allArgs;
   ocrGuid_t _t1;
   void* _t2;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, 0);
   _t2 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t2, so_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t2, so4, 0u, (unsigned long)395, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 8ul);
   allArgs->data.A = A;
   rocrExecute(_t2, so_main, _t1, &outEvt);
   rocrExit(_t2);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

static ocrGuid_t so4(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv)
{
   union __args_so4_22 so4_args;
   union __args_so4_22* allArgs;
   void* _t1;
   real_t (* A)[10000];
   int IT0;
   int IT1;
   int _t2;
   int i;
   int _t3;
   int i_1;
   int i_2;
   int i_3;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        so4_args = *(union __args_so4_22*)rocrArgs(depv),
        IT0 = so4_args.data.IT0,
        _t2 = (__mins_32(-128 * IT0 + 9997, 127)),
        IT1 = so4_args.data.IT1,
        A = so4_args.data.A,
        i = 0;
        i <= _t2; i++) {
      int _t4;
      int j;
      for (_t4 = (__mins_32(-2048 * IT1 + 9997, 2047)), j = 0; j <= _t4; j++) {
         A[j + (2048 * IT1 + 1)][i + (128 * IT0 + 1)] = (A[j + (2048 * IT1 + 1)
            ][i + (128 * IT0 + 1)] + A[j + (2048 * IT1 + 1)][i + 128 * IT0] + A
            [j + (2048 * IT1 + 1)][i + (128 * IT0 + 2)] + A[j + 2048 * IT1][i +
            (128 * IT0 + 1)] + A[j + (2048 * IT1 + 2)][i + (128 * IT0 + 1)]) /
            5.0f;
      }
   }
   for (_t3 = (__mins_32(78, IT0 + 1)),
        i_1 = (__maxs_32(IT0 + IT1 + -3, IT0)),
        i_2 = i_1 * -1 + IT0,
        i_3 = i_1 * 5;
        i_1 <= _t3; i_3 += 5, i_2--, i_1++) {
      ocrGuid_t _t5;
      union __args_so4_22* _t6;
      int _t7;
      _t5 = rocrAlloc((void**)&allArgs, 16ul);
      _t6 = allArgs;
      _t6->data.A = A;
      _t6->data.IT0 = i_1;
      _t7 = i_2 + (IT1 + 1);
      _t6->data.IT1 = _t7;
      rocrAutoDec(_t1, 0u, (unsigned long)(i_2 + (IT1 + 1) + i_3), _t5,
         so4_count_0, (void (*)(void*, long*))((void*)0l), (void (*)(void*,
         long*))((void*)0l), 2u, (long)i_1, (long)_t7);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

unsigned long so4_count_0(long* args)
{
   unsigned long rcNbInputDeps;
   int _t1;
   int _t2;
   int i;
   for (_t1 = (int)args[0],
        _t2 = (__mins_32(_t1, _t1 + (int)args[1] + -1)),
        i = (__maxs_32(_t1 + -1, 0)),
        rcNbInputDeps = 0ul;
        i <= _t2; i++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

static ocrGuid_t so_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_so_main_24 so_main_args;
   union __args_so4_22* allArgs;
   void* _t1;
   ocrGuid_t _t2;
   union __args_so4_22* _t3;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   so_main_args = *(union __args_so_main_24*)rocrArgs(depv);
   _t2 = rocrAlloc((void**)&allArgs, 16ul);
   _t3 = allArgs;
   _t3->data.A = so_main_args.data.A;
   _t3->data.IT0 = 0;
   _t3->data.IT1 = 0;
   rocrAutoDec(_t1, 0u, (unsigned long)0, _t2, so4_count_0, (void (*)(void*,
      long*))((void*)0l), (void (*)(void*, long*))((void*)0l), 2u, (long)0, (
      long)0);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void so_async(void* event, real_t (* A)[10000])
{
   union __args_so_main_24* allArgs;
   ocrGuid_t _t1;
   void* _t2;
   _t2 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t2, so_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t2, so4, 0u, (unsigned long)395, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 8ul);
   allArgs->data.A = A;
   rocrExecute(_t2, so_main, _t1, event);
   rocrExit(_t2);
}


/*
 * Optimization PARSE[-I//usr/include, -I/home/pradelle/work/pca-ocr-db-autodec/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -I../include, -I/home/pradelle/work/pca-ocr-db-autodec/runtime/codelet/ocr/inc, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DOCR_TARGET, -D__RSTREAM_CC__, ../src/one_d_imperfect_jacobi.c] (387ms)
 * Optimization SSA (38ms)
 * Optimization CCP (13ms)
 * Optimization DCE (8ms)
 * Optimization Raise[-liftnone, -stacktrace] (170ms)
 * Optimization PolyhedralMapperNew[-stacktrace, no-simplify-loop, no-spatial-layout, stacktrace=true, scal-dep, threadf=reduce_create=true, as=scopes, logfile=one_d_imperfect_jacobi-mapper.log] (2389ms)
 * Optimization Lower[-stacktrace] (448ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
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
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization DCE (3ms)
 * Optimization SyntaxPrepass (9ms)
 * Optimization OSR (18ms)
 * Optimization DCE (8ms)
 * Optimization DeSSA (63ms)
 */
#include <stdio.h>

#include <rstream_ocr.h>
#include <ocr.h>
#define ENABLE_EXTENSION_LEGACY
#include <ocr-legacy.h>
typedef float real_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_21;
union __args_imp4_22;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_23;
union __args_imp_main_24;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct __anon_21 {
   real_t* B;
   real_t* A;
   int IT0;
   int IT1;
};
union __args_imp4_22 {
   struct __anon_21 data;
   double padding[3];
};
struct __anon_23 {
   real_t* B;
   real_t* A;
};
union __args_imp_main_24 {
   struct __anon_23 data;
   double padding[2];
};
/*
 * Forward declarations of functions
 */
int check_vectors(char const*, char const*, void const*, void const*, int);
void initialize(void);
void initialize_once(void);
void show(void);
void kernel(void);
int check(void);
double flops_per_trial(void);
static void imp(real_t* A, real_t* B);
static void imp_1(real_t* A, real_t* B);
void imp_2(real_t* A, real_t* B);
static ocrGuid_t imp4(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
unsigned long imp4_count_0(long* args);
ocrGuid_t rocrAlloc(void**, unsigned long);
static ocrGuid_t imp_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv);
void imp_async(void* event, real_t* A, real_t* B);
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
static real_t a[5000];
static real_t b[5000];
static real_t aa[5000];
static real_t bb[5000];
int nb_samples = 25000000;
char const* function_name = "one_d_imperfect_jacobi";
/*
 * Definitions of functions
 */
/*
 * ../src/one_d_imperfect_jacobi.c:54.6
 */
void initialize(void)
{
   int i;
   for (i = 0; i < 5000; i++) {
      real_t _t1;
      _t1 = (float)(i / 2);
      bb[i] = _t1;
      b[i] = _t1;
      aa[i] = _t1;
      a[i] = _t1;
   }
}

/*
 * ../src/one_d_imperfect_jacobi.c:62.6
 */
void initialize_once(void)
{
   initialize();
}

/*
 * ../src/one_d_imperfect_jacobi.c:66.6
 */
void show(void)
{
   int i;
   int i_1;
   for (i = 0; i < 8; i++) {
      fprintf(stderr, "e[%d] = %10.10f\n", i, (double)a[i]);
   }
   for (i_1 = 4992; i_1 < 5000; i_1++) {
      fprintf(stderr, "e[%d] = %10.10f\n", i_1, (double)a[i_1]);
   }
}

/*
 * ../src/one_d_imperfect_jacobi.c:83.6
 */
void kernel(void)
{
   ocrGuid_t outEvt;
   union __args_imp_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_imp_main_24* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, 0);
   _t3 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, imp_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t3, imp4, 0u, (unsigned long)150, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 16ul);
   _t2 = allArgs;
   _t2->data.B = b;
   _t2->data.A = a;
   rocrExecute(_t3, imp_main, _t1, &outEvt);
   rocrExit(_t3);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

/*
 * ../src/one_d_imperfect_jacobi.c:87.5
 */
int check(void)
{
   int t;
   for (t = 0; t < 5000; t++) {
      int i;
      int i_1;
      int i_2;
      int j;
      for (i = 2, i_1 = 1, i_2 = 3; i < 4999; i_2++, i++, i_1++) {
         bb[i] = 0.3330000043f * (aa[i_1] + aa[i] + aa[i_2]);
      }
      for (j = 2; j < 4999; j++) {
         aa[j] = bb[j];
      }
   }
   return check_vectors("a", "aa", a, aa, 5000);
}

/*
 * ../src/one_d_imperfect_jacobi.c:94.8
 */
double flops_per_trial(void)
{
   return (double)(mul_flop + add_flop) * 2.0 * 5000.0 * 5000.0;
}

/*
 * ../src/one_d_imperfect_jacobi.c:79.6
 */
static void imp(real_t* A, real_t* B)
{
   int t;
   for (t = 0; t < 5000; t++) {
      int i;
      int i_1;
      int i_2;
      int j;
      for (i = 2, i_1 = 1, i_2 = 3; i < 4999; i_2++, i++, i_1++) {
         B[i] = 0.3330000043f * (A[i_1] + A[i] + A[i_2]);
      }
      for (j = 2; j < 4999; j++) {
         A[j] = B[j];
      }
   }
}

/*
 * ../src/one_d_imperfect_jacobi.c:79.6
 */
static void imp_1(real_t* A, real_t* B)
{
   int t;
   for (t = 0; t < 5000; t++) {
      int i;
      int i_1;
      int i_2;
      int j;
      for (i = 2, i_1 = 1, i_2 = 3; i < 4999; i_2++, i++, i_1++) {
         B[i] = 0.3330000043f * (A[i_1] + A[i] + A[i_2]);
      }
      for (j = 2; j < 4999; j++) {
         A[j] = B[j];
      }
   }
}

void imp_2(real_t* A, real_t* B)
{
   ocrGuid_t outEvt;
   union __args_imp_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_imp_main_24* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, 0);
   _t3 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, imp_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t3, imp4, 0u, (unsigned long)150, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 16ul);
   _t2 = allArgs;
   _t2->data.B = B;
   _t2->data.A = A;
   rocrExecute(_t3, imp_main, _t1, &outEvt);
   rocrExit(_t3);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

static ocrGuid_t imp4(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv)
{
   union __args_imp4_22 imp4_args;
   union __args_imp4_22* allArgs;
   void* _t1;
   real_t* B;
   real_t* A;
   int IT0;
   int IT1;
   int _t2;
   int i;
   int i_1;
   int i_2;
   int _t3;
   int i_3;
   int i_4;
   int i_5;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        imp4_args = *(union __args_imp4_22*)rocrArgs(depv),
        IT0 = imp4_args.data.IT0,
        _t2 = (__mins_32(-512 * IT0 + 4999, 511)),
        A = imp4_args.data.A,
        B = imp4_args.data.B,
        IT1 = imp4_args.data.IT1,
        i = (__maxs_32(0, -512 * IT0 + 512 * IT1 + -2498)),
        i_1 = i * -2,
        i_2 = i << 1;
        i <= _t2; i++, i_1 -= 2, i_2 += 2) {
      int _t4;
      int j;
      int j_1;
      int j_2;
      int j_3;
      int j_4;
      int j_5;
      int j_6;
      if (i + 512 * IT0 + -512 * IT1 >= 0) {
         B[2] = 0.3330000043f * (A[1] + A[2] + A[3]);
      }
      for (_t4 = (__mins_32(1023, i_2 + 1024 * IT0 + -1024 * IT1 + 4996)),
           j = (__maxs_32(i_2 + 1024 * IT0 + -1024 * IT1 + 1, 0)),
           j_6 = j + i_1,
           j_5 = j + i_1,
           j_1 = j + i_1,
           j_2 = j + i_1,
           j_3 = j + i_1,
           j_4 = j + i_1;
           j <= _t4;
           j_6++,
           j_5++,
           j_4++,
           j_3++,
           j_2++,
           j_1++,
           j++) {
         B[j_1 + (-1024 * IT0 + (1024 * IT1 + 2))] = 0.3330000043f * (A[j_2 + (
            -1024 * IT0 + (1024 * IT1 + 1))] + A[j_3 + (-1024 * IT0 + (1024 *
            IT1 + 2))] + A[j_4 + (-1024 * IT0 + (1024 * IT1 + 3))]);
         A[j_5 + (-1024 * IT0 + (1024 * IT1 + 1))] = B[j_6 + (-1024 * IT0 + (
            1024 * IT1 + 1))];
      }
      if (- i + -512 * IT0 + 512 * IT1 >= 1987) {
         A[4998] = B[4998];
      }
   }
   for (_t3 = (__mins_32(IT0 + IT1 + 1 >> 1, __mins_32(9, IT0 + 1))),
        i_3 = (__maxs_32(IT0, IT0 + IT1 + -4 + 1 >> 1)),
        i_4 = i_3 * -1 + IT0,
        i_5 = i_3 * 15;
        i_3 <= _t3; i_5 += 15, i_4--, i_3++) {
      ocrGuid_t _t5;
      union __args_imp4_22* _t6;
      int _t7;
      _t5 = rocrAlloc((void**)&allArgs, 24ul);
      _t6 = allArgs;
      _t6->data.B = B;
      _t6->data.A = A;
      _t6->data.IT0 = i_3;
      _t7 = i_4 + (IT1 + 1);
      _t6->data.IT1 = _t7;
      rocrAutoDec(_t1, 0u, (unsigned long)(i_4 + (IT1 + 1) + i_5), _t5,
         imp4_count_0, (void (*)(void*, long*))((void*)0l), (void (*)(void*,
         long*))((void*)0l), 2u, (long)i_3, (long)_t7);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

unsigned long imp4_count_0(long* args)
{
   unsigned long rcNbInputDeps;
   int _t1;
   int _t2;
   int _t3;
   int i;
   for (_t1 = (int)args[0],
        _t2 = (int)args[1],
        _t3 = (__mins_32(_t1 + _t2 + -1 >> 1, _t1)),
        i = (__maxs_32(_t1 + -1, __maxs_32(0, _t1 + _t2 + -6 + 1 >> 1))),
        rcNbInputDeps = 0ul;
        i <= _t3; i++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

static ocrGuid_t imp_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_imp_main_24 imp_main_args;
   union __args_imp4_22* allArgs;
   void* _t1;
   ocrGuid_t _t2;
   union __args_imp4_22* _t3;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   imp_main_args = *(union __args_imp_main_24*)rocrArgs(depv);
   _t2 = rocrAlloc((void**)&allArgs, 24ul);
   _t3 = allArgs;
   _t3->data.B = imp_main_args.data.B;
   _t3->data.A = imp_main_args.data.A;
   _t3->data.IT0 = 0;
   _t3->data.IT1 = 0;
   rocrAutoDec(_t1, 0u, (unsigned long)0, _t2, imp4_count_0, (void (*)(void*,
      long*))((void*)0l), (void (*)(void*, long*))((void*)0l), 2u, (long)0, (
      long)0);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void imp_async(void* event, real_t* A, real_t* B)
{
   union __args_imp_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_imp_main_24* _t2;
   void* _t3;
   _t3 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, imp_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t3, imp4, 0u, (unsigned long)150, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 16ul);
   _t2 = allArgs;
   _t2->data.B = B;
   _t2->data.A = A;
   rocrExecute(_t3, imp_main, _t1, event);
   rocrExit(_t3);
}


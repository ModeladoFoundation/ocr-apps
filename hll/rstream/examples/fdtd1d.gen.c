/*
 * Optimization PARSE[-I//usr/include, -I/home/pradelle/work/pca-ocr-db-autodec/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -I../include, -I/home/pradelle/work/pca-ocr-db-autodec/runtime/codelet/ocr/inc, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DOCR_TARGET, -D__RSTREAM_CC__, ../src/fdtd1d.c] (352ms)
 * Optimization SSA (39ms)
 * Optimization CCP (14ms)
 * Optimization DCE (8ms)
 * Optimization Raise[-liftnone, -stacktrace] (177ms)
 * Optimization PolyhedralMapperNew[-stacktrace, no-simplify-loop, no-spatial-layout, stacktrace=true, scal-dep, threadf=reduce_create=true, as=scopes, tile=edt_override, logfile=fdtd1d-mapper.log] (1733ms)
 * Optimization Lower[-stacktrace] (514ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization InsertMapperAnnotations (2ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization DCE (4ms)
 * Optimization SyntaxPrepass (10ms)
 * Optimization OSR (18ms)
 * Optimization DCE (9ms)
 * Optimization DeSSA (60ms)
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
union __args_fd4_22;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_23;
union __args_fd_main_24;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct __anon_21 {
   real_t* A;
   real_t* B;
   int IT0;
};
union __args_fd4_22 {
   struct __anon_21 data;
   double padding[3];
};
struct __anon_23 {
   real_t* A;
   real_t* B;
};
union __args_fd_main_24 {
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
static void fd(real_t* A, real_t* B);
static void fd_1(real_t* A, real_t* B);
void fd_2(real_t* A, real_t* B);
static ocrGuid_t fd4(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
unsigned long fd4_count_0(long* args);
ocrGuid_t rocrAlloc(void**, unsigned long);
static ocrGuid_t fd_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv);
void fd_async(void* event, real_t* A, real_t* B);
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
static real_t e[5000];
static real_t h[5000];
static real_t ee[5000];
static real_t hh[5000];
int nb_samples = 25000000;
char const* function_name = "fdtd1d";
/*
 * Definitions of functions
 */
/*
 * ../src/fdtd1d.c:55.6
 */
void initialize(void)
{
   int i;
   for (i = 0; i < 5000; i++) {
      real_t _t1;
      float _t2;
      _t1 = (float)i;
      ee[i] = _t1;
      e[i] = _t1;
      _t2 = (float)i * 0.5f;
      hh[i] = _t2;
      h[i] = _t2;
   }
}

/*
 * ../src/fdtd1d.c:65.6
 */
void initialize_once(void)
{
   initialize();
}

/*
 * ../src/fdtd1d.c:69.6
 */
void show(void)
{
   int i;
   int i_1;
   for (i = 0; i < 8; i++) {
      fprintf(stderr, "e[%d] = %10.10f\n", i, (double)e[i]);
   }
   for (i_1 = 4992; i_1 < 5000; i_1++) {
      fprintf(stderr, "e[%d] = %10.10f\n", i_1, (double)e[i_1]);
   }
}

/*
 * ../src/fdtd1d.c:84.6
 */
void kernel(void)
{
   ocrGuid_t outEvt;
   union __args_fd_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_fd_main_24* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, 0);
   _t3 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, fd_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t3, fd4, 0u, (unsigned long)313, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 16ul);
   _t2 = allArgs;
   _t2->data.A = e;
   _t2->data.B = h;
   rocrExecute(_t3, fd_main, _t1, &outEvt);
   rocrExit(_t3);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

/*
 * ../src/fdtd1d.c:88.5
 */
int check(void)
{
   int t;
   for (t = 0; t < 5000; t++) {
      int i;
      int i_1;
      int i_2;
      int i_3;
      for (i = 1, i_1 = 0; i < 4999; i++, i_1++) {
         ee[i] = ee[i] - 0.5f * (hh[i] - hh[i_1]);
      }
      for (i_2 = 0, i_3 = 1; i_2 < 4999; i_3++, i_2++) {
         hh[i_2] = hh[i_2] - 0.6999999881f * (ee[i_3] - ee[i_2]);
      }
   }
   return check_vectors("e", "ee", e, ee, 5000);
}

/*
 * ../src/fdtd1d.c:95.8
 */
double flops_per_trial(void)
{
   return (double)(mul_flop + add_flop) * 2.0 * 5000.0 * 5000.0;
}

/*
 * ../src/fdtd1d.c:81.6
 */
static void fd(real_t* A, real_t* B)
{
   int t;
   for (t = 0; t < 5000; t++) {
      int i;
      int i_1;
      int i_2;
      int i_3;
      for (i = 1, i_1 = 0; i < 4999; i++, i_1++) {
         A[i] = A[i] - 0.5f * (B[i] - B[i_1]);
      }
      for (i_2 = 0, i_3 = 1; i_2 < 4999; i_2++, i_3++) {
         B[i_2] = B[i_2] - 0.6999999881f * (A[i_3] - A[i_2]);
      }
   }
}

/*
 * ../src/fdtd1d.c:81.6
 */
static void fd_1(real_t* A, real_t* B)
{
   int t;
   for (t = 0; t < 5000; t++) {
      int i;
      int i_1;
      int i_2;
      int i_3;
      for (i = 1, i_1 = 0; i < 4999; i++, i_1++) {
         A[i] = A[i] - 0.5f * (B[i] - B[i_1]);
      }
      for (i_2 = 0, i_3 = 1; i_2 < 4999; i_2++, i_3++) {
         B[i_2] = B[i_2] - 0.6999999881f * (A[i_3] - A[i_2]);
      }
   }
}

void fd_2(real_t* A, real_t* B)
{
   ocrGuid_t outEvt;
   union __args_fd_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_fd_main_24* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, 0);
   _t3 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, fd_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t3, fd4, 0u, (unsigned long)313, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 16ul);
   _t2 = allArgs;
   _t2->data.A = A;
   _t2->data.B = B;
   rocrExecute(_t3, fd_main, _t1, &outEvt);
   rocrExit(_t3);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

static ocrGuid_t fd4(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv)
{
   union __args_fd4_22 fd4_args;
   union __args_fd4_22* allArgs;
   void* _t1;
   real_t* A;
   real_t* B;
   int IT0;
   int _t2;
   int i;
   int i_1;
   int i_2;
   int i_3;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        fd4_args = *(union __args_fd4_22*)rocrArgs(depv),
        IT0 = fd4_args.data.IT0,
        _t2 = (__mins_32(IT0 + 313 >> 2, 156)),
        B = fd4_args.data.B,
        A = fd4_args.data.A,
        i = IT0 + -3 + 3 >> 2,
        i_2 = i * -64,
        i_1 = i << 6,
        i_3 = i << 2;
        i <= _t2;
        i++,
        i_1 += 64,
        i_2 -= 64,
        i_3 += 4) {
      int _t3;
      int _t4;
      int j;
      int j_1;
      int j_2;
      int j_3;
      int j_4;
      int j_5;
      int j_6;
      int j_7;
      int j_8;
      if (IT0 >= 3 && ((- IT0 + 3 & 3) == 0 && i_3 + - IT0 == 313)) {
         B[4998] = B[4998] - 0.6999999881f * (A[4999] - A[4998]);
      }
      for (_t3 = (__mins_32(-16 * IT0 + 4999, 15)),
           j = (__maxs_32(i_1 + -16 * IT0 + -4997, 0)),
           j_8 = j + i_2,
           _t4 = j * -1,
           j_1 = _t4 + i_1,
           j_2 = j + i_2,
           j_3 = _t4 + i_1,
           j_4 = _t4 + i_1,
           j_5 = _t4 + i_1,
           j_6 = _t4 + i_1,
           j_7 = _t4 + i_1;
           j <= _t3;
           j++,
           j_8++,
           j_1--,
           j_2++,
           j_3--,
           j_4--,
           j_5--,
           j_6--,
           j_7--) {
         int _t5;
         int k;
         for (_t5 = (__mins_32(j_8 + 16 * IT0 + 4997, 63)), k = (__maxs_32(0,
                 j_2 + 16 * IT0)); k <= _t5; k++) {
            A[j_1 + (k + (-16 * IT0 + 1))] = A[j_1 + (k + (-16 * IT0 + 1))] -
               0.5f * (B[j_3 + (k + (-16 * IT0 + 1))] - B[j_4 + (k + -16 * IT0)
               ]);
            B[j_5 + (k + -16 * IT0)] = B[j_5 + (k + -16 * IT0)] - 0.6999999881f
               * (A[j_6 + (k + (-16 * IT0 + 1))] - A[j_7 + (k + -16 * IT0)]);
         }
         if (i_1 + - j + -16 * IT0 >= 4935) {
            B[4998] = B[4998] - 0.6999999881f * (A[4999] - A[4998]);
         }
      }
   }
   if (IT0 <= 311) {
      ocrGuid_t _t6;
      union __args_fd4_22* _t7;
      int _t8;
      _t6 = rocrAlloc((void**)&allArgs, 24ul);
      _t7 = allArgs;
      _t7->data.A = A;
      _t7->data.B = B;
      _t8 = IT0 + 1;
      _t7->data.IT0 = _t8;
      rocrAutoDec(_t1, 0u, (unsigned long)(IT0 + 1), _t6, fd4_count_0, (void (*
         )(void*, long*))((void*)0l), (void (*)(void*, long*))((void*)0l), 1u,
         (long)_t8);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

unsigned long fd4_count_0(long* args)
{
   unsigned long rcNbInputDeps;
   if ((int)args[0] >= 1) {
      rcNbInputDeps = 0ul + 1ul;
   } else {
      rcNbInputDeps = 0ul;
   }
   return rcNbInputDeps;
}

static ocrGuid_t fd_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_fd_main_24 fd_main_args;
   union __args_fd4_22* allArgs;
   void* _t1;
   ocrGuid_t _t2;
   union __args_fd4_22* _t3;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   fd_main_args = *(union __args_fd_main_24*)rocrArgs(depv);
   _t2 = rocrAlloc((void**)&allArgs, 24ul);
   _t3 = allArgs;
   _t3->data.A = fd_main_args.data.A;
   _t3->data.B = fd_main_args.data.B;
   _t3->data.IT0 = 0;
   rocrAutoDec(_t1, 0u, (unsigned long)0, _t2, fd4_count_0, (void (*)(void*,
      long*))((void*)0l), (void (*)(void*, long*))((void*)0l), 1u, (long)0);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void fd_async(void* event, real_t* A, real_t* B)
{
   union __args_fd_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_fd_main_24* _t2;
   void* _t3;
   _t3 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, fd_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t3, fd4, 0u, (unsigned long)313, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 16ul);
   _t2 = allArgs;
   _t2->data.A = A;
   _t2->data.B = B;
   rocrExecute(_t3, fd_main, _t1, event);
   rocrExit(_t3);
}


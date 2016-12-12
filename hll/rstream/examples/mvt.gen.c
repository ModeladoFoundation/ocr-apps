/*
 * Optimization PARSE[-I//usr/include, -I/home/pradelle/work/pca-ocr-db-autodec/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -I../include, -I/home/pradelle/work/pca-ocr-db-autodec/runtime/codelet/ocr/inc, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DOCR_TARGET, -D__RSTREAM_CC__, ../src/mvt.c] (385ms)
 * Optimization SSA (38ms)
 * Optimization CCP (14ms)
 * Optimization DCE (9ms)
 * Optimization Raise[-liftnone, -stacktrace] (184ms)
 * Optimization PolyhedralMapperNew[-stacktrace, no-simplify-loop, no-spatial-layout, stacktrace=true, scal-dep, threadf=reduce_create=true, as=scopes, logfile=mvt-mapper.log] (1758ms)
 * Optimization Lower[-stacktrace] (263ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization DCE (3ms)
 * Optimization SyntaxPrepass (11ms)
 * Optimization OSR (15ms)
 * Optimization DCE (8ms)
 * Optimization DeSSA (69ms)
 */
#include <rstream_ocr.h>
#include <ocr.h>
#define ENABLE_EXTENSION_LEGACY
#include <ocr-legacy.h>
typedef float real_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_21;
union __args_mv4_22;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_23;
union __args_mv_main_24;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct __anon_21 {
   real_t* B;
   real_t (* A)[2000];
   real_t* D;
   real_t* C;
   real_t* E;
   int IT0;
   int IT1;
};
union __args_mv4_22 {
   struct __anon_21 data;
   double padding[6];
};
struct __anon_23 {
   real_t* B;
   real_t (* A)[2000];
   real_t* D;
   real_t* C;
   real_t* E;
};
union __args_mv_main_24 {
   struct __anon_23 data;
   double padding[5];
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
static void mv(real_t (* A)[2000], real_t* B, real_t* C, real_t* D, real_t* E);
static void mv_1(real_t (* A)[2000], real_t* B, real_t* C, real_t* D, real_t* E
   );
void mv_2(real_t (* A)[2000], real_t* B, real_t* C, real_t* D, real_t* E);
static ocrGuid_t mv4(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
ocrGuid_t rocrAlloc(void**, unsigned long);
static ocrGuid_t mv_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv);
void mv_async(void* event, real_t (* A)[2000], real_t* B, real_t* C, real_t* D,
    real_t* E);
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
static real_t x1[2000];
static real_t x2[2000];
static real_t a[2000][2000];
static real_t y_1[2000];
static real_t y_2[2000];
static real_t Tx1[2000];
static real_t Tx2[2000];
static real_t Ta[2000][2000];
static real_t Ty_1[2000];
static real_t Ty_2[2000];
int nb_samples = 4000000;
char const* function_name = "mvt";
/*
 * Definitions of functions
 */
/*
 * ../src/mvt.c:52.6
 */
void initialize(void)
{
   int i;
   int i_1;
   int i_2;
   for (i = 0, i_1 = 0, i_2 = 0; i < 2000; i++, i_1 += 0, i_2++) {
      real_t _t1;
      real_t _t2;
      int j;
      int j_1;
      _t1 = (float)((double)i * 0.5 + 2.3);
      Tx2[i] = _t1;
      Tx1[i] = _t1;
      x2[i] = _t1;
      x1[i] = _t1;
      _t2 = (float)((double)i * 0.5 + 2.3);
      Ty_2[i] = _t2;
      Ty_1[i] = _t2;
      y_2[i] = _t2;
      y_1[i] = _t2;
      for (j = 0, j_1 = i_1; j < 2000; j++, j_1 += i_2) {
         float _t3;
         _t3 = (float)j_1 * 0.5f + 2.299999952f;
         Ta[i][j] = _t3;
         a[i][j] = _t3;
      }
   }
}

/*
 * ../src/mvt.c:65.6
 */
void initialize_once(void)
{
   initialize();
}

/*
 * ../src/mvt.c:69.6
 */
void show(void)
{
   print_submatrix("a", a, 2000, 2000, 0, 8, 0, 8);
   print_submatrix("a", a, 2000, 2000, 1992, 2000, 1992, 2000);
}

/*
 * ../src/mvt.c:78.6
 */
void kernel(void)
{
   ocrGuid_t outEvt;
   union __args_mv_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_mv_main_24* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, 0);
   _t3 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, mv_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t3, mv4, 0u, (unsigned long)16, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 40ul);
   _t2 = allArgs;
   _t2->data.B = x1;
   _t2->data.A = a;
   _t2->data.D = y_1;
   _t2->data.C = x2;
   _t2->data.E = y_2;
   rocrExecute(_t3, mv_main, _t1, &outEvt);
   rocrExit(_t3);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

/*
 * ../src/mvt.c:82.5
 */
int check(void)
{
   int i;
   int i_1;
   for (i = 0; i < 2000; i++) {
      int j;
      for (j = 0; j < 2000; j++) {
         Tx1[i] = Tx1[i] + Ta[i][j] * Ty_1[j];
      }
   }
   for (i_1 = 0; i_1 < 2000; i_1++) {
      int j;
      for (j = 0; j < 2000; j++) {
         Tx2[i_1] = Tx2[i_1] + Ta[j][i_1] * Ty_2[j];
      }
   }
   return check_matrices("a", "Ta", a, Ta, 2000, 2000);
}

/*
 * ../src/mvt.c:89.8
 */
double flops_per_trial(void)
{
   return (double)(mul_flop + add_flop) * 2.0 * 2000.0 * 2000.0;
}

/*
 * ../src/mvt.c:75.6
 */
static void mv(real_t (* A)[2000], real_t* B, real_t* C, real_t* D, real_t* E)
{
   int i;
   int i_1;
   for (i = 0; i < 2000; i++) {
      int j;
      for (j = 0; j < 2000; j++) {
         B[i] = B[i] + A[i][j] * D[j];
      }
   }
   for (i_1 = 0; i_1 < 2000; i_1++) {
      int j;
      for (j = 0; j < 2000; j++) {
         C[i_1] = C[i_1] + A[j][i_1] * E[j];
      }
   }
}

/*
 * ../src/mvt.c:75.6
 */
static void mv_1(real_t (* A)[2000], real_t* B, real_t* C, real_t* D, real_t* E
   )
{
   int i;
   int i_1;
   for (i = 0; i < 2000; i++) {
      int j;
      for (j = 0; j < 2000; j++) {
         B[i] = B[i] + A[i][j] * D[j];
      }
   }
   for (i_1 = 0; i_1 < 2000; i_1++) {
      int j;
      for (j = 0; j < 2000; j++) {
         C[i_1] = C[i_1] + A[j][i_1] * E[j];
      }
   }
}

void mv_2(real_t (* A)[2000], real_t* B, real_t* C, real_t* D, real_t* E)
{
   ocrGuid_t outEvt;
   union __args_mv_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_mv_main_24* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, 0);
   _t3 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, mv_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t3, mv4, 0u, (unsigned long)16, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 40ul);
   _t2 = allArgs;
   _t2->data.B = B;
   _t2->data.A = A;
   _t2->data.D = D;
   _t2->data.C = C;
   _t2->data.E = E;
   rocrExecute(_t3, mv_main, _t1, &outEvt);
   rocrExit(_t3);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

static ocrGuid_t mv4(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv)
{
   union __args_mv4_22 mv4_args;
   void* _t1;
   real_t* B;
   real_t (* A)[2000];
   real_t* D;
   real_t* C;
   real_t* E;
   int IT0;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mv4_args = *(union __args_mv4_22*)rocrArgs(depv);
   IT0 = mv4_args.data.IT0;
   E = mv4_args.data.E;
   C = mv4_args.data.C;
   D = mv4_args.data.D;
   A = mv4_args.data.A;
   B = mv4_args.data.B;
   if (mv4_args.data.IT1 == 0) {
      int _t2;
      int i;
      for (_t2 = (__mins_32(-128 * IT0 + 1999, 127)), i = 0; i <= _t2; i++) {
         int j;
         for (j = 0; j <= 1999; j++) {
            B[i + 128 * IT0] = B[i + 128 * IT0] + A[i + 128 * IT0][j] * D[j];
            C[i + 128 * IT0] = C[i + 128 * IT0] + A[j][i + 128 * IT0] * E[j];
         }
      }
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t mv_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mv_main_24 mv_main_args;
   union __args_mv4_22* allArgs;
   void* _t1;
   real_t* B;
   real_t (* A)[2000];
   real_t* D;
   real_t* C;
   real_t* E;
   int i;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        mv_main_args = *(union __args_mv_main_24*)rocrArgs(depv),
        E = mv_main_args.data.E,
        C = mv_main_args.data.C,
        D = mv_main_args.data.D,
        A = mv_main_args.data.A,
        B = mv_main_args.data.B,
        i = 0;
        i <= 15; i++) {
      ocrGuid_t _t2;
      union __args_mv4_22* _t3;
      _t2 = rocrAlloc((void**)&allArgs, 48ul);
      _t3 = allArgs;
      _t3->data.B = B;
      _t3->data.A = A;
      _t3->data.D = D;
      _t3->data.C = C;
      _t3->data.E = E;
      _t3->data.IT0 = i;
      _t3->data.IT1 = 0;
      rocrAutoDec(_t1, 0u, (unsigned long)i, _t2, (unsigned long (*)(long*))((
         void*)0l), (void (*)(void*, long*))((void*)0l), (void (*)(void*, long*
         ))((void*)0l), 2u, (long)i, (long)0);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mv_async(void* event, real_t (* A)[2000], real_t* B, real_t* C, real_t* D,
    real_t* E)
{
   union __args_mv_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_mv_main_24* _t2;
   void* _t3;
   _t3 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, mv_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t3, mv4, 0u, (unsigned long)16, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 40ul);
   _t2 = allArgs;
   _t2->data.B = B;
   _t2->data.A = A;
   _t2->data.D = D;
   _t2->data.C = C;
   _t2->data.E = E;
   rocrExecute(_t3, mv_main, _t1, event);
   rocrExit(_t3);
}


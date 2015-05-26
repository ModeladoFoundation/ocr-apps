/*
 * Optimization PARSE[-I//usr/include, -I/opt/reservoir/staging/rstream-3.15.0.1/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DOCR_TARGET=1, --c99, -D__RSTREAM_CC__, mvt.c] (349ms)
 * Optimization SSA (39ms)
 * Optimization CCP (13ms)
 * Optimization GVNGCM (45ms)
 *   Optimization SimplifyControl (13ms)
 *   Optimization SimplifyControl (2ms)
 *   Optimization SimplifyControl (2ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 * Optimization DCE (5ms)
 * Optimization Raise[-liftnone] (177ms)
 * Optimization PolyhedralMapperNew[threadf:reduce_create, no-simplify-loop, no-spatial-layout] (1481ms)
 * Optimization Lower (107ms)
 *   Optimization SSA (1ms)
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
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization DCE (2ms)
 * Optimization SyntaxPrepass (7ms)
 * Optimization OSR (12ms)
 * Optimization DCE (6ms)
 * Optimization DeSSA (39ms)
 */
#define SOCR true
#define SOCR true
#include <rstream_ocr.h>
#include <ocr.h>
typedef float real_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_21;
union __args_mv1_22;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_23;
union __args_mv2_24;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_25;
union __args_mv_main_26;
union ocrGuidUnion_t;
struct va_list_t;
struct __anon_21 {
   real_t* C;
   real_t (* A)[2000];
   real_t* E;
   int IT0;
   int IT1;
};
union __args_mv1_22 {
   struct __anon_21 data;
   double padding[4];
};
struct __anon_23 {
   real_t* B;
   real_t (* A)[2000];
   real_t* D;
   int IT0;
   int IT1;
};
union __args_mv2_24 {
   struct __anon_23 data;
   double padding[4];
};
struct __anon_25 {
   real_t* C;
   real_t (* A)[2000];
   real_t* E;
   real_t* B;
   real_t* D;
};
union __args_mv_main_26 {
   struct __anon_25 data;
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
void mv(real_t (* A)[2000], real_t* B, real_t* C, real_t* D, real_t* E);
static ocrGuid_t mv1(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
static ocrGuid_t mv2(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv);
ocrGuid_t rocrAlloc(void**, unsigned long);
static ocrGuid_t mv_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv);
void rocrExit(void);
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
 * mvt.c:52.6
 */
void initialize(void)
{
   double _t1;
   int i;
   int i_1;
   int i_2;
   for (i = 0, i_1 = 0, i_2 = 0; _t1 = (double)i * 0.5 + 2.3, i < 2000; i++,
           i_1 += 0, i_2++) {
      real_t _t2;
      real_t* _t3;
      real_t* _t4;
      int j;
      int j_1;
      _t2 = (float)_t1;
      Tx2[i] = _t2;
      Tx1[i] = _t2;
      x2[i] = _t2;
      x1[i] = _t2;
      Ty_2[i] = _t2;
      Ty_1[i] = _t2;
      y_2[i] = _t2;
      _t3 = a[i];
      _t4 = Ta[i];
      y_1[i] = _t2;
      for (j = 0, j_1 = i_1; j < 2000; j++, j_1 += i_2) {
         float _t5;
         _t5 = (float)j_1 * 0.5f + 2.299999952f;
         _t4[j] = _t5;
         _t3[j] = _t5;
      }
   }
}

/*
 * mvt.c:65.6
 */
void initialize_once(void)
{
   initialize();
}

/*
 * mvt.c:69.6
 */
void show(void)
{
   void const* _t1;
   _t1 = a;
   print_submatrix("a", _t1, 2000, 2000, 0, 8, 0, 8);
   print_submatrix("a", _t1, 2000, 2000, 1992, 2000, 1992, 2000);
}

/*
 * mvt.c:78.6
 */
void kernel(void)
{
   union __args_mv_main_26* allArgs;
   ocrGuid_t _t1;
   union __args_mv_main_26* _t2;
   rocrDeclareType(mv_main, 2, (unsigned int)1, 1, (unsigned long (*)(va_list))
      ((void*)0l));
   rocrDeclareType(mv1, 0, (unsigned int)16, 0, (unsigned long (*)(va_list))((
      void*)0l));
   rocrDeclareType(mv2, 1, (unsigned int)16, 0, (unsigned long (*)(va_list))((
      void*)0l));
   rocrInit();
   _t1 = rocrAlloc((void**)&allArgs, 40ul);
   _t2 = allArgs;
   _t2->data.C = x2;
   _t2->data.A = a;
   _t2->data.E = y_2;
   _t2->data.B = x1;
   _t2->data.D = y_1;
   rocrExecute(2, _t1);
   rocrExit();
}

/*
 * mvt.c:82.5
 */
int check(void)
{
   int i;
   int i_1;
   for (i = 0; i < 2000; i++) {
      real_t* _t1;
      real_t* _t2;
      int j;
      for (_t2 = Ta[i], _t1 = Tx1 + i, j = 0; j < 2000; j++) {
         *_t1 = *_t1 + _t2[j] * Ty_1[j];
      }
   }
   for (i_1 = 0; i_1 < 2000; i_1++) {
      real_t* _t3;
      int j;
      for (_t3 = Tx2 + i_1, j = 0; j < 2000; j++) {
         *_t3 = *_t3 + Ta[j][i_1] * Ty_2[j];
      }
   }
   return check_matrices("a", "Ta", a, Ta, 2000, 2000);
}

/*
 * mvt.c:89.8
 */
double flops_per_trial(void)
{
   return (double)(mul_flop + add_flop) * 2.0 * 2000.0 * 2000.0;
}

void mv(real_t (* A)[2000], real_t* B, real_t* C, real_t* D, real_t* E)
{
   union __args_mv_main_26* allArgs;
   ocrGuid_t _t1;
   union __args_mv_main_26* _t2;
   rocrDeclareType(mv_main, 2, (unsigned int)1, 1, (unsigned long (*)(va_list))
      ((void*)0l));
   rocrDeclareType(mv1, 0, (unsigned int)16, 0, (unsigned long (*)(va_list))((
      void*)0l));
   rocrDeclareType(mv2, 1, (unsigned int)16, 0, (unsigned long (*)(va_list))((
      void*)0l));
   rocrInit();
   _t1 = rocrAlloc((void**)&allArgs, 40ul);
   _t2 = allArgs;
   _t2->data.C = C;
   _t2->data.A = A;
   _t2->data.E = E;
   _t2->data.B = B;
   _t2->data.D = D;
   rocrExecute(2, _t1);
   rocrExit();
}

static ocrGuid_t mv1(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv)
{
   union __args_mv1_22 mv1_args;
   real_t* C;
   real_t (* A)[2000];
   real_t* E;
   int IT0;
   int _t1;
   int i;
   rocrParseInputArgs(paramc, paramv, depc, depv);
   for (mv1_args = *(union __args_mv1_22*)rocrArgs(depv),
        IT0 = mv1_args.data.IT0,
        _t1 = (__mins_32(-128 * IT0 + 1999, 127)),
        E = mv1_args.data.E,
        A = mv1_args.data.A,
        C = mv1_args.data.C,
        i = 0;
        i <= _t1; i++) {
      int j;
      for (j = 0; j <= 1999; j++) {
         C[i + 128 * IT0] = C[i + 128 * IT0] + A[j][i + 128 * IT0] * E[j];
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t mv2(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv)
{
   union __args_mv2_24 mv2_args;
   real_t* B;
   real_t (* A)[2000];
   real_t* D;
   int IT0;
   int _t1;
   int i;
   rocrParseInputArgs(paramc, paramv, depc, depv);
   for (mv2_args = *(union __args_mv2_24*)rocrArgs(depv),
        IT0 = mv2_args.data.IT0,
        _t1 = (__mins_32(-128 * IT0 + 1999, 127)),
        D = mv2_args.data.D,
        A = mv2_args.data.A,
        B = mv2_args.data.B,
        i = 0;
        i <= _t1; i++) {
      int j;
      for (j = 0; j <= 1999; j++) {
         B[i + 128 * IT0] = B[i + 128 * IT0] + A[i + 128 * IT0][j] * D[j];
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t mv_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mv_main_26 mv_main_args;
   union __args_mv1_22* allArgs;
   union __args_mv2_24* allArgs_1;
   real_t (* A)[2000];
   real_t* B;
   real_t* D;
   real_t* C;
   real_t* E;
   int i;
   int i_1;
   rocrParseInputArgs(paramc, paramv, depc, depv);
   for (mv_main_args = *(union __args_mv_main_26*)rocrArgs(depv),
        D = mv_main_args.data.D,
        B = mv_main_args.data.B,
        E = mv_main_args.data.E,
        A = mv_main_args.data.A,
        C = mv_main_args.data.C,
        i = 0;
        i <= 15; i++) {
      ocrGuid_t _t1;
      union __args_mv1_22* _t2;
      _t1 = rocrAlloc((void**)&allArgs, 32ul);
      _t2 = allArgs;
      _t2->data.C = C;
      _t2->data.A = A;
      _t2->data.E = E;
      _t2->data.IT0 = i;
      _t2->data.IT1 = 0;
      rocrCreateTask(0, _t1, (unsigned int)i, i, 0);
   }
   for (i_1 = 0; i_1 <= 15; i_1++) {
      ocrGuid_t _t3;
      union __args_mv2_24* _t4;
      _t3 = rocrAlloc((void**)&allArgs_1, 32ul);
      _t4 = allArgs_1;
      _t4->data.B = B;
      _t4->data.A = A;
      _t4->data.D = D;
      _t4->data.IT0 = i_1;
      _t4->data.IT1 = 0;
      rocrCreateTask(1, _t3, (unsigned int)i_1, i_1, 0);
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}


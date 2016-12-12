/*
 * Optimization PARSE[-I//usr/include, -I/home/pradelle/work/pca-ocr-db-autodec/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -I../include, -I/home/pradelle/work/pca-ocr-db-autodec/runtime/codelet/ocr/inc, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DOCR_TARGET, -D__RSTREAM_CC__, ../src/adi.c] (395ms)
 * Optimization SSA (47ms)
 * Optimization CCP (24ms)
 * Optimization DCE (12ms)
 * Optimization Raise[-liftnone, -stacktrace] (230ms)
 * Optimization PolyhedralMapperNew[-stacktrace, no-simplify-loop, no-spatial-layout, stacktrace=true, scal-dep, threadf=reduce_create=true, as=scopes, logfile=adi-mapper.log] (6341ms)
 * Optimization Lower[-stacktrace] (2232ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (8ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
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
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization DCE (11ms)
 * Optimization SyntaxPrepass (17ms)
 * Optimization OSR (48ms)
 * Optimization DCE (14ms)
 * Optimization DeSSA (152ms)
 */
#include <rstream_ocr.h>
#include <ocr.h>
#define ENABLE_EXTENSION_LEGACY
#include <ocr-legacy.h>
typedef float real_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_21;
union __args_ad4_22;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_23;
union __args_ad_main_24;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct __anon_21 {
   real_t (* R)[500];
   real_t (* P)[500];
   real_t (* Q)[500];
   int IT0;
   int IT1;
   int IT2;
};
union __args_ad4_22 {
   struct __anon_21 data;
   double padding[5];
};
struct __anon_23 {
   real_t (* R)[500];
   real_t (* P)[500];
   real_t (* Q)[500];
};
union __args_ad_main_24 {
   struct __anon_23 data;
   double padding[3];
};
/*
 * Forward declarations of functions
 */
void print_submatrix(char const*, void const*, int, int, int, int, int, int);
int check_vectors(char const*, char const*, void const*, void const*, int);
void initialize(void);
void initialize_once(void);
void show(void);
void kernel(void);
int check(void);
double flops_per_trial(void);
static void ad(real_t (* P)[500], real_t (* Q)[500], real_t (* R)[500]);
static void ad_1(real_t (* P)[500], real_t (* Q)[500], real_t (* R)[500]);
void ad_2(real_t (* P)[500], real_t (* Q)[500], real_t (* R)[500]);
static ocrGuid_t ad4(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
unsigned long ad4_count_0(long* args);
ocrGuid_t rocrAlloc(void**, unsigned long);
static ocrGuid_t ad_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv);
void ad_async(void* event, real_t (* P)[500], real_t (* Q)[500], real_t (* R)[
   500]);
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
static real_t X[500][500];
static real_t A[500][500];
static real_t B[500][500];
static real_t TX[500][500];
static real_t TA[500][500];
static real_t TB[500][500];
int nb_samples = 250000;
char const* function_name = "adi";
/*
 * Definitions of functions
 */
/*
 * ../src/adi.c:71.6
 */
void initialize(void)
{
   int i;
   int i_1;
   int i_2;
   int i_3;
   for (i = 0,
        i_1 = 0,
        i_2 = 0,
        i_3 = 0;
        i < 500;
        i++,
        i_1 += 0,
        i_2++,
        i_3++) {
      int j;
      int j_1;
      int j_2;
      for (j = 0, j_1 = i_1, j_2 = i_3; j < 500; j++, j_2++, j_1 += i_2) {
         real_t _t1;
         real_t _t2;
         real_t _t3;
         _t1 = (float)((double)j_2 + 100.23);
         TX[i][j] = _t1;
         X[i][j] = _t1;
         _t2 = (float)((double)j_1 * 3.23);
         TA[i][j] = _t2;
         A[i][j] = _t2;
         _t3 = (float)((double)(j_1 * j_2) + 55.5);
         TB[i][j] = _t3;
         B[i][j] = _t3;
      }
   }
}

/*
 * ../src/adi.c:83.6
 */
void initialize_once(void)
{
   initialize();
}

/*
 * ../src/adi.c:87.6
 */
void show(void)
{
   print_submatrix("A", A, 500, 500, 0, 8, 0, 8);
   print_submatrix("A", A, 500, 500, 492, 500, 492, 500);
}

/*
 * ../src/adi.c:97.6
 */
void kernel(void)
{
   ocrGuid_t outEvt;
   union __args_ad_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_ad_main_24* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, 0);
   _t3 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, ad_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t3, ad4, 0u, (unsigned long)512, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 24ul);
   _t2 = allArgs;
   _t2->data.R = X;
   _t2->data.P = A;
   _t2->data.Q = B;
   rocrExecute(_t3, ad_main, _t1, &outEvt);
   rocrExit(_t3);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

/*
 * ../src/adi.c:101.5
 */
int check(void)
{
   int t;
   for (t = 0; t < 500; t++) {
      int i1;
      int i1_1;
      int i1_2;
      for (i1 = 0; i1 < 500; i1++) {
         int i2;
         int i2_1;
         for (i2 = 1, i2_1 = 0; i2 < 500; i2++, i2_1++) {
            TX[i1][i2] = TX[i1][i2] - TX[i1][i2_1] * TA[i1][i2] / TB[i1][i2_1];
            TB[i1][i2] = TB[i1][i2] - TA[i1][i2] * TA[i1][i2] / TB[i1][i2_1];
         }
      }
      for (i1_1 = 1, i1_2 = 0; i1_1 < 500; i1_1++, i1_2++) {
         int i2;
         for (i2 = 0; i2 < 500; i2++) {
            TX[i1_1][i2] = TX[i1_1][i2] - TX[i1_2][i2] * TA[i1_1][i2] / TB[i1_2
               ][i2];
            TB[i1_1][i2] = TB[i1_1][i2] - TA[i1_1][i2] * TA[i1_1][i2] / TB[i1_2
               ][i2];
         }
      }
   }
   return check_vectors("A", "TA", A, TA, 500);
}

/*
 * ../src/adi.c:107.8
 */
double flops_per_trial(void)
{
   return (double)(mul_flop + add_flop) * 4.0 * 500.0 * 500.0 * 500.0;
}

/*
 * ../src/adi.c:93.6
 */
static void ad(real_t (* P)[500], real_t (* Q)[500], real_t (* R)[500])
{
   int t;
   for (t = 0; t < 500; t++) {
      int i1;
      int i1_1;
      int i1_2;
      for (i1 = 0; i1 < 500; i1++) {
         int i2;
         int i2_1;
         for (i2 = 1, i2_1 = 0; i2 < 500; i2++, i2_1++) {
            R[i1][i2] = R[i1][i2] - R[i1][i2_1] * P[i1][i2] / Q[i1][i2_1];
            Q[i1][i2] = Q[i1][i2] - P[i1][i2] * P[i1][i2] / Q[i1][i2_1];
         }
      }
      for (i1_1 = 1, i1_2 = 0; i1_1 < 500; i1_1++, i1_2++) {
         int i2;
         for (i2 = 0; i2 < 500; i2++) {
            R[i1_1][i2] = R[i1_1][i2] - R[i1_2][i2] * P[i1_1][i2] / Q[i1_2][i2]
               ;
            Q[i1_1][i2] = Q[i1_1][i2] - P[i1_1][i2] * P[i1_1][i2] / Q[i1_2][i2]
               ;
         }
      }
   }
}

/*
 * ../src/adi.c:93.6
 */
static void ad_1(real_t (* P)[500], real_t (* Q)[500], real_t (* R)[500])
{
   int t;
   for (t = 0; t < 500; t++) {
      int i1;
      int i1_1;
      int i1_2;
      for (i1 = 0; i1 < 500; i1++) {
         int i2;
         int i2_1;
         for (i2 = 1, i2_1 = 0; i2 < 500; i2++, i2_1++) {
            R[i1][i2] = R[i1][i2] - R[i1][i2_1] * P[i1][i2] / Q[i1][i2_1];
            Q[i1][i2] = Q[i1][i2] - P[i1][i2] * P[i1][i2] / Q[i1][i2_1];
         }
      }
      for (i1_1 = 1, i1_2 = 0; i1_1 < 500; i1_1++, i1_2++) {
         int i2;
         for (i2 = 0; i2 < 500; i2++) {
            R[i1_1][i2] = R[i1_1][i2] - R[i1_2][i2] * P[i1_1][i2] / Q[i1_2][i2]
               ;
            Q[i1_1][i2] = Q[i1_1][i2] - P[i1_1][i2] * P[i1_1][i2] / Q[i1_2][i2]
               ;
         }
      }
   }
}

void ad_2(real_t (* P)[500], real_t (* Q)[500], real_t (* R)[500])
{
   ocrGuid_t outEvt;
   union __args_ad_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_ad_main_24* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, 0);
   _t3 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, ad_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t3, ad4, 0u, (unsigned long)512, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 24ul);
   _t2 = allArgs;
   _t2->data.R = R;
   _t2->data.P = P;
   _t2->data.Q = Q;
   rocrExecute(_t3, ad_main, _t1, &outEvt);
   rocrExit(_t3);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

static ocrGuid_t ad4(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv)
{
   union __args_ad4_22 ad4_args;
   union __args_ad4_22* allArgs;
   void* _t1;
   real_t (* R)[500];
   real_t (* P)[500];
   real_t (* Q)[500];
   int IT0;
   int IT1;
   int IT2;
   int _t2;
   int i;
   int i_1;
   int _t3;
   int i_2;
   int i_3;
   int i_4;
   int i_5;
   int i_6;
   int i_7;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   ad4_args = *(union __args_ad4_22*)rocrArgs(depv);
   IT2 = ad4_args.data.IT2;
   Q = ad4_args.data.Q;
   P = ad4_args.data.P;
   R = ad4_args.data.R;
   IT1 = ad4_args.data.IT1;
   IT0 = ad4_args.data.IT0;
   if (IT0 + -2 * IT1 == -16) {
      int _t4;
      int i_8;
      for (_t4 = (__mins_32(499, 64 * IT1 + -500 * IT2)), i_8 = (__maxs_32(64 *
              IT1 + -500 * IT2 + -498, 0)); i_8 <= _t4; i_8++) {
         R[i_8 + -64 * IT1 + (500 * IT2 + 499)][499] = R[i_8 + -64 * IT1 + (500
            * IT2 + 499)][499] - R[i_8 + -64 * IT1 + (500 * IT2 + 498)][499] * P
            [i_8 + -64 * IT1 + (500 * IT2 + 499)][499] / Q[i_8 + -64 * IT1 + (
            500 * IT2 + 498)][499];
         Q[i_8 + -64 * IT1 + (500 * IT2 + 499)][499] = Q[i_8 + -64 * IT1 + (500
            * IT2 + 499)][499] - P[i_8 + -64 * IT1 + (500 * IT2 + 499)][499] * P
            [i_8 + -64 * IT1 + (500 * IT2 + 499)][499] / Q[i_8 + -64 * IT1 + (
            500 * IT2 + 498)][499];
      }
   }
   for (_t2 = (__mins_32(-32 * IT0 + 499, __mins_32(-32 * IT0 + 500 * IT2 + 498
           , 31))), i = (__maxs_32(0, __maxs_32(-32 * IT0 + 64 * IT1 + -498,
           -32 * IT0 + 500 * IT2 + -499))), i_1 = i * -1; i <= _t2; i++, i_1--)
       {
      int _t5;
      int j;
      int j_1;
      int j_2;
      int j_3;
      int j_4;
      int j_5;
      int j_6;
      int j_7;
      int j_8;
      int j_9;
      int j_10;
      int j_11;
      int j_12;
      int j_13;
      int j_14;
      int j_15;
      int j_16;
      int j_17;
      int j_18;
      int j_19;
      int j_20;
      int j_21;
      int j_22;
      int j_23;
      int j_24;
      int j_25;
      int j_26;
      int j_27;
      int j_28;
      int j_29;
      int j_30;
      for (_t5 = (__mins_32(63, i + 32 * IT0 + -64 * IT1 + 498)),
           j = (__maxs_32(i + 32 * IT0 + -64 * IT1, 0)),
           j_1 = j + i_1,
           j_2 = j + i_1,
           j_3 = j + i_1,
           j_4 = j + i_1,
           j_5 = j + i_1,
           j_6 = j + i_1,
           j_7 = j + i_1,
           j_8 = j + i_1,
           j_9 = j + i_1,
           j_10 = j + i_1,
           j_11 = j + i_1,
           j_12 = j + i_1,
           j_13 = j + i_1,
           j_14 = j + i_1,
           j_15 = j + i_1,
           j_16 = j + i_1,
           j_17 = j + i_1,
           j_18 = j + i_1,
           j_19 = j + i_1,
           j_20 = j + i_1,
           j_21 = j + i_1,
           j_22 = j + i_1,
           j_23 = j + i_1,
           j_24 = j + i_1,
           j_25 = j + i_1,
           j_26 = j + i_1,
           j_27 = j + i_1,
           j_28 = j + i_1,
           j_29 = j + i_1,
           j_30 = j + i_1;
           j <= _t5;
           j++,
           j_1++,
           j_2++,
           j_3++,
           j_4++,
           j_5++,
           j_6++,
           j_7++,
           j_8++,
           j_9++,
           j_10++,
           j_11++,
           j_12++,
           j_13++,
           j_14++,
           j_15++,
           j_16++,
           j_17++,
           j_18++,
           j_19++,
           j_20++,
           j_21++,
           j_22++,
           j_23++,
           j_24++,
           j_25++,
           j_26++,
           j_27++,
           j_28++,
           j_29++,
           j_30++) {
         int _t6;
         int k;
         int k_1;
         int k_2;
         int k_3;
         int k_4;
         int k_5;
         int k_6;
         int k_7;
         int k_8;
         int k_9;
         int k_10;
         int k_11;
         int k_12;
         int k_13;
         int k_14;
         int k_15;
         int k_16;
         int k_17;
         int k_18;
         int k_19;
         int k_20;
         if (i + 32 * IT0 + -500 * IT2 >= 0) {
            R[0][j_1 + (-32 * IT0 + (64 * IT1 + 1))] = R[0][j_2 + (-32 * IT0 + (
               64 * IT1 + 1))] - R[0][j_3 + (-32 * IT0 + 64 * IT1)] * P[0][j_4 +
               (-32 * IT0 + (64 * IT1 + 1))] / Q[0][j_5 + (-32 * IT0 + 64 * IT1
               )];
            Q[0][j_6 + (-32 * IT0 + (64 * IT1 + 1))] = Q[0][j_7 + (-32 * IT0 + (
               64 * IT1 + 1))] - P[0][j_8 + (-32 * IT0 + (64 * IT1 + 1))] * P[0
               ][j_9 + (-32 * IT0 + (64 * IT1 + 1))] / Q[0][j_10 + (-32 * IT0 +
               64 * IT1)];
         }
         for (_t6 = (__mins_32(499, i + 32 * IT0 + -500 * IT2 + 499)),
              k = (__maxs_32(i + 32 * IT0 + -500 * IT2 + 1, 0)),
              k_1 = k + i_1,
              k_2 = k + i_1,
              k_3 = k + i_1,
              k_4 = k + i_1,
              k_5 = k + i_1,
              k_6 = k + i_1,
              k_7 = k + i_1,
              k_8 = k + i_1,
              k_9 = k + i_1,
              k_10 = k + i_1,
              k_11 = k + i_1,
              k_12 = k + i_1,
              k_13 = k + i_1,
              k_14 = k + i_1,
              k_15 = k + i_1,
              k_16 = k + i_1,
              k_17 = k + i_1,
              k_18 = k + i_1,
              k_19 = k + i_1,
              k_20 = k + i_1;
              k <= _t6;
              k_20++,
              k_19++,
              k_18++,
              k_17++,
              k_16++,
              k_15++,
              k_14++,
              k_13++,
              k_12++,
              k_11++,
              k_10++,
              k_9++,
              k_8++,
              k_7++,
              k_6++,
              k_5++,
              k_4++,
              k_3++,
              k_2++,
              k_1++,
              k++) {
            R[k_7 + (-32 * IT0 + 500 * IT2)][j_17 + (-32 * IT0 + (64 * IT1 + 1)
               )] = R[k_8 + (-32 * IT0 + 500 * IT2)][j_18 + (-32 * IT0 + (64 *
               IT1 + 1))] - R[k_9 + (-32 * IT0 + 500 * IT2)][j_19 + (-32 * IT0 +
               64 * IT1)] * P[k_10 + (-32 * IT0 + 500 * IT2)][j_20 + (-32 * IT0
               + (64 * IT1 + 1))] / Q[k_11 + (-32 * IT0 + 500 * IT2)][j_21 + (
               -32 * IT0 + 64 * IT1)];
            Q[k_5 + (-32 * IT0 + 500 * IT2)][j_15 + (-32 * IT0 + (64 * IT1 + 1)
               )] = Q[k_6 + (-32 * IT0 + 500 * IT2)][j_16 + (-32 * IT0 + (64 *
               IT1 + 1))] - P[k_12 + (-32 * IT0 + 500 * IT2)][j_22 + (-32 * IT0
               + (64 * IT1 + 1))] * P[k_13 + (-32 * IT0 + 500 * IT2)][j_23 + (
               -32 * IT0 + (64 * IT1 + 1))] / Q[k_14 + (-32 * IT0 + 500 * IT2)]
               [j_24 + (-32 * IT0 + 64 * IT1)];
            R[k_3 + (-32 * IT0 + 500 * IT2)][j_13 + (-32 * IT0 + 64 * IT1)] = R
               [k_4 + (-32 * IT0 + 500 * IT2)][j_14 + (-32 * IT0 + 64 * IT1)] -
               R[k_15 + (-32 * IT0 + (500 * IT2 + -1))][j_25 + (-32 * IT0 + 64 *
               IT1)] * P[k_16 + (-32 * IT0 + 500 * IT2)][j_26 + (-32 * IT0 + 64
               * IT1)] / Q[k_17 + (-32 * IT0 + (500 * IT2 + -1))][j_27 + (-32 *
               IT0 + 64 * IT1)];
            Q[k_1 + (-32 * IT0 + 500 * IT2)][j_11 + (-32 * IT0 + 64 * IT1)] = Q
               [k_2 + (-32 * IT0 + 500 * IT2)][j_12 + (-32 * IT0 + 64 * IT1)] -
               P[k_18 + (-32 * IT0 + 500 * IT2)][j_28 + (-32 * IT0 + 64 * IT1)]
               * P[k_19 + (-32 * IT0 + 500 * IT2)][j_29 + (-32 * IT0 + 64 * IT1
               )] / Q[k_20 + (-32 * IT0 + (500 * IT2 + -1))][j_30 + (-32 * IT0 +
               64 * IT1)];
         }
      }
      if (- i + -32 * IT0 + 64 * IT1 >= 436) {
         int _t7;
         int j_31;
         int j_32;
         int j_33;
         int j_34;
         int j_35;
         int j_36;
         int j_37;
         int j_38;
         int j_39;
         int j_40;
         int j_41;
         for (_t7 = (__mins_32(499, i + 32 * IT0 + -500 * IT2 + 499)),
              j_31 = (__maxs_32(i + 32 * IT0 + -500 * IT2 + 1, 0)),
              j_32 = j_31 + i_1,
              j_33 = j_31 + i_1,
              j_34 = j_31 + i_1,
              j_35 = j_31 + i_1,
              j_36 = j_31 + i_1,
              j_37 = j_31 + i_1,
              j_38 = j_31 + i_1,
              j_39 = j_31 + i_1,
              j_40 = j_31 + i_1,
              j_41 = j_31 + i_1;
              j_31 <= _t7;
              j_41++,
              j_40++,
              j_39++,
              j_38++,
              j_37++,
              j_36++,
              j_35++,
              j_34++,
              j_33++,
              j_32++,
              j_31++) {
            R[j_34 + (-32 * IT0 + 500 * IT2)][499] = R[j_35 + (-32 * IT0 + 500 *
               IT2)][499] - R[j_36 + (-32 * IT0 + (500 * IT2 + -1))][499] * P[
               j_37 + (-32 * IT0 + 500 * IT2)][499] / Q[j_38 + (-32 * IT0 + (
               500 * IT2 + -1))][499];
            Q[j_32 + (-32 * IT0 + 500 * IT2)][499] = Q[j_33 + (-32 * IT0 + 500 *
               IT2)][499] - P[j_39 + (-32 * IT0 + 500 * IT2)][499] * P[j_40 + (
               -32 * IT0 + 500 * IT2)][499] / Q[j_41 + (-32 * IT0 + (500 * IT2 +
               -1))][499];
         }
      }
   }
   if (IT2 == 0 && IT0 == 15) {
      int _t8;
      int i_9;
      for (_t8 = (__mins_32(-64 * IT1 + 997, 63)), i_9 = (__maxs_32(0, -64 * IT1
              + 499)); i_9 <= _t8; i_9++) {
         R[0][i_9 + (64 * IT1 + -498)] = R[0][i_9 + (64 * IT1 + -498)] - R[0][
            i_9 + (64 * IT1 + -499)] * P[0][i_9 + (64 * IT1 + -498)] / Q[0][i_9
            + (64 * IT1 + -499)];
         Q[0][i_9 + (64 * IT1 + -498)] = Q[0][i_9 + (64 * IT1 + -498)] - P[0][
            i_9 + (64 * IT1 + -498)] * P[0][i_9 + (64 * IT1 + -498)] / Q[0][i_9
            + (64 * IT1 + -499)];
      }
   }
   for (_t3 = (__mins_32((2 * IT0 + 2 * IT1 + 3) / 3, __mins_32(15, IT0 + 1))),
        i_2 = (__maxs_32(IT0 + IT1 + IT2 + -15, __maxs_32(IT0, (2 * IT0 + 2 *
           IT1 + 2 * IT2 + -16 + 2) / 3))),
        i_3 = i_2 * -1,
        i_7 = i_2 + 16,
        i_5 = i_2 << 5,
        i_6 = i_5 + 32,
        i_4 = i_2 + 0;
        i_2 <= _t3;
        i_2++,
        i_3--,
        i_7++,
        i_4++,
        i_5 += 32,
        i_6 += 32) {
      int _t9;
      int _t10;
      int j;
      int j_1;
      int j_2;
      int j_3;
      for (_t9 = (__mins_32(- i_2 + IT0 + IT1 + 1, __mins_32(15, i_7 >> 1))),
           j = (__maxs_32(i_4 >> 1, __maxs_32(IT1, - i_2 + IT0 + IT1 + IT2))),
           j_3 = j << 1,
           _t10 = j * -1 + IT0,
           j_1 = _t10 + i_3,
           j_2 = _t10 + i_3;
           j <= _t9;
           j_3 += 2,
           j_2--,
           j_1--,
           j++) {
         ocrGuid_t _t11;
         union __args_ad4_22* _t12;
         int _t13;
         _t11 = rocrAlloc((void**)&allArgs, 40ul);
         _t12 = allArgs;
         _t12->data.R = R;
         _t12->data.P = P;
         _t12->data.Q = Q;
         _t12->data.IT0 = i_2;
         _t12->data.IT1 = j;
         _t13 = j_1 + (IT1 + (IT2 + 1));
         _t12->data.IT2 = _t13;
         rocrAutoDec(_t1, 0u, (unsigned long)(j >= 0 ? j_2 + (IT1 + (IT2 + 1)) +
            j_3 + i_5 : i_6), _t11, ad4_count_0, (void (*)(void*, long*))((void
            *)0l), (void (*)(void*, long*))((void*)0l), 3u, (long)i_2, (long)j,
             (long)_t13);
      }
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

unsigned long ad4_count_0(long* args)
{
   unsigned long rcNbInputDeps;
   int _t1;
   int _t2;
   int _t3;
   int _t4;
   int i;
   int i_1;
   int i_2;
   for (_t3 = (int)args[2],
        _t2 = (int)args[1],
        _t1 = (int)args[0],
        _t4 = (__mins_32((2 * _t1 + 2 * _t2 + 2 * _t3 + -1 + 6) / 3 + -2,
           __mins_32(_t1, _t1 + _t2 + _t3 + -1))),
        i = (__maxs_32(_t1 + -1, __maxs_32(0, (2 * _t1 + 2 * _t2 + -18 + 2) / 3
           ))),
        i_2 = i + 16,
        i_1 = i + 0,
        rcNbInputDeps = 0ul;
        i <= _t4; i++, i_2++, i_1++) {
      int _t5;
      int j;
      for (_t5 = (__mins_32(- i + _t1 + _t2 + _t3 + -1, __mins_32(_t2, i_2 >> 1
              ))), j = (__maxs_32(i_1 >> 1, __maxs_32(0, - i + _t1 + _t2 + -1))
              ); j <= _t5; j++, rcNbInputDeps++) {
      }
   }
   return rcNbInputDeps;
}

static ocrGuid_t ad_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_ad_main_24 ad_main_args;
   union __args_ad4_22* allArgs;
   void* _t1;
   ocrGuid_t _t2;
   union __args_ad4_22* _t3;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   ad_main_args = *(union __args_ad_main_24*)rocrArgs(depv);
   _t2 = rocrAlloc((void**)&allArgs, 40ul);
   _t3 = allArgs;
   _t3->data.R = ad_main_args.data.R;
   _t3->data.P = ad_main_args.data.P;
   _t3->data.Q = ad_main_args.data.Q;
   _t3->data.IT0 = 0;
   _t3->data.IT1 = 0;
   _t3->data.IT2 = 0;
   rocrAutoDec(_t1, 0u, (unsigned long)0, _t2, ad4_count_0, (void (*)(void*,
      long*))((void*)0l), (void (*)(void*, long*))((void*)0l), 3u, (long)0, (
      long)0, (long)0);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void ad_async(void* event, real_t (* P)[500], real_t (* Q)[500], real_t (* R)[
   500])
{
   union __args_ad_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_ad_main_24* _t2;
   void* _t3;
   _t3 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, ad_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t3, ad4, 0u, (unsigned long)512, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 24ul);
   _t2 = allArgs;
   _t2->data.R = R;
   _t2->data.P = P;
   _t2->data.Q = Q;
   rocrExecute(_t3, ad_main, _t1, event);
   rocrExit(_t3);
}


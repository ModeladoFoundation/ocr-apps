/*
 * Optimization PARSE[-I//usr/include, -I/opt/reservoir/staging/rstream-3.15.0.1/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DOCR_TARGET=1, --c99, -D__RSTREAM_CC__, adi.c] (377ms)
 * Optimization SSA (45ms)
 * Optimization CCP (20ms)
 * Optimization GVNGCM (48ms)
 *   Optimization SimplifyControl (14ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 * Optimization DCE (6ms)
 * Optimization Raise[-liftnone] (266ms)
 * Optimization PolyhedralMapperNew[threadf:reduce_create, no-simplify-loop, no-spatial-layout] (2195ms)
 * Optimization Lower (573ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (6ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization DCE (6ms)
 * Optimization SyntaxPrepass (11ms)
 * Optimization OSR (32ms)
 * Optimization DCE (11ms)
 * Optimization DeSSA (100ms)
 */
#define SOCR true
#include <rstream_ocr.h>
#include <ocr.h>
typedef float real_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_21;
union __args_ad1_22;
struct va_list_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_23;
union __args_ad_main_24;
union ocrGuidUnion_t;
struct va_list_t;
struct __anon_21 {
   real_t (* R)[500];
   real_t (* P)[500];
   real_t (* Q)[500];
   int IT0;
   int IT1;
   int IT2;
};
union __args_ad1_22 {
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
void ad(real_t (* P)[500], real_t (* Q)[500], real_t (* R)[500]);
static ocrGuid_t ad1(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
ocrGuid_t rocrAlloc(void**, unsigned long);
unsigned long ad1_count_0(va_list args);
static ocrGuid_t ad_main(unsigned int paramc, unsigned long* paramv,
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
 * adi.c:71.6
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
      real_t* _t1;
      real_t* _t2;
      real_t* _t3;
      real_t* _t4;
      real_t* _t5;
      real_t* _t6;
      int j;
      int j_1;
      int j_2;
      for (_t5 = B[i],
           _t6 = TB[i],
           _t4 = TA[i],
           _t3 = A[i],
           _t2 = TX[i],
           _t1 = X[i],
           j = 0,
           j_1 = i_1,
           j_2 = i_3;
           j < 500; j_2++, j++, j_1 += i_2) {
         real_t _t7;
         real_t _t8;
         real_t _t9;
         _t7 = (float)((double)j_2 + 100.23);
         _t2[j] = _t7;
         _t1[j] = _t7;
         _t8 = (float)((double)j_1 * 3.23);
         _t4[j] = _t8;
         _t3[j] = _t8;
         _t9 = (float)((double)(j_1 * j_2) + 55.5);
         _t6[j] = _t9;
         _t5[j] = _t9;
      }
   }
}

/*
 * adi.c:83.6
 */
void initialize_once(void)
{
   initialize();
}

/*
 * adi.c:87.6
 */
void show(void)
{
   void const* _t1;
   _t1 = A;
   print_submatrix("A", _t1, 500, 500, 0, 8, 0, 8);
   print_submatrix("A", _t1, 500, 500, 492, 500, 492, 500);
}

/*
 * adi.c:97.6
 */
void kernel(void)
{
   union __args_ad_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_ad_main_24* _t2;
   rocrDeclareType(ad_main, 1, (unsigned int)1, 1, (unsigned long (*)(va_list))
      ((void*)0l));
   rocrDeclareType(ad1, 0, (unsigned int)2048, 0, ad1_count_0);
   rocrInit();
   _t1 = rocrAlloc((void**)&allArgs, 24ul);
   _t2 = allArgs;
   _t2->data.R = X;
   _t2->data.P = A;
   _t2->data.Q = B;
   rocrExecute(1, _t1);
   rocrExit();
}

/*
 * adi.c:101.5
 */
int check(void)
{
   int t;
   for (t = 0; t < 500; t++) {
      int i1;
      int i1_1;
      int i1_2;
      for (i1 = 0; i1 < 500; i1++) {
         real_t* _t1;
         real_t* _t2;
         real_t* _t3;
         int i2;
         int i2_1;
         for (_t3 = TB[i1],
              _t2 = TA[i1],
              _t1 = TX[i1],
              i2 = 1,
              i2_1 = 0;
              i2 < 500; i2++, i2_1++) {
            real_t* _t4;
            real_t* _t5;
            float _t6;
            _t4 = _t2 + i2;
            _t5 = _t3 + i2_1;
            _t1[i2] = _t1[i2] - _t1[i2_1] * *_t4 / *_t5;
            _t6 = *_t4;
            _t3[i2] = _t3[i2] - _t6 * _t6 / *_t5;
         }
      }
      for (i1_1 = 1, i1_2 = 0; i1_1 < 500; i1_1++, i1_2++) {
         real_t* _t7;
         real_t* _t8;
         real_t* _t9;
         real_t* _t10;
         real_t* _t11;
         int i2;
         for (_t11 = TB[i1_1],
              _t9 = TA[i1_1],
              _t10 = TB[i1_2],
              _t8 = TX[i1_2],
              _t7 = TX[i1_1],
              i2 = 0;
              i2 < 500; i2++) {
            real_t* _t12;
            real_t* _t13;
            float _t14;
            _t12 = _t9 + i2;
            _t13 = _t10 + i2;
            _t7[i2] = _t7[i2] - _t8[i2] * *_t12 / *_t13;
            _t14 = *_t12;
            _t11[i2] = _t11[i2] - _t14 * _t14 / *_t13;
         }
      }
   }
   return check_vectors("A", "TA", A, TA, 500);
}

/*
 * adi.c:107.8
 */
double flops_per_trial(void)
{
   return (double)(mul_flop + add_flop) * 4.0 * 500.0 * 500.0 * 500.0;
}

void ad(real_t (* P)[500], real_t (* Q)[500], real_t (* R)[500])
{
   union __args_ad_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_ad_main_24* _t2;
   rocrDeclareType(ad_main, 1, (unsigned int)1, 1, (unsigned long (*)(va_list))
      ((void*)0l));
   rocrDeclareType(ad1, 0, (unsigned int)2048, 0, ad1_count_0);
   rocrInit();
   _t1 = rocrAlloc((void**)&allArgs, 24ul);
   _t2 = allArgs;
   _t2->data.R = R;
   _t2->data.P = P;
   _t2->data.Q = Q;
   rocrExecute(1, _t1);
   rocrExit();
}

static ocrGuid_t ad1(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv)
{
   union __args_ad1_22 ad1_args;
   union __args_ad1_22* allArgs;
   real_t (* R)[500];
   real_t (* P)[500];
   real_t (* Q)[500];
   int IT0;
   int IT1;
   int IT2;
   int _t1;
   int i;
   int i_1;
   int _t2;
   int i_2;
   int i_3;
   int i_4;
   int i_5;
   int i_6;
   int i_7;
   rocrParseInputArgs(paramc, paramv, depc, depv);
   ad1_args = *(union __args_ad1_22*)rocrArgs(depv);
   IT2 = ad1_args.data.IT2;
   Q = ad1_args.data.Q;
   P = ad1_args.data.P;
   R = ad1_args.data.R;
   IT1 = ad1_args.data.IT1;
   IT0 = ad1_args.data.IT0;
   if (IT0 + -2 * IT1 == -32) {
      int _t3;
      int i_8;
      for (_t3 = (__mins_32(499, 32 * IT1 + -500 * IT2)), i_8 = (__maxs_32(32 *
              IT1 + -500 * IT2 + -498, 0)); i_8 <= _t3; i_8++) {
         float _t4;
         R[i_8 + -32 * IT1 + (500 * IT2 + 499)][499] = R[i_8 + -32 * IT1 + (500
            * IT2 + 499)][499] - R[i_8 + -32 * IT1 + (500 * IT2 + 498)][499] * P
            [i_8 + -32 * IT1 + (500 * IT2 + 499)][499] / Q[i_8 + -32 * IT1 + (
            500 * IT2 + 498)][499];
         _t4 = P[i_8 + -32 * IT1 + (500 * IT2 + 499)][499];
         Q[i_8 + -32 * IT1 + (500 * IT2 + 499)][499] = Q[i_8 + -32 * IT1 + (500
            * IT2 + 499)][499] - _t4 * _t4 / Q[i_8 + -32 * IT1 + (500 * IT2 +
            498)][499];
      }
   }
   for (_t1 = (__mins_32(-16 * IT0 + 499, __mins_32(-16 * IT0 + 500 * IT2 + 498
           , 15))), i = (__maxs_32(0, __maxs_32(-16 * IT0 + 32 * IT1 + -498,
           -16 * IT0 + 500 * IT2 + -499))), i_1 = i * -1; i <= _t1; i++, i_1--)
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
      for (_t5 = (__mins_32(31, i + 16 * IT0 + -32 * IT1 + 498)),
           j = (__maxs_32(i + 16 * IT0 + -32 * IT1, 0)),
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
           j_21 = j + i_1;
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
           j_21++) {
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
         if (i + 16 * IT0 + -500 * IT2 >= 0) {
            float _t7;
            R[0][j_1 + (-16 * IT0 + (32 * IT1 + 1))] = R[0][j_1 + (-16 * IT0 + (
               32 * IT1 + 1))] - R[0][j_2 + (-16 * IT0 + 32 * IT1)] * P[0][j_3 +
               (-16 * IT0 + (32 * IT1 + 1))] / Q[0][j_4 + (-16 * IT0 + 32 * IT1
               )];
            _t7 = P[0][j_5 + (-16 * IT0 + (32 * IT1 + 1))];
            Q[0][j_7 + (-16 * IT0 + (32 * IT1 + 1))] = Q[0][j_7 + (-16 * IT0 + (
               32 * IT1 + 1))] - _t7 * _t7 / Q[0][j_6 + (-16 * IT0 + 32 * IT1)]
               ;
         }
         for (_t6 = (__mins_32(499, i + 16 * IT0 + -500 * IT2 + 499)),
              k = (__maxs_32(i + 16 * IT0 + -500 * IT2 + 1, 0)),
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
              k_14 = k + i_1;
              k <= _t6;
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
            float _t8;
            float _t9;
            R[k_4 + (-16 * IT0 + 500 * IT2)][j_11 + (-16 * IT0 + (32 * IT1 + 1)
               )] = R[k_4 + (-16 * IT0 + 500 * IT2)][j_11 + (-16 * IT0 + (32 *
               IT1 + 1))] - R[k_5 + (-16 * IT0 + 500 * IT2)][j_12 + (-16 * IT0 +
               32 * IT1)] * P[k_6 + (-16 * IT0 + 500 * IT2)][j_13 + (-16 * IT0 +
               (32 * IT1 + 1))] / Q[k_7 + (-16 * IT0 + 500 * IT2)][j_14 + (-16 *
               IT0 + 32 * IT1)];
            _t8 = P[k_8 + (-16 * IT0 + 500 * IT2)][j_15 + (-16 * IT0 + (32 * IT1
               + 1))];
            Q[k_3 + (-16 * IT0 + 500 * IT2)][j_10 + (-16 * IT0 + (32 * IT1 + 1)
               )] = Q[k_3 + (-16 * IT0 + 500 * IT2)][j_10 + (-16 * IT0 + (32 *
               IT1 + 1))] - _t8 * _t8 / Q[k_9 + (-16 * IT0 + 500 * IT2)][j_16 +
               (-16 * IT0 + 32 * IT1)];
            R[k_2 + (-16 * IT0 + 500 * IT2)][j_9 + (-16 * IT0 + 32 * IT1)] = R[
               k_2 + (-16 * IT0 + 500 * IT2)][j_9 + (-16 * IT0 + 32 * IT1)] - R
               [k_10 + (-16 * IT0 + (500 * IT2 + -1))][j_17 + (-16 * IT0 + 32 *
               IT1)] * P[k_11 + (-16 * IT0 + 500 * IT2)][j_18 + (-16 * IT0 + 32
               * IT1)] / Q[k_12 + (-16 * IT0 + (500 * IT2 + -1))][j_19 + (-16 *
               IT0 + 32 * IT1)];
            _t9 = P[k_13 + (-16 * IT0 + 500 * IT2)][j_20 + (-16 * IT0 + 32 * IT1
               )];
            Q[k_1 + (-16 * IT0 + 500 * IT2)][j_8 + (-16 * IT0 + 32 * IT1)] = Q[
               k_1 + (-16 * IT0 + 500 * IT2)][j_8 + (-16 * IT0 + 32 * IT1)] -
               _t9 * _t9 / Q[k_14 + (-16 * IT0 + (500 * IT2 + -1))][j_21 + (-16
               * IT0 + 32 * IT1)];
         }
      }
      if (- i + -16 * IT0 + 32 * IT1 >= 468) {
         int _t10;
         int j_22;
         int j_23;
         int j_24;
         int j_25;
         int j_26;
         int j_27;
         int j_28;
         int j_29;
         for (_t10 = (__mins_32(499, i + 16 * IT0 + -500 * IT2 + 499)),
              j_22 = (__maxs_32(i + 16 * IT0 + -500 * IT2 + 1, 0)),
              j_23 = j_22 + i_1,
              j_24 = j_22 + i_1,
              j_25 = j_22 + i_1,
              j_26 = j_22 + i_1,
              j_27 = j_22 + i_1,
              j_28 = j_22 + i_1,
              j_29 = j_22 + i_1;
              j_22 <= _t10;
              j_29++,
              j_28++,
              j_27++,
              j_26++,
              j_25++,
              j_24++,
              j_23++,
              j_22++) {
            float _t11;
            R[j_24 + (-16 * IT0 + 500 * IT2)][499] = R[j_24 + (-16 * IT0 + 500 *
               IT2)][499] - R[j_25 + (-16 * IT0 + (500 * IT2 + -1))][499] * P[
               j_26 + (-16 * IT0 + 500 * IT2)][499] / Q[j_27 + (-16 * IT0 + (
               500 * IT2 + -1))][499];
            _t11 = P[j_28 + (-16 * IT0 + 500 * IT2)][499];
            Q[j_23 + (-16 * IT0 + 500 * IT2)][499] = Q[j_23 + (-16 * IT0 + 500 *
               IT2)][499] - _t11 * _t11 / Q[j_29 + (-16 * IT0 + (500 * IT2 + -1
               ))][499];
         }
      }
   }
   if (IT2 == 0 && IT0 == 31) {
      int _t12;
      int i_9;
      for (_t12 = (__mins_32(-32 * IT1 + 997, 31)), i_9 = (__maxs_32(0, -32 *
              IT1 + 499)); i_9 <= _t12; i_9++) {
         float _t13;
         R[0][i_9 + (32 * IT1 + -498)] = R[0][i_9 + (32 * IT1 + -498)] - R[0][
            i_9 + (32 * IT1 + -499)] * P[0][i_9 + (32 * IT1 + -498)] / Q[0][i_9
            + (32 * IT1 + -499)];
         _t13 = P[0][i_9 + (32 * IT1 + -498)];
         Q[0][i_9 + (32 * IT1 + -498)] = Q[0][i_9 + (32 * IT1 + -498)] - _t13 *
            _t13 / Q[0][i_9 + (32 * IT1 + -499)];
      }
   }
   for (_t2 = (__mins_32((2 * IT0 + 2 * IT1 + 3) / 3, __mins_32(31, IT0 + 1))),
        i_2 = (__maxs_32(IT0 + IT1 + IT2 + -31, __maxs_32(IT0, (2 * IT0 + 2 *
           IT1 + 2 * IT2 + -32 + 2) / 3))),
        i_7 = i_2 + 32,
        i_3 = i_2 << 6,
        i_6 = i_3 + 64,
        i_5 = i_2 * -1,
        i_4 = i_2 + 0;
        i_2 <= _t2;
        i_2++,
        i_3 += 64,
        i_7++,
        i_4++,
        i_5--,
        i_6 += 64) {
      int _t14;
      int _t15;
      int j;
      int j_1;
      int j_2;
      int j_3;
      for (_t14 = (__mins_32(- i_2 + IT0 + IT1 + 1, __mins_32(31, i_7 >> 1))),
           j = (__maxs_32(i_4 >> 1, __maxs_32(IT1, - i_2 + IT0 + IT1 + IT2))),
           _t15 = j * -1 + IT0,
           j_3 = _t15 + i_5,
           j_2 = j << 1,
           j_1 = _t15 + i_5;
           j <= _t14;
           j_3--,
           j_2 += 2,
           j_1--,
           j++) {
         ocrGuid_t _t16;
         union __args_ad1_22* _t17;
         int _t18;
         _t16 = rocrAlloc((void**)&allArgs, 40ul);
         _t17 = allArgs;
         _t17->data.R = R;
         _t17->data.P = P;
         _t17->data.Q = Q;
         _t17->data.IT0 = i_2;
         _t17->data.IT1 = j;
         _t18 = j_3 + (IT1 + (IT2 + 1));
         _t17->data.IT2 = _t18;
         rocrAutoDec(0, _t16, (unsigned int)(j >= 0 ? j_1 + (IT1 + (IT2 + 1)) +
            j_2 + i_3 : i_6), i_2, j, _t18);
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

unsigned long ad1_count_0(va_list args)
{
   unsigned long rcNbInputDeps;
   int IT0;
   int _t1;
   int IT1;
   int _t2;
   int IT2;
   int _t3;
   int i;
   int i_1;
   int i_2;
   for (_t1 = va_arg(args, int),
        _t2 = va_arg(args, int),
        IT2 = va_arg(args, int),
        IT1 = _t2,
        IT0 = _t1,
        _t3 = (__mins_32((2 * IT0 + 2 * IT1 + 2 * IT2 + -1 + 6) / 3 + -2,
           __mins_32(IT0, IT0 + IT1 + IT2 + -1))),
        i = (__maxs_32(IT0 + -1, __maxs_32(0, (2 * IT0 + 2 * IT1 + -34 + 2) / 3
           ))),
        i_2 = i + 32,
        i_1 = i + 0,
        rcNbInputDeps = 0ul;
        i <= _t3; i++, i_2++, i_1++) {
      int _t4;
      int j;
      for (_t4 = (__mins_32(- i + IT0 + IT1 + IT2 + -1, __mins_32(IT1, i_2 >> 1
              ))), j = (__maxs_32(i_1 >> 1, __maxs_32(0, - i + IT0 + IT1 + -1))
              ); j <= _t4; j++, rcNbInputDeps++) {
      }
   }
   return rcNbInputDeps;
}

static ocrGuid_t ad_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_ad_main_24 ad_main_args;
   union __args_ad1_22* allArgs;
   ocrGuid_t _t1;
   union __args_ad1_22* _t2;
   rocrParseInputArgs(paramc, paramv, depc, depv);
   ad_main_args = *(union __args_ad_main_24*)rocrArgs(depv);
   _t1 = rocrAlloc((void**)&allArgs, 40ul);
   _t2 = allArgs;
   _t2->data.R = ad_main_args.data.R;
   _t2->data.P = ad_main_args.data.P;
   _t2->data.Q = ad_main_args.data.Q;
   _t2->data.IT0 = 0;
   _t2->data.IT1 = 0;
   _t2->data.IT2 = 0;
   rocrCreateTask(0, _t1, (unsigned int)0, 0, 0, 0);
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}


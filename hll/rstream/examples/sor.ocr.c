/*
 * Optimization PARSE[-I//usr/include, -I/opt/reservoir/staging/rstream-3.15.0.1/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DOCR_TARGET=1, --c99, -D__RSTREAM_CC__, sor.c] (362ms)
 * Optimization SSA (36ms)
 * Optimization CCP (12ms)
 * Optimization GVNGCM (44ms)
 *   Optimization SimplifyControl (12ms)
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
 * Optimization Raise[-liftnone] (174ms)
 * Optimization PolyhedralMapperNew[threadf:reduce_create, no-simplify-loop, no-spatial-layout] (1196ms)
 * Optimization Lower (124ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization DCE (2ms)
 * Optimization SyntaxPrepass (6ms)
 * Optimization OSR (13ms)
 * Optimization DCE (6ms)
 * Optimization DeSSA (42ms)
 */
#define SOCR true
#include <rstream_ocr.h>
#include <ocr.h>
typedef float real_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_21;
union __args_so1_22;
struct va_list_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_23;
union __args_so_main_24;
union ocrGuidUnion_t;
struct va_list_t;
struct __anon_21 {
   real_t (* A)[10000];
   int IT0;
   int IT1;
};
union __args_so1_22 {
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
void so(real_t (* A)[10000]);
static ocrGuid_t so1(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
ocrGuid_t rocrAlloc(void**, unsigned long);
unsigned long so1_count_0(va_list args);
static ocrGuid_t so_main(unsigned int paramc, unsigned long* paramv,
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
#ifdef GPU_TARGET
    #define __maxs_32(x,y) \
 max((int)(x),(int)(y))
#else
static RSTREAM_INLINE int __maxs_32(int x, int y) {
   return x > y ? x : y;
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
 * sor.c:45.6
 */
void initialize(void)
{
   int i;
   int i_1;
   int i_2;
   for (i = 0, i_1 = 0, i_2 = 0; i < 10000; i++, i_1 += 0, i_2++) {
      real_t* _t1;
      real_t* _t2;
      double _t3;
      int j;
      int j_1;
      for (_t2 = Q[i],
           _t1 = P[i],
           j = 0,
           j_1 = i_1;
           _t3 = 0.3 + (double)j_1 * 0.5, j < 10000; j++, j_1 += i_2) {
         real_t _t4;
         _t4 = (float)_t3;
         _t1[j] = _t4;
         _t2[j] = _t4;
      }
   }
}

/*
 * sor.c:56.6
 */
void initialize_once(void)
{
   initialize();
}

/*
 * sor.c:62.6
 */
void show(void)
{
   void const* _t1;
   _t1 = P;
   print_submatrix("P", _t1, 10000, 10000, 0, 8, 0, 8);
   print_submatrix("P", _t1, 10000, 10000, 9992, 10000, 9992, 10000);
}

/*
 * sor.c:72.6
 */
void kernel(void)
{
   union __args_so_main_24* allArgs;
   ocrGuid_t _t1;
   rocrDeclareType(so_main, 1, (unsigned int)1, 1, (unsigned long (*)(va_list))
      ((void*)0l));
   rocrDeclareType(so1, 0, (unsigned int)800, 0, so1_count_0);
   rocrInit();
   _t1 = rocrAlloc((void**)&allArgs, 8ul);
   allArgs->data.A = P;
   rocrExecute(1, _t1);
   rocrExit();
}

/*
 * sor.c:76.5
 */
int check(void)
{
   int i;
   int i_1;
   for (i = 1, i_1 = 0; i < 9999; ) {
      real_t* _t1;
      real_t* _t2;
      real_t* _t3;
      int j;
      int j_1;
      _t2 = Q[i_1];
      _t1 = Q[i];
      i++;
      _t3 = Q[i];
      i_1++;
      for (j_1 = 1, j = 0; j_1 < 9999; ) {
         real_t* _t4;
         float _t5;
         float _t6;
         int _t7;
         _t4 = _t1 + j_1;
         _t5 = *_t4;
         _t6 = _t1[j];
         _t7 = j_1 + 1;
         *_t4 = (_t5 + _t6 + _t1[_t7] + _t2[j_1] + _t3[j_1]) / 5.0f;
         j++;
         j_1 = _t7;
      }
   }
   return check_matrices("P", "Q", P, Q, 10000, 10000);
}

/*
 * sor.c:84.8
 */
double flops_per_trial(void)
{
   return (double)(mul_flop + add_flop) * 10000.0 * 10000.0;
}

void so(real_t (* A)[10000])
{
   union __args_so_main_24* allArgs;
   ocrGuid_t _t1;
   rocrDeclareType(so_main, 1, (unsigned int)1, 1, (unsigned long (*)(va_list))
      ((void*)0l));
   rocrDeclareType(so1, 0, (unsigned int)800, 0, so1_count_0);
   rocrInit();
   _t1 = rocrAlloc((void**)&allArgs, 8ul);
   allArgs->data.A = A;
   rocrExecute(1, _t1);
   rocrExit();
}

static ocrGuid_t so1(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv)
{
   union __args_so1_22 so1_args;
   union __args_so1_22* allArgs;
   real_t (* A)[10000];
   int IT0;
   int IT1;
   int _t1;
   int i;
   int _t2;
   int i_1;
   int i_2;
   int i_3;
   rocrParseInputArgs(paramc, paramv, depc, depv);
   for (so1_args = *(union __args_so1_22*)rocrArgs(depv),
        IT0 = so1_args.data.IT0,
        _t1 = (__mins_32(-256 * IT0 + 9997, 255)),
        IT1 = so1_args.data.IT1,
        A = so1_args.data.A,
        i = 0;
        i <= _t1; i++) {
      int _t3;
      int j;
      for (_t3 = (__mins_32(-512 * IT1 + 9997, 511)), j = 0; j <= _t3; j++) {
         A[i + (256 * IT0 + 1)][j + (512 * IT1 + 1)] = (A[i + (256 * IT0 + 1)][
            j + (512 * IT1 + 1)] + A[i + (256 * IT0 + 1)][j + 512 * IT1] + A[i +
            (256 * IT0 + 1)][j + (512 * IT1 + 2)] + A[i + 256 * IT0][j + (512 *
            IT1 + 1)] + A[i + (256 * IT0 + 2)][j + (512 * IT1 + 1)]) / 5.0f;
      }
   }
   for (_t2 = (__mins_32(39, IT0 + 1)),
        i_1 = (__maxs_32(IT0 + IT1 + -18, IT0)),
        i_2 = i_1 * 20,
        i_3 = i_1 * -1 + IT0;
        i_1 <= _t2; i_3--, i_2 += 20, i_1++) {
      ocrGuid_t _t4;
      union __args_so1_22* _t5;
      int _t6;
      _t4 = rocrAlloc((void**)&allArgs, 16ul);
      _t5 = allArgs;
      _t5->data.A = A;
      _t5->data.IT0 = i_1;
      _t6 = i_3 + (IT1 + 1);
      _t5->data.IT1 = _t6;
      rocrAutoDec(0, _t4, (unsigned int)(i_3 + (IT1 + 1) + i_2), i_1, _t6);
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

unsigned long so1_count_0(va_list args)
{
   unsigned long rcNbInputDeps;
   int IT0;
   int _t1;
   int _t2;
   int i;
   for (_t1 = va_arg(args, int),
        IT0 = _t1,
        _t2 = (__mins_32(IT0, IT0 + va_arg(args, int) + -1)),
        i = (__maxs_32(IT0 + -1, 0)),
        rcNbInputDeps = 0ul;
        i <= _t2; i++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

static ocrGuid_t so_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_so_main_24 so_main_args;
   union __args_so1_22* allArgs;
   ocrGuid_t _t1;
   union __args_so1_22* _t2;
   rocrParseInputArgs(paramc, paramv, depc, depv);
   so_main_args = *(union __args_so_main_24*)rocrArgs(depv);
   _t1 = rocrAlloc((void**)&allArgs, 16ul);
   _t2 = allArgs;
   _t2->data.A = so_main_args.data.A;
   _t2->data.IT0 = 0;
   _t2->data.IT1 = 0;
   rocrCreateTask(0, _t1, (unsigned int)0, 0, 0);
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}


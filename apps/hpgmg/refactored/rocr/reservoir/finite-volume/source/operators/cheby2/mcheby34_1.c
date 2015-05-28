/*
 * Optimization PARSE[-I//usr/include, -I/home/henretty/hg/pca/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DRSTREAM_CHEBY, -D__RSTREAM_CC__, finite-volume/source/operators/cheby2/cheby34_1.c] (126ms)
 * Optimization SSA (57ms)
 * Optimization CCP (20ms)
 * Optimization GVNGCM (46ms)
 *   Optimization SimplifyControl (15ms)
 * Optimization OSR (21ms)
 * Optimization DCE (6ms)
 * Optimization Raise[-liftnone] (553ms)
 * Optimization PolyhedralMapperNew[logfile=mapper.log] (4094ms)
 * Optimization Lower (291ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (1ms)
 *   Optimization InsertMapperAnnotations (2ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization DCE (8ms)
 * Optimization SyntaxPrepass (11ms)
 * Optimization DCE (11ms)
 * Optimization DeSSA (211ms)
 */
#define SOCR true
#include <rstream_ocr.h>
#include <ocr.h>
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon1_34_1;
union __args_cheby_mappable_kernel34_11_2;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon3_34_1;
union __args_cheby_mappable_kernel34_1_main_4;
union ocrGuidUnion_t;
struct va_list_t;
struct __anon1_34_1 {
   double const* chebyshev_c1;
   double const* chebyshev_c2;
   double (* x_np1_0)[34][34];
   double (* x_np1_1)[34][34];
   double const (* alpha)[34][34];
   double const (* beta_i)[34][34];
   double const (* valid)[34][34];
   double const (* beta_j)[34][34];
   double const (* beta_k)[34][34];
   double const (* rhs)[34][34];
   int H2INV;
   int s;
   int ss;
   int ghosts;
   int ghostsToOperateOn;
   int dim;
   int IT0;
   int IT1;
   int IT2;
};
union __args_cheby_mappable_kernel34_11_2 {
   struct __anon1_34_1 data;
   double padding[15];
};
struct __anon3_34_1 {
   double const* chebyshev_c1;
   double const* chebyshev_c2;
   double (* x_np1_0)[34][34];
   double (* x_np1_1)[34][34];
   double const (* alpha)[34][34];
   double const (* beta_i)[34][34];
   double const (* valid)[34][34];
   double const (* beta_j)[34][34];
   double const (* beta_k)[34][34];
   double const (* rhs)[34][34];
   int H2INV;
   int s;
   int ss;
   int ghosts;
   int ghostsToOperateOn;
   int dim;
};
union __args_cheby_mappable_kernel34_1_main_4 {
   struct __anon3_34_1 data;
   double padding[13];
};
/*
 * Forward declarations of functions
 */
void cheby_mappable_kernel34_1(int H2INV, int s, int ss, int ghosts, int
   ghostsToOperateOn, int dim, double const* chebyshev_c1, double const*
   chebyshev_c2, double (* x_np1_0)[34][34], double (* x_np1_1)[34][34], double
   const (* rhs)[34][34], double const (* alpha)[34][34], double const (*
   beta_i)[34][34], double const (* beta_j)[34][34], double const (* beta_k)[34
   ][34], double const (* Dinv)[34][34], double const (* valid)[34][34]);
static ocrGuid_t cheby_mappable_kernel34_11(unsigned int paramc, unsigned long*
    paramv, unsigned int depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
ocrGuid_t rocrAlloc(void** x0, unsigned long k);
static ocrGuid_t cheby_mappable_kernel34_1_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void rocrExit(void);
/* This can be redefined to be the appropriate 'inline' keyword */
#ifndef RSTREAM_INLINE
#define RSTREAM_INLINE
#endif
#ifdef GPU_TARGET
    #define __mins34_1_32(x,y) \
 min((int)(x),(int)(y))
#else
static RSTREAM_INLINE int __mins34_1_32(int x, int y) {
   return x < y ? x : y;
}
#endif
/*
 * Definitions of functions
 */
void cheby_mappable_kernel34_1(int H2INV, int s, int ss, int ghosts, int
   ghostsToOperateOn, int dim, double const* chebyshev_c1, double const*
   chebyshev_c2, double (* x_np1_0)[34][34], double (* x_np1_1)[34][34], double
   const (* rhs)[34][34], double const (* alpha)[34][34], double const (*
   beta_i)[34][34], double const (* beta_j)[34][34], double const (* beta_k)[34
   ][34], double const (* Dinv)[34][34], double const (* valid)[34][34])
{
   union __args_cheby_mappable_kernel34_1_main_4* allArgs;
   rocrDeclareType(cheby_mappable_kernel34_1_main, 1, (unsigned int)1, 1, (
      unsigned long (*)(va_list))((void*)0l));
   rocrDeclareType(cheby_mappable_kernel34_11, 0, (unsigned int)(2 *
      ghostsToOperateOn + dim >= 1 ? 1 + (-1 + dim + 2 * ghostsToOperateOn >> 1
      ) + (1 + (-1 + dim + 2 * ghostsToOperateOn >> 1)) * (-1 + dim + 2 *
      ghostsToOperateOn >> 5) + (1 + (-1 + dim + 2 * ghostsToOperateOn >> 1) + (
      1 + (-1 + dim + 2 * ghostsToOperateOn >> 1)) * (-1 + dim + 2 *
      ghostsToOperateOn >> 5)) * (-1 + dim + 2 * ghostsToOperateOn >> 9) : 0),
      0, (unsigned long (*)(va_list))((void*)0l));
   rocrInit();
   if (2 * ghostsToOperateOn + dim >= 1) {
      ocrGuid_t _t1;
      union __args_cheby_mappable_kernel34_1_main_4* _t2;
      _t1 = rocrAlloc((void**)&allArgs, 104ul);
      _t2 = allArgs;
      _t2->data.chebyshev_c1 = chebyshev_c1;
      _t2->data.chebyshev_c2 = chebyshev_c2;
      _t2->data.x_np1_0 = x_np1_0;
      _t2->data.x_np1_1 = x_np1_1;
      _t2->data.alpha = alpha;
      _t2->data.beta_i = beta_i;
      _t2->data.valid = valid;
      _t2->data.beta_j = beta_j;
      _t2->data.beta_k = beta_k;
      _t2->data.rhs = rhs;
      _t2->data.H2INV = H2INV;
      _t2->data.s = s;
      _t2->data.ss = ss;
      _t2->data.ghosts = ghosts;
      _t2->data.ghostsToOperateOn = ghostsToOperateOn;
      _t2->data.dim = dim;
      rocrExecute(1, _t1);
   }
   rocrExit();
}

static ocrGuid_t cheby_mappable_kernel34_11(unsigned int paramc, unsigned long*
    paramv, unsigned int depc, ocrEdtDep_t* depv)
{ rocrParseInputArgs(paramc, paramv, depc, depv);
   union __args_cheby_mappable_kernel34_11_2 cheby_mappable_kernel34_11_args;
   double const* chebyshev_c1;
   double const* chebyshev_c2;
   double (* x_np1_0)[34][34];
   double (* x_np1_1)[34][34];
   double const (* alpha)[34][34];
   double const (* beta_i)[34][34];
   double const (* valid)[34][34];
   double const (* beta_j)[34][34];
   double const (* beta_k)[34][34];
   double const (* rhs)[34][34];
   int H2INV;
   int ss;
   int ghostsToOperateOn;
   int dim;
   int IT0;
   int IT1;
   int IT2;
   int _t1;
   int i;
   for (cheby_mappable_kernel34_11_args = *(union
           __args_cheby_mappable_kernel34_11_2*)rocrArgs(depv),
        IT2 = cheby_mappable_kernel34_11_args.data.IT2,
        IT1 = cheby_mappable_kernel34_11_args.data.IT1,
        IT0 = cheby_mappable_kernel34_11_args.data.IT0,
        dim = cheby_mappable_kernel34_11_args.data.dim,
        ghostsToOperateOn = cheby_mappable_kernel34_11_args.data.
           ghostsToOperateOn,
        _t1 = (__mins34_1_32(1, 2 * ghostsToOperateOn + dim + -2 * IT0 + -1)),
        ss = cheby_mappable_kernel34_11_args.data.ss,
        H2INV = cheby_mappable_kernel34_11_args.data.H2INV,
        rhs = cheby_mappable_kernel34_11_args.data.rhs,
        beta_k = cheby_mappable_kernel34_11_args.data.beta_k,
        beta_j = cheby_mappable_kernel34_11_args.data.beta_j,
        valid = cheby_mappable_kernel34_11_args.data.valid,
        beta_i = cheby_mappable_kernel34_11_args.data.beta_i,
        alpha = cheby_mappable_kernel34_11_args.data.alpha,
        x_np1_1 = cheby_mappable_kernel34_11_args.data.x_np1_1,
        x_np1_0 = cheby_mappable_kernel34_11_args.data.x_np1_0,
        chebyshev_c2 = cheby_mappable_kernel34_11_args.data.chebyshev_c2,
        chebyshev_c1 = cheby_mappable_kernel34_11_args.data.chebyshev_c1,
        i = 0;
        i <= _t1; i++) {
      int _t2;
      int j;
      for (_t2 = (__mins34_1_32(31, 2 * ghostsToOperateOn + dim + -32 * IT1 + -1)),
               j = 0; j <= _t2; j++) {
         int _t3;
         int k;
         for (_t3 = (__mins34_1_32(511, 2 * ghostsToOperateOn + dim + -512 * IT2 +
                 -1)), k = 0; k <= _t3; k++) {
            double _t4;
            double _t5;
            double _t6;
            double _t7;
            double _t8;
            double _t9;
            double _t10;
            double _t11;
            double _t12;
            double _t13;
            double _t14;
            double _t15;
            double _t16;
            double _t17;
            double _t18;
            _t4 = x_np1_1[i + (-1 * ghostsToOperateOn + 2 * IT0)][j + (-1 *
               ghostsToOperateOn + 32 * IT1)][k + (-1 * ghostsToOperateOn + 512
               * IT2)];
            _t18 = 2.0 * _t4;
            _t17 = *(double*)(valid[i + -1 * ghostsToOperateOn + (2 * IT0 + 1)]
               [j + (-1 * ghostsToOperateOn + 32 * IT1)] + (k + (-1 *
               ghostsToOperateOn + 512 * IT2)));
            _t16 = *(double*)(beta_k[i + -1 * ghostsToOperateOn + (2 * IT0 + 1)
               ][j + (-1 * ghostsToOperateOn + 32 * IT1)] + (k + (-1 *
               ghostsToOperateOn + 512 * IT2)));
            _t15 = *(double*)(valid[i + (-1 * ghostsToOperateOn + 2 * IT0)][j +
               -1 * ghostsToOperateOn + (32 * IT1 + 1)] + (k + (-1 *
               ghostsToOperateOn + 512 * IT2)));
            _t14 = *(double*)(beta_j[i + (-1 * ghostsToOperateOn + 2 * IT0)][j +
               -1 * ghostsToOperateOn + (32 * IT1 + 1)] + (k + (-1 *
               ghostsToOperateOn + 512 * IT2)));
            _t13 = *(double*)(valid[i + (-1 * ghostsToOperateOn + 2 * IT0)][j +
               (-1 * ghostsToOperateOn + 32 * IT1)] + (k + -1 *
               ghostsToOperateOn + (512 * IT2 + 1)));
            _t12 = *(double*)(beta_i[i + (-1 * ghostsToOperateOn + 2 * IT0)][j +
               (-1 * ghostsToOperateOn + 32 * IT1)] + (k + -1 *
               ghostsToOperateOn + (512 * IT2 + 1)));
            _t11 = *(double*)(valid[i + -1 * ghostsToOperateOn + (2 * IT0 + -1)
               ][j + (-1 * ghostsToOperateOn + 32 * IT1)] + (k + (-1 *
               ghostsToOperateOn + 512 * IT2)));
            _t10 = *(double*)(beta_k[i + (-1 * ghostsToOperateOn + 2 * IT0)][j +
               (-1 * ghostsToOperateOn + 32 * IT1)] + (k + (-1 *
               ghostsToOperateOn + 512 * IT2)));
            _t9 = *(double*)(valid[i + (-1 * ghostsToOperateOn + 2 * IT0)][j +
               -1 * ghostsToOperateOn + (32 * IT1 + -1)] + (k + (-1 *
               ghostsToOperateOn + 512 * IT2)));
            _t8 = *(double*)(beta_j[i + (-1 * ghostsToOperateOn + 2 * IT0)][j +
               (-1 * ghostsToOperateOn + 32 * IT1)] + (k + (-1 *
               ghostsToOperateOn + 512 * IT2)));
            _t7 = *(double*)(valid[i + (-1 * ghostsToOperateOn + 2 * IT0)][j + (
               -1 * ghostsToOperateOn + 32 * IT1)] + (k + -1 * ghostsToOperateOn
               + (512 * IT2 + -1)));
            _t6 = *(double*)(beta_i[i + (-1 * ghostsToOperateOn + 2 * IT0)][j +
               (-1 * ghostsToOperateOn + 32 * IT1)] + (k + (-1 *
               ghostsToOperateOn + 512 * IT2)));
            _t5 = 0.0 * *(double*)(alpha[i + (-1 * ghostsToOperateOn + 2 * IT0)
               ][j + (-1 * ghostsToOperateOn + 32 * IT1)] + (k + (-1 *
               ghostsToOperateOn + 512 * IT2)));
            x_np1_0[i + (-1 * ghostsToOperateOn + 2 * IT0)][j + (-1 *
               ghostsToOperateOn + 32 * IT1)][k + (-1 * ghostsToOperateOn + 512
               * IT2)] = _t4 + *(double*)(chebyshev_c1 + ss) * (_t4 - x_np1_0[i
               + (-1 * ghostsToOperateOn + 2 * IT0)][j + (-1 * ghostsToOperateOn
               + 32 * IT1)][k + (-1 * ghostsToOperateOn + 512 * IT2)]) + *(
               double*)(chebyshev_c2 + ss) * (1.0 / (_t5 - 1.0 * (double)H2INV *
               (_t6 * (_t7 - 2.0) + _t8 * (_t9 - 2.0) + _t10 * (_t11 - 2.0) +
               _t12 * (_t13 - 2.0) + _t14 * (_t15 - 2.0) + _t16 * (_t17 - 2.0))
               )) * (*(double*)(rhs[i + (-1 * ghostsToOperateOn + 2 * IT0)][j +
               (-1 * ghostsToOperateOn + 32 * IT1)] + (k + (-1 *
               ghostsToOperateOn + 512 * IT2))) - (_t5 * _t4 - 1.0 * (double)
               H2INV * (_t6 * (_t7 * (_t4 + x_np1_1[i + (-1 * ghostsToOperateOn
               + 2 * IT0)][j + (-1 * ghostsToOperateOn + 32 * IT1)][k + -1 *
               ghostsToOperateOn + (512 * IT2 + -1)]) - _t18) + _t8 * (_t9 * (
               _t4 + x_np1_1[i + (-1 * ghostsToOperateOn + 2 * IT0)][j + -1 *
               ghostsToOperateOn + (32 * IT1 + -1)][k + (-1 * ghostsToOperateOn
               + 512 * IT2)]) - _t18) + _t10 * (_t11 * (_t4 + x_np1_1[i + -1 *
               ghostsToOperateOn + (2 * IT0 + -1)][j + (-1 * ghostsToOperateOn +
               32 * IT1)][k + (-1 * ghostsToOperateOn + 512 * IT2)]) - _t18) +
               _t12 * (_t13 * (_t4 + x_np1_1[i + (-1 * ghostsToOperateOn + 2 *
               IT0)][j + (-1 * ghostsToOperateOn + 32 * IT1)][k + -1 *
               ghostsToOperateOn + (512 * IT2 + 1)]) - _t18) + _t14 * (_t15 * (
               _t4 + x_np1_1[i + (-1 * ghostsToOperateOn + 2 * IT0)][j + -1 *
               ghostsToOperateOn + (32 * IT1 + 1)][k + (-1 * ghostsToOperateOn +
               512 * IT2)]) - _t18) + _t16 * (_t17 * (_t4 + x_np1_1[i + -1 *
               ghostsToOperateOn + (2 * IT0 + 1)][j + (-1 * ghostsToOperateOn +
               32 * IT1)][k + (-1 * ghostsToOperateOn + 512 * IT2)]) - _t18))))
               ;
         }
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t cheby_mappable_kernel34_1_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{ rocrParseInputArgs(paramc, paramv, depc, depv);
   union __args_cheby_mappable_kernel34_1_main_4
      cheby_mappable_kernel34_1_main_args;
   union __args_cheby_mappable_kernel34_11_2* allArgs;
   double const* chebyshev_c1;
   double const* chebyshev_c2;
   double (* x_np1_0)[34][34];
   double (* x_np1_1)[34][34];
   double const (* alpha)[34][34];
   double const (* beta_i)[34][34];
   double const (* valid)[34][34];
   double const (* beta_j)[34][34];
   double const (* beta_k)[34][34];
   double const (* rhs)[34][34];
   int H2INV;
   int s;
   int ss;
   int ghosts;
   int ghostsToOperateOn;
   int dim;
   int _t1;
   int i;
   for (cheby_mappable_kernel34_1_main_args = *(union
           __args_cheby_mappable_kernel34_1_main_4*)rocrArgs(depv),
        dim = cheby_mappable_kernel34_1_main_args.data.dim,
        ghostsToOperateOn = cheby_mappable_kernel34_1_main_args.data.
           ghostsToOperateOn,
        _t1 = 2 * ghostsToOperateOn + dim + -1 >> 1,
        ghosts = cheby_mappable_kernel34_1_main_args.data.ghosts,
        ss = cheby_mappable_kernel34_1_main_args.data.ss,
        s = cheby_mappable_kernel34_1_main_args.data.s,
        H2INV = cheby_mappable_kernel34_1_main_args.data.H2INV,
        rhs = cheby_mappable_kernel34_1_main_args.data.rhs,
        beta_k = cheby_mappable_kernel34_1_main_args.data.beta_k,
        beta_j = cheby_mappable_kernel34_1_main_args.data.beta_j,
        valid = cheby_mappable_kernel34_1_main_args.data.valid,
        beta_i = cheby_mappable_kernel34_1_main_args.data.beta_i,
        alpha = cheby_mappable_kernel34_1_main_args.data.alpha,
        x_np1_1 = cheby_mappable_kernel34_1_main_args.data.x_np1_1,
        x_np1_0 = cheby_mappable_kernel34_1_main_args.data.x_np1_0,
        chebyshev_c2 = cheby_mappable_kernel34_1_main_args.data.chebyshev_c2,
        chebyshev_c1 = cheby_mappable_kernel34_1_main_args.data.chebyshev_c1,
        i = 0;
        i <= _t1; i++) {
      int _t2;
      int j;
      for (_t2 = 2 * ghostsToOperateOn + dim + -1 >> 5, j = 0; j <= _t2; j++) {
         int _t3;
         int k;
         for (_t3 = 2 * ghostsToOperateOn + dim + -1 >> 9, k = 0; k <= _t3; k++
            ) {
            ocrGuid_t _t4;
            union __args_cheby_mappable_kernel34_11_2* _t5;
            _t4 = rocrAlloc((void**)&allArgs, 120ul);
            _t5 = allArgs;
            _t5->data.chebyshev_c1 = chebyshev_c1;
            _t5->data.chebyshev_c2 = chebyshev_c2;
            _t5->data.x_np1_0 = x_np1_0;
            _t5->data.x_np1_1 = x_np1_1;
            _t5->data.alpha = alpha;
            _t5->data.beta_i = beta_i;
            _t5->data.valid = valid;
            _t5->data.beta_j = beta_j;
            _t5->data.beta_k = beta_k;
            _t5->data.rhs = rhs;
            _t5->data.H2INV = H2INV;
            _t5->data.s = s;
            _t5->data.ss = ss;
            _t5->data.ghosts = ghosts;
            _t5->data.ghostsToOperateOn = ghostsToOperateOn;
            _t5->data.dim = dim;
            _t5->data.IT0 = i;
            _t5->data.IT1 = j;
            _t5->data.IT2 = k;
            rocrCreateTask(0, _t4, (unsigned int)(k + j + i + i * (-1 + dim + 2
               * ghostsToOperateOn >> 5) + (j + i + i * (-1 + dim + 2 *
               ghostsToOperateOn >> 5)) * (-1 + dim + 2 * ghostsToOperateOn >> 9
               )), H2INV, s, ss, ghosts, ghostsToOperateOn, dim, i, j, k);
         }
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}


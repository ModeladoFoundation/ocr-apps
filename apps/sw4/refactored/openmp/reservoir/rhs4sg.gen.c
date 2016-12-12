/*
 * Optimization PARSE[-I//usr/include, -I/home/pradelle/work/pca-asr-gpu/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -I../include, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -D__RSTREAM_CC__, ../src/rhs4sg.c] (807ms)
 * Optimization SSA (220ms)
 * Optimization CCP (118ms)
 * Optimization DCE (168ms)
 * Optimization GVNGCM[scc=true] (745ms)
 *   Optimization SimplifyControl (20ms)
 *   Optimization SimplifyControl (4ms)
 *   Optimization SimplifyControl (4ms)
 *   Optimization SimplifyControl (4ms)
 *   Optimization SimplifyControl (8ms)
 *   Optimization SimplifyControl (3ms)
 *   Optimization SimplifyControl (3ms)
 *   Optimization SimplifyControl (4ms)
 *   Optimization SimplifyControl (4ms)
 *   Optimization SimplifyControl (4ms)
 *   Optimization SimplifyControl (4ms)
 *   Optimization SimplifyControl (4ms)
 *   Optimization SimplifyControl (4ms)
 *   Optimization SimplifyControl (4ms)
 *   Optimization SimplifyControl (4ms)
 *   Optimization SimplifyControl (5ms)
 *   Optimization SimplifyControl (4ms)
 *   Optimization SimplifyControl (4ms)
 * Optimization Raise[-liftnone, -stacktrace] (2137ms)
 *   Optimization LiftScalars (5ms)
 *   Optimization CCP (38ms)
 *   Optimization DCE (85ms)
 *   Optimization DeSSA (499ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (12ms)
 *   Optimization InwardPropagation (36ms)
 *   Optimization DeSSA (40ms)
 *   Optimization DeSSA (13ms)
 *   Optimization DeSSA (13ms)
 * Optimization PolyhedralMapperNew[no-synchronization, no-placement, -stacktrace, as:force_identity=3, mplacetile:sizes={1-1-1}, pragmasimd, logfile=rhs4sg-mapper.log] (8450ms)
 * Optimization Lower[-stacktrace] (341ms)
 *   Optimization SSA (5ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization InsertMapperAnnotations (2ms)
 *   Optimization SSA (0ms)
 *   Optimization LiftScalars (16ms)
 *   Optimization LiftScalars (15ms)
 *   Optimization DCE (27ms)
 * Optimization SyntaxPrepass (16ms)
 * Optimization DCE (25ms)
 * Optimization HLE (53ms)
 * Optimization DeSSA (1818ms)
 */
#include <stdlib.h>
void omp_set_dynamic(int dynamic_threads);
typedef unsigned long size_t;
/*
 * Forward declarations of functions
 */
int fequal(double, double);
void* malloc(size_t);
void* calloc(size_t, size_t);
static void get_data(double x, double y, double z, double* u, double* v, double
   * w, double* mu, double* lambda, double* rho);
static void fg(double x, double y, double z, double* eqs);
static int wavepropbop_4__(double* iop, double* iop2, double* bop, double* bop2
   , double* gh2, double* h__, double* s);
static int bopext4th_(double* bop, double* bope);
static int varcoeffs4_(double* acof, double* ghcof);
void initialize_once(void);
void initialize(void);
void core_fn1(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided, double (* a_acof)[8][6], double (* a_bope)[6],
    double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
   ;
void core_fn2(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided, double (* a_acof)[8][6], double (* a_bope)[6],
    double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
   ;
void core_fn3(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided, double (* a_acof)[8][6], double (* a_bope)[6],
    double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
   ;
void mapped_fn1(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided, double (* a_acof)[8][6], double (* a_bope)[6],
    double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
   ;
void mapped_fn2(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided, double (* a_acof)[8][6], double (* a_bope)[6],
    double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
   ;
void mapped_fn3(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided, double (* a_acof)[8][6], double (* a_bope)[6],
    double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
   ;
void kernel(void);
static void unmapped_kernel(double (* lu)[25][25][3]);
void show(void);
int check(void);
double flops_per_trial(void);
#ifndef GPU_TARGET
extern double cos(double);
#endif
#ifndef GPU_TARGET
extern double sin(double);
#endif
extern double pow(double, double);
/*
 * Definitions of global variables
 */
static int onesided[6] = {0, 0, 0, 0, 0, 0};
static double acof[8][8][6];
static double bope[8][6];
static double ghcof[6];
static double (* lu)[25][25][3];
static double (* u)[25][25][3];
static double (* mu)[25][25];
static double (* lambda)[25][25];
static double strx[25];
static double stry[25];
static double strz[25];
static double (* eqs)[25][25][3];
static double (* rho)[25][25];
int nb_samples = 46875;
char const* function_name = "rhs4sg";
static int i__;
static int j;
static double d4a;
static double d4b;
/*
 * Definitions of functions
 */
/*
 * ../src/rhs4sg.c:47.13
 */
static void get_data(double x, double y, double z, double* u_1, double* v,
   double* w, double* mu_1, double* lambda_1, double* rho_1)
{
   double _t1;
   double _t2;
   double _t3;
   double _t4;
   *lambda_1 = cos(x) * pow(sin(3.0 * y), 2.0) * cos(z);
   _t3 = sin(z);
   *mu_1 = sin(3.0 * x) * sin(y) * _t3;
   _t2 = y * y;
   _t1 = x * x;
   *rho_1 = _t1 * x + 1.0 + _t2 + z * z;
   _t4 = x * y;
   *u_1 = cos(_t1) * sin(_t4) * z * z;
   *v = sin(x) * cos(_t2) * _t3;
   *w = cos(_t4) * sin(z * y);
}

/*
 * ../src/rhs4sg.c:60.13
 */
static void fg(double x, double y, double z, double* eqs_1)
{
   double _t1;
   double _t2;
   double _t3;
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
   double _t19;
   double _t20;
   double _t21;
   double _t22;
   double _t23;
   double _t24;
   double _t25;
   double _t26;
   double _t27;
   double _t28;
   double _t29;
   double _t30;
   double _t31;
   double _t32;
   double _t33;
   double _t34;
   double _t35;
   double _t36;
   double _t37;
   double _t38;
   double _t39;
   double _t40;
   double _t41;
   double _t42;
   double _t43;
   double _t44;
   double _t45;
   double _t46;
   double _t47;
   double _t48;
   double _t49;
   double _t50;
   double _t51;
   double _t52;
   double _t53;
   double _t54;
   double _t55;
   double _t56;
   double _t57;
   double _t58;
   double _t59;
   double _t60;
   double _t61;
   double _t62;
   double _t63;
   double _t64;
   double _t65;
   double _t66;
   double _t67;
   double _t68;
   double _t69;
   double _t70;
   double _t71;
   _t16 = z * z;
   _t29 = y * y;
   _t11 = x * x;
   _t44 = 1.0 / (_t11 * x + 1.0 + _t29 + _t16);
   _t14 = y * x;
   _t15 = sin(_t14);
   _t18 = cos(_t11);
   _t28 = _t18 * _t15;
   _t42 = 2.0 * _t28;
   _t32 = z * y;
   _t33 = cos(_t32);
   _t36 = _t15 * _t29 * _t33;
   _t4 = sin(z);
   _t2 = sin(y);
   _t1 = 3.0 * x;
   _t21 = sin(_t1);
   _t22 = _t21 * _t2;
   _t67 = _t22 * _t4;
   _t41 = sin(_t32);
   _t43 = - _t15 * y * _t41 + _t42 * z;
   _t9 = cos(z);
   _t60 = _t22 * _t9;
   _t17 = _t15 * _t16;
   _t27 = _t18 * _t11 * _t17;
   _t40 = y * _t4;
   _t30 = sin(_t29);
   _t23 = cos(x);
   _t19 = cos(_t14);
   _t20 = _t18 * _t19;
   _t38 = cos(_t29);
   _t39 = _t4 * (_t23 * _t38 * _t4 + _t20 * x * _t16);
   _t37 = _t21 * cos(y);
   _t6 = 3.0 * y;
   _t7 = sin(_t6);
   _t8 = _t7 * _t7;
   _t24 = _t23 * _t8;
   _t25 = _t24 * _t9;
   _t34 = _t19 * _t33;
   _t35 = _t34 * y;
   _t5 = sin(x);
   _t10 = _t5 * _t8 * _t9;
   _t31 = _t30 * y * _t4;
   _t12 = sin(_t11);
   _t13 = _t12 * x;
   _t61 = 4.0 * _t13;
   _t47 = 2.0 * _t12;
   _t69 = 2.0 * _t22;
   _t26 = _t69 * _t4 + _t25;
   _t71 = - (2.0 * _t13 * _t17) + _t20 * y * _t16;
   _t3 = cos(_t1) * _t2;
   eqs_1[0] = ((6.0 * _t3 * _t4 - _t10) * _t71 + _t26 * (-4.0 * _t27 - _t47 *
      _t15 * _t16 - _t61 * _t19 * y * _t16 - _t28 * _t29 * _t16) + 2.0 * (_t5 *
      _t5) * _t8 * _t9 * _t31 - 2.0 * (_t23 * _t23) * _t8 * _t9 * _t31 - _t10 *
      _t35 - _t25 * _t36 + _t37 * _t39 + _t67 * (-2.0 * _t23 * _t30 * _t40 -
      _t27) + _t60 * _t43 + _t67 * (- _t36 + _t42)) * _t44;
   _t45 = _t5 * _t38;
   _t46 = _t45 * _t4;
   _t56 = _t19 * _t41;
   _t57 = _t56 * _t32;
   _t54 = _t33 * y;
   _t53 = _t15 * x;
   _t55 = _t53 * _t54;
   _t58 = - _t53 * _t41 + _t34 * z + _t45 * _t9;
   _t50 = _t20 * _t16;
   _t49 = _t28 * _t14 * _t16;
   _t48 = _t47 * _t11 * _t19 * _t16;
   _t51 = cos(_t6);
   _t59 = 6.0 * (_t23 * _t7) * _t9;
   _t52 = _t26 * _t5;
   _t66 = 3.0 * _t3;
   eqs_1[1] = (_t66 * _t39 + _t67 * (- _t46 - _t48 - _t49 + _t50) - 2.0 * (2.0 *
      _t37 * _t4 + _t59 * _t51) * _t5 * _t31 - 4.0 * _t52 * _t38 * _t29 * _t4 -
      2.0 * _t52 * _t30 * _t4 + _t59 * (_t71 + _t35) * _t51 + _t25 * (- _t48 -
      _t49 + _t50 - _t55 - _t57 + _t34) + _t60 * _t58 + _t67 * (- _t55 - _t57 +
      _t34 - _t46)) * _t44;
   _t64 = 2.0 * (_t5 * _t30);
   _t65 = _t64 * y * _t9;
   _t63 = 2.0 * _t20 * _t32;
   _t62 = _t61 * _t15 * z;
   _t70 = _t24 * _t4;
   _t68 = - _t19;
   eqs_1[2] = (_t66 * _t4 * _t43 + _t67 * (_t68 * _t29 * _t41 - _t62 + _t63) +
      _t37 * _t4 * _t58 + _t67 * (_t68 * _t11 * _t41 - 2.0 * _t53 * _t33 * z -
      _t56 * _t16 - _t65) + (_t69 * _t9 - _t70) * _t19 * _t54 - _t26 * _t19 *
      _t41 * _t29 - _t70 * (_t71 - _t64 * _t40) + _t25 * (- _t62 + _t63 - _t65)
      ) * _t44;
}

/*
 * ../src/rhs4sg.c:216.29
 */
static int wavepropbop_4__(double* iop, double* iop2, double* bop, double* bop2
   , double* gh2, double* h__, double* s)
{
   (h__ + -1l)[1] = 0.3541666666666667;
   (h__ + -1l)[2] = 1.2291666666666667;
   (h__ + -1l)[3] = 0.8958333333333334;
   (h__ + -1l)[4] = 1.0208333333333333;
   (iop + -1l)[1] = 0.08333333333333333;
   (iop + -1l)[2] = -0.6666666666666666;
   (iop + -1l)[3] = 0.0;
   (iop + -1l)[4] = 0.6666666666666666;
   (iop + -1l)[5] = -0.08333333333333333;
   (bop + -5)[5] = -1.411764705882353;
   (bop + -5)[9] = 1.7352941176470589;
   (bop + -5)[13] = -0.23529411764705882;
   (bop + -5)[17] = -0.08823529411764706;
   (bop + -5)[21] = 0.0;
   (bop + -5)[25] = 0.0;
   (bop + -5)[6] = -0.5;
   (bop + -5)[10] = 0.0;
   (bop + -5)[14] = 0.5;
   (bop + -5)[18] = 0.0;
   (bop + -5)[22] = 0.0;
   (bop + -5)[26] = 0.0;
   (bop + -5)[7] = 0.09302325581395349;
   (bop + -5)[11] = -0.686046511627907;
   (bop + -5)[15] = 0.0;
   (bop + -5)[19] = 0.686046511627907;
   (bop + -5)[23] = -0.09302325581395349;
   (bop + -5)[27] = 0.0;
   (bop + -5)[8] = 0.030612244897959183;
   (bop + -5)[12] = 0.0;
   (bop + -5)[16] = -0.6020408163265306;
   (bop + -5)[20] = 0.0;
   (bop + -5)[24] = 0.6530612244897959;
   (bop + -5)[28] = -0.08163265306122448;
   (iop2 + -1l)[1] = -0.08333333333333333;
   (iop2 + -1l)[2] = 1.3333333333333333;
   (iop2 + -1l)[3] = -2.5;
   (iop2 + -1l)[4] = 1.3333333333333333;
   (iop2 + -1l)[5] = -0.08333333333333333;
   *gh2 = 0.7058823529411765;
   (bop2 + -5)[5] = -0.8235294117647058;
   (bop2 + -5)[9] = -0.7647058823529411;
   (bop2 + -5)[13] = 1.1764705882352942;
   (bop2 + -5)[17] = -0.29411764705882354;
   (bop2 + -5)[21] = 0.0;
   (bop2 + -5)[25] = 0.0;
   (bop2 + -5)[6] = 1.0;
   (bop2 + -5)[10] = -2.0;
   (bop2 + -5)[14] = 1.0;
   (bop2 + -5)[18] = 0.0;
   (bop2 + -5)[22] = 0.0;
   (bop2 + -5)[26] = 0.0;
   (bop2 + -5)[7] = -0.09302325581395349;
   (bop2 + -5)[11] = 1.372093023255814;
   (bop2 + -5)[15] = -2.558139534883721;
   (bop2 + -5)[19] = 1.372093023255814;
   (bop2 + -5)[23] = -0.09302325581395349;
   (bop2 + -5)[27] = 0.0;
   (bop2 + -5)[8] = -0.02040816326530612;
   (bop2 + -5)[12] = 0.0;
   (bop2 + -5)[16] = 1.2040816326530612;
   (bop2 + -5)[20] = -2.4081632653061225;
   (bop2 + -5)[24] = 1.3061224489795917;
   (bop2 + -5)[28] = -0.08163265306122448;
   s[0] = -0.25;
   s[1] = -0.8333333333333334;
   s[2] = 1.5;
   s[3] = -0.5;
   s[4] = 0.08333333333333333;
   return 0;
}

/*
 * ../src/rhs4sg.c:313.29
 */
static int bopext4th_(double* bop, double* bope_1)
{
   double* _t1;
   double* _t2;
   double* _t3;
   double* _t4;
   double* _t5;
   double* _t6;
   double* _t7;
   double* _t8;
   double* _t9;
   for (_t9 = bope_1 + -7 + 54,
        _t8 = bope_1 + -7 + 48,
        _t7 = bope_1 + -7 + 36,
        _t6 = bope_1 + -7 + 30,
        _t5 = bope_1 + -7 + 47,
        _t4 = bope_1 + -7 + 41,
        _t3 = bope_1 + -7 + 29,
        _t2 = bope_1 + -7 + 23,
        _t1 = bop + -5,
        j = 1;
        j <= 8; ) {
      int _t10;
      int _t11;
      int _t12;
      i__ = 1;
      for (i__ = 1,
           _t12 = j + 1,
           _t11 = j * 6,
           i__,
           _t10 = i__;
           _t10 <= 6; i__++) {
         (bope_1 + -7)[_t10 + _t11] = 0.0;
         _t12 = j + 1;
         _t11 = j * 6;
         i__;
         _t10 = i__;
      }
      j = _t12;
   }
   for (j = 1; j <= 6; ) {
      int _t13;
      int _t14;
      int _t15;
      int _t16;
      int _t17;
      i__ = 1;
      for (i__ = 1,
           _t17 = j + 1,
           _t14 = j,
           _t16 = _t14 << 2,
           j,
           i__,
           _t15 = _t14 * 6,
           i__,
           _t13 = i__;
           _t13 <= 4; i__++) {
         (bope_1 + -7)[_t13 + _t15] = _t1[_t13 + _t16];
         _t17 = j + 1;
         _t14 = j;
         _t16 = _t14 << 2;
         j;
         i__;
         _t15 = _t14 * 6;
         i__;
         _t13 = i__;
      }
      j = _t17;
   }
   d4a = 0.6666666666666666;
   d4b = -0.08333333333333333;
   *_t2 = - d4b;
   *_t3 = - d4a;
   *_t4 = d4a;
   *_t5 = d4b;
   *_t6 = - d4b;
   *_t7 = - d4a;
   *_t8 = d4a;
   *_t9 = d4b;
   return 0;
}

/*
 * ../src/rhs4sg.c:347.29
 */
static int varcoeffs4_(double* acof_1, double* ghcof_1)
{
   (ghcof_1 + -1l)[1] = 0.7058823529411765;
   (ghcof_1 + -1l)[2] = 0.0;
   (ghcof_1 + -1l)[3] = 0.0;
   (ghcof_1 + -1l)[4] = 0.0;
   (ghcof_1 + -1l)[5] = 0.0;
   (ghcof_1 + -1l)[6] = 0.0;
   (acof_1 + -55)[55] = 0.35986159169550175;
   (acof_1 + -55)[103] = -1.0166864283333032;
   (acof_1 + -55)[151] = -0.19053503754442955;
   (acof_1 + -55)[199] = -0.0027010804321728693;
   (acof_1 + -55)[247] = 0.0;
   (acof_1 + -55)[295] = 0.0;
   (acof_1 + -55)[343] = 0.0;
   (acof_1 + -55)[391] = 0.0;
   (acof_1 + -55)[61] = -1.7854671280276817;
   (acof_1 + -55)[109] = 0.44711810852932143;
   (acof_1 + -55)[157] = 0.6869721896966721;
   (acof_1 + -55)[205] = 0.0;
   (acof_1 + -55)[253] = 0.0;
   (acof_1 + -55)[301] = 0.0;
   (acof_1 + -55)[349] = 0.0;
   (acof_1 + -55)[397] = 0.0;
   (acof_1 + -55)[67] = 1.0795847750865053;
   (acof_1 + -55)[115] = 0.42052895029420795;
   (acof_1 + -55)[163] = -0.5647651673528509;
   (acof_1 + -55)[211] = 0.05312124849939976;
   (acof_1 + -55)[259] = 0.0;
   (acof_1 + -55)[307] = 0.0;
   (acof_1 + -55)[355] = 0.0;
   (acof_1 + -55)[403] = 0.0;
   (acof_1 + -55)[73] = -0.35986159169550175;
   (acof_1 + -55)[121] = 0.1490393695097738;
   (acof_1 + -55)[169] = 0.06604803344046237;
   (acof_1 + -55)[217] = 0.0;
   (acof_1 + -55)[265] = 0.0;
   (acof_1 + -55)[313] = 0.0;
   (acof_1 + -55)[361] = 0.0;
   (acof_1 + -55)[409] = 0.0;
   (acof_1 + -55)[79] = 0.0;
   (acof_1 + -55)[127] = 0.0;
   (acof_1 + -55)[175] = 0.002279981760145919;
   (acof_1 + -55)[223] = -0.057623049219687875;
   (acof_1 + -55)[271] = 0.0;
   (acof_1 + -55)[319] = 0.0;
   (acof_1 + -55)[367] = 0.0;
   (acof_1 + -55)[415] = 0.0;
   (acof_1 + -55)[85] = 0.0;
   (acof_1 + -55)[133] = 0.0;
   (acof_1 + -55)[181] = 0.0;
   (acof_1 + -55)[229] = 0.007202881152460984;
   (acof_1 + -55)[277] = 0.0;
   (acof_1 + -55)[325] = 0.0;
   (acof_1 + -55)[373] = 0.0;
   (acof_1 + -55)[421] = 0.0;
   (acof_1 + -55)[91] = 0.0;
   (acof_1 + -55)[139] = 0.0;
   (acof_1 + -55)[187] = 0.0;
   (acof_1 + -55)[235] = 0.0;
   (acof_1 + -55)[283] = 0.0;
   (acof_1 + -55)[331] = 0.0;
   (acof_1 + -55)[379] = 0.0;
   (acof_1 + -55)[427] = 0.0;
   (acof_1 + -55)[97] = 0.0;
   (acof_1 + -55)[145] = 0.0;
   (acof_1 + -55)[193] = 0.0;
   (acof_1 + -55)[241] = 0.0;
   (acof_1 + -55)[289] = 0.0;
   (acof_1 + -55)[337] = 0.0;
   (acof_1 + -55)[385] = 0.0;
   (acof_1 + -55)[433] = 0.0;
   (acof_1 + -55)[56] = 0.7058823529411765;
   (acof_1 + -55)[104] = 0.12883064144065193;
   (acof_1 + -55)[152] = 0.19794113940412586;
   (acof_1 + -55)[200] = 0.0;
   (acof_1 + -55)[248] = 0.0;
   (acof_1 + -55)[296] = 0.0;
   (acof_1 + -55)[344] = 0.0;
   (acof_1 + -55)[392] = 0.0;
   (acof_1 + -55)[62] = -0.8676470588235294;
   (acof_1 + -55)[110] = -0.38649192432195584;
   (acof_1 + -55)[158] = -0.8481941061801149;
   (acof_1 + -55)[206] = -0.03393316333234121;
   (acof_1 + -55)[254] = 0.0;
   (acof_1 + -55)[302] = 0.0;
   (acof_1 + -55)[350] = 0.0;
   (acof_1 + -55)[398] = 0.0;
   (acof_1 + -55)[68] = 0.11764705882352941;
   (acof_1 + -55)[116] = 0.38649192432195584;
   (acof_1 + -55)[164] = 0.6069354821155897;
   (acof_1 + -55)[212] = 0.10744920751114793;
   (acof_1 + -55)[260] = 0.0;
   (acof_1 + -55)[308] = 0.0;
   (acof_1 + -55)[356] = 0.0;
   (acof_1 + -55)[404] = 0.0;
   (acof_1 + -55)[74] = 0.04411764705882353;
   (acof_1 + -55)[122] = -0.12883064144065193;
   (acof_1 + -55)[170] = 0.038946796692662086;
   (acof_1 + -55)[218] = -0.11874864253939653;
   (acof_1 + -55)[266] = 0.0;
   (acof_1 + -55)[314] = 0.0;
   (acof_1 + -55)[362] = 0.0;
   (acof_1 + -55)[410] = 0.0;
   (acof_1 + -55)[80] = 0.0;
   (acof_1 + -55)[128] = 0.0;
   (acof_1 + -55)[176] = 0.004370687967737351;
   (acof_1 + -55)[224] = 0.0508823158747141;
   (acof_1 + -55)[272] = 0.0;
   (acof_1 + -55)[320] = 0.0;
   (acof_1 + -55)[368] = 0.0;
   (acof_1 + -55)[416] = 0.0;
   (acof_1 + -55)[86] = 0.0;
   (acof_1 + -55)[134] = 0.0;
   (acof_1 + -55)[182] = 0.0;
   (acof_1 + -55)[230] = -0.005649717514124294;
   (acof_1 + -55)[278] = 0.0;
   (acof_1 + -55)[326] = 0.0;
   (acof_1 + -55)[374] = 0.0;
   (acof_1 + -55)[422] = 0.0;
   (acof_1 + -55)[92] = 0.0;
   (acof_1 + -55)[140] = 0.0;
   (acof_1 + -55)[188] = 0.0;
   (acof_1 + -55)[236] = 0.0;
   (acof_1 + -55)[284] = 0.0;
   (acof_1 + -55)[332] = 0.0;
   (acof_1 + -55)[380] = 0.0;
   (acof_1 + -55)[428] = 0.0;
   (acof_1 + -55)[98] = 0.0;
   (acof_1 + -55)[146] = 0.0;
   (acof_1 + -55)[194] = 0.0;
   (acof_1 + -55)[242] = 0.0;
   (acof_1 + -55)[290] = 0.0;
   (acof_1 + -55)[338] = 0.0;
   (acof_1 + -55)[386] = 0.0;
   (acof_1 + -55)[434] = 0.0;
   (acof_1 + -55)[57] = -0.13132694938440492;
   (acof_1 + -55)[105] = 0.16625563151166362;
   (acof_1 + -55)[153] = -0.22327925220926662;
   (acof_1 + -55)[201] = 0.02100142382534409;
   (acof_1 + -55)[249] = 0.0;
   (acof_1 + -55)[297] = 0.0;
   (acof_1 + -55)[345] = 0.0;
   (acof_1 + -55)[393] = 0.0;
   (acof_1 + -55)[63] = 0.16142270861833105;
   (acof_1 + -55)[111] = 0.5303028729068696;
   (acof_1 + -55)[159] = 0.8327719405772044;
   (acof_1 + -55)[207] = 0.1474303079804123;
   (acof_1 + -55)[255] = 0.0;
   (acof_1 + -55)[303] = 0.0;
   (acof_1 + -55)[351] = 0.0;
   (acof_1 + -55)[399] = 0.0;
   (acof_1 + -55)[69] = -0.02188782489740082;
   (acof_1 + -55)[117] = -0.8733261287208232;
   (acof_1 + -55)[165] = -1.1586403084760135;
   (acof_1 + -55)[213] = -0.9155941987395283;
   (acof_1 + -55)[261] = -0.04477139693086537;
   (acof_1 + -55)[309] = 0.0;
   (acof_1 + -55)[357] = 0.0;
   (acof_1 + -55)[405] = 0.0;
   (acof_1 + -55)[75] = -0.008207934336525308;
   (acof_1 + -55)[123] = 0.17676762430228987;
   (acof_1 + -55)[171] = 0.7120818040574802;
   (acof_1 + -55)[219] = 0.6231167426408097;
   (acof_1 + -55)[267] = 0.18082581869957284;
   (acof_1 + -55)[315] = 0.0;
   (acof_1 + -55)[363] = 0.0;
   (acof_1 + -55)[411] = 0.0;
   (acof_1 + -55)[81] = 0.0;
   (acof_1 + -55)[129] = 0.0;
   (acof_1 + -55)[177] = -0.16293418394940454;
   (acof_1 + -55)[225] = 0.1197742482606888;
   (acof_1 + -55)[273] = -0.1343141907925961;
   (acof_1 + -55)[321] = 0.0;
   (acof_1 + -55)[369] = 0.0;
   (acof_1 + -55)[417] = 0.0;
   (acof_1 + -55)[87] = 0.0;
   (acof_1 + -55)[135] = 0.0;
   (acof_1 + -55)[183] = 0.0;
   (acof_1 + -55)[231] = 0.004271476032273375;
   (acof_1 + -55)[279] = -0.0017402309761113748;
   (acof_1 + -55)[327] = 0.0;
   (acof_1 + -55)[375] = 0.0;
   (acof_1 + -55)[423] = 0.0;
   (acof_1 + -55)[93] = 0.0;
   (acof_1 + -55)[141] = 0.0;
   (acof_1 + -55)[189] = 0.0;
   (acof_1 + -55)[237] = 0.0;
   (acof_1 + -55)[285] = 0.0;
   (acof_1 + -55)[333] = 0.0;
   (acof_1 + -55)[381] = 0.0;
   (acof_1 + -55)[429] = 0.0;
   (acof_1 + -55)[99] = 0.0;
   (acof_1 + -55)[147] = 0.0;
   (acof_1 + -55)[195] = 0.0;
   (acof_1 + -55)[243] = 0.0;
   (acof_1 + -55)[291] = 0.0;
   (acof_1 + -55)[339] = 0.0;
   (acof_1 + -55)[387] = 0.0;
   (acof_1 + -55)[435] = 0.0;
   (acof_1 + -55)[58] = -0.04321728691476591;
   (acof_1 + -55)[106] = 0.051707536360533775;
   (acof_1 + -55)[154] = 0.022914623846691025;
   (acof_1 + -55)[202] = 0.0;
   (acof_1 + -55)[250] = 0.0;
   (acof_1 + -55)[298] = 0.0;
   (acof_1 + -55)[346] = 0.0;
   (acof_1 + -55)[394] = 0.0;
   (acof_1 + -55)[64] = 0.05312124849939976;
   (acof_1 + -55)[112] = -0.1551226090816013;
   (acof_1 + -55)[160] = 0.04689512254830741;
   (acof_1 + -55)[208] = -0.14298305938417133;
   (acof_1 + -55)[256] = 0.0;
   (acof_1 + -55)[304] = 0.0;
   (acof_1 + -55)[352] = 0.0;
   (acof_1 + -55)[400] = 0.0;
   (acof_1 + -55)[70] = -0.007202881152460984;
   (acof_1 + -55)[118] = 0.1551226090816013;
   (acof_1 + -55)[166] = 0.6248881137647275;
   (acof_1 + -55)[214] = 0.5468167333378534;
   (acof_1 + -55)[262] = 0.15868388171595169;
   (acof_1 + -55)[310] = 0.0;
   (acof_1 + -55)[358] = 0.0;
   (acof_1 + -55)[406] = 0.0;
   (acof_1 + -55)[76] = -0.0027010804321728693;
   (acof_1 + -55)[124] = -0.051707536360533775;
   (acof_1 + -55)[172] = -0.8800792742346107;
   (acof_1 + -55)[220] = -0.7825518437085323;
   (acof_1 + -55)[268] = -0.802582257392753;
   (acof_1 + -55)[316] = -0.04081632653061224;
   (acof_1 + -55)[364] = 0.0;
   (acof_1 + -55)[412] = 0.0;
   (acof_1 + -55)[82] = 0.0;
   (acof_1 + -55)[130] = 0.0;
   (acof_1 + -55)[178] = 0.18538141407488481;
   (acof_1 + -55)[226] = 0.49658572494018965;
   (acof_1 + -55)[274] = 0.4760516451478551;
   (acof_1 + -55)[322] = 0.16326530612244897;
   (acof_1 + -55)[370] = 0.0;
   (acof_1 + -55)[418] = 0.0;
   (acof_1 + -55)[88] = 0.0;
   (acof_1 + -55)[136] = 0.0;
   (acof_1 + -55)[184] = 0.0;
   (acof_1 + -55)[232] = -0.11786755518533944;
   (acof_1 + -55)[280] = 0.16784673052894628;
   (acof_1 + -55)[328] = -0.12244897959183673;
   (acof_1 + -55)[376] = 0.0;
   (acof_1 + -55)[424] = 0.0;
   (acof_1 + -55)[94] = 0.0;
   (acof_1 + -55)[142] = 0.0;
   (acof_1 + -55)[190] = 0.0;
   (acof_1 + -55)[238] = 0.0;
   (acof_1 + -55)[286] = 0.0;
   (acof_1 + -55)[334] = 0.0;
   (acof_1 + -55)[382] = 0.0;
   (acof_1 + -55)[430] = 0.0;
   (acof_1 + -55)[100] = 0.0;
   (acof_1 + -55)[148] = 0.0;
   (acof_1 + -55)[196] = 0.0;
   (acof_1 + -55)[244] = 0.0;
   (acof_1 + -55)[292] = 0.0;
   (acof_1 + -55)[340] = 0.0;
   (acof_1 + -55)[388] = 0.0;
   (acof_1 + -55)[436] = 0.0;
   (acof_1 + -55)[59] = 0.0;
   (acof_1 + -55)[107] = 0.0;
   (acof_1 + -55)[155] = 8.074935400516795E-4;
   (acof_1 + -55)[203] = -0.02040816326530612;
   (acof_1 + -55)[251] = 0.0;
   (acof_1 + -55)[299] = 0.0;
   (acof_1 + -55)[347] = 0.0;
   (acof_1 + -55)[395] = 0.0;
   (acof_1 + -55)[65] = 0.0;
   (acof_1 + -55)[113] = 0.0;
   (acof_1 + -55)[161] = 0.005372303960343827;
   (acof_1 + -55)[209] = 0.06254284659600275;
   (acof_1 + -55)[257] = 0.0;
   (acof_1 + -55)[305] = 0.0;
   (acof_1 + -55)[353] = 0.0;
   (acof_1 + -55)[401] = 0.0;
   (acof_1 + -55)[71] = 0.0;
   (acof_1 + -55)[119] = 0.0;
   (acof_1 + -55)[167] = -0.14596187312134157;
   (acof_1 + -55)[215] = 0.10729776406686706;
   (acof_1 + -55)[263] = -0.12032312925170068;
   (acof_1 + -55)[311] = 0.0;
   (acof_1 + -55)[359] = 0.0;
   (acof_1 + -55)[407] = 0.0;
   (acof_1 + -55)[77] = 0.0;
   (acof_1 + -55)[125] = 0.0;
   (acof_1 + -55)[173] = 0.18924352686811158;
   (acof_1 + -55)[221] = 0.5069312608764436;
   (acof_1 + -55)[269] = 0.48596938775510207;
   (acof_1 + -55)[317] = 0.16666666666666666;
   (acof_1 + -55)[365] = 0.0;
   (acof_1 + -55)[413] = 0.0;
   (acof_1 + -55)[83] = 0.0;
   (acof_1 + -55)[131] = 0.0;
   (acof_1 + -55)[179] = -0.049461451247165535;
   (acof_1 + -55)[227] = -0.8172197173443021;
   (acof_1 + -55)[275] = -0.735969387755102;
   (acof_1 + -55)[323] = -0.8333333333333334;
   (acof_1 + -55)[371] = -0.041666666666666664;
   (acof_1 + -55)[419] = 0.0;
   (acof_1 + -55)[89] = 0.0;
   (acof_1 + -55)[137] = 0.0;
   (acof_1 + -55)[185] = 0.0;
   (acof_1 + -55)[233] = 0.1608560090702948;
   (acof_1 + -55)[281] = 0.4953231292517007;
   (acof_1 + -55)[329] = 0.5;
   (acof_1 + -55)[377] = 0.16666666666666666;
   (acof_1 + -55)[425] = 0.0;
   (acof_1 + -55)[95] = 0.0;
   (acof_1 + -55)[143] = 0.0;
   (acof_1 + -55)[191] = 0.0;
   (acof_1 + -55)[239] = 0.0;
   (acof_1 + -55)[287] = -0.125;
   (acof_1 + -55)[335] = 0.16666666666666666;
   (acof_1 + -55)[383] = -0.125;
   (acof_1 + -55)[431] = 0.0;
   (acof_1 + -55)[101] = 0.0;
   (acof_1 + -55)[149] = 0.0;
   (acof_1 + -55)[197] = 0.0;
   (acof_1 + -55)[245] = 0.0;
   (acof_1 + -55)[293] = 0.0;
   (acof_1 + -55)[341] = 0.0;
   (acof_1 + -55)[389] = 0.0;
   (acof_1 + -55)[437] = 0.0;
   (acof_1 + -55)[60] = 0.0;
   (acof_1 + -55)[108] = 0.0;
   (acof_1 + -55)[156] = 0.0;
   (acof_1 + -55)[204] = 0.002551020408163265;
   (acof_1 + -55)[252] = 0.0;
   (acof_1 + -55)[300] = 0.0;
   (acof_1 + -55)[348] = 0.0;
   (acof_1 + -55)[396] = 0.0;
   (acof_1 + -55)[66] = 0.0;
   (acof_1 + -55)[114] = 0.0;
   (acof_1 + -55)[162] = 0.0;
   (acof_1 + -55)[210] = -0.006944444444444444;
   (acof_1 + -55)[258] = 0.0;
   (acof_1 + -55)[306] = 0.0;
   (acof_1 + -55)[354] = 0.0;
   (acof_1 + -55)[402] = 0.0;
   (acof_1 + -55)[72] = 0.0;
   (acof_1 + -55)[120] = 0.0;
   (acof_1 + -55)[168] = 0.0;
   (acof_1 + -55)[216] = 0.003826530612244898;
   (acof_1 + -55)[264] = -0.0015589569160997733;
   (acof_1 + -55)[312] = 0.0;
   (acof_1 + -55)[360] = 0.0;
   (acof_1 + -55)[408] = 0.0;
   (acof_1 + -55)[78] = 0.0;
   (acof_1 + -55)[126] = 0.0;
   (acof_1 + -55)[174] = 0.0;
   (acof_1 + -55)[222] = -0.12032312925170068;
   (acof_1 + -55)[270] = 0.171343537414966;
   (acof_1 + -55)[318] = -0.125;
   (acof_1 + -55)[366] = 0.0;
   (acof_1 + -55)[414] = 0.0;
   (acof_1 + -55)[84] = 0.0;
   (acof_1 + -55)[132] = 0.0;
   (acof_1 + -55)[180] = 0.0;
   (acof_1 + -55)[228] = 0.1608560090702948;
   (acof_1 + -55)[276] = 0.4953231292517007;
   (acof_1 + -55)[324] = 0.5;
   (acof_1 + -55)[372] = 0.16666666666666666;
   (acof_1 + -55)[420] = 0.0;
   (acof_1 + -55)[90] = 0.0;
   (acof_1 + -55)[138] = 0.0;
   (acof_1 + -55)[186] = 0.0;
   (acof_1 + -55)[234] = -0.039965986394557826;
   (acof_1 + -55)[282] = -0.8317743764172335;
   (acof_1 + -55)[330] = -0.75;
   (acof_1 + -55)[378] = -0.8333333333333334;
   (acof_1 + -55)[426] = -0.041666666666666664;
   (acof_1 + -55)[96] = 0.0;
   (acof_1 + -55)[144] = 0.0;
   (acof_1 + -55)[192] = 0.0;
   (acof_1 + -55)[240] = 0.0;
   (acof_1 + -55)[288] = 0.16666666666666666;
   (acof_1 + -55)[336] = 0.5;
   (acof_1 + -55)[384] = 0.5;
   (acof_1 + -55)[432] = 0.16666666666666666;
   (acof_1 + -55)[102] = 0.0;
   (acof_1 + -55)[150] = 0.0;
   (acof_1 + -55)[198] = 0.0;
   (acof_1 + -55)[246] = 0.0;
   (acof_1 + -55)[294] = 0.0;
   (acof_1 + -55)[342] = -0.125;
   (acof_1 + -55)[390] = 0.16666666666666666;
   (acof_1 + -55)[438] = -0.125;
   return 0;
}

/*
 * ../src/rhs4sg.c:752.6
 */
void initialize_once(void)
{
   lu = (double (*)[25][25][3])malloc(375000ul);
   u = (double (*)[25][25][3])malloc(375000ul);
   eqs = (double (*)[25][25][3])malloc(375000ul);
   mu = (double (*)[25][25])malloc(125000ul);
   rho = (double (*)[25][25])malloc(125000ul);
   lambda = (double (*)[25][25])malloc(125000ul);
}

/*
 * ../src/rhs4sg.c:769.6
 */
void initialize(void)
{
   double* _t1;
   double* _t2;
   int k;
   _t2 = (double*)bope;
   _t1 = (double*)acof;
   double m_bop2[24];
   double bop[24];
   double m_iop2[5];
   double m_iop[5];
   double m_sbop[5];
   double m_hnorm[4];
   double gh2;
   for (k = 0; k < 25; k++) {
      double _t3;
      int j_1;
      for (_t3 = (double)k * 0.041666666666666664, j_1 = 0; j_1 < 25; j_1++) {
         double (* _t4)[3];
         double* _t5;
         double* _t6;
         double* _t7;
         double _t8;
         int i;
         _t8 = (double)j_1 * 0.041666666666666664;
         i = 0;
         for (_t8 = (double)j_1 * 0.041666666666666664,
              i = 0,
              _t7 = rho[k][j_1],
              _t6 = lambda[k][j_1],
              _t5 = mu[k][j_1],
              u,
              u,
              _t4 = u[k][j_1];
              i < 25; i++) {
            double _t9;
            double* _t10;
            int c;
            _t9 = (double)i * 0.041666666666666664;
            get_data(_t9, _t8, _t3, _t4[i] + 0, _t4[i] + 1, _t4[i] + 2, _t5 + i
               , _t6 + i, _t7 + i);
            fg(_t9, _t8, _t3, eqs[k][j_1][i] + 0);
            c = 0;
            for (c = 0, _t10 = lu[k][j_1][i]; c < 3; c++) {
               _t10[c] = 0.0;
               _t10 = lu[k][j_1][i];
            }
            _t7 = rho[k][j_1];
            _t6 = lambda[k][j_1];
            _t5 = mu[k][j_1];
            u;
            u;
            _t4 = u[k][j_1];
         }
      }
   }
   int i;
   for (i = 0; i < 25; i++) {
      strx[i] = 1.0;
   }
   int j_2;
   for (j_2 = 0; j_2 < 25; j_2++) {
      stry[j_2] = 1.0;
   }
   int k_1;
   for (k_1 = 0; k_1 < 25; k_1++) {
      strz[k_1] = 1.0;
   }
   varcoeffs4_(_t1, ghcof);
   wavepropbop_4__(m_iop, m_iop2, bop, m_bop2, &gh2, m_hnorm, m_sbop);
   bopext4th_(bop, _t2);
}

/*
 * ../src/rhs4sg.c:838.6
 */
void core_fn1(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided_1, double (* a_acof)[8][6], double (* a_bope)[6
   ], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
{
   double _t1;
   int _t2;
   int _t3;
   int _t4;
   int _t5;
   int _t6;
   int _t7;
   int _t8;
   int _t9;
   int _t10;
   int _t11;
   int k;
   for (_t11 = base3 + 3,
        _t10 = base3 + 2,
        _t9 = base3 + 1,
        _t8 = - kfirst,
        _t7 = - jfirst,
        _t6 = - ifirst,
        _t5 = ilast + -2,
        _t3 = jlast + -2,
        _t1 = 1.0 / (h * h),
        _t2 = jfirst + 2,
        _t4 = ifirst + 2,
        k = k1;
        k <= k2; k++) {
      double (* _t12)[25];
      int _t13;
      double (* _t14)[25];
      double* _t15;
      int _t16;
      double (* _t17)[25];
      double* _t18;
      int _t19;
      double (* _t20)[25];
      double* _t21;
      int _t22;
      double (* _t23)[25];
      double* _t24;
      double (* _t25)[25];
      double (* _t26)[25][3];
      double* _t27;
      double (* _t28)[25][3];
      double (* _t29)[25][3];
      double (* _t30)[25][3];
      double (* _t31)[25][3];
      double (* _t32)[25];
      double (* _t33)[25];
      double (* _t34)[25];
      double (* _t35)[25];
      double (* _t36)[25][3];
      int j_3;
      for (_t36 = a_lu[k],
           _t22 = k + 2,
           _t35 = a_lambda[_t22],
           _t19 = k + 1,
           _t34 = a_lambda[_t19],
           _t13 = k + -1,
           _t33 = a_lambda[_t13],
           _t16 = k + -2,
           _t32 = a_lambda[_t16],
           _t31 = a_u[_t22],
           _t30 = a_u[_t19],
           _t29 = a_u[_t13],
           _t28 = a_u[_t16],
           _t27 = a_strz + (k + _t8),
           _t26 = a_u[k],
           _t25 = a_lambda[k],
           _t24 = a_strz + (_t22 + _t8),
           _t23 = a_mu[_t22],
           _t21 = a_strz + (_t19 + _t8),
           _t20 = a_mu[_t19],
           _t18 = a_strz + (_t16 + _t8),
           _t17 = a_mu[_t16],
           _t15 = a_strz + (_t13 + _t8),
           _t14 = a_mu[_t13],
           _t12 = a_mu[k],
           j_3 = _t2;
           j_3 <= _t3; j_3++) {
         double* _t37;
         int _t38;
         double* _t39;
         double* _t40;
         int _t41;
         double* _t42;
         double* _t43;
         int _t44;
         double* _t45;
         double* _t46;
         int _t47;
         double* _t48;
         double* _t49;
         double* _t50;
         double* _t51;
         double* _t52;
         double* _t53;
         double* _t54;
         double (* _t55)[3];
         double* _t56;
         double (* _t57)[3];
         double (* _t58)[3];
         double (* _t59)[3];
         double (* _t60)[3];
         double (* _t61)[3];
         double (* _t62)[3];
         double (* _t63)[3];
         double (* _t64)[3];
         double* _t65;
         double* _t66;
         double* _t67;
         double* _t68;
         double* _t69;
         double (* _t70)[3];
         double (* _t71)[3];
         double (* _t72)[3];
         double (* _t73)[3];
         double* _t74;
         double (* _t75)[3];
         double (* _t76)[3];
         double (* _t77)[3];
         double (* _t78)[3];
         double* _t79;
         double (* _t80)[3];
         double (* _t81)[3];
         double (* _t82)[3];
         double (* _t83)[3];
         double* _t84;
         double (* _t85)[3];
         double (* _t86)[3];
         double (* _t87)[3];
         double (* _t88)[3];
         double (* _t89)[3];
         int i;
         for (_t89 = _t36[j_3],
              _t44 = j_3 + 1,
              _t88 = _t31[_t44],
              _t38 = j_3 + -1,
              _t87 = _t31[_t38],
              _t47 = j_3 + 2,
              _t86 = _t31[_t47],
              _t41 = j_3 + -2,
              _t85 = _t31[_t41],
              _t84 = _t35[j_3],
              _t83 = _t30[_t44],
              _t82 = _t30[_t38],
              _t81 = _t30[_t47],
              _t80 = _t30[_t41],
              _t79 = _t34[j_3],
              _t78 = _t29[_t44],
              _t77 = _t29[_t38],
              _t76 = _t29[_t47],
              _t75 = _t29[_t41],
              _t74 = _t33[j_3],
              _t73 = _t28[_t44],
              _t72 = _t28[_t38],
              _t71 = _t28[_t47],
              _t70 = _t28[_t41],
              _t69 = _t32[j_3],
              _t68 = _t25[_t47],
              _t67 = _t25[_t44],
              _t66 = _t25[_t38],
              _t65 = _t25[_t41],
              _t64 = _t31[j_3],
              _t63 = _t30[j_3],
              _t62 = _t29[j_3],
              _t61 = _t28[j_3],
              _t60 = _t26[_t47],
              _t59 = _t26[_t44],
              _t58 = _t26[_t38],
              _t57 = _t26[_t41],
              _t56 = a_stry + (j_3 + _t7),
              _t55 = _t26[j_3],
              _t54 = _t25[j_3],
              _t53 = _t23[j_3],
              _t52 = _t20[j_3],
              _t51 = _t17[j_3],
              _t50 = _t14[j_3],
              _t49 = a_stry + (_t47 + _t7),
              _t48 = _t12[_t47],
              _t46 = a_stry + (_t44 + _t7),
              _t45 = _t12[_t44],
              _t43 = a_stry + (_t41 + _t7),
              _t42 = _t12[_t41],
              _t40 = a_stry + (_t38 + _t7),
              _t39 = _t12[_t38],
              _t37 = _t12[j_3],
              i = _t4;
              i <= _t5; ) {
            int _t90;
            double _t91;
            int _t92;
            double _t93;
            double _t94;
            double _t95;
            double _t96;
            int _t97;
            double _t98;
            int _t99;
            double _t100;
            double _t101;
            double _t102;
            double _t103;
            double _t104;
            double _t105;
            double _t106;
            int _t107;
            double _t108;
            int _t109;
            double _t110;
            double _t111;
            double _t112;
            double _t113;
            double _t114;
            double _t115;
            double _t116;
            double _t117;
            double _t118;
            double _t119;
            double _t120;
            double _t121;
            double _t122;
            double _t123;
            double _t124;
            double _t125;
            double _t126;
            double _t127;
            double _t128;
            double _t129;
            double _t130;
            double _t131;
            double _t132;
            double _t133;
            double _t134;
            double _t135;
            double _t136;
            double _t137;
            double _t138;
            double _t139;
            double _t140;
            double _t141;
            double _t142;
            double _t143;
            double _t144;
            double _t145;
            double _t146;
            double _t147;
            double _t148;
            double _t149;
            double _t150;
            double _t151;
            double _t152;
            double _t153;
            double _t154;
            double _t155;
            double _t156;
            double _t157;
            double _t158;
            double _t159;
            double _t160;
            double _t161;
            double _t162;
            double _t163;
            double _t164;
            double _t165;
            double _t166;
            double _t167;
            double _t168;
            double _t169;
            double _t170;
            double _t171;
            double _t172;
            double _t173;
            double _t174;
            double _t175;
            double _t176;
            double _t177;
            double _t178;
            double _t179;
            double _t180;
            double _t181;
            double _t182;
            double _t183;
            double _t184;
            double _t185;
            double _t186;
            double _t187;
            double _t188;
            double _t189;
            double _t190;
            double _t191;
            double _t192;
            double _t193;
            double _t194;
            double _t195;
            double _t196;
            double _t197;
            double _t198;
            double _t199;
            double _t200;
            double _t201;
            double _t202;
            double _t203;
            double _t204;
            double _t205;
            double _t206;
            double _t207;
            double _t208;
            double _t209;
            double _t210;
            double _t211;
            double _t212;
            double _t213;
            double _t214;
            double _t215;
            double _t216;
            double _t217;
            double _t218;
            double _t219;
            double _t220;
            double _t221;
            double _t222;
            double _t223;
            double _t224;
            double _t225;
            double _t226;
            double _t227;
            double _t228;
            double _t229;
            double _t230;
            double _t231;
            double _t232;
            int _t233;
            int _t234;
            double _t235;
            double _t236;
            double _t237;
            double _t238;
            double _t239;
            double _t240;
            double _t241;
            double _t242;
            double _t243;
            double _t244;
            double _t245;
            double _t246;
            double _t247;
            double _t248;
            double _t249;
            double _t250;
            double _t251;
            double _t252;
            double _t253;
            double _t254;
            double _t255;
            double _t256;
            double _t257;
            double _t258;
            double _t259;
            double _t260;
            double _t261;
            double _t262;
            double _t263;
            double _t264;
            double _t265;
            double _t266;
            double _t267;
            double _t268;
            double _t269;
            double _t270;
            double _t271;
            double _t272;
            double _t273;
            double _t274;
            double _t275;
            double _t276;
            double _t277;
            double _t278;
            double _t279;
            double _t280;
            double _t281;
            double _t282;
            double _t283;
            double _t284;
            double _t285;
            double _t286;
            double _t287;
            double _t288;
            double _t289;
            double _t290;
            double _t291;
            double _t292;
            double _t293;
            double _t294;
            double _t295;
            double _t296;
            double _t297;
            double _t298;
            double _t299;
            double _t300;
            double _t301;
            double _t302;
            double _t303;
            double _t304;
            double _t305;
            double _t306;
            double _t307;
            double _t308;
            double _t309;
            double _t310;
            int _t311;
            _t233 = base + i;
            _t90 = _t233 + -1;
            _t91 = _t37[_t90];
            _t92 = i + -1;
            _t93 = a_strx[_t92 + _t6];
            _t95 = _t37[_t233];
            a_strx[i + _t6];
            _t97 = _t233 + -2;
            _t98 = _t37[_t97];
            _t99 = i + -2;
            _t100 = a_strx[_t99 + _t6];
            _t37[_t97];
            a_strx[_t99 + _t6];
            _t234 = _t233 + 1;
            _t103 = _t37[_t234];
            _t311 = i + 1;
            _t104 = a_strx[_t311 + _t6];
            _t37[_t233];
            a_strx[i + _t6];
            _t37[_t90];
            a_strx[_t92 + _t6];
            _t37[_t90];
            a_strx[_t92 + _t6];
            _t107 = _t233 + 2;
            _t108 = _t37[_t107];
            _t109 = i + 2;
            _t110 = a_strx[_t109 + _t6];
            _t37[_t234];
            a_strx[_t311 + _t6];
            _t37[_t233];
            a_strx[i + _t6];
            _t37[_t234];
            a_strx[_t311 + _t6];
            _t37[_t233];
            a_strx[i + _t6];
            _t37[_t107];
            a_strx[_t109 + _t6];
            _t114 = _t39[_t233];
            _t115 = *_t40;
            _t37[_t233];
            *_t56;
            _t118 = _t42[_t233];
            _t119 = *_t43;
            _t42[_t233];
            *_t43;
            _t122 = _t45[_t233];
            _t123 = *_t46;
            _t37[_t233];
            *_t56;
            _t39[_t233];
            *_t40;
            _t39[_t233];
            *_t40;
            _t126 = _t48[_t233];
            _t127 = *_t49;
            _t45[_t233];
            *_t46;
            _t37[_t233];
            *_t56;
            _t45[_t233];
            *_t46;
            _t37[_t233];
            *_t56;
            _t48[_t233];
            *_t49;
            _t131 = _t50[_t233];
            _t132 = *_t15;
            _t37[_t233];
            *_t27;
            _t135 = _t51[_t233];
            _t136 = *_t18;
            _t51[_t233];
            *_t18;
            _t139 = _t52[_t233];
            _t140 = *_t21;
            _t37[_t233];
            *_t27;
            _t50[_t233];
            *_t15;
            _t50[_t233];
            *_t15;
            _t143 = _t53[_t233];
            _t144 = *_t24;
            _t52[_t233];
            *_t21;
            _t37[_t233];
            *_t27;
            _t52[_t233];
            *_t21;
            _t37[_t233];
            *_t27;
            _t53[_t233];
            *_t24;
            _t148 = a_strx[i + _t6];
            _t149 = _t54[_t90];
            a_strx[_t92 + _t6];
            _t151 = _t54[_t233];
            a_strx[i + _t6];
            _t153 = _t54[_t97];
            a_strx[_t99 + _t6];
            _t155 = _t55[i][_t9];
            _t54[_t97];
            a_strx[_t99 + _t6];
            _t156 = _t54[_t234];
            a_strx[_t311 + _t6];
            _t54[_t233];
            a_strx[i + _t6];
            _t54[_t90];
            a_strx[_t92 + _t6];
            _t55[i][_t9];
            _t54[_t90];
            a_strx[_t92 + _t6];
            _t158 = _t54[_t107];
            a_strx[_t109 + _t6];
            _t54[_t234];
            a_strx[_t311 + _t6];
            _t54[_t233];
            a_strx[i + _t6];
            _t55[i][_t9];
            _t54[_t234];
            a_strx[_t311 + _t6];
            _t54[_t233];
            a_strx[i + _t6];
            _t54[_t107];
            a_strx[_t109 + _t6];
            _t55[i][_t9];
            _t160 = *_t56;
            _t55[i][_t9];
            _t55[i][_t9];
            _t55[i][_t9];
            _t55[i][_t9];
            _t161 = *_t27;
            _t55[i][_t9];
            _t55[i][_t9];
            _t55[i][_t9];
            _t55[i][_t9];
            a_strx[i + _t6];
            _t162 = _t55[_t99][_t10];
            _t55[i][_t10];
            _t163 = _t55[_t92][_t10];
            _t55[i][_t10];
            _t164 = _t55[_t311][_t10];
            _t55[i][_t10];
            _t165 = _t55[_t109][_t10];
            _t55[i][_t10];
            *_t56;
            _t66[_t233];
            *_t40;
            _t54[_t233];
            *_t56;
            _t65[_t233];
            *_t43;
            _t169 = _t57[i][_t10];
            _t55[i][_t10];
            _t65[_t233];
            *_t43;
            _t67[_t233];
            *_t46;
            _t54[_t233];
            *_t56;
            _t66[_t233];
            *_t40;
            _t171 = _t58[i][_t10];
            _t55[i][_t10];
            _t66[_t233];
            *_t40;
            _t68[_t233];
            *_t49;
            _t67[_t233];
            *_t46;
            _t54[_t233];
            *_t56;
            _t173 = _t59[i][_t10];
            _t55[i][_t10];
            _t67[_t233];
            *_t46;
            _t54[_t233];
            *_t56;
            _t68[_t233];
            *_t49;
            _t174 = _t60[i][_t10];
            _t55[i][_t10];
            *_t27;
            _t175 = _t61[i][_t10];
            _t55[i][_t10];
            _t176 = _t62[i][_t10];
            _t55[i][_t10];
            _t177 = _t63[i][_t10];
            _t55[i][_t10];
            _t178 = _t64[i][_t10];
            _t179 = _t55[i][_t10];
            a_strx[i + _t6];
            _t180 = _t55[_t99][_t11];
            _t55[i][_t11];
            _t181 = _t55[_t92][_t11];
            _t55[i][_t11];
            _t182 = _t55[_t311][_t11];
            _t55[i][_t11];
            _t183 = _t55[_t109][_t11];
            _t55[i][_t11];
            *_t56;
            _t184 = _t57[i][_t11];
            _t55[i][_t11];
            _t185 = _t58[i][_t11];
            _t55[i][_t11];
            _t186 = _t59[i][_t11];
            _t55[i][_t11];
            _t187 = _t60[i][_t11];
            _t55[i][_t11];
            *_t27;
            _t74[_t233];
            *_t15;
            _t54[_t233];
            *_t27;
            _t69[_t233];
            *_t18;
            _t191 = _t61[i][_t11];
            _t55[i][_t11];
            _t69[_t233];
            *_t18;
            _t79[_t233];
            *_t21;
            _t54[_t233];
            *_t27;
            _t74[_t233];
            *_t15;
            _t193 = _t62[i][_t11];
            _t55[i][_t11];
            _t74[_t233];
            *_t15;
            _t84[_t233];
            *_t24;
            _t79[_t233];
            *_t21;
            _t54[_t233];
            *_t27;
            _t195 = _t63[i][_t11];
            _t55[i][_t11];
            _t79[_t233];
            *_t21;
            _t54[_t233];
            *_t27;
            _t84[_t233];
            *_t24;
            _t196 = _t64[i][_t11];
            _t197 = _t55[i][_t11];
            a_strx[i + _t6];
            *_t56;
            _t54[_t97];
            _t199 = _t57[_t99][_t10];
            _t200 = _t60[_t99][_t10];
            _t201 = _t58[_t99][_t10];
            _t202 = _t59[_t99][_t10];
            _t54[_t90];
            _t203 = _t57[_t92][_t10];
            _t204 = _t60[_t92][_t10];
            _t206 = _t59[_t92][_t10];
            _t54[_t234];
            _t207 = _t57[_t311][_t10];
            _t208 = _t60[_t311][_t10];
            _t209 = _t58[_t311][_t10];
            _t210 = _t59[_t311][_t10];
            _t54[_t107];
            _t211 = _t57[_t109][_t10];
            _t212 = _t60[_t109][_t10];
            _t213 = _t58[_t109][_t10];
            _t214 = _t59[_t109][_t10];
            a_strx[i + _t6];
            *_t27;
            _t54[_t97];
            _t216 = _t61[_t99][_t11];
            _t217 = _t64[_t99][_t11];
            _t218 = _t62[_t99][_t11];
            _t219 = _t63[_t99][_t11];
            _t54[_t90];
            _t220 = _t61[_t92][_t11];
            _t221 = _t64[_t92][_t11];
            _t223 = _t63[_t92][_t11];
            _t54[_t234];
            _t224 = _t61[_t311][_t11];
            _t225 = _t64[_t311][_t11];
            _t226 = _t62[_t311][_t11];
            _t227 = _t63[_t311][_t11];
            _t54[_t107];
            _t228 = _t61[_t109][_t11];
            _t229 = _t64[_t109][_t11];
            _t230 = _t62[_t109][_t11];
            _t231 = _t63[_t109][_t11];
            a_strx[i + _t6];
            *_t56;
            _t42[_t233];
            _t57[_t99][_t10];
            _t57[_t109][_t10];
            _t57[_t92][_t10];
            _t57[_t311][_t10];
            _t39[_t233];
            _t58[_t99][_t10];
            _t58[_t109][_t10];
            _t58[_t92][_t10];
            _t58[_t311][_t10];
            _t45[_t233];
            _t59[_t99][_t10];
            _t59[_t109][_t10];
            _t59[_t92][_t10];
            _t59[_t311][_t10];
            _t48[_t233];
            _t60[_t99][_t10];
            _t60[_t109][_t10];
            _t60[_t92][_t10];
            _t60[_t311][_t10];
            a_strx[i + _t6];
            *_t27;
            _t51[_t233];
            _t61[_t99][_t11];
            _t61[_t109][_t11];
            _t61[_t92][_t11];
            _t61[_t311][_t11];
            _t50[_t233];
            _t62[_t99][_t11];
            _t62[_t109][_t11];
            _t62[_t92][_t11];
            _t62[_t311][_t11];
            _t52[_t233];
            _t63[_t99][_t11];
            _t63[_t109][_t11];
            _t63[_t92][_t11];
            _t63[_t311][_t11];
            _t53[_t233];
            _t64[_t99][_t11];
            _t64[_t109][_t11];
            _t64[_t92][_t11];
            _t64[_t311][_t11];
            a_strx[i + _t6];
            *_t56;
            _t37[_t97];
            _t57[_t99][_t9];
            _t60[_t99][_t9];
            _t58[_t99][_t9];
            _t59[_t99][_t9];
            _t37[_t90];
            _t57[_t92][_t9];
            _t60[_t92][_t9];
            _t58[_t92][_t9];
            _t59[_t92][_t9];
            _t37[_t234];
            _t57[_t311][_t9];
            _t60[_t311][_t9];
            _t58[_t311][_t9];
            _t59[_t311][_t9];
            _t37[_t107];
            _t57[_t109][_t9];
            _t60[_t109][_t9];
            _t58[_t109][_t9];
            _t59[_t109][_t9];
            a_strx[i + _t6];
            *_t56;
            _t65[_t233];
            _t235 = _t57[_t99][_t9];
            _t236 = _t57[_t109][_t9];
            _t237 = _t57[_t92][_t9];
            _t238 = _t57[_t311][_t9];
            _t66[_t233];
            _t239 = _t58[_t99][_t9];
            _t240 = _t58[_t109][_t9];
            _t241 = _t58[_t92][_t9];
            _t242 = _t58[_t311][_t9];
            _t67[_t233];
            _t243 = _t59[_t99][_t9];
            _t244 = _t59[_t109][_t9];
            _t245 = _t59[_t92][_t9];
            _t246 = _t59[_t311][_t9];
            _t68[_t233];
            _t247 = _t60[_t99][_t9];
            _t248 = _t60[_t109][_t9];
            _t249 = _t60[_t92][_t9];
            _t250 = _t60[_t311][_t9];
            *_t56;
            *_t27;
            _t252 = _t65[_t233];
            _t70[i][_t11];
            _t85[i][_t11];
            _t75[i][_t11];
            _t80[i][_t11];
            _t253 = _t66[_t233];
            _t72[i][_t11];
            _t87[i][_t11];
            _t77[i][_t11];
            _t82[i][_t11];
            _t255 = _t67[_t233];
            _t73[i][_t11];
            _t88[i][_t11];
            _t78[i][_t11];
            _t83[i][_t11];
            _t256 = _t68[_t233];
            _t71[i][_t11];
            _t86[i][_t11];
            _t76[i][_t11];
            _t81[i][_t11];
            *_t56;
            *_t27;
            _t51[_t233];
            _t257 = _t70[i][_t11];
            _t258 = _t71[i][_t11];
            _t259 = _t72[i][_t11];
            _t260 = _t73[i][_t11];
            _t50[_t233];
            _t261 = _t75[i][_t11];
            _t262 = _t76[i][_t11];
            _t263 = _t77[i][_t11];
            _t264 = _t78[i][_t11];
            _t52[_t233];
            _t265 = _t80[i][_t11];
            _t266 = _t81[i][_t11];
            _t267 = _t82[i][_t11];
            _t268 = _t83[i][_t11];
            _t53[_t233];
            _t269 = _t85[i][_t11];
            _t270 = _t86[i][_t11];
            _t271 = _t87[i][_t11];
            _t272 = _t88[i][_t11];
            a_strx[i + _t6];
            *_t27;
            _t37[_t97];
            _t61[_t99][_t9];
            _t64[_t99][_t9];
            _t62[_t99][_t9];
            _t63[_t99][_t9];
            _t37[_t90];
            _t61[_t92][_t9];
            _t64[_t92][_t9];
            _t62[_t92][_t9];
            _t63[_t92][_t9];
            _t37[_t234];
            _t61[_t311][_t9];
            _t64[_t311][_t9];
            _t62[_t311][_t9];
            _t63[_t311][_t9];
            _t37[_t107];
            _t61[_t109][_t9];
            _t64[_t109][_t9];
            _t62[_t109][_t9];
            _t63[_t109][_t9];
            *_t56;
            *_t27;
            _t42[_t233];
            _t70[i][_t10];
            _t85[i][_t10];
            _t75[i][_t10];
            _t80[i][_t10];
            _t39[_t233];
            _t72[i][_t10];
            _t87[i][_t10];
            _t77[i][_t10];
            _t82[i][_t10];
            _t45[_t233];
            _t73[i][_t10];
            _t88[i][_t10];
            _t78[i][_t10];
            _t83[i][_t10];
            _t48[_t233];
            _t71[i][_t10];
            _t86[i][_t10];
            _t76[i][_t10];
            _t81[i][_t10];
            a_strx[i + _t6];
            *_t27;
            _t69[_t233];
            _t275 = _t61[_t99][_t9];
            _t276 = _t61[_t109][_t9];
            _t277 = _t61[_t92][_t9];
            _t278 = _t61[_t311][_t9];
            _t74[_t233];
            _t279 = _t62[_t99][_t9];
            _t280 = _t62[_t109][_t9];
            _t281 = _t62[_t92][_t9];
            _t282 = _t62[_t311][_t9];
            _t79[_t233];
            _t283 = _t63[_t99][_t9];
            _t284 = _t63[_t109][_t9];
            _t285 = _t63[_t92][_t9];
            _t286 = _t63[_t311][_t9];
            _t84[_t233];
            _t287 = _t64[_t99][_t9];
            _t288 = _t64[_t109][_t9];
            _t289 = _t64[_t92][_t9];
            _t290 = _t64[_t311][_t9];
            *_t56;
            *_t27;
            _t291 = _t69[_t233];
            _t292 = _t70[i][_t10];
            _t293 = _t71[i][_t10];
            _t294 = _t72[i][_t10];
            _t295 = _t73[i][_t10];
            _t296 = _t74[_t233];
            _t297 = _t75[i][_t10];
            _t298 = _t76[i][_t10];
            _t299 = _t77[i][_t10];
            _t300 = _t78[i][_t10];
            _t301 = _t79[_t233];
            _t302 = _t80[i][_t10];
            _t303 = _t81[i][_t10];
            _t304 = _t82[i][_t10];
            _t305 = _t83[i][_t10];
            _t306 = _t84[_t233];
            _t307 = _t85[i][_t10];
            _t308 = _t86[i][_t10];
            _t309 = _t87[i][_t10];
            _t310 = _t88[i][_t10];
            _t222 = - _t62[_t92][_t11];
            _t215 = _t148 * _t161 * 0.006944444444444444;
            _t205 = - _t58[_t92][_t10];
            _t198 = _t148 * _t160 * 0.006944444444444444;
            _t145 = _t143 * _t144;
            _t134 = _t95 * _t161;
            _t141 = _t139 * _t140;
            _t147 = _t141 - 0.75 * (_t134 + _t145);
            _t133 = _t131 * _t132;
            _t146 = _t133 + _t145 + 3.0 * (_t141 + _t134);
            _t137 = _t135 * _t136;
            _t142 = _t137 + _t141 + 3.0 * (_t134 + _t133);
            _t138 = _t133 - 0.75 * (_t134 + _t137);
            _t128 = _t126 * _t127;
            _t117 = _t95 * _t160;
            _t124 = _t122 * _t123;
            _t130 = _t124 - 0.75 * (_t117 + _t128);
            _t116 = _t114 * _t115;
            _t129 = _t116 + _t128 + 3.0 * (_t124 + _t117);
            _t120 = _t118 * _t119;
            _t125 = _t120 + _t124 + 3.0 * (_t117 + _t116);
            _t121 = _t116 - 0.75 * (_t117 + _t120);
            _t159 = _t158 * _t110;
            _t152 = _t151 * _t148;
            _t157 = _t156 * _t104;
            _t111 = _t108 * _t110;
            _t96 = _t95 * _t148;
            _t105 = _t103 * _t104;
            _t113 = _t105 - 0.75 * (_t96 + _t111);
            _t150 = _t149 * _t93;
            _t94 = _t91 * _t93;
            _t112 = _t94 + _t111 + 3.0 * (_t105 + _t96);
            _t154 = _t153 * _t100;
            _t101 = _t98 * _t100;
            _t106 = _t101 + _t105 + 3.0 * (_t96 + _t94);
            _t102 = _t94 - 0.75 * (_t96 + _t101);
            _t89[i][_t9] = 0.0 * _t89[i][_t9] + _t1 * (0.16666666666666666 * (
               _t148 * ((2.0 * _t102 + _t150 - 0.75 * (_t152 + _t154)) * (_t55[
               _t99][_t9] - _t155) + (2.0 * _t106 + _t154 + _t157 + 3.0 * (
               _t152 + _t150)) * (_t55[_t92][_t9] - _t155) + (2.0 * _t112 +
               _t150 + _t159 + 3.0 * (_t157 + _t152)) * (_t55[_t311][_t9] -
               _t155) + (2.0 * _t113 + _t157 - 0.75 * (_t152 + _t159)) * (_t55[
               _t109][_t9] - _t155)) + _t160 * (_t121 * (_t57[i][_t9] - _t155) +
               _t125 * (_t58[i][_t9] - _t155) + _t129 * (_t59[i][_t9] - _t155) +
               _t130 * (_t60[i][_t9] - _t155)) + _t161 * (_t138 * (_t61[i][_t9]
               - _t155) + _t142 * (_t62[i][_t9] - _t155) + _t146 * (_t63[i][_t9
               ] - _t155) + _t147 * (_t64[i][_t9] - _t155))) + _t198 * (_t153 *
               (_t199 - _t200 + 8.0 * (- _t201 + _t202)) - 8.0 * (_t149 * (
               _t203 - _t204 + 8.0 * (_t205 + _t206))) + 8.0 * (_t156 * (_t207 -
               _t208 + 8.0 * (- _t209 + _t210))) - _t158 * (_t211 - _t212 + 8.0
               * (- _t213 + _t214))) + _t215 * (_t153 * (_t216 - _t217 + 8.0 * (
               - _t218 + _t219)) - 8.0 * (_t149 * (_t220 - _t221 + 8.0 * (_t222
               + _t223))) + 8.0 * (_t156 * (_t224 - _t225 + 8.0 * (- _t226 +
               _t227))) - _t158 * (_t228 - _t229 + 8.0 * (- _t230 + _t231))) +
               _t198 * (_t118 * (_t199 - _t211 + 8.0 * (- _t203 + _t207)) - 8.0
               * (_t114 * (_t201 - _t213 + 8.0 * (_t205 + _t209))) + 8.0 * (
               _t122 * (_t202 - _t214 + 8.0 * (- _t206 + _t210))) - _t126 * (
               _t200 - _t212 + 8.0 * (- _t204 + _t208))) + _t215 * (_t135 * (
               _t216 - _t228 + 8.0 * (- _t220 + _t224)) - 8.0 * (_t131 * (_t218
               - _t230 + 8.0 * (_t222 + _t226))) + 8.0 * (_t139 * (_t219 - _t231
               + 8.0 * (- _t223 + _t227))) - _t143 * (_t217 - _t229 + 8.0 * (-
               _t221 + _t225))));
            _t254 = - _t263;
            _t251 = _t160 * _t161 * 0.006944444444444444;
            _t232 = - _t241;
            _t172 = _t256 * _t127;
            _t167 = _t151 * _t160;
            _t170 = _t255 * _t123;
            _t166 = _t253 * _t115;
            _t168 = _t252 * _t119;
            _t89[i][_t10] = 0.0 * _t89[i][_t10] + _t1 * (0.16666666666666666 * (
               _t148 * (_t102 * (_t162 - _t179) + _t106 * (_t163 - _t179) +
               _t112 * (_t164 - _t179) + _t113 * (_t165 - _t179)) + _t160 * ((
               2.0 * _t121 + _t166 - 0.75 * (_t167 + _t168)) * (_t169 - _t179) +
               (2.0 * _t125 + _t168 + _t170 + 3.0 * (_t167 + _t166)) * (_t171 -
               _t179) + (2.0 * _t129 + _t166 + _t172 + 3.0 * (_t170 + _t167)) *
               (_t173 - _t179) + (2.0 * _t130 + _t170 - 0.75 * (_t167 + _t172))
               * (_t174 - _t179)) + _t161 * (_t138 * (_t175 - _t179) + _t142 * (
               _t176 - _t179) + _t146 * (_t177 - _t179) + _t147 * (_t178 - _t179
               ))) + _t198 * (_t98 * (_t235 - _t247 + 8.0 * (- _t239 + _t243)) -
               8.0 * (_t91 * (_t237 - _t249 + 8.0 * (_t232 + _t245))) + 8.0 * (
               _t103 * (_t238 - _t250 + 8.0 * (- _t242 + _t246))) - _t108 * (
               _t236 - _t248 + 8.0 * (- _t240 + _t244))) + _t198 * (_t252 * (
               _t235 - _t236 + 8.0 * (- _t237 + _t238)) - 8.0 * (_t253 * (_t239
               - _t240 + 8.0 * (_t232 + _t242))) + 8.0 * (_t255 * (_t243 - _t244
               + 8.0 * (- _t245 + _t246))) - _t256 * (_t247 - _t248 + 8.0 * (-
               _t249 + _t250))) + _t251 * (_t252 * (_t257 - _t269 + 8.0 * (-
               _t261 + _t265)) - 8.0 * (_t253 * (_t259 - _t271 + 8.0 * (_t254 +
               _t267))) + 8.0 * (_t255 * (_t260 - _t272 + 8.0 * (- _t264 + _t268
               ))) - _t256 * (_t258 - _t270 + 8.0 * (- _t262 + _t266))) + _t251
               * (_t135 * (_t257 - _t258 + 8.0 * (- _t259 + _t260)) - 8.0 * (
               _t131 * (_t261 - _t262 + 8.0 * (_t254 + _t264))) + 8.0 * (_t139 *
               (_t265 - _t266 + 8.0 * (- _t267 + _t268))) - _t143 * (_t269 -
               _t270 + 8.0 * (- _t271 + _t272))));
            _t274 = - _t299;
            _t273 = - _t281;
            _t194 = _t306 * _t144;
            _t189 = _t151 * _t161;
            _t192 = _t301 * _t140;
            _t188 = _t296 * _t132;
            _t190 = _t291 * _t136;
            _t89[i][_t11] = 0.0 * _t89[i][_t11] + _t1 * (0.16666666666666666 * (
               _t148 * (_t102 * (_t180 - _t197) + _t106 * (_t181 - _t197) +
               _t112 * (_t182 - _t197) + _t113 * (_t183 - _t197)) + _t160 * (
               _t121 * (_t184 - _t197) + _t125 * (_t185 - _t197) + _t129 * (
               _t186 - _t197) + _t130 * (_t187 - _t197)) + _t161 * ((2.0 * _t138
               + _t188 - 0.75 * (_t189 + _t190)) * (_t191 - _t197) + (2.0 *
               _t142 + _t190 + _t192 + 3.0 * (_t189 + _t188)) * (_t193 - _t197)
               + (2.0 * _t146 + _t188 + _t194 + 3.0 * (_t192 + _t189)) * (_t195
               - _t197) + (2.0 * _t147 + _t192 - 0.75 * (_t189 + _t194)) * (
               _t196 - _t197))) + _t215 * (_t98 * (_t275 - _t287 + 8.0 * (-
               _t279 + _t283)) - 8.0 * (_t91 * (_t277 - _t289 + 8.0 * (_t273 +
               _t285))) + 8.0 * (_t103 * (_t278 - _t290 + 8.0 * (- _t282 + _t286
               ))) - _t108 * (_t276 - _t288 + 8.0 * (- _t280 + _t284))) + _t251
               * (_t118 * (_t292 - _t307 + 8.0 * (- _t297 + _t302)) - 8.0 * (
               _t114 * (_t294 - _t309 + 8.0 * (_t274 + _t304))) + 8.0 * (_t122 *
               (_t295 - _t310 + 8.0 * (- _t300 + _t305))) - _t126 * (_t293 -
               _t308 + 8.0 * (- _t298 + _t303))) + _t215 * (_t291 * (_t275 -
               _t276 + 8.0 * (- _t277 + _t278)) - 8.0 * (_t296 * (_t279 - _t280
               + 8.0 * (_t273 + _t282))) + 8.0 * (_t301 * (_t283 - _t284 + 8.0 *
               (- _t285 + _t286))) - _t306 * (_t287 - _t288 + 8.0 * (- _t289 +
               _t290))) + _t251 * (_t291 * (_t292 - _t293 + 8.0 * (- _t294 +
               _t295)) - 8.0 * (_t296 * (_t297 - _t298 + 8.0 * (_t274 + _t300))
               ) + 8.0 * (_t301 * (_t302 - _t303 + 8.0 * (- _t304 + _t305))) -
               _t306 * (_t307 - _t308 + 8.0 * (- _t309 + _t310))));
            i = _t311;
         }
      }
   }
}

/*
 * ../src/rhs4sg.c:1095.6
 */
void core_fn2(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided_2, double (* a_acof)[8][6], double (* a_bope)[6
   ], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
{
   int _t1;
   int _t2;
   int _t3;
   int _t4;
   int _t5;
   int _t6;
   int _t7;
   int _t8;
   int _t9;
   double (* _t10)[25];
   double (* _t11)[25][3];
   int _t12;
   double (* _t13)[25];
   int _t14;
   double cof;
   int k;
   for (_t14 = base3 + 2,
        _t13 = a_lambda[1],
        _t12 = base3 + 3,
        _t11 = a_u[0],
        _t10 = a_mu[1],
        _t9 = base3 + 3,
        _t8 = base3 + 2,
        _t7 = - jfirst,
        _t6 = base3 + 1,
        _t5 = - ifirst,
        _t4 = ilast + -2,
        _t2 = jlast + -2,
        _t1 = jfirst + 2,
        _t3 = ifirst + 2,
        k = 1;
        k <= 6; k++) {
      double (* _t15)[25];
      double (* _t16)[25];
      double (* _t17)[25][3];
      int _t18;
      int _t19;
      double* _t20;
      double (* _t21)[25][3];
      int j_4;
      for (_t21 = a_lu[k],
           _t19 = k + -1,
           _t20 = a_ghcof + _t19,
           _t18 = k + -1,
           _t17 = a_u[k],
           _t16 = a_lambda[k],
           _t15 = a_mu[k],
           j_4 = _t1;
           j_4 <= _t2; j_4++) {
         double* _t22;
         int _t23;
         double* _t24;
         double* _t25;
         int _t26;
         double* _t27;
         double* _t28;
         int _t29;
         double* _t30;
         double* _t31;
         int _t32;
         double* _t33;
         double* _t34;
         double* _t35;
         double (* _t36)[3];
         double* _t37;
         double (* _t38)[3];
         double (* _t39)[3];
         double (* _t40)[3];
         double (* _t41)[3];
         double* _t42;
         double (* _t43)[3];
         double* _t44;
         double* _t45;
         double* _t46;
         double* _t47;
         double* _t48;
         double (* _t49)[3];
         int i;
         for (_t49 = _t21[j_4],
              _t48 = _t13[j_4],
              _t32 = j_4 + 2,
              _t47 = _t16[_t32],
              _t29 = j_4 + 1,
              _t46 = _t16[_t29],
              _t26 = j_4 + -2,
              _t45 = _t16[_t26],
              _t23 = j_4 + -1,
              _t44 = _t16[_t23],
              _t43 = _t11[j_4],
              _t42 = _t10[j_4],
              _t41 = _t17[_t32],
              _t40 = _t17[_t29],
              _t39 = _t17[_t23],
              _t38 = _t17[_t26],
              _t37 = a_stry + (j_4 + _t7),
              _t36 = _t17[j_4],
              _t35 = _t16[j_4],
              _t34 = a_stry + (_t32 + _t7),
              _t33 = _t15[_t32],
              _t31 = a_stry + (_t29 + _t7),
              _t30 = _t15[_t29],
              _t28 = a_stry + (_t26 + _t7),
              _t27 = _t15[_t26],
              _t25 = a_stry + (_t23 + _t7),
              _t24 = _t15[_t23],
              _t22 = _t15[j_4],
              i = _t3;
              i <= _t4; i++) {
            int _t50;
            int _t51;
            double _t52;
            int _t53;
            double _t54;
            double _t55;
            double _t56;
            double _t57;
            double _t58;
            int _t59;
            double _t60;
            double _t61;
            double _t62;
            double _t63;
            int _t64;
            double _t65;
            int _t66;
            double _t67;
            double _t68;
            double _t69;
            int _t70;
            double _t71;
            int _t72;
            double _t73;
            double _t74;
            double _t75;
            double _t76;
            double _t77;
            double _t78;
            double _t79;
            double _t80;
            double _t81;
            double _t82;
            double _t83;
            double _t84;
            double _t85;
            double _t86;
            double _t87;
            double _t88;
            double _t89;
            double _t90;
            double _t91;
            double _t92;
            double _t93;
            double _t94;
            double _t95;
            double _t96;
            double _t97;
            double _t98;
            double _t99;
            int _t100;
            double _t101;
            double _t102;
            double _t103;
            double _t104;
            double _t105;
            double _t106;
            double _t107;
            double _t108;
            double* _t109;
            double _t110;
            double _t111;
            double* _t112;
            double* _t113;
            double* _t114;
            double _t115;
            double* _t116;
            double* _t117;
            double* _t118;
            double _t119;
            double* _t120;
            double _t121;
            double _t122;
            double _t123;
            double* _t124;
            double _t125;
            double _t126;
            double* _t127;
            double _t128;
            double* _t129;
            double _t130;
            double _t131;
            double* _t132;
            double _t133;
            double* _t134;
            double _t135;
            double _t136;
            double* _t137;
            double _t138;
            double* _t139;
            double* _t140;
            double _t141;
            double* _t142;
            double* _t143;
            double _t144;
            double* _t145;
            double* _t146;
            double* _t147;
            double* _t148;
            double* _t149;
            double* _t150;
            double* _t151;
            double* _t152;
            double* _t153;
            double _t154;
            double _t155;
            double* _t156;
            double _t157;
            double* _t158;
            double _t159;
            double* _t160;
            double _t161;
            double* _t162;
            double _t163;
            double* _t164;
            double _t165;
            double* _t166;
            double _t167;
            double* _t168;
            double _t169;
            double* _t170;
            double _t171;
            double* _t172;
            double _t173;
            double* _t174;
            double _t175;
            double* _t176;
            double _t177;
            double* _t178;
            double _t179;
            double* _t180;
            double _t181;
            double* _t182;
            double _t183;
            double* _t184;
            double _t185;
            double* _t186;
            double _t187;
            double _t188;
            double _t189;
            double _t190;
            double _t191;
            double _t192;
            double* _t193;
            double _t194;
            double* _t195;
            double _t196;
            double* _t197;
            double _t198;
            double* _t199;
            double _t200;
            double _t201;
            double* _t202;
            double _t203;
            double* _t204;
            double _t205;
            double* _t206;
            double _t207;
            double* _t208;
            double _t209;
            double* _t210;
            double _t211;
            double* _t212;
            double _t213;
            double* _t214;
            double _t215;
            double* _t216;
            double _t217;
            double* _t218;
            double _t219;
            double* _t220;
            double _t221;
            double* _t222;
            double _t223;
            double* _t224;
            double _t225;
            double _t226;
            double _t227;
            double _t228;
            double _t229;
            double _t230;
            double _t231;
            double _t232;
            double _t233;
            double _t234;
            double _t235;
            double _t236;
            double _t237;
            double _t238;
            double* _t239;
            double _t240;
            double* _t241;
            double _t242;
            double* _t243;
            double _t244;
            double mu2zz;
            double mu1zz;
            int q;
            double mu3zz;
            *_t37;
            _t50 = base + i;
            _t22[_t50];
            _t70 = _t50 + 2;
            _t35[_t70];
            _t36[i][_t6];
            a_strx[i + _t5];
            _t30[_t50];
            *_t37;
            _t36[i][_t6];
            _t35[_t50];
            _t22[_t50];
            *_t31;
            _t72 = i + 2;
            a_strx[_t72 + _t5];
            _t53 = i + -1;
            a_strx[_t53 + _t5];
            _t66 = i + 1;
            a_strx[_t66 + _t5];
            *_t31;
            _t22[_t70];
            *_t25;
            _t109 = _t42 + _t50;
            _t112 = _t43[i] + _t6;
            _t100 = i + -2;
            _t113 = _t36[_t100] + _t14;
            a_strx[_t53 + _t5];
            _t114 = _t36[i] + _t14;
            a_strx[i + _t5];
            _t116 = _t36[_t53] + _t14;
            _t117 = _t36[_t66] + _t14;
            _t118 = _t36[_t72] + _t14;
            _t22[_t50];
            _t82 = *_t28;
            _t81 = _t27[_t50];
            _t83 = _t81 * _t82;
            _t106 = *_t37;
            _t56 = _t22[_t50];
            _t80 = _t56 * _t106;
            _t78 = *_t25;
            _t77 = _t24[_t50];
            _t79 = _t77 * _t78;
            _t84 = _t79 - 0.75 * (_t80 + _t83);
            _t119 = 2.0 * _t84;
            _t120 = _t44 + _t50;
            _t96 = _t35[_t50];
            _t123 = _t96 * _t106;
            _t27[_t50];
            _t124 = _t45 + _t50;
            a_strx[_t66 + _t5];
            a_strx[i + _t5];
            _t127 = _t38[i] + _t14;
            a_strx[_t66 + _t5];
            _t86 = *_t31;
            _t85 = _t30[_t50];
            _t87 = _t85 * _t86;
            _t88 = _t83 + _t87 + 3.0 * (_t80 + _t79);
            _t128 = 2.0 * _t88;
            _t129 = _t46 + _t50;
            _t132 = _t39[i] + _t14;
            _t90 = *_t34;
            _t89 = _t33[_t50];
            _t91 = _t89 * _t90;
            _t92 = _t79 + _t91 + 3.0 * (_t87 + _t80);
            _t133 = 2.0 * _t92;
            _t64 = _t50 + 1;
            _t22[_t64];
            _t134 = _t47 + _t50;
            _t137 = _t40[i] + _t14;
            _t93 = _t87 - 0.75 * (_t80 + _t91);
            _t138 = 2.0 * _t93;
            _t139 = _t41[i] + _t14;
            _t140 = _t43[i] + _t14;
            _t36[i][_t6];
            _t142 = _t36[_t100] + _t12;
            a_strx[i + _t5];
            _t143 = _t36[i] + _t12;
            _t145 = _t36[_t53] + _t12;
            _t146 = _t36[_t66] + _t12;
            _t35[_t64];
            _t22[_t50];
            _t147 = _t36[_t72] + _t12;
            _t148 = _t38[i] + _t12;
            *_t37;
            _t149 = _t39[i] + _t12;
            a_strx[i + _t5];
            _t150 = _t40[i] + _t12;
            _t30[_t50];
            _t36[i][_t6];
            _t151 = _t41[i] + _t12;
            a_strx[_t66 + _t5];
            _t152 = _t48 + _t50;
            _t153 = _t43[i] + _t12;
            _t57 = a_strx[i + _t5];
            _t155 = _t57 * _t106;
            _t22[_t64];
            _t156 = _t38[_t100] + _t14;
            _t158 = _t41[_t100] + _t14;
            _t160 = _t39[_t100] + _t14;
            a_strx[_t72 + _t5];
            _t35[_t50];
            _t162 = _t40[_t100] + _t14;
            a_strx[_t53 + _t5];
            a_strx[_t72 + _t5];
            _t164 = _t38[_t53] + _t14;
            a_strx[_t53 + _t5];
            _t166 = _t41[_t53] + _t14;
            _t168 = _t39[_t53] + _t14;
            _t36[i][_t6];
            _t51 = _t50 + -1;
            _t22[_t51];
            _t170 = _t40[_t53] + _t14;
            _t172 = _t38[_t66] + _t14;
            a_strx[_t100 + _t5];
            _t24[_t50];
            _t174 = _t41[_t66] + _t14;
            _t35[_t50];
            a_strx[_t53 + _t5];
            _t176 = _t39[_t66] + _t14;
            _t178 = _t40[_t66] + _t14;
            _t22[_t51];
            _t180 = _t38[_t72] + _t14;
            _t182 = _t41[_t72] + _t14;
            _t22[_t50];
            _t184 = _t39[_t72] + _t14;
            a_strx[i + _t5];
            _t186 = _t40[_t72] + _t14;
            _t104 = _t35[_t70];
            _t189 = - _t104;
            _t102 = _t35[_t64];
            _t190 = 8.0 * _t102;
            a_strx[_t100 + _t5];
            *_t25;
            _t22[_t50];
            _t94 = _t35[_t51];
            _t191 = 8.0 * _t94;
            _t193 = _t38[_t100] + _t6;
            _t24[_t50];
            _t195 = _t41[_t100] + _t6;
            _t59 = _t50 + -2;
            _t35[_t59];
            _t197 = _t39[_t100] + _t6;
            _t33[_t50];
            _t199 = _t40[_t100] + _t6;
            *_t34;
            _t36[i][_t6];
            _t202 = _t41[_t53] + _t6;
            _t204 = _t39[_t53] + _t6;
            _t206 = _t40[_t53] + _t6;
            _t208 = _t38[_t66] + _t6;
            a_strx[_t100 + _t5];
            _t210 = _t41[_t66] + _t6;
            _t212 = _t39[_t66] + _t6;
            _t214 = _t40[_t66] + _t6;
            _t216 = _t38[_t72] + _t6;
            _t35[_t64];
            a_strx[_t66 + _t5];
            a_strx[i + _t5];
            _t22[_t59];
            _t218 = _t41[_t72] + _t6;
            _t220 = _t39[_t72] + _t6;
            _t222 = _t40[_t72] + _t6;
            _t22[_t50];
            _t224 = _t38[_t53] + _t6;
            _t71 = _t22[_t70];
            _t230 = - _t71;
            _t65 = _t22[_t64];
            _t231 = 8.0 * _t65;
            _t52 = _t22[_t51];
            _t232 = 8.0 * _t52;
            a_strx[i + _t5];
            _t234 = - _t89;
            _t235 = 8.0 * _t85;
            _t236 = 8.0 * _t77;
            _t35[_t51];
            _t239 = _t49[i] + _t6;
            *_t37;
            _t36[i][_t6];
            _t241 = _t49[i] + _t14;
            _t243 = _t49[i] + _t12;
            _t101 = _t36[i][_t6];
            _t73 = a_strx[_t72 + _t5];
            _t105 = _t104 * _t73;
            _t97 = _t96 * _t57;
            _t67 = a_strx[_t66 + _t5];
            _t103 = _t102 * _t67;
            _t74 = _t71 * _t73;
            _t58 = _t56 * _t57;
            _t68 = _t65 * _t67;
            _t76 = _t68 - 0.75 * (_t58 + _t74);
            _t54 = a_strx[_t53 + _t5];
            _t95 = _t94 * _t54;
            _t55 = _t52 * _t54;
            _t75 = _t55 + _t74 + 3.0 * (_t68 + _t58);
            _t61 = a_strx[_t100 + _t5];
            _t98 = _t35[_t59];
            _t99 = _t98 * _t61;
            _t60 = _t22[_t59];
            _t62 = _t60 * _t61;
            _t69 = _t62 + _t68 + 3.0 * (_t58 + _t55);
            _t63 = _t55 - 0.75 * (_t58 + _t62);
            _t107 = 0.16666666666666666 * (_t57 * ((2.0 * _t63 + _t95 - 0.75 * (
               _t97 + _t99)) * (_t36[_t100][_t6] - _t101) + (2.0 * _t69 + _t99 +
               _t103 + 3.0 * (_t97 + _t95)) * (_t36[_t53][_t6] - _t101) + (2.0 *
               _t75 + _t95 + _t105 + 3.0 * (_t103 + _t97)) * (_t36[_t66][_t6] -
               _t101) + (2.0 * _t76 + _t103 - 0.75 * (_t97 + _t105)) * (_t36[
               _t72][_t6] - _t101)) + _t106 * (_t84 * (_t38[i][_t6] - _t101) +
               _t88 * (_t39[i][_t6] - _t101) + _t92 * (_t40[i][_t6] - _t101) +
               _t93 * (_t41[i][_t6] - _t101)));
            *_t28;
            _t35[_t51];
            for (mu3zz = 0.0,
                 q = 1,
                 mu1zz = 0.0,
                 mu2zz = 0.0;
                 q <= 8; ) {
               int _t245;
               double* _t246;
               double _t247;
               double* _t248;
               double _t249;
               double* _t250;
               int _t251;
               double mucof;
               int m;
               double lap2mu;
               for (_t250 = a_u[q][j_4][i] + _t9,
                    _t248 = a_u[q][j_4][i] + _t8,
                    _t246 = a_u[q][j_4][i] + _t6,
                    _t245 = q + -1,
                    lap2mu = 0.0,
                    m = 1,
                    mucof = 0.0;
                    m <= 8; ) {
                  double _t252;
                  double _t253;
                  double _t254;
                  int _t255;
                  _t254 = a_mu[m][j_4][_t50];
                  _t253 = a_acof[m + -1][_t245][_t18];
                  _t252 = mucof + _t253 * _t254;
                  _t255 = m + 1;
                  lap2mu = lap2mu + _t253 * (a_lambda[m][j_4][_t50] + 2.0 *
                     _t254);
                  m = _t255;
                  mucof = _t252;
               }
               _t249 = mu2zz + mucof * *_t248;
               _t251 = q + 1;
               _t247 = mu1zz + mucof * *_t246;
               mu3zz = mu3zz + lap2mu * *_t250;
               q = _t251;
               mu1zz = _t247;
               mu2zz = _t249;
            }
            double u3zim1;
            double u3zip1;
            double u3zip2;
            int q_1;
            double u3zim2;
            *_t124;
            _t110 = *_t109;
            _t108 = *_t20;
            _t111 = _t108 * _t110;
            _t115 = *_t114;
            _t135 = *_t134;
            _t136 = _t135 * _t90;
            _t130 = *_t129;
            _t131 = _t130 * _t86;
            _t121 = *_t120;
            _t122 = _t121 * _t78;
            _t125 = *_t124;
            _t126 = _t125 * _t82;
            _t141 = 0.16666666666666666 * (_t57 * (_t63 * (*_t113 - _t115) +
               _t69 * (*_t116 - _t115) + _t75 * (*_t117 - _t115) + _t76 * (*
               _t118 - _t115)) + _t106 * ((_t119 + _t122 - 0.75 * (_t123 + _t126
               )) * (*_t127 - _t115) + (_t128 + _t126 + _t131 + 3.0 * (_t123 +
               _t122)) * (*_t132 - _t115) + (_t133 + _t122 + _t136 + 3.0 * (
               _t131 + _t123)) * (*_t137 - _t115) + (_t138 + _t131 - 0.75 * (
               _t123 + _t136)) * (*_t139 - _t115))) + (mu2zz + _t111 * *_t140);
            _t35[_t59];
            *_t120;
            *_t170;
            *_t109;
            *_t114;
            *_t143;
            _t35[_t50];
            *_t114;
            *_t143;
            *_t20;
            *_t28;
            _t30[_t50];
            *_t31;
            *_t178;
            *_t184;
            *_t37;
            *_t37;
            *_t143;
            _t35[_t50];
            _t27[_t50];
            *_t186;
            *_t25;
            _t33[_t50];
            *_t31;
            *_t172;
            *_t114;
            _t35[_t50];
            a_strx[i + _t5];
            *_t37;
            _t35[_t51];
            *_t25;
            *_t114;
            *_t34;
            *_t158;
            *_t114;
            *_t20;
            *_t37;
            *_t164;
            *_t109;
            *_t176;
            *_t25;
            *_t114;
            *_t143;
            *_t182;
            *_t143;
            *_t129;
            *_t168;
            a_strx[i + _t5];
            *_t114;
            *_t120;
            *_t134;
            *_t37;
            *_t129;
            *_t37;
            _t35[_t70];
            *_t166;
            *_t28;
            _t35[_t50];
            *_t160;
            *_t37;
            *_t162;
            *_t180;
            *_t143;
            *_t156;
            a_strx[i + _t5];
            _t226 = - _t135;
            *_t174;
            _t227 = 8.0 * _t130;
            _t144 = *_t143;
            _t154 = 0.16666666666666666 * (_t57 * (_t63 * (*_t142 - _t144) +
               _t69 * (*_t145 - _t144) + _t75 * (*_t146 - _t144) + _t76 * (*
               _t147 - _t144)) + _t106 * (_t84 * (*_t148 - _t144) + _t88 * (*
               _t149 - _t144) + _t92 * (*_t150 - _t144) + _t93 * (*_t151 - _t144
               ))) + (mu3zz + _t108 * (*_t152 + 2.0 * _t110) * *_t153);
            *_t34;
            _t35[_t64];
            _t228 = 8.0 * _t121;
            _t175 = *_t174;
            _t167 = *_t166;
            _t183 = *_t182;
            _t159 = *_t158;
            _t179 = *_t178;
            _t171 = *_t170;
            _t187 = *_t186;
            _t163 = *_t162;
            _t177 = *_t176;
            _t169 = - *_t168;
            _t185 = *_t184;
            _t161 = *_t160;
            _t173 = *_t172;
            _t165 = *_t164;
            _t181 = *_t180;
            _t157 = *_t156;
            _t188 = _t107 + (mu1zz + _t111 * *_t112) + _t155 * (
               0.006944444444444444 * (_t98 * (_t157 - _t159 + 8.0 * (- _t161 +
               _t163)) - 8.0 * (_t94 * (_t165 - _t167 + 8.0 * (_t169 + _t171)))
               + 8.0 * (_t102 * (_t173 - _t175 + 8.0 * (- _t177 + _t179))) -
               _t104 * (_t181 - _t183 + 8.0 * (- _t185 + _t187))) +
               0.006944444444444444 * (_t81 * (_t157 - _t181 + 8.0 * (- _t165 +
               _t173)) - 8.0 * (_t77 * (_t161 - _t185 + 8.0 * (_t169 + _t177)))
               + 8.0 * (_t85 * (_t163 - _t187 + 8.0 * (- _t171 + _t179))) - _t89
               * (_t159 - _t183 + 8.0 * (- _t167 + _t175))));
            *_t143;
            *_t31;
            _t24[_t50];
            u3zim2 = 0.0;
            q_1 = 1;
            u3zip2 = 0.0;
            u3zip1 = 0.0;
            u3zim1 = 0.0;
            for (u3zim2 = 0.0,
                 q_1 = 1,
                 u3zip2 = 0.0,
                 u3zip1 = 0.0,
                 u3zim1 = 0.0,
                 _t192 = _t188 + _t57 * (0.08333333333333333 * (_t189 * u3zip2 +
                    _t190 * u3zip1 - _t191 * u3zim1 + _t98 * u3zim2));
                 q_1 <= 8; ) {
               double _t256;
               double _t257;
               double _t258;
               double _t259;
               int _t260;
               _t260 = q_1 + 1;
               _t258 = a_bope[q_1 + -1][_t19];
               _t257 = u3zip1 + _t258 * a_u[q_1][j_4][_t66][_t12];
               _t256 = u3zip2 + _t258 * a_u[q_1][j_4][_t72][_t12];
               _t259 = u3zim1 + _t258 * a_u[q_1][j_4][_t53][_t12];
               u3zim2 = u3zim2 + _t258 * a_u[q_1][j_4][_t100][_t12];
               q_1 = _t260;
               u3zip2 = _t256;
               u3zip1 = _t257;
               u3zim1 = _t259;
               _t192 = _t188 + _t57 * (0.08333333333333333 * (_t189 * u3zip2 +
                  _t190 * u3zip1 - _t191 * u3zim1 + _t98 * u3zim2));
            }
            int q_2;
            double mu3xz;
            _t35[_t59];
            _t35[_t64];
            _t35[_t51];
            _t35[_t70];
            a_strx[i + _t5];
            mu3xz = 0.0;
            q_2 = 1;
            for (mu3xz = 0.0, q_2 = 1, _t240 = cof * (_t192 + _t57 * mu3xz);
                    q_2 <= 8; ) {
               int _t261;
               _t261 = q_2 + 1;
               mu3xz = mu3xz + a_bope[q_2 + -1][_t19] * (a_mu[q_2][j_4][_t50] *
                  0.08333333333333333 * (- a_u[q_2][j_4][_t72][_t12] + 8.0 * a_u
                  [q_2][j_4][_t66][_t12] - 8.0 * a_u[q_2][j_4][_t53][_t12] + a_u
                  [q_2][j_4][_t100][_t12]));
               q_2 = _t261;
               _t240 = cof * (_t192 + _t57 * mu3xz);
            }
            double u3zjm1;
            double u3zjp1;
            double u3zjp2;
            int q_3;
            double u3zjm2;
            _t22[_t51];
            _t22[_t64];
            _t22[_t70];
            *_t224;
            *_t216;
            *_t202;
            *_t222;
            *_t212;
            *_t124;
            *_t206;
            *_t134;
            *_t220;
            *_t218;
            *_t199;
            _t22[_t59];
            *_t120;
            *_t208;
            *_t197;
            *_t214;
            *_t210;
            *_t37;
            *_t193;
            a_strx[i + _t5];
            *_t204;
            _t211 = *_t210;
            _t203 = *_t202;
            _t219 = *_t218;
            _t196 = *_t195;
            _t215 = *_t214;
            _t207 = *_t206;
            _t223 = *_t222;
            _t200 = *_t199;
            _t213 = *_t212;
            _t205 = - *_t204;
            _t221 = *_t220;
            _t198 = *_t197;
            _t209 = *_t208;
            _t201 = *_t224;
            _t217 = *_t216;
            _t194 = *_t193;
            _t225 = _t141 + _t155 * (0.006944444444444444 * (_t60 * (_t194 -
               _t196 + 8.0 * (- _t198 + _t200)) - 8.0 * (_t52 * (_t201 - _t203 +
               8.0 * (_t205 + _t207))) + 8.0 * (_t65 * (_t209 - _t211 + 8.0 * (
               - _t213 + _t215))) - _t71 * (_t217 - _t219 + 8.0 * (- _t221 +
               _t223))) + 0.006944444444444444 * (_t125 * (_t194 - _t217 + 8.0 *
               (- _t201 + _t209)) - 8.0 * (_t121 * (_t198 - _t221 + 8.0 * (
               _t205 + _t213))) + 8.0 * (_t130 * (_t200 - _t223 + 8.0 * (-
               _t207 + _t215))) - _t135 * (_t196 - _t219 + 8.0 * (- _t203 +
               _t211))));
            *_t195;
            a_strx[i + _t5];
            *_t129;
            u3zjm2 = 0.0;
            q_3 = 1;
            u3zjp2 = 0.0;
            u3zjp1 = 0.0;
            u3zjm1 = 0.0;
            for (u3zjm2 = 0.0,
                 q_3 = 1,
                 u3zjp2 = 0.0,
                 u3zjp1 = 0.0,
                 u3zjm1 = 0.0,
                 _t229 = _t225 + _t106 * (0.08333333333333333 * (_t226 * u3zjp2
                    + _t227 * u3zjp1 - _t228 * u3zjm1 + _t125 * u3zjm2));
                 q_3 <= 8; ) {
               double _t262;
               double _t263;
               double _t264;
               double _t265;
               int _t266;
               _t266 = q_3 + 1;
               _t264 = a_bope[q_3 + -1][_t19];
               _t262 = u3zjp2 + _t264 * a_u[q_3][_t32][i][_t12];
               _t265 = u3zjm1 + _t264 * a_u[q_3][_t23][i][_t12];
               _t263 = u3zjp1 + _t264 * a_u[q_3][_t29][i][_t12];
               u3zjm2 = u3zjm2 + _t264 * a_u[q_3][_t26][i][_t12];
               q_3 = _t266;
               u3zjp2 = _t262;
               u3zjp1 = _t263;
               u3zjm1 = _t265;
               _t229 = _t225 + _t106 * (0.08333333333333333 * (_t226 * u3zjp2 +
                  _t227 * u3zjp1 - _t228 * u3zjm1 + _t125 * u3zjm2));
            }
            int q_4;
            double mu3yz;
            *_t129;
            *_t37;
            *_t120;
            *_t134;
            *_t124;
            mu3yz = 0.0;
            q_4 = 1;
            for (mu3yz = 0.0, q_4 = 1, _t242 = cof * (_t229 + _t106 * mu3yz);
                    q_4 <= 8; ) {
               int _t267;
               _t267 = q_4 + 1;
               mu3yz = mu3yz + a_bope[q_4 + -1][_t19] * (a_mu[q_4][j_4][_t50] *
                  0.08333333333333333 * (- a_u[q_4][_t32][i][_t12] + 8.0 * a_u[
                  q_4][_t29][i][_t12] - 8.0 * a_u[q_4][_t23][i][_t12] + a_u[q_4
                  ][_t26][i][_t12]));
               q_4 = _t267;
               _t242 = cof * (_t229 + _t106 * mu3yz);
            }
            double u1zim1;
            double u1zip1;
            double u1zip2;
            int q_5;
            double u1zim2;
            *_t37;
            u1zim2 = 0.0;
            q_5 = 1;
            u1zip2 = 0.0;
            u1zip1 = 0.0;
            u1zim1 = 0.0;
            for (u1zim2 = 0.0,
                 q_5 = 1,
                 u1zip2 = 0.0,
                 u1zip1 = 0.0,
                 u1zim1 = 0.0,
                 _t233 = _t154 + _t57 * (0.08333333333333333 * (_t230 * u1zip2 +
                    _t231 * u1zip1 - _t232 * u1zim1 + _t60 * u1zim2));
                 q_5 <= 8; ) {
               double _t268;
               double _t269;
               double _t270;
               double _t271;
               int _t272;
               _t272 = q_5 + 1;
               _t270 = a_bope[q_5 + -1][_t19];
               _t269 = u1zip1 + _t270 * a_u[q_5][j_4][_t66][_t6];
               _t268 = u1zip2 + _t270 * a_u[q_5][j_4][_t72][_t6];
               _t271 = u1zim1 + _t270 * a_u[q_5][j_4][_t53][_t6];
               u1zim2 = u1zim2 + _t270 * a_u[q_5][j_4][_t100][_t6];
               q_5 = _t272;
               u1zip2 = _t268;
               u1zip1 = _t269;
               u1zim1 = _t271;
               _t233 = _t154 + _t57 * (0.08333333333333333 * (_t230 * u1zip2 +
                  _t231 * u1zip1 - _t232 * u1zim1 + _t60 * u1zim2));
            }
            double u2zjm1;
            double u2zjp1;
            double u2zjp2;
            int q_6;
            double u2zjm2;
            a_strx[i + _t5];
            _t22[_t64];
            _t22[_t59];
            _t22[_t51];
            _t22[_t70];
            u2zjm2 = 0.0;
            q_6 = 1;
            u2zjp2 = 0.0;
            u2zjp1 = 0.0;
            u2zjm1 = 0.0;
            for (u2zjm2 = 0.0,
                 q_6 = 1,
                 u2zjp2 = 0.0,
                 u2zjp1 = 0.0,
                 u2zjm1 = 0.0,
                 _t237 = _t233 + _t106 * (0.08333333333333333 * (_t234 * u2zjp2
                    + _t235 * u2zjp1 - _t236 * u2zjm1 + _t81 * u2zjm2));
                 q_6 <= 8; ) {
               double _t273;
               double _t274;
               double _t275;
               double _t276;
               int _t277;
               _t277 = q_6 + 1;
               _t275 = a_bope[q_6 + -1][_t19];
               _t273 = u2zjp2 + _t275 * a_u[q_6][_t32][i][_t14];
               _t276 = u2zjm1 + _t275 * a_u[q_6][_t23][i][_t14];
               _t274 = u2zjp1 + _t275 * a_u[q_6][_t29][i][_t14];
               u2zjm2 = u2zjm2 + _t275 * a_u[q_6][_t26][i][_t14];
               q_6 = _t277;
               u2zjp2 = _t273;
               u2zjp1 = _t274;
               u2zjm1 = _t276;
               _t237 = _t233 + _t106 * (0.08333333333333333 * (_t234 * u2zjp2 +
                  _t235 * u2zjp1 - _t236 * u2zjm1 + _t81 * u2zjm2));
            }
            int q_7;
            double lau1xz;
            _t30[_t50];
            *_t37;
            _t24[_t50];
            _t33[_t50];
            _t27[_t50];
            lau1xz = 0.0;
            q_7 = 1;
            for (lau1xz = 0.0, q_7 = 1, _t238 = _t237 + _t57 * lau1xz; q_7 <= 8
                    ; ) {
               int _t278;
               _t278 = q_7 + 1;
               lau1xz = lau1xz + a_bope[q_7 + -1][_t19] * (a_lambda[q_7][j_4][
                  _t50] * 0.08333333333333333 * (- a_u[q_7][j_4][_t72][_t6] +
                  8.0 * a_u[q_7][j_4][_t66][_t6] - 8.0 * a_u[q_7][j_4][_t53][
                  _t6] + a_u[q_7][j_4][_t100][_t6]));
               q_7 = _t278;
               _t238 = _t237 + _t57 * lau1xz;
            }
            int q_8;
            double lau2yz;
            a_strx[i + _t5];
            lau2yz = 0.0;
            q_8 = 1;
            for (lau2yz = 0.0, q_8 = 1, _t244 = cof * (_t238 + _t106 * lau2yz);
                     q_8 <= 8; ) {
               int _t279;
               _t279 = q_8 + 1;
               lau2yz = lau2yz + a_bope[q_8 + -1][_t19] * (a_lambda[q_8][j_4][
                  _t50] * 0.08333333333333333 * (- a_u[q_8][_t32][i][_t14] + 8.0
                  * a_u[q_8][_t29][i][_t14] - 8.0 * a_u[q_8][_t23][i][_t14] +
                  a_u[q_8][_t26][i][_t14]));
               q_8 = _t279;
               _t244 = cof * (_t238 + _t106 * lau2yz);
            }
            *_t37;
            *_t239 = 0.0 * *_t239 + _t240;
            *_t241 = 0.0 * *_t241 + _t242;
            *_t243 = 0.0 * *_t243 + _t244;
         }
      }
   }
}

/*
 * ../src/rhs4sg.c:1359.6
 */
void core_fn3(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided_3, double (* a_acof)[8][6], double (* a_bope)[6
   ], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
{
   int _t1;
   int _t2;
   int _t3;
   int _t4;
   int _t5;
   int _t6;
   int _t7;
   int _t8;
   int _t9;
   int _t10;
   double (* _t11)[25];
   double (* _t12)[25][3];
   int _t13;
   double (* _t14)[25];
   int _t15;
   double cof;
   int k;
   for (_t15 = base3 + 2,
        _t14 = a_lambda[nk],
        _t13 = base3 + 3,
        _t12 = a_u[nk + 1],
        _t11 = a_mu[nk],
        _t10 = base3 + 3,
        _t9 = base3 + 2,
        _t8 = - jfirst,
        _t7 = base3 + 1,
        _t6 = - ifirst,
        _t5 = ilast + -2,
        _t3 = jlast + -2,
        _t1 = nk + -5,
        _t2 = jfirst + 2,
        _t4 = ifirst + 2,
        k = _t1;
        k <= nk; k++) {
      double (* _t16)[25];
      double (* _t17)[25];
      double (* _t18)[25][3];
      int _t19;
      int _t20;
      int _t21;
      double* _t22;
      double (* _t23)[25][3];
      int j_5;
      for (_t23 = a_lu[k],
           _t19 = nk + - k + 1,
           _t21 = _t19 + -1,
           _t22 = a_ghcof + _t21,
           _t20 = _t19 + -1,
           _t18 = a_u[k],
           _t17 = a_lambda[k],
           _t16 = a_mu[k],
           j_5 = _t2;
           j_5 <= _t3; j_5++) {
         double* _t24;
         int _t25;
         double* _t26;
         double* _t27;
         int _t28;
         double* _t29;
         double* _t30;
         int _t31;
         double* _t32;
         double* _t33;
         int _t34;
         double* _t35;
         double* _t36;
         double* _t37;
         double (* _t38)[3];
         double* _t39;
         double (* _t40)[3];
         double (* _t41)[3];
         double (* _t42)[3];
         double (* _t43)[3];
         double* _t44;
         double (* _t45)[3];
         double* _t46;
         double* _t47;
         double* _t48;
         double* _t49;
         double* _t50;
         double (* _t51)[3];
         int i;
         for (_t51 = _t23[j_5],
              _t50 = _t14[j_5],
              _t34 = j_5 + 2,
              _t49 = _t17[_t34],
              _t31 = j_5 + 1,
              _t48 = _t17[_t31],
              _t28 = j_5 + -2,
              _t47 = _t17[_t28],
              _t25 = j_5 + -1,
              _t46 = _t17[_t25],
              _t45 = _t12[j_5],
              _t44 = _t11[j_5],
              _t43 = _t18[_t34],
              _t42 = _t18[_t31],
              _t41 = _t18[_t25],
              _t40 = _t18[_t28],
              _t39 = a_stry + (j_5 + _t8),
              _t38 = _t18[j_5],
              _t37 = _t17[j_5],
              _t36 = a_stry + (_t34 + _t8),
              _t35 = _t16[_t34],
              _t33 = a_stry + (_t31 + _t8),
              _t32 = _t16[_t31],
              _t30 = a_stry + (_t28 + _t8),
              _t29 = _t16[_t28],
              _t27 = a_stry + (_t25 + _t8),
              _t26 = _t16[_t25],
              _t24 = _t16[j_5],
              i = _t4;
              i <= _t5; i++) {
            int _t52;
            int _t53;
            double _t54;
            int _t55;
            double _t56;
            double _t57;
            double _t58;
            double _t59;
            double _t60;
            int _t61;
            double _t62;
            int _t63;
            double _t64;
            double _t65;
            double _t66;
            int _t67;
            double _t68;
            double _t69;
            double _t70;
            double _t71;
            int _t72;
            double _t73;
            int _t74;
            double _t75;
            double _t76;
            double _t77;
            double _t78;
            double _t79;
            double _t80;
            double _t81;
            double _t82;
            double _t83;
            double _t84;
            double _t85;
            double _t86;
            double _t87;
            double _t88;
            double _t89;
            double _t90;
            double _t91;
            double _t92;
            double _t93;
            double _t94;
            double _t95;
            double _t96;
            double _t97;
            double _t98;
            double _t99;
            double _t100;
            double _t101;
            double _t102;
            double _t103;
            double _t104;
            double _t105;
            double _t106;
            int _t107;
            double _t108;
            double _t109;
            double _t110;
            double* _t111;
            double _t112;
            double _t113;
            double* _t114;
            double* _t115;
            double* _t116;
            double _t117;
            double* _t118;
            double* _t119;
            double* _t120;
            double _t121;
            double* _t122;
            double _t123;
            double _t124;
            double _t125;
            double* _t126;
            double _t127;
            double _t128;
            double* _t129;
            double _t130;
            double* _t131;
            double _t132;
            double _t133;
            double* _t134;
            double _t135;
            double* _t136;
            double _t137;
            double _t138;
            double* _t139;
            double _t140;
            double* _t141;
            double* _t142;
            double _t143;
            double* _t144;
            double* _t145;
            double _t146;
            double* _t147;
            double* _t148;
            double* _t149;
            double* _t150;
            double* _t151;
            double* _t152;
            double* _t153;
            double* _t154;
            double* _t155;
            double _t156;
            double _t157;
            double* _t158;
            double _t159;
            double* _t160;
            double _t161;
            double* _t162;
            double _t163;
            double* _t164;
            double _t165;
            double* _t166;
            double _t167;
            double* _t168;
            double _t169;
            double* _t170;
            double _t171;
            double* _t172;
            double _t173;
            double* _t174;
            double _t175;
            double* _t176;
            double _t177;
            double* _t178;
            double _t179;
            double* _t180;
            double _t181;
            double* _t182;
            double _t183;
            double* _t184;
            double _t185;
            double* _t186;
            double _t187;
            double* _t188;
            double _t189;
            double _t190;
            double _t191;
            double _t192;
            double _t193;
            double _t194;
            double* _t195;
            double _t196;
            double* _t197;
            double _t198;
            double* _t199;
            double _t200;
            double* _t201;
            double _t202;
            double _t203;
            double* _t204;
            double _t205;
            double* _t206;
            double _t207;
            double* _t208;
            double _t209;
            double* _t210;
            double _t211;
            double* _t212;
            double _t213;
            double* _t214;
            double _t215;
            double* _t216;
            double _t217;
            double* _t218;
            double _t219;
            double* _t220;
            double _t221;
            double* _t222;
            double _t223;
            double* _t224;
            double _t225;
            double* _t226;
            double _t227;
            double _t228;
            double _t229;
            double _t230;
            double _t231;
            double _t232;
            double _t233;
            double _t234;
            double _t235;
            double _t236;
            double _t237;
            double _t238;
            double _t239;
            double _t240;
            double* _t241;
            double _t242;
            double* _t243;
            double _t244;
            double* _t245;
            double _t246;
            double mu2zz;
            double mu1zz;
            int qb;
            double mu3zz;
            _t52 = base + i;
            _t24[_t52];
            _t107 = i + 1;
            a_strx[_t107 + _t6];
            _t24[_t52];
            *_t27;
            _t55 = i + -1;
            a_strx[_t55 + _t6];
            a_strx[_t55 + _t6];
            *_t30;
            a_strx[_t107 + _t6];
            _t32[_t52];
            _t26[_t52];
            *_t39;
            *_t33;
            _t74 = i + 2;
            a_strx[_t74 + _t6];
            a_strx[_t55 + _t6];
            _t72 = _t52 + 2;
            _t24[_t72];
            _t111 = _t44 + _t52;
            _t114 = _t45[i] + _t7;
            _t63 = i + -2;
            _t115 = _t38[_t63] + _t15;
            _t116 = _t38[i] + _t15;
            a_strx[i + _t6];
            _t118 = _t38[_t55] + _t15;
            _t119 = _t38[_t107] + _t15;
            _t120 = _t38[_t74] + _t15;
            a_strx[_t107 + _t6];
            _t24[_t52];
            _t84 = *_t30;
            _t83 = _t29[_t52];
            _t85 = _t83 * _t84;
            _t108 = *_t39;
            _t58 = _t24[_t52];
            _t82 = _t58 * _t108;
            _t80 = *_t27;
            _t79 = _t26[_t52];
            _t81 = _t79 * _t80;
            _t86 = _t81 - 0.75 * (_t82 + _t85);
            _t121 = 2.0 * _t86;
            _t122 = _t46 + _t52;
            _t38[i][_t7];
            _t98 = _t37[_t52];
            _t125 = _t98 * _t108;
            _t126 = _t47 + _t52;
            _t26[_t52];
            _t129 = _t40[i] + _t15;
            a_strx[_t107 + _t6];
            _t88 = *_t33;
            _t87 = _t32[_t52];
            _t89 = _t87 * _t88;
            _t90 = _t85 + _t89 + 3.0 * (_t82 + _t81);
            _t130 = 2.0 * _t90;
            _t131 = _t48 + _t52;
            _t134 = _t41[i] + _t15;
            _t38[i][_t7];
            _t92 = *_t36;
            _t91 = _t35[_t52];
            _t93 = _t91 * _t92;
            _t94 = _t81 + _t93 + 3.0 * (_t89 + _t82);
            _t135 = 2.0 * _t94;
            _t67 = _t52 + 1;
            _t24[_t67];
            _t136 = _t49 + _t52;
            _t139 = _t42[i] + _t15;
            _t95 = _t89 - 0.75 * (_t82 + _t93);
            _t140 = 2.0 * _t95;
            a_strx[_t74 + _t6];
            _t141 = _t43[i] + _t15;
            _t142 = _t45[i] + _t15;
            _t144 = _t38[_t63] + _t13;
            a_strx[i + _t6];
            _t145 = _t38[i] + _t13;
            _t147 = _t38[_t55] + _t13;
            _t148 = _t38[_t107] + _t13;
            a_strx[i + _t6];
            a_strx[i + _t6];
            _t37[_t67];
            _t24[_t52];
            _t149 = _t38[_t74] + _t13;
            _t150 = _t40[i] + _t13;
            _t151 = _t41[i] + _t13;
            _t152 = _t42[i] + _t13;
            _t153 = _t43[i] + _t13;
            a_strx[_t107 + _t6];
            _t154 = _t50 + _t52;
            _t155 = _t45[i] + _t13;
            _t59 = a_strx[i + _t6];
            _t157 = _t59 * _t108;
            _t37[_t52];
            _t24[_t67];
            _t158 = _t40[_t63] + _t15;
            _t160 = _t43[_t63] + _t15;
            _t162 = _t41[_t63] + _t15;
            _t38[i][_t7];
            _t164 = _t42[_t63] + _t15;
            _t166 = _t40[_t55] + _t15;
            _t37[_t72];
            a_strx[_t55 + _t6];
            _t168 = _t43[_t55] + _t15;
            _t170 = _t41[_t55] + _t15;
            _t29[_t52];
            _t37[_t52];
            _t32[_t52];
            _t53 = _t52 + -1;
            _t24[_t53];
            _t172 = _t42[_t55] + _t15;
            *_t39;
            _t174 = _t40[_t107] + _t15;
            _t176 = _t43[_t107] + _t15;
            a_strx[_t55 + _t6];
            _t178 = _t41[_t107] + _t15;
            _t180 = _t42[_t107] + _t15;
            _t35[_t52];
            a_strx[_t63 + _t6];
            _t24[_t53];
            _t182 = _t40[_t74] + _t15;
            _t24[_t52];
            _t184 = _t43[_t74] + _t15;
            _t37[_t67];
            _t37[_t52];
            _t186 = _t41[_t74] + _t15;
            a_strx[_t74 + _t6];
            a_strx[i + _t6];
            _t188 = _t42[_t74] + _t15;
            _t104 = _t37[_t72];
            _t191 = - _t104;
            _t106 = _t37[_t67];
            _t192 = 8.0 * _t106;
            _t38[i][_t7];
            *_t27;
            _t24[_t52];
            _t96 = _t37[_t53];
            _t193 = 8.0 * _t96;
            _t195 = _t40[_t63] + _t7;
            _t38[i][_t7];
            _t197 = _t43[_t63] + _t7;
            _t199 = _t41[_t63] + _t7;
            _t201 = _t42[_t63] + _t7;
            _t37[_t53];
            _t204 = _t43[_t55] + _t7;
            _t206 = _t41[_t55] + _t7;
            _t208 = _t42[_t55] + _t7;
            *_t39;
            _t210 = _t40[_t107] + _t7;
            _t61 = _t52 + -2;
            _t37[_t61];
            _t212 = _t43[_t107] + _t7;
            a_strx[_t63 + _t6];
            _t214 = _t41[_t107] + _t7;
            _t216 = _t42[_t107] + _t7;
            _t218 = _t40[_t74] + _t7;
            _t220 = _t43[_t74] + _t7;
            _t24[_t52];
            _t24[_t61];
            _t222 = _t41[_t74] + _t7;
            _t224 = _t42[_t74] + _t7;
            _t226 = _t40[_t55] + _t7;
            _t73 = _t24[_t72];
            _t232 = - _t73;
            a_strx[i + _t6];
            _t68 = _t24[_t67];
            _t233 = 8.0 * _t68;
            _t54 = _t24[_t53];
            _t234 = 8.0 * _t54;
            _t38[i][_t7];
            *_t33;
            _t236 = - _t91;
            *_t39;
            _t237 = 8.0 * _t87;
            a_strx[i + _t6];
            _t238 = 8.0 * _t79;
            _t38[i][_t7];
            _t241 = _t51[i] + _t7;
            _t102 = _t38[i][_t7];
            _t75 = a_strx[_t74 + _t6];
            _t105 = _t104 * _t75;
            _t99 = _t98 * _t59;
            _t69 = a_strx[_t107 + _t6];
            _t103 = _t106 * _t69;
            _t76 = _t73 * _t75;
            _t60 = _t58 * _t59;
            _t70 = _t68 * _t69;
            _t78 = _t70 - 0.75 * (_t60 + _t76);
            _t56 = a_strx[_t55 + _t6];
            _t97 = _t96 * _t56;
            _t57 = _t54 * _t56;
            _t77 = _t57 + _t76 + 3.0 * (_t70 + _t60);
            _t64 = a_strx[_t63 + _t6];
            _t100 = _t37[_t61];
            _t101 = _t100 * _t64;
            _t62 = _t24[_t61];
            _t65 = _t62 * _t64;
            _t71 = _t65 + _t70 + 3.0 * (_t60 + _t57);
            _t66 = _t57 - 0.75 * (_t60 + _t65);
            _t109 = 0.16666666666666666 * (_t59 * ((2.0 * _t66 + _t97 - 0.75 * (
               _t99 + _t101)) * (_t38[_t63][_t7] - _t102) + (2.0 * _t71 + _t101
               + _t103 + 3.0 * (_t99 + _t97)) * (_t38[_t55][_t7] - _t102) + (
               2.0 * _t77 + _t97 + _t105 + 3.0 * (_t103 + _t99)) * (_t38[_t107]
               [_t7] - _t102) + (2.0 * _t78 + _t103 - 0.75 * (_t99 + _t105)) * (
               _t38[_t74][_t7] - _t102)) + _t108 * (_t86 * (_t40[i][_t7] - _t102
               ) + _t90 * (_t41[i][_t7] - _t102) + _t94 * (_t42[i][_t7] - _t102
               ) + _t95 * (_t43[i][_t7] - _t102)));
            _t243 = _t51[i] + _t15;
            _t245 = _t51[i] + _t13;
            _t37[_t53];
            a_strx[_t63 + _t6];
            *_t36;
            a_strx[i + _t6];
            for (mu3zz = 0.0,
                 qb = 1,
                 mu1zz = 0.0,
                 mu2zz = 0.0;
                 qb <= 8; ) {
               int _t247;
               double* _t248;
               double _t249;
               double* _t250;
               double _t251;
               double* _t252;
               int _t253;
               double mucof;
               int mb;
               double lap2mu;
               for (_t252 = a_u[nk + - qb + 1][j_5][i] + _t10,
                    _t250 = a_u[nk + - qb + 1][j_5][i] + _t9,
                    _t248 = a_u[nk + - qb + 1][j_5][i] + _t7,
                    _t247 = qb + -1,
                    lap2mu = 0.0,
                    mb = 1,
                    mucof = 0.0;
                    mb <= 8; ) {
                  double _t254;
                  double _t255;
                  int _t256;
                  double _t257;
                  int _t258;
                  _t258 = mb + 1;
                  _t256 = nk + - mb + 1;
                  _t257 = a_mu[_t256][j_5][_t52];
                  _t254 = a_acof[mb + -1][_t247][_t20];
                  _t255 = mucof + _t254 * _t257;
                  lap2mu = lap2mu + _t254 * (2.0 * _t257 + a_lambda[_t256][j_5]
                     [_t52]);
                  mb = _t258;
                  mucof = _t255;
               }
               _t253 = qb + 1;
               _t251 = mu2zz + mucof * *_t250;
               _t249 = mu1zz + mucof * *_t248;
               mu3zz = mu3zz + lap2mu * *_t252;
               qb = _t253;
               mu1zz = _t249;
               mu2zz = _t251;
            }
            double u3zim1;
            double u3zip1;
            double u3zip2;
            int qb_1;
            double u3zim2;
            *_t122;
            *_t172;
            *_t116;
            *_t170;
            _t37[_t52];
            *_t39;
            *_t111;
            *_t111;
            *_t145;
            _t29[_t52];
            *_t186;
            *_t30;
            *_t22;
            *_t36;
            *_t180;
            *_t164;
            *_t116;
            *_t39;
            *_t145;
            *_t145;
            _t37[_t52];
            *_t145;
            _t32[_t52];
            *_t27;
            *_t116;
            *_t27;
            *_t162;
            _t35[_t52];
            _t37[_t61];
            *_t33;
            *_t122;
            *_t39;
            *_t116;
            *_t27;
            *_t116;
            *_t188;
            *_t160;
            *_t36;
            *_t116;
            *_t145;
            *_t39;
            *_t33;
            *_t184;
            a_strx[i + _t6];
            *_t39;
            *_t116;
            _t37[_t52];
            *_t145;
            *_t136;
            *_t174;
            *_t22;
            a_strx[i + _t6];
            *_t168;
            _t37[_t67];
            _t37[_t52];
            *_t30;
            _t37[_t53];
            _t112 = *_t111;
            _t110 = *_t22;
            _t113 = _t110 * _t112;
            _t117 = *_t116;
            _t137 = *_t136;
            _t138 = _t137 * _t92;
            _t132 = *_t131;
            _t133 = _t132 * _t88;
            _t123 = *_t122;
            _t124 = _t123 * _t80;
            _t127 = *_t126;
            _t128 = _t127 * _t84;
            _t143 = 0.16666666666666666 * (_t59 * (_t66 * (*_t115 - _t117) +
               _t71 * (*_t118 - _t117) + _t77 * (*_t119 - _t117) + _t78 * (*
               _t120 - _t117)) + _t108 * ((_t121 + _t124 - 0.75 * (_t125 + _t128
               )) * (*_t129 - _t117) + (_t130 + _t128 + _t133 + 3.0 * (_t125 +
               _t124)) * (*_t134 - _t117) + (_t135 + _t124 + _t138 + 3.0 * (
               _t133 + _t125)) * (*_t139 - _t117) + (_t140 + _t133 - 0.75 * (
               _t125 + _t138)) * (*_t141 - _t117))) + (mu2zz + _t113 * *_t142);
            _t37[_t72];
            *_t158;
            *_t39;
            *_t145;
            *_t39;
            *_t131;
            a_strx[i + _t6];
            _t228 = - _t137;
            *_t176;
            *_t178;
            _t229 = 8.0 * _t132;
            *_t166;
            _t230 = 8.0 * _t123;
            _t177 = *_t176;
            _t169 = *_t168;
            _t185 = *_t184;
            _t161 = *_t160;
            _t181 = *_t180;
            _t173 = *_t172;
            _t189 = *_t188;
            _t165 = *_t164;
            _t179 = *_t178;
            _t171 = - *_t170;
            _t187 = *_t186;
            _t163 = *_t162;
            _t175 = *_t174;
            _t167 = *_t166;
            _t183 = *_t182;
            _t159 = *_t158;
            _t190 = _t109 + (mu1zz + _t113 * *_t114) + _t157 * (
               0.006944444444444444 * (_t100 * (_t159 - _t161 + 8.0 * (- _t163 +
               _t165)) - 8.0 * (_t96 * (_t167 - _t169 + 8.0 * (_t171 + _t173)))
               + 8.0 * (_t106 * (_t175 - _t177 + 8.0 * (- _t179 + _t181))) -
               _t104 * (_t183 - _t185 + 8.0 * (- _t187 + _t189))) +
               0.006944444444444444 * (_t83 * (_t159 - _t183 + 8.0 * (- _t167 +
               _t175)) - 8.0 * (_t79 * (_t163 - _t187 + 8.0 * (_t171 + _t179)))
               + 8.0 * (_t87 * (_t165 - _t189 + 8.0 * (- _t173 + _t181))) - _t91
               * (_t161 - _t185 + 8.0 * (- _t169 + _t177))));
            _t146 = *_t145;
            _t156 = 0.16666666666666666 * (_t59 * (_t66 * (*_t144 - _t146) +
               _t71 * (*_t147 - _t146) + _t77 * (*_t148 - _t146) + _t78 * (*
               _t149 - _t146)) + _t108 * (_t86 * (*_t150 - _t146) + _t90 * (*
               _t151 - _t146) + _t94 * (*_t152 - _t146) + _t95 * (*_t153 - _t146
               ))) + (mu3zz + _t110 * (*_t154 + 2.0 * _t112) * *_t155);
            *_t33;
            *_t182;
            _t26[_t52];
            *_t126;
            *_t131;
            u3zim2 = 0.0;
            qb_1 = 1;
            u3zip2 = 0.0;
            u3zip1 = 0.0;
            u3zim1 = 0.0;
            for (u3zim2 = 0.0,
                 qb_1 = 1,
                 u3zip2 = 0.0,
                 u3zip1 = 0.0,
                 u3zim1 = 0.0,
                 _t194 = _t190 + _t59 * (0.08333333333333333 * (_t191 * u3zip2 +
                    _t192 * u3zip1 - _t193 * u3zim1 + _t100 * u3zim2));
                 qb_1 <= 8; ) {
               double _t259;
               double _t260;
               double _t261;
               double _t262;
               int _t263;
               _t263 = qb_1 + 1;
               _t259 = a_bope[qb_1 + -1][_t21];
               _t262 = u3zim1 - _t259 * a_u[nk + - qb_1 + 1][j_5][_t55][_t13];
               _t261 = u3zip1 - _t259 * a_u[nk + - qb_1 + 1][j_5][_t107][_t13];
               _t260 = u3zip2 - _t259 * a_u[nk + - qb_1 + 1][j_5][_t74][_t13];
               u3zim2 = u3zim2 - _t259 * a_u[nk + - qb_1 + 1][j_5][_t63][_t13];
               qb_1 = _t263;
               u3zip2 = _t260;
               u3zip1 = _t261;
               u3zim1 = _t262;
               _t194 = _t190 + _t59 * (0.08333333333333333 * (_t191 * u3zip2 +
                  _t192 * u3zip1 - _t193 * u3zim1 + _t100 * u3zim2));
            }
            int qb_2;
            double mu3xz;
            _t37[_t61];
            _t37[_t67];
            _t37[_t53];
            _t37[_t72];
            a_strx[i + _t6];
            mu3xz = 0.0;
            qb_2 = 1;
            for (mu3xz = 0.0, qb_2 = 1, _t242 = cof * (_t194 + _t59 * mu3xz);
                    qb_2 <= 8; ) {
               int _t264;
               int _t265;
               _t265 = qb_2 + 1;
               _t264 = nk + - qb_2 + 1;
               mu3xz = mu3xz - a_bope[qb_2 + -1][_t21] * (a_mu[_t264][j_5][_t52
                  ] * 0.08333333333333333 * (- a_u[_t264][j_5][_t74][_t13] + 8.0
                  * a_u[_t264][j_5][_t107][_t13] - 8.0 * a_u[_t264][j_5][_t55][
                  _t13] + a_u[_t264][j_5][_t63][_t13]));
               qb_2 = _t265;
               _t242 = cof * (_t194 + _t59 * mu3xz);
            }
            double u3zjm1;
            double u3zjp1;
            double u3zjp2;
            int qb_3;
            double u3zjm2;
            _t24[_t53];
            _t24[_t67];
            _t24[_t72];
            *_t226;
            *_t218;
            *_t204;
            *_t224;
            *_t214;
            *_t126;
            *_t208;
            *_t136;
            *_t222;
            *_t220;
            *_t201;
            _t24[_t61];
            *_t122;
            *_t210;
            *_t199;
            *_t216;
            *_t212;
            *_t39;
            *_t195;
            a_strx[i + _t6];
            *_t206;
            _t213 = *_t212;
            _t205 = *_t204;
            _t221 = *_t220;
            _t198 = *_t197;
            _t217 = *_t216;
            _t209 = *_t208;
            _t225 = *_t224;
            _t202 = *_t201;
            _t215 = *_t214;
            _t207 = - *_t206;
            _t223 = *_t222;
            _t200 = *_t199;
            _t211 = *_t210;
            _t203 = *_t226;
            _t219 = *_t218;
            _t196 = *_t195;
            _t227 = _t143 + _t157 * (0.006944444444444444 * (_t62 * (_t196 -
               _t198 + 8.0 * (- _t200 + _t202)) - 8.0 * (_t54 * (_t203 - _t205 +
               8.0 * (_t207 + _t209))) + 8.0 * (_t68 * (_t211 - _t213 + 8.0 * (
               - _t215 + _t217))) - _t73 * (_t219 - _t221 + 8.0 * (- _t223 +
               _t225))) + 0.006944444444444444 * (_t127 * (_t196 - _t219 + 8.0 *
               (- _t203 + _t211)) - 8.0 * (_t123 * (_t200 - _t223 + 8.0 * (
               _t207 + _t215))) + 8.0 * (_t132 * (_t202 - _t225 + 8.0 * (-
               _t209 + _t217))) - _t137 * (_t198 - _t221 + 8.0 * (- _t205 +
               _t213))));
            *_t197;
            a_strx[i + _t6];
            *_t131;
            u3zjm2 = 0.0;
            qb_3 = 1;
            u3zjp2 = 0.0;
            u3zjp1 = 0.0;
            u3zjm1 = 0.0;
            for (u3zjm2 = 0.0,
                 qb_3 = 1,
                 u3zjp2 = 0.0,
                 u3zjp1 = 0.0,
                 u3zjm1 = 0.0,
                 _t231 = _t227 + _t108 * (0.08333333333333333 * (_t228 * u3zjp2
                    + _t229 * u3zjp1 - _t230 * u3zjm1 + _t127 * u3zjm2));
                 qb_3 <= 8; ) {
               double _t266;
               double _t267;
               double _t268;
               double _t269;
               int _t270;
               _t270 = qb_3 + 1;
               _t266 = a_bope[qb_3 + -1][_t21];
               _t269 = u3zjm1 - _t266 * a_u[nk + - qb_3 + 1][_t25][i][_t13];
               _t268 = u3zjp1 - _t266 * a_u[nk + - qb_3 + 1][_t31][i][_t13];
               _t267 = u3zjp2 - _t266 * a_u[nk + - qb_3 + 1][_t34][i][_t13];
               u3zjm2 = u3zjm2 - _t266 * a_u[nk + - qb_3 + 1][_t28][i][_t13];
               qb_3 = _t270;
               u3zjp2 = _t267;
               u3zjp1 = _t268;
               u3zjm1 = _t269;
               _t231 = _t227 + _t108 * (0.08333333333333333 * (_t228 * u3zjp2 +
                  _t229 * u3zjp1 - _t230 * u3zjm1 + _t127 * u3zjm2));
            }
            int qb_4;
            double mu3yz;
            *_t131;
            *_t39;
            *_t122;
            *_t136;
            *_t126;
            mu3yz = 0.0;
            qb_4 = 1;
            for (mu3yz = 0.0, qb_4 = 1, _t244 = cof * (_t231 + _t108 * mu3yz);
                    qb_4 <= 8; ) {
               int _t271;
               int _t272;
               _t272 = qb_4 + 1;
               _t271 = nk + - qb_4 + 1;
               mu3yz = mu3yz - a_bope[qb_4 + -1][_t21] * (a_mu[_t271][j_5][_t52
                  ] * 0.08333333333333333 * (- a_u[_t271][_t34][i][_t13] + 8.0 *
                  a_u[_t271][_t31][i][_t13] - 8.0 * a_u[_t271][_t25][i][_t13] +
                  a_u[_t271][_t28][i][_t13]));
               qb_4 = _t272;
               _t244 = cof * (_t231 + _t108 * mu3yz);
            }
            double u1zim1;
            double u1zip1;
            double u1zip2;
            int qb_5;
            double u1zim2;
            *_t39;
            u1zim2 = 0.0;
            qb_5 = 1;
            u1zip2 = 0.0;
            u1zip1 = 0.0;
            u1zim1 = 0.0;
            for (u1zim2 = 0.0,
                 qb_5 = 1,
                 u1zip2 = 0.0,
                 u1zip1 = 0.0,
                 u1zim1 = 0.0,
                 _t235 = _t156 + _t59 * (0.08333333333333333 * (_t232 * u1zip2 +
                    _t233 * u1zip1 - _t234 * u1zim1 + _t62 * u1zim2));
                 qb_5 <= 8; ) {
               double _t273;
               double _t274;
               double _t275;
               double _t276;
               int _t277;
               _t277 = qb_5 + 1;
               _t273 = a_bope[qb_5 + -1][_t21];
               _t276 = u1zim1 - _t273 * a_u[nk + - qb_5 + 1][j_5][_t55][_t7];
               _t275 = u1zip1 - _t273 * a_u[nk + - qb_5 + 1][j_5][_t107][_t7];
               _t274 = u1zip2 - _t273 * a_u[nk + - qb_5 + 1][j_5][_t74][_t7];
               u1zim2 = u1zim2 - _t273 * a_u[nk + - qb_5 + 1][j_5][_t63][_t7];
               qb_5 = _t277;
               u1zip2 = _t274;
               u1zip1 = _t275;
               u1zim1 = _t276;
               _t235 = _t156 + _t59 * (0.08333333333333333 * (_t232 * u1zip2 +
                  _t233 * u1zip1 - _t234 * u1zim1 + _t62 * u1zim2));
            }
            double u2zjm1;
            double u2zjp1;
            double u2zjp2;
            int qb_6;
            double u2zjm2;
            a_strx[i + _t6];
            _t24[_t67];
            _t24[_t61];
            _t24[_t53];
            _t24[_t72];
            u2zjm2 = 0.0;
            qb_6 = 1;
            u2zjp2 = 0.0;
            u2zjp1 = 0.0;
            u2zjm1 = 0.0;
            for (u2zjm2 = 0.0,
                 qb_6 = 1,
                 u2zjp2 = 0.0,
                 u2zjp1 = 0.0,
                 u2zjm1 = 0.0,
                 _t239 = _t235 + _t108 * (0.08333333333333333 * (_t236 * u2zjp2
                    + _t237 * u2zjp1 - _t238 * u2zjm1 + _t83 * u2zjm2));
                 qb_6 <= 8; ) {
               double _t278;
               double _t279;
               double _t280;
               double _t281;
               int _t282;
               _t282 = qb_6 + 1;
               _t278 = a_bope[qb_6 + -1][_t21];
               _t281 = u2zjm1 - _t278 * a_u[nk + - qb_6 + 1][_t25][i][_t15];
               _t280 = u2zjp1 - _t278 * a_u[nk + - qb_6 + 1][_t31][i][_t15];
               _t279 = u2zjp2 - _t278 * a_u[nk + - qb_6 + 1][_t34][i][_t15];
               u2zjm2 = u2zjm2 - _t278 * a_u[nk + - qb_6 + 1][_t28][i][_t15];
               qb_6 = _t282;
               u2zjp2 = _t279;
               u2zjp1 = _t280;
               u2zjm1 = _t281;
               _t239 = _t235 + _t108 * (0.08333333333333333 * (_t236 * u2zjp2 +
                  _t237 * u2zjp1 - _t238 * u2zjm1 + _t83 * u2zjm2));
            }
            int qb_7;
            double lau1xz;
            _t32[_t52];
            *_t39;
            _t26[_t52];
            _t35[_t52];
            _t29[_t52];
            lau1xz = 0.0;
            qb_7 = 1;
            for (lau1xz = 0.0, qb_7 = 1, _t240 = _t239 + _t59 * lau1xz; qb_7 <=
                    8; ) {
               int _t283;
               int _t284;
               _t284 = qb_7 + 1;
               _t283 = nk + - qb_7 + 1;
               lau1xz = lau1xz - a_bope[qb_7 + -1][_t21] * (a_lambda[_t283][j_5
                  ][_t52] * 0.08333333333333333 * (- a_u[_t283][j_5][_t74][_t7]
                  + 8.0 * a_u[_t283][j_5][_t107][_t7] - 8.0 * a_u[_t283][j_5][
                  _t55][_t7] + a_u[_t283][j_5][_t63][_t7]));
               qb_7 = _t284;
               _t240 = _t239 + _t59 * lau1xz;
            }
            int qb_8;
            double lau2yz;
            a_strx[i + _t6];
            lau2yz = 0.0;
            qb_8 = 1;
            for (lau2yz = 0.0, qb_8 = 1, _t246 = cof * (_t240 + _t108 * lau2yz)
                    ; qb_8 <= 8; ) {
               int _t285;
               int _t286;
               _t286 = qb_8 + 1;
               _t285 = nk + - qb_8 + 1;
               lau2yz = lau2yz - a_bope[qb_8 + -1][_t21] * (a_lambda[_t285][j_5
                  ][_t52] * 0.08333333333333333 * (- a_u[_t285][_t34][i][_t15] +
                  8.0 * a_u[_t285][_t31][i][_t15] - 8.0 * a_u[_t285][_t25][i][
                  _t15] + a_u[_t285][_t28][i][_t15]));
               qb_8 = _t286;
               _t246 = cof * (_t240 + _t108 * lau2yz);
            }
            *_t39;
            *_t241 = 0.0 * *_t241 + _t242;
            *_t243 = 0.0 * *_t243 + _t244;
            *_t245 = 0.0 * *_t245 + _t246;
         }
      }
   }
}

/*
 * ../src/rhs4sg.c:1643.6
 */
void mapped_fn1(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided_4, double (* a_acof)[8][6], double (* a_bope)[6
   ], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
{
   if (- ifirst + ilast >= 4 && - jfirst + jlast >= 4) {
      int _t1;
      int i;
      _t1 = - k1 + k2;
#pragma omp parallel for private(i)
      for (i = 0; i <= _t1; i++) {
         int _t2;
         int j_6;
         _t2 = - jfirst + jlast + -4;
#ifdef RSTREAM_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef RSTREAM_IVDEP
#pragma ivdep
#endif
         for (j_6 = 0; j_6 <= _t2; j_6++) {
            int _t3;
            int k;
            _t3 = - ifirst + ilast + -4;
#ifdef RSTREAM_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef RSTREAM_IVDEP
#pragma ivdep
#endif
            for (k = 0; k <= _t3; k++) {
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
               double _t19;
               double _t20;
               double _t21;
               double _t22;
               double _t23;
               double _t24;
               double _t25;
               double _t26;
               double _t27;
               double _t28;
               double _t29;
               double _t30;
               double _t31;
               double _t32;
               double _t33;
               double _t34;
               double _t35;
               double _t36;
               double _t37;
               double _t38;
               double _t39;
               double _t40;
               double _t41;
               double _t42;
               double _t43;
               double _t44;
               double _t45;
               double _t46;
               double _t47;
               double _t48;
               double _t49;
               double _t50;
               double _t51;
               double _t52;
               double _t53;
               double _t54;
               double _t55;
               double _t56;
               double _t57;
               double _t58;
               double _t59;
               double _t60;
               double _t61;
               double _t62;
               double _t63;
               double _t64;
               double _t65;
               double _t66;
               double _t67;
               double _t68;
               double _t69;
               double _t70;
               double _t71;
               double _t72;
               double _t73;
               double _t74;
               double _t75;
               double _t76;
               double _t77;
               double _t78;
               double _t79;
               double _t80;
               double _t81;
               double _t82;
               double _t83;
               double _t84;
               double _t85;
               double _t86;
               double _t87;
               double _t88;
               double _t89;
               double _t90;
               double _t91;
               double _t92;
               double _t93;
               double _t94;
               double _t95;
               double _t96;
               double _t97;
               double _t98;
               double _t99;
               double _t100;
               double _t101;
               double _t102;
               double _t103;
               double _t104;
               double _t105;
               double _t106;
               double _t107;
               double _t108;
               double _t109;
               double _t110;
               double _t111;
               double _t112;
               double _t113;
               double _t114;
               double _t115;
               double _t116;
               double _t117;
               double _t118;
               double _t119;
               double _t120;
               double _t121;
               double _t122;
               double _t123;
               double _t124;
               double _t125;
               double _t126;
               double _t127;
               double _t128;
               double _t129;
               double _t130;
               double _t131;
               double _t132;
               double _t133;
               double _t134;
               double _t135;
               double _t136;
               double _t137;
               double _t138;
               double _t139;
               double _t140;
               double _t141;
               double _t142;
               double _t143;
               double _t144;
               double _t145;
               double _t146;
               double _t147;
               double _t148;
               double _t149;
               double _t150;
               double _t151;
               double _t152;
               double _t153;
               double _t154;
               double _t155;
               double _t156;
               double _t157;
               double _t158;
               double _t159;
               double _t160;
               double _t161;
               double _t162;
               double _t163;
               double _t164;
               double _t165;
               double _t166;
               double _t167;
               double _t168;
               double _t169;
               double _t170;
               double _t171;
               double _t172;
               double _t173;
               double _t174;
               double _t175;
               double _t176;
               double _t177;
               double _t178;
               double _t179;
               double _t180;
               double _t181;
               double _t182;
               double _t183;
               double _t184;
               double _t185;
               double _t186;
               double _t187;
               double _t188;
               _t101 = a_stry[j_6 + 2];
               _t74 = a_strx[k + 2];
               _t17 = a_strz[i + (-1 * kfirst + k1)];
               _t78 = a_lambda[i + k1][j_6 + (jfirst + 2)][k + ifirst + (base +
                  2)];
               _t66 = a_strz[i + -1 * kfirst + (k1 + 2)];
               _t65 = a_mu[i + (k1 + 2)][j_6 + (jfirst + 2)][k + ifirst + (base
                  + 2)];
               _t59 = a_strz[i + -1 * kfirst + (k1 + 1)];
               _t58 = a_mu[i + (k1 + 1)][j_6 + (jfirst + 2)][k + ifirst + (base
                  + 2)];
               _t7 = a_mu[i + k1][j_6 + (jfirst + 2)][k + ifirst + (base + 2)];
               _t49 = a_strz[i + -1 * kfirst + (k1 + -1)];
               _t48 = a_mu[i + (k1 + -1)][j_6 + (jfirst + 2)][k + ifirst + (
                  base + 2)];
               _t54 = a_strz[i + -1 * kfirst + (k1 + -2)];
               _t53 = a_mu[i + (k1 + -2)][j_6 + (jfirst + 2)][k + ifirst + (
                  base + 2)];
               _t43 = a_stry[j_6 + 4];
               _t42 = a_mu[i + k1][j_6 + (jfirst + 4)][k + ifirst + (base + 2)]
                  ;
               _t37 = a_stry[j_6 + 3];
               _t36 = a_mu[i + k1][j_6 + (jfirst + 3)][k + ifirst + (base + 2)]
                  ;
               _t28 = a_stry[j_6 + 1];
               _t27 = a_mu[i + k1][j_6 + (jfirst + 1)][k + ifirst + (base + 2)]
                  ;
               _t32 = a_stry[j_6];
               _t31 = a_mu[i + k1][j_6 + jfirst][k + ifirst + (base + 2)];
               _t21 = a_strx[k + 4];
               _t20 = a_mu[i + k1][j_6 + (jfirst + 2)][k + ifirst + (base + 4)]
                  ;
               _t15 = a_strx[k + 3];
               _t14 = a_mu[i + k1][j_6 + (jfirst + 2)][k + ifirst + (base + 3)]
                  ;
               _t5 = a_strx[k + 1];
               _t4 = a_mu[i + k1][j_6 + (jfirst + 2)][k + ifirst + (base + 1)];
               _t10 = a_strx[k];
               _t9 = a_mu[i + k1][j_6 + (jfirst + 2)][k + (ifirst + base)];
               _t71 = a_u[i + (k1 + 2)][j_6 + (jfirst + 2)][k + (ifirst + 3)][
                  base3 + 3];
               _t83 = a_u[i + (k1 + 2)][j_6 + (jfirst + 2)][k + (ifirst + 1)][
                  base3 + 3];
               _t62 = a_u[i + (k1 + 2)][j_6 + (jfirst + 2)][k + (ifirst + 4)][
                  base3 + 3];
               _t90 = a_u[i + (k1 + 2)][j_6 + (jfirst + 2)][k + ifirst][base3 +
                  3];
               _t68 = a_u[i + (k1 + 1)][j_6 + (jfirst + 2)][k + (ifirst + 3)][
                  base3 + 3];
               _t75 = a_u[i + (k1 + 1)][j_6 + (jfirst + 2)][k + (ifirst + 1)][
                  base3 + 3];
               _t57 = a_u[i + (k1 + 1)][j_6 + (jfirst + 2)][k + (ifirst + 4)][
                  base3 + 3];
               _t88 = a_u[i + (k1 + 1)][j_6 + (jfirst + 2)][k + ifirst][base3 +
                  3];
               _t69 = a_u[i + (k1 + -1)][j_6 + (jfirst + 2)][k + (ifirst + 3)][
                  base3 + 3];
               _t61 = a_u[i + (k1 + -1)][j_6 + (jfirst + 2)][k + (ifirst + 4)][
                  base3 + 3];
               _t89 = a_u[i + (k1 + -1)][j_6 + (jfirst + 2)][k + ifirst][base3 +
                  3];
               _t72 = a_u[i + (k1 + -2)][j_6 + (jfirst + 2)][k + (ifirst + 3)][
                  base3 + 3];
               _t84 = a_u[i + (k1 + -2)][j_6 + (jfirst + 2)][k + (ifirst + 1)][
                  base3 + 3];
               _t64 = a_u[i + (k1 + -2)][j_6 + (jfirst + 2)][k + (ifirst + 4)][
                  base3 + 3];
               _t91 = a_u[i + (k1 + -2)][j_6 + (jfirst + 2)][k + ifirst][base3 +
                  3];
               _t102 = a_u[i + k1][j_6 + (jfirst + 4)][k + (ifirst + 3)][base3 +
                  2];
               _t23 = a_u[i + k1][j_6 + (jfirst + 4)][k + (ifirst + 1)][base3 +
                  2];
               _t97 = a_u[i + k1][j_6 + (jfirst + 4)][k + (ifirst + 4)][base3 +
                  2];
               _t41 = a_u[i + k1][j_6 + (jfirst + 4)][k + ifirst][base3 + 2];
               _t99 = a_u[i + k1][j_6 + (jfirst + 3)][k + (ifirst + 3)][base3 +
                  2];
               _t13 = a_u[i + k1][j_6 + (jfirst + 3)][k + (ifirst + 1)][base3 +
                  2];
               _t95 = a_u[i + k1][j_6 + (jfirst + 3)][k + (ifirst + 4)][base3 +
                  2];
               _t35 = a_u[i + k1][j_6 + (jfirst + 3)][k + ifirst][base3 + 2];
               _t100 = a_u[i + k1][j_6 + (jfirst + 1)][k + (ifirst + 3)][base3 +
                  2];
               _t96 = a_u[i + k1][j_6 + (jfirst + 1)][k + (ifirst + 4)][base3 +
                  2];
               _t39 = a_u[i + k1][j_6 + (jfirst + 1)][k + ifirst][base3 + 2];
               _t103 = a_u[i + k1][j_6 + jfirst][k + (ifirst + 3)][base3 + 2];
               _t25 = a_u[i + k1][j_6 + jfirst][k + (ifirst + 1)][base3 + 2];
               _t98 = a_u[i + k1][j_6 + jfirst][k + (ifirst + 4)][base3 + 2];
               _t45 = a_u[i + k1][j_6 + jfirst][k + ifirst][base3 + 2];
               _t92 = a_lambda[i + k1][j_6 + (jfirst + 2)][k + ifirst + (base +
                  4)];
               _t86 = a_lambda[i + k1][j_6 + (jfirst + 2)][k + ifirst + (base +
                  3)];
               _t76 = a_lambda[i + k1][j_6 + (jfirst + 2)][k + ifirst + (base +
                  1)];
               _t81 = a_lambda[i + k1][j_6 + (jfirst + 2)][k + (ifirst + base)]
                  ;
               _t85 = a_u[i + k1][j_6 + (jfirst + 2)][k + (ifirst + 2)][base3 +
                  1];
               _t79 = - a_u[i + (k1 + -1)][j_6 + (jfirst + 2)][k + (ifirst + 1)
                  ][base3 + 3];
               _t93 = _t74 * _t17 * 0.006944444444444444;
               _t18 = - a_u[i + k1][j_6 + (jfirst + 1)][k + (ifirst + 1)][base3
                  + 2];
               _t51 = _t74 * _t101 * 0.006944444444444444;
               _t67 = _t65 * _t66;
               _t52 = _t7 * _t17;
               _t60 = _t58 * _t59;
               _t73 = _t60 - 0.75 * (_t52 + _t67);
               _t50 = _t48 * _t49;
               _t70 = _t50 + _t67 + 3.0 * (_t60 + _t52);
               _t55 = _t53 * _t54;
               _t63 = _t55 + _t60 + 3.0 * (_t52 + _t50);
               _t56 = _t50 - 0.75 * (_t52 + _t55);
               _t44 = _t42 * _t43;
               _t30 = _t7 * _t101;
               _t38 = _t36 * _t37;
               _t47 = _t38 - 0.75 * (_t30 + _t44);
               _t29 = _t27 * _t28;
               _t46 = _t29 + _t44 + 3.0 * (_t38 + _t30);
               _t33 = _t31 * _t32;
               _t40 = _t33 + _t38 + 3.0 * (_t30 + _t29);
               _t34 = _t29 - 0.75 * (_t30 + _t33);
               _t94 = _t92 * _t21;
               _t80 = _t78 * _t74;
               _t87 = _t86 * _t15;
               _t22 = _t20 * _t21;
               _t8 = _t7 * _t74;
               _t16 = _t14 * _t15;
               _t26 = _t16 - 0.75 * (_t8 + _t22);
               _t77 = _t76 * _t5;
               _t6 = _t4 * _t5;
               _t24 = _t6 + _t22 + 3.0 * (_t16 + _t8);
               _t82 = _t81 * _t10;
               _t11 = _t9 * _t10;
               _t19 = _t11 + _t16 + 3.0 * (_t8 + _t6);
               _t12 = _t6 - 0.75 * (_t8 + _t11);
               a_lu[i + k1][j_6 + (jfirst + 2)][k + (ifirst + 2)][base3 + 1] =
                  0.0 * a_lu[i + k1][j_6 + (jfirst + 2)][k + (ifirst + 2)][
                  base3 + 1] + 1.0 / (h * h) * (0.16666666666666666 * (_t74 * (
                  (2.0 * _t12 + _t77 - 0.75 * (_t80 + _t82)) * (a_u[i + k1][j_6
                  + (jfirst + 2)][k + ifirst][base3 + 1] - _t85) + (2.0 * _t19 +
                  _t82 + _t87 + 3.0 * (_t80 + _t77)) * (a_u[i + k1][j_6 + (
                  jfirst + 2)][k + (ifirst + 1)][base3 + 1] - _t85) + (2.0 *
                  _t24 + _t77 + _t94 + 3.0 * (_t87 + _t80)) * (a_u[i + k1][j_6 +
                  (jfirst + 2)][k + (ifirst + 3)][base3 + 1] - _t85) + (2.0 *
                  _t26 + _t87 - 0.75 * (_t80 + _t94)) * (a_u[i + k1][j_6 + (
                  jfirst + 2)][k + (ifirst + 4)][base3 + 1] - _t85)) + _t101 * (
                  _t34 * (a_u[i + k1][j_6 + jfirst][k + (ifirst + 2)][base3 + 1
                  ] - _t85) + _t40 * (a_u[i + k1][j_6 + (jfirst + 1)][k + (
                  ifirst + 2)][base3 + 1] - _t85) + _t46 * (a_u[i + k1][j_6 + (
                  jfirst + 3)][k + (ifirst + 2)][base3 + 1] - _t85) + _t47 * (
                  a_u[i + k1][j_6 + (jfirst + 4)][k + (ifirst + 2)][base3 + 1] -
                  _t85)) + _t17 * (_t56 * (a_u[i + (k1 + -2)][j_6 + (jfirst + 2
                  )][k + (ifirst + 2)][base3 + 1] - _t85) + _t63 * (a_u[i + (k1
                  + -1)][j_6 + (jfirst + 2)][k + (ifirst + 2)][base3 + 1] - _t85
                  ) + _t70 * (a_u[i + (k1 + 1)][j_6 + (jfirst + 2)][k + (ifirst
                  + 2)][base3 + 1] - _t85) + _t73 * (a_u[i + (k1 + 2)][j_6 + (
                  jfirst + 2)][k + (ifirst + 2)][base3 + 1] - _t85))) + _t51 * (
                  _t81 * (_t45 - _t41 + 8.0 * (- _t39 + _t35)) - 8.0 * (_t76 * (
                  _t25 - _t23 + 8.0 * (_t18 + _t13))) + 8.0 * (_t86 * (_t103 -
                  _t102 + 8.0 * (- _t100 + _t99))) - _t92 * (_t98 - _t97 + 8.0 *
                  (- _t96 + _t95))) + _t93 * (_t81 * (_t91 - _t90 + 8.0 * (-
                  _t89 + _t88)) - 8.0 * (_t76 * (_t84 - _t83 + 8.0 * (_t79 +
                  _t75))) + 8.0 * (_t86 * (_t72 - _t71 + 8.0 * (- _t69 + _t68))
                  ) - _t92 * (_t64 - _t62 + 8.0 * (- _t61 + _t57))) + _t51 * (
                  _t31 * (_t45 - _t98 + 8.0 * (- _t25 + _t103)) - 8.0 * (_t27 *
                  (_t39 - _t96 + 8.0 * (_t18 + _t100))) + 8.0 * (_t36 * (_t35 -
                  _t95 + 8.0 * (- _t13 + _t99))) - _t42 * (_t41 - _t97 + 8.0 * (
                  - _t23 + _t102))) + _t93 * (_t53 * (_t91 - _t64 + 8.0 * (-
                  _t84 + _t72)) - 8.0 * (_t48 * (_t89 - _t61 + 8.0 * (_t79 +
                  _t69))) + 8.0 * (_t58 * (_t88 - _t57 + 8.0 * (- _t75 + _t68))
                  ) - _t65 * (_t90 - _t62 + 8.0 * (- _t83 + _t71))));
               _t126 = a_u[i + (k1 + 2)][j_6 + (jfirst + 3)][k + (ifirst + 2)][
                  base3 + 3];
               _t123 = a_u[i + (k1 + 2)][j_6 + (jfirst + 1)][k + (ifirst + 2)][
                  base3 + 3];
               _t122 = a_u[i + (k1 + 2)][j_6 + (jfirst + 4)][k + (ifirst + 2)][
                  base3 + 3];
               _t120 = a_u[i + (k1 + 2)][j_6 + jfirst][k + (ifirst + 2)][base3 +
                  3];
               _t112 = a_u[i + (k1 + 1)][j_6 + (jfirst + 3)][k + (ifirst + 2)][
                  base3 + 3];
               _t111 = a_u[i + (k1 + 1)][j_6 + (jfirst + 1)][k + (ifirst + 2)][
                  base3 + 3];
               _t108 = a_u[i + (k1 + 1)][j_6 + (jfirst + 4)][k + (ifirst + 2)][
                  base3 + 3];
               _t104 = a_u[i + (k1 + 1)][j_6 + jfirst][k + (ifirst + 2)][base3 +
                  3];
               _t115 = a_u[i + (k1 + -1)][j_6 + (jfirst + 3)][k + (ifirst + 2)]
                  [base3 + 3];
               _t107 = a_u[i + (k1 + -1)][j_6 + (jfirst + 4)][k + (ifirst + 2)]
                  [base3 + 3];
               _t142 = a_u[i + (k1 + -1)][j_6 + jfirst][k + (ifirst + 2)][base3
                  + 3];
               _t125 = a_u[i + (k1 + -2)][j_6 + (jfirst + 3)][k + (ifirst + 2)]
                  [base3 + 3];
               _t135 = a_u[i + (k1 + -2)][j_6 + (jfirst + 1)][k + (ifirst + 2)]
                  [base3 + 3];
               _t109 = a_u[i + (k1 + -2)][j_6 + (jfirst + 4)][k + (ifirst + 2)]
                  [base3 + 3];
               _t119 = a_u[i + (k1 + -2)][j_6 + jfirst][k + (ifirst + 2)][base3
                  + 3];
               _t139 = a_lambda[i + k1][j_6 + (jfirst + 4)][k + ifirst + (base +
                  2)];
               _t134 = a_lambda[i + k1][j_6 + (jfirst + 3)][k + ifirst + (base +
                  2)];
               _t117 = a_lambda[i + k1][j_6 + (jfirst + 1)][k + ifirst + (base +
                  2)];
               _t121 = a_lambda[i + k1][j_6 + jfirst][k + ifirst + (base + 2)];
               _t132 = a_u[i + k1][j_6 + (jfirst + 4)][k + (ifirst + 3)][base3 +
                  1];
               _t138 = a_u[i + k1][j_6 + (jfirst + 4)][k + (ifirst + 1)][base3 +
                  1];
               _t143 = a_u[i + k1][j_6 + (jfirst + 4)][k + (ifirst + 4)][base3 +
                  1];
               _t110 = a_u[i + k1][j_6 + (jfirst + 4)][k + ifirst][base3 + 1];
               _t136 = a_u[i + k1][j_6 + (jfirst + 3)][k + (ifirst + 3)][base3 +
                  1];
               _t114 = a_u[i + k1][j_6 + (jfirst + 3)][k + (ifirst + 1)][base3 +
                  1];
               _t140 = a_u[i + k1][j_6 + (jfirst + 3)][k + (ifirst + 4)][base3 +
                  1];
               _t146 = a_u[i + k1][j_6 + (jfirst + 3)][k + ifirst][base3 + 1];
               _t116 = a_u[i + k1][j_6 + (jfirst + 1)][k + (ifirst + 3)][base3 +
                  1];
               _t113 = a_u[i + k1][j_6 + (jfirst + 1)][k + (ifirst + 4)][base3 +
                  1];
               _t141 = a_u[i + k1][j_6 + (jfirst + 1)][k + ifirst][base3 + 1];
               _t127 = a_u[i + k1][j_6 + jfirst][k + (ifirst + 3)][base3 + 1];
               _t118 = a_u[i + k1][j_6 + jfirst][k + (ifirst + 1)][base3 + 1];
               _t105 = a_u[i + k1][j_6 + jfirst][k + (ifirst + 4)][base3 + 1];
               _t145 = a_u[i + k1][j_6 + jfirst][k + ifirst][base3 + 1];
               _t106 = a_u[i + k1][j_6 + (jfirst + 2)][k + (ifirst + 2)][base3 +
                  2];
               _t128 = - a_u[i + (k1 + -1)][j_6 + (jfirst + 1)][k + (ifirst + 2
                  )][base3 + 3];
               _t124 = _t101 * _t17 * 0.006944444444444444;
               _t130 = - a_u[i + k1][j_6 + (jfirst + 1)][k + (ifirst + 1)][
                  base3 + 1];
               _t144 = _t139 * _t43;
               _t131 = _t78 * _t101;
               _t137 = _t134 * _t37;
               _t129 = _t117 * _t28;
               _t133 = _t121 * _t32;
               a_lu[i + k1][j_6 + (jfirst + 2)][k + (ifirst + 2)][base3 + 2] =
                  0.0 * a_lu[i + k1][j_6 + (jfirst + 2)][k + (ifirst + 2)][
                  base3 + 2] + 1.0 / (h * h) * (0.16666666666666666 * (_t74 * (
                  _t12 * (a_u[i + k1][j_6 + (jfirst + 2)][k + ifirst][base3 + 2
                  ] - _t106) + _t19 * (a_u[i + k1][j_6 + (jfirst + 2)][k + (
                  ifirst + 1)][base3 + 2] - _t106) + _t24 * (a_u[i + k1][j_6 + (
                  jfirst + 2)][k + (ifirst + 3)][base3 + 2] - _t106) + _t26 * (
                  a_u[i + k1][j_6 + (jfirst + 2)][k + (ifirst + 4)][base3 + 2] -
                  _t106)) + _t101 * ((2.0 * _t34 + _t129 - 0.75 * (_t131 + _t133
                  )) * (a_u[i + k1][j_6 + jfirst][k + (ifirst + 2)][base3 + 2] -
                  _t106) + (2.0 * _t40 + _t133 + _t137 + 3.0 * (_t131 + _t129))
                  * (a_u[i + k1][j_6 + (jfirst + 1)][k + (ifirst + 2)][base3 + 2
                  ] - _t106) + (2.0 * _t46 + _t129 + _t144 + 3.0 * (_t137 +
                  _t131)) * (a_u[i + k1][j_6 + (jfirst + 3)][k + (ifirst + 2)][
                  base3 + 2] - _t106) + (2.0 * _t47 + _t137 - 0.75 * (_t131 +
                  _t144)) * (a_u[i + k1][j_6 + (jfirst + 4)][k + (ifirst + 2)][
                  base3 + 2] - _t106)) + _t17 * (_t56 * (a_u[i + (k1 + -2)][j_6
                  + (jfirst + 2)][k + (ifirst + 2)][base3 + 2] - _t106) + _t63 *
                  (a_u[i + (k1 + -1)][j_6 + (jfirst + 2)][k + (ifirst + 2)][
                  base3 + 2] - _t106) + _t70 * (a_u[i + (k1 + 1)][j_6 + (jfirst
                  + 2)][k + (ifirst + 2)][base3 + 2] - _t106) + _t73 * (a_u[i +
                  (k1 + 2)][j_6 + (jfirst + 2)][k + (ifirst + 2)][base3 + 2] -
                  _t106))) + _t51 * (_t9 * (_t145 - _t110 + 8.0 * (- _t141 +
                  _t146)) - 8.0 * (_t4 * (_t118 - _t138 + 8.0 * (_t130 + _t114)
                  )) + 8.0 * (_t14 * (_t127 - _t132 + 8.0 * (- _t116 + _t136)))
                  - _t20 * (_t105 - _t143 + 8.0 * (- _t113 + _t140))) + _t51 * (
                  _t121 * (_t145 - _t105 + 8.0 * (- _t118 + _t127)) - 8.0 * (
                  _t117 * (_t141 - _t113 + 8.0 * (_t130 + _t116))) + 8.0 * (
                  _t134 * (_t146 - _t140 + 8.0 * (- _t114 + _t136))) - _t139 * (
                  _t110 - _t143 + 8.0 * (- _t138 + _t132))) + _t124 * (_t121 * (
                  _t119 - _t120 + 8.0 * (- _t142 + _t104)) - 8.0 * (_t117 * (
                  _t135 - _t123 + 8.0 * (_t128 + _t111))) + 8.0 * (_t134 * (
                  _t125 - _t126 + 8.0 * (- _t115 + _t112))) - _t139 * (_t109 -
                  _t122 + 8.0 * (- _t107 + _t108))) + _t124 * (_t53 * (_t119 -
                  _t109 + 8.0 * (- _t135 + _t125)) - 8.0 * (_t48 * (_t142 -
                  _t107 + 8.0 * (_t128 + _t115))) + 8.0 * (_t58 * (_t104 - _t108
                  + 8.0 * (- _t111 + _t112))) - _t65 * (_t120 - _t122 + 8.0 * (
                  - _t123 + _t126))));
               _t148 = a_u[i + (k1 + 2)][j_6 + (jfirst + 3)][k + (ifirst + 2)][
                  base3 + 2];
               _t150 = a_u[i + (k1 + 2)][j_6 + (jfirst + 1)][k + (ifirst + 2)][
                  base3 + 2];
               _t151 = a_u[i + (k1 + 2)][j_6 + (jfirst + 4)][k + (ifirst + 2)][
                  base3 + 2];
               _t153 = a_u[i + (k1 + 2)][j_6 + jfirst][k + (ifirst + 2)][base3 +
                  2];
               _t154 = a_lambda[i + (k1 + 2)][j_6 + (jfirst + 2)][k + ifirst + (
                  base + 2)];
               _t155 = a_u[i + (k1 + 1)][j_6 + (jfirst + 3)][k + (ifirst + 2)][
                  base3 + 2];
               _t156 = a_u[i + (k1 + 1)][j_6 + (jfirst + 1)][k + (ifirst + 2)][
                  base3 + 2];
               _t157 = a_u[i + (k1 + 1)][j_6 + (jfirst + 4)][k + (ifirst + 2)][
                  base3 + 2];
               _t159 = a_u[i + (k1 + 1)][j_6 + jfirst][k + (ifirst + 2)][base3 +
                  2];
               _t160 = a_lambda[i + (k1 + 1)][j_6 + (jfirst + 2)][k + ifirst + (
                  base + 2)];
               _t162 = a_u[i + (k1 + -1)][j_6 + (jfirst + 3)][k + (ifirst + 2)]
                  [base3 + 2];
               _t165 = a_u[i + (k1 + -1)][j_6 + (jfirst + 4)][k + (ifirst + 2)]
                  [base3 + 2];
               _t167 = a_u[i + (k1 + -1)][j_6 + jfirst][k + (ifirst + 2)][base3
                  + 2];
               _t169 = a_lambda[i + (k1 + -1)][j_6 + (jfirst + 2)][k + ifirst +
                  (base + 2)];
               _t171 = a_u[i + (k1 + -2)][j_6 + (jfirst + 3)][k + (ifirst + 2)]
                  [base3 + 2];
               _t172 = a_u[i + (k1 + -2)][j_6 + (jfirst + 1)][k + (ifirst + 2)]
                  [base3 + 2];
               _t173 = a_u[i + (k1 + -2)][j_6 + (jfirst + 4)][k + (ifirst + 2)]
                  [base3 + 2];
               _t175 = a_u[i + (k1 + -2)][j_6 + jfirst][k + (ifirst + 2)][base3
                  + 2];
               _t176 = a_lambda[i + (k1 + -2)][j_6 + (jfirst + 2)][k + ifirst +
                  (base + 2)];
               _t178 = a_u[i + (k1 + 2)][j_6 + (jfirst + 2)][k + (ifirst + 3)][
                  base3 + 1];
               _t179 = a_u[i + (k1 + 2)][j_6 + (jfirst + 2)][k + (ifirst + 1)][
                  base3 + 1];
               _t180 = a_u[i + (k1 + 2)][j_6 + (jfirst + 2)][k + (ifirst + 4)][
                  base3 + 1];
               _t181 = a_u[i + (k1 + 2)][j_6 + (jfirst + 2)][k + ifirst][base3 +
                  1];
               _t182 = a_u[i + (k1 + 1)][j_6 + (jfirst + 2)][k + (ifirst + 3)][
                  base3 + 1];
               _t184 = a_u[i + (k1 + 1)][j_6 + (jfirst + 2)][k + (ifirst + 1)][
                  base3 + 1];
               _t186 = a_u[i + (k1 + 1)][j_6 + (jfirst + 2)][k + (ifirst + 4)][
                  base3 + 1];
               _t187 = a_u[i + (k1 + 1)][j_6 + (jfirst + 2)][k + ifirst][base3 +
                  1];
               _t147 = a_u[i + (k1 + -1)][j_6 + (jfirst + 2)][k + (ifirst + 3)]
                  [base3 + 1];
               _t149 = a_u[i + (k1 + -1)][j_6 + (jfirst + 2)][k + (ifirst + 4)]
                  [base3 + 1];
               _t152 = a_u[i + (k1 + -1)][j_6 + (jfirst + 2)][k + ifirst][base3
                  + 1];
               _t158 = a_u[i + (k1 + -2)][j_6 + (jfirst + 2)][k + (ifirst + 3)]
                  [base3 + 1];
               _t161 = a_u[i + (k1 + -2)][j_6 + (jfirst + 2)][k + (ifirst + 1)]
                  [base3 + 1];
               _t164 = a_u[i + (k1 + -2)][j_6 + (jfirst + 2)][k + (ifirst + 4)]
                  [base3 + 1];
               _t168 = a_u[i + (k1 + -2)][j_6 + (jfirst + 2)][k + ifirst][base3
                  + 1];
               _t166 = a_u[i + k1][j_6 + (jfirst + 2)][k + (ifirst + 2)][base3 +
                  3];
               _t174 = - a_u[i + (k1 + -1)][j_6 + (jfirst + 1)][k + (ifirst + 2
                  )][base3 + 2];
               _t185 = - a_u[i + (k1 + -1)][j_6 + (jfirst + 2)][k + (ifirst + 1
                  )][base3 + 1];
               _t183 = _t154 * _t66;
               _t163 = _t78 * _t17;
               _t170 = _t160 * _t59;
               _t177 = _t169 * _t49;
               _t188 = _t176 * _t54;
               a_lu[i + k1][j_6 + (jfirst + 2)][k + (ifirst + 2)][base3 + 3] =
                  0.0 * a_lu[i + k1][j_6 + (jfirst + 2)][k + (ifirst + 2)][
                  base3 + 3] + 1.0 / (h * h) * (0.16666666666666666 * (_t74 * (
                  _t12 * (a_u[i + k1][j_6 + (jfirst + 2)][k + ifirst][base3 + 3
                  ] - _t166) + _t19 * (a_u[i + k1][j_6 + (jfirst + 2)][k + (
                  ifirst + 1)][base3 + 3] - _t166) + _t24 * (a_u[i + k1][j_6 + (
                  jfirst + 2)][k + (ifirst + 3)][base3 + 3] - _t166) + _t26 * (
                  a_u[i + k1][j_6 + (jfirst + 2)][k + (ifirst + 4)][base3 + 3] -
                  _t166)) + _t101 * (_t34 * (a_u[i + k1][j_6 + jfirst][k + (
                  ifirst + 2)][base3 + 3] - _t166) + _t40 * (a_u[i + k1][j_6 + (
                  jfirst + 1)][k + (ifirst + 2)][base3 + 3] - _t166) + _t46 * (
                  a_u[i + k1][j_6 + (jfirst + 3)][k + (ifirst + 2)][base3 + 3] -
                  _t166) + _t47 * (a_u[i + k1][j_6 + (jfirst + 4)][k + (ifirst +
                  2)][base3 + 3] - _t166)) + _t17 * ((2.0 * _t56 + _t177 - 0.75
                  * (_t163 + _t188)) * (a_u[i + (k1 + -2)][j_6 + (jfirst + 2)][
                  k + (ifirst + 2)][base3 + 3] - _t166) + (2.0 * _t63 + _t188 +
                  _t170 + 3.0 * (_t163 + _t177)) * (a_u[i + (k1 + -1)][j_6 + (
                  jfirst + 2)][k + (ifirst + 2)][base3 + 3] - _t166) + (2.0 *
                  _t70 + _t177 + _t183 + 3.0 * (_t170 + _t163)) * (a_u[i + (k1 +
                  1)][j_6 + (jfirst + 2)][k + (ifirst + 2)][base3 + 3] - _t166)
                  + (2.0 * _t73 + _t170 - 0.75 * (_t163 + _t183)) * (a_u[i + (
                  k1 + 2)][j_6 + (jfirst + 2)][k + (ifirst + 2)][base3 + 3] -
                  _t166))) + _t93 * (_t9 * (_t168 - _t181 + 8.0 * (- _t152 +
                  _t187)) - 8.0 * (_t4 * (_t161 - _t179 + 8.0 * (_t185 + _t184)
                  )) + 8.0 * (_t14 * (_t158 - _t178 + 8.0 * (- _t147 + _t182)))
                  - _t20 * (_t164 - _t180 + 8.0 * (- _t149 + _t186))) + _t124 *
                  (_t31 * (_t175 - _t153 + 8.0 * (- _t167 + _t159)) - 8.0 * (
                  _t27 * (_t172 - _t150 + 8.0 * (_t174 + _t156))) + 8.0 * (_t36
                  * (_t171 - _t148 + 8.0 * (- _t162 + _t155))) - _t42 * (_t173 -
                  _t151 + 8.0 * (- _t165 + _t157))) + _t93 * (_t176 * (_t168 -
                  _t164 + 8.0 * (- _t161 + _t158)) - 8.0 * (_t169 * (_t152 -
                  _t149 + 8.0 * (_t185 + _t147))) + 8.0 * (_t160 * (_t187 -
                  _t186 + 8.0 * (- _t184 + _t182))) - _t154 * (_t181 - _t180 +
                  8.0 * (- _t179 + _t178))) + _t124 * (_t176 * (_t175 - _t173 +
                  8.0 * (- _t172 + _t171)) - 8.0 * (_t169 * (_t167 - _t165 + 8.0
                  * (_t174 + _t162))) + 8.0 * (_t160 * (_t159 - _t157 + 8.0 * (
                  - _t156 + _t155))) - _t154 * (_t153 - _t151 + 8.0 * (- _t150 +
                  _t148))));
            }
         }
      }
   }
}

/*
 * ../src/rhs4sg.c:1656.6
 */
void mapped_fn2(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided_5, double (* a_acof)[8][6], double (* a_bope)[6
   ], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
{
   core_fn2(ifirst, ilast, jfirst, jlast, kfirst, klast, nk, onesided_5, a_acof
      , a_bope, a_ghcof, a_lu, a_u, a_mu, a_lambda, h, a_strx, a_stry, a_strz,
      k1, k2, base, base3);
}

/*
 * ../src/rhs4sg.c:1669.6
 */
void mapped_fn3(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided_6, double (* a_acof)[8][6], double (* a_bope)[6
   ], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
{
   core_fn3(ifirst, ilast, jfirst, jlast, kfirst, klast, nk, onesided_6, a_acof
      , a_bope, a_ghcof, a_lu, a_u, a_mu, a_lambda, h, a_strx, a_stry, a_strz,
      k1, k2, base, base3);
}

/*
 * ../src/rhs4sg.c:1681.6
 */
void kernel(void)
{
   int* _t1;
   double (* _t2)[25][25][3];
   double (* _t3)[25][25][3];
   double (* _t4)[25][25];
   double (* _t5)[25][25];
   double (* _t6)[25][25][3];
   double (* _t7)[25][25][3];
   double (* _t8)[25][25];
   double (* _t9)[25][25];
   double (* _t10)[25][25][3];
   double (* _t11)[25][25][3];
   double (* _t12)[25][25];
   double (* _t13)[25][25];
   int k1;
   _t5 = lambda;
   _t4 = mu;
   _t3 = u;
   _t2 = lu;
   _t1 = onesided + 5;
   if (onesided[4] == 1) {
      k1 = 7;
   } else {
      k1 = 2;
   }
   int k2;
   if (*_t1 == 1) {
      k2 = 19;
   } else {
      k2 = 22;
   }
   mapped_fn1(0, 24, 0, 24, 0, 24, 25, onesided, acof, bope, ghcof, _t2, _t3,
      _t4, _t5, 0.041666666666666664, strx, stry, strz, k1, k2, 0, -1);
   _t8 = mu;
   _t7 = u;
   _t6 = lu;
   _t9 = lambda;
   if (onesided[4] == 1) {
      mapped_fn2(0, 24, 0, 24, 0, 24, 25, onesided, acof, bope, ghcof, _t6, _t7
         , _t8, _t9, 0.041666666666666664, strx, stry, strz, k1, k2, 0, -1);
   }
   _t12 = mu;
   _t11 = u;
   _t10 = lu;
   _t13 = lambda;
   if (*_t1 == 1) {
      mapped_fn3(0, 24, 0, 24, 0, 24, 25, onesided, acof, bope, ghcof, _t10,
         _t11, _t12, _t13, 0.041666666666666664, strx, stry, strz, k1, k2, 0,
         -1);
   }
}

/*
 * ../src/rhs4sg.c:1723.13
 */
static void unmapped_kernel(double (* lu_1)[25][25][3])
{
   int* _t1;
   double (* _t2)[25][25][3];
   double (* _t3)[25][25];
   double (* _t4)[25][25];
   double (* _t5)[25][25][3];
   double (* _t6)[25][25];
   double (* _t7)[25][25];
   double (* _t8)[25][25][3];
   double (* _t9)[25][25];
   double (* _t10)[25][25];
   int k1;
   _t4 = lambda;
   _t3 = mu;
   _t2 = u;
   _t1 = onesided + 5;
   if (onesided[4] == 1) {
      k1 = 7;
   } else {
      k1 = 2;
   }
   int k2;
   if (*_t1 == 1) {
      k2 = 19;
   } else {
      k2 = 22;
   }
   core_fn1(0, 24, 0, 24, 0, 24, 25, onesided, acof, bope, ghcof, lu_1, _t2,
      _t3, _t4, 0.041666666666666664, strx, stry, strz, k1, k2, 0, -1);
   _t6 = mu;
   _t5 = u;
   _t7 = lambda;
   if (onesided[4] == 1) {
      core_fn2(0, 24, 0, 24, 0, 24, 25, onesided, acof, bope, ghcof, lu_1, _t5,
          _t6, _t7, 0.041666666666666664, strx, stry, strz, k1, k2, 0, -1);
   }
   _t9 = mu;
   _t8 = u;
   _t10 = lambda;
   if (*_t1 == 1) {
      core_fn3(0, 24, 0, 24, 0, 24, 25, onesided, acof, bope, ghcof, lu_1, _t8,
          _t9, _t10, 0.041666666666666664, strx, stry, strz, k1, k2, 0, -1);
   }
}

/*
 * ../src/rhs4sg.c:1765.6
 */
void show(void)
{
}

/*
 * ../src/rhs4sg.c:1768.5
 */
int check(void)
{
   double (* _t1)[25][25][3];
   int k;
   _t1 = (double (*)[25][25][3])calloc(46875ul, 8ul);
   unmapped_kernel(_t1);
   int check_result;
   for (k = 0; k < 25; k++) {
      double (* _t2)[25][3];
      int j_7;
      for (_t2 = _t1[k], j_7 = 0; j_7 < 25; j_7++) {
         double (* _t3)[3];
         int i;
         for (_t3 = _t2[j_7], i = 0; i < 25; i++) {
            double* _t4;
            int c;
            _t4 = _t3[i];
            c = 0;
            while (1) {
               double* _t5;
               _t5 = lu[k][j_7][i];
               if (c < 3) {
                  if (!(fequal(_t5[c], _t4[c]) != 0))
                     break;
                  c++;
               } else
                  goto LL10;
            }
            check_result = 1;
LL12:
            return check_result;
LL10:;
         }
      }
   }
   check_result = 0;
   goto LL12;
}

/*
 * ../src/rhs4sg.c:1790.8
 */
double flops_per_trial(void)
{
   return 6167826.0;
}


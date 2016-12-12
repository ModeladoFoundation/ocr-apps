/*
 * Optimization PARSE[-I//usr/include, -I/home/pradelle/work/pca-ocr-db-autodec/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -I../include, -I/home/pradelle/work/pca-ocr-db-autodec/runtime/codelet/ocr/inc, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DOCR_TARGET, --c99, -D__RSTREAM_CC__, ../src/rhs4sg.c] (883ms)
 * Optimization SSA (234ms)
 * Optimization CCP (158ms)
 * Optimization DCE (41ms)
 * Optimization Raise[-liftnone, -stacktrace] (6484ms)
 *   Optimization CCP (49ms)
 *   Optimization DCE (6ms)
 *   Optimization DeSSA (3006ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (15ms)
 * Optimization PolyhedralMapperNew[-stacktrace, no-simplify-loop, no-spatial-layout, stacktrace=true, scal-dep, threadf=reduce_create=true, as=scopes, as=force_identity=3, tile=sizes={1,21,21}, logfile=rhs4sg-mapper.log] (46050ms)
 * Optimization Lower[-stacktrace] (604ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (5ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization InsertMapperAnnotations (3ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization DCE (24ms)
 * Optimization SyntaxPrepass (49ms)
 * Optimization OSR (2009ms)
 * Optimization DCE (41ms)
 * Optimization DeSSA (11614ms)
 */
#include <stdlib.h>

#include <rstream_ocr.h>
#include <ocr.h>
#define ENABLE_EXTENSION_LEGACY
#include <extensions/ocr-legacy.h>
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_37;
union __args_mapped_fn1_dpbr4_38;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_39;
union __args_mapped_fn1_dpbr_main_40;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct __anon_37 {
   double* h;
   double (* a_mu)[25][25];
   double* a_strx;
   double (* a_lu)[25][25][3];
   double (* a_u)[25][25][3];
   double (* a_lambda)[25][25];
   double* a_stry;
   double* a_strz;
   int ifirst;
   int ilast;
   int jfirst;
   int jlast;
   int kfirst;
   int klast;
   int nk;
   int k1;
   int k2;
   int base;
   int base3;
   int IT0;
   int IT1;
   int IT2;
};
union __args_mapped_fn1_dpbr4_38 {
   struct __anon_37 data;
   double padding[15];
};
struct __anon_39 {
   double* h;
   double (* a_mu)[25][25];
   double* a_strx;
   double (* a_lu)[25][25][3];
   double (* a_u)[25][25][3];
   double (* a_lambda)[25][25];
   double* a_stry;
   double* a_strz;
   int ifirst;
   int ilast;
   int jfirst;
   int jlast;
   int kfirst;
   int klast;
   int nk;
   int k1;
   int k2;
   int base;
   int base3;
};
union __args_mapped_fn1_dpbr_main_40 {
   struct __anon_39 data;
   double padding[14];
};
/*
 * Forward declarations of functions
 */
int fequal(double, double);
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
static void mapped_fn1_1(int ifirst, int ilast, int jfirst, int jlast, int
   kfirst, int klast, int nk, int* onesided, double (* a_acof)[8][6], double (*
    a_bope)[6], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25]
   [25][3], double (* a_mu)[25][25], double (* a_lambda)[25][25], double h,
   double* a_strx, double* a_stry, double* a_strz, int k1, int k2, int base,
   int base3);
static void mapped_fn1_2(int ifirst, int ilast, int jfirst, int jlast, int
   kfirst, int klast, int nk, int* onesided, double (* a_acof)[8][6], double (*
    a_bope)[6], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25]
   [25][3], double (* a_mu)[25][25], double (* a_lambda)[25][25], double h,
   double* a_strx, double* a_stry, double* a_strz, int k1, int k2, int base,
   int base3);
void mapped_fn1_dpbr(int ifirst, int ilast, int jfirst, int jlast, int kfirst,
   int klast, int nk, int* onesided, double (* a_acof)[8][6], double (* a_bope)
   [6], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
    double (* a_mu)[25][25], double (* a_lambda)[25][25], double* h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
   ;
static ocrGuid_t mapped_fn1_dpbr4(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
ocrGuid_t rocrAlloc(void**, unsigned long);
static ocrGuid_t mapped_fn1_dpbr_main(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_fn1_async(void* event, int ifirst, int ilast, int jfirst, int jlast
   , int kfirst, int klast, int nk, int* onesided, double (* a_acof)[8][6],
   double (* a_bope)[6], double* a_ghcof, double (* a_lu)[25][25][3], double (*
    a_u)[25][25][3], double (* a_mu)[25][25], double (* a_lambda)[25][25],
   double h, double* a_strx, double* a_stry, double* a_strz, int k1, int k2,
   int base, int base3);
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
 * ../src/rhs4sg.c:48.13
 */
static void get_data(double x, double y, double z, double* u_1, double* v,
   double* w, double* mu_1, double* lambda_1, double* rho_1)
{
   *lambda_1 = cos(x) * pow(sin(3.0 * y), 2.0) * cos(z);
   *mu_1 = sin(3.0 * x) * sin(y) * sin(z);
   *rho_1 = x * x * x + 1.0 + y * y + z * z;
   *u_1 = cos(x * x) * sin(y * x) * z * z;
   *v = sin(x) * cos(y * y) * sin(z);
   *w = cos(x * y) * sin(z * y);
}

/*
 * ../src/rhs4sg.c:61.13
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
   _t16 = z * z;
   _t31 = y * y;
   _t11 = x * x;
   _t45 = 1.0 / (_t11 * x + 1.0 + _t31 + _t16);
   _t14 = y * x;
   _t15 = sin(_t14);
   _t19 = cos(_t11);
   _t30 = _t19 * _t15;
   _t34 = z * y;
   _t35 = cos(_t34);
   _t38 = _t15 * _t31 * _t35;
   _t4 = sin(z);
   _t2 = sin(y);
   _t1 = 3.0 * x;
   _t23 = sin(_t1);
   _t24 = _t23 * _t2;
   _t43 = sin(_t34);
   _t44 = - _t15 * y * _t43 + 2.0 * _t30 * z;
   _t9 = cos(z);
   _t17 = _t15 * _t16;
   _t29 = _t19 * _t11 * _t17;
   _t42 = y * _t4;
   _t32 = sin(_t31);
   _t25 = cos(x);
   _t20 = cos(_t14);
   _t21 = _t19 * _t20;
   _t40 = cos(_t31);
   _t41 = _t4 * (_t25 * _t40 * _t4 + _t21 * x * _t16);
   _t39 = _t23 * cos(y);
   _t6 = 3.0 * y;
   _t7 = sin(_t6);
   _t8 = _t7 * _t7;
   _t26 = _t25 * _t8;
   _t27 = _t26 * _t9;
   _t36 = _t20 * _t35;
   _t37 = _t36 * y;
   _t5 = sin(x);
   _t10 = _t5 * _t8 * _t9;
   _t33 = _t32 * y * _t4;
   _t12 = sin(_t11);
   _t13 = _t12 * x;
   _t28 = 2.0 * _t24 * _t4 + _t27;
   _t22 = _t21 * y * _t16;
   _t18 = 2.0 * _t13 * _t17;
   _t3 = cos(_t1) * _t2;
   eqs_1[0] = ((6.0 * _t3 * _t4 - _t10) * (- _t18 + _t22) + _t28 * (-4.0 * _t29
      - 2.0 * _t12 * _t15 * _t16 - 4.0 * _t13 * _t20 * y * _t16 - _t30 * _t31 *
      _t16) + 2.0 * (_t5 * _t5) * _t8 * _t9 * _t33 - 2.0 * (_t25 * _t25) * _t8 *
      _t9 * _t33 - _t10 * _t37 - _t27 * _t38 + _t39 * _t41 + _t24 * _t4 * (-2.0
      * _t25 * _t32 * _t42 - _t29) + _t24 * _t9 * _t44 + _t24 * _t4 * (- _t38 +
      2.0 * _t30)) * _t45;
   _t46 = _t5 * _t40;
   _t47 = _t46 * _t4;
   _t57 = _t20 * _t43;
   _t58 = _t57 * _t34;
   _t55 = _t35 * y;
   _t54 = _t15 * x;
   _t56 = _t54 * _t55;
   _t59 = - _t54 * _t43 + _t36 * z + _t46 * _t9;
   _t50 = _t21 * _t16;
   _t49 = _t30 * _t14 * _t16;
   _t48 = 2.0 * _t12 * _t11 * _t20 * _t16;
   _t52 = cos(_t6);
   _t51 = _t25 * _t7;
   _t53 = _t28 * _t5;
   eqs_1[1] = (3.0 * _t3 * _t41 + _t24 * _t4 * (- _t47 - _t48 - _t49 + _t50) -
      2.0 * (2.0 * _t39 * _t4 + 6.0 * _t51 * _t9 * _t52) * _t5 * _t33 - 4.0 *
      _t53 * _t40 * _t31 * _t4 - 2.0 * _t53 * _t32 * _t4 + 6.0 * _t51 * _t9 * (
      - _t18 + _t22 + _t37) * _t52 + _t26 * _t9 * (- _t48 - _t49 + _t50 - _t56 -
      _t58 + _t36) + _t24 * _t9 * _t59 + _t24 * _t4 * (- _t56 - _t58 + _t36 -
      _t47)) * _t45;
   _t62 = _t5 * _t32;
   _t63 = 2.0 * _t62 * y * _t9;
   _t61 = 2.0 * _t21 * _t34;
   _t60 = 4.0 * _t13 * _t15 * z;
   eqs_1[2] = (3.0 * _t3 * _t4 * _t44 + _t24 * _t4 * (- _t20 * _t31 * _t43 -
      _t60 + _t61) + _t39 * _t4 * _t59 + _t24 * _t4 * (- _t20 * _t11 * _t43 -
      2.0 * _t54 * _t35 * z - _t57 * _t16 - _t63) + (2.0 * _t24 * _t9 - _t26 *
      _t4) * _t20 * _t55 - _t28 * _t20 * _t43 * _t31 - _t26 * _t4 * (- _t18 +
      _t22 - 2.0 * _t62 * _t42) + _t26 * _t9 * (- _t60 + _t61 - _t63)) * _t45;
}

/*
 * ../src/rhs4sg.c:217.29
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
 * ../src/rhs4sg.c:314.29
 */
static int bopext4th_(double* bop, double* bope_1)
{
   double* _t1;
   double* _t2;
   for (_t2 = bop + -5, _t1 = bope_1 + -7, j = 1; j <= 8; j++) {
      for (i__ = 1; i__ <= 6; i__++) {
         _t1[i__ + j * 6] = 0.0;
      }
   }
   for (j = 1; j <= 6; j++) {
      for (i__ = 1; i__ <= 4; i__++) {
         _t1[i__ + j * 6] = _t2[i__ + (j << 2)];
      }
   }
   d4a = 0.6666666666666666;
   d4b = -0.08333333333333333;
   _t1[23] = - d4b;
   _t1[29] = - d4a;
   _t1[41] = d4a;
   _t1[47] = d4b;
   _t1[30] = - d4b;
   _t1[36] = - d4a;
   _t1[48] = d4a;
   _t1[54] = d4b;
   return 0;
}

/*
 * ../src/rhs4sg.c:348.29
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
 * ../src/rhs4sg.c:753.6
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
 * ../src/rhs4sg.c:770.6
 */
void initialize(void)
{
   double m_iop[5];
   double m_iop2[5];
   double bop[24];
   double m_bop2[24];
   double gh2;
   double m_hnorm[4];
   double m_sbop[5];
   int k;
   int i;
   int j_1;
   int k_1;
   for (k = 0; k < 25; k++) {
      int j_2;
      for (j_2 = 0; j_2 < 25; j_2++) {
         int i_1;
         for (i_1 = 0; i_1 < 25; i_1++) {
            int c;
            get_data((double)i_1 * 0.041666666666666664, (double)j_2 *
               0.041666666666666664, (double)k * 0.041666666666666664, u[k][j_2
               ][i_1] + 0, u[k][j_2][i_1] + 1, u[k][j_2][i_1] + 2, mu[k][j_2] +
               i_1, lambda[k][j_2] + i_1, rho[k][j_2] + i_1);
            fg((double)i_1 * 0.041666666666666664, (double)j_2 *
               0.041666666666666664, (double)k * 0.041666666666666664, eqs[k][
               j_2][i_1] + 0);
            for (c = 0; c < 3; c++) {
               lu[k][j_2][i_1][c] = 0.0;
            }
         }
      }
   }
   for (i = 0; i < 25; i++) {
      strx[i] = 1.0;
   }
   for (j_1 = 0; j_1 < 25; j_1++) {
      stry[j_1] = 1.0;
   }
   for (k_1 = 0; k_1 < 25; k_1++) {
      strz[k_1] = 1.0;
   }
   varcoeffs4_((double*)acof, ghcof);
   wavepropbop_4__(m_iop, m_iop2, bop, m_bop2, &gh2, m_hnorm, m_sbop);
   bopext4th_(bop, (double*)bope);
}

/*
 * ../src/rhs4sg.c:839.6
 */
void core_fn1(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided_1, double (* a_acof)[8][6], double (* a_bope)[6
   ], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
{
   double _t1;
   int k;
   int k_1;
   int k_2;
   int k_3;
   int k_4;
   for (_t1 = 1.0 / (h * h),
        k_1 = k1 + -1,
        k_2 = k1 + -2,
        k_3 = k1 + 1,
        k_4 = k1 + 2,
        k = k1;
        k <= k2;
        k++,
        k_1++,
        k_2++,
        k_3++,
        k_4++) {
      int j_3;
      int j_4;
      int j_5;
      int j_6;
      int j_7;
      for (j_3 = jfirst + 2,
           j_7 = jfirst + 4,
           j_4 = jfirst + 1,
           j_6 = jfirst + 3,
           j_5 = jfirst + 0;
           j_3 <= jlast + -2;
           j_3++,
           j_4++,
           j_5++,
           j_6++,
           j_7++) {
         int i;
         int i_1;
         int i_2;
         int i_3;
         int i_4;
         int i_5;
         int i_6;
         int i_7;
         int i_8;
         int i_9;
         for (i = ifirst + 2,
              i_1 = i + base,
              i_2 = i_1 + -1,
              i_9 = ifirst + 4,
              i_3 = ifirst + 1,
              i_4 = i_1 + -2,
              i_7 = ifirst + 3,
              i_5 = ifirst + 0,
              i_6 = i_1 + 1,
              i_8 = i_1 + 2;
              i <= ilast + -2;
              i++,
              i_9++,
              i_8++,
              i_7++,
              i_6++,
              i_5++,
              i_4++,
              i_3++,
              i_2++,
              i_1++) {
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
            double _t233;
            double _t234;
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
            _t14 = a_strx[i + - ifirst];
            _t15 = a_u[k][j_3][i_5][base3 + 2];
            _t16 = a_u[k][j_3][i][base3 + 2];
            _t17 = a_u[k][j_3][i_3][base3 + 2];
            _t18 = a_u[k][j_3][i][base3 + 2];
            _t19 = a_u[k][j_3][i_7][base3 + 2];
            _t20 = a_u[k][j_3][i][base3 + 2];
            _t21 = a_u[k][j_3][i_9][base3 + 2];
            _t22 = a_u[k][j_3][i][base3 + 2];
            _t23 = a_stry[j_3 + - jfirst];
            _t24 = a_lambda[k][j_4][i_1];
            _t25 = a_stry[j_4 + - jfirst];
            _t26 = a_lambda[k][j_3][i_1];
            _t27 = a_stry[j_3 + - jfirst];
            _t28 = a_lambda[k][j_5][i_1];
            _t29 = a_stry[j_5 + - jfirst];
            _t30 = a_u[k][j_5][i][base3 + 2];
            _t31 = a_u[k][j_3][i][base3 + 2];
            _t32 = a_lambda[k][j_5][i_1];
            _t33 = a_stry[j_5 + - jfirst];
            _t34 = a_lambda[k][j_6][i_1];
            _t35 = a_stry[j_6 + - jfirst];
            _t36 = a_lambda[k][j_3][i_1];
            _t37 = a_stry[j_3 + - jfirst];
            _t38 = a_lambda[k][j_4][i_1];
            _t39 = a_stry[j_4 + - jfirst];
            _t40 = a_u[k][j_4][i][base3 + 2];
            _t41 = a_u[k][j_3][i][base3 + 2];
            _t42 = a_lambda[k][j_4][i_1];
            _t43 = a_stry[j_4 + - jfirst];
            _t44 = a_lambda[k][j_7][i_1];
            _t45 = a_stry[j_7 + - jfirst];
            _t46 = a_lambda[k][j_6][i_1];
            _t47 = a_stry[j_6 + - jfirst];
            _t48 = a_lambda[k][j_3][i_1];
            _t49 = a_stry[j_3 + - jfirst];
            _t50 = a_u[k][j_6][i][base3 + 2];
            _t51 = a_u[k][j_3][i][base3 + 2];
            _t52 = a_lambda[k][j_6][i_1];
            _t53 = a_stry[j_6 + - jfirst];
            _t54 = a_lambda[k][j_3][i_1];
            _t55 = a_stry[j_3 + - jfirst];
            _t56 = a_lambda[k][j_7][i_1];
            _t57 = a_stry[j_7 + - jfirst];
            _t58 = a_u[k][j_7][i][base3 + 2];
            _t59 = a_u[k][j_3][i][base3 + 2];
            _t60 = a_strz[k + - kfirst];
            _t61 = a_u[k_2][j_3][i][base3 + 2];
            _t62 = a_u[k][j_3][i][base3 + 2];
            _t63 = a_u[k_1][j_3][i][base3 + 2];
            _t64 = a_u[k][j_3][i][base3 + 2];
            _t65 = a_u[k_3][j_3][i][base3 + 2];
            _t66 = a_u[k][j_3][i][base3 + 2];
            _t67 = a_u[k_4][j_3][i][base3 + 2];
            _t68 = a_u[k][j_3][i][base3 + 2];
            _t69 = a_strx[i + - ifirst];
            _t70 = a_u[k][j_3][i_5][base3 + 3];
            _t71 = a_u[k][j_3][i][base3 + 3];
            _t72 = a_u[k][j_3][i_3][base3 + 3];
            _t73 = a_u[k][j_3][i][base3 + 3];
            _t74 = a_u[k][j_3][i_7][base3 + 3];
            _t75 = a_u[k][j_3][i][base3 + 3];
            _t76 = a_u[k][j_3][i_9][base3 + 3];
            _t77 = a_u[k][j_3][i][base3 + 3];
            _t78 = a_stry[j_3 + - jfirst];
            _t79 = a_u[k][j_5][i][base3 + 3];
            _t80 = a_u[k][j_3][i][base3 + 3];
            _t81 = a_u[k][j_4][i][base3 + 3];
            _t82 = a_u[k][j_3][i][base3 + 3];
            _t83 = a_u[k][j_6][i][base3 + 3];
            _t84 = a_u[k][j_3][i][base3 + 3];
            _t85 = a_u[k][j_7][i][base3 + 3];
            _t86 = a_u[k][j_3][i][base3 + 3];
            _t87 = a_strz[k + - kfirst];
            _t88 = a_lambda[k_1][j_3][i_1];
            _t89 = a_strz[k_1 + - kfirst];
            _t90 = a_lambda[k][j_3][i_1];
            _t91 = a_strz[k + - kfirst];
            _t92 = a_lambda[k_2][j_3][i_1];
            _t93 = a_strz[k_2 + - kfirst];
            _t94 = a_u[k_2][j_3][i][base3 + 3];
            _t95 = a_u[k][j_3][i][base3 + 3];
            _t96 = a_lambda[k_2][j_3][i_1];
            _t97 = a_strz[k_2 + - kfirst];
            _t98 = a_lambda[k_3][j_3][i_1];
            _t99 = a_strz[k_3 + - kfirst];
            _t100 = a_lambda[k][j_3][i_1];
            _t101 = a_strz[k + - kfirst];
            _t102 = a_lambda[k_1][j_3][i_1];
            _t103 = a_strz[k_1 + - kfirst];
            _t104 = a_u[k_1][j_3][i][base3 + 3];
            _t105 = a_u[k][j_3][i][base3 + 3];
            _t106 = a_lambda[k_1][j_3][i_1];
            _t107 = a_strz[k_1 + - kfirst];
            _t108 = a_lambda[k_4][j_3][i_1];
            _t109 = a_strz[k_4 + - kfirst];
            _t110 = a_lambda[k_3][j_3][i_1];
            _t111 = a_strz[k_3 + - kfirst];
            _t112 = a_lambda[k][j_3][i_1];
            _t113 = a_strz[k + - kfirst];
            _t114 = a_u[k_3][j_3][i][base3 + 3];
            _t115 = a_u[k][j_3][i][base3 + 3];
            _t116 = a_lambda[k_3][j_3][i_1];
            _t117 = a_strz[k_3 + - kfirst];
            _t118 = a_lambda[k][j_3][i_1];
            _t119 = a_strz[k + - kfirst];
            _t120 = a_lambda[k_4][j_3][i_1];
            _t121 = a_strz[k_4 + - kfirst];
            _t122 = a_u[k_4][j_3][i][base3 + 3];
            _t123 = a_u[k][j_3][i][base3 + 3];
            _t124 = a_strx[i + - ifirst];
            _t125 = a_stry[j_3 + - jfirst];
            _t126 = a_mu[k][j_3][i_4];
            _t127 = a_u[k][j_5][i_5][base3 + 1];
            _t128 = a_u[k][j_7][i_5][base3 + 1];
            _t129 = a_u[k][j_4][i_5][base3 + 1];
            _t130 = a_u[k][j_6][i_5][base3 + 1];
            _t131 = a_mu[k][j_3][i_2];
            _t132 = a_u[k][j_5][i_3][base3 + 1];
            _t133 = a_u[k][j_7][i_3][base3 + 1];
            _t134 = a_u[k][j_4][i_3][base3 + 1];
            _t135 = a_u[k][j_6][i_3][base3 + 1];
            _t136 = a_mu[k][j_3][i_6];
            _t137 = a_u[k][j_5][i_7][base3 + 1];
            _t138 = a_u[k][j_7][i_7][base3 + 1];
            _t139 = a_u[k][j_4][i_7][base3 + 1];
            _t140 = a_u[k][j_6][i_7][base3 + 1];
            _t141 = a_mu[k][j_3][i_8];
            _t142 = a_u[k][j_5][i_9][base3 + 1];
            _t143 = a_u[k][j_7][i_9][base3 + 1];
            _t144 = a_u[k][j_4][i_9][base3 + 1];
            _t145 = a_u[k][j_6][i_9][base3 + 1];
            _t146 = a_strx[i + - ifirst];
            _t147 = a_stry[j_3 + - jfirst];
            _t148 = a_lambda[k][j_5][i_1];
            _t149 = a_u[k][j_5][i_5][base3 + 1];
            _t150 = a_u[k][j_5][i_9][base3 + 1];
            _t151 = a_u[k][j_5][i_3][base3 + 1];
            _t152 = a_u[k][j_5][i_7][base3 + 1];
            _t153 = a_lambda[k][j_4][i_1];
            _t154 = a_u[k][j_4][i_5][base3 + 1];
            _t155 = a_u[k][j_4][i_9][base3 + 1];
            _t156 = a_u[k][j_4][i_3][base3 + 1];
            _t157 = a_u[k][j_4][i_7][base3 + 1];
            _t158 = a_lambda[k][j_6][i_1];
            _t159 = a_u[k][j_6][i_5][base3 + 1];
            _t160 = a_u[k][j_6][i_9][base3 + 1];
            _t161 = a_u[k][j_6][i_3][base3 + 1];
            _t162 = a_u[k][j_6][i_7][base3 + 1];
            _t163 = a_lambda[k][j_7][i_1];
            _t164 = a_u[k][j_7][i_5][base3 + 1];
            _t165 = a_u[k][j_7][i_9][base3 + 1];
            _t166 = a_u[k][j_7][i_3][base3 + 1];
            _t167 = a_u[k][j_7][i_7][base3 + 1];
            _t168 = a_stry[j_3 + - jfirst];
            _t169 = a_strz[k + - kfirst];
            _t170 = a_lambda[k][j_5][i_1];
            _t171 = a_u[k_2][j_5][i][base3 + 3];
            _t172 = a_u[k_4][j_5][i][base3 + 3];
            _t173 = a_u[k_1][j_5][i][base3 + 3];
            _t174 = a_u[k_3][j_5][i][base3 + 3];
            _t175 = a_lambda[k][j_4][i_1];
            _t176 = a_u[k_2][j_4][i][base3 + 3];
            _t177 = a_u[k_4][j_4][i][base3 + 3];
            _t178 = a_u[k_1][j_4][i][base3 + 3];
            _t179 = a_u[k_3][j_4][i][base3 + 3];
            _t180 = a_lambda[k][j_6][i_1];
            _t181 = a_u[k_2][j_6][i][base3 + 3];
            _t182 = a_u[k_4][j_6][i][base3 + 3];
            _t183 = a_u[k_1][j_6][i][base3 + 3];
            _t184 = a_u[k_3][j_6][i][base3 + 3];
            _t185 = a_lambda[k][j_7][i_1];
            _t186 = a_u[k_2][j_7][i][base3 + 3];
            _t187 = a_u[k_4][j_7][i][base3 + 3];
            _t188 = a_u[k_1][j_7][i][base3 + 3];
            _t189 = a_u[k_3][j_7][i][base3 + 3];
            _t190 = a_stry[j_3 + - jfirst];
            _t191 = a_strz[k + - kfirst];
            _t192 = a_mu[k_2][j_3][i_1];
            _t193 = a_u[k_2][j_5][i][base3 + 3];
            _t194 = a_u[k_2][j_7][i][base3 + 3];
            _t195 = a_u[k_2][j_4][i][base3 + 3];
            _t196 = a_u[k_2][j_6][i][base3 + 3];
            _t197 = a_mu[k_1][j_3][i_1];
            _t198 = a_u[k_1][j_5][i][base3 + 3];
            _t199 = a_u[k_1][j_7][i][base3 + 3];
            _t200 = a_u[k_1][j_4][i][base3 + 3];
            _t201 = a_u[k_1][j_6][i][base3 + 3];
            _t202 = a_mu[k_3][j_3][i_1];
            _t203 = a_u[k_3][j_5][i][base3 + 3];
            _t204 = a_u[k_3][j_7][i][base3 + 3];
            _t205 = a_u[k_3][j_4][i][base3 + 3];
            _t206 = a_u[k_3][j_6][i][base3 + 3];
            _t207 = a_mu[k_4][j_3][i_1];
            _t208 = a_u[k_4][j_5][i][base3 + 3];
            _t209 = a_u[k_4][j_7][i][base3 + 3];
            _t210 = a_u[k_4][j_4][i][base3 + 3];
            _t211 = a_u[k_4][j_6][i][base3 + 3];
            _t212 = a_strx[i + - ifirst];
            _t213 = a_strz[k + - kfirst];
            _t214 = a_mu[k][j_3][i_4];
            _t215 = a_u[k_2][j_3][i_5][base3 + 1];
            _t216 = a_u[k_4][j_3][i_5][base3 + 1];
            _t217 = a_u[k_1][j_3][i_5][base3 + 1];
            _t218 = a_u[k_3][j_3][i_5][base3 + 1];
            _t219 = a_mu[k][j_3][i_2];
            _t220 = a_u[k_2][j_3][i_3][base3 + 1];
            _t221 = a_u[k_4][j_3][i_3][base3 + 1];
            _t222 = a_u[k_1][j_3][i_3][base3 + 1];
            _t223 = a_u[k_3][j_3][i_3][base3 + 1];
            _t224 = a_mu[k][j_3][i_6];
            _t225 = a_u[k_2][j_3][i_7][base3 + 1];
            _t226 = a_u[k_4][j_3][i_7][base3 + 1];
            _t227 = a_u[k_1][j_3][i_7][base3 + 1];
            _t228 = a_u[k_3][j_3][i_7][base3 + 1];
            _t229 = a_mu[k][j_3][i_8];
            _t230 = a_u[k_2][j_3][i_9][base3 + 1];
            _t231 = a_u[k_4][j_3][i_9][base3 + 1];
            _t232 = a_u[k_1][j_3][i_9][base3 + 1];
            _t233 = a_u[k_3][j_3][i_9][base3 + 1];
            _t234 = a_stry[j_3 + - jfirst];
            _t235 = a_strz[k + - kfirst];
            _t236 = a_mu[k][j_5][i_1];
            _t237 = a_u[k_2][j_5][i][base3 + 2];
            _t238 = a_u[k_4][j_5][i][base3 + 2];
            _t239 = a_u[k_1][j_5][i][base3 + 2];
            _t240 = a_u[k_3][j_5][i][base3 + 2];
            _t241 = a_mu[k][j_4][i_1];
            _t242 = a_u[k_2][j_4][i][base3 + 2];
            _t243 = a_u[k_4][j_4][i][base3 + 2];
            _t244 = a_u[k_1][j_4][i][base3 + 2];
            _t245 = a_u[k_3][j_4][i][base3 + 2];
            _t246 = a_mu[k][j_6][i_1];
            _t247 = a_u[k_2][j_6][i][base3 + 2];
            _t248 = a_u[k_4][j_6][i][base3 + 2];
            _t249 = a_u[k_1][j_6][i][base3 + 2];
            _t250 = a_u[k_3][j_6][i][base3 + 2];
            _t251 = a_mu[k][j_7][i_1];
            _t252 = a_u[k_2][j_7][i][base3 + 2];
            _t253 = a_u[k_4][j_7][i][base3 + 2];
            _t254 = a_u[k_1][j_7][i][base3 + 2];
            _t255 = a_u[k_3][j_7][i][base3 + 2];
            _t256 = a_strx[i + - ifirst];
            _t257 = a_strz[k + - kfirst];
            _t258 = a_lambda[k_2][j_3][i_1];
            _t259 = a_u[k_2][j_3][i_5][base3 + 1];
            _t260 = a_u[k_2][j_3][i_9][base3 + 1];
            _t261 = a_u[k_2][j_3][i_3][base3 + 1];
            _t262 = a_u[k_2][j_3][i_7][base3 + 1];
            _t263 = a_lambda[k_1][j_3][i_1];
            _t264 = a_u[k_1][j_3][i_5][base3 + 1];
            _t265 = a_u[k_1][j_3][i_9][base3 + 1];
            _t266 = a_u[k_1][j_3][i_3][base3 + 1];
            _t267 = a_u[k_1][j_3][i_7][base3 + 1];
            _t268 = a_lambda[k_3][j_3][i_1];
            _t269 = a_u[k_3][j_3][i_5][base3 + 1];
            _t270 = a_u[k_3][j_3][i_9][base3 + 1];
            _t271 = a_u[k_3][j_3][i_3][base3 + 1];
            _t272 = a_u[k_3][j_3][i_7][base3 + 1];
            _t273 = a_lambda[k_4][j_3][i_1];
            _t274 = a_u[k_4][j_3][i_5][base3 + 1];
            _t275 = a_u[k_4][j_3][i_9][base3 + 1];
            _t276 = a_u[k_4][j_3][i_3][base3 + 1];
            _t277 = a_u[k_4][j_3][i_7][base3 + 1];
            _t278 = a_stry[j_3 + - jfirst];
            _t279 = a_strz[k + - kfirst];
            _t280 = a_lambda[k_2][j_3][i_1];
            _t281 = a_u[k_2][j_5][i][base3 + 2];
            _t282 = a_u[k_2][j_7][i][base3 + 2];
            _t283 = a_u[k_2][j_4][i][base3 + 2];
            _t284 = a_u[k_2][j_6][i][base3 + 2];
            _t285 = a_lambda[k_1][j_3][i_1];
            _t286 = a_u[k_1][j_5][i][base3 + 2];
            _t287 = a_u[k_1][j_7][i][base3 + 2];
            _t288 = a_u[k_1][j_4][i][base3 + 2];
            _t289 = a_u[k_1][j_6][i][base3 + 2];
            _t290 = a_lambda[k_3][j_3][i_1];
            _t291 = a_u[k_3][j_5][i][base3 + 2];
            _t292 = a_u[k_3][j_7][i][base3 + 2];
            _t293 = a_u[k_3][j_4][i][base3 + 2];
            _t294 = a_u[k_3][j_6][i][base3 + 2];
            _t295 = a_lambda[k_4][j_3][i_1];
            _t296 = a_u[k_4][j_5][i][base3 + 2];
            _t297 = a_u[k_4][j_7][i][base3 + 2];
            _t298 = a_u[k_4][j_4][i][base3 + 2];
            _t299 = a_u[k_4][j_6][i][base3 + 2];
            _t13 = a_mu[k_3][j_3][i_1] * a_strz[k_3 + - kfirst] - 0.75 * (a_mu[
               k][j_3][i_1] * a_strz[k + - kfirst] + a_mu[k_4][j_3][i_1] *
               a_strz[k_4 + - kfirst]);
            _t12 = a_mu[k_1][j_3][i_1] * a_strz[k_1 + - kfirst] + a_mu[k_4][j_3
               ][i_1] * a_strz[k_4 + - kfirst] + 3.0 * (a_mu[k_3][j_3][i_1] *
               a_strz[k_3 + - kfirst] + a_mu[k][j_3][i_1] * a_strz[k + - kfirst
               ]);
            _t11 = a_mu[k_2][j_3][i_1] * a_strz[k_2 + - kfirst] + a_mu[k_3][j_3
               ][i_1] * a_strz[k_3 + - kfirst] + 3.0 * (a_mu[k][j_3][i_1] *
               a_strz[k + - kfirst] + a_mu[k_1][j_3][i_1] * a_strz[k_1 + -
               kfirst]);
            _t10 = a_mu[k_1][j_3][i_1] * a_strz[k_1 + - kfirst] - 0.75 * (a_mu[
               k][j_3][i_1] * a_strz[k + - kfirst] + a_mu[k_2][j_3][i_1] *
               a_strz[k_2 + - kfirst]);
            _t9 = a_mu[k][j_6][i_1] * a_stry[j_6 + - jfirst] - 0.75 * (a_mu[k][
               j_3][i_1] * a_stry[j_3 + - jfirst] + a_mu[k][j_7][i_1] * a_stry[
               j_7 + - jfirst]);
            _t8 = a_mu[k][j_4][i_1] * a_stry[j_4 + - jfirst] + a_mu[k][j_7][i_1
               ] * a_stry[j_7 + - jfirst] + 3.0 * (a_mu[k][j_6][i_1] * a_stry[
               j_6 + - jfirst] + a_mu[k][j_3][i_1] * a_stry[j_3 + - jfirst]);
            _t7 = a_mu[k][j_5][i_1] * a_stry[j_5 + - jfirst] + a_mu[k][j_6][i_1
               ] * a_stry[j_6 + - jfirst] + 3.0 * (a_mu[k][j_3][i_1] * a_stry[
               j_3 + - jfirst] + a_mu[k][j_4][i_1] * a_stry[j_4 + - jfirst]);
            _t6 = a_mu[k][j_4][i_1] * a_stry[j_4 + - jfirst] - 0.75 * (a_mu[k][
               j_3][i_1] * a_stry[j_3 + - jfirst] + a_mu[k][j_5][i_1] * a_stry[
               j_5 + - jfirst]);
            _t5 = a_mu[k][j_3][i_6] * a_strx[i_7 + - ifirst] - 0.75 * (a_mu[k][
               j_3][i_1] * a_strx[i + - ifirst] + a_mu[k][j_3][i_8] * a_strx[
               i_9 + - ifirst]);
            _t4 = a_mu[k][j_3][i_2] * a_strx[i_3 + - ifirst] + a_mu[k][j_3][i_8
               ] * a_strx[i_9 + - ifirst] + 3.0 * (a_mu[k][j_3][i_6] * a_strx[
               i_7 + - ifirst] + a_mu[k][j_3][i_1] * a_strx[i + - ifirst]);
            _t3 = a_mu[k][j_3][i_4] * a_strx[i_5 + - ifirst] + a_mu[k][j_3][i_6
               ] * a_strx[i_7 + - ifirst] + 3.0 * (a_mu[k][j_3][i_1] * a_strx[i
               + - ifirst] + a_mu[k][j_3][i_2] * a_strx[i_3 + - ifirst]);
            _t2 = a_mu[k][j_3][i_2] * a_strx[i_3 + - ifirst] - 0.75 * (a_mu[k][
               j_3][i_1] * a_strx[i + - ifirst] + a_mu[k][j_3][i_4] * a_strx[
               i_5 + - ifirst]);
            a_lu[k][j_3][i][base3 + 1] = 0.0 * a_lu[k][j_3][i][base3 + 1] + _t1
               * (0.16666666666666666 * (a_strx[i + - ifirst] * ((2.0 * _t2 +
               a_lambda[k][j_3][i_2] * a_strx[i_3 + - ifirst] - 0.75 * (
               a_lambda[k][j_3][i_1] * a_strx[i + - ifirst] + a_lambda[k][j_3][
               i_4] * a_strx[i_5 + - ifirst])) * (a_u[k][j_3][i_5][base3 + 1] -
               a_u[k][j_3][i][base3 + 1]) + (2.0 * _t3 + a_lambda[k][j_3][i_4] *
               a_strx[i_5 + - ifirst] + a_lambda[k][j_3][i_6] * a_strx[i_7 + -
               ifirst] + 3.0 * (a_lambda[k][j_3][i_1] * a_strx[i + - ifirst] +
               a_lambda[k][j_3][i_2] * a_strx[i_3 + - ifirst])) * (a_u[k][j_3][
               i_3][base3 + 1] - a_u[k][j_3][i][base3 + 1]) + (2.0 * _t4 +
               a_lambda[k][j_3][i_2] * a_strx[i_3 + - ifirst] + a_lambda[k][j_3
               ][i_8] * a_strx[i_9 + - ifirst] + 3.0 * (a_lambda[k][j_3][i_6] *
               a_strx[i_7 + - ifirst] + a_lambda[k][j_3][i_1] * a_strx[i + -
               ifirst])) * (a_u[k][j_3][i_7][base3 + 1] - a_u[k][j_3][i][base3 +
               1]) + (2.0 * _t5 + a_lambda[k][j_3][i_6] * a_strx[i_7 + - ifirst
               ] - 0.75 * (a_lambda[k][j_3][i_1] * a_strx[i + - ifirst] +
               a_lambda[k][j_3][i_8] * a_strx[i_9 + - ifirst])) * (a_u[k][j_3][
               i_9][base3 + 1] - a_u[k][j_3][i][base3 + 1])) + a_stry[j_3 + -
               jfirst] * (_t6 * (a_u[k][j_5][i][base3 + 1] - a_u[k][j_3][i][
               base3 + 1]) + _t7 * (a_u[k][j_4][i][base3 + 1] - a_u[k][j_3][i][
               base3 + 1]) + _t8 * (a_u[k][j_6][i][base3 + 1] - a_u[k][j_3][i][
               base3 + 1]) + _t9 * (a_u[k][j_7][i][base3 + 1] - a_u[k][j_3][i][
               base3 + 1])) + a_strz[k + - kfirst] * (_t10 * (a_u[k_2][j_3][i][
               base3 + 1] - a_u[k][j_3][i][base3 + 1]) + _t11 * (a_u[k_1][j_3][
               i][base3 + 1] - a_u[k][j_3][i][base3 + 1]) + _t12 * (a_u[k_3][
               j_3][i][base3 + 1] - a_u[k][j_3][i][base3 + 1]) + _t13 * (a_u[
               k_4][j_3][i][base3 + 1] - a_u[k][j_3][i][base3 + 1]))) + a_strx[
               i + - ifirst] * a_stry[j_3 + - jfirst] * 0.006944444444444444 * (
               a_lambda[k][j_3][i_4] * (a_u[k][j_5][i_5][base3 + 2] - a_u[k][
               j_7][i_5][base3 + 2] + 8.0 * (- a_u[k][j_4][i_5][base3 + 2] + a_u
               [k][j_6][i_5][base3 + 2])) - 8.0 * (a_lambda[k][j_3][i_2] * (a_u
               [k][j_5][i_3][base3 + 2] - a_u[k][j_7][i_3][base3 + 2] + 8.0 * (
               - a_u[k][j_4][i_3][base3 + 2] + a_u[k][j_6][i_3][base3 + 2]))) +
               8.0 * (a_lambda[k][j_3][i_6] * (a_u[k][j_5][i_7][base3 + 2] - a_u
               [k][j_7][i_7][base3 + 2] + 8.0 * (- a_u[k][j_4][i_7][base3 + 2] +
               a_u[k][j_6][i_7][base3 + 2]))) - a_lambda[k][j_3][i_8] * (a_u[k]
               [j_5][i_9][base3 + 2] - a_u[k][j_7][i_9][base3 + 2] + 8.0 * (-
               a_u[k][j_4][i_9][base3 + 2] + a_u[k][j_6][i_9][base3 + 2]))) +
               a_strx[i + - ifirst] * a_strz[k + - kfirst] *
               0.006944444444444444 * (a_lambda[k][j_3][i_4] * (a_u[k_2][j_3][
               i_5][base3 + 3] - a_u[k_4][j_3][i_5][base3 + 3] + 8.0 * (- a_u[
               k_1][j_3][i_5][base3 + 3] + a_u[k_3][j_3][i_5][base3 + 3])) - 8.0
               * (a_lambda[k][j_3][i_2] * (a_u[k_2][j_3][i_3][base3 + 3] - a_u[
               k_4][j_3][i_3][base3 + 3] + 8.0 * (- a_u[k_1][j_3][i_3][base3 + 3
               ] + a_u[k_3][j_3][i_3][base3 + 3]))) + 8.0 * (a_lambda[k][j_3][
               i_6] * (a_u[k_2][j_3][i_7][base3 + 3] - a_u[k_4][j_3][i_7][base3
               + 3] + 8.0 * (- a_u[k_1][j_3][i_7][base3 + 3] + a_u[k_3][j_3][
               i_7][base3 + 3]))) - a_lambda[k][j_3][i_8] * (a_u[k_2][j_3][i_9]
               [base3 + 3] - a_u[k_4][j_3][i_9][base3 + 3] + 8.0 * (- a_u[k_1][
               j_3][i_9][base3 + 3] + a_u[k_3][j_3][i_9][base3 + 3]))) + a_strx
               [i + - ifirst] * a_stry[j_3 + - jfirst] * 0.006944444444444444 *
               (a_mu[k][j_5][i_1] * (a_u[k][j_5][i_5][base3 + 2] - a_u[k][j_5][
               i_9][base3 + 2] + 8.0 * (- a_u[k][j_5][i_3][base3 + 2] + a_u[k][
               j_5][i_7][base3 + 2])) - 8.0 * (a_mu[k][j_4][i_1] * (a_u[k][j_4]
               [i_5][base3 + 2] - a_u[k][j_4][i_9][base3 + 2] + 8.0 * (- a_u[k]
               [j_4][i_3][base3 + 2] + a_u[k][j_4][i_7][base3 + 2]))) + 8.0 * (
               a_mu[k][j_6][i_1] * (a_u[k][j_6][i_5][base3 + 2] - a_u[k][j_6][
               i_9][base3 + 2] + 8.0 * (- a_u[k][j_6][i_3][base3 + 2] + a_u[k][
               j_6][i_7][base3 + 2]))) - a_mu[k][j_7][i_1] * (a_u[k][j_7][i_5][
               base3 + 2] - a_u[k][j_7][i_9][base3 + 2] + 8.0 * (- a_u[k][j_7][
               i_3][base3 + 2] + a_u[k][j_7][i_7][base3 + 2]))) + a_strx[i + -
               ifirst] * a_strz[k + - kfirst] * 0.006944444444444444 * (a_mu[
               k_2][j_3][i_1] * (a_u[k_2][j_3][i_5][base3 + 3] - a_u[k_2][j_3][
               i_9][base3 + 3] + 8.0 * (- a_u[k_2][j_3][i_3][base3 + 3] + a_u[
               k_2][j_3][i_7][base3 + 3])) - 8.0 * (a_mu[k_1][j_3][i_1] * (a_u[
               k_1][j_3][i_5][base3 + 3] - a_u[k_1][j_3][i_9][base3 + 3] + 8.0 *
               (- a_u[k_1][j_3][i_3][base3 + 3] + a_u[k_1][j_3][i_7][base3 + 3]
               ))) + 8.0 * (a_mu[k_3][j_3][i_1] * (a_u[k_3][j_3][i_5][base3 + 3
               ] - a_u[k_3][j_3][i_9][base3 + 3] + 8.0 * (- a_u[k_3][j_3][i_3][
               base3 + 3] + a_u[k_3][j_3][i_7][base3 + 3]))) - a_mu[k_4][j_3][
               i_1] * (a_u[k_4][j_3][i_5][base3 + 3] - a_u[k_4][j_3][i_9][base3
               + 3] + 8.0 * (- a_u[k_4][j_3][i_3][base3 + 3] + a_u[k_4][j_3][
               i_7][base3 + 3]))));
            a_lu[k][j_3][i][base3 + 2] = 0.0 * a_lu[k][j_3][i][base3 + 2] + _t1
               * (0.16666666666666666 * (_t14 * (_t2 * (_t15 - _t16) + _t3 * (
               _t17 - _t18) + _t4 * (_t19 - _t20) + _t5 * (_t21 - _t22)) + _t23
               * ((2.0 * _t6 + _t24 * _t25 - 0.75 * (_t26 * _t27 + _t28 * _t29)
               ) * (_t30 - _t31) + (2.0 * _t7 + _t32 * _t33 + _t34 * _t35 + 3.0
               * (_t36 * _t37 + _t38 * _t39)) * (_t40 - _t41) + (2.0 * _t8 +
               _t42 * _t43 + _t44 * _t45 + 3.0 * (_t46 * _t47 + _t48 * _t49)) *
               (_t50 - _t51) + (2.0 * _t9 + _t52 * _t53 - 0.75 * (_t54 * _t55 +
               _t56 * _t57)) * (_t58 - _t59)) + _t60 * (_t10 * (_t61 - _t62) +
               _t11 * (_t63 - _t64) + _t12 * (_t65 - _t66) + _t13 * (_t67 - _t68
               ))) + _t124 * _t125 * 0.006944444444444444 * (_t126 * (_t127 -
               _t128 + 8.0 * (- _t129 + _t130)) - 8.0 * (_t131 * (_t132 - _t133
               + 8.0 * (- _t134 + _t135))) + 8.0 * (_t136 * (_t137 - _t138 + 8.0
               * (- _t139 + _t140))) - _t141 * (_t142 - _t143 + 8.0 * (- _t144 +
               _t145))) + _t146 * _t147 * 0.006944444444444444 * (_t148 * (
               _t149 - _t150 + 8.0 * (- _t151 + _t152)) - 8.0 * (_t153 * (_t154
               - _t155 + 8.0 * (- _t156 + _t157))) + 8.0 * (_t158 * (_t159 -
               _t160 + 8.0 * (- _t161 + _t162))) - _t163 * (_t164 - _t165 + 8.0
               * (- _t166 + _t167))) + _t168 * _t169 * 0.006944444444444444 * (
               _t170 * (_t171 - _t172 + 8.0 * (- _t173 + _t174)) - 8.0 * (_t175
               * (_t176 - _t177 + 8.0 * (- _t178 + _t179))) + 8.0 * (_t180 * (
               _t181 - _t182 + 8.0 * (- _t183 + _t184))) - _t185 * (_t186 -
               _t187 + 8.0 * (- _t188 + _t189))) + _t190 * _t191 *
               0.006944444444444444 * (_t192 * (_t193 - _t194 + 8.0 * (- _t195 +
               _t196)) - 8.0 * (_t197 * (_t198 - _t199 + 8.0 * (- _t200 + _t201
               ))) + 8.0 * (_t202 * (_t203 - _t204 + 8.0 * (- _t205 + _t206))) -
               _t207 * (_t208 - _t209 + 8.0 * (- _t210 + _t211))));
            a_lu[k][j_3][i][base3 + 3] = 0.0 * a_lu[k][j_3][i][base3 + 3] + _t1
               * (0.16666666666666666 * (_t69 * (_t2 * (_t70 - _t71) + _t3 * (
               _t72 - _t73) + _t4 * (_t74 - _t75) + _t5 * (_t76 - _t77)) + _t78
               * (_t6 * (_t79 - _t80) + _t7 * (_t81 - _t82) + _t8 * (_t83 - _t84
               ) + _t9 * (_t85 - _t86)) + _t87 * ((2.0 * _t10 + _t88 * _t89 -
               0.75 * (_t90 * _t91 + _t92 * _t93)) * (_t94 - _t95) + (2.0 * _t11
               + _t96 * _t97 + _t98 * _t99 + 3.0 * (_t100 * _t101 + _t102 *
               _t103)) * (_t104 - _t105) + (2.0 * _t12 + _t106 * _t107 + _t108 *
               _t109 + 3.0 * (_t110 * _t111 + _t112 * _t113)) * (_t114 - _t115)
               + (2.0 * _t13 + _t116 * _t117 - 0.75 * (_t118 * _t119 + _t120 *
               _t121)) * (_t122 - _t123))) + _t212 * _t213 *
               0.006944444444444444 * (_t214 * (_t215 - _t216 + 8.0 * (- _t217 +
               _t218)) - 8.0 * (_t219 * (_t220 - _t221 + 8.0 * (- _t222 + _t223
               ))) + 8.0 * (_t224 * (_t225 - _t226 + 8.0 * (- _t227 + _t228))) -
               _t229 * (_t230 - _t231 + 8.0 * (- _t232 + _t233))) + _t234 *
               _t235 * 0.006944444444444444 * (_t236 * (_t237 - _t238 + 8.0 * (
               - _t239 + _t240)) - 8.0 * (_t241 * (_t242 - _t243 + 8.0 * (-
               _t244 + _t245))) + 8.0 * (_t246 * (_t247 - _t248 + 8.0 * (-
               _t249 + _t250))) - _t251 * (_t252 - _t253 + 8.0 * (- _t254 +
               _t255))) + _t256 * _t257 * 0.006944444444444444 * (_t258 * (
               _t259 - _t260 + 8.0 * (- _t261 + _t262)) - 8.0 * (_t263 * (_t264
               - _t265 + 8.0 * (- _t266 + _t267))) + 8.0 * (_t268 * (_t269 -
               _t270 + 8.0 * (- _t271 + _t272))) - _t273 * (_t274 - _t275 + 8.0
               * (- _t276 + _t277))) + _t278 * _t279 * 0.006944444444444444 * (
               _t280 * (_t281 - _t282 + 8.0 * (- _t283 + _t284)) - 8.0 * (_t285
               * (_t286 - _t287 + 8.0 * (- _t288 + _t289))) + 8.0 * (_t290 * (
               _t291 - _t292 + 8.0 * (- _t293 + _t294))) - _t295 * (_t296 -
               _t297 + 8.0 * (- _t298 + _t299))));
         }
      }
   }
}

/*
 * ../src/rhs4sg.c:1096.6
 */
void core_fn2(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided_2, double (* a_acof)[8][6], double (* a_bope)[6
   ], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
{
   int k;
   int k_1;
   for (k = 1, k_1 = 0; k <= 6; k++, k_1++) {
      int j_8;
      int j_9;
      int j_10;
      int j_11;
      int j_12;
      for (j_8 = jfirst + 2,
           j_12 = jfirst + 4,
           j_9 = jfirst + 1,
           j_11 = jfirst + 3,
           j_10 = jfirst + 0;
           j_8 <= jlast + -2;
           j_8++,
           j_9++,
           j_10++,
           j_11++,
           j_12++) {
         int i;
         int i_1;
         int i_2;
         int i_3;
         int i_4;
         int i_5;
         int i_6;
         int i_7;
         int i_8;
         int i_9;
         for (i = ifirst + 2,
              i_1 = i + base,
              i_2 = i_1 + -1,
              i_9 = ifirst + 4,
              i_3 = ifirst + 1,
              i_4 = i_1 + -2,
              i_7 = ifirst + 3,
              i_5 = ifirst + 0,
              i_6 = i_1 + 1,
              i_8 = i_1 + 2;
              i <= ilast + -2;
              i++,
              i_1++,
              i_2++,
              i_3++,
              i_4++,
              i_5++,
              i_6++,
              i_7++,
              i_8++,
              i_9++) {
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
            double mu1zz;
            double mu2zz;
            double mu3zz;
            double u3zip2;
            double u3zip1;
            double u3zim1;
            double u3zim2;
            double mu3xz;
            double u3zjp2;
            double u3zjp1;
            double u3zjm1;
            double u3zjm2;
            double mu3yz;
            double u1zip2;
            double u1zip1;
            double u1zim1;
            double u1zim2;
            double u2zjp2;
            double u2zjp1;
            double u2zjm1;
            double u2zjm2;
            double lau1xz;
            double lau2yz;
            double cof;
            int q;
            int q_1;
            int q_2;
            int q_3;
            int q_4;
            int q_5;
            int q_6;
            int q_7;
            int q_8;
            int q_9;
            int q_10;
            int q_11;
            int q_12;
            int q_13;
            int q_14;
            int q_15;
            int q_16;
            int q_17;
            for (_t8 = a_mu[k][j_11][i_1] * a_stry[j_11 + - jfirst] - 0.75 * (
                    a_mu[k][j_8][i_1] * a_stry[j_8 + - jfirst] + a_mu[k][j_12][
                    i_1] * a_stry[j_12 + - jfirst]),
                 _t7 = a_mu[k][j_9][i_1] * a_stry[j_9 + - jfirst] + a_mu[k][
                    j_12][i_1] * a_stry[j_12 + - jfirst] + 3.0 * (a_mu[k][j_11]
                    [i_1] * a_stry[j_11 + - jfirst] + a_mu[k][j_8][i_1] * a_stry
                    [j_8 + - jfirst]),
                 _t6 = a_mu[k][j_10][i_1] * a_stry[j_10 + - jfirst] + a_mu[k][
                    j_11][i_1] * a_stry[j_11 + - jfirst] + 3.0 * (a_mu[k][j_8][
                    i_1] * a_stry[j_8 + - jfirst] + a_mu[k][j_9][i_1] * a_stry[
                    j_9 + - jfirst]),
                 _t5 = a_mu[k][j_9][i_1] * a_stry[j_9 + - jfirst] - 0.75 * (
                    a_mu[k][j_8][i_1] * a_stry[j_8 + - jfirst] + a_mu[k][j_10][
                    i_1] * a_stry[j_10 + - jfirst]),
                 _t4 = a_mu[k][j_8][i_6] * a_strx[i_7 + - ifirst] - 0.75 * (
                    a_mu[k][j_8][i_1] * a_strx[i + - ifirst] + a_mu[k][j_8][i_8
                    ] * a_strx[i_9 + - ifirst]),
                 _t3 = a_mu[k][j_8][i_2] * a_strx[i_3 + - ifirst] + a_mu[k][j_8
                    ][i_8] * a_strx[i_9 + - ifirst] + 3.0 * (a_mu[k][j_8][i_6] *
                    a_strx[i_7 + - ifirst] + a_mu[k][j_8][i_1] * a_strx[i + -
                    ifirst]),
                 _t2 = a_mu[k][j_8][i_4] * a_strx[i_5 + - ifirst] + a_mu[k][j_8
                    ][i_6] * a_strx[i_7 + - ifirst] + 3.0 * (a_mu[k][j_8][i_1] *
                    a_strx[i + - ifirst] + a_mu[k][j_8][i_2] * a_strx[i_3 + -
                    ifirst]),
                 _t1 = a_mu[k][j_8][i_2] * a_strx[i_3 + - ifirst] - 0.75 * (
                    a_mu[k][j_8][i_1] * a_strx[i + - ifirst] + a_mu[k][j_8][i_4
                    ] * a_strx[i_5 + - ifirst]),
                 _t9 = 0.16666666666666666 * (a_strx[i + - ifirst] * ((2.0 * _t1
                    + a_lambda[k][j_8][i_2] * a_strx[i_3 + - ifirst] - 0.75 * (
                    a_lambda[k][j_8][i_1] * a_strx[i + - ifirst] + a_lambda[k][
                    j_8][i_4] * a_strx[i_5 + - ifirst])) * (a_u[k][j_8][i_5][
                    base3 + 1] - a_u[k][j_8][i][base3 + 1]) + (2.0 * _t2 +
                    a_lambda[k][j_8][i_4] * a_strx[i_5 + - ifirst] + a_lambda[k
                    ][j_8][i_6] * a_strx[i_7 + - ifirst] + 3.0 * (a_lambda[k][
                    j_8][i_1] * a_strx[i + - ifirst] + a_lambda[k][j_8][i_2] *
                    a_strx[i_3 + - ifirst])) * (a_u[k][j_8][i_3][base3 + 1] -
                    a_u[k][j_8][i][base3 + 1]) + (2.0 * _t3 + a_lambda[k][j_8][
                    i_2] * a_strx[i_3 + - ifirst] + a_lambda[k][j_8][i_8] *
                    a_strx[i_9 + - ifirst] + 3.0 * (a_lambda[k][j_8][i_6] *
                    a_strx[i_7 + - ifirst] + a_lambda[k][j_8][i_1] * a_strx[i +
                    - ifirst])) * (a_u[k][j_8][i_7][base3 + 1] - a_u[k][j_8][i]
                    [base3 + 1]) + (2.0 * _t4 + a_lambda[k][j_8][i_6] * a_strx[
                    i_7 + - ifirst] - 0.75 * (a_lambda[k][j_8][i_1] * a_strx[i +
                    - ifirst] + a_lambda[k][j_8][i_8] * a_strx[i_9 + - ifirst])
                    ) * (a_u[k][j_8][i_9][base3 + 1] - a_u[k][j_8][i][base3 + 1
                    ])) + a_stry[j_8 + - jfirst] * (_t5 * (a_u[k][j_10][i][
                    base3 + 1] - a_u[k][j_8][i][base3 + 1]) + _t6 * (a_u[k][j_9
                    ][i][base3 + 1] - a_u[k][j_8][i][base3 + 1]) + _t7 * (a_u[k
                    ][j_11][i][base3 + 1] - a_u[k][j_8][i][base3 + 1]) + _t8 * (
                    a_u[k][j_12][i][base3 + 1] - a_u[k][j_8][i][base3 + 1]))),
                 mu3zz = 0.0,
                 q_1 = 1,
                 mu1zz = 0.0,
                 mu2zz = 0.0,
                 q = 0;
                 q_1 <= 8; ) {
               double mucof;
               double lap2mu;
               int _t22;
               int m;
               int m_1;
               for (lap2mu = 0.0,
                    m = 1,
                    mucof = 0.0,
                    m_1 = 0;
                    m <= 8; ) {
                  int _t23;
                  int _t24;
                  _t24 = m_1 + 1;
                  _t23 = m + 1;
                  lap2mu = lap2mu + a_acof[m_1][q][k_1] * (a_lambda[m][j_8][i_1
                     ] + 2.0 * a_mu[m][j_8][i_1]);
                  mucof = mucof + a_acof[m_1][q][k_1] * a_mu[m][j_8][i_1];
                  m = _t23;
                  m_1 = _t24;
               }
               _t22 = q_1 + 1;
               mu3zz = mu3zz + lap2mu * a_u[q_1][j_8][i][base3 + 3];
               mu1zz = mu1zz + mucof * a_u[q_1][j_8][i][base3 + 1];
               mu2zz = mu2zz + mucof * a_u[q_1][j_8][i][base3 + 2];
               q++;
               q_1 = _t22;
            }
            for (_t10 = 0.16666666666666666 * (a_strx[i + - ifirst] * (_t1 * (
                    a_u[k][j_8][i_5][base3 + 2] - a_u[k][j_8][i][base3 + 2]) +
                    _t2 * (a_u[k][j_8][i_3][base3 + 2] - a_u[k][j_8][i][base3 +
                    2]) + _t3 * (a_u[k][j_8][i_7][base3 + 2] - a_u[k][j_8][i][
                    base3 + 2]) + _t4 * (a_u[k][j_8][i_9][base3 + 2] - a_u[k][
                    j_8][i][base3 + 2])) + a_stry[j_8 + - jfirst] * ((2.0 * _t5
                    + a_lambda[k][j_9][i_1] * a_stry[j_9 + - jfirst] - 0.75 * (
                    a_lambda[k][j_8][i_1] * a_stry[j_8 + - jfirst] + a_lambda[k
                    ][j_10][i_1] * a_stry[j_10 + - jfirst])) * (a_u[k][j_10][i]
                    [base3 + 2] - a_u[k][j_8][i][base3 + 2]) + (2.0 * _t6 +
                    a_lambda[k][j_10][i_1] * a_stry[j_10 + - jfirst] + a_lambda
                    [k][j_11][i_1] * a_stry[j_11 + - jfirst] + 3.0 * (a_lambda[
                    k][j_8][i_1] * a_stry[j_8 + - jfirst] + a_lambda[k][j_9][
                    i_1] * a_stry[j_9 + - jfirst])) * (a_u[k][j_9][i][base3 + 2
                    ] - a_u[k][j_8][i][base3 + 2]) + (2.0 * _t7 + a_lambda[k][
                    j_9][i_1] * a_stry[j_9 + - jfirst] + a_lambda[k][j_12][i_1]
                    * a_stry[j_12 + - jfirst] + 3.0 * (a_lambda[k][j_11][i_1] *
                    a_stry[j_11 + - jfirst] + a_lambda[k][j_8][i_1] * a_stry[
                    j_8 + - jfirst])) * (a_u[k][j_11][i][base3 + 2] - a_u[k][
                    j_8][i][base3 + 2]) + (2.0 * _t8 + a_lambda[k][j_11][i_1] *
                    a_stry[j_11 + - jfirst] - 0.75 * (a_lambda[k][j_8][i_1] *
                    a_stry[j_8 + - jfirst] + a_lambda[k][j_12][i_1] * a_stry[
                    j_12 + - jfirst])) * (a_u[k][j_12][i][base3 + 2] - a_u[k][
                    j_8][i][base3 + 2]))) + (mu2zz + a_ghcof[k_1] * a_mu[1][j_8
                    ][i_1] * a_u[0][j_8][i][base3 + 2]),
                 _t11 = 0.16666666666666666 * (a_strx[i + - ifirst] * (_t1 * (
                    a_u[k][j_8][i_5][base3 + 3] - a_u[k][j_8][i][base3 + 3]) +
                    _t2 * (a_u[k][j_8][i_3][base3 + 3] - a_u[k][j_8][i][base3 +
                    3]) + _t3 * (a_u[k][j_8][i_7][base3 + 3] - a_u[k][j_8][i][
                    base3 + 3]) + _t4 * (a_u[k][j_8][i_9][base3 + 3] - a_u[k][
                    j_8][i][base3 + 3])) + a_stry[j_8 + - jfirst] * (_t5 * (a_u
                    [k][j_10][i][base3 + 3] - a_u[k][j_8][i][base3 + 3]) + _t6 *
                    (a_u[k][j_9][i][base3 + 3] - a_u[k][j_8][i][base3 + 3]) +
                    _t7 * (a_u[k][j_11][i][base3 + 3] - a_u[k][j_8][i][base3 + 3
                    ]) + _t8 * (a_u[k][j_12][i][base3 + 3] - a_u[k][j_8][i][
                    base3 + 3]))) + (mu3zz + a_ghcof[k_1] * (a_lambda[1][j_8][
                    i_1] + 2.0 * a_mu[1][j_8][i_1]) * a_u[0][j_8][i][base3 + 3]
                    ),
                 _t12 = _t9 + (mu1zz + a_ghcof[k_1] * a_mu[1][j_8][i_1] * a_u[0
                    ][j_8][i][base3 + 1]) + a_strx[i + - ifirst] * a_stry[j_8 +
                    - jfirst] * (0.006944444444444444 * (a_lambda[k][j_8][i_4] *
                    (a_u[k][j_10][i_5][base3 + 2] - a_u[k][j_12][i_5][base3 + 2
                    ] + 8.0 * (- a_u[k][j_9][i_5][base3 + 2] + a_u[k][j_11][i_5
                    ][base3 + 2])) - 8.0 * (a_lambda[k][j_8][i_2] * (a_u[k][
                    j_10][i_3][base3 + 2] - a_u[k][j_12][i_3][base3 + 2] + 8.0 *
                    (- a_u[k][j_9][i_3][base3 + 2] + a_u[k][j_11][i_3][base3 + 2
                    ]))) + 8.0 * (a_lambda[k][j_8][i_6] * (a_u[k][j_10][i_7][
                    base3 + 2] - a_u[k][j_12][i_7][base3 + 2] + 8.0 * (- a_u[k]
                    [j_9][i_7][base3 + 2] + a_u[k][j_11][i_7][base3 + 2]))) -
                    a_lambda[k][j_8][i_8] * (a_u[k][j_10][i_9][base3 + 2] - a_u
                    [k][j_12][i_9][base3 + 2] + 8.0 * (- a_u[k][j_9][i_9][base3
                    + 2] + a_u[k][j_11][i_9][base3 + 2]))) +
                    0.006944444444444444 * (a_mu[k][j_10][i_1] * (a_u[k][j_10][
                    i_5][base3 + 2] - a_u[k][j_10][i_9][base3 + 2] + 8.0 * (-
                    a_u[k][j_10][i_3][base3 + 2] + a_u[k][j_10][i_7][base3 + 2]
                    )) - 8.0 * (a_mu[k][j_9][i_1] * (a_u[k][j_9][i_5][base3 + 2
                    ] - a_u[k][j_9][i_9][base3 + 2] + 8.0 * (- a_u[k][j_9][i_3]
                    [base3 + 2] + a_u[k][j_9][i_7][base3 + 2]))) + 8.0 * (a_mu[
                    k][j_11][i_1] * (a_u[k][j_11][i_5][base3 + 2] - a_u[k][j_11
                    ][i_9][base3 + 2] + 8.0 * (- a_u[k][j_11][i_3][base3 + 2] +
                    a_u[k][j_11][i_7][base3 + 2]))) - a_mu[k][j_12][i_1] * (a_u
                    [k][j_12][i_5][base3 + 2] - a_u[k][j_12][i_9][base3 + 2] +
                    8.0 * (- a_u[k][j_12][i_3][base3 + 2] + a_u[k][j_12][i_7][
                    base3 + 2])))),
                 u3zim2 = 0.0,
                 q_2 = 1,
                 u3zip2 = 0.0,
                 u3zip1 = 0.0,
                 u3zim1 = 0.0,
                 q_3 = 0;
                 q_2 <= 8; ) {
               int _t25;
               int _t26;
               _t26 = q_3 + 1;
               _t25 = q_2 + 1;
               u3zip1 = u3zip1 + a_bope[q_3][k_1] * a_u[q_2][j_8][i_7][base3 + 3
                  ];
               u3zim2 = u3zim2 + a_bope[q_3][k_1] * a_u[q_2][j_8][i_5][base3 + 3
                  ];
               u3zip2 = u3zip2 + a_bope[q_3][k_1] * a_u[q_2][j_8][i_9][base3 + 3
                  ];
               u3zim1 = u3zim1 + a_bope[q_3][k_1] * a_u[q_2][j_8][i_3][base3 + 3
                  ];
               q_2 = _t25;
               q_3 = _t26;
            }
            for (_t13 = _t12 + a_strx[i + - ifirst] * (0.08333333333333333 * (-
                     a_lambda[k][j_8][i_8] * u3zip2 + 8.0 * a_lambda[k][j_8][
                    i_6] * u3zip1 - 8.0 * a_lambda[k][j_8][i_2] * u3zim1 +
                    a_lambda[k][j_8][i_4] * u3zim2)),
                 mu3xz = 0.0,
                 q_4 = 1,
                 q_5 = 0;
                 q_4 <= 8; ) {
               int _t27;
               int _t28;
               _t28 = q_5 + 1;
               _t27 = q_4 + 1;
               mu3xz = mu3xz + a_bope[q_5][k_1] * (a_mu[q_4][j_8][i_1] *
                  0.08333333333333333 * (- a_u[q_4][j_8][i_9][base3 + 3] + 8.0 *
                  a_u[q_4][j_8][i_7][base3 + 3] - 8.0 * a_u[q_4][j_8][i_3][
                  base3 + 3] + a_u[q_4][j_8][i_5][base3 + 3]));
               q_4 = _t27;
               q_5 = _t28;
            }
            for (_t15 = _t10 + a_strx[i + - ifirst] * a_stry[j_8 + - jfirst] * (
                    0.006944444444444444 * (a_mu[k][j_8][i_4] * (a_u[k][j_10][
                    i_5][base3 + 1] - a_u[k][j_12][i_5][base3 + 1] + 8.0 * (-
                    a_u[k][j_9][i_5][base3 + 1] + a_u[k][j_11][i_5][base3 + 1])
                    ) - 8.0 * (a_mu[k][j_8][i_2] * (a_u[k][j_10][i_3][base3 + 1
                    ] - a_u[k][j_12][i_3][base3 + 1] + 8.0 * (- a_u[k][j_9][i_3
                    ][base3 + 1] + a_u[k][j_11][i_3][base3 + 1]))) + 8.0 * (
                    a_mu[k][j_8][i_6] * (a_u[k][j_10][i_7][base3 + 1] - a_u[k][
                    j_12][i_7][base3 + 1] + 8.0 * (- a_u[k][j_9][i_7][base3 + 1
                    ] + a_u[k][j_11][i_7][base3 + 1]))) - a_mu[k][j_8][i_8] * (
                    a_u[k][j_10][i_9][base3 + 1] - a_u[k][j_12][i_9][base3 + 1]
                    + 8.0 * (- a_u[k][j_9][i_9][base3 + 1] + a_u[k][j_11][i_9][
                    base3 + 1]))) + 0.006944444444444444 * (a_lambda[k][j_10][
                    i_1] * (a_u[k][j_10][i_5][base3 + 1] - a_u[k][j_10][i_9][
                    base3 + 1] + 8.0 * (- a_u[k][j_10][i_3][base3 + 1] + a_u[k]
                    [j_10][i_7][base3 + 1])) - 8.0 * (a_lambda[k][j_9][i_1] * (
                    a_u[k][j_9][i_5][base3 + 1] - a_u[k][j_9][i_9][base3 + 1] +
                    8.0 * (- a_u[k][j_9][i_3][base3 + 1] + a_u[k][j_9][i_7][
                    base3 + 1]))) + 8.0 * (a_lambda[k][j_11][i_1] * (a_u[k][
                    j_11][i_5][base3 + 1] - a_u[k][j_11][i_9][base3 + 1] + 8.0 *
                    (- a_u[k][j_11][i_3][base3 + 1] + a_u[k][j_11][i_7][base3 +
                    1]))) - a_lambda[k][j_12][i_1] * (a_u[k][j_12][i_5][base3 +
                    1] - a_u[k][j_12][i_9][base3 + 1] + 8.0 * (- a_u[k][j_12][
                    i_3][base3 + 1] + a_u[k][j_12][i_7][base3 + 1])))),
                 _t14 = _t13 + a_strx[i + - ifirst] * mu3xz,
                 u3zjm2 = 0.0,
                 q_6 = 1,
                 u3zjp2 = 0.0,
                 u3zjp1 = 0.0,
                 u3zjm1 = 0.0,
                 q_7 = 0;
                 q_6 <= 8; ) {
               int _t29;
               int _t30;
               u3zjp1 = u3zjp1 + a_bope[q_7][k_1] * a_u[q_6][j_11][i][base3 + 3
                  ];
               _t30 = q_7 + 1;
               _t29 = q_6 + 1;
               u3zjm2 = u3zjm2 + a_bope[q_7][k_1] * a_u[q_6][j_10][i][base3 + 3
                  ];
               u3zjp2 = u3zjp2 + a_bope[q_7][k_1] * a_u[q_6][j_12][i][base3 + 3
                  ];
               u3zjm1 = u3zjm1 + a_bope[q_7][k_1] * a_u[q_6][j_9][i][base3 + 3]
                  ;
               q_6 = _t29;
               q_7 = _t30;
            }
            for (_t16 = _t15 + a_stry[j_8 + - jfirst] * (0.08333333333333333 * (
                    - a_lambda[k][j_12][i_1] * u3zjp2 + 8.0 * a_lambda[k][j_11]
                    [i_1] * u3zjp1 - 8.0 * a_lambda[k][j_9][i_1] * u3zjm1 +
                    a_lambda[k][j_10][i_1] * u3zjm2)),
                 mu3yz = 0.0,
                 q_8 = 1,
                 q_9 = 0;
                 q_8 <= 8; ) {
               int _t31;
               int _t32;
               _t32 = q_9 + 1;
               _t31 = q_8 + 1;
               mu3yz = mu3yz + a_bope[q_9][k_1] * (a_mu[q_8][j_8][i_1] *
                  0.08333333333333333 * (- a_u[q_8][j_12][i][base3 + 3] + 8.0 *
                  a_u[q_8][j_11][i][base3 + 3] - 8.0 * a_u[q_8][j_9][i][base3 +
                  3] + a_u[q_8][j_10][i][base3 + 3]));
               q_8 = _t31;
               q_9 = _t32;
            }
            for (_t17 = _t16 + a_stry[j_8 + - jfirst] * mu3yz,
                 u1zim2 = 0.0,
                 q_10 = 1,
                 u1zip2 = 0.0,
                 u1zip1 = 0.0,
                 u1zim1 = 0.0,
                 q_11 = 0;
                 q_10 <= 8; ) {
               int _t33;
               int _t34;
               _t34 = q_11 + 1;
               _t33 = q_10 + 1;
               u1zip1 = u1zip1 + a_bope[q_11][k_1] * a_u[q_10][j_8][i_7][base3 +
                  1];
               u1zim2 = u1zim2 + a_bope[q_11][k_1] * a_u[q_10][j_8][i_5][base3 +
                  1];
               u1zip2 = u1zip2 + a_bope[q_11][k_1] * a_u[q_10][j_8][i_9][base3 +
                  1];
               u1zim1 = u1zim1 + a_bope[q_11][k_1] * a_u[q_10][j_8][i_3][base3 +
                  1];
               q_10 = _t33;
               q_11 = _t34;
            }
            for (_t18 = _t11 + a_strx[i + - ifirst] * (0.08333333333333333 * (-
                     a_mu[k][j_8][i_8] * u1zip2 + 8.0 * a_mu[k][j_8][i_6] *
                    u1zip1 - 8.0 * a_mu[k][j_8][i_2] * u1zim1 + a_mu[k][j_8][
                    i_4] * u1zim2)),
                 u2zjm2 = 0.0,
                 q_12 = 1,
                 u2zjp2 = 0.0,
                 u2zjp1 = 0.0,
                 u2zjm1 = 0.0,
                 q_13 = 0;
                 q_12 <= 8; ) {
               int _t35;
               int _t36;
               u2zjp1 = u2zjp1 + a_bope[q_13][k_1] * a_u[q_12][j_11][i][base3 +
                  2];
               _t36 = q_13 + 1;
               _t35 = q_12 + 1;
               u2zjm2 = u2zjm2 + a_bope[q_13][k_1] * a_u[q_12][j_10][i][base3 +
                  2];
               u2zjp2 = u2zjp2 + a_bope[q_13][k_1] * a_u[q_12][j_12][i][base3 +
                  2];
               u2zjm1 = u2zjm1 + a_bope[q_13][k_1] * a_u[q_12][j_9][i][base3 + 2
                  ];
               q_12 = _t35;
               q_13 = _t36;
            }
            for (_t19 = _t18 + a_stry[j_8 + - jfirst] * (0.08333333333333333 * (
                    - a_mu[k][j_12][i_1] * u2zjp2 + 8.0 * a_mu[k][j_11][i_1] *
                    u2zjp1 - 8.0 * a_mu[k][j_9][i_1] * u2zjm1 + a_mu[k][j_10][
                    i_1] * u2zjm2)),
                 lau1xz = 0.0,
                 q_14 = 1,
                 q_15 = 0;
                 q_14 <= 8; ) {
               int _t37;
               int _t38;
               _t38 = q_15 + 1;
               _t37 = q_14 + 1;
               lau1xz = lau1xz + a_bope[q_15][k_1] * (a_lambda[q_14][j_8][i_1] *
                  0.08333333333333333 * (- a_u[q_14][j_8][i_9][base3 + 1] + 8.0
                  * a_u[q_14][j_8][i_7][base3 + 1] - 8.0 * a_u[q_14][j_8][i_3][
                  base3 + 1] + a_u[q_14][j_8][i_5][base3 + 1]));
               q_14 = _t37;
               q_15 = _t38;
            }
            for (_t20 = _t19 + a_strx[i + - ifirst] * lau1xz,
                 lau2yz = 0.0,
                 q_16 = 1,
                 q_17 = 0;
                 q_16 <= 8; ) {
               int _t39;
               int _t40;
               _t40 = q_17 + 1;
               _t39 = q_16 + 1;
               lau2yz = lau2yz + a_bope[q_17][k_1] * (a_lambda[q_16][j_8][i_1] *
                  0.08333333333333333 * (- a_u[q_16][j_12][i][base3 + 2] + 8.0 *
                  a_u[q_16][j_11][i][base3 + 2] - 8.0 * a_u[q_16][j_9][i][base3
                  + 2] + a_u[q_16][j_10][i][base3 + 2]));
               q_16 = _t39;
               q_17 = _t40;
            }
            _t21 = a_stry[j_8 + - jfirst];
            a_lu[k][j_8][i][base3 + 1] = 0.0 * a_lu[k][j_8][i][base3 + 1] + cof
               * _t14;
            a_lu[k][j_8][i][base3 + 2] = 0.0 * a_lu[k][j_8][i][base3 + 2] + cof
               * _t17;
            a_lu[k][j_8][i][base3 + 3] = 0.0 * a_lu[k][j_8][i][base3 + 3] + cof
               * (_t20 + _t21 * lau2yz);
         }
      }
   }
}

/*
 * ../src/rhs4sg.c:1360.6
 */
void core_fn3(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided_3, double (* a_acof)[8][6], double (* a_bope)[6
   ], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
{
   int k;
   for (k = nk + -5; k <= nk; k++) {
      int j_13;
      int j_14;
      int j_15;
      int j_16;
      int j_17;
      for (j_13 = jfirst + 2,
           j_17 = jfirst + 4,
           j_14 = jfirst + 1,
           j_16 = jfirst + 3,
           j_15 = jfirst + 0;
           j_13 <= jlast + -2;
           j_13++,
           j_14++,
           j_15++,
           j_16++,
           j_17++) {
         int i;
         int i_1;
         int i_2;
         int i_3;
         int i_4;
         int i_5;
         int i_6;
         int i_7;
         int i_8;
         int i_9;
         for (i = ifirst + 2,
              i_1 = i + base,
              i_2 = i_1 + -1,
              i_9 = ifirst + 4,
              i_3 = ifirst + 1,
              i_4 = i_1 + -2,
              i_7 = ifirst + 3,
              i_5 = ifirst + 0,
              i_6 = i_1 + 1,
              i_8 = i_1 + 2;
              i <= ilast + -2;
              i++,
              i_1++,
              i_2++,
              i_3++,
              i_4++,
              i_5++,
              i_6++,
              i_7++,
              i_8++,
              i_9++) {
            double _t1;
            double _t2;
            double _t3;
            double _t4;
            double _t5;
            double _t6;
            double _t7;
            double _t8;
            double _t9;
            int _t10;
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
            double mu1zz;
            double mu2zz;
            double mu3zz;
            double u3zip2;
            double u3zip1;
            double u3zim1;
            double u3zim2;
            double mu3xz;
            double u3zjp2;
            double u3zjp1;
            double u3zjm1;
            double u3zjm2;
            double mu3yz;
            double u1zip2;
            double u1zip1;
            double u1zim1;
            double u1zim2;
            double u2zjp2;
            double u2zjp1;
            double u2zjm1;
            double u2zjm2;
            double lau1xz;
            double lau2yz;
            double cof;
            int qb;
            int qb_1;
            int qb_2;
            int qb_3;
            int qb_4;
            int qb_5;
            int qb_6;
            int qb_7;
            int qb_8;
            int qb_9;
            int qb_10;
            int qb_11;
            int qb_12;
            int qb_13;
            int qb_14;
            int qb_15;
            int qb_16;
            int qb_17;
            for (_t8 = a_mu[k][j_16][i_1] * a_stry[j_16 + - jfirst] - 0.75 * (
                    a_mu[k][j_13][i_1] * a_stry[j_13 + - jfirst] + a_mu[k][j_17
                    ][i_1] * a_stry[j_17 + - jfirst]),
                 _t7 = a_mu[k][j_14][i_1] * a_stry[j_14 + - jfirst] + a_mu[k][
                    j_17][i_1] * a_stry[j_17 + - jfirst] + 3.0 * (a_mu[k][j_16]
                    [i_1] * a_stry[j_16 + - jfirst] + a_mu[k][j_13][i_1] *
                    a_stry[j_13 + - jfirst]),
                 _t6 = a_mu[k][j_15][i_1] * a_stry[j_15 + - jfirst] + a_mu[k][
                    j_16][i_1] * a_stry[j_16 + - jfirst] + 3.0 * (a_mu[k][j_13]
                    [i_1] * a_stry[j_13 + - jfirst] + a_mu[k][j_14][i_1] *
                    a_stry[j_14 + - jfirst]),
                 _t5 = a_mu[k][j_14][i_1] * a_stry[j_14 + - jfirst] - 0.75 * (
                    a_mu[k][j_13][i_1] * a_stry[j_13 + - jfirst] + a_mu[k][j_15
                    ][i_1] * a_stry[j_15 + - jfirst]),
                 _t4 = a_mu[k][j_13][i_6] * a_strx[i_7 + - ifirst] - 0.75 * (
                    a_mu[k][j_13][i_1] * a_strx[i + - ifirst] + a_mu[k][j_13][
                    i_8] * a_strx[i_9 + - ifirst]),
                 _t3 = a_mu[k][j_13][i_2] * a_strx[i_3 + - ifirst] + a_mu[k][
                    j_13][i_8] * a_strx[i_9 + - ifirst] + 3.0 * (a_mu[k][j_13][
                    i_6] * a_strx[i_7 + - ifirst] + a_mu[k][j_13][i_1] * a_strx
                    [i + - ifirst]),
                 _t2 = a_mu[k][j_13][i_4] * a_strx[i_5 + - ifirst] + a_mu[k][
                    j_13][i_6] * a_strx[i_7 + - ifirst] + 3.0 * (a_mu[k][j_13][
                    i_1] * a_strx[i + - ifirst] + a_mu[k][j_13][i_2] * a_strx[
                    i_3 + - ifirst]),
                 _t1 = a_mu[k][j_13][i_2] * a_strx[i_3 + - ifirst] - 0.75 * (
                    a_mu[k][j_13][i_1] * a_strx[i + - ifirst] + a_mu[k][j_13][
                    i_4] * a_strx[i_5 + - ifirst]),
                 _t9 = 0.16666666666666666 * (a_strx[i + - ifirst] * ((2.0 * _t1
                    + a_lambda[k][j_13][i_2] * a_strx[i_3 + - ifirst] - 0.75 * (
                    a_lambda[k][j_13][i_1] * a_strx[i + - ifirst] + a_lambda[k]
                    [j_13][i_4] * a_strx[i_5 + - ifirst])) * (a_u[k][j_13][i_5]
                    [base3 + 1] - a_u[k][j_13][i][base3 + 1]) + (2.0 * _t2 +
                    a_lambda[k][j_13][i_4] * a_strx[i_5 + - ifirst] + a_lambda[
                    k][j_13][i_6] * a_strx[i_7 + - ifirst] + 3.0 * (a_lambda[k]
                    [j_13][i_1] * a_strx[i + - ifirst] + a_lambda[k][j_13][i_2]
                    * a_strx[i_3 + - ifirst])) * (a_u[k][j_13][i_3][base3 + 1] -
                    a_u[k][j_13][i][base3 + 1]) + (2.0 * _t3 + a_lambda[k][j_13
                    ][i_2] * a_strx[i_3 + - ifirst] + a_lambda[k][j_13][i_8] *
                    a_strx[i_9 + - ifirst] + 3.0 * (a_lambda[k][j_13][i_6] *
                    a_strx[i_7 + - ifirst] + a_lambda[k][j_13][i_1] * a_strx[i +
                    - ifirst])) * (a_u[k][j_13][i_7][base3 + 1] - a_u[k][j_13][
                    i][base3 + 1]) + (2.0 * _t4 + a_lambda[k][j_13][i_6] *
                    a_strx[i_7 + - ifirst] - 0.75 * (a_lambda[k][j_13][i_1] *
                    a_strx[i + - ifirst] + a_lambda[k][j_13][i_8] * a_strx[i_9 +
                    - ifirst])) * (a_u[k][j_13][i_9][base3 + 1] - a_u[k][j_13][
                    i][base3 + 1])) + a_stry[j_13 + - jfirst] * (_t5 * (a_u[k][
                    j_15][i][base3 + 1] - a_u[k][j_13][i][base3 + 1]) + _t6 * (
                    a_u[k][j_14][i][base3 + 1] - a_u[k][j_13][i][base3 + 1]) +
                    _t7 * (a_u[k][j_16][i][base3 + 1] - a_u[k][j_13][i][base3 +
                    1]) + _t8 * (a_u[k][j_17][i][base3 + 1] - a_u[k][j_13][i][
                    base3 + 1]))),
                 _t10 = nk + - k + 1,
                 mu3zz = 0.0,
                 qb_1 = 1,
                 mu1zz = 0.0,
                 mu2zz = 0.0,
                 qb = 0;
                 qb_1 <= 8; ) {
               double mucof;
               double lap2mu;
               int _t23;
               int mb;
               int mb_1;
               for (lap2mu = 0.0,
                    mb = 1,
                    mucof = 0.0,
                    mb_1 = 0;
                    mb <= 8; ) {
                  int _t24;
                  int _t25;
                  _t25 = mb_1 + 1;
                  _t24 = mb + 1;
                  lap2mu = lap2mu + a_acof[mb_1][qb][_t10 + -1] * (2.0 * a_mu[
                     nk + - mb + 1][j_13][i_1] + a_lambda[nk + - mb + 1][j_13][
                     i_1]);
                  mucof = mucof + a_acof[mb_1][qb][_t10 + -1] * a_mu[nk + - mb +
                     1][j_13][i_1];
                  mb = _t24;
                  mb_1 = _t25;
               }
               _t23 = qb_1 + 1;
               mu3zz = mu3zz + lap2mu * a_u[nk + - qb_1 + 1][j_13][i][base3 + 3
                  ];
               mu2zz = mu2zz + mucof * a_u[nk + - qb_1 + 1][j_13][i][base3 + 2]
                  ;
               mu1zz = mu1zz + mucof * a_u[nk + - qb_1 + 1][j_13][i][base3 + 1]
                  ;
               qb++;
               qb_1 = _t23;
            }
            for (_t11 = 0.16666666666666666 * (a_strx[i + - ifirst] * (_t1 * (
                    a_u[k][j_13][i_5][base3 + 2] - a_u[k][j_13][i][base3 + 2]) +
                    _t2 * (a_u[k][j_13][i_3][base3 + 2] - a_u[k][j_13][i][base3
                    + 2]) + _t3 * (a_u[k][j_13][i_7][base3 + 2] - a_u[k][j_13][
                    i][base3 + 2]) + _t4 * (a_u[k][j_13][i_9][base3 + 2] - a_u[
                    k][j_13][i][base3 + 2])) + a_stry[j_13 + - jfirst] * ((2.0 *
                    _t5 + a_lambda[k][j_14][i_1] * a_stry[j_14 + - jfirst] -
                    0.75 * (a_lambda[k][j_13][i_1] * a_stry[j_13 + - jfirst] +
                    a_lambda[k][j_15][i_1] * a_stry[j_15 + - jfirst])) * (a_u[k
                    ][j_15][i][base3 + 2] - a_u[k][j_13][i][base3 + 2]) + (2.0 *
                    _t6 + a_lambda[k][j_15][i_1] * a_stry[j_15 + - jfirst] +
                    a_lambda[k][j_16][i_1] * a_stry[j_16 + - jfirst] + 3.0 * (
                    a_lambda[k][j_13][i_1] * a_stry[j_13 + - jfirst] + a_lambda
                    [k][j_14][i_1] * a_stry[j_14 + - jfirst])) * (a_u[k][j_14][
                    i][base3 + 2] - a_u[k][j_13][i][base3 + 2]) + (2.0 * _t7 +
                    a_lambda[k][j_14][i_1] * a_stry[j_14 + - jfirst] + a_lambda
                    [k][j_17][i_1] * a_stry[j_17 + - jfirst] + 3.0 * (a_lambda[
                    k][j_16][i_1] * a_stry[j_16 + - jfirst] + a_lambda[k][j_13]
                    [i_1] * a_stry[j_13 + - jfirst])) * (a_u[k][j_16][i][base3 +
                    2] - a_u[k][j_13][i][base3 + 2]) + (2.0 * _t8 + a_lambda[k]
                    [j_16][i_1] * a_stry[j_16 + - jfirst] - 0.75 * (a_lambda[k]
                    [j_13][i_1] * a_stry[j_13 + - jfirst] + a_lambda[k][j_17][
                    i_1] * a_stry[j_17 + - jfirst])) * (a_u[k][j_17][i][base3 +
                    2] - a_u[k][j_13][i][base3 + 2]))) + (mu2zz + a_ghcof[_t10 +
                    -1] * a_mu[nk][j_13][i_1] * a_u[nk + 1][j_13][i][base3 + 2]
                    ),
                 _t12 = 0.16666666666666666 * (a_strx[i + - ifirst] * (_t1 * (
                    a_u[k][j_13][i_5][base3 + 3] - a_u[k][j_13][i][base3 + 3]) +
                    _t2 * (a_u[k][j_13][i_3][base3 + 3] - a_u[k][j_13][i][base3
                    + 3]) + _t3 * (a_u[k][j_13][i_7][base3 + 3] - a_u[k][j_13][
                    i][base3 + 3]) + _t4 * (a_u[k][j_13][i_9][base3 + 3] - a_u[
                    k][j_13][i][base3 + 3])) + a_stry[j_13 + - jfirst] * (_t5 *
                    (a_u[k][j_15][i][base3 + 3] - a_u[k][j_13][i][base3 + 3]) +
                    _t6 * (a_u[k][j_14][i][base3 + 3] - a_u[k][j_13][i][base3 +
                    3]) + _t7 * (a_u[k][j_16][i][base3 + 3] - a_u[k][j_13][i][
                    base3 + 3]) + _t8 * (a_u[k][j_17][i][base3 + 3] - a_u[k][
                    j_13][i][base3 + 3]))) + (mu3zz + a_ghcof[_t10 + -1] * (
                    a_lambda[nk][j_13][i_1] + 2.0 * a_mu[nk][j_13][i_1]) * a_u[
                    nk + 1][j_13][i][base3 + 3]),
                 _t13 = _t9 + (mu1zz + a_ghcof[_t10 + -1] * a_mu[nk][j_13][i_1]
                    * a_u[nk + 1][j_13][i][base3 + 1]) + a_strx[i + - ifirst] *
                    a_stry[j_13 + - jfirst] * (0.006944444444444444 * (a_lambda
                    [k][j_13][i_4] * (a_u[k][j_15][i_5][base3 + 2] - a_u[k][
                    j_17][i_5][base3 + 2] + 8.0 * (- a_u[k][j_14][i_5][base3 + 2
                    ] + a_u[k][j_16][i_5][base3 + 2])) - 8.0 * (a_lambda[k][
                    j_13][i_2] * (a_u[k][j_15][i_3][base3 + 2] - a_u[k][j_17][
                    i_3][base3 + 2] + 8.0 * (- a_u[k][j_14][i_3][base3 + 2] +
                    a_u[k][j_16][i_3][base3 + 2]))) + 8.0 * (a_lambda[k][j_13][
                    i_6] * (a_u[k][j_15][i_7][base3 + 2] - a_u[k][j_17][i_7][
                    base3 + 2] + 8.0 * (- a_u[k][j_14][i_7][base3 + 2] + a_u[k]
                    [j_16][i_7][base3 + 2]))) - a_lambda[k][j_13][i_8] * (a_u[k
                    ][j_15][i_9][base3 + 2] - a_u[k][j_17][i_9][base3 + 2] + 8.0
                    * (- a_u[k][j_14][i_9][base3 + 2] + a_u[k][j_16][i_9][base3
                    + 2]))) + 0.006944444444444444 * (a_mu[k][j_15][i_1] * (a_u
                    [k][j_15][i_5][base3 + 2] - a_u[k][j_15][i_9][base3 + 2] +
                    8.0 * (- a_u[k][j_15][i_3][base3 + 2] + a_u[k][j_15][i_7][
                    base3 + 2])) - 8.0 * (a_mu[k][j_14][i_1] * (a_u[k][j_14][
                    i_5][base3 + 2] - a_u[k][j_14][i_9][base3 + 2] + 8.0 * (-
                    a_u[k][j_14][i_3][base3 + 2] + a_u[k][j_14][i_7][base3 + 2]
                    ))) + 8.0 * (a_mu[k][j_16][i_1] * (a_u[k][j_16][i_5][base3 +
                    2] - a_u[k][j_16][i_9][base3 + 2] + 8.0 * (- a_u[k][j_16][
                    i_3][base3 + 2] + a_u[k][j_16][i_7][base3 + 2]))) - a_mu[k]
                    [j_17][i_1] * (a_u[k][j_17][i_5][base3 + 2] - a_u[k][j_17][
                    i_9][base3 + 2] + 8.0 * (- a_u[k][j_17][i_3][base3 + 2] +
                    a_u[k][j_17][i_7][base3 + 2])))),
                 u3zim2 = 0.0,
                 qb_2 = 1,
                 u3zip2 = 0.0,
                 u3zip1 = 0.0,
                 u3zim1 = 0.0,
                 qb_3 = 0;
                 qb_2 <= 8; ) {
               int _t26;
               int _t27;
               _t27 = qb_3 + 1;
               _t26 = qb_2 + 1;
               u3zim1 = u3zim1 - a_bope[qb_3][_t10 + -1] * a_u[nk + - qb_2 + 1]
                  [j_13][i_3][base3 + 3];
               u3zim2 = u3zim2 - a_bope[qb_3][_t10 + -1] * a_u[nk + - qb_2 + 1]
                  [j_13][i_5][base3 + 3];
               u3zip2 = u3zip2 - a_bope[qb_3][_t10 + -1] * a_u[nk + - qb_2 + 1]
                  [j_13][i_9][base3 + 3];
               u3zip1 = u3zip1 - a_bope[qb_3][_t10 + -1] * a_u[nk + - qb_2 + 1]
                  [j_13][i_7][base3 + 3];
               qb_2 = _t26;
               qb_3 = _t27;
            }
            for (_t14 = _t13 + a_strx[i + - ifirst] * (0.08333333333333333 * (-
                     a_lambda[k][j_13][i_8] * u3zip2 + 8.0 * a_lambda[k][j_13][
                    i_6] * u3zip1 - 8.0 * a_lambda[k][j_13][i_2] * u3zim1 +
                    a_lambda[k][j_13][i_4] * u3zim2)),
                 mu3xz = 0.0,
                 qb_4 = 1,
                 qb_5 = 0;
                 qb_4 <= 8; ) {
               int _t28;
               int _t29;
               _t29 = qb_5 + 1;
               _t28 = qb_4 + 1;
               mu3xz = mu3xz - a_bope[qb_5][_t10 + -1] * (a_mu[nk + - qb_4 + 1]
                  [j_13][i_1] * 0.08333333333333333 * (- a_u[nk + - qb_4 + 1][
                  j_13][i_9][base3 + 3] + 8.0 * a_u[nk + - qb_4 + 1][j_13][i_7]
                  [base3 + 3] - 8.0 * a_u[nk + - qb_4 + 1][j_13][i_3][base3 + 3
                  ] + a_u[nk + - qb_4 + 1][j_13][i_5][base3 + 3]));
               qb_4 = _t28;
               qb_5 = _t29;
            }
            for (_t16 = _t11 + a_strx[i + - ifirst] * a_stry[j_13 + - jfirst] *
                    (0.006944444444444444 * (a_mu[k][j_13][i_4] * (a_u[k][j_15]
                    [i_5][base3 + 1] - a_u[k][j_17][i_5][base3 + 1] + 8.0 * (-
                    a_u[k][j_14][i_5][base3 + 1] + a_u[k][j_16][i_5][base3 + 1]
                    )) - 8.0 * (a_mu[k][j_13][i_2] * (a_u[k][j_15][i_3][base3 +
                    1] - a_u[k][j_17][i_3][base3 + 1] + 8.0 * (- a_u[k][j_14][
                    i_3][base3 + 1] + a_u[k][j_16][i_3][base3 + 1]))) + 8.0 * (
                    a_mu[k][j_13][i_6] * (a_u[k][j_15][i_7][base3 + 1] - a_u[k]
                    [j_17][i_7][base3 + 1] + 8.0 * (- a_u[k][j_14][i_7][base3 +
                    1] + a_u[k][j_16][i_7][base3 + 1]))) - a_mu[k][j_13][i_8] *
                    (a_u[k][j_15][i_9][base3 + 1] - a_u[k][j_17][i_9][base3 + 1
                    ] + 8.0 * (- a_u[k][j_14][i_9][base3 + 1] + a_u[k][j_16][
                    i_9][base3 + 1]))) + 0.006944444444444444 * (a_lambda[k][
                    j_15][i_1] * (a_u[k][j_15][i_5][base3 + 1] - a_u[k][j_15][
                    i_9][base3 + 1] + 8.0 * (- a_u[k][j_15][i_3][base3 + 1] +
                    a_u[k][j_15][i_7][base3 + 1])) - 8.0 * (a_lambda[k][j_14][
                    i_1] * (a_u[k][j_14][i_5][base3 + 1] - a_u[k][j_14][i_9][
                    base3 + 1] + 8.0 * (- a_u[k][j_14][i_3][base3 + 1] + a_u[k]
                    [j_14][i_7][base3 + 1]))) + 8.0 * (a_lambda[k][j_16][i_1] *
                    (a_u[k][j_16][i_5][base3 + 1] - a_u[k][j_16][i_9][base3 + 1
                    ] + 8.0 * (- a_u[k][j_16][i_3][base3 + 1] + a_u[k][j_16][
                    i_7][base3 + 1]))) - a_lambda[k][j_17][i_1] * (a_u[k][j_17]
                    [i_5][base3 + 1] - a_u[k][j_17][i_9][base3 + 1] + 8.0 * (-
                    a_u[k][j_17][i_3][base3 + 1] + a_u[k][j_17][i_7][base3 + 1]
                    )))),
                 _t15 = _t14 + a_strx[i + - ifirst] * mu3xz,
                 u3zjm2 = 0.0,
                 qb_6 = 1,
                 u3zjp2 = 0.0,
                 u3zjp1 = 0.0,
                 u3zjm1 = 0.0,
                 qb_7 = 0;
                 qb_6 <= 8; ) {
               int _t30;
               int _t31;
               u3zjm1 = u3zjm1 - a_bope[qb_7][_t10 + -1] * a_u[nk + - qb_6 + 1]
                  [j_14][i][base3 + 3];
               _t31 = qb_7 + 1;
               _t30 = qb_6 + 1;
               u3zjm2 = u3zjm2 - a_bope[qb_7][_t10 + -1] * a_u[nk + - qb_6 + 1]
                  [j_15][i][base3 + 3];
               u3zjp2 = u3zjp2 - a_bope[qb_7][_t10 + -1] * a_u[nk + - qb_6 + 1]
                  [j_17][i][base3 + 3];
               u3zjp1 = u3zjp1 - a_bope[qb_7][_t10 + -1] * a_u[nk + - qb_6 + 1]
                  [j_16][i][base3 + 3];
               qb_6 = _t30;
               qb_7 = _t31;
            }
            for (_t17 = _t16 + a_stry[j_13 + - jfirst] * (0.08333333333333333 *
                    (- a_lambda[k][j_17][i_1] * u3zjp2 + 8.0 * a_lambda[k][j_16
                    ][i_1] * u3zjp1 - 8.0 * a_lambda[k][j_14][i_1] * u3zjm1 +
                    a_lambda[k][j_15][i_1] * u3zjm2)),
                 mu3yz = 0.0,
                 qb_8 = 1,
                 qb_9 = 0;
                 qb_8 <= 8; ) {
               int _t32;
               int _t33;
               _t33 = qb_9 + 1;
               _t32 = qb_8 + 1;
               mu3yz = mu3yz - a_bope[qb_9][_t10 + -1] * (a_mu[nk + - qb_8 + 1]
                  [j_13][i_1] * 0.08333333333333333 * (- a_u[nk + - qb_8 + 1][
                  j_17][i][base3 + 3] + 8.0 * a_u[nk + - qb_8 + 1][j_16][i][
                  base3 + 3] - 8.0 * a_u[nk + - qb_8 + 1][j_14][i][base3 + 3] +
                  a_u[nk + - qb_8 + 1][j_15][i][base3 + 3]));
               qb_8 = _t32;
               qb_9 = _t33;
            }
            for (_t18 = _t17 + a_stry[j_13 + - jfirst] * mu3yz,
                 u1zim2 = 0.0,
                 qb_10 = 1,
                 u1zip2 = 0.0,
                 u1zip1 = 0.0,
                 u1zim1 = 0.0,
                 qb_11 = 0;
                 qb_10 <= 8; ) {
               int _t34;
               int _t35;
               _t35 = qb_11 + 1;
               _t34 = qb_10 + 1;
               u1zim1 = u1zim1 - a_bope[qb_11][_t10 + -1] * a_u[nk + - qb_10 + 1
                  ][j_13][i_3][base3 + 1];
               u1zim2 = u1zim2 - a_bope[qb_11][_t10 + -1] * a_u[nk + - qb_10 + 1
                  ][j_13][i_5][base3 + 1];
               u1zip2 = u1zip2 - a_bope[qb_11][_t10 + -1] * a_u[nk + - qb_10 + 1
                  ][j_13][i_9][base3 + 1];
               u1zip1 = u1zip1 - a_bope[qb_11][_t10 + -1] * a_u[nk + - qb_10 + 1
                  ][j_13][i_7][base3 + 1];
               qb_10 = _t34;
               qb_11 = _t35;
            }
            for (_t19 = _t12 + a_strx[i + - ifirst] * (0.08333333333333333 * (-
                     a_mu[k][j_13][i_8] * u1zip2 + 8.0 * a_mu[k][j_13][i_6] *
                    u1zip1 - 8.0 * a_mu[k][j_13][i_2] * u1zim1 + a_mu[k][j_13][
                    i_4] * u1zim2)),
                 u2zjm2 = 0.0,
                 qb_12 = 1,
                 u2zjp2 = 0.0,
                 u2zjp1 = 0.0,
                 u2zjm1 = 0.0,
                 qb_13 = 0;
                 qb_12 <= 8; ) {
               int _t36;
               int _t37;
               u2zjm1 = u2zjm1 - a_bope[qb_13][_t10 + -1] * a_u[nk + - qb_12 + 1
                  ][j_14][i][base3 + 2];
               _t37 = qb_13 + 1;
               _t36 = qb_12 + 1;
               u2zjm2 = u2zjm2 - a_bope[qb_13][_t10 + -1] * a_u[nk + - qb_12 + 1
                  ][j_15][i][base3 + 2];
               u2zjp2 = u2zjp2 - a_bope[qb_13][_t10 + -1] * a_u[nk + - qb_12 + 1
                  ][j_17][i][base3 + 2];
               u2zjp1 = u2zjp1 - a_bope[qb_13][_t10 + -1] * a_u[nk + - qb_12 + 1
                  ][j_16][i][base3 + 2];
               qb_12 = _t36;
               qb_13 = _t37;
            }
            for (_t20 = _t19 + a_stry[j_13 + - jfirst] * (0.08333333333333333 *
                    (- a_mu[k][j_17][i_1] * u2zjp2 + 8.0 * a_mu[k][j_16][i_1] *
                    u2zjp1 - 8.0 * a_mu[k][j_14][i_1] * u2zjm1 + a_mu[k][j_15][
                    i_1] * u2zjm2)),
                 lau1xz = 0.0,
                 qb_14 = 1,
                 qb_15 = 0;
                 qb_14 <= 8; ) {
               int _t38;
               int _t39;
               _t39 = qb_15 + 1;
               _t38 = qb_14 + 1;
               lau1xz = lau1xz - a_bope[qb_15][_t10 + -1] * (a_lambda[nk + -
                  qb_14 + 1][j_13][i_1] * 0.08333333333333333 * (- a_u[nk + -
                  qb_14 + 1][j_13][i_9][base3 + 1] + 8.0 * a_u[nk + - qb_14 + 1
                  ][j_13][i_7][base3 + 1] - 8.0 * a_u[nk + - qb_14 + 1][j_13][
                  i_3][base3 + 1] + a_u[nk + - qb_14 + 1][j_13][i_5][base3 + 1]
                  ));
               qb_14 = _t38;
               qb_15 = _t39;
            }
            for (_t21 = _t20 + a_strx[i + - ifirst] * lau1xz,
                 lau2yz = 0.0,
                 qb_16 = 1,
                 qb_17 = 0;
                 qb_16 <= 8; ) {
               int _t40;
               int _t41;
               _t41 = qb_17 + 1;
               _t40 = qb_16 + 1;
               lau2yz = lau2yz - a_bope[qb_17][_t10 + -1] * (a_lambda[nk + -
                  qb_16 + 1][j_13][i_1] * 0.08333333333333333 * (- a_u[nk + -
                  qb_16 + 1][j_17][i][base3 + 2] + 8.0 * a_u[nk + - qb_16 + 1][
                  j_16][i][base3 + 2] - 8.0 * a_u[nk + - qb_16 + 1][j_14][i][
                  base3 + 2] + a_u[nk + - qb_16 + 1][j_15][i][base3 + 2]));
               qb_16 = _t40;
               qb_17 = _t41;
            }
            _t22 = a_stry[j_13 + - jfirst];
            a_lu[k][j_13][i][base3 + 1] = 0.0 * a_lu[k][j_13][i][base3 + 1] +
               cof * _t15;
            a_lu[k][j_13][i][base3 + 2] = 0.0 * a_lu[k][j_13][i][base3 + 2] +
               cof * _t18;
            a_lu[k][j_13][i][base3 + 3] = 0.0 * a_lu[k][j_13][i][base3 + 3] +
               cof * (_t21 + _t22 * lau2yz);
         }
      }
   }
}

/*
 * ../src/rhs4sg.c:1647.6
 */
void mapped_fn1(int ifirst, int ilast, int jfirst, int jlast, int kfirst, int
   klast, int nk, int* onesided_4, double (* a_acof)[8][6], double (* a_bope)[6
   ], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][25][3],
   double (* a_mu)[25][25], double (* a_lambda)[25][25], double h, double*
   a_strx, double* a_stry, double* a_strz, int k1, int k2, int base, int base3)
{
   double h_var;
   ocrGuid_t outEvt;
   union __args_mapped_fn1_dpbr_main_40* allArgs;
   void* _t1;
   h_var = h;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, 0);
   _t1 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t1, mapped_fn1_dpbr_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t1, mapped_fn1_dpbr4, 0u, (unsigned long)(1 + k2 + -1 * k1)
      , 0u);
   if (jfirst + - jlast == -24 && ifirst + - ilast == -24) {
      ocrGuid_t _t2;
      union __args_mapped_fn1_dpbr_main_40* _t3;
      _t2 = rocrAlloc((void**)&allArgs, 112ul);
      _t3 = allArgs;
      _t3->data.h = &h_var;
      _t3->data.a_mu = a_mu;
      _t3->data.a_strx = a_strx;
      _t3->data.a_lu = a_lu;
      _t3->data.a_u = a_u;
      _t3->data.a_lambda = a_lambda;
      _t3->data.a_stry = a_stry;
      _t3->data.a_strz = a_strz;
      _t3->data.ifirst = ifirst;
      _t3->data.ilast = ilast;
      _t3->data.jfirst = jfirst;
      _t3->data.jlast = jlast;
      _t3->data.kfirst = kfirst;
      _t3->data.klast = klast;
      _t3->data.nk = nk;
      _t3->data.k1 = k1;
      _t3->data.k2 = k2;
      _t3->data.base = base;
      _t3->data.base3 = base3;
      rocrExecute(_t1, mapped_fn1_dpbr_main, _t2, &outEvt);
   }
   rocrExit(_t1);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

/*
 * ../src/rhs4sg.c:1660.6
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
 * ../src/rhs4sg.c:1673.6
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
 * ../src/rhs4sg.c:1685.6
 */
void kernel(void)
{
   int k2;
   int k1;
   if (onesided[4] == 1) {
      k1 = 7;
   } else {
      k1 = 2;
   }
   if (onesided[5] == 1) {
      k2 = 19;
   } else {
      k2 = 22;
   }
   mapped_fn1(0, 24, 0, 24, 0, 24, 25, onesided, acof, bope, ghcof, lu, u, mu,
      lambda, 0.041666666666666664, strx, stry, strz, k1, k2, 0, -1);
   if (onesided[4] == 1) {
      mapped_fn2(0, 24, 0, 24, 0, 24, 25, onesided, acof, bope, ghcof, lu, u,
         mu, lambda, 0.041666666666666664, strx, stry, strz, k1, k2, 0, -1);
   }
   if (onesided[5] == 1) {
      mapped_fn3(0, 24, 0, 24, 0, 24, 25, onesided, acof, bope, ghcof, lu, u,
         mu, lambda, 0.041666666666666664, strx, stry, strz, k1, k2, 0, -1);
   }
}

/*
 * ../src/rhs4sg.c:1727.13
 */
static void unmapped_kernel(double (* lu_1)[25][25][3])
{
   int k2;
   int k1;
   if (onesided[4] == 1) {
      k1 = 7;
   } else {
      k1 = 2;
   }
   if (onesided[5] == 1) {
      k2 = 19;
   } else {
      k2 = 22;
   }
   core_fn1(0, 24, 0, 24, 0, 24, 25, onesided, acof, bope, ghcof, lu_1, u, mu,
      lambda, 0.041666666666666664, strx, stry, strz, k1, k2, 0, -1);
   if (onesided[4] == 1) {
      core_fn2(0, 24, 0, 24, 0, 24, 25, onesided, acof, bope, ghcof, lu_1, u,
         mu, lambda, 0.041666666666666664, strx, stry, strz, k1, k2, 0, -1);
   }
   if (onesided[5] == 1) {
      core_fn3(0, 24, 0, 24, 0, 24, 25, onesided, acof, bope, ghcof, lu_1, u,
         mu, lambda, 0.041666666666666664, strx, stry, strz, k1, k2, 0, -1);
   }
}

/*
 * ../src/rhs4sg.c:1769.6
 */
void show(void)
{
}

/*
 * ../src/rhs4sg.c:1772.5
 */
int check(void)
{
   double (* _t1)[25][25][3];
   int check_result;
   int k;
   _t1 = (double (*)[25][25][3])calloc(46875ul, 8ul);
   unmapped_kernel(_t1);
   for (k = 0; k < 25; k++) {
      int j_18;
      for (j_18 = 0; j_18 < 25; j_18++) {
         int i;
         for (i = 0; i < 25; i++) {
            int c;
            c = 0;
            while (1) {
               if (c < 3) {
                  if (!(fequal(lu[k][j_18][i][c], _t1[k][j_18][i][c]) != 0))
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
 * ../src/rhs4sg.c:1794.8
 */
double flops_per_trial(void)
{
   return 6167826.0;
}

/*
 * ../src/rhs4sg.c:1647.6
 */
static void mapped_fn1_1(int ifirst, int ilast, int jfirst, int jlast, int
   kfirst, int klast, int nk, int* onesided_7, double (* a_acof)[8][6], double
   (* a_bope)[6], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[
   25][25][3], double (* a_mu)[25][25], double (* a_lambda)[25][25], double h,
   double* a_strx, double* a_stry, double* a_strz, int k1, int k2, int base,
   int base3)
{
   double _t1;
   int k;
   int k_1;
   int k_2;
   int k_3;
   int k_4;
   for (_t1 = 1.0 / (h * h),
        k_1 = k1 + -1,
        k_2 = k1 + -2,
        k_3 = k1 + 1,
        k_4 = k1 + 2,
        k = k1;
        k <= k2;
        k++,
        k_1++,
        k_2++,
        k_3++,
        k_4++) {
      int j_19;
      int j_20;
      int j_21;
      int j_22;
      int j_23;
      for (j_19 = jfirst + 2,
           j_23 = jfirst + 4,
           j_20 = jfirst + 1,
           j_22 = jfirst + 3,
           j_21 = jfirst + 0;
           j_19 <= jlast + -2;
           j_19++,
           j_20++,
           j_21++,
           j_22++,
           j_23++) {
         int i;
         int i_1;
         int i_2;
         int i_3;
         int i_4;
         int i_5;
         int i_6;
         int i_7;
         int i_8;
         int i_9;
         for (i = ifirst + 2,
              i_1 = i + base,
              i_2 = i_1 + -1,
              i_9 = ifirst + 4,
              i_3 = ifirst + 1,
              i_4 = i_1 + -2,
              i_7 = ifirst + 3,
              i_5 = ifirst + 0,
              i_6 = i_1 + 1,
              i_8 = i_1 + 2;
              i <= ilast + -2;
              i_9++,
              i++,
              i_8++,
              i_7++,
              i_6++,
              i_5++,
              i_4++,
              i_3++,
              i_2++,
              i_1++) {
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
            double _t233;
            double _t234;
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
            _t84 = a_strx[i + - ifirst];
            _t86 = a_u[k][j_19][i_5][base3 + 2];
            _t87 = a_u[k][j_19][i][base3 + 2];
            _t88 = a_u[k][j_19][i_3][base3 + 2];
            _t90 = a_u[k][j_19][i][base3 + 2];
            _t91 = a_u[k][j_19][i_7][base3 + 2];
            _t92 = a_u[k][j_19][i][base3 + 2];
            _t95 = a_u[k][j_19][i_9][base3 + 2];
            _t96 = a_u[k][j_19][i][base3 + 2];
            _t97 = a_stry[j_19 + - jfirst];
            _t99 = a_lambda[k][j_20][i_1];
            _t100 = a_stry[j_20 + - jfirst];
            _t101 = a_lambda[k][j_19][i_1];
            _t102 = a_stry[j_19 + - jfirst];
            _t104 = a_lambda[k][j_21][i_1];
            _t105 = a_stry[j_21 + - jfirst];
            _t106 = a_u[k][j_21][i][base3 + 2];
            _t109 = a_u[k][j_19][i][base3 + 2];
            _t110 = a_lambda[k][j_21][i_1];
            _t112 = a_stry[j_21 + - jfirst];
            _t114 = a_lambda[k][j_22][i_1];
            _t115 = a_stry[j_22 + - jfirst];
            _t117 = a_lambda[k][j_19][i_1];
            _t118 = a_stry[j_19 + - jfirst];
            _t119 = a_lambda[k][j_20][i_1];
            _t120 = a_stry[j_20 + - jfirst];
            _t122 = a_u[k][j_20][i][base3 + 2];
            _t124 = a_u[k][j_19][i][base3 + 2];
            _t126 = a_lambda[k][j_20][i_1];
            _t128 = a_stry[j_20 + - jfirst];
            _t129 = a_lambda[k][j_23][i_1];
            _t130 = a_stry[j_23 + - jfirst];
            _t132 = a_lambda[k][j_22][i_1];
            _t133 = a_stry[j_22 + - jfirst];
            _t134 = a_lambda[k][j_19][i_1];
            _t135 = a_stry[j_19 + - jfirst];
            _t137 = a_u[k][j_22][i][base3 + 2];
            _t139 = a_u[k][j_19][i][base3 + 2];
            _t141 = a_lambda[k][j_22][i_1];
            _t142 = a_stry[j_22 + - jfirst];
            _t143 = a_lambda[k][j_19][i_1];
            _t144 = a_stry[j_19 + - jfirst];
            _t146 = a_lambda[k][j_23][i_1];
            _t147 = a_stry[j_23 + - jfirst];
            _t148 = a_u[k][j_23][i][base3 + 2];
            _t150 = a_u[k][j_19][i][base3 + 2];
            _t151 = a_strz[k + - kfirst];
            _t153 = a_u[k_2][j_19][i][base3 + 2];
            _t154 = a_u[k][j_19][i][base3 + 2];
            _t156 = a_u[k_1][j_19][i][base3 + 2];
            _t158 = a_u[k][j_19][i][base3 + 2];
            _t160 = a_u[k_3][j_19][i][base3 + 2];
            _t161 = a_u[k][j_19][i][base3 + 2];
            _t163 = a_u[k_4][j_19][i][base3 + 2];
            _t164 = a_u[k][j_19][i][base3 + 2];
            _t166 = a_strx[i + - ifirst];
            _t167 = a_u[k][j_19][i_5][base3 + 3];
            _t169 = a_u[k][j_19][i][base3 + 3];
            _t170 = a_u[k][j_19][i_3][base3 + 3];
            _t172 = a_u[k][j_19][i][base3 + 3];
            _t173 = a_u[k][j_19][i_7][base3 + 3];
            _t175 = a_u[k][j_19][i][base3 + 3];
            _t177 = a_u[k][j_19][i_9][base3 + 3];
            _t178 = a_u[k][j_19][i][base3 + 3];
            _t180 = a_stry[j_19 + - jfirst];
            _t183 = a_u[k][j_21][i][base3 + 3];
            _t184 = a_u[k][j_19][i][base3 + 3];
            _t185 = a_u[k][j_20][i][base3 + 3];
            _t187 = a_u[k][j_19][i][base3 + 3];
            _t189 = a_u[k][j_22][i][base3 + 3];
            _t190 = a_u[k][j_19][i][base3 + 3];
            _t192 = a_u[k][j_23][i][base3 + 3];
            _t193 = a_u[k][j_19][i][base3 + 3];
            _t194 = a_strz[k + - kfirst];
            _t198 = a_lambda[k_1][j_19][i_1];
            _t199 = a_strz[k_1 + - kfirst];
            _t200 = a_lambda[k][j_19][i_1];
            _t201 = a_strz[k + - kfirst];
            _t203 = a_lambda[k_2][j_19][i_1];
            _t204 = a_strz[k_2 + - kfirst];
            _t205 = a_u[k_2][j_19][i][base3 + 3];
            _t207 = a_u[k][j_19][i][base3 + 3];
            _t209 = a_lambda[k_2][j_19][i_1];
            _t210 = a_strz[k_2 + - kfirst];
            _t212 = a_lambda[k_3][j_19][i_1];
            _t213 = a_strz[k_3 + - kfirst];
            _t214 = a_lambda[k][j_19][i_1];
            _t215 = a_strz[k + - kfirst];
            _t217 = a_lambda[k_1][j_19][i_1];
            _t219 = a_strz[k_1 + - kfirst];
            _t221 = a_u[k_1][j_19][i][base3 + 3];
            _t223 = a_u[k][j_19][i][base3 + 3];
            _t225 = a_lambda[k_1][j_19][i_1];
            _t226 = a_strz[k_1 + - kfirst];
            _t227 = a_lambda[k_4][j_19][i_1];
            _t228 = a_strz[k_4 + - kfirst];
            _t230 = a_lambda[k_3][j_19][i_1];
            _t231 = a_strz[k_3 + - kfirst];
            _t232 = a_lambda[k][j_19][i_1];
            _t233 = a_strz[k + - kfirst];
            _t236 = a_u[k_3][j_19][i][base3 + 3];
            _t237 = a_u[k][j_19][i][base3 + 3];
            _t240 = a_lambda[k_3][j_19][i_1];
            _t241 = a_strz[k_3 + - kfirst];
            _t242 = a_lambda[k][j_19][i_1];
            _t243 = a_strz[k + - kfirst];
            _t245 = a_lambda[k_4][j_19][i_1];
            _t246 = a_strz[k_4 + - kfirst];
            _t247 = a_u[k_4][j_19][i][base3 + 3];
            _t249 = a_u[k][j_19][i][base3 + 3];
            _t77 = a_strx[i + - ifirst];
            _t9 = a_stry[j_19 + - jfirst];
            _t283 = a_mu[k][j_19][i_4];
            _t293 = a_u[k][j_21][i_5][base3 + 1];
            _t281 = a_u[k][j_23][i_5][base3 + 1];
            _t268 = a_u[k][j_20][i_5][base3 + 1];
            _t255 = a_u[k][j_22][i_5][base3 + 1];
            _t218 = a_mu[k][j_19][i_2];
            _t127 = a_u[k][j_21][i_3][base3 + 1];
            _t79 = a_u[k][j_23][i_3][base3 + 1];
            _t41 = a_u[k][j_20][i_3][base3 + 1];
            _t21 = a_u[k][j_22][i_3][base3 + 1];
            _t2 = a_mu[k][j_19][i_6];
            _t5 = a_u[k][j_21][i_7][base3 + 1];
            _t8 = a_u[k][j_23][i_7][base3 + 1];
            _t12 = a_u[k][j_20][i_7][base3 + 1];
            _t20 = a_u[k][j_22][i_7][base3 + 1];
            _t26 = a_mu[k][j_19][i_8];
            _t31 = a_u[k][j_21][i_9][base3 + 1];
            _t38 = a_u[k][j_23][i_9][base3 + 1];
            _t44 = a_u[k][j_20][i_9][base3 + 1];
            _t49 = a_u[k][j_22][i_9][base3 + 1];
            _t54 = a_strx[i + - ifirst];
            _t58 = a_stry[j_19 + - jfirst];
            _t65 = a_lambda[k][j_21][i_1];
            _t71 = a_u[k][j_21][i_5][base3 + 1];
            _t75 = a_u[k][j_21][i_9][base3 + 1];
            _t82 = a_u[k][j_21][i_3][base3 + 1];
            _t93 = a_u[k][j_21][i_7][base3 + 1];
            _t107 = a_lambda[k][j_20][i_1];
            _t123 = a_u[k][j_20][i_5][base3 + 1];
            _t138 = a_u[k][j_20][i_9][base3 + 1];
            _t152 = a_u[k][j_20][i_3][base3 + 1];
            _t165 = a_u[k][j_20][i_7][base3 + 1];
            _t181 = a_lambda[k][j_22][i_1];
            _t195 = a_u[k][j_22][i_5][base3 + 1];
            _t208 = a_u[k][j_22][i_9][base3 + 1];
            _t222 = a_u[k][j_22][i_3][base3 + 1];
            _t238 = a_u[k][j_22][i_7][base3 + 1];
            _t251 = a_lambda[k][j_23][i_1];
            _t256 = a_u[k][j_23][i_5][base3 + 1];
            _t260 = a_u[k][j_23][i_9][base3 + 1];
            _t266 = a_u[k][j_23][i_3][base3 + 1];
            _t272 = a_u[k][j_23][i_7][base3 + 1];
            _t275 = a_stry[j_19 + - jfirst];
            _t279 = a_strz[k + - kfirst];
            _t286 = a_lambda[k][j_21][i_1];
            _t291 = a_u[k_2][j_21][i][base3 + 3];
            _t297 = a_u[k_4][j_21][i][base3 + 3];
            _t288 = a_u[k_1][j_21][i][base3 + 3];
            _t264 = a_u[k_3][j_21][i][base3 + 3];
            _t16 = a_lambda[k][j_20][i_1];
            _t69 = a_u[k_2][j_20][i][base3 + 3];
            _t174 = a_u[k_4][j_20][i][base3 + 3];
            _t270 = a_u[k_1][j_20][i][base3 + 3];
            _t60 = a_u[k_3][j_20][i][base3 + 3];
            _t188 = a_lambda[k][j_22][i_1];
            _t61 = a_u[k_2][j_22][i][base3 + 3];
            _t295 = a_u[k_4][j_22][i][base3 + 3];
            _t277 = a_u[k_1][j_22][i][base3 + 3];
            _t262 = a_u[k_3][j_22][i][base3 + 3];
            _t235 = a_lambda[k][j_23][i_1];
            _t196 = a_u[k_2][j_23][i][base3 + 3];
            _t157 = a_u[k_4][j_23][i][base3 + 3];
            _t111 = a_u[k_1][j_23][i][base3 + 3];
            _t80 = a_u[k_3][j_23][i][base3 + 3];
            _t64 = a_stry[j_19 + - jfirst];
            _t56 = a_strz[k + - kfirst];
            _t36 = a_mu[k_2][j_19][i_1];
            _t17 = a_u[k_2][j_21][i][base3 + 3];
            _t6 = a_u[k_2][j_23][i][base3 + 3];
            _t52 = a_u[k_2][j_20][i][base3 + 3];
            _t252 = a_u[k_2][j_22][i][base3 + 3];
            _t285 = a_mu[k_1][j_19][i_1];
            _t67 = a_u[k_1][j_21][i][base3 + 3];
            _t4 = a_u[k_1][j_23][i][base3 + 3];
            _t179 = a_u[k_1][j_20][i][base3 + 3];
            _t299 = a_u[k_1][j_22][i][base3 + 3];
            _t298 = a_mu[k_3][j_19][i_1];
            _t296 = a_u[k_3][j_21][i][base3 + 3];
            _t294 = a_u[k_3][j_23][i][base3 + 3];
            _t292 = a_u[k_3][j_20][i][base3 + 3];
            _t290 = a_u[k_3][j_22][i][base3 + 3];
            _t289 = a_mu[k_4][j_19][i_1];
            _t287 = a_u[k_4][j_21][i][base3 + 3];
            _t284 = a_u[k_4][j_23][i][base3 + 3];
            _t282 = a_u[k_4][j_20][i][base3 + 3];
            _t280 = a_u[k_4][j_22][i][base3 + 3];
            _t278 = a_strx[i + - ifirst];
            _t276 = a_strz[k + - kfirst];
            _t274 = a_mu[k][j_19][i_4];
            _t273 = a_u[k_2][j_19][i_5][base3 + 1];
            _t271 = a_u[k_4][j_19][i_5][base3 + 1];
            _t269 = a_u[k_1][j_19][i_5][base3 + 1];
            _t267 = a_u[k_3][j_19][i_5][base3 + 1];
            _t265 = a_mu[k][j_19][i_2];
            _t263 = a_u[k_2][j_19][i_3][base3 + 1];
            _t261 = a_u[k_4][j_19][i_3][base3 + 1];
            _t259 = a_u[k_1][j_19][i_3][base3 + 1];
            _t258 = a_u[k_3][j_19][i_3][base3 + 1];
            _t257 = a_mu[k][j_19][i_6];
            _t254 = a_u[k_2][j_19][i_7][base3 + 1];
            _t253 = a_u[k_4][j_19][i_7][base3 + 1];
            _t250 = a_u[k_1][j_19][i_7][base3 + 1];
            _t248 = a_u[k_3][j_19][i_7][base3 + 1];
            _t244 = a_mu[k][j_19][i_8];
            _t239 = a_u[k_2][j_19][i_9][base3 + 1];
            _t234 = a_u[k_4][j_19][i_9][base3 + 1];
            _t229 = a_u[k_1][j_19][i_9][base3 + 1];
            _t224 = a_u[k_3][j_19][i_9][base3 + 1];
            _t220 = a_stry[j_19 + - jfirst];
            _t216 = a_strz[k + - kfirst];
            _t211 = a_mu[k][j_21][i_1];
            _t206 = a_u[k_2][j_21][i][base3 + 2];
            _t202 = a_u[k_4][j_21][i][base3 + 2];
            _t197 = a_u[k_1][j_21][i][base3 + 2];
            _t191 = a_u[k_3][j_21][i][base3 + 2];
            _t186 = a_mu[k][j_20][i_1];
            _t182 = a_u[k_2][j_20][i][base3 + 2];
            _t176 = a_u[k_4][j_20][i][base3 + 2];
            _t171 = a_u[k_1][j_20][i][base3 + 2];
            _t168 = a_u[k_3][j_20][i][base3 + 2];
            _t162 = a_mu[k][j_22][i_1];
            _t159 = a_u[k_2][j_22][i][base3 + 2];
            _t155 = a_u[k_4][j_22][i][base3 + 2];
            _t149 = a_u[k_1][j_22][i][base3 + 2];
            _t145 = a_u[k_3][j_22][i][base3 + 2];
            _t140 = a_mu[k][j_23][i_1];
            _t136 = a_u[k_2][j_23][i][base3 + 2];
            _t131 = a_u[k_4][j_23][i][base3 + 2];
            _t125 = a_u[k_1][j_23][i][base3 + 2];
            _t121 = a_u[k_3][j_23][i][base3 + 2];
            _t116 = a_strx[i + - ifirst];
            _t113 = a_strz[k + - kfirst];
            _t108 = a_lambda[k_2][j_19][i_1];
            _t103 = a_u[k_2][j_19][i_5][base3 + 1];
            _t98 = a_u[k_2][j_19][i_9][base3 + 1];
            _t94 = a_u[k_2][j_19][i_3][base3 + 1];
            _t89 = a_u[k_2][j_19][i_7][base3 + 1];
            _t85 = a_lambda[k_1][j_19][i_1];
            _t83 = a_u[k_1][j_19][i_5][base3 + 1];
            _t81 = a_u[k_1][j_19][i_9][base3 + 1];
            _t78 = a_u[k_1][j_19][i_3][base3 + 1];
            _t76 = a_u[k_1][j_19][i_7][base3 + 1];
            _t74 = a_lambda[k_3][j_19][i_1];
            _t73 = a_u[k_3][j_19][i_5][base3 + 1];
            _t72 = a_u[k_3][j_19][i_9][base3 + 1];
            _t70 = a_u[k_3][j_19][i_3][base3 + 1];
            _t68 = a_u[k_3][j_19][i_7][base3 + 1];
            _t66 = a_lambda[k_4][j_19][i_1];
            _t63 = a_u[k_4][j_19][i_5][base3 + 1];
            _t62 = a_u[k_4][j_19][i_9][base3 + 1];
            _t59 = a_u[k_4][j_19][i_3][base3 + 1];
            _t57 = a_u[k_4][j_19][i_7][base3 + 1];
            _t55 = a_stry[j_19 + - jfirst];
            _t53 = a_strz[k + - kfirst];
            _t51 = a_lambda[k_2][j_19][i_1];
            _t50 = a_u[k_2][j_21][i][base3 + 2];
            _t47 = a_u[k_2][j_23][i][base3 + 2];
            _t46 = a_u[k_2][j_20][i][base3 + 2];
            _t43 = a_u[k_2][j_22][i][base3 + 2];
            _t42 = a_lambda[k_1][j_19][i_1];
            _t39 = a_u[k_1][j_21][i][base3 + 2];
            _t37 = a_u[k_1][j_23][i][base3 + 2];
            _t34 = a_u[k_1][j_20][i][base3 + 2];
            _t33 = a_u[k_1][j_22][i][base3 + 2];
            _t30 = a_lambda[k_3][j_19][i_1];
            _t29 = a_u[k_3][j_21][i][base3 + 2];
            _t27 = a_u[k_3][j_23][i][base3 + 2];
            _t25 = a_u[k_3][j_20][i][base3 + 2];
            _t23 = a_u[k_3][j_22][i][base3 + 2];
            _t22 = a_lambda[k_4][j_19][i_1];
            _t18 = a_u[k_4][j_21][i][base3 + 2];
            _t15 = a_u[k_4][j_23][i][base3 + 2];
            _t13 = a_u[k_4][j_20][i][base3 + 2];
            _t11 = a_u[k_4][j_22][i][base3 + 2];
            _t48 = a_mu[k_3][j_19][i_1] * a_strz[k_3 + - kfirst] - 0.75 * (a_mu
               [k][j_19][i_1] * a_strz[k + - kfirst] + a_mu[k_4][j_19][i_1] *
               a_strz[k_4 + - kfirst]);
            _t45 = a_mu[k_1][j_19][i_1] * a_strz[k_1 + - kfirst] + a_mu[k_4][
               j_19][i_1] * a_strz[k_4 + - kfirst] + 3.0 * (a_mu[k_3][j_19][i_1
               ] * a_strz[k_3 + - kfirst] + a_mu[k][j_19][i_1] * a_strz[k + -
               kfirst]);
            _t40 = a_mu[k_2][j_19][i_1] * a_strz[k_2 + - kfirst] + a_mu[k_3][
               j_19][i_1] * a_strz[k_3 + - kfirst] + 3.0 * (a_mu[k][j_19][i_1] *
               a_strz[k + - kfirst] + a_mu[k_1][j_19][i_1] * a_strz[k_1 + -
               kfirst]);
            _t35 = a_mu[k_1][j_19][i_1] * a_strz[k_1 + - kfirst] - 0.75 * (a_mu
               [k][j_19][i_1] * a_strz[k + - kfirst] + a_mu[k_2][j_19][i_1] *
               a_strz[k_2 + - kfirst]);
            _t32 = a_mu[k][j_22][i_1] * a_stry[j_22 + - jfirst] - 0.75 * (a_mu[
               k][j_19][i_1] * a_stry[j_19 + - jfirst] + a_mu[k][j_23][i_1] *
               a_stry[j_23 + - jfirst]);
            _t28 = a_mu[k][j_20][i_1] * a_stry[j_20 + - jfirst] + a_mu[k][j_23]
               [i_1] * a_stry[j_23 + - jfirst] + 3.0 * (a_mu[k][j_22][i_1] *
               a_stry[j_22 + - jfirst] + a_mu[k][j_19][i_1] * a_stry[j_19 + -
               jfirst]);
            _t24 = a_mu[k][j_21][i_1] * a_stry[j_21 + - jfirst] + a_mu[k][j_22]
               [i_1] * a_stry[j_22 + - jfirst] + 3.0 * (a_mu[k][j_19][i_1] *
               a_stry[j_19 + - jfirst] + a_mu[k][j_20][i_1] * a_stry[j_20 + -
               jfirst]);
            _t19 = a_mu[k][j_20][i_1] * a_stry[j_20 + - jfirst] - 0.75 * (a_mu[
               k][j_19][i_1] * a_stry[j_19 + - jfirst] + a_mu[k][j_21][i_1] *
               a_stry[j_21 + - jfirst]);
            _t14 = a_mu[k][j_19][i_6] * a_strx[i_7 + - ifirst] - 0.75 * (a_mu[k
               ][j_19][i_1] * a_strx[i + - ifirst] + a_mu[k][j_19][i_8] * a_strx
               [i_9 + - ifirst]);
            _t10 = a_mu[k][j_19][i_2] * a_strx[i_3 + - ifirst] + a_mu[k][j_19][
               i_8] * a_strx[i_9 + - ifirst] + 3.0 * (a_mu[k][j_19][i_6] *
               a_strx[i_7 + - ifirst] + a_mu[k][j_19][i_1] * a_strx[i + -
               ifirst]);
            _t7 = a_mu[k][j_19][i_4] * a_strx[i_5 + - ifirst] + a_mu[k][j_19][
               i_6] * a_strx[i_7 + - ifirst] + 3.0 * (a_mu[k][j_19][i_1] *
               a_strx[i + - ifirst] + a_mu[k][j_19][i_2] * a_strx[i_3 + -
               ifirst]);
            _t3 = a_mu[k][j_19][i_2] * a_strx[i_3 + - ifirst] - 0.75 * (a_mu[k]
               [j_19][i_1] * a_strx[i + - ifirst] + a_mu[k][j_19][i_4] * a_strx
               [i_5 + - ifirst]);
            a_lu[k][j_19][i][base3 + 1] = 0.0 * a_lu[k][j_19][i][base3 + 1] +
               _t1 * (0.16666666666666666 * (a_strx[i + - ifirst] * ((2.0 * _t3
               + a_lambda[k][j_19][i_2] * a_strx[i_3 + - ifirst] - 0.75 * (
               a_lambda[k][j_19][i_1] * a_strx[i + - ifirst] + a_lambda[k][j_19
               ][i_4] * a_strx[i_5 + - ifirst])) * (a_u[k][j_19][i_5][base3 + 1
               ] - a_u[k][j_19][i][base3 + 1]) + (2.0 * _t7 + a_lambda[k][j_19]
               [i_4] * a_strx[i_5 + - ifirst] + a_lambda[k][j_19][i_6] * a_strx
               [i_7 + - ifirst] + 3.0 * (a_lambda[k][j_19][i_1] * a_strx[i + -
               ifirst] + a_lambda[k][j_19][i_2] * a_strx[i_3 + - ifirst])) * (
               a_u[k][j_19][i_3][base3 + 1] - a_u[k][j_19][i][base3 + 1]) + (
               2.0 * _t10 + a_lambda[k][j_19][i_2] * a_strx[i_3 + - ifirst] +
               a_lambda[k][j_19][i_8] * a_strx[i_9 + - ifirst] + 3.0 * (
               a_lambda[k][j_19][i_6] * a_strx[i_7 + - ifirst] + a_lambda[k][
               j_19][i_1] * a_strx[i + - ifirst])) * (a_u[k][j_19][i_7][base3 +
               1] - a_u[k][j_19][i][base3 + 1]) + (2.0 * _t14 + a_lambda[k][
               j_19][i_6] * a_strx[i_7 + - ifirst] - 0.75 * (a_lambda[k][j_19][
               i_1] * a_strx[i + - ifirst] + a_lambda[k][j_19][i_8] * a_strx[
               i_9 + - ifirst])) * (a_u[k][j_19][i_9][base3 + 1] - a_u[k][j_19]
               [i][base3 + 1])) + a_stry[j_19 + - jfirst] * (_t19 * (a_u[k][
               j_21][i][base3 + 1] - a_u[k][j_19][i][base3 + 1]) + _t24 * (a_u[
               k][j_20][i][base3 + 1] - a_u[k][j_19][i][base3 + 1]) + _t28 * (
               a_u[k][j_22][i][base3 + 1] - a_u[k][j_19][i][base3 + 1]) + _t32 *
               (a_u[k][j_23][i][base3 + 1] - a_u[k][j_19][i][base3 + 1])) +
               a_strz[k + - kfirst] * (_t35 * (a_u[k_2][j_19][i][base3 + 1] -
               a_u[k][j_19][i][base3 + 1]) + _t40 * (a_u[k_1][j_19][i][base3 + 1
               ] - a_u[k][j_19][i][base3 + 1]) + _t45 * (a_u[k_3][j_19][i][
               base3 + 1] - a_u[k][j_19][i][base3 + 1]) + _t48 * (a_u[k_4][j_19
               ][i][base3 + 1] - a_u[k][j_19][i][base3 + 1]))) + a_strx[i + -
               ifirst] * a_stry[j_19 + - jfirst] * 0.006944444444444444 * (
               a_lambda[k][j_19][i_4] * (a_u[k][j_21][i_5][base3 + 2] - a_u[k][
               j_23][i_5][base3 + 2] + 8.0 * (- a_u[k][j_20][i_5][base3 + 2] +
               a_u[k][j_22][i_5][base3 + 2])) - 8.0 * (a_lambda[k][j_19][i_2] *
               (a_u[k][j_21][i_3][base3 + 2] - a_u[k][j_23][i_3][base3 + 2] +
               8.0 * (- a_u[k][j_20][i_3][base3 + 2] + a_u[k][j_22][i_3][base3 +
               2]))) + 8.0 * (a_lambda[k][j_19][i_6] * (a_u[k][j_21][i_7][base3
               + 2] - a_u[k][j_23][i_7][base3 + 2] + 8.0 * (- a_u[k][j_20][i_7]
               [base3 + 2] + a_u[k][j_22][i_7][base3 + 2]))) - a_lambda[k][j_19
               ][i_8] * (a_u[k][j_21][i_9][base3 + 2] - a_u[k][j_23][i_9][base3
               + 2] + 8.0 * (- a_u[k][j_20][i_9][base3 + 2] + a_u[k][j_22][i_9]
               [base3 + 2]))) + a_strx[i + - ifirst] * a_strz[k + - kfirst] *
               0.006944444444444444 * (a_lambda[k][j_19][i_4] * (a_u[k_2][j_19]
               [i_5][base3 + 3] - a_u[k_4][j_19][i_5][base3 + 3] + 8.0 * (- a_u
               [k_1][j_19][i_5][base3 + 3] + a_u[k_3][j_19][i_5][base3 + 3])) -
               8.0 * (a_lambda[k][j_19][i_2] * (a_u[k_2][j_19][i_3][base3 + 3] -
               a_u[k_4][j_19][i_3][base3 + 3] + 8.0 * (- a_u[k_1][j_19][i_3][
               base3 + 3] + a_u[k_3][j_19][i_3][base3 + 3]))) + 8.0 * (a_lambda
               [k][j_19][i_6] * (a_u[k_2][j_19][i_7][base3 + 3] - a_u[k_4][j_19
               ][i_7][base3 + 3] + 8.0 * (- a_u[k_1][j_19][i_7][base3 + 3] + a_u
               [k_3][j_19][i_7][base3 + 3]))) - a_lambda[k][j_19][i_8] * (a_u[
               k_2][j_19][i_9][base3 + 3] - a_u[k_4][j_19][i_9][base3 + 3] + 8.0
               * (- a_u[k_1][j_19][i_9][base3 + 3] + a_u[k_3][j_19][i_9][base3 +
               3]))) + a_strx[i + - ifirst] * a_stry[j_19 + - jfirst] *
               0.006944444444444444 * (a_mu[k][j_21][i_1] * (a_u[k][j_21][i_5][
               base3 + 2] - a_u[k][j_21][i_9][base3 + 2] + 8.0 * (- a_u[k][j_21
               ][i_3][base3 + 2] + a_u[k][j_21][i_7][base3 + 2])) - 8.0 * (a_mu
               [k][j_20][i_1] * (a_u[k][j_20][i_5][base3 + 2] - a_u[k][j_20][
               i_9][base3 + 2] + 8.0 * (- a_u[k][j_20][i_3][base3 + 2] + a_u[k]
               [j_20][i_7][base3 + 2]))) + 8.0 * (a_mu[k][j_22][i_1] * (a_u[k][
               j_22][i_5][base3 + 2] - a_u[k][j_22][i_9][base3 + 2] + 8.0 * (-
               a_u[k][j_22][i_3][base3 + 2] + a_u[k][j_22][i_7][base3 + 2]))) -
               a_mu[k][j_23][i_1] * (a_u[k][j_23][i_5][base3 + 2] - a_u[k][j_23
               ][i_9][base3 + 2] + 8.0 * (- a_u[k][j_23][i_3][base3 + 2] + a_u[
               k][j_23][i_7][base3 + 2]))) + a_strx[i + - ifirst] * a_strz[k + -
                kfirst] * 0.006944444444444444 * (a_mu[k_2][j_19][i_1] * (a_u[
               k_2][j_19][i_5][base3 + 3] - a_u[k_2][j_19][i_9][base3 + 3] + 8.0
               * (- a_u[k_2][j_19][i_3][base3 + 3] + a_u[k_2][j_19][i_7][base3 +
               3])) - 8.0 * (a_mu[k_1][j_19][i_1] * (a_u[k_1][j_19][i_5][base3 +
               3] - a_u[k_1][j_19][i_9][base3 + 3] + 8.0 * (- a_u[k_1][j_19][
               i_3][base3 + 3] + a_u[k_1][j_19][i_7][base3 + 3]))) + 8.0 * (
               a_mu[k_3][j_19][i_1] * (a_u[k_3][j_19][i_5][base3 + 3] - a_u[k_3
               ][j_19][i_9][base3 + 3] + 8.0 * (- a_u[k_3][j_19][i_3][base3 + 3
               ] + a_u[k_3][j_19][i_7][base3 + 3]))) - a_mu[k_4][j_19][i_1] * (
               a_u[k_4][j_19][i_5][base3 + 3] - a_u[k_4][j_19][i_9][base3 + 3] +
               8.0 * (- a_u[k_4][j_19][i_3][base3 + 3] + a_u[k_4][j_19][i_7][
               base3 + 3]))));
            a_lu[k][j_19][i][base3 + 2] = 0.0 * a_lu[k][j_19][i][base3 + 2] +
               _t1 * (0.16666666666666666 * (_t84 * (_t3 * (_t86 - _t87) + _t7 *
               (_t88 - _t90) + _t10 * (_t91 - _t92) + _t14 * (_t95 - _t96)) +
               _t97 * ((2.0 * _t19 + _t99 * _t100 - 0.75 * (_t101 * _t102 +
               _t104 * _t105)) * (_t106 - _t109) + (2.0 * _t24 + _t110 * _t112 +
               _t114 * _t115 + 3.0 * (_t117 * _t118 + _t119 * _t120)) * (_t122 -
               _t124) + (2.0 * _t28 + _t126 * _t128 + _t129 * _t130 + 3.0 * (
               _t132 * _t133 + _t134 * _t135)) * (_t137 - _t139) + (2.0 * _t32 +
               _t141 * _t142 - 0.75 * (_t143 * _t144 + _t146 * _t147)) * (_t148
               - _t150)) + _t151 * (_t35 * (_t153 - _t154) + _t40 * (_t156 -
               _t158) + _t45 * (_t160 - _t161) + _t48 * (_t163 - _t164))) + _t77
               * _t9 * 0.006944444444444444 * (_t283 * (_t293 - _t281 + 8.0 * (
               - _t268 + _t255)) - 8.0 * (_t218 * (_t127 - _t79 + 8.0 * (- _t41
               + _t21))) + 8.0 * (_t2 * (_t5 - _t8 + 8.0 * (- _t12 + _t20))) -
               _t26 * (_t31 - _t38 + 8.0 * (- _t44 + _t49))) + _t54 * _t58 *
               0.006944444444444444 * (_t65 * (_t71 - _t75 + 8.0 * (- _t82 +
               _t93)) - 8.0 * (_t107 * (_t123 - _t138 + 8.0 * (- _t152 + _t165)
               )) + 8.0 * (_t181 * (_t195 - _t208 + 8.0 * (- _t222 + _t238))) -
               _t251 * (_t256 - _t260 + 8.0 * (- _t266 + _t272))) + _t275 *
               _t279 * 0.006944444444444444 * (_t286 * (_t291 - _t297 + 8.0 * (
               - _t288 + _t264)) - 8.0 * (_t16 * (_t69 - _t174 + 8.0 * (- _t270
               + _t60))) + 8.0 * (_t188 * (_t61 - _t295 + 8.0 * (- _t277 + _t262
               ))) - _t235 * (_t196 - _t157 + 8.0 * (- _t111 + _t80))) + _t64 *
               _t56 * 0.006944444444444444 * (_t36 * (_t17 - _t6 + 8.0 * (-
               _t52 + _t252)) - 8.0 * (_t285 * (_t67 - _t4 + 8.0 * (- _t179 +
               _t299))) + 8.0 * (_t298 * (_t296 - _t294 + 8.0 * (- _t292 + _t290
               ))) - _t289 * (_t287 - _t284 + 8.0 * (- _t282 + _t280))));
            a_lu[k][j_19][i][base3 + 3] = 0.0 * a_lu[k][j_19][i][base3 + 3] +
               _t1 * (0.16666666666666666 * (_t166 * (_t3 * (_t167 - _t169) +
               _t7 * (_t170 - _t172) + _t10 * (_t173 - _t175) + _t14 * (_t177 -
               _t178)) + _t180 * (_t19 * (_t183 - _t184) + _t24 * (_t185 - _t187
               ) + _t28 * (_t189 - _t190) + _t32 * (_t192 - _t193)) + _t194 * (
               (2.0 * _t35 + _t198 * _t199 - 0.75 * (_t200 * _t201 + _t203 *
               _t204)) * (_t205 - _t207) + (2.0 * _t40 + _t209 * _t210 + _t212 *
               _t213 + 3.0 * (_t214 * _t215 + _t217 * _t219)) * (_t221 - _t223)
               + (2.0 * _t45 + _t225 * _t226 + _t227 * _t228 + 3.0 * (_t230 *
               _t231 + _t232 * _t233)) * (_t236 - _t237) + (2.0 * _t48 + _t240 *
               _t241 - 0.75 * (_t242 * _t243 + _t245 * _t246)) * (_t247 - _t249
               ))) + _t278 * _t276 * 0.006944444444444444 * (_t274 * (_t273 -
               _t271 + 8.0 * (- _t269 + _t267)) - 8.0 * (_t265 * (_t263 - _t261
               + 8.0 * (- _t259 + _t258))) + 8.0 * (_t257 * (_t254 - _t253 + 8.0
               * (- _t250 + _t248))) - _t244 * (_t239 - _t234 + 8.0 * (- _t229 +
               _t224))) + _t220 * _t216 * 0.006944444444444444 * (_t211 * (
               _t206 - _t202 + 8.0 * (- _t197 + _t191)) - 8.0 * (_t186 * (_t182
               - _t176 + 8.0 * (- _t171 + _t168))) + 8.0 * (_t162 * (_t159 -
               _t155 + 8.0 * (- _t149 + _t145))) - _t140 * (_t136 - _t131 + 8.0
               * (- _t125 + _t121))) + _t116 * _t113 * 0.006944444444444444 * (
               _t108 * (_t103 - _t98 + 8.0 * (- _t94 + _t89)) - 8.0 * (_t85 * (
               _t83 - _t81 + 8.0 * (- _t78 + _t76))) + 8.0 * (_t74 * (_t73 -
               _t72 + 8.0 * (- _t70 + _t68))) - _t66 * (_t63 - _t62 + 8.0 * (-
               _t59 + _t57))) + _t55 * _t53 * 0.006944444444444444 * (_t51 * (
               _t50 - _t47 + 8.0 * (- _t46 + _t43)) - 8.0 * (_t42 * (_t39 - _t37
               + 8.0 * (- _t34 + _t33))) + 8.0 * (_t30 * (_t29 - _t27 + 8.0 * (
               - _t25 + _t23))) - _t22 * (_t18 - _t15 + 8.0 * (- _t13 + _t11)))
               );
         }
      }
   }
}

/*
 * ../src/rhs4sg.c:1647.6
 */
static void mapped_fn1_2(int ifirst, int ilast, int jfirst, int jlast, int
   kfirst, int klast, int nk, int* onesided_8, double (* a_acof)[8][6], double
   (* a_bope)[6], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[
   25][25][3], double (* a_mu)[25][25], double (* a_lambda)[25][25], double h,
   double* a_strx, double* a_stry, double* a_strz, int k1, int k2, int base,
   int base3)
{
   double _t1;
   int k;
   int k_1;
   int k_2;
   int k_3;
   int k_4;
   for (_t1 = 1.0 / (h * h),
        k_1 = k1 + -1,
        k_2 = k1 + -2,
        k_3 = k1 + 1,
        k_4 = k1 + 2,
        k = k1;
        k <= k2;
        k++,
        k_1++,
        k_2++,
        k_3++,
        k_4++) {
      int j_24;
      int j_25;
      int j_26;
      int j_27;
      int j_28;
      for (j_24 = jfirst + 2,
           j_28 = jfirst + 4,
           j_25 = jfirst + 1,
           j_27 = jfirst + 3,
           j_26 = jfirst + 0;
           j_24 <= jlast + -2;
           j_24++,
           j_25++,
           j_26++,
           j_27++,
           j_28++) {
         int i;
         int i_1;
         int i_2;
         int i_3;
         int i_4;
         int i_5;
         int i_6;
         int i_7;
         int i_8;
         int i_9;
         for (i = ifirst + 2,
              i_1 = i + base,
              i_2 = i_1 + -1,
              i_9 = ifirst + 4,
              i_3 = ifirst + 1,
              i_4 = i_1 + -2,
              i_7 = ifirst + 3,
              i_5 = ifirst + 0,
              i_6 = i_1 + 1,
              i_8 = i_1 + 2;
              i <= ilast + -2;
              i_9++,
              i++,
              i_8++,
              i_7++,
              i_6++,
              i_5++,
              i_4++,
              i_3++,
              i_2++,
              i_1++) {
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
            double _t233;
            double _t234;
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
            _t84 = a_strx[i + - ifirst];
            _t86 = a_u[k][j_24][i_5][base3 + 2];
            _t87 = a_u[k][j_24][i][base3 + 2];
            _t88 = a_u[k][j_24][i_3][base3 + 2];
            _t90 = a_u[k][j_24][i][base3 + 2];
            _t91 = a_u[k][j_24][i_7][base3 + 2];
            _t92 = a_u[k][j_24][i][base3 + 2];
            _t95 = a_u[k][j_24][i_9][base3 + 2];
            _t96 = a_u[k][j_24][i][base3 + 2];
            _t97 = a_stry[j_24 + - jfirst];
            _t99 = a_lambda[k][j_25][i_1];
            _t100 = a_stry[j_25 + - jfirst];
            _t101 = a_lambda[k][j_24][i_1];
            _t102 = a_stry[j_24 + - jfirst];
            _t104 = a_lambda[k][j_26][i_1];
            _t105 = a_stry[j_26 + - jfirst];
            _t106 = a_u[k][j_26][i][base3 + 2];
            _t109 = a_u[k][j_24][i][base3 + 2];
            _t110 = a_lambda[k][j_26][i_1];
            _t112 = a_stry[j_26 + - jfirst];
            _t114 = a_lambda[k][j_27][i_1];
            _t115 = a_stry[j_27 + - jfirst];
            _t117 = a_lambda[k][j_24][i_1];
            _t118 = a_stry[j_24 + - jfirst];
            _t119 = a_lambda[k][j_25][i_1];
            _t120 = a_stry[j_25 + - jfirst];
            _t122 = a_u[k][j_25][i][base3 + 2];
            _t124 = a_u[k][j_24][i][base3 + 2];
            _t126 = a_lambda[k][j_25][i_1];
            _t128 = a_stry[j_25 + - jfirst];
            _t129 = a_lambda[k][j_28][i_1];
            _t130 = a_stry[j_28 + - jfirst];
            _t132 = a_lambda[k][j_27][i_1];
            _t133 = a_stry[j_27 + - jfirst];
            _t134 = a_lambda[k][j_24][i_1];
            _t135 = a_stry[j_24 + - jfirst];
            _t137 = a_u[k][j_27][i][base3 + 2];
            _t139 = a_u[k][j_24][i][base3 + 2];
            _t141 = a_lambda[k][j_27][i_1];
            _t142 = a_stry[j_27 + - jfirst];
            _t143 = a_lambda[k][j_24][i_1];
            _t144 = a_stry[j_24 + - jfirst];
            _t146 = a_lambda[k][j_28][i_1];
            _t147 = a_stry[j_28 + - jfirst];
            _t148 = a_u[k][j_28][i][base3 + 2];
            _t150 = a_u[k][j_24][i][base3 + 2];
            _t151 = a_strz[k + - kfirst];
            _t153 = a_u[k_2][j_24][i][base3 + 2];
            _t154 = a_u[k][j_24][i][base3 + 2];
            _t156 = a_u[k_1][j_24][i][base3 + 2];
            _t158 = a_u[k][j_24][i][base3 + 2];
            _t160 = a_u[k_3][j_24][i][base3 + 2];
            _t161 = a_u[k][j_24][i][base3 + 2];
            _t163 = a_u[k_4][j_24][i][base3 + 2];
            _t164 = a_u[k][j_24][i][base3 + 2];
            _t166 = a_strx[i + - ifirst];
            _t167 = a_u[k][j_24][i_5][base3 + 3];
            _t169 = a_u[k][j_24][i][base3 + 3];
            _t170 = a_u[k][j_24][i_3][base3 + 3];
            _t172 = a_u[k][j_24][i][base3 + 3];
            _t173 = a_u[k][j_24][i_7][base3 + 3];
            _t175 = a_u[k][j_24][i][base3 + 3];
            _t177 = a_u[k][j_24][i_9][base3 + 3];
            _t178 = a_u[k][j_24][i][base3 + 3];
            _t180 = a_stry[j_24 + - jfirst];
            _t183 = a_u[k][j_26][i][base3 + 3];
            _t184 = a_u[k][j_24][i][base3 + 3];
            _t185 = a_u[k][j_25][i][base3 + 3];
            _t187 = a_u[k][j_24][i][base3 + 3];
            _t189 = a_u[k][j_27][i][base3 + 3];
            _t190 = a_u[k][j_24][i][base3 + 3];
            _t192 = a_u[k][j_28][i][base3 + 3];
            _t193 = a_u[k][j_24][i][base3 + 3];
            _t194 = a_strz[k + - kfirst];
            _t198 = a_lambda[k_1][j_24][i_1];
            _t199 = a_strz[k_1 + - kfirst];
            _t200 = a_lambda[k][j_24][i_1];
            _t201 = a_strz[k + - kfirst];
            _t203 = a_lambda[k_2][j_24][i_1];
            _t204 = a_strz[k_2 + - kfirst];
            _t205 = a_u[k_2][j_24][i][base3 + 3];
            _t207 = a_u[k][j_24][i][base3 + 3];
            _t209 = a_lambda[k_2][j_24][i_1];
            _t210 = a_strz[k_2 + - kfirst];
            _t212 = a_lambda[k_3][j_24][i_1];
            _t213 = a_strz[k_3 + - kfirst];
            _t214 = a_lambda[k][j_24][i_1];
            _t215 = a_strz[k + - kfirst];
            _t217 = a_lambda[k_1][j_24][i_1];
            _t219 = a_strz[k_1 + - kfirst];
            _t221 = a_u[k_1][j_24][i][base3 + 3];
            _t223 = a_u[k][j_24][i][base3 + 3];
            _t225 = a_lambda[k_1][j_24][i_1];
            _t226 = a_strz[k_1 + - kfirst];
            _t227 = a_lambda[k_4][j_24][i_1];
            _t228 = a_strz[k_4 + - kfirst];
            _t230 = a_lambda[k_3][j_24][i_1];
            _t231 = a_strz[k_3 + - kfirst];
            _t232 = a_lambda[k][j_24][i_1];
            _t233 = a_strz[k + - kfirst];
            _t236 = a_u[k_3][j_24][i][base3 + 3];
            _t237 = a_u[k][j_24][i][base3 + 3];
            _t240 = a_lambda[k_3][j_24][i_1];
            _t241 = a_strz[k_3 + - kfirst];
            _t242 = a_lambda[k][j_24][i_1];
            _t243 = a_strz[k + - kfirst];
            _t245 = a_lambda[k_4][j_24][i_1];
            _t246 = a_strz[k_4 + - kfirst];
            _t247 = a_u[k_4][j_24][i][base3 + 3];
            _t249 = a_u[k][j_24][i][base3 + 3];
            _t77 = a_strx[i + - ifirst];
            _t9 = a_stry[j_24 + - jfirst];
            _t283 = a_mu[k][j_24][i_4];
            _t293 = a_u[k][j_26][i_5][base3 + 1];
            _t281 = a_u[k][j_28][i_5][base3 + 1];
            _t268 = a_u[k][j_25][i_5][base3 + 1];
            _t255 = a_u[k][j_27][i_5][base3 + 1];
            _t218 = a_mu[k][j_24][i_2];
            _t127 = a_u[k][j_26][i_3][base3 + 1];
            _t79 = a_u[k][j_28][i_3][base3 + 1];
            _t41 = a_u[k][j_25][i_3][base3 + 1];
            _t21 = a_u[k][j_27][i_3][base3 + 1];
            _t2 = a_mu[k][j_24][i_6];
            _t5 = a_u[k][j_26][i_7][base3 + 1];
            _t8 = a_u[k][j_28][i_7][base3 + 1];
            _t12 = a_u[k][j_25][i_7][base3 + 1];
            _t20 = a_u[k][j_27][i_7][base3 + 1];
            _t26 = a_mu[k][j_24][i_8];
            _t31 = a_u[k][j_26][i_9][base3 + 1];
            _t38 = a_u[k][j_28][i_9][base3 + 1];
            _t44 = a_u[k][j_25][i_9][base3 + 1];
            _t49 = a_u[k][j_27][i_9][base3 + 1];
            _t54 = a_strx[i + - ifirst];
            _t58 = a_stry[j_24 + - jfirst];
            _t65 = a_lambda[k][j_26][i_1];
            _t71 = a_u[k][j_26][i_5][base3 + 1];
            _t75 = a_u[k][j_26][i_9][base3 + 1];
            _t82 = a_u[k][j_26][i_3][base3 + 1];
            _t93 = a_u[k][j_26][i_7][base3 + 1];
            _t107 = a_lambda[k][j_25][i_1];
            _t123 = a_u[k][j_25][i_5][base3 + 1];
            _t138 = a_u[k][j_25][i_9][base3 + 1];
            _t152 = a_u[k][j_25][i_3][base3 + 1];
            _t165 = a_u[k][j_25][i_7][base3 + 1];
            _t181 = a_lambda[k][j_27][i_1];
            _t195 = a_u[k][j_27][i_5][base3 + 1];
            _t208 = a_u[k][j_27][i_9][base3 + 1];
            _t222 = a_u[k][j_27][i_3][base3 + 1];
            _t238 = a_u[k][j_27][i_7][base3 + 1];
            _t251 = a_lambda[k][j_28][i_1];
            _t256 = a_u[k][j_28][i_5][base3 + 1];
            _t260 = a_u[k][j_28][i_9][base3 + 1];
            _t266 = a_u[k][j_28][i_3][base3 + 1];
            _t272 = a_u[k][j_28][i_7][base3 + 1];
            _t275 = a_stry[j_24 + - jfirst];
            _t279 = a_strz[k + - kfirst];
            _t286 = a_lambda[k][j_26][i_1];
            _t291 = a_u[k_2][j_26][i][base3 + 3];
            _t297 = a_u[k_4][j_26][i][base3 + 3];
            _t288 = a_u[k_1][j_26][i][base3 + 3];
            _t264 = a_u[k_3][j_26][i][base3 + 3];
            _t16 = a_lambda[k][j_25][i_1];
            _t69 = a_u[k_2][j_25][i][base3 + 3];
            _t174 = a_u[k_4][j_25][i][base3 + 3];
            _t270 = a_u[k_1][j_25][i][base3 + 3];
            _t60 = a_u[k_3][j_25][i][base3 + 3];
            _t188 = a_lambda[k][j_27][i_1];
            _t61 = a_u[k_2][j_27][i][base3 + 3];
            _t295 = a_u[k_4][j_27][i][base3 + 3];
            _t277 = a_u[k_1][j_27][i][base3 + 3];
            _t262 = a_u[k_3][j_27][i][base3 + 3];
            _t235 = a_lambda[k][j_28][i_1];
            _t196 = a_u[k_2][j_28][i][base3 + 3];
            _t157 = a_u[k_4][j_28][i][base3 + 3];
            _t111 = a_u[k_1][j_28][i][base3 + 3];
            _t80 = a_u[k_3][j_28][i][base3 + 3];
            _t64 = a_stry[j_24 + - jfirst];
            _t56 = a_strz[k + - kfirst];
            _t36 = a_mu[k_2][j_24][i_1];
            _t17 = a_u[k_2][j_26][i][base3 + 3];
            _t6 = a_u[k_2][j_28][i][base3 + 3];
            _t52 = a_u[k_2][j_25][i][base3 + 3];
            _t252 = a_u[k_2][j_27][i][base3 + 3];
            _t285 = a_mu[k_1][j_24][i_1];
            _t67 = a_u[k_1][j_26][i][base3 + 3];
            _t4 = a_u[k_1][j_28][i][base3 + 3];
            _t179 = a_u[k_1][j_25][i][base3 + 3];
            _t299 = a_u[k_1][j_27][i][base3 + 3];
            _t298 = a_mu[k_3][j_24][i_1];
            _t296 = a_u[k_3][j_26][i][base3 + 3];
            _t294 = a_u[k_3][j_28][i][base3 + 3];
            _t292 = a_u[k_3][j_25][i][base3 + 3];
            _t290 = a_u[k_3][j_27][i][base3 + 3];
            _t289 = a_mu[k_4][j_24][i_1];
            _t287 = a_u[k_4][j_26][i][base3 + 3];
            _t284 = a_u[k_4][j_28][i][base3 + 3];
            _t282 = a_u[k_4][j_25][i][base3 + 3];
            _t280 = a_u[k_4][j_27][i][base3 + 3];
            _t278 = a_strx[i + - ifirst];
            _t276 = a_strz[k + - kfirst];
            _t274 = a_mu[k][j_24][i_4];
            _t273 = a_u[k_2][j_24][i_5][base3 + 1];
            _t271 = a_u[k_4][j_24][i_5][base3 + 1];
            _t269 = a_u[k_1][j_24][i_5][base3 + 1];
            _t267 = a_u[k_3][j_24][i_5][base3 + 1];
            _t265 = a_mu[k][j_24][i_2];
            _t263 = a_u[k_2][j_24][i_3][base3 + 1];
            _t261 = a_u[k_4][j_24][i_3][base3 + 1];
            _t259 = a_u[k_1][j_24][i_3][base3 + 1];
            _t258 = a_u[k_3][j_24][i_3][base3 + 1];
            _t257 = a_mu[k][j_24][i_6];
            _t254 = a_u[k_2][j_24][i_7][base3 + 1];
            _t253 = a_u[k_4][j_24][i_7][base3 + 1];
            _t250 = a_u[k_1][j_24][i_7][base3 + 1];
            _t248 = a_u[k_3][j_24][i_7][base3 + 1];
            _t244 = a_mu[k][j_24][i_8];
            _t239 = a_u[k_2][j_24][i_9][base3 + 1];
            _t234 = a_u[k_4][j_24][i_9][base3 + 1];
            _t229 = a_u[k_1][j_24][i_9][base3 + 1];
            _t224 = a_u[k_3][j_24][i_9][base3 + 1];
            _t220 = a_stry[j_24 + - jfirst];
            _t216 = a_strz[k + - kfirst];
            _t211 = a_mu[k][j_26][i_1];
            _t206 = a_u[k_2][j_26][i][base3 + 2];
            _t202 = a_u[k_4][j_26][i][base3 + 2];
            _t197 = a_u[k_1][j_26][i][base3 + 2];
            _t191 = a_u[k_3][j_26][i][base3 + 2];
            _t186 = a_mu[k][j_25][i_1];
            _t182 = a_u[k_2][j_25][i][base3 + 2];
            _t176 = a_u[k_4][j_25][i][base3 + 2];
            _t171 = a_u[k_1][j_25][i][base3 + 2];
            _t168 = a_u[k_3][j_25][i][base3 + 2];
            _t162 = a_mu[k][j_27][i_1];
            _t159 = a_u[k_2][j_27][i][base3 + 2];
            _t155 = a_u[k_4][j_27][i][base3 + 2];
            _t149 = a_u[k_1][j_27][i][base3 + 2];
            _t145 = a_u[k_3][j_27][i][base3 + 2];
            _t140 = a_mu[k][j_28][i_1];
            _t136 = a_u[k_2][j_28][i][base3 + 2];
            _t131 = a_u[k_4][j_28][i][base3 + 2];
            _t125 = a_u[k_1][j_28][i][base3 + 2];
            _t121 = a_u[k_3][j_28][i][base3 + 2];
            _t116 = a_strx[i + - ifirst];
            _t113 = a_strz[k + - kfirst];
            _t108 = a_lambda[k_2][j_24][i_1];
            _t103 = a_u[k_2][j_24][i_5][base3 + 1];
            _t98 = a_u[k_2][j_24][i_9][base3 + 1];
            _t94 = a_u[k_2][j_24][i_3][base3 + 1];
            _t89 = a_u[k_2][j_24][i_7][base3 + 1];
            _t85 = a_lambda[k_1][j_24][i_1];
            _t83 = a_u[k_1][j_24][i_5][base3 + 1];
            _t81 = a_u[k_1][j_24][i_9][base3 + 1];
            _t78 = a_u[k_1][j_24][i_3][base3 + 1];
            _t76 = a_u[k_1][j_24][i_7][base3 + 1];
            _t74 = a_lambda[k_3][j_24][i_1];
            _t73 = a_u[k_3][j_24][i_5][base3 + 1];
            _t72 = a_u[k_3][j_24][i_9][base3 + 1];
            _t70 = a_u[k_3][j_24][i_3][base3 + 1];
            _t68 = a_u[k_3][j_24][i_7][base3 + 1];
            _t66 = a_lambda[k_4][j_24][i_1];
            _t63 = a_u[k_4][j_24][i_5][base3 + 1];
            _t62 = a_u[k_4][j_24][i_9][base3 + 1];
            _t59 = a_u[k_4][j_24][i_3][base3 + 1];
            _t57 = a_u[k_4][j_24][i_7][base3 + 1];
            _t55 = a_stry[j_24 + - jfirst];
            _t53 = a_strz[k + - kfirst];
            _t51 = a_lambda[k_2][j_24][i_1];
            _t50 = a_u[k_2][j_26][i][base3 + 2];
            _t47 = a_u[k_2][j_28][i][base3 + 2];
            _t46 = a_u[k_2][j_25][i][base3 + 2];
            _t43 = a_u[k_2][j_27][i][base3 + 2];
            _t42 = a_lambda[k_1][j_24][i_1];
            _t39 = a_u[k_1][j_26][i][base3 + 2];
            _t37 = a_u[k_1][j_28][i][base3 + 2];
            _t34 = a_u[k_1][j_25][i][base3 + 2];
            _t33 = a_u[k_1][j_27][i][base3 + 2];
            _t30 = a_lambda[k_3][j_24][i_1];
            _t29 = a_u[k_3][j_26][i][base3 + 2];
            _t27 = a_u[k_3][j_28][i][base3 + 2];
            _t25 = a_u[k_3][j_25][i][base3 + 2];
            _t23 = a_u[k_3][j_27][i][base3 + 2];
            _t22 = a_lambda[k_4][j_24][i_1];
            _t18 = a_u[k_4][j_26][i][base3 + 2];
            _t15 = a_u[k_4][j_28][i][base3 + 2];
            _t13 = a_u[k_4][j_25][i][base3 + 2];
            _t11 = a_u[k_4][j_27][i][base3 + 2];
            _t48 = a_mu[k_3][j_24][i_1] * a_strz[k_3 + - kfirst] - 0.75 * (a_mu
               [k][j_24][i_1] * a_strz[k + - kfirst] + a_mu[k_4][j_24][i_1] *
               a_strz[k_4 + - kfirst]);
            _t45 = a_mu[k_1][j_24][i_1] * a_strz[k_1 + - kfirst] + a_mu[k_4][
               j_24][i_1] * a_strz[k_4 + - kfirst] + 3.0 * (a_mu[k_3][j_24][i_1
               ] * a_strz[k_3 + - kfirst] + a_mu[k][j_24][i_1] * a_strz[k + -
               kfirst]);
            _t40 = a_mu[k_2][j_24][i_1] * a_strz[k_2 + - kfirst] + a_mu[k_3][
               j_24][i_1] * a_strz[k_3 + - kfirst] + 3.0 * (a_mu[k][j_24][i_1] *
               a_strz[k + - kfirst] + a_mu[k_1][j_24][i_1] * a_strz[k_1 + -
               kfirst]);
            _t35 = a_mu[k_1][j_24][i_1] * a_strz[k_1 + - kfirst] - 0.75 * (a_mu
               [k][j_24][i_1] * a_strz[k + - kfirst] + a_mu[k_2][j_24][i_1] *
               a_strz[k_2 + - kfirst]);
            _t32 = a_mu[k][j_27][i_1] * a_stry[j_27 + - jfirst] - 0.75 * (a_mu[
               k][j_24][i_1] * a_stry[j_24 + - jfirst] + a_mu[k][j_28][i_1] *
               a_stry[j_28 + - jfirst]);
            _t28 = a_mu[k][j_25][i_1] * a_stry[j_25 + - jfirst] + a_mu[k][j_28]
               [i_1] * a_stry[j_28 + - jfirst] + 3.0 * (a_mu[k][j_27][i_1] *
               a_stry[j_27 + - jfirst] + a_mu[k][j_24][i_1] * a_stry[j_24 + -
               jfirst]);
            _t24 = a_mu[k][j_26][i_1] * a_stry[j_26 + - jfirst] + a_mu[k][j_27]
               [i_1] * a_stry[j_27 + - jfirst] + 3.0 * (a_mu[k][j_24][i_1] *
               a_stry[j_24 + - jfirst] + a_mu[k][j_25][i_1] * a_stry[j_25 + -
               jfirst]);
            _t19 = a_mu[k][j_25][i_1] * a_stry[j_25 + - jfirst] - 0.75 * (a_mu[
               k][j_24][i_1] * a_stry[j_24 + - jfirst] + a_mu[k][j_26][i_1] *
               a_stry[j_26 + - jfirst]);
            _t14 = a_mu[k][j_24][i_6] * a_strx[i_7 + - ifirst] - 0.75 * (a_mu[k
               ][j_24][i_1] * a_strx[i + - ifirst] + a_mu[k][j_24][i_8] * a_strx
               [i_9 + - ifirst]);
            _t10 = a_mu[k][j_24][i_2] * a_strx[i_3 + - ifirst] + a_mu[k][j_24][
               i_8] * a_strx[i_9 + - ifirst] + 3.0 * (a_mu[k][j_24][i_6] *
               a_strx[i_7 + - ifirst] + a_mu[k][j_24][i_1] * a_strx[i + -
               ifirst]);
            _t7 = a_mu[k][j_24][i_4] * a_strx[i_5 + - ifirst] + a_mu[k][j_24][
               i_6] * a_strx[i_7 + - ifirst] + 3.0 * (a_mu[k][j_24][i_1] *
               a_strx[i + - ifirst] + a_mu[k][j_24][i_2] * a_strx[i_3 + -
               ifirst]);
            _t3 = a_mu[k][j_24][i_2] * a_strx[i_3 + - ifirst] - 0.75 * (a_mu[k]
               [j_24][i_1] * a_strx[i + - ifirst] + a_mu[k][j_24][i_4] * a_strx
               [i_5 + - ifirst]);
            a_lu[k][j_24][i][base3 + 1] = 0.0 * a_lu[k][j_24][i][base3 + 1] +
               _t1 * (0.16666666666666666 * (a_strx[i + - ifirst] * ((2.0 * _t3
               + a_lambda[k][j_24][i_2] * a_strx[i_3 + - ifirst] - 0.75 * (
               a_lambda[k][j_24][i_1] * a_strx[i + - ifirst] + a_lambda[k][j_24
               ][i_4] * a_strx[i_5 + - ifirst])) * (a_u[k][j_24][i_5][base3 + 1
               ] - a_u[k][j_24][i][base3 + 1]) + (2.0 * _t7 + a_lambda[k][j_24]
               [i_4] * a_strx[i_5 + - ifirst] + a_lambda[k][j_24][i_6] * a_strx
               [i_7 + - ifirst] + 3.0 * (a_lambda[k][j_24][i_1] * a_strx[i + -
               ifirst] + a_lambda[k][j_24][i_2] * a_strx[i_3 + - ifirst])) * (
               a_u[k][j_24][i_3][base3 + 1] - a_u[k][j_24][i][base3 + 1]) + (
               2.0 * _t10 + a_lambda[k][j_24][i_2] * a_strx[i_3 + - ifirst] +
               a_lambda[k][j_24][i_8] * a_strx[i_9 + - ifirst] + 3.0 * (
               a_lambda[k][j_24][i_6] * a_strx[i_7 + - ifirst] + a_lambda[k][
               j_24][i_1] * a_strx[i + - ifirst])) * (a_u[k][j_24][i_7][base3 +
               1] - a_u[k][j_24][i][base3 + 1]) + (2.0 * _t14 + a_lambda[k][
               j_24][i_6] * a_strx[i_7 + - ifirst] - 0.75 * (a_lambda[k][j_24][
               i_1] * a_strx[i + - ifirst] + a_lambda[k][j_24][i_8] * a_strx[
               i_9 + - ifirst])) * (a_u[k][j_24][i_9][base3 + 1] - a_u[k][j_24]
               [i][base3 + 1])) + a_stry[j_24 + - jfirst] * (_t19 * (a_u[k][
               j_26][i][base3 + 1] - a_u[k][j_24][i][base3 + 1]) + _t24 * (a_u[
               k][j_25][i][base3 + 1] - a_u[k][j_24][i][base3 + 1]) + _t28 * (
               a_u[k][j_27][i][base3 + 1] - a_u[k][j_24][i][base3 + 1]) + _t32 *
               (a_u[k][j_28][i][base3 + 1] - a_u[k][j_24][i][base3 + 1])) +
               a_strz[k + - kfirst] * (_t35 * (a_u[k_2][j_24][i][base3 + 1] -
               a_u[k][j_24][i][base3 + 1]) + _t40 * (a_u[k_1][j_24][i][base3 + 1
               ] - a_u[k][j_24][i][base3 + 1]) + _t45 * (a_u[k_3][j_24][i][
               base3 + 1] - a_u[k][j_24][i][base3 + 1]) + _t48 * (a_u[k_4][j_24
               ][i][base3 + 1] - a_u[k][j_24][i][base3 + 1]))) + a_strx[i + -
               ifirst] * a_stry[j_24 + - jfirst] * 0.006944444444444444 * (
               a_lambda[k][j_24][i_4] * (a_u[k][j_26][i_5][base3 + 2] - a_u[k][
               j_28][i_5][base3 + 2] + 8.0 * (- a_u[k][j_25][i_5][base3 + 2] +
               a_u[k][j_27][i_5][base3 + 2])) - 8.0 * (a_lambda[k][j_24][i_2] *
               (a_u[k][j_26][i_3][base3 + 2] - a_u[k][j_28][i_3][base3 + 2] +
               8.0 * (- a_u[k][j_25][i_3][base3 + 2] + a_u[k][j_27][i_3][base3 +
               2]))) + 8.0 * (a_lambda[k][j_24][i_6] * (a_u[k][j_26][i_7][base3
               + 2] - a_u[k][j_28][i_7][base3 + 2] + 8.0 * (- a_u[k][j_25][i_7]
               [base3 + 2] + a_u[k][j_27][i_7][base3 + 2]))) - a_lambda[k][j_24
               ][i_8] * (a_u[k][j_26][i_9][base3 + 2] - a_u[k][j_28][i_9][base3
               + 2] + 8.0 * (- a_u[k][j_25][i_9][base3 + 2] + a_u[k][j_27][i_9]
               [base3 + 2]))) + a_strx[i + - ifirst] * a_strz[k + - kfirst] *
               0.006944444444444444 * (a_lambda[k][j_24][i_4] * (a_u[k_2][j_24]
               [i_5][base3 + 3] - a_u[k_4][j_24][i_5][base3 + 3] + 8.0 * (- a_u
               [k_1][j_24][i_5][base3 + 3] + a_u[k_3][j_24][i_5][base3 + 3])) -
               8.0 * (a_lambda[k][j_24][i_2] * (a_u[k_2][j_24][i_3][base3 + 3] -
               a_u[k_4][j_24][i_3][base3 + 3] + 8.0 * (- a_u[k_1][j_24][i_3][
               base3 + 3] + a_u[k_3][j_24][i_3][base3 + 3]))) + 8.0 * (a_lambda
               [k][j_24][i_6] * (a_u[k_2][j_24][i_7][base3 + 3] - a_u[k_4][j_24
               ][i_7][base3 + 3] + 8.0 * (- a_u[k_1][j_24][i_7][base3 + 3] + a_u
               [k_3][j_24][i_7][base3 + 3]))) - a_lambda[k][j_24][i_8] * (a_u[
               k_2][j_24][i_9][base3 + 3] - a_u[k_4][j_24][i_9][base3 + 3] + 8.0
               * (- a_u[k_1][j_24][i_9][base3 + 3] + a_u[k_3][j_24][i_9][base3 +
               3]))) + a_strx[i + - ifirst] * a_stry[j_24 + - jfirst] *
               0.006944444444444444 * (a_mu[k][j_26][i_1] * (a_u[k][j_26][i_5][
               base3 + 2] - a_u[k][j_26][i_9][base3 + 2] + 8.0 * (- a_u[k][j_26
               ][i_3][base3 + 2] + a_u[k][j_26][i_7][base3 + 2])) - 8.0 * (a_mu
               [k][j_25][i_1] * (a_u[k][j_25][i_5][base3 + 2] - a_u[k][j_25][
               i_9][base3 + 2] + 8.0 * (- a_u[k][j_25][i_3][base3 + 2] + a_u[k]
               [j_25][i_7][base3 + 2]))) + 8.0 * (a_mu[k][j_27][i_1] * (a_u[k][
               j_27][i_5][base3 + 2] - a_u[k][j_27][i_9][base3 + 2] + 8.0 * (-
               a_u[k][j_27][i_3][base3 + 2] + a_u[k][j_27][i_7][base3 + 2]))) -
               a_mu[k][j_28][i_1] * (a_u[k][j_28][i_5][base3 + 2] - a_u[k][j_28
               ][i_9][base3 + 2] + 8.0 * (- a_u[k][j_28][i_3][base3 + 2] + a_u[
               k][j_28][i_7][base3 + 2]))) + a_strx[i + - ifirst] * a_strz[k + -
                kfirst] * 0.006944444444444444 * (a_mu[k_2][j_24][i_1] * (a_u[
               k_2][j_24][i_5][base3 + 3] - a_u[k_2][j_24][i_9][base3 + 3] + 8.0
               * (- a_u[k_2][j_24][i_3][base3 + 3] + a_u[k_2][j_24][i_7][base3 +
               3])) - 8.0 * (a_mu[k_1][j_24][i_1] * (a_u[k_1][j_24][i_5][base3 +
               3] - a_u[k_1][j_24][i_9][base3 + 3] + 8.0 * (- a_u[k_1][j_24][
               i_3][base3 + 3] + a_u[k_1][j_24][i_7][base3 + 3]))) + 8.0 * (
               a_mu[k_3][j_24][i_1] * (a_u[k_3][j_24][i_5][base3 + 3] - a_u[k_3
               ][j_24][i_9][base3 + 3] + 8.0 * (- a_u[k_3][j_24][i_3][base3 + 3
               ] + a_u[k_3][j_24][i_7][base3 + 3]))) - a_mu[k_4][j_24][i_1] * (
               a_u[k_4][j_24][i_5][base3 + 3] - a_u[k_4][j_24][i_9][base3 + 3] +
               8.0 * (- a_u[k_4][j_24][i_3][base3 + 3] + a_u[k_4][j_24][i_7][
               base3 + 3]))));
            a_lu[k][j_24][i][base3 + 2] = 0.0 * a_lu[k][j_24][i][base3 + 2] +
               _t1 * (0.16666666666666666 * (_t84 * (_t3 * (_t86 - _t87) + _t7 *
               (_t88 - _t90) + _t10 * (_t91 - _t92) + _t14 * (_t95 - _t96)) +
               _t97 * ((2.0 * _t19 + _t99 * _t100 - 0.75 * (_t101 * _t102 +
               _t104 * _t105)) * (_t106 - _t109) + (2.0 * _t24 + _t110 * _t112 +
               _t114 * _t115 + 3.0 * (_t117 * _t118 + _t119 * _t120)) * (_t122 -
               _t124) + (2.0 * _t28 + _t126 * _t128 + _t129 * _t130 + 3.0 * (
               _t132 * _t133 + _t134 * _t135)) * (_t137 - _t139) + (2.0 * _t32 +
               _t141 * _t142 - 0.75 * (_t143 * _t144 + _t146 * _t147)) * (_t148
               - _t150)) + _t151 * (_t35 * (_t153 - _t154) + _t40 * (_t156 -
               _t158) + _t45 * (_t160 - _t161) + _t48 * (_t163 - _t164))) + _t77
               * _t9 * 0.006944444444444444 * (_t283 * (_t293 - _t281 + 8.0 * (
               - _t268 + _t255)) - 8.0 * (_t218 * (_t127 - _t79 + 8.0 * (- _t41
               + _t21))) + 8.0 * (_t2 * (_t5 - _t8 + 8.0 * (- _t12 + _t20))) -
               _t26 * (_t31 - _t38 + 8.0 * (- _t44 + _t49))) + _t54 * _t58 *
               0.006944444444444444 * (_t65 * (_t71 - _t75 + 8.0 * (- _t82 +
               _t93)) - 8.0 * (_t107 * (_t123 - _t138 + 8.0 * (- _t152 + _t165)
               )) + 8.0 * (_t181 * (_t195 - _t208 + 8.0 * (- _t222 + _t238))) -
               _t251 * (_t256 - _t260 + 8.0 * (- _t266 + _t272))) + _t275 *
               _t279 * 0.006944444444444444 * (_t286 * (_t291 - _t297 + 8.0 * (
               - _t288 + _t264)) - 8.0 * (_t16 * (_t69 - _t174 + 8.0 * (- _t270
               + _t60))) + 8.0 * (_t188 * (_t61 - _t295 + 8.0 * (- _t277 + _t262
               ))) - _t235 * (_t196 - _t157 + 8.0 * (- _t111 + _t80))) + _t64 *
               _t56 * 0.006944444444444444 * (_t36 * (_t17 - _t6 + 8.0 * (-
               _t52 + _t252)) - 8.0 * (_t285 * (_t67 - _t4 + 8.0 * (- _t179 +
               _t299))) + 8.0 * (_t298 * (_t296 - _t294 + 8.0 * (- _t292 + _t290
               ))) - _t289 * (_t287 - _t284 + 8.0 * (- _t282 + _t280))));
            a_lu[k][j_24][i][base3 + 3] = 0.0 * a_lu[k][j_24][i][base3 + 3] +
               _t1 * (0.16666666666666666 * (_t166 * (_t3 * (_t167 - _t169) +
               _t7 * (_t170 - _t172) + _t10 * (_t173 - _t175) + _t14 * (_t177 -
               _t178)) + _t180 * (_t19 * (_t183 - _t184) + _t24 * (_t185 - _t187
               ) + _t28 * (_t189 - _t190) + _t32 * (_t192 - _t193)) + _t194 * (
               (2.0 * _t35 + _t198 * _t199 - 0.75 * (_t200 * _t201 + _t203 *
               _t204)) * (_t205 - _t207) + (2.0 * _t40 + _t209 * _t210 + _t212 *
               _t213 + 3.0 * (_t214 * _t215 + _t217 * _t219)) * (_t221 - _t223)
               + (2.0 * _t45 + _t225 * _t226 + _t227 * _t228 + 3.0 * (_t230 *
               _t231 + _t232 * _t233)) * (_t236 - _t237) + (2.0 * _t48 + _t240 *
               _t241 - 0.75 * (_t242 * _t243 + _t245 * _t246)) * (_t247 - _t249
               ))) + _t278 * _t276 * 0.006944444444444444 * (_t274 * (_t273 -
               _t271 + 8.0 * (- _t269 + _t267)) - 8.0 * (_t265 * (_t263 - _t261
               + 8.0 * (- _t259 + _t258))) + 8.0 * (_t257 * (_t254 - _t253 + 8.0
               * (- _t250 + _t248))) - _t244 * (_t239 - _t234 + 8.0 * (- _t229 +
               _t224))) + _t220 * _t216 * 0.006944444444444444 * (_t211 * (
               _t206 - _t202 + 8.0 * (- _t197 + _t191)) - 8.0 * (_t186 * (_t182
               - _t176 + 8.0 * (- _t171 + _t168))) + 8.0 * (_t162 * (_t159 -
               _t155 + 8.0 * (- _t149 + _t145))) - _t140 * (_t136 - _t131 + 8.0
               * (- _t125 + _t121))) + _t116 * _t113 * 0.006944444444444444 * (
               _t108 * (_t103 - _t98 + 8.0 * (- _t94 + _t89)) - 8.0 * (_t85 * (
               _t83 - _t81 + 8.0 * (- _t78 + _t76))) + 8.0 * (_t74 * (_t73 -
               _t72 + 8.0 * (- _t70 + _t68))) - _t66 * (_t63 - _t62 + 8.0 * (-
               _t59 + _t57))) + _t55 * _t53 * 0.006944444444444444 * (_t51 * (
               _t50 - _t47 + 8.0 * (- _t46 + _t43)) - 8.0 * (_t42 * (_t39 - _t37
               + 8.0 * (- _t34 + _t33))) + 8.0 * (_t30 * (_t29 - _t27 + 8.0 * (
               - _t25 + _t23))) - _t22 * (_t18 - _t15 + 8.0 * (- _t13 + _t11)))
               );
         }
      }
   }
}

void mapped_fn1_dpbr(int ifirst, int ilast, int jfirst, int jlast, int kfirst,
   int klast, int nk, int* onesided_9, double (* a_acof)[8][6], double (*
   a_bope)[6], double* a_ghcof, double (* a_lu)[25][25][3], double (* a_u)[25][
   25][3], double (* a_mu)[25][25], double (* a_lambda)[25][25], double* h,
   double* a_strx, double* a_stry, double* a_strz, int k1, int k2, int base,
   int base3)
{
   ocrGuid_t outEvt;
   union __args_mapped_fn1_dpbr_main_40* allArgs;
   void* _t1;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, 0);
   _t1 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t1, mapped_fn1_dpbr_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t1, mapped_fn1_dpbr4, 0u, (unsigned long)(1 + k2 + -1 * k1)
      , 0u);
   if (jfirst + - jlast == -24 && ifirst + - ilast == -24) {
      ocrGuid_t _t2;
      union __args_mapped_fn1_dpbr_main_40* _t3;
      _t2 = rocrAlloc((void**)&allArgs, 112ul);
      _t3 = allArgs;
      _t3->data.h = h;
      _t3->data.a_mu = a_mu;
      _t3->data.a_strx = a_strx;
      _t3->data.a_lu = a_lu;
      _t3->data.a_u = a_u;
      _t3->data.a_lambda = a_lambda;
      _t3->data.a_stry = a_stry;
      _t3->data.a_strz = a_strz;
      _t3->data.ifirst = ifirst;
      _t3->data.ilast = ilast;
      _t3->data.jfirst = jfirst;
      _t3->data.jlast = jlast;
      _t3->data.kfirst = kfirst;
      _t3->data.klast = klast;
      _t3->data.nk = nk;
      _t3->data.k1 = k1;
      _t3->data.k2 = k2;
      _t3->data.base = base;
      _t3->data.base3 = base3;
      rocrExecute(_t1, mapped_fn1_dpbr_main, _t2, &outEvt);
   }
   rocrExit(_t1);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

static ocrGuid_t mapped_fn1_dpbr4(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_fn1_dpbr4_38 mapped_fn1_dpbr4_args;
   void* _t1;
   double* h;
   double (* a_mu)[25][25];
   double* a_strx;
   double (* a_lu)[25][25][3];
   double (* a_u)[25][25][3];
   double (* a_lambda)[25][25];
   double* a_stry;
   double* a_strz;
   int ilast;
   int jlast;
   int kfirst;
   int k1;
   int base;
   int base3;
   int IT0;
   int IT1;
   int jfirst;
   int ifirst;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_fn1_dpbr4_args = *(union __args_mapped_fn1_dpbr4_38*)rocrArgs(depv);
   IT1 = mapped_fn1_dpbr4_args.data.IT1;
   IT0 = mapped_fn1_dpbr4_args.data.IT0;
   base3 = mapped_fn1_dpbr4_args.data.base3;
   base = mapped_fn1_dpbr4_args.data.base;
   k1 = mapped_fn1_dpbr4_args.data.k1;
   kfirst = mapped_fn1_dpbr4_args.data.kfirst;
   jlast = mapped_fn1_dpbr4_args.data.jlast;
   ilast = mapped_fn1_dpbr4_args.data.ilast;
   a_strz = mapped_fn1_dpbr4_args.data.a_strz;
   a_stry = mapped_fn1_dpbr4_args.data.a_stry;
   a_lambda = mapped_fn1_dpbr4_args.data.a_lambda;
   a_u = mapped_fn1_dpbr4_args.data.a_u;
   a_lu = mapped_fn1_dpbr4_args.data.a_lu;
   a_strx = mapped_fn1_dpbr4_args.data.a_strx;
   a_mu = mapped_fn1_dpbr4_args.data.a_mu;
   h = mapped_fn1_dpbr4_args.data.h;
   ifirst = mapped_fn1_dpbr4_args.data.ifirst;
   jfirst = mapped_fn1_dpbr4_args.data.jfirst;
   if (mapped_fn1_dpbr4_args.data.IT2 == 0 && (IT1 == 0 && (jfirst + - jlast ==
          -24 && ifirst + - ilast == -24))) {
      int i;
      int i_1;
      int i_2;
      int i_3;
      int i_4;
      int i_5;
      for (i = 0,
           i_1 = jfirst,
           i_2 = 1,
           i_4 = 3,
           i_3 = 2,
           i_5 = 4;
           i <= 20;
           i++,
           i_1++,
           i_2++,
           i_3++,
           i_4++,
           i_5++) {
         int j_29;
         int j_30;
         int j_31;
         int j_32;
         int j_33;
         int j_34;
         for (j_29 = 0,
              j_30 = ifirst,
              j_32 = 2,
              j_31 = 1,
              j_33 = 3,
              j_34 = 4;
              j_29 <= 20;
              j_34++,
              j_33++,
              j_32++,
              j_31++,
              j_30++,
              j_29++) {
            double _sp_0;
            double _sp_1;
            double _sp_2;
            double _sp_3;
            double _sp_4;
            double _sp_5;
            double _sp_6;
            double _sp_7;
            double _sp_8;
            double _sp_9;
            double _sp_10;
            double _sp_11;
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
            _t4 = a_lu[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 +
               1];
            _t14 = a_mu[k1 + (IT0 + 1)][i + (jfirst + 2)][j_30 + (base + 2)] *
               a_strz[-1 * kfirst + k1 + (IT0 + 1)] - 0.75 * (a_mu[k1 + IT0][i +
               (jfirst + 2)][j_30 + (base + 2)] * a_strz[-1 * kfirst + (k1 + IT0
               )] + a_mu[k1 + (IT0 + 2)][i + (jfirst + 2)][j_30 + (base + 2)] *
               a_strz[-1 * kfirst + k1 + (IT0 + 2)]);
            _sp_11 = _t14;
            _t13 = a_mu[k1 + (IT0 + -1)][i + (jfirst + 2)][j_30 + (base + 2)] *
               a_strz[-1 * kfirst + k1 + (IT0 + -1)] + a_mu[k1 + (IT0 + 2)][i +
               (jfirst + 2)][j_30 + (base + 2)] * a_strz[-1 * kfirst + k1 + (
               IT0 + 2)] + 3.0 * (a_mu[k1 + (IT0 + 1)][i + (jfirst + 2)][j_30 +
               (base + 2)] * a_strz[-1 * kfirst + k1 + (IT0 + 1)] + a_mu[k1 +
               IT0][i + (jfirst + 2)][j_30 + (base + 2)] * a_strz[-1 * kfirst +
               (k1 + IT0)]);
            _sp_10 = _t13;
            _t12 = a_mu[k1 + (IT0 + -2)][i + (jfirst + 2)][j_30 + (base + 2)] *
               a_strz[-1 * kfirst + k1 + (IT0 + -2)] + a_mu[k1 + (IT0 + 1)][i +
               (jfirst + 2)][j_30 + (base + 2)] * a_strz[-1 * kfirst + k1 + (
               IT0 + 1)] + 3.0 * (a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base
               + 2)] * a_strz[-1 * kfirst + (k1 + IT0)] + a_mu[k1 + (IT0 + -1)]
               [i + (jfirst + 2)][j_30 + (base + 2)] * a_strz[-1 * kfirst + k1 +
               (IT0 + -1)]);
            _sp_9 = _t12;
            _t11 = a_mu[k1 + (IT0 + -1)][i + (jfirst + 2)][j_30 + (base + 2)] *
               a_strz[-1 * kfirst + k1 + (IT0 + -1)] - 0.75 * (a_mu[k1 + IT0][i
               + (jfirst + 2)][j_30 + (base + 2)] * a_strz[-1 * kfirst + (k1 +
               IT0)] + a_mu[k1 + (IT0 + -2)][i + (jfirst + 2)][j_30 + (base + 2
               )] * a_strz[-1 * kfirst + k1 + (IT0 + -2)]);
            _sp_8 = _t11;
            _t10 = a_mu[k1 + IT0][i + (jfirst + 3)][j_30 + (base + 2)] * a_stry
               [i_4] - 0.75 * (a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 2
               )] * a_stry[i_3] + a_mu[k1 + IT0][i + (jfirst + 4)][j_30 + (base
               + 2)] * a_stry[i_5]);
            _sp_7 = _t10;
            _t9 = a_mu[k1 + IT0][i + (jfirst + 1)][j_30 + (base + 2)] * a_stry[
               i_2] + a_mu[k1 + IT0][i + (jfirst + 4)][j_30 + (base + 2)] *
               a_stry[i_5] + 3.0 * (a_mu[k1 + IT0][i + (jfirst + 3)][j_30 + (
               base + 2)] * a_stry[i_4] + a_mu[k1 + IT0][i + (jfirst + 2)][j_30
               + (base + 2)] * a_stry[i_3]);
            _sp_6 = _t9;
            _t8 = a_mu[k1 + IT0][i_1][j_30 + (base + 2)] * a_stry[i] + a_mu[k1 +
               IT0][i + (jfirst + 3)][j_30 + (base + 2)] * a_stry[i_4] + 3.0 * (
               a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 2)] * a_stry[i_3
               ] + a_mu[k1 + IT0][i + (jfirst + 1)][j_30 + (base + 2)] * a_stry
               [i_2]);
            _sp_5 = _t8;
            _t7 = a_mu[k1 + IT0][i + (jfirst + 1)][j_30 + (base + 2)] * a_stry[
               i_2] - 0.75 * (a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 2
               )] * a_stry[i_3] + a_mu[k1 + IT0][i_1][j_30 + (base + 2)] *
               a_stry[i]);
            _sp_4 = _t7;
            _t6 = a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 3)] * a_strx[
               j_33] - 0.75 * (a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 2
               )] * a_strx[j_32] + a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (
               base + 4)] * a_strx[j_34]);
            _sp_3 = _t6;
            _t5 = a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 1)] * a_strx[
               j_31] + a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 4)] *
               a_strx[j_34] + 3.0 * (a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (
               base + 3)] * a_strx[j_33] + a_mu[k1 + IT0][i + (jfirst + 2)][
               j_30 + (base + 2)] * a_strx[j_32]);
            _sp_2 = _t5;
            _t3 = a_mu[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + base)] *
               a_strx[j_29] + a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 3
               )] * a_strx[j_33] + 3.0 * (a_mu[k1 + IT0][i + (jfirst + 2)][j_30
               + (base + 2)] * a_strx[j_32] + a_mu[k1 + IT0][i + (jfirst + 2)][
               j_30 + (base + 1)] * a_strx[j_31]);
            _sp_1 = _t3;
            _t2 = a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 1)] * a_strx[
               j_31] - 0.75 * (a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 2
               )] * a_strx[j_32] + a_mu[k1 + IT0][i + (jfirst + 2)][j_29 + (
               ifirst + base)] * a_strx[j_29]);
            _sp_0 = _t2;
            _t15 = *h;
            a_lu[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 1] =
               0.0 * _t4 + 1.0 / (_t15 * _t15) * (0.16666666666666666 * (a_strx
               [j_32] * ((2.0 * _t2 + a_lambda[k1 + IT0][i + (jfirst + 2)][j_30
               + (base + 1)] * a_strx[j_31] - 0.75 * (a_lambda[k1 + IT0][i + (
               jfirst + 2)][j_30 + (base + 2)] * a_strx[j_32] + a_lambda[k1 +
               IT0][i + (jfirst + 2)][j_29 + (ifirst + base)] * a_strx[j_29])) *
               (a_u[k1 + IT0][i + (jfirst + 2)][j_30][base3 + 1] - a_u[k1 + IT0
               ][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 1]) + (2.0 * _t3
               + a_lambda[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + base)] *
               a_strx[j_29] + a_lambda[k1 + IT0][i + (jfirst + 2)][j_30 + (base
               + 3)] * a_strx[j_33] + 3.0 * (a_lambda[k1 + IT0][i + (jfirst + 2
               )][j_30 + (base + 2)] * a_strx[j_32] + a_lambda[k1 + IT0][i + (
               jfirst + 2)][j_30 + (base + 1)] * a_strx[j_31])) * (a_u[k1 + IT0
               ][i + (jfirst + 2)][j_29 + (ifirst + 1)][base3 + 1] - a_u[k1 +
               IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 1]) + (2.0 *
               _t5 + a_lambda[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 1)] *
               a_strx[j_31] + a_lambda[k1 + IT0][i + (jfirst + 2)][j_30 + (base
               + 4)] * a_strx[j_34] + 3.0 * (a_lambda[k1 + IT0][i + (jfirst + 2
               )][j_30 + (base + 3)] * a_strx[j_33] + a_lambda[k1 + IT0][i + (
               jfirst + 2)][j_30 + (base + 2)] * a_strx[j_32])) * (a_u[k1 + IT0
               ][i + (jfirst + 2)][j_29 + (ifirst + 3)][base3 + 1] - a_u[k1 +
               IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 1]) + (2.0 *
               _t6 + a_lambda[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 3)] *
               a_strx[j_33] - 0.75 * (a_lambda[k1 + IT0][i + (jfirst + 2)][j_30
               + (base + 2)] * a_strx[j_32] + a_lambda[k1 + IT0][i + (jfirst + 2
               )][j_30 + (base + 4)] * a_strx[j_34])) * (a_u[k1 + IT0][i + (
               jfirst + 2)][j_29 + (ifirst + 4)][base3 + 1] - a_u[k1 + IT0][i +
               (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 1])) + a_stry[i_3] * (
               _t7 * (a_u[k1 + IT0][i_1][j_29 + (ifirst + 2)][base3 + 1] - a_u[
               k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 1]) +
               _t8 * (a_u[k1 + IT0][i + (jfirst + 1)][j_29 + (ifirst + 2)][
               base3 + 1] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)
               ][base3 + 1]) + _t9 * (a_u[k1 + IT0][i + (jfirst + 3)][j_29 + (
               ifirst + 2)][base3 + 1] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 +
               (ifirst + 2)][base3 + 1]) + _t10 * (a_u[k1 + IT0][i + (jfirst + 4
               )][j_29 + (ifirst + 2)][base3 + 1] - a_u[k1 + IT0][i + (jfirst +
               2)][j_29 + (ifirst + 2)][base3 + 1])) + a_strz[-1 * kfirst + (k1
               + IT0)] * (_t11 * (a_u[k1 + (IT0 + -2)][i + (jfirst + 2)][j_29 +
               (ifirst + 2)][base3 + 1] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 +
               (ifirst + 2)][base3 + 1]) + _t12 * (a_u[k1 + (IT0 + -1)][i + (
               jfirst + 2)][j_29 + (ifirst + 2)][base3 + 1] - a_u[k1 + IT0][i +
               (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 1]) + _t13 * (a_u[k1 +
               (IT0 + 1)][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 1] -
               a_u[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 1])
               + _t14 * (a_u[k1 + (IT0 + 2)][i + (jfirst + 2)][j_29 + (ifirst +
               2)][base3 + 1] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst +
               2)][base3 + 1]))) + a_strx[j_32] * a_stry[i_3] *
               0.006944444444444444 * (a_lambda[k1 + IT0][i + (jfirst + 2)][
               j_29 + (ifirst + base)] * (a_u[k1 + IT0][i_1][j_30][base3 + 2] -
               a_u[k1 + IT0][i + (jfirst + 4)][j_30][base3 + 2] + 8.0 * (- a_u[
               k1 + IT0][i + (jfirst + 1)][j_30][base3 + 2] + a_u[k1 + IT0][i +
               (jfirst + 3)][j_30][base3 + 2])) - 8.0 * (a_lambda[k1 + IT0][i +
               (jfirst + 2)][j_30 + (base + 1)] * (a_u[k1 + IT0][i_1][j_29 + (
               ifirst + 1)][base3 + 2] - a_u[k1 + IT0][i + (jfirst + 4)][j_29 +
               (ifirst + 1)][base3 + 2] + 8.0 * (- a_u[k1 + IT0][i + (jfirst + 1
               )][j_29 + (ifirst + 1)][base3 + 2] + a_u[k1 + IT0][i + (jfirst +
               3)][j_29 + (ifirst + 1)][base3 + 2]))) + 8.0 * (a_lambda[k1 + IT0
               ][i + (jfirst + 2)][j_30 + (base + 3)] * (a_u[k1 + IT0][i_1][
               j_29 + (ifirst + 3)][base3 + 2] - a_u[k1 + IT0][i + (jfirst + 4)
               ][j_29 + (ifirst + 3)][base3 + 2] + 8.0 * (- a_u[k1 + IT0][i + (
               jfirst + 1)][j_29 + (ifirst + 3)][base3 + 2] + a_u[k1 + IT0][i +
               (jfirst + 3)][j_29 + (ifirst + 3)][base3 + 2]))) - a_lambda[k1 +
               IT0][i + (jfirst + 2)][j_30 + (base + 4)] * (a_u[k1 + IT0][i_1][
               j_29 + (ifirst + 4)][base3 + 2] - a_u[k1 + IT0][i + (jfirst + 4)
               ][j_29 + (ifirst + 4)][base3 + 2] + 8.0 * (- a_u[k1 + IT0][i + (
               jfirst + 1)][j_29 + (ifirst + 4)][base3 + 2] + a_u[k1 + IT0][i +
               (jfirst + 3)][j_29 + (ifirst + 4)][base3 + 2]))) + a_strx[j_32] *
               a_strz[-1 * kfirst + (k1 + IT0)] * 0.006944444444444444 * (
               a_lambda[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + base)] * (
               a_u[k1 + (IT0 + -2)][i + (jfirst + 2)][j_30][base3 + 3] - a_u[k1
               + (IT0 + 2)][i + (jfirst + 2)][j_30][base3 + 3] + 8.0 * (- a_u[
               k1 + (IT0 + -1)][i + (jfirst + 2)][j_30][base3 + 3] + a_u[k1 + (
               IT0 + 1)][i + (jfirst + 2)][j_30][base3 + 3])) - 8.0 * (a_lambda
               [k1 + IT0][i + (jfirst + 2)][j_30 + (base + 1)] * (a_u[k1 + (IT0
               + -2)][i + (jfirst + 2)][j_29 + (ifirst + 1)][base3 + 3] - a_u[
               k1 + (IT0 + 2)][i + (jfirst + 2)][j_29 + (ifirst + 1)][base3 + 3
               ] + 8.0 * (- a_u[k1 + (IT0 + -1)][i + (jfirst + 2)][j_29 + (
               ifirst + 1)][base3 + 3] + a_u[k1 + (IT0 + 1)][i + (jfirst + 2)][
               j_29 + (ifirst + 1)][base3 + 3]))) + 8.0 * (a_lambda[k1 + IT0][i
               + (jfirst + 2)][j_30 + (base + 3)] * (a_u[k1 + (IT0 + -2)][i + (
               jfirst + 2)][j_29 + (ifirst + 3)][base3 + 3] - a_u[k1 + (IT0 + 2
               )][i + (jfirst + 2)][j_29 + (ifirst + 3)][base3 + 3] + 8.0 * (-
               a_u[k1 + (IT0 + -1)][i + (jfirst + 2)][j_29 + (ifirst + 3)][
               base3 + 3] + a_u[k1 + (IT0 + 1)][i + (jfirst + 2)][j_29 + (
               ifirst + 3)][base3 + 3]))) - a_lambda[k1 + IT0][i + (jfirst + 2)
               ][j_30 + (base + 4)] * (a_u[k1 + (IT0 + -2)][i + (jfirst + 2)][
               j_29 + (ifirst + 4)][base3 + 3] - a_u[k1 + (IT0 + 2)][i + (
               jfirst + 2)][j_29 + (ifirst + 4)][base3 + 3] + 8.0 * (- a_u[k1 +
               (IT0 + -1)][i + (jfirst + 2)][j_29 + (ifirst + 4)][base3 + 3] +
               a_u[k1 + (IT0 + 1)][i + (jfirst + 2)][j_29 + (ifirst + 4)][base3
               + 3]))) + a_strx[j_32] * a_stry[i_3] * 0.006944444444444444 * (
               a_mu[k1 + IT0][i_1][j_30 + (base + 2)] * (a_u[k1 + IT0][i_1][
               j_30][base3 + 2] - a_u[k1 + IT0][i_1][j_29 + (ifirst + 4)][base3
               + 2] + 8.0 * (- a_u[k1 + IT0][i_1][j_29 + (ifirst + 1)][base3 + 2
               ] + a_u[k1 + IT0][i_1][j_29 + (ifirst + 3)][base3 + 2])) - 8.0 *
               (a_mu[k1 + IT0][i + (jfirst + 1)][j_30 + (base + 2)] * (a_u[k1 +
               IT0][i + (jfirst + 1)][j_30][base3 + 2] - a_u[k1 + IT0][i + (
               jfirst + 1)][j_29 + (ifirst + 4)][base3 + 2] + 8.0 * (- a_u[k1 +
               IT0][i + (jfirst + 1)][j_29 + (ifirst + 1)][base3 + 2] + a_u[k1 +
               IT0][i + (jfirst + 1)][j_29 + (ifirst + 3)][base3 + 2]))) + 8.0 *
               (a_mu[k1 + IT0][i + (jfirst + 3)][j_30 + (base + 2)] * (a_u[k1 +
               IT0][i + (jfirst + 3)][j_30][base3 + 2] - a_u[k1 + IT0][i + (
               jfirst + 3)][j_29 + (ifirst + 4)][base3 + 2] + 8.0 * (- a_u[k1 +
               IT0][i + (jfirst + 3)][j_29 + (ifirst + 1)][base3 + 2] + a_u[k1 +
               IT0][i + (jfirst + 3)][j_29 + (ifirst + 3)][base3 + 2]))) - a_mu
               [k1 + IT0][i + (jfirst + 4)][j_30 + (base + 2)] * (a_u[k1 + IT0]
               [i + (jfirst + 4)][j_30][base3 + 2] - a_u[k1 + IT0][i + (jfirst +
               4)][j_29 + (ifirst + 4)][base3 + 2] + 8.0 * (- a_u[k1 + IT0][i +
               (jfirst + 4)][j_29 + (ifirst + 1)][base3 + 2] + a_u[k1 + IT0][i +
               (jfirst + 4)][j_29 + (ifirst + 3)][base3 + 2]))) + a_strx[j_32] *
               a_strz[-1 * kfirst + (k1 + IT0)] * 0.006944444444444444 * (a_mu[
               k1 + (IT0 + -2)][i + (jfirst + 2)][j_30 + (base + 2)] * (a_u[k1 +
               (IT0 + -2)][i + (jfirst + 2)][j_30][base3 + 3] - a_u[k1 + (IT0 +
               -2)][i + (jfirst + 2)][j_29 + (ifirst + 4)][base3 + 3] + 8.0 * (
               - a_u[k1 + (IT0 + -2)][i + (jfirst + 2)][j_29 + (ifirst + 1)][
               base3 + 3] + a_u[k1 + (IT0 + -2)][i + (jfirst + 2)][j_29 + (
               ifirst + 3)][base3 + 3])) - 8.0 * (a_mu[k1 + (IT0 + -1)][i + (
               jfirst + 2)][j_30 + (base + 2)] * (a_u[k1 + (IT0 + -1)][i + (
               jfirst + 2)][j_30][base3 + 3] - a_u[k1 + (IT0 + -1)][i + (jfirst
               + 2)][j_29 + (ifirst + 4)][base3 + 3] + 8.0 * (- a_u[k1 + (IT0 +
               -1)][i + (jfirst + 2)][j_29 + (ifirst + 1)][base3 + 3] + a_u[k1 +
               (IT0 + -1)][i + (jfirst + 2)][j_29 + (ifirst + 3)][base3 + 3])))
               + 8.0 * (a_mu[k1 + (IT0 + 1)][i + (jfirst + 2)][j_30 + (base + 2
               )] * (a_u[k1 + (IT0 + 1)][i + (jfirst + 2)][j_30][base3 + 3] -
               a_u[k1 + (IT0 + 1)][i + (jfirst + 2)][j_29 + (ifirst + 4)][base3
               + 3] + 8.0 * (- a_u[k1 + (IT0 + 1)][i + (jfirst + 2)][j_29 + (
               ifirst + 1)][base3 + 3] + a_u[k1 + (IT0 + 1)][i + (jfirst + 2)][
               j_29 + (ifirst + 3)][base3 + 3]))) - a_mu[k1 + (IT0 + 2)][i + (
               jfirst + 2)][j_30 + (base + 2)] * (a_u[k1 + (IT0 + 2)][i + (
               jfirst + 2)][j_30][base3 + 3] - a_u[k1 + (IT0 + 2)][i + (jfirst +
               2)][j_29 + (ifirst + 4)][base3 + 3] + 8.0 * (- a_u[k1 + (IT0 + 2
               )][i + (jfirst + 2)][j_29 + (ifirst + 1)][base3 + 3] + a_u[k1 + (
               IT0 + 2)][i + (jfirst + 2)][j_29 + (ifirst + 3)][base3 + 3]))));
            _t16 = *h;
            a_lu[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 2] =
               0.0 * a_lu[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)][
               base3 + 2] + 1.0 / (_t16 * _t16) * (0.16666666666666666 * (
               a_strx[j_32] * (_sp_0 * (a_u[k1 + IT0][i + (jfirst + 2)][j_30][
               base3 + 2] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)
               ][base3 + 2]) + _sp_1 * (a_u[k1 + IT0][i + (jfirst + 2)][j_29 + (
               ifirst + 1)][base3 + 2] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 +
               (ifirst + 2)][base3 + 2]) + _sp_2 * (a_u[k1 + IT0][i + (jfirst +
               2)][j_29 + (ifirst + 3)][base3 + 2] - a_u[k1 + IT0][i + (jfirst +
               2)][j_29 + (ifirst + 2)][base3 + 2]) + _sp_3 * (a_u[k1 + IT0][i +
               (jfirst + 2)][j_29 + (ifirst + 4)][base3 + 2] - a_u[k1 + IT0][i +
               (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 2])) + a_stry[i_3] * (
               (2.0 * _sp_4 + a_lambda[k1 + IT0][i + (jfirst + 1)][j_30 + (base
               + 2)] * a_stry[i_2] - 0.75 * (a_lambda[k1 + IT0][i + (jfirst + 2
               )][j_30 + (base + 2)] * a_stry[i_3] + a_lambda[k1 + IT0][i_1][
               j_30 + (base + 2)] * a_stry[i])) * (a_u[k1 + IT0][i_1][j_29 + (
               ifirst + 2)][base3 + 2] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 +
               (ifirst + 2)][base3 + 2]) + (2.0 * _sp_5 + a_lambda[k1 + IT0][
               i_1][j_30 + (base + 2)] * a_stry[i] + a_lambda[k1 + IT0][i + (
               jfirst + 3)][j_30 + (base + 2)] * a_stry[i_4] + 3.0 * (a_lambda[
               k1 + IT0][i + (jfirst + 2)][j_30 + (base + 2)] * a_stry[i_3] +
               a_lambda[k1 + IT0][i + (jfirst + 1)][j_30 + (base + 2)] * a_stry
               [i_2])) * (a_u[k1 + IT0][i + (jfirst + 1)][j_29 + (ifirst + 2)][
               base3 + 2] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)
               ][base3 + 2]) + (2.0 * _sp_6 + a_lambda[k1 + IT0][i + (jfirst + 1
               )][j_30 + (base + 2)] * a_stry[i_2] + a_lambda[k1 + IT0][i + (
               jfirst + 4)][j_30 + (base + 2)] * a_stry[i_5] + 3.0 * (a_lambda[
               k1 + IT0][i + (jfirst + 3)][j_30 + (base + 2)] * a_stry[i_4] +
               a_lambda[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 2)] * a_stry
               [i_3])) * (a_u[k1 + IT0][i + (jfirst + 3)][j_29 + (ifirst + 2)][
               base3 + 2] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)
               ][base3 + 2]) + (2.0 * _sp_7 + a_lambda[k1 + IT0][i + (jfirst + 3
               )][j_30 + (base + 2)] * a_stry[i_4] - 0.75 * (a_lambda[k1 + IT0]
               [i + (jfirst + 2)][j_30 + (base + 2)] * a_stry[i_3] + a_lambda[
               k1 + IT0][i + (jfirst + 4)][j_30 + (base + 2)] * a_stry[i_5])) *
               (a_u[k1 + IT0][i + (jfirst + 4)][j_29 + (ifirst + 2)][base3 + 2]
               - a_u[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 2
               ])) + a_strz[-1 * kfirst + (k1 + IT0)] * (_sp_8 * (a_u[k1 + (IT0
               + -2)][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 2] - a_u[
               k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 2]) +
               _sp_9 * (a_u[k1 + (IT0 + -1)][i + (jfirst + 2)][j_29 + (ifirst +
               2)][base3 + 2] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst +
               2)][base3 + 2]) + _sp_10 * (a_u[k1 + (IT0 + 1)][i + (jfirst + 2)
               ][j_29 + (ifirst + 2)][base3 + 2] - a_u[k1 + IT0][i + (jfirst + 2
               )][j_29 + (ifirst + 2)][base3 + 2]) + _sp_11 * (a_u[k1 + (IT0 + 2
               )][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 2] - a_u[k1 +
               IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 2]))) +
               a_strx[j_32] * a_stry[i_3] * 0.006944444444444444 * (a_mu[k1 +
               IT0][i + (jfirst + 2)][j_29 + (ifirst + base)] * (a_u[k1 + IT0][
               i_1][j_30][base3 + 1] - a_u[k1 + IT0][i + (jfirst + 4)][j_30][
               base3 + 1] + 8.0 * (- a_u[k1 + IT0][i + (jfirst + 1)][j_30][
               base3 + 1] + a_u[k1 + IT0][i + (jfirst + 3)][j_30][base3 + 1])) -
               8.0 * (a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 1)] * (
               a_u[k1 + IT0][i_1][j_29 + (ifirst + 1)][base3 + 1] - a_u[k1 + IT0
               ][i + (jfirst + 4)][j_29 + (ifirst + 1)][base3 + 1] + 8.0 * (-
               a_u[k1 + IT0][i + (jfirst + 1)][j_29 + (ifirst + 1)][base3 + 1] +
               a_u[k1 + IT0][i + (jfirst + 3)][j_29 + (ifirst + 1)][base3 + 1])
               )) + 8.0 * (a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 3)] *
               (a_u[k1 + IT0][i_1][j_29 + (ifirst + 3)][base3 + 1] - a_u[k1 +
               IT0][i + (jfirst + 4)][j_29 + (ifirst + 3)][base3 + 1] + 8.0 * (
               - a_u[k1 + IT0][i + (jfirst + 1)][j_29 + (ifirst + 3)][base3 + 1
               ] + a_u[k1 + IT0][i + (jfirst + 3)][j_29 + (ifirst + 3)][base3 +
               1]))) - a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 4)] * (
               a_u[k1 + IT0][i_1][j_29 + (ifirst + 4)][base3 + 1] - a_u[k1 + IT0
               ][i + (jfirst + 4)][j_29 + (ifirst + 4)][base3 + 1] + 8.0 * (-
               a_u[k1 + IT0][i + (jfirst + 1)][j_29 + (ifirst + 4)][base3 + 1] +
               a_u[k1 + IT0][i + (jfirst + 3)][j_29 + (ifirst + 4)][base3 + 1])
               )) + a_strx[j_32] * a_stry[i_3] * 0.006944444444444444 * (
               a_lambda[k1 + IT0][i_1][j_30 + (base + 2)] * (a_u[k1 + IT0][i_1]
               [j_30][base3 + 1] - a_u[k1 + IT0][i_1][j_29 + (ifirst + 4)][
               base3 + 1] + 8.0 * (- a_u[k1 + IT0][i_1][j_29 + (ifirst + 1)][
               base3 + 1] + a_u[k1 + IT0][i_1][j_29 + (ifirst + 3)][base3 + 1])
               ) - 8.0 * (a_lambda[k1 + IT0][i + (jfirst + 1)][j_30 + (base + 2
               )] * (a_u[k1 + IT0][i + (jfirst + 1)][j_30][base3 + 1] - a_u[k1 +
               IT0][i + (jfirst + 1)][j_29 + (ifirst + 4)][base3 + 1] + 8.0 * (
               - a_u[k1 + IT0][i + (jfirst + 1)][j_29 + (ifirst + 1)][base3 + 1
               ] + a_u[k1 + IT0][i + (jfirst + 1)][j_29 + (ifirst + 3)][base3 +
               1]))) + 8.0 * (a_lambda[k1 + IT0][i + (jfirst + 3)][j_30 + (base
               + 2)] * (a_u[k1 + IT0][i + (jfirst + 3)][j_30][base3 + 1] - a_u[
               k1 + IT0][i + (jfirst + 3)][j_29 + (ifirst + 4)][base3 + 1] + 8.0
               * (- a_u[k1 + IT0][i + (jfirst + 3)][j_29 + (ifirst + 1)][base3 +
               1] + a_u[k1 + IT0][i + (jfirst + 3)][j_29 + (ifirst + 3)][base3 +
               1]))) - a_lambda[k1 + IT0][i + (jfirst + 4)][j_30 + (base + 2)] *
               (a_u[k1 + IT0][i + (jfirst + 4)][j_30][base3 + 1] - a_u[k1 + IT0
               ][i + (jfirst + 4)][j_29 + (ifirst + 4)][base3 + 1] + 8.0 * (-
               a_u[k1 + IT0][i + (jfirst + 4)][j_29 + (ifirst + 1)][base3 + 1] +
               a_u[k1 + IT0][i + (jfirst + 4)][j_29 + (ifirst + 3)][base3 + 1])
               )) + a_stry[i_3] * a_strz[-1 * kfirst + (k1 + IT0)] *
               0.006944444444444444 * (a_lambda[k1 + IT0][i_1][j_30 + (base + 2
               )] * (a_u[k1 + (IT0 + -2)][i_1][j_29 + (ifirst + 2)][base3 + 3] -
               a_u[k1 + (IT0 + 2)][i_1][j_29 + (ifirst + 2)][base3 + 3] + 8.0 *
               (- a_u[k1 + (IT0 + -1)][i_1][j_29 + (ifirst + 2)][base3 + 3] +
               a_u[k1 + (IT0 + 1)][i_1][j_29 + (ifirst + 2)][base3 + 3])) - 8.0
               * (a_lambda[k1 + IT0][i + (jfirst + 1)][j_30 + (base + 2)] * (
               a_u[k1 + (IT0 + -2)][i + (jfirst + 1)][j_29 + (ifirst + 2)][
               base3 + 3] - a_u[k1 + (IT0 + 2)][i + (jfirst + 1)][j_29 + (
               ifirst + 2)][base3 + 3] + 8.0 * (- a_u[k1 + (IT0 + -1)][i + (
               jfirst + 1)][j_29 + (ifirst + 2)][base3 + 3] + a_u[k1 + (IT0 + 1
               )][i + (jfirst + 1)][j_29 + (ifirst + 2)][base3 + 3]))) + 8.0 * (
               a_lambda[k1 + IT0][i + (jfirst + 3)][j_30 + (base + 2)] * (a_u[
               k1 + (IT0 + -2)][i + (jfirst + 3)][j_29 + (ifirst + 2)][base3 + 3
               ] - a_u[k1 + (IT0 + 2)][i + (jfirst + 3)][j_29 + (ifirst + 2)][
               base3 + 3] + 8.0 * (- a_u[k1 + (IT0 + -1)][i + (jfirst + 3)][
               j_29 + (ifirst + 2)][base3 + 3] + a_u[k1 + (IT0 + 1)][i + (
               jfirst + 3)][j_29 + (ifirst + 2)][base3 + 3]))) - a_lambda[k1 +
               IT0][i + (jfirst + 4)][j_30 + (base + 2)] * (a_u[k1 + (IT0 + -2)
               ][i + (jfirst + 4)][j_29 + (ifirst + 2)][base3 + 3] - a_u[k1 + (
               IT0 + 2)][i + (jfirst + 4)][j_29 + (ifirst + 2)][base3 + 3] + 8.0
               * (- a_u[k1 + (IT0 + -1)][i + (jfirst + 4)][j_29 + (ifirst + 2)]
               [base3 + 3] + a_u[k1 + (IT0 + 1)][i + (jfirst + 4)][j_29 + (
               ifirst + 2)][base3 + 3]))) + a_stry[i_3] * a_strz[-1 * kfirst + (
               k1 + IT0)] * 0.006944444444444444 * (a_mu[k1 + (IT0 + -2)][i + (
               jfirst + 2)][j_30 + (base + 2)] * (a_u[k1 + (IT0 + -2)][i_1][
               j_29 + (ifirst + 2)][base3 + 3] - a_u[k1 + (IT0 + -2)][i + (
               jfirst + 4)][j_29 + (ifirst + 2)][base3 + 3] + 8.0 * (- a_u[k1 +
               (IT0 + -2)][i + (jfirst + 1)][j_29 + (ifirst + 2)][base3 + 3] +
               a_u[k1 + (IT0 + -2)][i + (jfirst + 3)][j_29 + (ifirst + 2)][
               base3 + 3])) - 8.0 * (a_mu[k1 + (IT0 + -1)][i + (jfirst + 2)][
               j_30 + (base + 2)] * (a_u[k1 + (IT0 + -1)][i_1][j_29 + (ifirst +
               2)][base3 + 3] - a_u[k1 + (IT0 + -1)][i + (jfirst + 4)][j_29 + (
               ifirst + 2)][base3 + 3] + 8.0 * (- a_u[k1 + (IT0 + -1)][i + (
               jfirst + 1)][j_29 + (ifirst + 2)][base3 + 3] + a_u[k1 + (IT0 + -1
               )][i + (jfirst + 3)][j_29 + (ifirst + 2)][base3 + 3]))) + 8.0 * (
               a_mu[k1 + (IT0 + 1)][i + (jfirst + 2)][j_30 + (base + 2)] * (a_u
               [k1 + (IT0 + 1)][i_1][j_29 + (ifirst + 2)][base3 + 3] - a_u[k1 +
               (IT0 + 1)][i + (jfirst + 4)][j_29 + (ifirst + 2)][base3 + 3] +
               8.0 * (- a_u[k1 + (IT0 + 1)][i + (jfirst + 1)][j_29 + (ifirst + 2
               )][base3 + 3] + a_u[k1 + (IT0 + 1)][i + (jfirst + 3)][j_29 + (
               ifirst + 2)][base3 + 3]))) - a_mu[k1 + (IT0 + 2)][i + (jfirst + 2
               )][j_30 + (base + 2)] * (a_u[k1 + (IT0 + 2)][i_1][j_29 + (ifirst
               + 2)][base3 + 3] - a_u[k1 + (IT0 + 2)][i + (jfirst + 4)][j_29 + (
               ifirst + 2)][base3 + 3] + 8.0 * (- a_u[k1 + (IT0 + 2)][i + (
               jfirst + 1)][j_29 + (ifirst + 2)][base3 + 3] + a_u[k1 + (IT0 + 2
               )][i + (jfirst + 3)][j_29 + (ifirst + 2)][base3 + 3]))));
            _t17 = *h;
            a_lu[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 3] =
               0.0 * a_lu[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)][
               base3 + 3] + 1.0 / (_t17 * _t17) * (0.16666666666666666 * (
               a_strx[j_32] * (_sp_0 * (a_u[k1 + IT0][i + (jfirst + 2)][j_30][
               base3 + 3] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)
               ][base3 + 3]) + _sp_1 * (a_u[k1 + IT0][i + (jfirst + 2)][j_29 + (
               ifirst + 1)][base3 + 3] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 +
               (ifirst + 2)][base3 + 3]) + _sp_2 * (a_u[k1 + IT0][i + (jfirst +
               2)][j_29 + (ifirst + 3)][base3 + 3] - a_u[k1 + IT0][i + (jfirst +
               2)][j_29 + (ifirst + 2)][base3 + 3]) + _sp_3 * (a_u[k1 + IT0][i +
               (jfirst + 2)][j_29 + (ifirst + 4)][base3 + 3] - a_u[k1 + IT0][i +
               (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 3])) + a_stry[i_3] * (
               _sp_4 * (a_u[k1 + IT0][i_1][j_29 + (ifirst + 2)][base3 + 3] - a_u
               [k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)][base3 + 3]) +
               _sp_5 * (a_u[k1 + IT0][i + (jfirst + 1)][j_29 + (ifirst + 2)][
               base3 + 3] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst + 2)
               ][base3 + 3]) + _sp_6 * (a_u[k1 + IT0][i + (jfirst + 3)][j_29 + (
               ifirst + 2)][base3 + 3] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 +
               (ifirst + 2)][base3 + 3]) + _sp_7 * (a_u[k1 + IT0][i + (jfirst +
               4)][j_29 + (ifirst + 2)][base3 + 3] - a_u[k1 + IT0][i + (jfirst +
               2)][j_29 + (ifirst + 2)][base3 + 3])) + a_strz[-1 * kfirst + (k1
               + IT0)] * ((2.0 * _sp_8 + a_lambda[k1 + (IT0 + -1)][i + (jfirst +
               2)][j_30 + (base + 2)] * a_strz[-1 * kfirst + k1 + (IT0 + -1)] -
               0.75 * (a_lambda[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 2)] *
               a_strz[-1 * kfirst + (k1 + IT0)] + a_lambda[k1 + (IT0 + -2)][i +
               (jfirst + 2)][j_30 + (base + 2)] * a_strz[-1 * kfirst + k1 + (
               IT0 + -2)])) * (a_u[k1 + (IT0 + -2)][i + (jfirst + 2)][j_29 + (
               ifirst + 2)][base3 + 3] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 +
               (ifirst + 2)][base3 + 3]) + (2.0 * _sp_9 + a_lambda[k1 + (IT0 +
               -2)][i + (jfirst + 2)][j_30 + (base + 2)] * a_strz[-1 * kfirst +
               k1 + (IT0 + -2)] + a_lambda[k1 + (IT0 + 1)][i + (jfirst + 2)][
               j_30 + (base + 2)] * a_strz[-1 * kfirst + k1 + (IT0 + 1)] + 3.0 *
               (a_lambda[k1 + IT0][i + (jfirst + 2)][j_30 + (base + 2)] * a_strz
               [-1 * kfirst + (k1 + IT0)] + a_lambda[k1 + (IT0 + -1)][i + (
               jfirst + 2)][j_30 + (base + 2)] * a_strz[-1 * kfirst + k1 + (IT0
               + -1)])) * (a_u[k1 + (IT0 + -1)][i + (jfirst + 2)][j_29 + (
               ifirst + 2)][base3 + 3] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 +
               (ifirst + 2)][base3 + 3]) + (2.0 * _sp_10 + a_lambda[k1 + (IT0 +
               -1)][i + (jfirst + 2)][j_30 + (base + 2)] * a_strz[-1 * kfirst +
               k1 + (IT0 + -1)] + a_lambda[k1 + (IT0 + 2)][i + (jfirst + 2)][
               j_30 + (base + 2)] * a_strz[-1 * kfirst + k1 + (IT0 + 2)] + 3.0 *
               (a_lambda[k1 + (IT0 + 1)][i + (jfirst + 2)][j_30 + (base + 2)] *
               a_strz[-1 * kfirst + k1 + (IT0 + 1)] + a_lambda[k1 + IT0][i + (
               jfirst + 2)][j_30 + (base + 2)] * a_strz[-1 * kfirst + (k1 + IT0
               )])) * (a_u[k1 + (IT0 + 1)][i + (jfirst + 2)][j_29 + (ifirst + 2
               )][base3 + 3] - a_u[k1 + IT0][i + (jfirst + 2)][j_29 + (ifirst +
               2)][base3 + 3]) + (2.0 * _sp_11 + a_lambda[k1 + (IT0 + 1)][i + (
               jfirst + 2)][j_30 + (base + 2)] * a_strz[-1 * kfirst + k1 + (IT0
               + 1)] - 0.75 * (a_lambda[k1 + IT0][i + (jfirst + 2)][j_30 + (
               base + 2)] * a_strz[-1 * kfirst + (k1 + IT0)] + a_lambda[k1 + (
               IT0 + 2)][i + (jfirst + 2)][j_30 + (base + 2)] * a_strz[-1 *
               kfirst + k1 + (IT0 + 2)])) * (a_u[k1 + (IT0 + 2)][i + (jfirst + 2
               )][j_29 + (ifirst + 2)][base3 + 3] - a_u[k1 + IT0][i + (jfirst +
               2)][j_29 + (ifirst + 2)][base3 + 3]))) + a_strx[j_32] * a_strz[
               -1 * kfirst + (k1 + IT0)] * 0.006944444444444444 * (a_mu[k1 + IT0
               ][i + (jfirst + 2)][j_29 + (ifirst + base)] * (a_u[k1 + (IT0 + -2
               )][i + (jfirst + 2)][j_30][base3 + 1] - a_u[k1 + (IT0 + 2)][i + (
               jfirst + 2)][j_30][base3 + 1] + 8.0 * (- a_u[k1 + (IT0 + -1)][i +
               (jfirst + 2)][j_30][base3 + 1] + a_u[k1 + (IT0 + 1)][i + (jfirst
               + 2)][j_30][base3 + 1])) - 8.0 * (a_mu[k1 + IT0][i + (jfirst + 2
               )][j_30 + (base + 1)] * (a_u[k1 + (IT0 + -2)][i + (jfirst + 2)][
               j_29 + (ifirst + 1)][base3 + 1] - a_u[k1 + (IT0 + 2)][i + (
               jfirst + 2)][j_29 + (ifirst + 1)][base3 + 1] + 8.0 * (- a_u[k1 +
               (IT0 + -1)][i + (jfirst + 2)][j_29 + (ifirst + 1)][base3 + 1] +
               a_u[k1 + (IT0 + 1)][i + (jfirst + 2)][j_29 + (ifirst + 1)][base3
               + 1]))) + 8.0 * (a_mu[k1 + IT0][i + (jfirst + 2)][j_30 + (base +
               3)] * (a_u[k1 + (IT0 + -2)][i + (jfirst + 2)][j_29 + (ifirst + 3
               )][base3 + 1] - a_u[k1 + (IT0 + 2)][i + (jfirst + 2)][j_29 + (
               ifirst + 3)][base3 + 1] + 8.0 * (- a_u[k1 + (IT0 + -1)][i + (
               jfirst + 2)][j_29 + (ifirst + 3)][base3 + 1] + a_u[k1 + (IT0 + 1
               )][i + (jfirst + 2)][j_29 + (ifirst + 3)][base3 + 1]))) - a_mu[
               k1 + IT0][i + (jfirst + 2)][j_30 + (base + 4)] * (a_u[k1 + (IT0 +
               -2)][i + (jfirst + 2)][j_29 + (ifirst + 4)][base3 + 1] - a_u[k1 +
               (IT0 + 2)][i + (jfirst + 2)][j_29 + (ifirst + 4)][base3 + 1] +
               8.0 * (- a_u[k1 + (IT0 + -1)][i + (jfirst + 2)][j_29 + (ifirst +
               4)][base3 + 1] + a_u[k1 + (IT0 + 1)][i + (jfirst + 2)][j_29 + (
               ifirst + 4)][base3 + 1]))) + a_stry[i_3] * a_strz[-1 * kfirst + (
               k1 + IT0)] * 0.006944444444444444 * (a_mu[k1 + IT0][i_1][j_30 + (
               base + 2)] * (a_u[k1 + (IT0 + -2)][i_1][j_29 + (ifirst + 2)][
               base3 + 2] - a_u[k1 + (IT0 + 2)][i_1][j_29 + (ifirst + 2)][base3
               + 2] + 8.0 * (- a_u[k1 + (IT0 + -1)][i_1][j_29 + (ifirst + 2)][
               base3 + 2] + a_u[k1 + (IT0 + 1)][i_1][j_29 + (ifirst + 2)][base3
               + 2])) - 8.0 * (a_mu[k1 + IT0][i + (jfirst + 1)][j_30 + (base + 2
               )] * (a_u[k1 + (IT0 + -2)][i + (jfirst + 1)][j_29 + (ifirst + 2)
               ][base3 + 2] - a_u[k1 + (IT0 + 2)][i + (jfirst + 1)][j_29 + (
               ifirst + 2)][base3 + 2] + 8.0 * (- a_u[k1 + (IT0 + -1)][i + (
               jfirst + 1)][j_29 + (ifirst + 2)][base3 + 2] + a_u[k1 + (IT0 + 1
               )][i + (jfirst + 1)][j_29 + (ifirst + 2)][base3 + 2]))) + 8.0 * (
               a_mu[k1 + IT0][i + (jfirst + 3)][j_30 + (base + 2)] * (a_u[k1 + (
               IT0 + -2)][i + (jfirst + 3)][j_29 + (ifirst + 2)][base3 + 2] -
               a_u[k1 + (IT0 + 2)][i + (jfirst + 3)][j_29 + (ifirst + 2)][base3
               + 2] + 8.0 * (- a_u[k1 + (IT0 + -1)][i + (jfirst + 3)][j_29 + (
               ifirst + 2)][base3 + 2] + a_u[k1 + (IT0 + 1)][i + (jfirst + 3)][
               j_29 + (ifirst + 2)][base3 + 2]))) - a_mu[k1 + IT0][i + (jfirst +
               4)][j_30 + (base + 2)] * (a_u[k1 + (IT0 + -2)][i + (jfirst + 4)]
               [j_29 + (ifirst + 2)][base3 + 2] - a_u[k1 + (IT0 + 2)][i + (
               jfirst + 4)][j_29 + (ifirst + 2)][base3 + 2] + 8.0 * (- a_u[k1 +
               (IT0 + -1)][i + (jfirst + 4)][j_29 + (ifirst + 2)][base3 + 2] +
               a_u[k1 + (IT0 + 1)][i + (jfirst + 4)][j_29 + (ifirst + 2)][base3
               + 2]))) + a_strx[j_32] * a_strz[-1 * kfirst + (k1 + IT0)] *
               0.006944444444444444 * (a_lambda[k1 + (IT0 + -2)][i + (jfirst + 2
               )][j_30 + (base + 2)] * (a_u[k1 + (IT0 + -2)][i + (jfirst + 2)][
               j_30][base3 + 1] - a_u[k1 + (IT0 + -2)][i + (jfirst + 2)][j_29 +
               (ifirst + 4)][base3 + 1] + 8.0 * (- a_u[k1 + (IT0 + -2)][i + (
               jfirst + 2)][j_29 + (ifirst + 1)][base3 + 1] + a_u[k1 + (IT0 + -2
               )][i + (jfirst + 2)][j_29 + (ifirst + 3)][base3 + 1])) - 8.0 * (
               a_lambda[k1 + (IT0 + -1)][i + (jfirst + 2)][j_30 + (base + 2)] *
               (a_u[k1 + (IT0 + -1)][i + (jfirst + 2)][j_30][base3 + 1] - a_u[
               k1 + (IT0 + -1)][i + (jfirst + 2)][j_29 + (ifirst + 4)][base3 + 1
               ] + 8.0 * (- a_u[k1 + (IT0 + -1)][i + (jfirst + 2)][j_29 + (
               ifirst + 1)][base3 + 1] + a_u[k1 + (IT0 + -1)][i + (jfirst + 2)]
               [j_29 + (ifirst + 3)][base3 + 1]))) + 8.0 * (a_lambda[k1 + (IT0 +
               1)][i + (jfirst + 2)][j_30 + (base + 2)] * (a_u[k1 + (IT0 + 1)][
               i + (jfirst + 2)][j_30][base3 + 1] - a_u[k1 + (IT0 + 1)][i + (
               jfirst + 2)][j_29 + (ifirst + 4)][base3 + 1] + 8.0 * (- a_u[k1 +
               (IT0 + 1)][i + (jfirst + 2)][j_29 + (ifirst + 1)][base3 + 1] +
               a_u[k1 + (IT0 + 1)][i + (jfirst + 2)][j_29 + (ifirst + 3)][base3
               + 1]))) - a_lambda[k1 + (IT0 + 2)][i + (jfirst + 2)][j_30 + (
               base + 2)] * (a_u[k1 + (IT0 + 2)][i + (jfirst + 2)][j_30][base3 +
               1] - a_u[k1 + (IT0 + 2)][i + (jfirst + 2)][j_29 + (ifirst + 4)][
               base3 + 1] + 8.0 * (- a_u[k1 + (IT0 + 2)][i + (jfirst + 2)][j_29
               + (ifirst + 1)][base3 + 1] + a_u[k1 + (IT0 + 2)][i + (jfirst + 2
               )][j_29 + (ifirst + 3)][base3 + 1]))) + a_stry[i_3] * a_strz[-1 *
               kfirst + (k1 + IT0)] * 0.006944444444444444 * (a_lambda[k1 + (
               IT0 + -2)][i + (jfirst + 2)][j_30 + (base + 2)] * (a_u[k1 + (IT0
               + -2)][i_1][j_29 + (ifirst + 2)][base3 + 2] - a_u[k1 + (IT0 + -2
               )][i + (jfirst + 4)][j_29 + (ifirst + 2)][base3 + 2] + 8.0 * (-
               a_u[k1 + (IT0 + -2)][i + (jfirst + 1)][j_29 + (ifirst + 2)][
               base3 + 2] + a_u[k1 + (IT0 + -2)][i + (jfirst + 3)][j_29 + (
               ifirst + 2)][base3 + 2])) - 8.0 * (a_lambda[k1 + (IT0 + -1)][i +
               (jfirst + 2)][j_30 + (base + 2)] * (a_u[k1 + (IT0 + -1)][i_1][
               j_29 + (ifirst + 2)][base3 + 2] - a_u[k1 + (IT0 + -1)][i + (
               jfirst + 4)][j_29 + (ifirst + 2)][base3 + 2] + 8.0 * (- a_u[k1 +
               (IT0 + -1)][i + (jfirst + 1)][j_29 + (ifirst + 2)][base3 + 2] +
               a_u[k1 + (IT0 + -1)][i + (jfirst + 3)][j_29 + (ifirst + 2)][
               base3 + 2]))) + 8.0 * (a_lambda[k1 + (IT0 + 1)][i + (jfirst + 2)
               ][j_30 + (base + 2)] * (a_u[k1 + (IT0 + 1)][i_1][j_29 + (ifirst +
               2)][base3 + 2] - a_u[k1 + (IT0 + 1)][i + (jfirst + 4)][j_29 + (
               ifirst + 2)][base3 + 2] + 8.0 * (- a_u[k1 + (IT0 + 1)][i + (
               jfirst + 1)][j_29 + (ifirst + 2)][base3 + 2] + a_u[k1 + (IT0 + 1
               )][i + (jfirst + 3)][j_29 + (ifirst + 2)][base3 + 2]))) -
               a_lambda[k1 + (IT0 + 2)][i + (jfirst + 2)][j_30 + (base + 2)] * (
               a_u[k1 + (IT0 + 2)][i_1][j_29 + (ifirst + 2)][base3 + 2] - a_u[
               k1 + (IT0 + 2)][i + (jfirst + 4)][j_29 + (ifirst + 2)][base3 + 2
               ] + 8.0 * (- a_u[k1 + (IT0 + 2)][i + (jfirst + 1)][j_29 + (
               ifirst + 2)][base3 + 2] + a_u[k1 + (IT0 + 2)][i + (jfirst + 3)][
               j_29 + (ifirst + 2)][base3 + 2]))));
         }
      }
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t mapped_fn1_dpbr_main(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_fn1_dpbr_main_40 mapped_fn1_dpbr_main_args;
   union __args_mapped_fn1_dpbr4_38* allArgs;
   void* _t1;
   double* h;
   double (* a_mu)[25][25];
   double* a_strx;
   double (* a_lu)[25][25][3];
   double (* a_u)[25][25][3];
   double (* a_lambda)[25][25];
   double* a_stry;
   double* a_strz;
   int ifirst;
   int ilast;
   int jfirst;
   int jlast;
   int kfirst;
   int klast;
   int nk;
   int k1;
   int k2;
   int base;
   int base3;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_fn1_dpbr_main_args = *(union __args_mapped_fn1_dpbr_main_40*)rocrArgs
      (depv);
   base3 = mapped_fn1_dpbr_main_args.data.base3;
   base = mapped_fn1_dpbr_main_args.data.base;
   k2 = mapped_fn1_dpbr_main_args.data.k2;
   k1 = mapped_fn1_dpbr_main_args.data.k1;
   nk = mapped_fn1_dpbr_main_args.data.nk;
   klast = mapped_fn1_dpbr_main_args.data.klast;
   kfirst = mapped_fn1_dpbr_main_args.data.kfirst;
   ilast = mapped_fn1_dpbr_main_args.data.ilast;
   ifirst = mapped_fn1_dpbr_main_args.data.ifirst;
   a_strz = mapped_fn1_dpbr_main_args.data.a_strz;
   a_stry = mapped_fn1_dpbr_main_args.data.a_stry;
   a_lambda = mapped_fn1_dpbr_main_args.data.a_lambda;
   a_u = mapped_fn1_dpbr_main_args.data.a_u;
   a_lu = mapped_fn1_dpbr_main_args.data.a_lu;
   a_strx = mapped_fn1_dpbr_main_args.data.a_strx;
   a_mu = mapped_fn1_dpbr_main_args.data.a_mu;
   h = mapped_fn1_dpbr_main_args.data.h;
   jlast = mapped_fn1_dpbr_main_args.data.jlast;
   jfirst = mapped_fn1_dpbr_main_args.data.jfirst;
   if (jfirst + - jlast == -24 && ifirst + - ilast == -24) {
      int _t2;
      int i;
      for (_t2 = - k1 + k2, i = 0; i <= _t2; i++) {
         ocrGuid_t _t3;
         union __args_mapped_fn1_dpbr4_38* _t4;
         _t3 = rocrAlloc((void**)&allArgs, 120ul);
         _t4 = allArgs;
         _t4->data.h = h;
         _t4->data.a_mu = a_mu;
         _t4->data.a_strx = a_strx;
         _t4->data.a_lu = a_lu;
         _t4->data.a_u = a_u;
         _t4->data.a_lambda = a_lambda;
         _t4->data.a_stry = a_stry;
         _t4->data.a_strz = a_strz;
         _t4->data.ifirst = ifirst;
         _t4->data.ilast = ilast;
         _t4->data.jfirst = jfirst;
         _t4->data.jlast = jlast;
         _t4->data.kfirst = kfirst;
         _t4->data.klast = klast;
         _t4->data.nk = nk;
         _t4->data.k1 = k1;
         _t4->data.k2 = k2;
         _t4->data.base = base;
         _t4->data.base3 = base3;
         _t4->data.IT0 = i;
         _t4->data.IT1 = 0;
         _t4->data.IT2 = 0;
         rocrAutoDec(_t1, 0u, (unsigned long)i, _t3, (unsigned long (*)(long*))
            ((void*)0l), (void (*)(void*, long*))((void*)0l), (void (*)(void*,
            long*))((void*)0l), 14u, (long)ifirst, (long)ilast, (long)jfirst, (
            long)jlast, (long)kfirst, (long)klast, (long)nk, (long)k1, (long)k2
            , (long)base, (long)base3, (long)i, (long)0, (long)0);
      }
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

/*
 * ../src/rhs4sg.c:1647.6
 */
void mapped_fn1_async(void* event, int ifirst, int ilast, int jfirst, int jlast
   , int kfirst, int klast, int nk, int* onesided_10, double (* a_acof)[8][6],
   double (* a_bope)[6], double* a_ghcof, double (* a_lu)[25][25][3], double (*
    a_u)[25][25][3], double (* a_mu)[25][25], double (* a_lambda)[25][25],
   double h, double* a_strx, double* a_stry, double* a_strz, int k1, int k2,
   int base, int base3)
{
   double h_var;
   union __args_mapped_fn1_dpbr_main_40* allArgs;
   void* _t1;
   h_var = h;
   _t1 = rocrInit(2u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t1, mapped_fn1_dpbr_main, 1u, (unsigned long)1, 0u);
   rocrDeclareType(_t1, mapped_fn1_dpbr4, 0u, (unsigned long)(1 + k2 + -1 * k1)
      , 0u);
   if (jfirst + - jlast == -24 && ifirst + - ilast == -24) {
      ocrGuid_t _t2;
      union __args_mapped_fn1_dpbr_main_40* _t3;
      _t2 = rocrAlloc((void**)&allArgs, 112ul);
      _t3 = allArgs;
      _t3->data.h = &h_var;
      _t3->data.a_mu = a_mu;
      _t3->data.a_strx = a_strx;
      _t3->data.a_lu = a_lu;
      _t3->data.a_u = a_u;
      _t3->data.a_lambda = a_lambda;
      _t3->data.a_stry = a_stry;
      _t3->data.a_strz = a_strz;
      _t3->data.ifirst = ifirst;
      _t3->data.ilast = ilast;
      _t3->data.jfirst = jfirst;
      _t3->data.jlast = jlast;
      _t3->data.kfirst = kfirst;
      _t3->data.klast = klast;
      _t3->data.nk = nk;
      _t3->data.k1 = k1;
      _t3->data.k2 = k2;
      _t3->data.base = base;
      _t3->data.base3 = base3;
      rocrExecute(_t1, mapped_fn1_dpbr_main, _t2, event);
   }
   rocrExit(_t1);
}


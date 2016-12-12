/*
 * Optimization PARSE[-I//usr/include, -I/home/pradelle/work/pca-ocr-db-autodec/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -I../include, -I/home/pradelle/work/pca-ocr-db-autodec/runtime/codelet/ocr/inc, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DOCR_TARGET, --c99, -D__RSTREAM_CC__, ../src/hpcg_precond_fissioned.c] (978ms)
 * Optimization SSA (197ms)
 * Optimization CCP (90ms)
 * Optimization DCE (36ms)
 * Optimization Raise[-liftnone, -stacktrace] (1725ms)
 *   Optimization CCP (23ms)
 *   Optimization DCE (8ms)
 *   Optimization DeSSA (46ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (6ms)
 *   Optimization DeSSA (19ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (16ms)
 *   Optimization DeSSA (27ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (7ms)
 * Optimization PolyhedralMapperNew[-stacktrace, no-simplify-loop, no-spatial-layout, stacktrace=true, scal-dep, threadf=reduce_create=true, as=scopes, logfile=hpcg_precond_fissioned-mapper.log] (9935ms)
 * Optimization Lower[-stacktrace] (1663ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (4ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (4ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (4ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization InsertMapperAnnotations (4ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (2ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
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
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization DCE (20ms)
 * Optimization SyntaxPrepass (40ms)
 * Optimization OSR (67ms)
 * Optimization DCE (31ms)
 * Optimization DeSSA (480ms)
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rstream_ocr.h>
#include <ocr.h>
#define ENABLE_EXTENSION_LEGACY
#include <ocr-legacy.h>
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_49;
union __args_mapped_step1_noGVars11_50;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_51;
union __args_mapped_step1_noGVars13_52;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_53;
union __args_mapped_step1_noGVars16_54;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_55;
union __args_mapped_step1_noGVars19_56;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_57;
union __args_mapped_step1_noGVars21_58;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_59;
union __args_mapped_step1_noGVars_main_60;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_61;
union __args_mapped_step5_dpbr_noGVars15_62;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_63;
union __args_mapped_step5_dpbr_noGVars17_64;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_65;
union __args_mapped_step5_dpbr_noGVars20_66;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_67;
union __args_mapped_step5_dpbr_noGVars22_68;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_69;
union __args_mapped_step5_dpbr_noGVars24_70;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_71;
union __args_mapped_step5_dpbr_noGVars27_72;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_73;
union __args_mapped_step5_dpbr_noGVars29_74;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_75;
union __args_mapped_step5_dpbr_noGVars_main_76;
union ocrGuidUnion_t;
struct __anon_49 {
   int const* nPerLvl_l;
   int* _p;
   int* _p_1;
   int* _p_2;
   double* _p_3;
   int* _p_4;
   double* normr;
   double*restrict* r_l;
   double* Ap_l;
   int*restrict* nnzPerRow_l;
   double* b_l;
   int (*restrict* colInds_l)[27];
   double (*restrict* A_l)[27];
   double* p_l;
   double* x;
};
union __args_mapped_step1_noGVars11_50 {
   struct __anon_49 data;
   double padding[15];
};
struct __anon_51 {
   double* p_l;
   double* x;
   int* _p;
   double* normr;
   double*restrict* r_l;
   double* Ap_l;
   int*restrict* nnzPerRow_l;
   double* b_l;
   int (*restrict* colInds_l)[27];
   double (*restrict* A_l)[27];
   double* _p_1;
   int* _p_2;
   int* _p_3;
   int* _p_4;
   long IT0;
};
union __args_mapped_step1_noGVars13_52 {
   struct __anon_51 data;
   double padding[15];
};
struct __anon_53 {
   double* Ap_l;
   int*restrict* nnzPerRow_l;
   double*restrict* r_l;
   double* b_l;
   int* _p;
   int* _p_1;
   int (*restrict* colInds_l)[27];
   double (*restrict* A_l)[27];
   double* p_l;
   double* normr;
   double* _p_2;
   int* _p_3;
   long IT0;
};
union __args_mapped_step1_noGVars16_54 {
   struct __anon_53 data;
   double padding[13];
};
struct __anon_55 {
   double*restrict* r_l;
   int* _p;
   double* _p_1;
   double* normr;
   long IT0;
};
union __args_mapped_step1_noGVars19_56 {
   struct __anon_55 data;
   double padding[5];
};
struct __anon_57 {
   double* normr;
   double* _p;
};
union __args_mapped_step1_noGVars21_58 {
   struct __anon_57 data;
   double padding[2];
};
struct __anon_59 {
   int const* nPerLvl_l;
   double* normr;
   double*restrict* r_l;
   double* Ap_l;
   int*restrict* nnzPerRow_l;
   double* b_l;
   int (*restrict* colInds_l)[27];
   double (*restrict* A_l)[27];
   double* p_l;
   double* x;
};
union __args_mapped_step1_noGVars_main_60 {
   struct __anon_59 data;
   double padding[10];
};
struct __anon_61 {
   int const* nPerLvl_l;
   int* _p;
   double* _p_1;
   int* _p_2;
   int* _p_3;
   int* _p_4;
   double* _p_5;
   int* _p_6;
   double* normr;
   double*restrict* r_l;
   double* x;
   double* p_l;
   double* Ap_l;
   double* pAp;
   double* alpha;
   double* rtz;
   int*restrict* nnzPerRow_l;
   int (*restrict* colInds_l)[27];
   double (*restrict* A_l)[27];
   double* _p_7;
   double* _p_8;
};
union __args_mapped_step5_dpbr_noGVars15_62 {
   struct __anon_61 data;
   double padding[21];
};
struct __anon_63 {
   double* Ap_l;
   int*restrict* nnzPerRow_l;
   int* _p;
   int (*restrict* colInds_l)[27];
   double (*restrict* A_l)[27];
   double* p_l;
   double* normr;
   double*restrict* r_l;
   double* x;
   double* pAp;
   double* alpha;
   double* rtz;
   double* _p_1;
   int* _p_2;
   int* _p_3;
   double* _p_4;
   int* _p_5;
   double* _p_6;
   double* _p_7;
   int* _p_8;
   long IT0;
};
union __args_mapped_step5_dpbr_noGVars17_64 {
   struct __anon_63 data;
   double padding[21];
};
struct __anon_65 {
   double* p_l;
   double* Ap_l;
   int* _p;
   double* _p_1;
   double* normr;
   double*restrict* r_l;
   double* x;
   double* pAp;
   double* alpha;
   double* rtz;
   double* _p_2;
   int* _p_3;
   int* _p_4;
   double* _p_5;
   int* _p_6;
   double* _p_7;
   long IT0;
};
union __args_mapped_step5_dpbr_noGVars20_66 {
   struct __anon_65 data;
   double padding[17];
};
struct __anon_67 {
   double* pAp;
   double* alpha;
   double* rtz;
   double* _p;
   double* _p_1;
   double* _p_2;
   double* normr;
   double*restrict* r_l;
   double* x;
   double* p_l;
   double* Ap_l;
   double* _p_3;
   int* _p_4;
   int* _p_5;
   int* _p_6;
};
union __args_mapped_step5_dpbr_noGVars22_68 {
   struct __anon_67 data;
   double padding[15];
};
struct __anon_69 {
   double* x;
   double* p_l;
   double*restrict* r_l;
   double* Ap_l;
   int* _p;
   double* _p_1;
   int* _p_2;
   double* _p_3;
   double* normr;
   double* _p_4;
   int* _p_5;
   long IT0;
};
union __args_mapped_step5_dpbr_noGVars24_70 {
   struct __anon_69 data;
   double padding[12];
};
struct __anon_71 {
   double*restrict* r_l;
   int* _p;
   double* _p_1;
   double* normr;
   long IT0;
};
union __args_mapped_step5_dpbr_noGVars27_72 {
   struct __anon_71 data;
   double padding[5];
};
struct __anon_73 {
   double* normr;
   double* _p;
};
union __args_mapped_step5_dpbr_noGVars29_74 {
   struct __anon_73 data;
   double padding[2];
};
struct __anon_75 {
   int const* nPerLvl_l;
   double* normr;
   double*restrict* r_l;
   double* x;
   double* p_l;
   double* Ap_l;
   double* pAp;
   double* alpha;
   double* rtz;
   int*restrict* nnzPerRow_l;
   int (*restrict* colInds_l)[27];
   double (*restrict* A_l)[27];
};
union __args_mapped_step5_dpbr_noGVars_main_76 {
   struct __anon_75 data;
   double padding[12];
};
/*
 * Forward declarations of functions
 */
int fequal(double, double);
static void readMatrix(FILE* fd, int nbRows, int nbCols, int nnz, int*
   nnzPerRow, int (* colInds)[27], double (* A)[27], double* A_diag);
static void readData(char const* file);
void initialize_once(void);
void initialize(void);
void CopyVector(double const* src, double* dst, int const n);
void ComputeWAXPBY(double const alpha, double* x, double const beta, double* y,
    double* w, int const n);
void ComputeDotProduct(int const n, double* x, double* y, double* result);
void spmvBB_img(int* colInds, double* A, double* x, double* y);
void spmvBB(int* colInds, double* A, double* x, double* y);
void ComputeSPMV(int* nnzPerRow, int (* colInds)[27], double (* A)[27], double*
    x, double* y, int n);
void symgsBB_img(int* colInds, double* A, double* x, double* sum);
void symgsBB(int* colInds, double* A, double* x, double* sum);
void ComputeSYMGS(int* nnzPerRow, int (* colInds)[27], double (* A)[27], double
   * A_diag, double* x, double* r, int n);
void ComputeSYMGS_mapped(int* nnzPerRow, int (* colInds)[27], double (* A)[27],
    double* A_diag, double* x, double* r, int n);
void restrictionBB_img(int i, double* Axf, double* r_lvl, double* r_lvl1, int*
   f2c);
void restrictionBB(int i, double* Axf, double* r_lvl, double* r_lvl1, int* f2c)
   ;
void ComputeRestriction(int lvl, double* Axf, double*restrict* r, int n, int*
   f2c);
void prolongationBB_img(int i, double* z_lvl, double* z_lvl1, int* f2c);
void prolongationBB(int i, double* z_lvl, double* z_lvl1, int* f2c);
void ComputeProlongation(int lvl, double*restrict* z, int n, int* f2c);
void ComputeMG_mapped();
void mapped_step2(int i);
void step2(int i);
void mapped_step3(int i);
void step3(int i);
void ComputeMG();
void hpcg_mapped(double* x, double* normr, double* normr0);
void mapped_step1(double* x, double* normr);
void step1(double* x, double* normr);
void mapped_step4(double* beta, double* rtz, double oldrtz);
void step4(double* beta, double* rtz, double oldrtz);
void mapped_step5(double* alpha, double rtz, double* pAp, double* x, double*
   normr);
void step5(double* alpha, double rtz, double* pAp, double* x, double* normr);
void hpcg(double* x, double* normr, double* normr0);
void kernel(void);
void show(void);
int check(void);
double flops_per_trial(void);
static void spmvBB_img_1(int* colInds, double* A, double* x, double* y);
static void spmvBB_img_2(int* colInds, double* A, double* x, double* y);
static void restrictionBB_img_1(int i, double* Axf, double* r_lvl, double*
   r_lvl1, int* f2c);
static void restrictionBB_img_2(int i, double* Axf, double* r_lvl, double*
   r_lvl1, int* f2c);
static void prolongationBB_img_1(int i, double* z_lvl, double* z_lvl1, int* f2c
   );
static void prolongationBB_img_2(int i, double* z_lvl, double* z_lvl1, int* f2c
   );
static void mapped_step1_1(double* x, double* normr);
static void mapped_step1_2(double* x, double* normr);
static void mapped_step5_1(double* alpha, double rtz, double* pAp, double* x,
   double* normr);
static void mapped_step5_2(double* alpha, double rtz, double* pAp, double* x,
   double* normr);
static void mapped_step5_dpbr(double* alpha, double* rtz, double* pAp, double*
   x, double* normr);
void mapped_step1_noGVars(double* x, double* normr, int const* nPerLvl_l,
   double* p_l, double*restrict* r_l, double (*restrict* A_l)[27], double* Ap_l
   , int*restrict* nnzPerRow_l, int (*restrict* colInds_l)[27], double* b_l);
static ocrGuid_t mapped_step1_noGVars11(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
static ocrGuid_t mapped_step1_noGVars13(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
static ocrGuid_t mapped_step1_noGVars16(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
static ocrGuid_t mapped_step1_noGVars19(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
static ocrGuid_t mapped_step1_noGVars21(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
ocrGuid_t rocrAlloc(void**, unsigned long);
unsigned long mapped_step1_noGVars21_count_4(long* args);
unsigned long mapped_step1_noGVars19_count_3(long* args);
unsigned long mapped_step1_noGVars16_count_2(long* args);
unsigned long mapped_step1_noGVars13_count_1(long* args);
static ocrGuid_t mapped_step1_noGVars_main(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars_async(void* event, double* x, double* normr, int const
   * nPerLvl_l, double* p_l, double*restrict* r_l, double (*restrict* A_l)[27],
    double* Ap_l, int*restrict* nnzPerRow_l, int (*restrict* colInds_l)[27],
   double* b_l);
void mapped_step5_dpbr_noGVars(double* alpha, double* rtz, double* pAp, double*
    x, double* normr, double* p_l, double (*restrict* A_l)[27], double* Ap_l,
   int const* nPerLvl_l, int*restrict* nnzPerRow_l, int (*restrict* colInds_l)[
   27], double*restrict* r_l);
static ocrGuid_t mapped_step5_dpbr_noGVars15(unsigned int paramc, unsigned long
   * paramv, unsigned int depc, ocrEdtDep_t* depv);
static ocrGuid_t mapped_step5_dpbr_noGVars17(unsigned int paramc, unsigned long
   * paramv, unsigned int depc, ocrEdtDep_t* depv);
static ocrGuid_t mapped_step5_dpbr_noGVars20(unsigned int paramc, unsigned long
   * paramv, unsigned int depc, ocrEdtDep_t* depv);
static ocrGuid_t mapped_step5_dpbr_noGVars22(unsigned int paramc, unsigned long
   * paramv, unsigned int depc, ocrEdtDep_t* depv);
static ocrGuid_t mapped_step5_dpbr_noGVars24(unsigned int paramc, unsigned long
   * paramv, unsigned int depc, ocrEdtDep_t* depv);
static ocrGuid_t mapped_step5_dpbr_noGVars27(unsigned int paramc, unsigned long
   * paramv, unsigned int depc, ocrEdtDep_t* depv);
static ocrGuid_t mapped_step5_dpbr_noGVars29(unsigned int paramc, unsigned long
   * paramv, unsigned int depc, ocrEdtDep_t* depv);
unsigned long mapped_step5_dpbr_noGVars29_count_6(long* args);
unsigned long mapped_step5_dpbr_noGVars27_count_5(long* args);
unsigned long mapped_step5_dpbr_noGVars24_count_4(long* args);
unsigned long mapped_step5_dpbr_noGVars22_count_3(long* args);
unsigned long mapped_step5_dpbr_noGVars20_count_2(long* args);
unsigned long mapped_step5_dpbr_noGVars17_count_1(long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_async(void* event, double* alpha, double rtz, double* pAp,
   double* x, double* normr);
/*
 * Forward declarations of global variables
 */
#ifndef GPU_TARGET
extern double sqrt(double);
#endif
/* This can be redefined to be the appropriate 'inline' keyword */
#ifndef RSTREAM_INLINE
#define RSTREAM_INLINE
#endif
#ifdef GPU_TARGET
    #define __mins_64(x,y) \
 min((long long)(x),(long long)(y))
#else
static RSTREAM_INLINE long __mins_64(long x, long y) {
   return x < y ? x : y;
}
#endif
#ifdef GPU_TARGET
    #define __maxs_64(x,y) \
 max((long long)(x),(long long)(y))
#else
static RSTREAM_INLINE long __maxs_64(long x, long y) {
   return x > y ? x : y;
}
#endif
/*
 * Definitions of global variables
 */
static int*restrict* nnzPerRow;
static int (*restrict* colInds)[27];
static double (*restrict* A)[27];
static double*restrict* A_diag;
static double* b;
static double*restrict* z;
static double*restrict* r;
static double*restrict* Axf;
static double normr;
static double normr0;
static int*restrict* f2c;
static double* x;
static double* x_check;
static int const nPerLvl[4] = {1124864, 140608, 17576, 2197};
static int const nnzPerLvl[4] = {29791000, 3652264, 438976, 50653};
static int const blockSzPerLvl[4] = {104, 52, 26, 13};
static double* p;
static double* Ap;
static double normr_check;
static double normr0_check;
int nb_samples;
char const* function_name = "hpcg";
/*
 * Definitions of functions
 */
/*
 * ../src/hpcg_precond_fissioned.c:95.13
 */
static void readMatrix(FILE* fd, int nbRows, int nbCols, int nnz, int*
   nnzPerRow_1, int (* colInds_1)[27], double (* A_1)[27], double* A_diag_1)
{
   unsigned int mRows;
   unsigned int mCols;
   unsigned int mNnz;
   unsigned char nranges;
   int rangeVals[28];
   int cond;
   int cond_1;
   unsigned int i;
   unsigned int i_1;
   mRows = 0u;
   mCols = 0u;
   mNnz = 0u;
   fread(&mRows, 4ul, 1ul, fd);
   fread(&mCols, 4ul, 1ul, fd);
   fread(&mNnz, 4ul, 1ul, fd);
   if (mRows != (unsigned int)nbRows) {
      cond = 1;
   } else {
      cond = mCols != (unsigned int)nbCols;
   }
   if (cond) {
      cond_1 = 1;
   } else {
      cond_1 = mNnz != (unsigned int)nnz;
   }
   if (cond_1) {
      fprintf(stderr, "Invalid data file format\n");
      abort();
   }
   for (i = 0u, i_1 = 4294967295u; i < mRows; i++, i_1++) {
      int cond_2;
      unsigned int j;
      if (fread(&nranges, 1ul, 1ul, fd) < 1ul) {
         fprintf(stderr, "Unexpected EOF\n");
         abort();
      }
      if ((int)nranges > 28) {
         cond_2 = (int)nranges != 255;
      } else {
         cond_2 = 0;
      }
      if (cond_2) {
         fprintf(stderr, "Unexpected number of ranges: %u\n", (int)nranges);
         abort();
      }
      if ((int)nranges != 255) {
         unsigned long _t1;
         unsigned char curNnz;
         unsigned char r_1;
         _t1 = fread(rangeVals, 4ul, (unsigned long)(2 * (int)nranges), fd);
         if (_t1 < (unsigned long)(2 * (int)nranges)) {
            fprintf(stderr, "Unexpected EOF\n");
            abort();
         }
         for (r_1 = 0u, curNnz = 0u; (int)r_1 < (int)nranges; ) {
            int _t2;
            int colInd;
            for (_t2 = rangeVals[2 * (int)r_1 + 1], colInd = rangeVals[2 * (int
                    )r_1]; colInd <= _t2; ) {
               colInds_1[i][curNnz] = colInd;
               if ((unsigned int)colInd == i) {
                  A_1[i][curNnz] = 26.0;
                  A_diag_1[i] = 26.0;
               } else {
                  A_1[i][curNnz] = -1.0;
               }
               colInd++;
               curNnz = (unsigned char)((int)curNnz + 1);
            }
            r_1 = (unsigned char)((int)r_1 + 1);
         }
         if ((int)curNnz > 27) {
            fprintf(stderr, "Unexpected number of non zeros: %d\n", (int)curNnz
               );
            abort();
         }
         nnzPerRow_1[i] = (int)curNnz;
      } else {
         unsigned char curNnz;
         if (i == 0u) {
            fprintf(stderr, "First line cannot be compressed in dataset\n");
            abort();
         }
         nnzPerRow_1[i] = nnzPerRow_1[i_1];
         for (curNnz = 0u; (int)curNnz < nnzPerRow_1[i]; ) {
            colInds_1[i][curNnz] = colInds_1[i_1][curNnz] + 1;
            if ((unsigned int)colInds_1[i][curNnz] == i) {
               A_1[i][curNnz] = 26.0;
               A_diag_1[i] = 26.0;
            } else {
               A_1[i][curNnz] = -1.0;
            }
            curNnz = (unsigned char)((int)curNnz + 1);
         }
      }
      for (j = (unsigned int)nnzPerRow_1[i]; j < 27u; j++) {
         colInds_1[i][j] = 2147483647;
         A_1[i][j] = (0.0/0.0) /* NaN */;
      }
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:183.13
 */
static void readData(char const* file)
{
   FILE* _t1;
   unsigned long _t2;
   _t1 = fopen(file, "r");
   if (_t1 == ((FILE*)0l)) {
      fprintf(stderr, "Cannot find data file %s\n", file);
      abort();
   }
   readMatrix(_t1, *(int*)(nPerLvl + 0), *(int*)(nPerLvl + 0), *(int*)(
      nnzPerLvl + 0), *(int**)(nnzPerRow + 0), *(int (**)[27])(colInds + 0), *(
      double (**)[27])(A + 0), *(double**)(A_diag + 0));
   _t2 = fread(b, 8ul, (unsigned long)*(int*)(nPerLvl + 0), _t1);
   if (_t2 < (unsigned long)*(int*)(nPerLvl + 0)) {
LL4:
      fprintf(stderr, "%s: invalid file format\n", file);
      abort();
   } else {
      unsigned long _t3;
      _t3 = fread(x, 8ul, (unsigned long)*(int*)(nPerLvl + 0), _t1);
      if (_t3 < (unsigned long)*(int*)(nPerLvl + 0))
         goto LL4; else {
         int i;
         int i_1;
         for (i = 1, i_1 = 0; i < 4; i++, i_1++) {
            int _t4;
            int _t5;
            int _t6;
            int _t7;
            int _t8;
            int izc;
            int izc_1;
            int izc_2;
            readMatrix(_t1, *(int*)(nPerLvl + i), *(int*)(nPerLvl + i), *(int*)
               (nnzPerLvl + i), *(int**)(nnzPerRow + i), *(int (**)[27])(
               colInds + i), *(double (**)[27])(A + i), *(double**)(A_diag + i)
               );
            for (_t4 = *(int*)(blockSzPerLvl + i_1),
                 _t6 = _t4 << 1,
                 _t7 = _t6 * _t4,
                 _t5 = *(int*)(blockSzPerLvl + i),
                 _t8 = _t5 * _t5,
                 izc = 0,
                 izc_1 = 0,
                 izc_2 = 0;
                 izc < _t5; izc++, izc_2 += _t8, izc_1 += _t7) {
               int iyc;
               int iyc_1;
               int iyc_2;
               for (iyc = 0, iyc_1 = izc_1, iyc_2 = izc_2; iyc < _t5; iyc++,
                       iyc_2 += _t5, iyc_1 += _t6) {
                  int ixc;
                  int ixc_1;
                  int ixc_2;
                  for (ixc = 0, ixc_2 = iyc_2, ixc_1 = iyc_1; ixc < _t5; ixc++,
                           ixc_2++, ixc_1 += 2) {
                     (*(int**)(f2c + i))[ixc_2] = ixc_1;
                  }
               }
            }
         }
         fclose(_t1);
      }
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:232.6
 */
void initialize_once(void)
{
   int i;
   for (nnzPerRow = (int*restrict*)malloc(32ul),
        colInds = (int (*restrict*)[27])malloc(32ul),
        A = (double (*restrict*)[27])malloc(32ul),
        A_diag = (double*restrict*)malloc(32ul),
        z = (double*restrict*)malloc(32ul),
        r = (double*restrict*)malloc(32ul),
        Axf = (double*restrict*)malloc(32ul),
        f2c = (int*restrict*)malloc(32ul),
        i = 0;
        i < 4; i++) {
      int*restrict* _t1;
      int* _t2;
      int (*restrict* _t3)[27];
      int (* _t4)[27];
      double (*restrict* _t5)[27];
      double (* _t6)[27];
      double*restrict* _t7;
      double* _t8;
      double*restrict* _t9;
      double* _t10;
      double*restrict* _t11;
      double* _t12;
      double*restrict* _t13;
      double* _t14;
      int*restrict* _t15;
      int* _t16;
      _t1 = nnzPerRow;
      _t2 = (int*)malloc(4499456ul);
      _t1[i] = _t2;
      _t3 = colInds;
      _t4 = (int (*)[27])malloc(121485312ul);
      _t3[i] = _t4;
      _t5 = A;
      _t6 = (double (*)[27])malloc(242970624ul);
      _t5[i] = _t6;
      _t7 = A_diag;
      _t8 = (double*)malloc(8998912ul);
      _t7[i] = _t8;
      _t9 = z;
      _t10 = (double*)malloc(8998912ul);
      _t9[i] = _t10;
      _t11 = r;
      _t12 = (double*)malloc(8998912ul);
      _t11[i] = _t12;
      _t13 = Axf;
      _t14 = (double*)malloc(8998912ul);
      _t13[i] = _t14;
      _t15 = f2c;
      _t16 = (int*)malloc(4499456ul);
      _t15[i] = _t16;
   }
   b = (double*)malloc(8998912ul);
   x = (double*)malloc(8998912ul);
   x_check = (double*)malloc(8998912ul);
   p = (double*)malloc(8998912ul);
   Ap = (double*)malloc(8998912ul);
   readData("./data/hpcg_cg_compressed.dat");
   memcpy(x_check, x, 8998912ul);
   nb_samples = *(int*)(nPerLvl + 0);
}

/*
 * ../src/hpcg_precond_fissioned.c:270.6
 */
void initialize(void)
{
   int i;
   int i_1;
   normr = (0.0/0.0) /* NaN */;
   normr0 = (0.0/0.0) /* NaN */;
   memcpy(x, x_check, 8998912ul);
   for (i = 0; i < 4; i++) {
      int j;
      for (j = 0; j < 1124864; j++) {
         (*(double**)(r + i))[j] = (0.0/0.0) /* NaN */;
         (*(double**)(Axf + i))[j] = (0.0/0.0) /* NaN */;
         (*(double**)(z + i))[j] = (0.0/0.0) /* NaN */;
      }
   }
   for (i_1 = 0; i_1 < 1124864; i_1++) {
      double* _t1;
      _t1 = p;
      Ap[i_1] = (0.0/0.0) /* NaN */;
      _t1[i_1] = (0.0/0.0) /* NaN */;
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:293.6
 */
void CopyVector(double const* src, double* dst, int const n)
{
   int i;
   for (i = 0; i < 1124864; i++) {
      if (i < n) {
         dst[i] = *(double*)(src + i);
      }
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:303.6
 */
void ComputeWAXPBY(double const alpha, double* x_1, double const beta, double*
   y, double* w, int const n)
{
   if (alpha == 1.0) {
      int i;
      for (i = 0; i < 1124864; i++) {
         if (i < n) {
            w[i] = x_1[i] + beta * y[i];
         }
      }
   } else {
      if (beta == 1.0) {
         int i;
         for (i = 0; i < 1124864; i++) {
            if (i < n) {
               w[i] = alpha * x_1[i] + y[i];
            }
         }
      } else {
         int i;
         for (i = 0; i < 1124864; i++) {
            if (i < n) {
               w[i] = alpha * x_1[i] + beta * y[i];
            }
         }
      }
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:329.6
 */
void ComputeDotProduct(int const n, double* x_2, double* y, double* result)
{
   double local_result;
   int i;
   for (local_result = 0.0, i = 0; i < 1124864; i++) {
      if (i < n) {
         local_result = local_result + x_2[i] * y[i];
      }
   }
   *result = local_result;
}

/*
 * ../src/hpcg_precond_fissioned.c:351.6
 */
void spmvBB_img(int* colInds_2, double* A_2, double* x_3, double* y)
{
   int k;
   *y = *A_2 * (double)*colInds_2;
   for (k = 0; k < 1124864; k++) {
      *y = *y + x_3[k];
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:359.6
 */
void spmvBB(int* colInds_3, double* A_3, double* x_4, double* y)
{
   *y = *y + *A_3 * x_4[*colInds_3];
}

/*
 * ../src/hpcg_precond_fissioned.c:365.6
 */
void ComputeSPMV(int* nnzPerRow_2, int (* colInds_4)[27], double (* A_4)[27],
   double* x_5, double* y, int n)
{
   int i;
   for (i = 0; i < 1124864; i++) {
      if (i < n) {
         int j;
         y[i] = 0.0;
         for (j = 0; j < 27; j++) {
            if (j < nnzPerRow_2[i]) {
               spmvBB(colInds_4[i] + j, A_4[i] + j, x_5, y + i);
            }
         }
      }
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:383.6
 */
void symgsBB_img(int* colInds_5, double* A_5, double* x_6, double* sum)
{
   int k;
   *sum = *sum - (*A_5 + (double)*colInds_5);
   for (k = 0; k < 1124864; k++) {
      *sum = *sum - x_6[k];
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:392.6
 */
void symgsBB(int* colInds_6, double* A_6, double* x_7, double* sum)
{
   *sum = *sum - *A_6 * x_7[*colInds_6];
}

/*
 * ../src/hpcg_precond_fissioned.c:397.6
 */
void ComputeSYMGS(int* nnzPerRow_3, int (* colInds_7)[27], double (* A_7)[27],
   double* A_diag_2, double* x_8, double* r_2, int n)
{
   double sum;
   double sum_1;
   int i;
   int i_1;
   for (i = 0; i < 1124864; i++) {
      if (i < n) {
         int j;
         for (sum = r_2[i], j = 0; j < 27; j++) {
            if (j < nnzPerRow_3[i]) {
               symgsBB(colInds_7[i] + j, A_7[i] + j, x_8, &sum);
            }
         }
         sum = sum + x_8[i] * A_diag_2[i];
         x_8[i] = sum / A_diag_2[i];
      }
   }
   for (i_1 = 1124864; i_1 >= 0; i_1--) {
      if (i_1 < n) {
         int j;
         for (sum_1 = r_2[i_1], j = 0; j < 27; j++) {
            if (j < nnzPerRow_3[i_1]) {
               symgsBB(colInds_7[i_1] + j, A_7[i_1] + j, x_8, &sum_1);
            }
         }
         sum_1 = sum_1 + x_8[i_1] * A_diag_2[i_1];
         x_8[i_1] = sum_1 / A_diag_2[i_1];
      }
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:432.6
 */
void ComputeSYMGS_mapped(int* nnzPerRow_4, int (* colInds_8)[27], double (* A_8
   )[27], double* A_diag_3, double* x_9, double* r_3, int n)
{
   ComputeSYMGS(nnzPerRow_4, colInds_8, A_8, A_diag_3, x_9, r_3, n);
}

/*
 * ../src/hpcg_precond_fissioned.c:439.6
 */
void restrictionBB_img(int i, double* Axf_1, double* r_lvl, double* r_lvl1, int
   * f2c_1)
{
   double* _t1;
   int* _t2;
   int j;
   for (_t2 = f2c_1 + i, _t1 = r_lvl1 + i, j = 0; j < 1124864; j++) {
      *_t1 = r_lvl[j] + Axf_1[j] + (double)*_t2;
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:449.6
 */
void restrictionBB(int i, double* Axf_2, double* r_lvl, double* r_lvl1, int*
   f2c_2)
{
   r_lvl1[i] = r_lvl[f2c_2[i]] - Axf_2[f2c_2[i]];
}

/*
 * ../src/hpcg_precond_fissioned.c:457.6
 */
void ComputeRestriction(int lvl, double* Axf_3, double*restrict* r_4, int n,
   int* f2c_3)
{
   int i;
   for (i = 0; i < 1124864; i++) {
      if (i < n) {
         restrictionBB(i, Axf_3, *(double**)(r_4 + lvl), *(double**)(r_4 + (lvl
            + 1)), f2c_3);
      }
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:470.6
 */
void prolongationBB_img(int i, double* z_lvl, double* z_lvl1, int* f2c_4)
{
   int* _t1;
   int j;
   for (_t1 = f2c_4 + i, j = 0; j < 1124864; j++) {
      z_lvl[j] = z_lvl[j] + ((double)*_t1 + z_lvl1[i]);
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:479.6
 */
void prolongationBB(int i, double* z_lvl, double* z_lvl1, int* f2c_5)
{
   z_lvl[f2c_5[i]] = z_lvl[f2c_5[i]] + z_lvl1[i];
}

/*
 * ../src/hpcg_precond_fissioned.c:484.6
 */
void ComputeProlongation(int lvl, double*restrict* z_1, int n, int* f2c_6)
{
   int i;
   for (i = 0; i < 1124864; i++) {
      if (i < n) {
         prolongationBB(i, *(double**)(z_1 + lvl), *(double**)(z_1 + (lvl + 1))
            , f2c_6);
      }
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:497.6
 */
void ComputeMG_mapped()
{
   int i;
   int j;
   int i_1;
   for (i = 0; i < 3; i++) {
      int j_1;
      int j_2;
      for (j_1 = 0; j_1 < 1124864; j_1++) {
         if (j_1 < *(int*)(nPerLvl + i)) {
            (*(double**)(z + i))[j_1] = 0.0;
         }
      }
      for (j_2 = 0; j_2 < 1; j_2++) {
         ComputeSYMGS_mapped(*(int**)(nnzPerRow + i), *(int (**)[27])(colInds +
            i), *(double (**)[27])(A + i), *(double**)(A_diag + i), *(double**)
            (z + i), *(double**)(r + i), *(int*)(nPerLvl + i));
      }
      mapped_step2(i);
   }
   for (j = 0; j < 1124864; j++) {
      if (j < *(int*)(nPerLvl + 3)) {
         (*(double**)(z + 3))[j] = 0.0;
      }
   }
   ComputeSYMGS_mapped(*(int**)(nnzPerRow + 3), *(int (**)[27])(colInds + 3), *
      (double (**)[27])(A + 3), *(double**)(A_diag + 3), *(double**)(z + 3), *(
      double**)(r + 3), *(int*)(nPerLvl + 3));
   for (i_1 = 2; i_1 >= 0; i_1--) {
      int j_3;
      mapped_step3(i_1);
      for (j_3 = 0; j_3 < 1; j_3++) {
         ComputeSYMGS_mapped(*(int**)(nnzPerRow + i_1), *(int (**)[27])(colInds
            + i_1), *(double (**)[27])(A + i_1), *(double**)(A_diag + i_1), *(
            double**)(z + i_1), *(double**)(r + i_1), *(int*)(nPerLvl + i_1));
      }
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:669.6
 */
void mapped_step2(int i)
{
   step2(i);
}

/*
 * ../src/hpcg_precond_fissioned.c:699.6
 */
void step2(int i)
{
   ComputeSPMV(*(int**)(nnzPerRow + i), *(int (**)[27])(colInds + i), *(double
      (**)[27])(A + i), *(double**)(z + i), *(double**)(Axf + (i + 1)), *(int*)
      (nPerLvl + i));
   ComputeRestriction(i, *(double**)(Axf + (i + 1)), r, *(int*)(nPerLvl + (i + 1
      )), *(int**)(f2c + (i + 1)));
}

/*
 * ../src/hpcg_precond_fissioned.c:674.6
 */
void mapped_step3(int i)
{
   step3(i);
}

/*
 * ../src/hpcg_precond_fissioned.c:705.6
 */
void step3(int i)
{
   ComputeProlongation(i, z, *(int*)(nPerLvl + (i + 1)), *(int**)(f2c + (i + 1)
      ));
}

/*
 * ../src/hpcg_precond_fissioned.c:537.6
 */
void ComputeMG()
{
   int i;
   int j;
   int i_1;
   for (i = 0; i < 3; i++) {
      int j_1;
      int j_2;
      for (j_1 = 0; j_1 < 1124864; j_1++) {
         if (j_1 < *(int*)(nPerLvl + i)) {
            (*(double**)(z + i))[j_1] = 0.0;
         }
      }
      for (j_2 = 0; j_2 < 1; j_2++) {
         ComputeSYMGS(*(int**)(nnzPerRow + i), *(int (**)[27])(colInds + i), *(
            double (**)[27])(A + i), *(double**)(A_diag + i), *(double**)(z + i
            ), *(double**)(r + i), *(int*)(nPerLvl + i));
      }
      step2(i);
   }
   for (j = 0; j < 1124864; j++) {
      if (j < *(int*)(nPerLvl + 3)) {
         (*(double**)(z + 3))[j] = 0.0;
      }
   }
   ComputeSYMGS(*(int**)(nnzPerRow + 3), *(int (**)[27])(colInds + 3), *(double
       (**)[27])(A + 3), *(double**)(A_diag + 3), *(double**)(z + 3), *(double*
      *)(r + 3), *(int*)(nPerLvl + 3));
   for (i_1 = 2; i_1 >= 0; i_1--) {
      int j_3;
      step3(i_1);
      for (j_3 = 0; j_3 < 1; j_3++) {
         ComputeSYMGS(*(int**)(nnzPerRow + i_1), *(int (**)[27])(colInds + i_1)
            , *(double (**)[27])(A + i_1), *(double**)(A_diag + i_1), *(double*
            *)(z + i_1), *(double**)(r + i_1), *(int*)(nPerLvl + i_1));
      }
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:578.6
 */
void hpcg_mapped(double* x_10, double* normr_1, double* normr0_1)
{
   double rtz;
   double alpha;
   double beta;
   double pAp;
   int k;
   rtz = 0.0;
   alpha = 0.0;
   beta = 0.0;
   pAp = 0.0;
   *normr_1 = 0.0;
   mapped_step1(x_10, normr_1);
   *normr_1 = sqrt(*normr_1);
   *normr0_1 = *normr_1;
   for (k = 1; k <= 50; k++) {
      ComputeMG_mapped();
      if (k == 1) {
         CopyVector(*(double**)(z + 0), p, (int const)*(int*)(nPerLvl + 0));
         ComputeDotProduct((int const)*(int*)(nPerLvl + 0), *(double**)(r + 0),
             *(double**)(z + 0), &rtz);
      } else {
         mapped_step4(&beta, &rtz, rtz);
      }
      mapped_step5(&alpha, rtz, &pAp, x_10, normr_1);
      *normr_1 = sqrt(*normr_1);
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:660.6
 */
void mapped_step1(double* x_11, double* normr_2)
{
   ocrGuid_t outEvt;
   union __args_mapped_step1_noGVars_main_60* allArgs;
   ocrGuid_t _t1;
   union __args_mapped_step1_noGVars_main_60* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, (int)0l);
   _t3 = rocrInit(6u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, mapped_step1_noGVars_main, 5u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars11, 0u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars13, 1u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars16, 2u, (unsigned long)138l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars19, 3u, (unsigned long)9l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars21, 4u, (unsigned long)1l, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 80ul);
   _t2 = allArgs;
   _t2->data.nPerLvl_l = nPerLvl;
   _t2->data.normr = normr_2;
   _t2->data.r_l = r;
   _t2->data.Ap_l = Ap;
   _t2->data.nnzPerRow_l = nnzPerRow;
   _t2->data.b_l = b;
   _t2->data.colInds_l = colInds;
   _t2->data.A_l = A;
   _t2->data.p_l = p;
   _t2->data.x = x_11;
   rocrExecute(_t3, mapped_step1_noGVars_main, _t1, &outEvt);
   rocrExit(_t3);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

/*
 * ../src/hpcg_precond_fissioned.c:691.6
 */
void step1(double* x_12, double* normr_3)
{
   CopyVector(x_12, p, (int const)*(int*)(nPerLvl + 0));
   ComputeSPMV(*(int**)(nnzPerRow + 0), *(int (**)[27])(colInds + 0), *(double
      (**)[27])(A + 0), p, Ap, *(int*)(nPerLvl + 0));
   ComputeWAXPBY((double const)1.0, b, (double const)-1.0, Ap, *(double**)(r + 0
      ), (int const)*(int*)(nPerLvl + 0));
   ComputeDotProduct((int const)*(int*)(nPerLvl + 0), *(double**)(r + 0), *(
      double**)(r + 0), normr_3);
}

/*
 * ../src/hpcg_precond_fissioned.c:679.6
 */
void mapped_step4(double* beta, double* rtz, double oldrtz)
{
   step4(beta, rtz, oldrtz);
}

/*
 * ../src/hpcg_precond_fissioned.c:710.6
 */
void step4(double* beta, double* rtz, double oldrtz)
{
   ComputeDotProduct((int const)*(int*)(nPerLvl + 0), *(double**)(r + 0), *(
      double**)(z + 0), rtz);
   *beta = *rtz / oldrtz;
   ComputeWAXPBY((double const)1.0, *(double**)(z + 0), (double const)*beta, p,
       p, (int const)*(int*)(nPerLvl + 0));
}

/*
 * ../src/hpcg_precond_fissioned.c:684.6
 */
void mapped_step5(double* alpha, double rtz, double* pAp, double* x_13, double*
    normr_4)
{
   double rtz_var;
   rtz_var = rtz;
   mapped_step5_dpbr(alpha, &rtz_var, pAp, x_13, normr_4);
}

/*
 * ../src/hpcg_precond_fissioned.c:717.6
 */
void step5(double* alpha, double rtz, double* pAp, double* x_14, double*
   normr_5)
{
   ComputeSPMV(*(int**)(nnzPerRow + 0), *(int (**)[27])(colInds + 0), *(double
      (**)[27])(A + 0), p, Ap, *(int*)(nPerLvl + 0));
   ComputeDotProduct((int const)*(int*)(nPerLvl + 0), p, Ap, pAp);
   *alpha = rtz / *pAp;
   ComputeWAXPBY((double const)1.0, x_14, (double const)*alpha, p, x_14, (int
      const)*(int*)(nPerLvl + 0));
   ComputeWAXPBY((double const)1.0, *(double**)(r + 0), (double const)- *alpha,
       Ap, *(double**)(r + 0), (int const)*(int*)(nPerLvl + 0));
   ComputeDotProduct((int const)*(int*)(nPerLvl + 0), *(double**)(r + 0), *(
      double**)(r + 0), normr_5);
}

/*
 * ../src/hpcg_precond_fissioned.c:618.6
 */
void hpcg(double* x_15, double* normr_6, double* normr0_2)
{
   double rtz;
   double alpha;
   double beta;
   double pAp;
   int k;
   rtz = 0.0;
   alpha = 0.0;
   beta = 0.0;
   pAp = 0.0;
   *normr_6 = 0.0;
   step1(x_15, normr_6);
   *normr_6 = sqrt(*normr_6);
   *normr0_2 = *normr_6;
   for (k = 1; k <= 50; k++) {
      ComputeMG();
      if (k == 1) {
         CopyVector(*(double**)(z + 0), p, (int const)*(int*)(nPerLvl + 0));
         ComputeDotProduct((int const)*(int*)(nPerLvl + 0), *(double**)(r + 0),
             *(double**)(z + 0), &rtz);
      } else {
         step4(&beta, &rtz, rtz);
      }
      step5(&alpha, rtz, &pAp, x_15, normr_6);
      *normr_6 = sqrt(*normr_6);
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:728.6
 */
void kernel(void)
{
   hpcg_mapped(x, &normr, &normr0);
}

/*
 * ../src/hpcg_precond_fissioned.c:733.6
 */
void show(void)
{
}

/*
 * ../src/hpcg_precond_fissioned.c:736.5
 */
int check(void)
{
   int check_result;
   hpcg(x_check, &normr_check, &normr0_check);
   printf("Computed residual: %f (%f / %f)\n", normr / normr0, normr, normr0);
   printf("Expected residual: %f\n", normr_check / normr0_check);
   if (!(fequal(normr0, normr0_check) != 0)) {
      check_result = 1;
   } else {
      if (!(fequal(normr, normr_check) != 0)) {
         check_result = 1;
      } else {
         int j;
         j = 0;
         while (1) {
            if (j < *(int*)(nPerLvl + 0)) {
               if (!(fequal(x[j], x_check[j]) != 0))
                  break;
               j++;
            } else
               goto LL8;
         }
         printf("Different x[%d]: %f vs. %f\n", j, x[j], x_check[j]);
         check_result = 1;
      }
   }
LL10:
   return check_result;
LL8:
   check_result = 0;
   goto LL10;
}

/*
 * ../src/hpcg_precond_fissioned.c:755.8
 */
double flops_per_trial(void)
{
   return 0.0;
}

/*
 * ../src/hpcg_precond_fissioned.c:351.6
 */
static void spmvBB_img_1(int* colInds_9, double* A_9, double* x_16, double* y)
{
   int k;
   *y = *A_9 * (double)*colInds_9;
   for (k = 0; k < 1124864; k++) {
      *y = *y + x_16[k];
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:351.6
 */
static void spmvBB_img_2(int* colInds_10, double* A_10, double* x_17, double* y
   )
{
   int k;
   *y = *A_10 * (double)*colInds_10;
   for (k = 0; k < 1124864; k++) {
      *y = *y + x_17[k];
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:439.6
 */
static void restrictionBB_img_1(int i, double* Axf_4, double* r_lvl, double*
   r_lvl1, int* f2c_7)
{
   int j;
   for (j = 0; j < 1124864; j++) {
      r_lvl1[i] = r_lvl[j] + Axf_4[j] + (double)f2c_7[i];
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:439.6
 */
static void restrictionBB_img_2(int i, double* Axf_5, double* r_lvl, double*
   r_lvl1, int* f2c_8)
{
   int j;
   for (j = 0; j < 1124864; j++) {
      r_lvl1[i] = r_lvl[j] + Axf_5[j] + (double)f2c_8[i];
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:470.6
 */
static void prolongationBB_img_1(int i, double* z_lvl, double* z_lvl1, int*
   f2c_9)
{
   int j;
   for (j = 0; j < 1124864; j++) {
      z_lvl[j] = z_lvl[j] + ((double)f2c_9[i] + z_lvl1[i]);
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:470.6
 */
static void prolongationBB_img_2(int i, double* z_lvl, double* z_lvl1, int*
   f2c_10)
{
   int j;
   for (j = 0; j < 1124864; j++) {
      z_lvl[j] = z_lvl[j] + ((double)f2c_10[i] + z_lvl1[i]);
   }
}

/*
 * ../src/hpcg_precond_fissioned.c:660.6
 */
static void mapped_step1_1(double* x_18, double* normr_7)
{
   int const* _t1;
   int const* _t2;
   int const* _t3;
   double local_result;
   int _t4;
   double* _t5;
   int i;
   double* _t6;
   double (* _t7)[27];
   double* _t8;
   int _t9;
   int* _t10;
   int (* _t11)[27];
   int i_1;
   double* _t12;
   int _t13;
   double* _t14;
   double* _t15;
   int i_2;
   double* _t16;
   double* _t17;
   int _t18;
   int i_3;
   for (_t5 = p,
        _t3 = nPerLvl + 0,
        _t2 = nPerLvl + 0,
        _t1 = nPerLvl + 0,
        _t4 = *(int*)(nPerLvl + 0),
        i = 0;
        i < 1124864; i++) {
      if (i < _t4) {
         _t5[i] = x_18[i];
      }
   }
   for (_t11 = *(int (**)[27])(colInds + 0),
        _t10 = *(int**)(nnzPerRow + 0),
        _t9 = *(int*)_t2,
        _t8 = Ap,
        _t7 = *(double (**)[27])(A + 0),
        _t6 = p,
        i_1 = 0;
        i_1 < 1124864; i_1++) {
      if (i_1 < _t9) {
         int j;
         _t8[i_1] = 0.0;
         for (j = 0; j < 27; j++) {
            if (j < _t10[i_1]) {
               spmvBB(_t11[i_1] + j, _t7[i_1] + j, _t6, _t8 + i_1);
            }
         }
      }
   }
   for (_t15 = b,
        _t14 = Ap,
        _t13 = *(int*)_t1,
        _t12 = *(double**)(r + 0),
        i_2 = 0;
        i_2 < 1124864; i_2++) {
      if (i_2 < _t13) {
         _t12[i_2] = _t15[i_2] + -1.0 * _t14[i_2];
      }
   }
   for (_t18 = *(int*)_t3,
        _t17 = *(double**)(r + 0),
        _t16 = *(double**)(r + 0),
        local_result = 0.0,
        i_3 = 0;
        i_3 < 1124864; i_3++) {
      if (i_3 < _t18) {
         local_result = local_result + _t16[i_3] * _t17[i_3];
      }
   }
   *normr_7 = local_result;
}

/*
 * ../src/hpcg_precond_fissioned.c:660.6
 */
static void mapped_step1_2(double* x_19, double* normr_8)
{
   int const* _t1;
   int const* _t2;
   int const* _t3;
   double local_result;
   int _t4;
   double* _t5;
   int i;
   double* _t6;
   double (* _t7)[27];
   double* _t8;
   int _t9;
   int* _t10;
   int (* _t11)[27];
   int i_1;
   double* _t12;
   int _t13;
   double* _t14;
   double* _t15;
   int i_2;
   double* _t16;
   double* _t17;
   int _t18;
   int i_3;
   for (_t5 = p,
        _t3 = nPerLvl + 0,
        _t2 = nPerLvl + 0,
        _t1 = nPerLvl + 0,
        _t4 = *(int*)(nPerLvl + 0),
        i = 0;
        i < 1124864; i++) {
      if (i < _t4) {
         _t5[i] = x_19[i];
      }
   }
   for (_t11 = *(int (**)[27])(colInds + 0),
        _t10 = *(int**)(nnzPerRow + 0),
        _t9 = *(int*)_t2,
        _t8 = Ap,
        _t7 = *(double (**)[27])(A + 0),
        _t6 = p,
        i_1 = 0;
        i_1 < 1124864; i_1++) {
      if (i_1 < _t9) {
         int j;
         _t8[i_1] = 0.0;
         for (j = 0; j < 27; j++) {
            if (j < _t10[i_1]) {
               spmvBB(_t11[i_1] + j, _t7[i_1] + j, _t6, _t8 + i_1);
            }
         }
      }
   }
   for (_t15 = b,
        _t14 = Ap,
        _t13 = *(int*)_t1,
        _t12 = *(double**)(r + 0),
        i_2 = 0;
        i_2 < 1124864; i_2++) {
      if (i_2 < _t13) {
         _t12[i_2] = _t15[i_2] + -1.0 * _t14[i_2];
      }
   }
   for (_t18 = *(int*)_t3,
        _t17 = *(double**)(r + 0),
        _t16 = *(double**)(r + 0),
        local_result = 0.0,
        i_3 = 0;
        i_3 < 1124864; i_3++) {
      if (i_3 < _t18) {
         local_result = local_result + _t16[i_3] * _t17[i_3];
      }
   }
   *normr_8 = local_result;
}

/*
 * ../src/hpcg_precond_fissioned.c:684.6
 */
static void mapped_step5_1(double* alpha, double rtz, double* pAp, double* x_20
   , double* normr_9)
{
   int const* _t1;
   int const* _t2;
   int const* _t3;
   int const* _t4;
   double local_result;
   double local_result_1;
   double* _t5;
   double (* _t6)[27];
   double* _t7;
   int _t8;
   int* _t9;
   int (* _t10)[27];
   int i;
   double* _t11;
   double* _t12;
   int _t13;
   int i_1;
   int _t14;
   double _t15;
   double* _t16;
   int i_2;
   double* _t17;
   double* _t18;
   double _t19;
   int _t20;
   double* _t21;
   int i_3;
   int _t22;
   double* _t23;
   double* _t24;
   int i_4;
   for (_t10 = *(int (**)[27])(colInds + 0),
        _t9 = *(int**)(nnzPerRow + 0),
        _t4 = nPerLvl + 0,
        _t3 = nPerLvl + 0,
        _t2 = nPerLvl + 0,
        _t1 = nPerLvl + 0,
        _t8 = *(int*)(nPerLvl + 0),
        _t7 = Ap,
        _t6 = *(double (**)[27])(A + 0),
        _t5 = p,
        i = 0;
        i < 1124864; i++) {
      if (i < _t8) {
         int j;
         _t7[i] = 0.0;
         for (j = 0; j < 27; j++) {
            if (j < _t9[i]) {
               spmvBB(_t10[i] + j, _t6[i] + j, _t5, _t7 + i);
            }
         }
      }
   }
   for (_t13 = *(int*)_t1,
        _t12 = Ap,
        _t11 = p,
        local_result_1 = 0.0,
        i_1 = 0;
        i_1 < 1124864; i_1++) {
      if (i_1 < _t13) {
         local_result_1 = local_result_1 + _t11[i_1] * _t12[i_1];
      }
   }
   *pAp = local_result_1;
   *alpha = rtz / *pAp;
   for (_t16 = p,
        _t15 = *alpha,
        _t14 = *(int*)_t2,
        i_2 = 0;
        i_2 < 1124864; i_2++) {
      if (i_2 < _t14) {
         x_20[i_2] = x_20[i_2] + _t15 * _t16[i_2];
      }
   }
   for (_t21 = *(double**)(r + 0),
        _t20 = *(int*)_t3,
        _t19 = - *alpha,
        _t18 = Ap,
        _t17 = *(double**)(r + 0),
        i_3 = 0;
        i_3 < 1124864; i_3++) {
      if (i_3 < _t20) {
         _t21[i_3] = _t17[i_3] + _t19 * _t18[i_3];
      }
   }
   for (_t24 = *(double**)(r + 0),
        _t23 = *(double**)(r + 0),
        _t22 = *(int*)_t4,
        local_result = 0.0,
        i_4 = 0;
        i_4 < 1124864; i_4++) {
      if (i_4 < _t22) {
         local_result = local_result + _t24[i_4] * _t23[i_4];
      }
   }
   *normr_9 = local_result;
}

/*
 * ../src/hpcg_precond_fissioned.c:684.6
 */
static void mapped_step5_2(double* alpha, double rtz, double* pAp, double* x_21
   , double* normr_10)
{
   int const* _t1;
   int const* _t2;
   int const* _t3;
   int const* _t4;
   double local_result;
   double local_result_1;
   double* _t5;
   double (* _t6)[27];
   double* _t7;
   int _t8;
   int* _t9;
   int (* _t10)[27];
   int i;
   double* _t11;
   double* _t12;
   int _t13;
   int i_1;
   int _t14;
   double _t15;
   double* _t16;
   int i_2;
   double* _t17;
   double* _t18;
   double _t19;
   int _t20;
   double* _t21;
   int i_3;
   int _t22;
   double* _t23;
   double* _t24;
   int i_4;
   for (_t10 = *(int (**)[27])(colInds + 0),
        _t9 = *(int**)(nnzPerRow + 0),
        _t4 = nPerLvl + 0,
        _t3 = nPerLvl + 0,
        _t2 = nPerLvl + 0,
        _t1 = nPerLvl + 0,
        _t8 = *(int*)(nPerLvl + 0),
        _t7 = Ap,
        _t6 = *(double (**)[27])(A + 0),
        _t5 = p,
        i = 0;
        i < 1124864; i++) {
      if (i < _t8) {
         int j;
         _t7[i] = 0.0;
         for (j = 0; j < 27; j++) {
            if (j < _t9[i]) {
               spmvBB(_t10[i] + j, _t6[i] + j, _t5, _t7 + i);
            }
         }
      }
   }
   for (_t13 = *(int*)_t1,
        _t12 = Ap,
        _t11 = p,
        local_result_1 = 0.0,
        i_1 = 0;
        i_1 < 1124864; i_1++) {
      if (i_1 < _t13) {
         local_result_1 = local_result_1 + _t11[i_1] * _t12[i_1];
      }
   }
   *pAp = local_result_1;
   *alpha = rtz / *pAp;
   for (_t16 = p,
        _t15 = *alpha,
        _t14 = *(int*)_t2,
        i_2 = 0;
        i_2 < 1124864; i_2++) {
      if (i_2 < _t14) {
         x_21[i_2] = x_21[i_2] + _t15 * _t16[i_2];
      }
   }
   for (_t21 = *(double**)(r + 0),
        _t20 = *(int*)_t3,
        _t19 = - *alpha,
        _t18 = Ap,
        _t17 = *(double**)(r + 0),
        i_3 = 0;
        i_3 < 1124864; i_3++) {
      if (i_3 < _t20) {
         _t21[i_3] = _t17[i_3] + _t19 * _t18[i_3];
      }
   }
   for (_t24 = *(double**)(r + 0),
        _t23 = *(double**)(r + 0),
        _t22 = *(int*)_t4,
        local_result = 0.0,
        i_4 = 0;
        i_4 < 1124864; i_4++) {
      if (i_4 < _t22) {
         local_result = local_result + _t24[i_4] * _t23[i_4];
      }
   }
   *normr_10 = local_result;
}

/*
 * ../src/hpcg_precond_fissioned.c:684.6
 */
static void mapped_step5_dpbr(double* alpha, double* rtz, double* pAp, double*
   x_22, double* normr_11)
{
   ocrGuid_t outEvt;
   union __args_mapped_step5_dpbr_noGVars_main_76* allArgs;
   ocrGuid_t _t1;
   union __args_mapped_step5_dpbr_noGVars_main_76* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, (int)0l);
   _t3 = rocrInit(8u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars_main, 7u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars15, 0u, (unsigned long)1l, 0u)
      ;
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars17, 1u, (unsigned long)138l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars20, 2u, (unsigned long)18l, 0u
      );
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars22, 3u, (unsigned long)1l, 0u)
      ;
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars24, 4u, (unsigned long)35l, 0u
      );
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars27, 5u, (unsigned long)9l, 0u)
      ;
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars29, 6u, (unsigned long)1l, 0u)
      ;
   _t1 = rocrAlloc((void**)&allArgs, 96ul);
   _t2 = allArgs;
   _t2->data.nPerLvl_l = nPerLvl;
   _t2->data.normr = normr_11;
   _t2->data.r_l = r;
   _t2->data.x = x_22;
   _t2->data.p_l = p;
   _t2->data.Ap_l = Ap;
   _t2->data.pAp = pAp;
   _t2->data.alpha = alpha;
   _t2->data.rtz = rtz;
   _t2->data.nnzPerRow_l = nnzPerRow;
   _t2->data.colInds_l = colInds;
   _t2->data.A_l = A;
   rocrExecute(_t3, mapped_step5_dpbr_noGVars_main, _t1, &outEvt);
   rocrExit(_t3);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

void mapped_step1_noGVars(double* x_23, double* normr_12, int const* nPerLvl_l,
    double* p_l, double*restrict* r_l, double (*restrict* A_l)[27], double*
   Ap_l, int*restrict* nnzPerRow_l, int (*restrict* colInds_l)[27], double* b_l
   )
{
   ocrGuid_t outEvt;
   union __args_mapped_step1_noGVars_main_60* allArgs;
   ocrGuid_t _t1;
   union __args_mapped_step1_noGVars_main_60* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, (int)0l);
   _t3 = rocrInit(6u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, mapped_step1_noGVars_main, 5u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars11, 0u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars13, 1u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars16, 2u, (unsigned long)138l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars19, 3u, (unsigned long)9l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars21, 4u, (unsigned long)1l, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 80ul);
   _t2 = allArgs;
   _t2->data.nPerLvl_l = nPerLvl_l;
   _t2->data.normr = normr_12;
   _t2->data.r_l = r_l;
   _t2->data.Ap_l = Ap_l;
   _t2->data.nnzPerRow_l = nnzPerRow_l;
   _t2->data.b_l = b_l;
   _t2->data.colInds_l = colInds_l;
   _t2->data.A_l = A_l;
   _t2->data.p_l = p_l;
   _t2->data.x = x_23;
   rocrExecute(_t3, mapped_step1_noGVars_main, _t1, &outEvt);
   rocrExit(_t3);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

static ocrGuid_t mapped_step1_noGVars11(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars11_50 mapped_step1_noGVars11_args;
   union __args_mapped_step1_noGVars21_58* allArgs;
   union __args_mapped_step1_noGVars19_56* allArgs_1;
   union __args_mapped_step1_noGVars16_54* allArgs_2;
   union __args_mapped_step1_noGVars13_52* allArgs_3;
   void* _t1;
   int const* nPerLvl_l;
   int* _p;
   int* _p_1;
   int* _p_2;
   double* _p_3;
   int* _p_4;
   double* normr_13;
   double*restrict* r_l;
   double* Ap_l;
   int*restrict* nnzPerRow_l;
   double* b_l;
   int (*restrict* colInds_l)[27];
   double (*restrict* A_l)[27];
   double* p_l;
   double* x_24;
   ocrGuid_t _t2;
   union __args_mapped_step1_noGVars21_58* _t3;
   long i;
   long i_1;
   long i_2;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars11_args = *(union __args_mapped_step1_noGVars11_50*)
      rocrArgs(depv);
   nPerLvl_l = mapped_step1_noGVars11_args.data.nPerLvl_l;
   _p = mapped_step1_noGVars11_args.data._p;
   *_p = *(int*)nPerLvl_l;
   _p_1 = mapped_step1_noGVars11_args.data._p_1;
   *_p_1 = *(int*)nPerLvl_l;
   _p_2 = mapped_step1_noGVars11_args.data._p_2;
   *_p_2 = *(int*)nPerLvl_l;
   _p_3 = mapped_step1_noGVars11_args.data._p_3;
   *_p_3 = 0.0;
   _p_4 = mapped_step1_noGVars11_args.data._p_4;
   *_p_4 = *(int*)nPerLvl_l;
   _t2 = rocrAlloc((void**)&allArgs, 16ul);
   _t3 = allArgs;
   normr_13 = mapped_step1_noGVars11_args.data.normr;
   _t3->data.normr = normr_13;
   _t3->data._p = _p_3;
   rocrAutoDec(_t1, 4u, (unsigned long)0l, _t2, mapped_step1_noGVars21_count_4,
       (void (*)(void*, long*))((void*)0l), (void (*)(void*, long*))((void*)0l)
      , 0u);
   for (x_24 = mapped_step1_noGVars11_args.data.x,
        p_l = mapped_step1_noGVars11_args.data.p_l,
        A_l = mapped_step1_noGVars11_args.data.A_l,
        colInds_l = mapped_step1_noGVars11_args.data.colInds_l,
        b_l = mapped_step1_noGVars11_args.data.b_l,
        nnzPerRow_l = mapped_step1_noGVars11_args.data.nnzPerRow_l,
        Ap_l = mapped_step1_noGVars11_args.data.Ap_l,
        r_l = mapped_step1_noGVars11_args.data.r_l,
        i = 0l;
        i <= 8l; i++) {
      ocrGuid_t _t4;
      union __args_mapped_step1_noGVars19_56* _t5;
      _t4 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t5 = allArgs_1;
      _t5->data.r_l = r_l;
      _t5->data._p = _p_4;
      _t5->data._p_1 = _p_3;
      _t5->data.normr = normr_13;
      _t5->data.IT0 = i;
      rocrAutoDec(_t1, 3u, (unsigned long)i, _t4,
         mapped_step1_noGVars19_count_3, (void (*)(void*, long*))((void*)0l), (
         void (*)(void*, long*))((void*)0l), 1u, (long)i);
   }
   for (i_1 = 0l; i_1 <= 137l; i_1++) {
      ocrGuid_t _t6;
      union __args_mapped_step1_noGVars16_54* _t7;
      _t6 = rocrAlloc((void**)&allArgs_2, 104ul);
      _t7 = allArgs_2;
      _t7->data.Ap_l = Ap_l;
      _t7->data.nnzPerRow_l = nnzPerRow_l;
      _t7->data.r_l = r_l;
      _t7->data.b_l = b_l;
      _t7->data._p = _p_1;
      _t7->data._p_1 = _p_2;
      _t7->data.colInds_l = colInds_l;
      _t7->data.A_l = A_l;
      _t7->data.p_l = p_l;
      _t7->data.normr = normr_13;
      _t7->data._p_2 = _p_3;
      _t7->data._p_3 = _p_4;
      _t7->data.IT0 = i_1;
      rocrAutoDec(_t1, 2u, (unsigned long)i_1, _t6,
         mapped_step1_noGVars16_count_2, (void (*)(void*, long*))((void*)0l), (
         void (*)(void*, long*))((void*)0l), 1u, (long)i_1);
   }
   for (i_2 = 0l; i_2 <= 17l; i_2++) {
      ocrGuid_t _t8;
      union __args_mapped_step1_noGVars13_52* _t9;
      _t8 = rocrAlloc((void**)&allArgs_3, 120ul);
      _t9 = allArgs_3;
      _t9->data.p_l = p_l;
      _t9->data.x = x_24;
      _t9->data._p = _p;
      _t9->data.normr = normr_13;
      _t9->data.r_l = r_l;
      _t9->data.Ap_l = Ap_l;
      _t9->data.nnzPerRow_l = nnzPerRow_l;
      _t9->data.b_l = b_l;
      _t9->data.colInds_l = colInds_l;
      _t9->data.A_l = A_l;
      _t9->data._p_1 = _p_3;
      _t9->data._p_2 = _p_4;
      _t9->data._p_3 = _p_1;
      _t9->data._p_4 = _p_2;
      _t9->data.IT0 = i_2;
      rocrAutoDec(_t1, 1u, (unsigned long)i_2, _t8,
         mapped_step1_noGVars13_count_1, (void (*)(void*, long*))((void*)0l), (
         void (*)(void*, long*))((void*)0l), 1u, (long)i_2);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t mapped_step1_noGVars13(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars13_52 mapped_step1_noGVars13_args;
   union __args_mapped_step1_noGVars16_54* allArgs;
   void* _t1;
   double* p_l;
   double* normr_14;
   double*restrict* r_l;
   double* Ap_l;
   int*restrict* nnzPerRow_l;
   double* b_l;
   int (*restrict* colInds_l)[27];
   double (*restrict* A_l)[27];
   double* _p_1;
   int* _p_2;
   int* _p_3;
   int* _p_4;
   double* x_25;
   int* _p;
   long IT0;
   long _t2;
   long i;
   long i_1;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        mapped_step1_noGVars13_args = *(union __args_mapped_step1_noGVars13_52*
           )rocrArgs(depv),
        IT0 = mapped_step1_noGVars13_args.data.IT0,
        _t2 = (__mins_64(-65536l * IT0 + 1124863l, 65535l)),
        _p_4 = mapped_step1_noGVars13_args.data._p_4,
        _p_3 = mapped_step1_noGVars13_args.data._p_3,
        _p_2 = mapped_step1_noGVars13_args.data._p_2,
        _p_1 = mapped_step1_noGVars13_args.data._p_1,
        A_l = mapped_step1_noGVars13_args.data.A_l,
        colInds_l = mapped_step1_noGVars13_args.data.colInds_l,
        b_l = mapped_step1_noGVars13_args.data.b_l,
        nnzPerRow_l = mapped_step1_noGVars13_args.data.nnzPerRow_l,
        Ap_l = mapped_step1_noGVars13_args.data.Ap_l,
        r_l = mapped_step1_noGVars13_args.data.r_l,
        normr_14 = mapped_step1_noGVars13_args.data.normr,
        _p = mapped_step1_noGVars13_args.data._p,
        x_25 = mapped_step1_noGVars13_args.data.x,
        p_l = mapped_step1_noGVars13_args.data.p_l,
        i = 0l;
        i <= _t2; i++) {
      int _p2;
      _p2 = (int)(i + 65536l * IT0) < *_p;
      if (_p2) {
         p_l[i + 65536l * IT0] = x_25[i + 65536l * IT0];
      }
   }
   for (i_1 = 0l; i_1 <= 137l; i_1++) {
      ocrGuid_t _t3;
      union __args_mapped_step1_noGVars16_54* _t4;
      _t3 = rocrAlloc((void**)&allArgs, 104ul);
      _t4 = allArgs;
      _t4->data.Ap_l = Ap_l;
      _t4->data.nnzPerRow_l = nnzPerRow_l;
      _t4->data.r_l = r_l;
      _t4->data.b_l = b_l;
      _t4->data._p = _p_3;
      _t4->data._p_1 = _p_4;
      _t4->data.colInds_l = colInds_l;
      _t4->data.A_l = A_l;
      _t4->data.p_l = p_l;
      _t4->data.normr = normr_14;
      _t4->data._p_2 = _p_1;
      _t4->data._p_3 = _p_2;
      _t4->data.IT0 = i_1;
      rocrAutoDec(_t1, 2u, (unsigned long)i_1, _t3,
         mapped_step1_noGVars16_count_2, (void (*)(void*, long*))((void*)0l), (
         void (*)(void*, long*))((void*)0l), 1u, (long)i_1);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t mapped_step1_noGVars16(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars16_54 mapped_step1_noGVars16_args;
   int _p3[8192];
   int _p5[8192];
   union __args_mapped_step1_noGVars19_56* allArgs;
   void* _t1;
   double* Ap_l;
   int*restrict* nnzPerRow_l;
   double*restrict* r_l;
   double* b_l;
   int (*restrict* colInds_l)[27];
   double (*restrict* A_l)[27];
   double* p_l;
   double* normr_15;
   double* _p_2;
   int* _p_3;
   long IT0;
   int* _p;
   int* _p_1;
   long _t2;
   long i;
   long i_1;
   long i_2;
   long i_3;
   long _t3;
   long i_4;
   long _t4;
   long i_5;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        mapped_step1_noGVars16_args = *(union __args_mapped_step1_noGVars16_54*
           )rocrArgs(depv),
        IT0 = mapped_step1_noGVars16_args.data.IT0,
        _t2 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)),
        _p_3 = mapped_step1_noGVars16_args.data._p_3,
        _p_2 = mapped_step1_noGVars16_args.data._p_2,
        normr_15 = mapped_step1_noGVars16_args.data.normr,
        p_l = mapped_step1_noGVars16_args.data.p_l,
        A_l = mapped_step1_noGVars16_args.data.A_l,
        colInds_l = mapped_step1_noGVars16_args.data.colInds_l,
        _p_1 = mapped_step1_noGVars16_args.data._p_1,
        _p = mapped_step1_noGVars16_args.data._p,
        b_l = mapped_step1_noGVars16_args.data.b_l,
        r_l = mapped_step1_noGVars16_args.data.r_l,
        nnzPerRow_l = mapped_step1_noGVars16_args.data.nnzPerRow_l,
        Ap_l = mapped_step1_noGVars16_args.data.Ap_l,
        i = 0l;
        i <= _t2; i++) {
      _p3[i] = (int)(i + 8192l * IT0) < *_p;
      if (_p3[i]) {
         Ap_l[i + 8192l * IT0] = 0.0;
      }
      _p5[i] = (int)(i + 8192l * IT0) < *_p_1;
   }
   for (i_1 = 0l, i_2 = 0l, i_3 = 26l; i_1 <= 1l; i_1++, i_3 -= 16, i_2 += 16)
      {
      long _t5;
      long j;
      for (_t5 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)), j = 0l; j <= _t5;
               j++) {
         if (_p3[j]) {
            long _t6;
            long k;
            long k_1;
            long k_2;
            long k_3;
            for (_t6 = (__mins_64(i_3, 15l)),
                 k = 0l,
                 k_1 = i_2,
                 k_3 = i_2,
                 k_2 = i_2;
                 k <= _t6;
                 k++,
                 k_1++,
                 k_2++,
                 k_3++) {
               int _p4;
               _p4 = (int)k_1 < nnzPerRow_l[0l][j + 8192l * IT0];
               if (_p4) {
                  spmvBB(colInds_l[0l][j + 8192l * IT0] + k_2, A_l[0l][j + 8192l
                     * IT0] + k_3, p_l, Ap_l + (j + 8192l * IT0));
               }
            }
         }
      }
   }
   for (_t3 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)), i_4 = 0l; i_4 <= _t3
           ; i_4++) {
      if (_p5[i_4]) {
         r_l[0l][i_4 + 8192l * IT0] = b_l[i_4 + 8192l * IT0] + -1.0 * Ap_l[i_4 +
            8192l * IT0];
      }
   }
   for (_t4 = (__mins_64(IT0 >> 4l, 8l)), i_5 = (__maxs_64(0l, IT0 + -15l + 15l
           >> 4l)); i_5 <= _t4; i_5++) {
      ocrGuid_t _t7;
      union __args_mapped_step1_noGVars19_56* _t8;
      _t7 = rocrAlloc((void**)&allArgs, 40ul);
      _t8 = allArgs;
      _t8->data.r_l = r_l;
      _t8->data._p = _p_3;
      _t8->data._p_1 = _p_2;
      _t8->data.normr = normr_15;
      _t8->data.IT0 = i_5;
      rocrAutoDec(_t1, 3u, (unsigned long)i_5, _t7,
         mapped_step1_noGVars19_count_3, (void (*)(void*, long*))((void*)0l), (
         void (*)(void*, long*))((void*)0l), 1u, (long)i_5);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t mapped_step1_noGVars19(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars19_56 mapped_step1_noGVars19_args;
   union __args_mapped_step1_noGVars21_58* allArgs;
   union __args_mapped_step1_noGVars19_56* allArgs_1;
   void* _t1;
   double*restrict* r_l;
   int* _p;
   double* _p_1;
   double* normr_16;
   long IT0;
   ocrGuid_t _t2;
   union __args_mapped_step1_noGVars21_58* _t3;
   long _t4;
   long i;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        mapped_step1_noGVars19_args = *(union __args_mapped_step1_noGVars19_56*
           )rocrArgs(depv),
        IT0 = mapped_step1_noGVars19_args.data.IT0,
        _t4 = (__mins_64(-131072l * IT0 + 1124863l, 131071l)),
        normr_16 = mapped_step1_noGVars19_args.data.normr,
        _p_1 = mapped_step1_noGVars19_args.data._p_1,
        _p = mapped_step1_noGVars19_args.data._p,
        r_l = mapped_step1_noGVars19_args.data.r_l,
        i = 0l;
        i <= _t4; i++) {
      int _p1;
      _p1 = (int)(i + 131072l * IT0) < *_p;
      if (_p1) {
         *_p_1 = *_p_1 + r_l[0l][i + 131072l * IT0] * r_l[0l][i + 131072l * IT0
            ];
      }
   }
   _t2 = rocrAlloc((void**)&allArgs, 16ul);
   _t3 = allArgs;
   _t3->data.normr = normr_16;
   _t3->data._p = _p_1;
   rocrAutoDec(_t1, 4u, (unsigned long)0l, _t2, mapped_step1_noGVars21_count_4,
       (void (*)(void*, long*))((void*)0l), (void (*)(void*, long*))((void*)0l)
      , 0u);
   if (IT0 <= 7l) {
      ocrGuid_t _t5;
      union __args_mapped_step1_noGVars19_56* _t6;
      long _t7;
      _t5 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t6 = allArgs_1;
      _t6->data.r_l = r_l;
      _t6->data._p = _p;
      _t6->data._p_1 = _p_1;
      _t6->data.normr = normr_16;
      _t7 = IT0 + 1l;
      _t6->data.IT0 = _t7;
      rocrAutoDec(_t1, 3u, (unsigned long)(IT0 + 1l), _t5,
         mapped_step1_noGVars19_count_3, (void (*)(void*, long*))((void*)0l), (
         void (*)(void*, long*))((void*)0l), 1u, (long)_t7);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t mapped_step1_noGVars21(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars21_58 mapped_step1_noGVars21_args;
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars21_args = *(union __args_mapped_step1_noGVars21_58*)
      rocrArgs(depv);
   *mapped_step1_noGVars21_args.data.normr = *mapped_step1_noGVars21_args.data.
      _p;
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

unsigned long mapped_step1_noGVars21_count_4(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul, i = 0l; i <= 8l; i++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars19_count_3(long* args)
{
   long _t1;
   unsigned long rcNbInputDeps;
   unsigned long rcNbInputDeps_1;
   long _t2;
   long i;
   for (_t1 = (long)args[0l],
        _t2 = (__mins_64(137l, 16l * _t1 + 15l)),
        rcNbInputDeps = 0ul + 1ul,
        i = 16l * _t1,
        rcNbInputDeps_1 = 0ul + 2ul;
        i <= _t2; i++, rcNbInputDeps++, rcNbInputDeps_1++) {
   }
   if (_t1 >= 1l) {
      rcNbInputDeps = rcNbInputDeps_1;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars16_count_2(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul, i = 0l; i <= 17l; i++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars13_count_1(long* args)
{
   return 0ul + 1ul;
}

static ocrGuid_t mapped_step1_noGVars_main(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars_main_60 mapped_step1_noGVars_main_args;
   union __args_mapped_step1_noGVars11_50* allArgs;
   void* _t1;
   int* _t2;
   void* _t3;
   int* _t4;
   void* _t5;
   int* _t6;
   void* _t7;
   double* _t8;
   void* _t9;
   int* _t10;
   void* _t11;
   ocrGuid_t _t12;
   union __args_mapped_step1_noGVars11_50* _t13;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars_main_args = *(union __args_mapped_step1_noGVars_main_60
      *)rocrArgs(depv);
   _t3 = malloc(4ul);
   _t5 = malloc(4ul);
   _t7 = malloc(4ul);
   _t9 = malloc(8ul);
   _t11 = malloc(4ul);
   _t12 = rocrAlloc((void**)&allArgs, 120ul);
   _t13 = allArgs;
   _t13->data.nPerLvl_l = mapped_step1_noGVars_main_args.data.nPerLvl_l;
   _t2 = (int*)_t3;
   _t13->data._p = _t2;
   _t4 = (int*)_t5;
   _t13->data._p_1 = _t4;
   _t6 = (int*)_t7;
   _t13->data._p_2 = _t6;
   _t8 = (double*)_t9;
   _t13->data._p_3 = _t8;
   _t10 = (int*)_t11;
   _t13->data._p_4 = _t10;
   _t13->data.normr = mapped_step1_noGVars_main_args.data.normr;
   _t13->data.r_l = mapped_step1_noGVars_main_args.data.r_l;
   _t13->data.Ap_l = mapped_step1_noGVars_main_args.data.Ap_l;
   _t13->data.nnzPerRow_l = mapped_step1_noGVars_main_args.data.nnzPerRow_l;
   _t13->data.b_l = mapped_step1_noGVars_main_args.data.b_l;
   _t13->data.colInds_l = mapped_step1_noGVars_main_args.data.colInds_l;
   _t13->data.A_l = mapped_step1_noGVars_main_args.data.A_l;
   _t13->data.p_l = mapped_step1_noGVars_main_args.data.p_l;
   _t13->data.x = mapped_step1_noGVars_main_args.data.x;
   rocrAutoDec(_t1, 0u, (unsigned long)0l, _t12, (unsigned long (*)(long*))((
      void*)0l), (void (*)(void*, long*))((void*)0l), (void (*)(void*, long*))(
      (void*)0l), 0u);
   rocrAsyncFree(_t2);
   rocrAsyncFree(_t4);
   rocrAsyncFree(_t6);
   rocrAsyncFree(_t8);
   rocrAsyncFree(_t10);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars_async(void* event, double* x_26, double* normr_17,
   int const* nPerLvl_l, double* p_l, double*restrict* r_l, double (*restrict*
   A_l)[27], double* Ap_l, int*restrict* nnzPerRow_l, int (*restrict* colInds_l
   )[27], double* b_l)
{
   union __args_mapped_step1_noGVars_main_60* allArgs;
   ocrGuid_t _t1;
   union __args_mapped_step1_noGVars_main_60* _t2;
   void* _t3;
   _t3 = rocrInit(6u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, mapped_step1_noGVars_main, 5u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars11, 0u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars13, 1u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars16, 2u, (unsigned long)138l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars19, 3u, (unsigned long)9l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars21, 4u, (unsigned long)1l, 0u);
   _t1 = rocrAlloc((void**)&allArgs, 80ul);
   _t2 = allArgs;
   _t2->data.nPerLvl_l = nPerLvl_l;
   _t2->data.normr = normr_17;
   _t2->data.r_l = r_l;
   _t2->data.Ap_l = Ap_l;
   _t2->data.nnzPerRow_l = nnzPerRow_l;
   _t2->data.b_l = b_l;
   _t2->data.colInds_l = colInds_l;
   _t2->data.A_l = A_l;
   _t2->data.p_l = p_l;
   _t2->data.x = x_26;
   rocrExecute(_t3, mapped_step1_noGVars_main, _t1, event);
   rocrExit(_t3);
}

void mapped_step5_dpbr_noGVars(double* alpha, double* rtz, double* pAp, double*
    x_27, double* normr_18, double* p_l, double (*restrict* A_l)[27], double*
   Ap_l, int const* nPerLvl_l, int*restrict* nnzPerRow_l, int (*restrict*
   colInds_l)[27], double*restrict* r_l)
{
   ocrGuid_t outEvt;
   union __args_mapped_step5_dpbr_noGVars_main_76* allArgs;
   ocrGuid_t _t1;
   union __args_mapped_step5_dpbr_noGVars_main_76* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, (int)0l);
   _t3 = rocrInit(8u, 0u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars_main, 7u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars15, 0u, (unsigned long)1l, 0u)
      ;
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars17, 1u, (unsigned long)138l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars20, 2u, (unsigned long)18l, 0u
      );
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars22, 3u, (unsigned long)1l, 0u)
      ;
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars24, 4u, (unsigned long)35l, 0u
      );
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars27, 5u, (unsigned long)9l, 0u)
      ;
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars29, 6u, (unsigned long)1l, 0u)
      ;
   _t1 = rocrAlloc((void**)&allArgs, 96ul);
   _t2 = allArgs;
   _t2->data.nPerLvl_l = nPerLvl_l;
   _t2->data.normr = normr_18;
   _t2->data.r_l = r_l;
   _t2->data.x = x_27;
   _t2->data.p_l = p_l;
   _t2->data.Ap_l = Ap_l;
   _t2->data.pAp = pAp;
   _t2->data.alpha = alpha;
   _t2->data.rtz = rtz;
   _t2->data.nnzPerRow_l = nnzPerRow_l;
   _t2->data.colInds_l = colInds_l;
   _t2->data.A_l = A_l;
   rocrExecute(_t3, mapped_step5_dpbr_noGVars_main, _t1, &outEvt);
   rocrExit(_t3);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

static ocrGuid_t mapped_step5_dpbr_noGVars15(unsigned int paramc, unsigned long
   * paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars15_62 mapped_step5_dpbr_noGVars15_args
      ;
   union __args_mapped_step5_dpbr_noGVars29_74* allArgs;
   union __args_mapped_step5_dpbr_noGVars27_72* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars24_70* allArgs_2;
   union __args_mapped_step5_dpbr_noGVars22_68* allArgs_3;
   union __args_mapped_step5_dpbr_noGVars20_66* allArgs_4;
   union __args_mapped_step5_dpbr_noGVars17_64* allArgs_5;
   void* _t1;
   int const* nPerLvl_l;
   int* _p;
   double* _p_1;
   int* _p_2;
   int* _p_3;
   int* _p_4;
   double* _p_5;
   int* _p_6;
   double* normr_19;
   double*restrict* r_l;
   double* x_28;
   double* p_l;
   double* Ap_l;
   double* pAp;
   double* alpha;
   double* rtz;
   int*restrict* nnzPerRow_l;
   int (*restrict* colInds_l)[27];
   double (*restrict* A_l)[27];
   double* _p_7;
   double* _p_8;
   ocrGuid_t _t2;
   union __args_mapped_step5_dpbr_noGVars22_68* _t3;
   ocrGuid_t _t4;
   union __args_mapped_step5_dpbr_noGVars29_74* _t5;
   long i;
   long i_1;
   long i_2;
   long i_3;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars15_args = *(union
      __args_mapped_step5_dpbr_noGVars15_62*)rocrArgs(depv);
   nPerLvl_l = mapped_step5_dpbr_noGVars15_args.data.nPerLvl_l;
   _p = mapped_step5_dpbr_noGVars15_args.data._p;
   *_p = *(int*)nPerLvl_l;
   _p_1 = mapped_step5_dpbr_noGVars15_args.data._p_1;
   *_p_1 = 0.0;
   _p_2 = mapped_step5_dpbr_noGVars15_args.data._p_2;
   *_p_2 = *(int*)nPerLvl_l;
   _p_3 = mapped_step5_dpbr_noGVars15_args.data._p_3;
   *_p_3 = *(int*)nPerLvl_l;
   _p_4 = mapped_step5_dpbr_noGVars15_args.data._p_4;
   *_p_4 = *(int*)nPerLvl_l;
   _p_5 = mapped_step5_dpbr_noGVars15_args.data._p_5;
   *_p_5 = 0.0;
   _p_6 = mapped_step5_dpbr_noGVars15_args.data._p_6;
   *_p_6 = *(int*)nPerLvl_l;
   _t4 = rocrAlloc((void**)&allArgs, 16ul);
   _t5 = allArgs;
   normr_19 = mapped_step5_dpbr_noGVars15_args.data.normr;
   _t5->data.normr = normr_19;
   _t5->data._p = _p_5;
   rocrAutoDec(_t1, 6u, (unsigned long)0l, _t4,
      mapped_step5_dpbr_noGVars29_count_6, (void (*)(void*, long*))((void*)0l),
       (void (*)(void*, long*))((void*)0l), 0u);
   for (_p_8 = mapped_step5_dpbr_noGVars15_args.data._p_8,
        _p_7 = mapped_step5_dpbr_noGVars15_args.data._p_7,
        A_l = mapped_step5_dpbr_noGVars15_args.data.A_l,
        colInds_l = mapped_step5_dpbr_noGVars15_args.data.colInds_l,
        nnzPerRow_l = mapped_step5_dpbr_noGVars15_args.data.nnzPerRow_l,
        rtz = mapped_step5_dpbr_noGVars15_args.data.rtz,
        alpha = mapped_step5_dpbr_noGVars15_args.data.alpha,
        pAp = mapped_step5_dpbr_noGVars15_args.data.pAp,
        Ap_l = mapped_step5_dpbr_noGVars15_args.data.Ap_l,
        p_l = mapped_step5_dpbr_noGVars15_args.data.p_l,
        x_28 = mapped_step5_dpbr_noGVars15_args.data.x,
        r_l = mapped_step5_dpbr_noGVars15_args.data.r_l,
        i = 0l;
        i <= 8l; i++) {
      ocrGuid_t _t6;
      union __args_mapped_step5_dpbr_noGVars27_72* _t7;
      _t6 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t7 = allArgs_1;
      _t7->data.r_l = r_l;
      _t7->data._p = _p_6;
      _t7->data._p_1 = _p_5;
      _t7->data.normr = normr_19;
      _t7->data.IT0 = i;
      rocrAutoDec(_t1, 5u, (unsigned long)i, _t6,
         mapped_step5_dpbr_noGVars27_count_5, (void (*)(void*, long*))((void*)
         0l), (void (*)(void*, long*))((void*)0l), 1u, (long)i);
   }
   for (i_1 = 0l; i_1 <= 34l; i_1++) {
      ocrGuid_t _t8;
      union __args_mapped_step5_dpbr_noGVars24_70* _t9;
      _t8 = rocrAlloc((void**)&allArgs_2, 96ul);
      _t9 = allArgs_2;
      _t9->data.x = x_28;
      _t9->data.p_l = p_l;
      _t9->data.r_l = r_l;
      _t9->data.Ap_l = Ap_l;
      _t9->data._p = _p_3;
      _t9->data._p_1 = _p_7;
      _t9->data._p_2 = _p_4;
      _t9->data._p_3 = _p_8;
      _t9->data.normr = normr_19;
      _t9->data._p_4 = _p_5;
      _t9->data._p_5 = _p_6;
      _t9->data.IT0 = i_1;
      rocrAutoDec(_t1, 4u, (unsigned long)i_1, _t8,
         mapped_step5_dpbr_noGVars24_count_4, (void (*)(void*, long*))((void*)
         0l), (void (*)(void*, long*))((void*)0l), 1u, (long)i_1);
   }
   _t2 = rocrAlloc((void**)&allArgs_3, 120ul);
   _t3 = allArgs_3;
   _t3->data.pAp = pAp;
   _t3->data.alpha = alpha;
   _t3->data.rtz = rtz;
   _t3->data._p = _p_1;
   _t3->data._p_1 = _p_7;
   _t3->data._p_2 = _p_8;
   _t3->data.normr = normr_19;
   _t3->data.r_l = r_l;
   _t3->data.x = x_28;
   _t3->data.p_l = p_l;
   _t3->data.Ap_l = Ap_l;
   _t3->data._p_3 = _p_5;
   _t3->data._p_4 = _p_6;
   _t3->data._p_5 = _p_3;
   _t3->data._p_6 = _p_4;
   rocrAutoDec(_t1, 3u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars22_count_3, (void (*)(void*, long*))((void*)0l),
       (void (*)(void*, long*))((void*)0l), 0u);
   for (i_2 = 0l; i_2 <= 17l; i_2++) {
      ocrGuid_t _t10;
      union __args_mapped_step5_dpbr_noGVars20_66* _t11;
      _t10 = rocrAlloc((void**)&allArgs_4, 136ul);
      _t11 = allArgs_4;
      _t11->data.p_l = p_l;
      _t11->data.Ap_l = Ap_l;
      _t11->data._p = _p_2;
      _t11->data._p_1 = _p_1;
      _t11->data.normr = normr_19;
      _t11->data.r_l = r_l;
      _t11->data.x = x_28;
      _t11->data.pAp = pAp;
      _t11->data.alpha = alpha;
      _t11->data.rtz = rtz;
      _t11->data._p_2 = _p_5;
      _t11->data._p_3 = _p_6;
      _t11->data._p_4 = _p_3;
      _t11->data._p_5 = _p_7;
      _t11->data._p_6 = _p_4;
      _t11->data._p_7 = _p_8;
      _t11->data.IT0 = i_2;
      rocrAutoDec(_t1, 2u, (unsigned long)i_2, _t10,
         mapped_step5_dpbr_noGVars20_count_2, (void (*)(void*, long*))((void*)
         0l), (void (*)(void*, long*))((void*)0l), 1u, (long)i_2);
   }
   for (i_3 = 0l; i_3 <= 137l; i_3++) {
      ocrGuid_t _t12;
      union __args_mapped_step5_dpbr_noGVars17_64* _t13;
      _t12 = rocrAlloc((void**)&allArgs_5, 168ul);
      _t13 = allArgs_5;
      _t13->data.Ap_l = Ap_l;
      _t13->data.nnzPerRow_l = nnzPerRow_l;
      _t13->data._p = _p;
      _t13->data.colInds_l = colInds_l;
      _t13->data.A_l = A_l;
      _t13->data.p_l = p_l;
      _t13->data.normr = normr_19;
      _t13->data.r_l = r_l;
      _t13->data.x = x_28;
      _t13->data.pAp = pAp;
      _t13->data.alpha = alpha;
      _t13->data.rtz = rtz;
      _t13->data._p_1 = _p_5;
      _t13->data._p_2 = _p_6;
      _t13->data._p_3 = _p_3;
      _t13->data._p_4 = _p_7;
      _t13->data._p_5 = _p_4;
      _t13->data._p_6 = _p_8;
      _t13->data._p_7 = _p_1;
      _t13->data._p_8 = _p_2;
      _t13->data.IT0 = i_3;
      rocrAutoDec(_t1, 1u, (unsigned long)i_3, _t12,
         mapped_step5_dpbr_noGVars17_count_1, (void (*)(void*, long*))((void*)
         0l), (void (*)(void*, long*))((void*)0l), 1u, (long)i_3);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t mapped_step5_dpbr_noGVars17(unsigned int paramc, unsigned long
   * paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars17_64 mapped_step5_dpbr_noGVars17_args
      ;
   int _p2[8192];
   union __args_mapped_step5_dpbr_noGVars24_70* allArgs;
   union __args_mapped_step5_dpbr_noGVars20_66* allArgs_1;
   void* _t1;
   double* Ap_l;
   int*restrict* nnzPerRow_l;
   int (*restrict* colInds_l)[27];
   double (*restrict* A_l)[27];
   double* p_l;
   double* normr_20;
   double*restrict* r_l;
   double* x_29;
   double* pAp;
   double* alpha;
   double* rtz;
   double* _p_1;
   int* _p_2;
   int* _p_3;
   double* _p_4;
   int* _p_5;
   double* _p_6;
   double* _p_7;
   int* _p_8;
   long IT0;
   int* _p;
   long _t2;
   long i;
   long i_1;
   long i_2;
   long i_3;
   long _t3;
   long i_4;
   long _t4;
   long i_5;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        mapped_step5_dpbr_noGVars17_args = *(union
           __args_mapped_step5_dpbr_noGVars17_64*)rocrArgs(depv),
        IT0 = mapped_step5_dpbr_noGVars17_args.data.IT0,
        _t2 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)),
        _p_8 = mapped_step5_dpbr_noGVars17_args.data._p_8,
        _p_7 = mapped_step5_dpbr_noGVars17_args.data._p_7,
        _p_6 = mapped_step5_dpbr_noGVars17_args.data._p_6,
        _p_5 = mapped_step5_dpbr_noGVars17_args.data._p_5,
        _p_4 = mapped_step5_dpbr_noGVars17_args.data._p_4,
        _p_3 = mapped_step5_dpbr_noGVars17_args.data._p_3,
        _p_2 = mapped_step5_dpbr_noGVars17_args.data._p_2,
        _p_1 = mapped_step5_dpbr_noGVars17_args.data._p_1,
        rtz = mapped_step5_dpbr_noGVars17_args.data.rtz,
        alpha = mapped_step5_dpbr_noGVars17_args.data.alpha,
        pAp = mapped_step5_dpbr_noGVars17_args.data.pAp,
        x_29 = mapped_step5_dpbr_noGVars17_args.data.x,
        r_l = mapped_step5_dpbr_noGVars17_args.data.r_l,
        normr_20 = mapped_step5_dpbr_noGVars17_args.data.normr,
        p_l = mapped_step5_dpbr_noGVars17_args.data.p_l,
        A_l = mapped_step5_dpbr_noGVars17_args.data.A_l,
        colInds_l = mapped_step5_dpbr_noGVars17_args.data.colInds_l,
        _p = mapped_step5_dpbr_noGVars17_args.data._p,
        nnzPerRow_l = mapped_step5_dpbr_noGVars17_args.data.nnzPerRow_l,
        Ap_l = mapped_step5_dpbr_noGVars17_args.data.Ap_l,
        i = 0l;
        i <= _t2; i++) {
      _p2[i] = (int)(i + 8192l * IT0) < *_p;
      if (_p2[i]) {
         Ap_l[i + 8192l * IT0] = 0.0;
      }
   }
   for (i_1 = 0l, i_2 = 0l, i_3 = 26l; i_1 <= 1l; i_1++, i_3 -= 16, i_2 += 16)
      {
      long _t5;
      long j;
      for (_t5 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)), j = 0l; j <= _t5;
               j++) {
         if (_p2[j]) {
            long _t6;
            long k;
            long k_1;
            long k_2;
            long k_3;
            for (_t6 = (__mins_64(i_3, 15l)),
                 k = 0l,
                 k_1 = i_2,
                 k_3 = i_2,
                 k_2 = i_2;
                 k <= _t6;
                 k++,
                 k_1++,
                 k_2++,
                 k_3++) {
               int _p3;
               _p3 = (int)k_1 < nnzPerRow_l[0l][j + 8192l * IT0];
               if (_p3) {
                  spmvBB(colInds_l[0l][j + 8192l * IT0] + k_2, A_l[0l][j + 8192l
                     * IT0] + k_3, p_l, Ap_l + (j + 8192l * IT0));
               }
            }
         }
      }
   }
   for (_t3 = (__mins_64(IT0 >> 2l, 34l)), i_4 = (__maxs_64(0l, IT0 + -3l + 3l
           >> 2l)); i_4 <= _t3; i_4++) {
      ocrGuid_t _t7;
      union __args_mapped_step5_dpbr_noGVars24_70* _t8;
      _t7 = rocrAlloc((void**)&allArgs, 96ul);
      _t8 = allArgs;
      _t8->data.x = x_29;
      _t8->data.p_l = p_l;
      _t8->data.r_l = r_l;
      _t8->data.Ap_l = Ap_l;
      _t8->data._p = _p_3;
      _t8->data._p_1 = _p_4;
      _t8->data._p_2 = _p_5;
      _t8->data._p_3 = _p_6;
      _t8->data.normr = normr_20;
      _t8->data._p_4 = _p_1;
      _t8->data._p_5 = _p_2;
      _t8->data.IT0 = i_4;
      rocrAutoDec(_t1, 4u, (unsigned long)i_4, _t7,
         mapped_step5_dpbr_noGVars24_count_4, (void (*)(void*, long*))((void*)
         0l), (void (*)(void*, long*))((void*)0l), 1u, (long)i_4);
   }
   for (_t4 = (__mins_64(IT0 >> 3l, 17l)), i_5 = (__maxs_64(0l, IT0 + -7l + 7l
           >> 3l)); i_5 <= _t4; i_5++) {
      ocrGuid_t _t9;
      union __args_mapped_step5_dpbr_noGVars20_66* _t10;
      _t9 = rocrAlloc((void**)&allArgs_1, 136ul);
      _t10 = allArgs_1;
      _t10->data.p_l = p_l;
      _t10->data.Ap_l = Ap_l;
      _t10->data._p = _p_8;
      _t10->data._p_1 = _p_7;
      _t10->data.normr = normr_20;
      _t10->data.r_l = r_l;
      _t10->data.x = x_29;
      _t10->data.pAp = pAp;
      _t10->data.alpha = alpha;
      _t10->data.rtz = rtz;
      _t10->data._p_2 = _p_1;
      _t10->data._p_3 = _p_2;
      _t10->data._p_4 = _p_3;
      _t10->data._p_5 = _p_4;
      _t10->data._p_6 = _p_5;
      _t10->data._p_7 = _p_6;
      _t10->data.IT0 = i_5;
      rocrAutoDec(_t1, 2u, (unsigned long)i_5, _t9,
         mapped_step5_dpbr_noGVars20_count_2, (void (*)(void*, long*))((void*)
         0l), (void (*)(void*, long*))((void*)0l), 1u, (long)i_5);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t mapped_step5_dpbr_noGVars20(unsigned int paramc, unsigned long
   * paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars20_66 mapped_step5_dpbr_noGVars20_args
      ;
   union __args_mapped_step5_dpbr_noGVars22_68* allArgs;
   union __args_mapped_step5_dpbr_noGVars20_66* allArgs_1;
   void* _t1;
   double* p_l;
   double* Ap_l;
   int* _p;
   double* _p_1;
   double* normr_21;
   double*restrict* r_l;
   double* x_30;
   double* pAp;
   double* alpha;
   double* rtz;
   double* _p_2;
   int* _p_3;
   int* _p_4;
   double* _p_5;
   int* _p_6;
   double* _p_7;
   long IT0;
   ocrGuid_t _t2;
   union __args_mapped_step5_dpbr_noGVars22_68* _t3;
   long _t4;
   long i;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        mapped_step5_dpbr_noGVars20_args = *(union
           __args_mapped_step5_dpbr_noGVars20_66*)rocrArgs(depv),
        IT0 = mapped_step5_dpbr_noGVars20_args.data.IT0,
        _t4 = (__mins_64(-65536l * IT0 + 1124863l, 65535l)),
        _p_7 = mapped_step5_dpbr_noGVars20_args.data._p_7,
        _p_6 = mapped_step5_dpbr_noGVars20_args.data._p_6,
        _p_5 = mapped_step5_dpbr_noGVars20_args.data._p_5,
        _p_4 = mapped_step5_dpbr_noGVars20_args.data._p_4,
        _p_3 = mapped_step5_dpbr_noGVars20_args.data._p_3,
        _p_2 = mapped_step5_dpbr_noGVars20_args.data._p_2,
        rtz = mapped_step5_dpbr_noGVars20_args.data.rtz,
        alpha = mapped_step5_dpbr_noGVars20_args.data.alpha,
        pAp = mapped_step5_dpbr_noGVars20_args.data.pAp,
        x_30 = mapped_step5_dpbr_noGVars20_args.data.x,
        r_l = mapped_step5_dpbr_noGVars20_args.data.r_l,
        normr_21 = mapped_step5_dpbr_noGVars20_args.data.normr,
        _p_1 = mapped_step5_dpbr_noGVars20_args.data._p_1,
        _p = mapped_step5_dpbr_noGVars20_args.data._p,
        Ap_l = mapped_step5_dpbr_noGVars20_args.data.Ap_l,
        p_l = mapped_step5_dpbr_noGVars20_args.data.p_l,
        i = 0l;
        i <= _t4; i++) {
      int _p4;
      _p4 = (int)(i + 65536l * IT0) < *_p;
      if (_p4) {
         *_p_1 = *_p_1 + p_l[i + 65536l * IT0] * Ap_l[i + 65536l * IT0];
      }
   }
   _t2 = rocrAlloc((void**)&allArgs, 120ul);
   _t3 = allArgs;
   _t3->data.pAp = pAp;
   _t3->data.alpha = alpha;
   _t3->data.rtz = rtz;
   _t3->data._p = _p_1;
   _t3->data._p_1 = _p_5;
   _t3->data._p_2 = _p_7;
   _t3->data.normr = normr_21;
   _t3->data.r_l = r_l;
   _t3->data.x = x_30;
   _t3->data.p_l = p_l;
   _t3->data.Ap_l = Ap_l;
   _t3->data._p_3 = _p_2;
   _t3->data._p_4 = _p_3;
   _t3->data._p_5 = _p_4;
   _t3->data._p_6 = _p_6;
   rocrAutoDec(_t1, 3u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars22_count_3, (void (*)(void*, long*))((void*)0l),
       (void (*)(void*, long*))((void*)0l), 0u);
   if (IT0 <= 16l) {
      ocrGuid_t _t5;
      union __args_mapped_step5_dpbr_noGVars20_66* _t6;
      long _t7;
      _t5 = rocrAlloc((void**)&allArgs_1, 136ul);
      _t6 = allArgs_1;
      _t6->data.p_l = p_l;
      _t6->data.Ap_l = Ap_l;
      _t6->data._p = _p;
      _t6->data._p_1 = _p_1;
      _t6->data.normr = normr_21;
      _t6->data.r_l = r_l;
      _t6->data.x = x_30;
      _t6->data.pAp = pAp;
      _t6->data.alpha = alpha;
      _t6->data.rtz = rtz;
      _t6->data._p_2 = _p_2;
      _t6->data._p_3 = _p_3;
      _t6->data._p_4 = _p_4;
      _t6->data._p_5 = _p_5;
      _t6->data._p_6 = _p_6;
      _t6->data._p_7 = _p_7;
      _t7 = IT0 + 1l;
      _t6->data.IT0 = _t7;
      rocrAutoDec(_t1, 2u, (unsigned long)(IT0 + 1l), _t5,
         mapped_step5_dpbr_noGVars20_count_2, (void (*)(void*, long*))((void*)
         0l), (void (*)(void*, long*))((void*)0l), 1u, (long)_t7);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t mapped_step5_dpbr_noGVars22(unsigned int paramc, unsigned long
   * paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars22_68 mapped_step5_dpbr_noGVars22_args
      ;
   union __args_mapped_step5_dpbr_noGVars24_70* allArgs;
   void* _t1;
   double* pAp;
   double* alpha;
   double* _p_1;
   double* _p_2;
   double* normr_22;
   double*restrict* r_l;
   double* x_31;
   double* p_l;
   double* Ap_l;
   double* _p_3;
   int* _p_4;
   int* _p_5;
   int* _p_6;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars22_args = *(union
      __args_mapped_step5_dpbr_noGVars22_68*)rocrArgs(depv);
   pAp = mapped_step5_dpbr_noGVars22_args.data.pAp;
   *pAp = *mapped_step5_dpbr_noGVars22_args.data._p;
   alpha = mapped_step5_dpbr_noGVars22_args.data.alpha;
   *alpha = *mapped_step5_dpbr_noGVars22_args.data.rtz / *pAp;
   _p_1 = mapped_step5_dpbr_noGVars22_args.data._p_1;
   *_p_1 = *alpha;
   _p_2 = mapped_step5_dpbr_noGVars22_args.data._p_2;
   *_p_2 = *alpha;
   for (_p_6 = mapped_step5_dpbr_noGVars22_args.data._p_6,
        _p_5 = mapped_step5_dpbr_noGVars22_args.data._p_5,
        _p_4 = mapped_step5_dpbr_noGVars22_args.data._p_4,
        _p_3 = mapped_step5_dpbr_noGVars22_args.data._p_3,
        Ap_l = mapped_step5_dpbr_noGVars22_args.data.Ap_l,
        p_l = mapped_step5_dpbr_noGVars22_args.data.p_l,
        x_31 = mapped_step5_dpbr_noGVars22_args.data.x,
        r_l = mapped_step5_dpbr_noGVars22_args.data.r_l,
        normr_22 = mapped_step5_dpbr_noGVars22_args.data.normr,
        i = 0l;
        i <= 34l; i++) {
      ocrGuid_t _t2;
      union __args_mapped_step5_dpbr_noGVars24_70* _t3;
      _t2 = rocrAlloc((void**)&allArgs, 96ul);
      _t3 = allArgs;
      _t3->data.x = x_31;
      _t3->data.p_l = p_l;
      _t3->data.r_l = r_l;
      _t3->data.Ap_l = Ap_l;
      _t3->data._p = _p_5;
      _t3->data._p_1 = _p_1;
      _t3->data._p_2 = _p_6;
      _t3->data._p_3 = _p_2;
      _t3->data.normr = normr_22;
      _t3->data._p_4 = _p_3;
      _t3->data._p_5 = _p_4;
      _t3->data.IT0 = i;
      rocrAutoDec(_t1, 4u, (unsigned long)i, _t2,
         mapped_step5_dpbr_noGVars24_count_4, (void (*)(void*, long*))((void*)
         0l), (void (*)(void*, long*))((void*)0l), 1u, (long)i);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t mapped_step5_dpbr_noGVars24(unsigned int paramc, unsigned long
   * paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars24_70 mapped_step5_dpbr_noGVars24_args
      ;
   union __args_mapped_step5_dpbr_noGVars27_72* allArgs;
   void* _t1;
   double*restrict* r_l;
   double* normr_23;
   double* _p_4;
   int* _p_5;
   long IT0;
   double* x_32;
   double* p_l;
   double* Ap_l;
   int* _p;
   double* _p_1;
   int* _p_2;
   double* _p_3;
   long _t2;
   long i;
   long _t3;
   long i_1;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        mapped_step5_dpbr_noGVars24_args = *(union
           __args_mapped_step5_dpbr_noGVars24_70*)rocrArgs(depv),
        IT0 = mapped_step5_dpbr_noGVars24_args.data.IT0,
        _t2 = (__mins_64(-32768l * IT0 + 1124863l, 32767l)),
        _p_5 = mapped_step5_dpbr_noGVars24_args.data._p_5,
        _p_4 = mapped_step5_dpbr_noGVars24_args.data._p_4,
        normr_23 = mapped_step5_dpbr_noGVars24_args.data.normr,
        _p_3 = mapped_step5_dpbr_noGVars24_args.data._p_3,
        _p_2 = mapped_step5_dpbr_noGVars24_args.data._p_2,
        _p_1 = mapped_step5_dpbr_noGVars24_args.data._p_1,
        _p = mapped_step5_dpbr_noGVars24_args.data._p,
        Ap_l = mapped_step5_dpbr_noGVars24_args.data.Ap_l,
        r_l = mapped_step5_dpbr_noGVars24_args.data.r_l,
        p_l = mapped_step5_dpbr_noGVars24_args.data.p_l,
        x_32 = mapped_step5_dpbr_noGVars24_args.data.x,
        i = 0l;
        i <= _t2; i++) {
      int _p6;
      int _p5;
      _p6 = (int)(i + 32768l * IT0) < *_p;
      if (_p6) {
         x_32[i + 32768l * IT0] = x_32[i + 32768l * IT0] + *_p_1 * p_l[i +
            32768l * IT0];
      }
      _p5 = (int)(i + 32768l * IT0) < *_p_2;
      if (_p5) {
         r_l[0l][i + 32768l * IT0] = r_l[0l][i + 32768l * IT0] + - *_p_3 * Ap_l
            [i + 32768l * IT0];
      }
   }
   for (_t3 = (__mins_64(IT0 >> 2l, 8l)), i_1 = (__maxs_64(0l, IT0 + -3l + 3l >>
           2l)); i_1 <= _t3; i_1++) {
      ocrGuid_t _t4;
      union __args_mapped_step5_dpbr_noGVars27_72* _t5;
      _t4 = rocrAlloc((void**)&allArgs, 40ul);
      _t5 = allArgs;
      _t5->data.r_l = r_l;
      _t5->data._p = _p_5;
      _t5->data._p_1 = _p_4;
      _t5->data.normr = normr_23;
      _t5->data.IT0 = i_1;
      rocrAutoDec(_t1, 5u, (unsigned long)i_1, _t4,
         mapped_step5_dpbr_noGVars27_count_5, (void (*)(void*, long*))((void*)
         0l), (void (*)(void*, long*))((void*)0l), 1u, (long)i_1);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t mapped_step5_dpbr_noGVars27(unsigned int paramc, unsigned long
   * paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars27_72 mapped_step5_dpbr_noGVars27_args
      ;
   union __args_mapped_step5_dpbr_noGVars29_74* allArgs;
   union __args_mapped_step5_dpbr_noGVars27_72* allArgs_1;
   void* _t1;
   double*restrict* r_l;
   int* _p;
   double* _p_1;
   double* normr_24;
   long IT0;
   ocrGuid_t _t2;
   union __args_mapped_step5_dpbr_noGVars29_74* _t3;
   long _t4;
   long i;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        mapped_step5_dpbr_noGVars27_args = *(union
           __args_mapped_step5_dpbr_noGVars27_72*)rocrArgs(depv),
        IT0 = mapped_step5_dpbr_noGVars27_args.data.IT0,
        _t4 = (__mins_64(-131072l * IT0 + 1124863l, 131071l)),
        normr_24 = mapped_step5_dpbr_noGVars27_args.data.normr,
        _p_1 = mapped_step5_dpbr_noGVars27_args.data._p_1,
        _p = mapped_step5_dpbr_noGVars27_args.data._p,
        r_l = mapped_step5_dpbr_noGVars27_args.data.r_l,
        i = 0l;
        i <= _t4; i++) {
      int _p1;
      _p1 = (int)(i + 131072l * IT0) < *_p;
      if (_p1) {
         *_p_1 = *_p_1 + r_l[0l][i + 131072l * IT0] * r_l[0l][i + 131072l * IT0
            ];
      }
   }
   _t2 = rocrAlloc((void**)&allArgs, 16ul);
   _t3 = allArgs;
   _t3->data.normr = normr_24;
   _t3->data._p = _p_1;
   rocrAutoDec(_t1, 6u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars29_count_6, (void (*)(void*, long*))((void*)0l),
       (void (*)(void*, long*))((void*)0l), 0u);
   if (IT0 <= 7l) {
      ocrGuid_t _t5;
      union __args_mapped_step5_dpbr_noGVars27_72* _t6;
      long _t7;
      _t5 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t6 = allArgs_1;
      _t6->data.r_l = r_l;
      _t6->data._p = _p;
      _t6->data._p_1 = _p_1;
      _t6->data.normr = normr_24;
      _t7 = IT0 + 1l;
      _t6->data.IT0 = _t7;
      rocrAutoDec(_t1, 5u, (unsigned long)(IT0 + 1l), _t5,
         mapped_step5_dpbr_noGVars27_count_5, (void (*)(void*, long*))((void*)
         0l), (void (*)(void*, long*))((void*)0l), 1u, (long)_t7);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t mapped_step5_dpbr_noGVars29(unsigned int paramc, unsigned long
   * paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars29_74 mapped_step5_dpbr_noGVars29_args
      ;
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars29_args = *(union
      __args_mapped_step5_dpbr_noGVars29_74*)rocrArgs(depv);
   *mapped_step5_dpbr_noGVars29_args.data.normr = *
      mapped_step5_dpbr_noGVars29_args.data._p;
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

unsigned long mapped_step5_dpbr_noGVars29_count_6(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul, i = 0l; i <= 8l; i++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars27_count_5(long* args)
{
   long _t1;
   unsigned long rcNbInputDeps;
   unsigned long rcNbInputDeps_1;
   long _t2;
   long i;
   for (_t1 = (long)args[0l],
        _t2 = (__mins_64(34l, 4l * _t1 + 3l)),
        rcNbInputDeps = 0ul + 1ul,
        i = 4l * _t1,
        rcNbInputDeps_1 = 0ul + 2ul;
        i <= _t2; i++, rcNbInputDeps++, rcNbInputDeps_1++) {
   }
   if (_t1 >= 1l) {
      rcNbInputDeps = rcNbInputDeps_1;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars24_count_4(long* args)
{
   unsigned long rcNbInputDeps;
   long _t1;
   long _t2;
   long i;
   for (_t1 = (long)args[0l],
        _t2 = (__mins_64(137l, 4l * _t1 + 3l)),
        rcNbInputDeps = 0ul + 2ul,
        i = 4l * _t1;
        i <= _t2; i++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars22_count_3(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul, i = 0l; i <= 17l; i++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars20_count_2(long* args)
{
   long _t1;
   unsigned long rcNbInputDeps;
   unsigned long rcNbInputDeps_1;
   long _t2;
   long i;
   for (_t1 = (long)args[0l],
        _t2 = (__mins_64(137l, 8l * _t1 + 7l)),
        rcNbInputDeps = 0ul + 1ul,
        i = 8l * _t1,
        rcNbInputDeps_1 = 0ul + 2ul;
        i <= _t2; i++, rcNbInputDeps++, rcNbInputDeps_1++) {
   }
   if (_t1 >= 1l) {
      rcNbInputDeps = rcNbInputDeps_1;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars17_count_1(long* args)
{
   return 0ul + 1ul;
}

static ocrGuid_t mapped_step5_dpbr_noGVars_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars_main_76
      mapped_step5_dpbr_noGVars_main_args;
   union __args_mapped_step5_dpbr_noGVars15_62* allArgs;
   void* _t1;
   int* _t2;
   void* _t3;
   double* _t4;
   void* _t5;
   int* _t6;
   void* _t7;
   int* _t8;
   void* _t9;
   int* _t10;
   void* _t11;
   double* _t12;
   void* _t13;
   int* _t14;
   void* _t15;
   double* _t16;
   void* _t17;
   double* _t18;
   void* _t19;
   ocrGuid_t _t20;
   union __args_mapped_step5_dpbr_noGVars15_62* _t21;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars_main_args = *(union
      __args_mapped_step5_dpbr_noGVars_main_76*)rocrArgs(depv);
   _t3 = malloc(4ul);
   _t5 = malloc(8ul);
   _t7 = malloc(4ul);
   _t9 = malloc(4ul);
   _t11 = malloc(4ul);
   _t13 = malloc(8ul);
   _t15 = malloc(4ul);
   _t17 = malloc(8ul);
   _t19 = malloc(8ul);
   _t20 = rocrAlloc((void**)&allArgs, 168ul);
   _t21 = allArgs;
   _t21->data.nPerLvl_l = mapped_step5_dpbr_noGVars_main_args.data.nPerLvl_l;
   _t2 = (int*)_t3;
   _t21->data._p = _t2;
   _t4 = (double*)_t5;
   _t21->data._p_1 = _t4;
   _t6 = (int*)_t7;
   _t21->data._p_2 = _t6;
   _t8 = (int*)_t9;
   _t21->data._p_3 = _t8;
   _t10 = (int*)_t11;
   _t21->data._p_4 = _t10;
   _t12 = (double*)_t13;
   _t21->data._p_5 = _t12;
   _t14 = (int*)_t15;
   _t21->data._p_6 = _t14;
   _t21->data.normr = mapped_step5_dpbr_noGVars_main_args.data.normr;
   _t21->data.r_l = mapped_step5_dpbr_noGVars_main_args.data.r_l;
   _t21->data.x = mapped_step5_dpbr_noGVars_main_args.data.x;
   _t21->data.p_l = mapped_step5_dpbr_noGVars_main_args.data.p_l;
   _t21->data.Ap_l = mapped_step5_dpbr_noGVars_main_args.data.Ap_l;
   _t21->data.pAp = mapped_step5_dpbr_noGVars_main_args.data.pAp;
   _t21->data.alpha = mapped_step5_dpbr_noGVars_main_args.data.alpha;
   _t21->data.rtz = mapped_step5_dpbr_noGVars_main_args.data.rtz;
   _t21->data.nnzPerRow_l = mapped_step5_dpbr_noGVars_main_args.data.
      nnzPerRow_l;
   _t21->data.colInds_l = mapped_step5_dpbr_noGVars_main_args.data.colInds_l;
   _t21->data.A_l = mapped_step5_dpbr_noGVars_main_args.data.A_l;
   _t16 = (double*)_t17;
   _t21->data._p_7 = _t16;
   _t18 = (double*)_t19;
   _t21->data._p_8 = _t18;
   rocrAutoDec(_t1, 0u, (unsigned long)0l, _t20, (unsigned long (*)(long*))((
      void*)0l), (void (*)(void*, long*))((void*)0l), (void (*)(void*, long*))(
      (void*)0l), 0u);
   rocrAsyncFree(_t2);
   rocrAsyncFree(_t4);
   rocrAsyncFree(_t6);
   rocrAsyncFree(_t8);
   rocrAsyncFree(_t10);
   rocrAsyncFree(_t12);
   rocrAsyncFree(_t14);
   rocrAsyncFree(_t16);
   rocrAsyncFree(_t18);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

/*
 * ../src/hpcg_precond_fissioned.c:684.6
 */
void mapped_step5_async(void* event, double* alpha, double rtz, double* pAp,
   double* x_33, double* normr_25)
{
   double rtz_var;
   rtz_var = rtz;
   mapped_step5_dpbr(alpha, &rtz_var, pAp, x_33, normr_25);
}


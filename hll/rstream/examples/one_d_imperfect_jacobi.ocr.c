/*
 * Optimization PARSE[-I//usr/include, -I/opt/reservoir/staging/rstream-3.15.0.1/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DOCR_TARGET=1, --c99, -D__RSTREAM_CC__, one_d_imperfect_jacobi.c] (358ms)
 * Optimization SSA (38ms)
 * Optimization CCP (12ms)
 * Optimization GVNGCM (44ms)
 *   Optimization SimplifyControl (13ms)
 *   Optimization SimplifyControl (2ms)
 *   Optimization SimplifyControl (2ms)
 *   Optimization SimplifyControl (2ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 *   Optimization SimplifyControl (1ms)
 * Optimization DCE (5ms)
 * Optimization Raise[-liftnone] (173ms)
 * Optimization PolyhedralMapperNew[threadf:reduce_create, no-simplify-loop, no-spatial-layout] (1268ms)
 * Optimization Lower (154ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
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
 *   Optimization DCE (2ms)
 * Optimization SyntaxPrepass (7ms)
 * Optimization OSR (14ms)
 * Optimization DCE (5ms)
 * Optimization DeSSA (64ms)
 */
#define SOCR true
#include <rstream_ocr.h>
#include <ocr.h>
typedef long __off_t;
typedef long __off64_t;
typedef unsigned long size_t;
struct _IO_FILE;
typedef float real_t;
typedef struct _IO_FILE FILE;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_21;
union __args_imp1_22;
struct va_list_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_23;
union __args_imp_main_24;
union ocrGuidUnion_t;
struct va_list_t;
struct _IO_marker;
typedef void _IO_lock_t;
struct _IO_FILE {
   int _flags;
   char* _IO_read_ptr;
   char* _IO_read_end;
   char* _IO_read_base;
   char* _IO_write_base;
   char* _IO_write_ptr;
   char* _IO_write_end;
   char* _IO_buf_base;
   char* _IO_buf_end;
   char* _IO_save_base;
   char* _IO_backup_base;
   char* _IO_save_end;
   struct _IO_marker* _markers;
   struct _IO_FILE* _chain;
   int _fileno;
   int _flags2;
   __off_t _old_offset;
   unsigned short _cur_column;
   signed char _vtable_offset;
   char _shortbuf[1];
   _IO_lock_t* _lock;
   __off64_t _offset;
   void* __pad1;
   void* __pad2;
   void* __pad3;
   void* __pad4;
   size_t __pad5;
   int _mode;
   char _unused2[20];
};
struct __anon_21 {
   real_t* A;
   real_t* B;
   int IT0;
   int IT1;
};
union __args_imp1_22 {
   struct __anon_21 data;
   double padding[3];
};
struct __anon_23 {
   real_t* B;
   real_t* A;
};
union __args_imp_main_24 {
   struct __anon_23 data;
   double padding[2];
};
struct _IO_marker {
   struct _IO_marker* _next;
   struct _IO_FILE* _sbuf;
   int _pos;
};
/*
 * Forward declarations of functions
 */
int check_vectors(char const*, char const*, void const*, void const*, int);
int fprintf(FILE*, char const*, ...);
void initialize(void);
void initialize_once(void);
void show(void);
void kernel(void);
int check(void);
double flops_per_trial(void);
void imp(real_t* A, real_t* B);
static ocrGuid_t imp1(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
ocrGuid_t rocrAlloc(void**, unsigned long);
unsigned long imp1_count_0(va_list args);
static ocrGuid_t imp_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv);
void rocrExit(void);
/*
 * Forward declarations of global variables
 */
extern int add_flop;
extern int mul_flop;
extern struct _IO_FILE* stderr;
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
extern struct _IO_FILE* stderr;
static real_t a[5000];
static real_t b[5000];
static real_t aa[5000];
static real_t bb[5000];
int nb_samples = 25000000;
char const* function_name = "one_d_imperfect_jacobi";
/*
 * Definitions of functions
 */
/*
 * one_d_imperfect_jacobi.c:54.6
 */
void initialize(void)
{
   int i;
   for (i = 0; i < 5000; i++) {
      real_t _t1;
      _t1 = (float)(i / 2);
      bb[i] = _t1;
      b[i] = _t1;
      aa[i] = _t1;
      a[i] = _t1;
   }
}

/*
 * one_d_imperfect_jacobi.c:62.6
 */
void initialize_once(void)
{
   initialize();
}

/*
 * one_d_imperfect_jacobi.c:66.6
 */
void show(void)
{
   int i;
   int i_1;
   for (i = 0; i < 8; i++) {
      fprintf(stderr, "e[%d] = %10.10f\n", i, (double)a[i]);
   }
   for (i_1 = 4992; i_1 < 5000; i_1++) {
      fprintf(stderr, "e[%d] = %10.10f\n", i_1, (double)a[i_1]);
   }
}

/*
 * one_d_imperfect_jacobi.c:83.6
 */
void kernel(void)
{
   union __args_imp_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_imp_main_24* _t2;
   rocrDeclareType(imp_main, 1, (unsigned int)1, 1, (unsigned long (*)(va_list)
      )((void*)0l));
   rocrDeclareType(imp1, 0, (unsigned int)2360, 0, imp1_count_0);
   rocrInit();
   _t1 = rocrAlloc((void**)&allArgs, 16ul);
   _t2 = allArgs;
   _t2->data.B = b;
   _t2->data.A = a;
   rocrExecute(1, _t1);
   rocrExit();
}

/*
 * one_d_imperfect_jacobi.c:87.5
 */
int check(void)
{
   int t;
   for (t = 0; t < 5000; t++) {
      int i;
      int i_1;
      int j;
      for (i_1 = 2, i = 1; i_1 < 4999; ) {
         float _t1;
         float _t2;
         int _t3;
         _t1 = aa[i];
         _t2 = aa[i_1];
         _t3 = i_1 + 1;
         bb[i_1] = 0.3330000043f * (_t1 + _t2 + aa[_t3]);
         i++;
         i_1 = _t3;
      }
      for (j = 2; j < 4999; j++) {
         aa[j] = bb[j];
      }
   }
   return check_vectors("a", "aa", a, aa, 5000);
}

/*
 * one_d_imperfect_jacobi.c:94.8
 */
double flops_per_trial(void)
{
   return (double)(mul_flop + add_flop) * 2.0 * 5000.0 * 5000.0;
}

void imp(real_t* A, real_t* B)
{
   union __args_imp_main_24* allArgs;
   ocrGuid_t _t1;
   union __args_imp_main_24* _t2;
   rocrDeclareType(imp_main, 1, (unsigned int)1, 1, (unsigned long (*)(va_list)
      )((void*)0l));
   rocrDeclareType(imp1, 0, (unsigned int)2360, 0, imp1_count_0);
   rocrInit();
   _t1 = rocrAlloc((void**)&allArgs, 16ul);
   _t2 = allArgs;
   _t2->data.B = B;
   _t2->data.A = A;
   rocrExecute(1, _t1);
   rocrExit();
}

static ocrGuid_t imp1(unsigned int paramc, unsigned long* paramv, unsigned int
   depc, ocrEdtDep_t* depv)
{
   union __args_imp1_22 imp1_args;
   union __args_imp1_22* allArgs;
   real_t* A;
   real_t* B;
   int IT0;
   int IT1;
   int _t1;
   int i;
   int i_1;
   int i_2;
   int _t2;
   int i_3;
   int i_4;
   int i_5;
   rocrParseInputArgs(paramc, paramv, depc, depv);
   for (imp1_args = *(union __args_imp1_22*)rocrArgs(depv),
        IT0 = imp1_args.data.IT0,
        _t1 = (__mins_32(-128 * IT0 + 4999, 127)),
        B = imp1_args.data.B,
        A = imp1_args.data.A,
        IT1 = imp1_args.data.IT1,
        i = (__maxs_32(0, -128 * IT0 + 128 * IT1 + -2498)),
        i_1 = i * -2,
        i_2 = i << 1;
        i <= _t1; i++, i_1 -= 2, i_2 += 2) {
      int _t3;
      int j;
      int j_1;
      int j_2;
      int j_3;
      int j_4;
      int j_5;
      int j_6;
      if (i + 128 * IT0 + -128 * IT1 >= 0) {
         B[2] = 0.3330000043f * (A[1] + A[2] + A[3]);
      }
      for (_t3 = (__mins_32(255, i_2 + 256 * IT0 + -256 * IT1 + 4996)),
           j = (__maxs_32(i_2 + 256 * IT0 + -256 * IT1 + 1, 0)),
           j_6 = j + i_1,
           j_5 = j + i_1,
           j_1 = j + i_1,
           j_2 = j + i_1,
           j_3 = j + i_1,
           j_4 = j + i_1;
           j <= _t3;
           j_6++,
           j_5++,
           j_4++,
           j_3++,
           j_2++,
           j_1++,
           j++) {
         B[j_1 + (-256 * IT0 + (256 * IT1 + 2))] = 0.3330000043f * (A[j_2 + (
            -256 * IT0 + (256 * IT1 + 1))] + A[j_3 + (-256 * IT0 + (256 * IT1 +
            2))] + A[j_4 + (-256 * IT0 + (256 * IT1 + 3))]);
         A[j_5 + (-256 * IT0 + (256 * IT1 + 1))] = B[j_6 + (-256 * IT0 + (256 *
            IT1 + 1))];
      }
      if (- i + -128 * IT0 + 128 * IT1 >= 2371) {
         A[4998] = B[4998];
      }
   }
   for (_t2 = (__mins_32(IT0 + IT1 + 1 >> 1, __mins_32(39, IT0 + 1))),
        i_3 = (__maxs_32(IT0 + IT1 + -57, __maxs_32(IT0, IT0 + IT1 + -19 + 1 >>
           1))),
        i_4 = i_3 * 59,
        i_5 = i_3 * -1 + IT0;
        i_3 <= _t2; i_5--, i_4 += 59, i_3++) {
      ocrGuid_t _t4;
      union __args_imp1_22* _t5;
      int _t6;
      _t4 = rocrAlloc((void**)&allArgs, 24ul);
      _t5 = allArgs;
      _t5->data.A = A;
      _t5->data.B = B;
      _t5->data.IT0 = i_3;
      _t6 = i_5 + (IT1 + 1);
      _t5->data.IT1 = _t6;
      rocrAutoDec(0, _t4, (unsigned int)(i_5 + (IT1 + 1) + i_4), i_3, _t6);
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

unsigned long imp1_count_0(va_list args)
{
   unsigned long rcNbInputDeps;
   int IT0;
   int IT1;
   int _t1;
   int i;
   for (IT0 = va_arg(args, int),
        IT1 = va_arg(args, int),
        _t1 = (__mins_32(IT0 + IT1 + -1 >> 1, IT0)),
        i = (__maxs_32(IT0 + -1, __maxs_32(0, IT0 + IT1 + -21 + 1 >> 1))),
        rcNbInputDeps = 0ul;
        i <= _t1; i++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

static ocrGuid_t imp_main(unsigned int paramc, unsigned long* paramv,
   unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_imp_main_24 imp_main_args;
   union __args_imp1_22* allArgs;
   ocrGuid_t _t1;
   union __args_imp1_22* _t2;
   rocrParseInputArgs(paramc, paramv, depc, depv);
   imp_main_args = *(union __args_imp_main_24*)rocrArgs(depv);
   _t1 = rocrAlloc((void**)&allArgs, 24ul);
   _t2 = allArgs;
   _t2->data.A = imp_main_args.data.A;
   _t2->data.B = imp_main_args.data.B;
   _t2->data.IT0 = 0;
   _t2->data.IT1 = 0;
   rocrCreateTask(0, _t1, (unsigned int)0, 0, 0);
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}


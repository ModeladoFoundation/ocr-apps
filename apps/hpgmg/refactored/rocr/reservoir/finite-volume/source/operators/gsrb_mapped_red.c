/*
 * Optimization PARSE[-I//usr/include, -I/home/konstantinidis/Work/ocr-db-autodec/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DbSIZE=4, -DH2INV=4.0, -D__RSTREAM_CC__, /home/konstantinidis/HPGMG/xstack-traleika-apps/sandbox/konstantinidis/hpgmg/finite-volume/source/operators/gsrb_mapped_kernel4_red.c] (233ms)
 * Optimization SSA (104ms)
 * Optimization CCP (50ms)
 * Optimization GVNGCM (74ms)
 *   Optimization SimplifyControl (17ms)
 * Optimization OSR (59ms)
 * Optimization DCE (13ms)
 * Optimization Raise[-liftnone] (707ms)
 * Optimization PolyhedralMapperNew[tile=sizes={8-16-16}] (3116ms)
 * Optimization Lower (592ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (13ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization InsertMapperAnnotations (2ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization DCE (49ms)
 * Optimization SyntaxPrepass (16ms)
 * Optimization DCE (36ms)
 * Optimization DeSSA (807ms)
 */
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_1_red_4;
union __args_gsrb_mapped_kernel4_red1_2;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_3_red_4;
union __args_gsrb_mapped_kernel4_red_main_4;
union ocrGuidUnion_t;
struct va_list_t;
struct __anon_1_red_4 {
   double (* phi)[4][4];
   double const (* alpha)[4][4];
   double const (* beta_i)[4][4];
   double const (* valid)[4][4];
   double const (* beta_j)[4][4];
   double const (* beta_k)[4][4];
   double const (* rhs)[4][4];
   int dim;
   int H;
   int IT0;
   int IT1;
   int IT2;
};
union __args_gsrb_mapped_kernel4_red1_2 {
   struct __anon_1_red_4 data;
   double padding[10];
};
struct __anon_3_red_4 {
   double (* phi)[4][4];
   double const (* alpha)[4][4];
   double const (* beta_i)[4][4];
   double const (* valid)[4][4];
   double const (* beta_j)[4][4];
   double const (* beta_k)[4][4];
   double const (* rhs)[4][4];
   int dim;
   int H;
};
union __args_gsrb_mapped_kernel4_red_main_4 {
   struct __anon_3_red_4 data;
   double padding[8];
};
/*
 * Forward declarations of functions
 */
void gsrb_mapped_kernel4_red(int dim, double (* phi)[4][4], double const (*
   Dinv)[4][4], double const (* rhs)[4][4], double a, int H, double const (*
   alpha)[4][4], double const (* beta_i)[4][4], double const (* beta_j)[4][4],
   double const (* beta_k)[4][4], double const (* valid)[4][4]);
static ocrGuid_t gsrb_mapped_kernel4_red1(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
ocrGuid_t rocrAlloc(void** dim, unsigned long k);
static ocrGuid_t gsrb_mapped_kernel4_red_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void rocrExit(void);
/*
 * Definitions of functions
 */
void gsrb_mapped_kernel4_red(int dim, double (* phi)[4][4], double const (*
   Dinv)[4][4], double const (* rhs)[4][4], double a, int H, double const (*
   alpha)[4][4], double const (* beta_i)[4][4], double const (* beta_j)[4][4],
   double const (* beta_k)[4][4], double const (* valid)[4][4])
{
   union __args_gsrb_mapped_kernel4_red_main_4* allArgs;
   rocrDeclareType(gsrb_mapped_kernel4_red_main, 1, (unsigned int)1, 1, (
      unsigned long (*)(va_list))((void*)0l));
   rocrDeclareType(gsrb_mapped_kernel4_red1, 0, (unsigned int)(dim >= 1 ? 1 + (
      -1 + dim >> 4) + (2 + 2 * (-1 + dim >> 4)) * (-1 + dim >> 5) + (1 + (-1 +
      dim >> 4)) * (-1 + dim >> 5) * (-1 + dim >> 5) : 0), 0, (unsigned long (*
      )(va_list))((void*)0l));
   rocrInit();
   if (dim >= 1) {
      ocrGuid_t _t1;
      union __args_gsrb_mapped_kernel4_red_main_4* _t2;
      _t1 = rocrAlloc((void**)&allArgs, 64ul);
      _t2 = allArgs;
      _t2->data.phi = phi;
      _t2->data.alpha = alpha;
      _t2->data.beta_i = beta_i;
      _t2->data.valid = valid;
      _t2->data.beta_j = beta_j;
      _t2->data.beta_k = beta_k;
      _t2->data.rhs = rhs;
      _t2->data.dim = dim;
      _t2->data.H = H;
      rocrExecute(1, _t1);
   }
   rocrExit();
}

static ocrGuid_t gsrb_mapped_kernel4_red1(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{ rocrParseInputArgs(paramc, paramv, depc, depv);
   union __args_gsrb_mapped_kernel4_red1_2 gsrb_mapped_kernel4_red1_args;
   double (* phi)[4][4];
   double const (* alpha)[4][4];
   double const (* beta_i)[4][4];
   double const (* valid)[4][4];
   double const (* beta_j)[4][4];
   double const (* beta_k)[4][4];
   double const (* rhs)[4][4];
   int dim;
   int H;
   int IT0;
   int IT1;
   int IT2;
   double _v1;
   gsrb_mapped_kernel4_red1_args = *(union __args_gsrb_mapped_kernel4_red1_2*
      )rocrArgs(depv);
   IT1 = gsrb_mapped_kernel4_red1_args.data.IT1;
   IT0 = gsrb_mapped_kernel4_red1_args.data.IT0;
   H = gsrb_mapped_kernel4_red1_args.data.H;
   rhs = gsrb_mapped_kernel4_red1_args.data.rhs;
   beta_k = gsrb_mapped_kernel4_red1_args.data.beta_k;
   beta_j = gsrb_mapped_kernel4_red1_args.data.beta_j;
   valid = gsrb_mapped_kernel4_red1_args.data.valid;
   beta_i = gsrb_mapped_kernel4_red1_args.data.beta_i;
   alpha = gsrb_mapped_kernel4_red1_args.data.alpha;
   phi = gsrb_mapped_kernel4_red1_args.data.phi;
   IT2 = gsrb_mapped_kernel4_red1_args.data.IT2;
   dim = gsrb_mapped_kernel4_red1_args.data.dim;
   if (dim + -32 * IT2 >= 2) {
      int _t1;
      int i;
      for (_t1 = (__mins_32(dim + -16 * IT0 + -1 >> 1, 7)), i = 0; i <= _t1; i
              ++) {
         int _t2;
         int j;
         for (_t2 = (__mins_32(dim + -32 * IT1 + -1 >> 1, 15)), j = 0; j <= _t2
                 ; j++) {
            int _t3;
            int k;
            for (_t3 = (__mins_32(dim + -32 * IT2 + -2 >> 1, 15)), k = 0; k <=
                    _t3; k++) {
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
               _t18 = phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2
                  + 1)];
               _t4 = 2.0 * _t18;
               _t17 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t16 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t15 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t14 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1)));
               _t13 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 2)));
               _t12 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  2 * k + (32 * IT2 + 2)));
               _t11 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t10 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  2 * k + (32 * IT2 + 1)));
               _t9 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + -1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t8 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 1)));
               _t7 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2 *
                  k + 32 * IT2));
               _t6 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 1)));
               _t5 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + 32 * IT1]
                  + (2 * k + (32 * IT2 + 1)));
               phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2 + 1)] =
                  _t18 + 1.0 / (_t5 - 1.0 * (double)H * (_t6 * (_t7 - 2.0) + _t8
                  * (_t9 - 2.0) + _t10 * (_t11 - 2.0) + _t12 * (_t13 - 2.0) +
                  _t14 * (_t15 - 2.0) + _t16 * (_t17 - 2.0))) * (*(double*)(rhs
                  [2 * i + 16 * IT0][2 * j + 32 * IT1] + (2 * k + (32 * IT2 + 1
                  ))) - (_t5 * _t18 - 1.0 * (double)H * (_t6 * (_t7 * (_t18 +
                  phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + 32 * IT2]) -
                  _t4) + _t8 * (_t9 * (_t18 + phi[2 * i + 16 * IT0][2 * j + (32
                  * IT1 + -1)][2 * k + (32 * IT2 + 1)]) - _t4) + _t10 * (_t11 *
                  (_t18 + phi[2 * i + (16 * IT0 + -1)][2 * j + 32 * IT1][2 * k +
                  (32 * IT2 + 1)]) - _t4) + _t12 * (_t13 * (_t18 + phi[2 * i +
                  16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2 + 2)]) - _t4) +
                  _t14 * (_t15 * (_t18 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)][2 * k + (32 * IT2 + 1)]) - _t4) + _t16 * (_t17 * (_t18 +
                  phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (32 *
                  IT2 + 1)]) - _t4))));
               _t33 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + 32 *
                  IT2];
               _t19 = 2.0 * _t33;
               _t32 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t31 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t30 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)
                  ] + (2 * k + 32 * IT2));
               _t29 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2
                  )] + (2 * k + 32 * IT2));
               _t28 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t27 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1)));
               _t26 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t25 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t24 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + 32 * IT2));
               _t23 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t22 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + -1)));
               _t21 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t20 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + 32 * IT2] =
                  _t33 + 1.0 / (_t20 - 1.0 * (double)H * (_t21 * (_t22 - 2.0) +
                  _t23 * (_t24 - 2.0) + _t25 * (_t26 - 2.0) + _t27 * (_t28 - 2.0
                  ) + _t29 * (_t30 - 2.0) + _t31 * (_t32 - 2.0))) * (*(double*)
                  (rhs[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + (2 * k + 32 *
                  IT2)) - (_t20 * _t33 - 1.0 * (double)H * (_t21 * (_t22 * (
                  _t33 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (
                  32 * IT2 + -1)]) - _t19) + _t23 * (_t24 * (_t33 + phi[2 * i +
                  16 * IT0][2 * j + 32 * IT1][2 * k + 32 * IT2]) - _t19) + _t25
                  * (_t26 * (_t33 + phi[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)][2 * k + 32 * IT2]) - _t19) + _t27 * (_t28 * (_t33 +
                  phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (32 *
                  IT2 + 1)]) - _t19) + _t29 * (_t30 * (_t33 + phi[2 * i + 16 *
                  IT0][2 * j + (32 * IT1 + 2)][2 * k + 32 * IT2]) - _t19) + _t31
                  * (_t32 * (_t33 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)][2 * k + 32 * IT2]) - _t19))));
               _t48 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + 32 *
                  IT2];
               _t34 = 2.0 * _t48;
               _t47 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t46 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t45 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t44 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t43 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t42 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t41 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + 32 * IT2));
               _t40 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t39 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + -1)] + (2 * k + 32 * IT2));
               _t38 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t37 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + -1)));
               _t36 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t35 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1] + (2 * k + 32 * IT2));
               phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + 32 * IT2] =
                  _t48 + 1.0 / (_t35 - 1.0 * (double)H * (_t36 * (_t37 - 2.0) +
                  _t38 * (_t39 - 2.0) + _t40 * (_t41 - 2.0) + _t42 * (_t43 - 2.0
                  ) + _t44 * (_t45 - 2.0) + _t46 * (_t47 - 2.0))) * (*(double*)
                  (rhs[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + (2 * k + 32 *
                  IT2)) - (_t35 * _t48 - 1.0 * (double)H * (_t36 * (_t37 * (
                  _t48 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (
                  32 * IT2 + -1)]) - _t34) + _t38 * (_t39 * (_t48 + phi[2 * i +
                  (16 * IT0 + 1)][2 * j + (32 * IT1 + -1)][2 * k + 32 * IT2]) -
                  _t34) + _t40 * (_t41 * (_t48 + phi[2 * i + 16 * IT0][2 * j +
                  32 * IT1][2 * k + 32 * IT2]) - _t34) + _t42 * (_t43 * (_t48 +
                  phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (32 *
                  IT2 + 1)]) - _t34) + _t44 * (_t45 * (_t48 + phi[2 * i + (16 *
                  IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k + 32 * IT2]) - _t34) +
                  _t46 * (_t47 * (_t48 + phi[2 * i + (16 * IT0 + 2)][2 * j + 32
                  * IT1][2 * k + 32 * IT2]) - _t34))));
               _t63 = phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k
                  + (32 * IT2 + 1)];
               _t49 = 2.0 * _t63;
               _t62 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + (32 * IT1
                  + 1)] + (2 * k + (32 * IT2 + 1)));
               _t61 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t60 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 2)] + (2 * k + (32 * IT2 + 1)));
               _t59 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 2)] + (2 * k + (32 * IT2 + 1)));
               _t58 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + (32 * IT2 + 2)));
               _t57 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 2)));
               _t56 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t55 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t54 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t53 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t52 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t51 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t50 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + (
                  32 * IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k + (32 *
                  IT2 + 1)] = _t63 + 1.0 / (_t50 - 1.0 * (double)H * (_t51 * (
                  _t52 - 2.0) + _t53 * (_t54 - 2.0) + _t55 * (_t56 - 2.0) + _t57
                  * (_t58 - 2.0) + _t59 * (_t60 - 2.0) + _t61 * (_t62 - 2.0))) *
                  (*(double*)(rhs[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1))) - (_t50 * _t63 - 1.0 * (double
                  )H * (_t51 * (_t52 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j
                  + (32 * IT1 + 1)][2 * k + 32 * IT2]) - _t49) + _t53 * (_t54 *
                  (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k +
                  (32 * IT2 + 1)]) - _t49) + _t55 * (_t56 * (_t63 + phi[2 * i +
                  16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (32 * IT2 + 1)]) -
                  _t49) + _t57 * (_t58 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 *
                  j + (32 * IT1 + 1)][2 * k + (32 * IT2 + 2)]) - _t49) + _t59 *
                  (_t60 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 2)][2 * k + (32 * IT2 + 1)]) - _t49) + _t61 * (_t62 * (_t63
                  + phi[2 * i + (16 * IT0 + 2)][2 * j + (32 * IT1 + 1)][2 * k +
                  (32 * IT2 + 1)]) - _t49))));
            }
            if (- dim + 32 * IT2 >= -31 && (dim + -32 * IT2 >= 3 && (dim + 1 & 1
                   ) == 0)) {
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
               _t78 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -1];
               _t64 = 2.0 * _t78;
               _t77 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               _t76 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t75 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)
                  ] + (dim + -1));
               _t74 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2
                  )] + (dim + -1));
               _t73 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + dim);
               _t72 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + dim);
               _t71 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t70 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t69 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  dim + -1));
               _t68 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t67 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (dim + -2));
               _t66 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t65 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -1] = _t78 +
                  1.0 / (_t65 - 1.0 * (double)H * (_t66 * (_t67 - 2.0) + _t68 *
                  (_t69 - 2.0) + _t70 * (_t71 - 2.0) + _t72 * (_t73 - 2.0) +
                  _t74 * (_t75 - 2.0) + _t76 * (_t77 - 2.0))) * (*(double*)(rhs
                  [2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + (dim + -1)) - (
                  _t65 * _t78 - 1.0 * (double)H * (_t66 * (_t67 * (_t78 + phi[2
                  * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -2]) - _t64) +
                  _t68 * (_t69 * (_t78 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1
                  ][dim + -1]) - _t64) + _t70 * (_t71 * (_t78 + phi[2 * i + (16
                  * IT0 + -1)][2 * j + (32 * IT1 + 1)][dim + -1]) - _t64) + _t72
                  * (_t73 * (_t78 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )][dim]) - _t64) + _t74 * (_t75 * (_t78 + phi[2 * i + 16 * IT0
                  ][2 * j + (32 * IT1 + 2)][dim + -1]) - _t64) + _t76 * (_t77 *
                  (_t78 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][
                  dim + -1]) - _t64))));
               _t93 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -1];
               _t79 = 2.0 * _t93;
               _t92 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t91 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t90 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               _t89 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t88 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + dim);
               _t87 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + dim);
               _t86 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  dim + -1));
               _t85 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t84 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + -1)] + (dim + -1));
               _t83 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t82 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -2));
               _t81 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t80 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1] + (dim + -1));
               phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -1] = _t93 +
                  1.0 / (_t80 - 1.0 * (double)H * (_t81 * (_t82 - 2.0) + _t83 *
                  (_t84 - 2.0) + _t85 * (_t86 - 2.0) + _t87 * (_t88 - 2.0) +
                  _t89 * (_t90 - 2.0) + _t91 * (_t92 - 2.0))) * (*(double*)(rhs
                  [2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + (dim + -1)) - (
                  _t80 * _t93 - 1.0 * (double)H * (_t81 * (_t82 * (_t93 + phi[2
                  * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -2]) - _t79) +
                  _t83 * (_t84 * (_t93 + phi[2 * i + (16 * IT0 + 1)][2 * j + (
                  32 * IT1 + -1)][dim + -1]) - _t79) + _t85 * (_t86 * (_t93 +
                  phi[2 * i + 16 * IT0][2 * j + 32 * IT1][dim + -1]) - _t79) +
                  _t87 * (_t88 * (_t93 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1][dim]) - _t79) + _t89 * (_t90 * (_t93 + phi[2 * i + (16
                  * IT0 + 1)][2 * j + (32 * IT1 + 1)][dim + -1]) - _t79) + _t91
                  * (_t92 * (_t93 + phi[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ][dim + -1]) - _t79))));
            }
         }
      }
   }
   if (dim + -32 * IT2 == 1) {
      int _t94;
      int i;
      for (_t94 = (__mins_32(-8 * IT0 + 16 * IT2, 7)), i = 0; i <= _t94; i++) {
         int _t95;
         int j;
         for (_t95 = (__mins_32(-16 * IT1 + 16 * IT2, 15)), j = 0; j <= _t95; j
                 ++) {
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
            _t110 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2];
            _t96 = 2.0 * _t110;
            _t109 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            _t108 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t107 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)] +
               32 * IT2);
            _t106 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)]
               + 32 * IT2);
            _t105 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               (32 * IT2 + 1));
            _t104 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + (32 * IT2 + 1));
            _t103 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t102 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + 32 * IT2);
            _t101 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + 32 *
               IT2);
            _t100 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + 32 * IT2);
            _t99 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               (32 * IT2 + -1));
            _t98 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               32 * IT2);
            _t97 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2] = _t110 +
               1.0 / (_t97 - 1.0 * (double)H * (_t98 * (_t99 - 2.0) + _t100 * (
               _t101 - 2.0) + _t102 * (_t103 - 2.0) + _t104 * (_t105 - 2.0) +
               _t106 * (_t107 - 2.0) + _t108 * (_t109 - 2.0))) * (*(double*)(
               rhs[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + 32 * IT2) - (
               _t97 * _t110 - 1.0 * (double)H * (_t98 * (_t99 * (_t110 + phi[2 *
               i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2 + -1]) - _t96) +
               _t100 * (_t101 * (_t110 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1
               ][32 * IT2]) - _t96) + _t102 * (_t103 * (_t110 + phi[2 * i + (16
               * IT0 + -1)][2 * j + (32 * IT1 + 1)][32 * IT2]) - _t96) + _t104 *
               (_t105 * (_t110 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][
               32 * IT2 + 1]) - _t96) + _t106 * (_t107 * (_t110 + phi[2 * i + 16
               * IT0][2 * j + (32 * IT1 + 2)][32 * IT2]) - _t96) + _t108 * (
               _t109 * (_t110 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)][32 * IT2]) - _t96))));
            _t125 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2];
            _t111 = 2.0 * _t125;
            _t124 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1] +
               32 * IT2);
            _t123 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t122 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            _t121 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t120 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] +
               (32 * IT2 + 1));
            _t119 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + (32 * IT2 + 1));
            _t118 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + 32 *
               IT2);
            _t117 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t116 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               -1)] + 32 * IT2);
            _t115 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t114 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] +
               (32 * IT2 + -1));
            _t113 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t112 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32 *
               IT1] + 32 * IT2);
            phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2] = _t125 +
               1.0 / (_t112 - 1.0 * (double)H * (_t113 * (_t114 - 2.0) + _t115 *
               (_t116 - 2.0) + _t117 * (_t118 - 2.0) + _t119 * (_t120 - 2.0) +
               _t121 * (_t122 - 2.0) + _t123 * (_t124 - 2.0))) * (*(double*)(
               rhs[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + 32 * IT2) - (
               _t112 * _t125 - 1.0 * (double)H * (_t113 * (_t114 * (_t125 + phi
               [2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2 + -1]) -
               _t111) + _t115 * (_t116 * (_t125 + phi[2 * i + (16 * IT0 + 1)][2
               * j + (32 * IT1 + -1)][32 * IT2]) - _t111) + _t117 * (_t118 * (
               _t125 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1][32 * IT2]) -
               _t111) + _t119 * (_t120 * (_t125 + phi[2 * i + (16 * IT0 + 1)][2
               * j + 32 * IT1][32 * IT2 + 1]) - _t111) + _t121 * (_t122 * (
               _t125 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][32 *
               IT2]) - _t111) + _t123 * (_t124 * (_t125 + phi[2 * i + (16 * IT0
               + 2)][2 * j + 32 * IT1][32 * IT2]) - _t111))));
         }
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t gsrb_mapped_kernel4_red_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{ rocrParseInputArgs(paramc, paramv, depc, depv);
   union __args_gsrb_mapped_kernel4_red_main_4
      gsrb_mapped_kernel4_red_main_args;
   union __args_gsrb_mapped_kernel4_red1_2* allArgs;
   double (* phi)[4][4];
   double const (* alpha)[4][4];
   double const (* beta_i)[4][4];
   double const (* valid)[4][4];
   double const (* beta_j)[4][4];
   double const (* beta_k)[4][4];
   double const (* rhs)[4][4];
   int dim;
   int H;
   int _t1;
   int i;
   for (gsrb_mapped_kernel4_red_main_args = *(union
           __args_gsrb_mapped_kernel4_red_main_4*)rocrArgs(depv),
        dim = gsrb_mapped_kernel4_red_main_args.data.dim,
        _t1 = dim + -1 >> 4,
        H = gsrb_mapped_kernel4_red_main_args.data.H,
        rhs = gsrb_mapped_kernel4_red_main_args.data.rhs,
        beta_k = gsrb_mapped_kernel4_red_main_args.data.beta_k,
        beta_j = gsrb_mapped_kernel4_red_main_args.data.beta_j,
        valid = gsrb_mapped_kernel4_red_main_args.data.valid,
        beta_i = gsrb_mapped_kernel4_red_main_args.data.beta_i,
        alpha = gsrb_mapped_kernel4_red_main_args.data.alpha,
        phi = gsrb_mapped_kernel4_red_main_args.data.phi,
        i = 0;
        i <= _t1; i++) {
      int _t2;
      int j;
      for (_t2 = dim + -1 >> 5, j = 0; j <= _t2; j++) {
         int _t3;
         int k;
         for (_t3 = dim + -1 >> 5, k = 0; k <= _t3; k++) {
            ocrGuid_t _t4;
            union __args_gsrb_mapped_kernel4_red1_2* _t5;
            _t4 = rocrAlloc((void**)&allArgs, 80ul);
            _t5 = allArgs;
            _t5->data.phi = phi;
            _t5->data.alpha = alpha;
            _t5->data.beta_i = beta_i;
            _t5->data.valid = valid;
            _t5->data.beta_j = beta_j;
            _t5->data.beta_k = beta_k;
            _t5->data.rhs = rhs;
            _t5->data.dim = dim;
            _t5->data.H = H;
            _t5->data.IT0 = i;
            _t5->data.IT1 = j;
            _t5->data.IT2 = k;
            rocrCreateTask(0, _t4, (unsigned int)(k + j + i + (j + 2 * i) * (-1
               + dim >> 5) + i * (-1 + dim >> 5) * (-1 + dim >> 5)), dim, H, i,
                j, k);
         }
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

/*
 * Optimization PARSE[-I//usr/include, -I/home/konstantinidis/Work/ocr-db-autodec/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DbSIZE=6, -DH2INV=16.0, -D__RSTREAM_CC__, /home/konstantinidis/HPGMG/xstack-traleika-apps/sandbox/konstantinidis/hpgmg/finite-volume/source/operators/gsrb_mapped_kernel6_red.c] (231ms)
 * Optimization SSA (96ms)
 * Optimization CCP (50ms)
 * Optimization GVNGCM (74ms)
 *   Optimization SimplifyControl (16ms)
 * Optimization OSR (58ms)
 * Optimization DCE (12ms)
 * Optimization Raise[-liftnone] (697ms)
 * Optimization PolyhedralMapperNew[tile=sizes={8-16-16}] (3225ms)
 * Optimization Lower (546ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (13ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization InsertMapperAnnotations (2ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization DCE (49ms)
 * Optimization SyntaxPrepass (16ms)
 * Optimization DCE (36ms)
 * Optimization DeSSA (817ms)
 */
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_1_red_6;
union __args_gsrb_mapped_kernel6_red1_2;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_3_red_6;
union __args_gsrb_mapped_kernel6_red_main_4;
union ocrGuidUnion_t;
struct va_list_t;
struct __anon_1_red_6 {
   double (* phi)[6][6];
   double const (* alpha)[6][6];
   double const (* beta_i)[6][6];
   double const (* valid)[6][6];
   double const (* beta_j)[6][6];
   double const (* beta_k)[6][6];
   double const (* rhs)[6][6];
   int dim;
   int H;
   int IT0;
   int IT1;
   int IT2;
};
union __args_gsrb_mapped_kernel6_red1_2 {
   struct __anon_1_red_6 data;
   double padding[10];
};
struct __anon_3_red_6 {
   double (* phi)[6][6];
   double const (* alpha)[6][6];
   double const (* beta_i)[6][6];
   double const (* valid)[6][6];
   double const (* beta_j)[6][6];
   double const (* beta_k)[6][6];
   double const (* rhs)[6][6];
   int dim;
   int H;
};
union __args_gsrb_mapped_kernel6_red_main_4 {
   struct __anon_3_red_6 data;
   double padding[8];
};
/*
 * Forward declarations of functions
 */
void gsrb_mapped_kernel6_red(int dim, double (* phi)[6][6], double const (*
   Dinv)[6][6], double const (* rhs)[6][6], double a, int H, double const (*
   alpha)[6][6], double const (* beta_i)[6][6], double const (* beta_j)[6][6],
   double const (* beta_k)[6][6], double const (* valid)[6][6]);
static ocrGuid_t gsrb_mapped_kernel6_red1(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
ocrGuid_t rocrAlloc(void** dim, unsigned long k);
static ocrGuid_t gsrb_mapped_kernel6_red_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void rocrExit(void);
/*
 * Definitions of functions
 */
void gsrb_mapped_kernel6_red(int dim, double (* phi)[6][6], double const (*
   Dinv)[6][6], double const (* rhs)[6][6], double a, int H, double const (*
   alpha)[6][6], double const (* beta_i)[6][6], double const (* beta_j)[6][6],
   double const (* beta_k)[6][6], double const (* valid)[6][6])
{
   union __args_gsrb_mapped_kernel6_red_main_4* allArgs;
   rocrDeclareType(gsrb_mapped_kernel6_red_main, 1, (unsigned int)1, 1, (
      unsigned long (*)(va_list))((void*)0l));
   rocrDeclareType(gsrb_mapped_kernel6_red1, 0, (unsigned int)(dim >= 1 ? 1 + (
      -1 + dim >> 4) + (2 + 2 * (-1 + dim >> 4)) * (-1 + dim >> 5) + (1 + (-1 +
      dim >> 4)) * (-1 + dim >> 5) * (-1 + dim >> 5) : 0), 0, (unsigned long (*
      )(va_list))((void*)0l));
   rocrInit();
   if (dim >= 1) {
      ocrGuid_t _t1;
      union __args_gsrb_mapped_kernel6_red_main_4* _t2;
      _t1 = rocrAlloc((void**)&allArgs, 64ul);
      _t2 = allArgs;
      _t2->data.phi = phi;
      _t2->data.alpha = alpha;
      _t2->data.beta_i = beta_i;
      _t2->data.valid = valid;
      _t2->data.beta_j = beta_j;
      _t2->data.beta_k = beta_k;
      _t2->data.rhs = rhs;
      _t2->data.dim = dim;
      _t2->data.H = H;
      rocrExecute(1, _t1);
   }
   rocrExit();
}

static ocrGuid_t gsrb_mapped_kernel6_red1(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{ rocrParseInputArgs(paramc, paramv, depc, depv);
   union __args_gsrb_mapped_kernel6_red1_2 gsrb_mapped_kernel6_red1_args;
   double (* phi)[6][6];
   double const (* alpha)[6][6];
   double const (* beta_i)[6][6];
   double const (* valid)[6][6];
   double const (* beta_j)[6][6];
   double const (* beta_k)[6][6];
   double const (* rhs)[6][6];
   int dim;
   int H;
   int IT0;
   int IT1;
   int IT2;
   double _v1;
   gsrb_mapped_kernel6_red1_args = *(union __args_gsrb_mapped_kernel6_red1_2*
      )rocrArgs(depv);
   IT1 = gsrb_mapped_kernel6_red1_args.data.IT1;
   IT0 = gsrb_mapped_kernel6_red1_args.data.IT0;
   H = gsrb_mapped_kernel6_red1_args.data.H;
   rhs = gsrb_mapped_kernel6_red1_args.data.rhs;
   beta_k = gsrb_mapped_kernel6_red1_args.data.beta_k;
   beta_j = gsrb_mapped_kernel6_red1_args.data.beta_j;
   valid = gsrb_mapped_kernel6_red1_args.data.valid;
   beta_i = gsrb_mapped_kernel6_red1_args.data.beta_i;
   alpha = gsrb_mapped_kernel6_red1_args.data.alpha;
   phi = gsrb_mapped_kernel6_red1_args.data.phi;
   IT2 = gsrb_mapped_kernel6_red1_args.data.IT2;
   dim = gsrb_mapped_kernel6_red1_args.data.dim;
   if (dim + -32 * IT2 >= 2) {
      int _t1;
      int i;
      for (_t1 = (__mins_32(dim + -16 * IT0 + -1 >> 1, 7)), i = 0; i <= _t1; i
              ++) {
         int _t2;
         int j;
         for (_t2 = (__mins_32(dim + -32 * IT1 + -1 >> 1, 15)), j = 0; j <= _t2
                 ; j++) {
            int _t3;
            int k;
            for (_t3 = (__mins_32(dim + -32 * IT2 + -2 >> 1, 15)), k = 0; k <=
                    _t3; k++) {
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
               _t18 = phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2
                  + 1)];
               _t4 = 2.0 * _t18;
               _t17 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t16 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t15 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t14 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1)));
               _t13 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 2)));
               _t12 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  2 * k + (32 * IT2 + 2)));
               _t11 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t10 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  2 * k + (32 * IT2 + 1)));
               _t9 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + -1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t8 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 1)));
               _t7 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2 *
                  k + 32 * IT2));
               _t6 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 1)));
               _t5 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + 32 * IT1]
                  + (2 * k + (32 * IT2 + 1)));
               phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2 + 1)] =
                  _t18 + 1.0 / (_t5 - 1.0 * (double)H * (_t6 * (_t7 - 2.0) + _t8
                  * (_t9 - 2.0) + _t10 * (_t11 - 2.0) + _t12 * (_t13 - 2.0) +
                  _t14 * (_t15 - 2.0) + _t16 * (_t17 - 2.0))) * (*(double*)(rhs
                  [2 * i + 16 * IT0][2 * j + 32 * IT1] + (2 * k + (32 * IT2 + 1
                  ))) - (_t5 * _t18 - 1.0 * (double)H * (_t6 * (_t7 * (_t18 +
                  phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + 32 * IT2]) -
                  _t4) + _t8 * (_t9 * (_t18 + phi[2 * i + 16 * IT0][2 * j + (32
                  * IT1 + -1)][2 * k + (32 * IT2 + 1)]) - _t4) + _t10 * (_t11 *
                  (_t18 + phi[2 * i + (16 * IT0 + -1)][2 * j + 32 * IT1][2 * k +
                  (32 * IT2 + 1)]) - _t4) + _t12 * (_t13 * (_t18 + phi[2 * i +
                  16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2 + 2)]) - _t4) +
                  _t14 * (_t15 * (_t18 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)][2 * k + (32 * IT2 + 1)]) - _t4) + _t16 * (_t17 * (_t18 +
                  phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (32 *
                  IT2 + 1)]) - _t4))));
               _t33 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + 32 *
                  IT2];
               _t19 = 2.0 * _t33;
               _t32 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t31 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t30 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)
                  ] + (2 * k + 32 * IT2));
               _t29 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2
                  )] + (2 * k + 32 * IT2));
               _t28 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t27 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1)));
               _t26 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t25 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t24 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + 32 * IT2));
               _t23 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t22 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + -1)));
               _t21 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t20 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + 32 * IT2] =
                  _t33 + 1.0 / (_t20 - 1.0 * (double)H * (_t21 * (_t22 - 2.0) +
                  _t23 * (_t24 - 2.0) + _t25 * (_t26 - 2.0) + _t27 * (_t28 - 2.0
                  ) + _t29 * (_t30 - 2.0) + _t31 * (_t32 - 2.0))) * (*(double*)
                  (rhs[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + (2 * k + 32 *
                  IT2)) - (_t20 * _t33 - 1.0 * (double)H * (_t21 * (_t22 * (
                  _t33 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (
                  32 * IT2 + -1)]) - _t19) + _t23 * (_t24 * (_t33 + phi[2 * i +
                  16 * IT0][2 * j + 32 * IT1][2 * k + 32 * IT2]) - _t19) + _t25
                  * (_t26 * (_t33 + phi[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)][2 * k + 32 * IT2]) - _t19) + _t27 * (_t28 * (_t33 +
                  phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (32 *
                  IT2 + 1)]) - _t19) + _t29 * (_t30 * (_t33 + phi[2 * i + 16 *
                  IT0][2 * j + (32 * IT1 + 2)][2 * k + 32 * IT2]) - _t19) + _t31
                  * (_t32 * (_t33 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)][2 * k + 32 * IT2]) - _t19))));
               _t48 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + 32 *
                  IT2];
               _t34 = 2.0 * _t48;
               _t47 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t46 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t45 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t44 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t43 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t42 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t41 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + 32 * IT2));
               _t40 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t39 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + -1)] + (2 * k + 32 * IT2));
               _t38 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t37 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + -1)));
               _t36 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t35 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1] + (2 * k + 32 * IT2));
               phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + 32 * IT2] =
                  _t48 + 1.0 / (_t35 - 1.0 * (double)H * (_t36 * (_t37 - 2.0) +
                  _t38 * (_t39 - 2.0) + _t40 * (_t41 - 2.0) + _t42 * (_t43 - 2.0
                  ) + _t44 * (_t45 - 2.0) + _t46 * (_t47 - 2.0))) * (*(double*)
                  (rhs[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + (2 * k + 32 *
                  IT2)) - (_t35 * _t48 - 1.0 * (double)H * (_t36 * (_t37 * (
                  _t48 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (
                  32 * IT2 + -1)]) - _t34) + _t38 * (_t39 * (_t48 + phi[2 * i +
                  (16 * IT0 + 1)][2 * j + (32 * IT1 + -1)][2 * k + 32 * IT2]) -
                  _t34) + _t40 * (_t41 * (_t48 + phi[2 * i + 16 * IT0][2 * j +
                  32 * IT1][2 * k + 32 * IT2]) - _t34) + _t42 * (_t43 * (_t48 +
                  phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (32 *
                  IT2 + 1)]) - _t34) + _t44 * (_t45 * (_t48 + phi[2 * i + (16 *
                  IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k + 32 * IT2]) - _t34) +
                  _t46 * (_t47 * (_t48 + phi[2 * i + (16 * IT0 + 2)][2 * j + 32
                  * IT1][2 * k + 32 * IT2]) - _t34))));
               _t63 = phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k
                  + (32 * IT2 + 1)];
               _t49 = 2.0 * _t63;
               _t62 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + (32 * IT1
                  + 1)] + (2 * k + (32 * IT2 + 1)));
               _t61 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t60 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 2)] + (2 * k + (32 * IT2 + 1)));
               _t59 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 2)] + (2 * k + (32 * IT2 + 1)));
               _t58 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + (32 * IT2 + 2)));
               _t57 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 2)));
               _t56 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t55 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t54 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t53 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t52 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t51 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t50 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + (
                  32 * IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k + (32 *
                  IT2 + 1)] = _t63 + 1.0 / (_t50 - 1.0 * (double)H * (_t51 * (
                  _t52 - 2.0) + _t53 * (_t54 - 2.0) + _t55 * (_t56 - 2.0) + _t57
                  * (_t58 - 2.0) + _t59 * (_t60 - 2.0) + _t61 * (_t62 - 2.0))) *
                  (*(double*)(rhs[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1))) - (_t50 * _t63 - 1.0 * (double
                  )H * (_t51 * (_t52 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j
                  + (32 * IT1 + 1)][2 * k + 32 * IT2]) - _t49) + _t53 * (_t54 *
                  (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k +
                  (32 * IT2 + 1)]) - _t49) + _t55 * (_t56 * (_t63 + phi[2 * i +
                  16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (32 * IT2 + 1)]) -
                  _t49) + _t57 * (_t58 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 *
                  j + (32 * IT1 + 1)][2 * k + (32 * IT2 + 2)]) - _t49) + _t59 *
                  (_t60 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 2)][2 * k + (32 * IT2 + 1)]) - _t49) + _t61 * (_t62 * (_t63
                  + phi[2 * i + (16 * IT0 + 2)][2 * j + (32 * IT1 + 1)][2 * k +
                  (32 * IT2 + 1)]) - _t49))));
            }
            if (- dim + 32 * IT2 >= -31 && (dim + -32 * IT2 >= 3 && (dim + 1 & 1
                   ) == 0)) {
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
               _t78 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -1];
               _t64 = 2.0 * _t78;
               _t77 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               _t76 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t75 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)
                  ] + (dim + -1));
               _t74 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2
                  )] + (dim + -1));
               _t73 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + dim);
               _t72 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + dim);
               _t71 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t70 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t69 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  dim + -1));
               _t68 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t67 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (dim + -2));
               _t66 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t65 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -1] = _t78 +
                  1.0 / (_t65 - 1.0 * (double)H * (_t66 * (_t67 - 2.0) + _t68 *
                  (_t69 - 2.0) + _t70 * (_t71 - 2.0) + _t72 * (_t73 - 2.0) +
                  _t74 * (_t75 - 2.0) + _t76 * (_t77 - 2.0))) * (*(double*)(rhs
                  [2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + (dim + -1)) - (
                  _t65 * _t78 - 1.0 * (double)H * (_t66 * (_t67 * (_t78 + phi[2
                  * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -2]) - _t64) +
                  _t68 * (_t69 * (_t78 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1
                  ][dim + -1]) - _t64) + _t70 * (_t71 * (_t78 + phi[2 * i + (16
                  * IT0 + -1)][2 * j + (32 * IT1 + 1)][dim + -1]) - _t64) + _t72
                  * (_t73 * (_t78 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )][dim]) - _t64) + _t74 * (_t75 * (_t78 + phi[2 * i + 16 * IT0
                  ][2 * j + (32 * IT1 + 2)][dim + -1]) - _t64) + _t76 * (_t77 *
                  (_t78 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][
                  dim + -1]) - _t64))));
               _t93 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -1];
               _t79 = 2.0 * _t93;
               _t92 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t91 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t90 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               _t89 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t88 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + dim);
               _t87 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + dim);
               _t86 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  dim + -1));
               _t85 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t84 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + -1)] + (dim + -1));
               _t83 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t82 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -2));
               _t81 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t80 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1] + (dim + -1));
               phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -1] = _t93 +
                  1.0 / (_t80 - 1.0 * (double)H * (_t81 * (_t82 - 2.0) + _t83 *
                  (_t84 - 2.0) + _t85 * (_t86 - 2.0) + _t87 * (_t88 - 2.0) +
                  _t89 * (_t90 - 2.0) + _t91 * (_t92 - 2.0))) * (*(double*)(rhs
                  [2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + (dim + -1)) - (
                  _t80 * _t93 - 1.0 * (double)H * (_t81 * (_t82 * (_t93 + phi[2
                  * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -2]) - _t79) +
                  _t83 * (_t84 * (_t93 + phi[2 * i + (16 * IT0 + 1)][2 * j + (
                  32 * IT1 + -1)][dim + -1]) - _t79) + _t85 * (_t86 * (_t93 +
                  phi[2 * i + 16 * IT0][2 * j + 32 * IT1][dim + -1]) - _t79) +
                  _t87 * (_t88 * (_t93 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1][dim]) - _t79) + _t89 * (_t90 * (_t93 + phi[2 * i + (16
                  * IT0 + 1)][2 * j + (32 * IT1 + 1)][dim + -1]) - _t79) + _t91
                  * (_t92 * (_t93 + phi[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ][dim + -1]) - _t79))));
            }
         }
      }
   }
   if (dim + -32 * IT2 == 1) {
      int _t94;
      int i;
      for (_t94 = (__mins_32(-8 * IT0 + 16 * IT2, 7)), i = 0; i <= _t94; i++) {
         int _t95;
         int j;
         for (_t95 = (__mins_32(-16 * IT1 + 16 * IT2, 15)), j = 0; j <= _t95; j
                 ++) {
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
            _t110 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2];
            _t96 = 2.0 * _t110;
            _t109 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            _t108 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t107 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)] +
               32 * IT2);
            _t106 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)]
               + 32 * IT2);
            _t105 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               (32 * IT2 + 1));
            _t104 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + (32 * IT2 + 1));
            _t103 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t102 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + 32 * IT2);
            _t101 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + 32 *
               IT2);
            _t100 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + 32 * IT2);
            _t99 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               (32 * IT2 + -1));
            _t98 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               32 * IT2);
            _t97 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2] = _t110 +
               1.0 / (_t97 - 1.0 * (double)H * (_t98 * (_t99 - 2.0) + _t100 * (
               _t101 - 2.0) + _t102 * (_t103 - 2.0) + _t104 * (_t105 - 2.0) +
               _t106 * (_t107 - 2.0) + _t108 * (_t109 - 2.0))) * (*(double*)(
               rhs[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + 32 * IT2) - (
               _t97 * _t110 - 1.0 * (double)H * (_t98 * (_t99 * (_t110 + phi[2 *
               i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2 + -1]) - _t96) +
               _t100 * (_t101 * (_t110 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1
               ][32 * IT2]) - _t96) + _t102 * (_t103 * (_t110 + phi[2 * i + (16
               * IT0 + -1)][2 * j + (32 * IT1 + 1)][32 * IT2]) - _t96) + _t104 *
               (_t105 * (_t110 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][
               32 * IT2 + 1]) - _t96) + _t106 * (_t107 * (_t110 + phi[2 * i + 16
               * IT0][2 * j + (32 * IT1 + 2)][32 * IT2]) - _t96) + _t108 * (
               _t109 * (_t110 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)][32 * IT2]) - _t96))));
            _t125 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2];
            _t111 = 2.0 * _t125;
            _t124 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1] +
               32 * IT2);
            _t123 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t122 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            _t121 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t120 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] +
               (32 * IT2 + 1));
            _t119 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + (32 * IT2 + 1));
            _t118 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + 32 *
               IT2);
            _t117 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t116 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               -1)] + 32 * IT2);
            _t115 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t114 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] +
               (32 * IT2 + -1));
            _t113 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t112 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32 *
               IT1] + 32 * IT2);
            phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2] = _t125 +
               1.0 / (_t112 - 1.0 * (double)H * (_t113 * (_t114 - 2.0) + _t115 *
               (_t116 - 2.0) + _t117 * (_t118 - 2.0) + _t119 * (_t120 - 2.0) +
               _t121 * (_t122 - 2.0) + _t123 * (_t124 - 2.0))) * (*(double*)(
               rhs[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + 32 * IT2) - (
               _t112 * _t125 - 1.0 * (double)H * (_t113 * (_t114 * (_t125 + phi
               [2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2 + -1]) -
               _t111) + _t115 * (_t116 * (_t125 + phi[2 * i + (16 * IT0 + 1)][2
               * j + (32 * IT1 + -1)][32 * IT2]) - _t111) + _t117 * (_t118 * (
               _t125 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1][32 * IT2]) -
               _t111) + _t119 * (_t120 * (_t125 + phi[2 * i + (16 * IT0 + 1)][2
               * j + 32 * IT1][32 * IT2 + 1]) - _t111) + _t121 * (_t122 * (
               _t125 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][32 *
               IT2]) - _t111) + _t123 * (_t124 * (_t125 + phi[2 * i + (16 * IT0
               + 2)][2 * j + 32 * IT1][32 * IT2]) - _t111))));
         }
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t gsrb_mapped_kernel6_red_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{ rocrParseInputArgs(paramc, paramv, depc, depv);
   union __args_gsrb_mapped_kernel6_red_main_4
      gsrb_mapped_kernel6_red_main_args;
   union __args_gsrb_mapped_kernel6_red1_2* allArgs;
   double (* phi)[6][6];
   double const (* alpha)[6][6];
   double const (* beta_i)[6][6];
   double const (* valid)[6][6];
   double const (* beta_j)[6][6];
   double const (* beta_k)[6][6];
   double const (* rhs)[6][6];
   int dim;
   int H;
   int _t1;
   int i;
   for (gsrb_mapped_kernel6_red_main_args = *(union
           __args_gsrb_mapped_kernel6_red_main_4*)rocrArgs(depv),
        dim = gsrb_mapped_kernel6_red_main_args.data.dim,
        _t1 = dim + -1 >> 4,
        H = gsrb_mapped_kernel6_red_main_args.data.H,
        rhs = gsrb_mapped_kernel6_red_main_args.data.rhs,
        beta_k = gsrb_mapped_kernel6_red_main_args.data.beta_k,
        beta_j = gsrb_mapped_kernel6_red_main_args.data.beta_j,
        valid = gsrb_mapped_kernel6_red_main_args.data.valid,
        beta_i = gsrb_mapped_kernel6_red_main_args.data.beta_i,
        alpha = gsrb_mapped_kernel6_red_main_args.data.alpha,
        phi = gsrb_mapped_kernel6_red_main_args.data.phi,
        i = 0;
        i <= _t1; i++) {
      int _t2;
      int j;
      for (_t2 = dim + -1 >> 5, j = 0; j <= _t2; j++) {
         int _t3;
         int k;
         for (_t3 = dim + -1 >> 5, k = 0; k <= _t3; k++) {
            ocrGuid_t _t4;
            union __args_gsrb_mapped_kernel6_red1_2* _t5;
            _t4 = rocrAlloc((void**)&allArgs, 80ul);
            _t5 = allArgs;
            _t5->data.phi = phi;
            _t5->data.alpha = alpha;
            _t5->data.beta_i = beta_i;
            _t5->data.valid = valid;
            _t5->data.beta_j = beta_j;
            _t5->data.beta_k = beta_k;
            _t5->data.rhs = rhs;
            _t5->data.dim = dim;
            _t5->data.H = H;
            _t5->data.IT0 = i;
            _t5->data.IT1 = j;
            _t5->data.IT2 = k;
            rocrCreateTask(0, _t4, (unsigned int)(k + j + i + (j + 2 * i) * (-1
               + dim >> 5) + i * (-1 + dim >> 5) * (-1 + dim >> 5)), dim, H, i,
                j, k);
         }
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

/*
 * Optimization PARSE[-I//usr/include, -I/home/konstantinidis/Work/ocr-db-autodec/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DbSIZE=10, -DH2INV=64.0, -D__RSTREAM_CC__, /home/konstantinidis/HPGMG/xstack-traleika-apps/sandbox/konstantinidis/hpgmg/finite-volume/source/operators/gsrb_mapped_kernel10_red.c] (231ms)
 * Optimization SSA (98ms)
 * Optimization CCP (49ms)
 * Optimization GVNGCM (73ms)
 *   Optimization SimplifyControl (16ms)
 * Optimization OSR (57ms)
 * Optimization DCE (12ms)
 * Optimization Raise[-liftnone] (694ms)
 * Optimization PolyhedralMapperNew[tile=sizes={8-16-16}] (3117ms)
 * Optimization Lower (628ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (13ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization InsertMapperAnnotations (2ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization DCE (50ms)
 * Optimization SyntaxPrepass (16ms)
 * Optimization DCE (38ms)
 * Optimization DeSSA (810ms)
 */
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_1_red_10;
union __args_gsrb_mapped_kernel10_red1_2;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_3_red_10;
union __args_gsrb_mapped_kernel10_red_main_4;
union ocrGuidUnion_t;
struct va_list_t;
struct __anon_1_red_10 {
   double (* phi)[10][10];
   double const (* alpha)[10][10];
   double const (* beta_i)[10][10];
   double const (* valid)[10][10];
   double const (* beta_j)[10][10];
   double const (* beta_k)[10][10];
   double const (* rhs)[10][10];
   int dim;
   int H;
   int IT0;
   int IT1;
   int IT2;
};
union __args_gsrb_mapped_kernel10_red1_2 {
   struct __anon_1_red_10 data;
   double padding[10];
};
struct __anon_3_red_10 {
   double (* phi)[10][10];
   double const (* alpha)[10][10];
   double const (* beta_i)[10][10];
   double const (* valid)[10][10];
   double const (* beta_j)[10][10];
   double const (* beta_k)[10][10];
   double const (* rhs)[10][10];
   int dim;
   int H;
};
union __args_gsrb_mapped_kernel10_red_main_4 {
   struct __anon_3_red_10 data;
   double padding[8];
};
/*
 * Forward declarations of functions
 */
void gsrb_mapped_kernel10_red(int dim, double (* phi)[10][10], double const (*
   Dinv)[10][10], double const (* rhs)[10][10], double a, int H, double const (
   * alpha)[10][10], double const (* beta_i)[10][10], double const (* beta_j)[
   10][10], double const (* beta_k)[10][10], double const (* valid)[10][10]);
static ocrGuid_t gsrb_mapped_kernel10_red1(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
ocrGuid_t rocrAlloc(void** dim, unsigned long k);
static ocrGuid_t gsrb_mapped_kernel10_red_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void rocrExit(void);
/*
 * Definitions of functions
 */
void gsrb_mapped_kernel10_red(int dim, double (* phi)[10][10], double const (*
   Dinv)[10][10], double const (* rhs)[10][10], double a, int H, double const (
   * alpha)[10][10], double const (* beta_i)[10][10], double const (* beta_j)[
   10][10], double const (* beta_k)[10][10], double const (* valid)[10][10])
{
   union __args_gsrb_mapped_kernel10_red_main_4* allArgs;
   rocrDeclareType(gsrb_mapped_kernel10_red_main, 1, (unsigned int)1, 1, (
      unsigned long (*)(va_list))((void*)0l));
   rocrDeclareType(gsrb_mapped_kernel10_red1, 0, (unsigned int)(dim >= 1 ? 1 + (
      -1 + dim >> 4) + (2 + 2 * (-1 + dim >> 4)) * (-1 + dim >> 5) + (1 + (-1 +
      dim >> 4)) * (-1 + dim >> 5) * (-1 + dim >> 5) : 0), 0, (unsigned long (*
      )(va_list))((void*)0l));
   rocrInit();
   if (dim >= 1) {
      ocrGuid_t _t1;
      union __args_gsrb_mapped_kernel10_red_main_4* _t2;
      _t1 = rocrAlloc((void**)&allArgs, 64ul);
      _t2 = allArgs;
      _t2->data.phi = phi;
      _t2->data.alpha = alpha;
      _t2->data.beta_i = beta_i;
      _t2->data.valid = valid;
      _t2->data.beta_j = beta_j;
      _t2->data.beta_k = beta_k;
      _t2->data.rhs = rhs;
      _t2->data.dim = dim;
      _t2->data.H = H;
      rocrExecute(1, _t1);
   }
   rocrExit();
}

static ocrGuid_t gsrb_mapped_kernel10_red1(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{ rocrParseInputArgs(paramc, paramv, depc, depv);
   union __args_gsrb_mapped_kernel10_red1_2 gsrb_mapped_kernel10_red1_args;
   double (* phi)[10][10];
   double const (* alpha)[10][10];
   double const (* beta_i)[10][10];
   double const (* valid)[10][10];
   double const (* beta_j)[10][10];
   double const (* beta_k)[10][10];
   double const (* rhs)[10][10];
   int dim;
   int H;
   int IT0;
   int IT1;
   int IT2;
   double _v1;
   gsrb_mapped_kernel10_red1_args = *(union __args_gsrb_mapped_kernel10_red1_2*
      )rocrArgs(depv);
   IT1 = gsrb_mapped_kernel10_red1_args.data.IT1;
   IT0 = gsrb_mapped_kernel10_red1_args.data.IT0;
   H = gsrb_mapped_kernel10_red1_args.data.H;
   rhs = gsrb_mapped_kernel10_red1_args.data.rhs;
   beta_k = gsrb_mapped_kernel10_red1_args.data.beta_k;
   beta_j = gsrb_mapped_kernel10_red1_args.data.beta_j;
   valid = gsrb_mapped_kernel10_red1_args.data.valid;
   beta_i = gsrb_mapped_kernel10_red1_args.data.beta_i;
   alpha = gsrb_mapped_kernel10_red1_args.data.alpha;
   phi = gsrb_mapped_kernel10_red1_args.data.phi;
   IT2 = gsrb_mapped_kernel10_red1_args.data.IT2;
   dim = gsrb_mapped_kernel10_red1_args.data.dim;
   if (dim + -32 * IT2 >= 2) {
      int _t1;
      int i;
      for (_t1 = (__mins_32(dim + -16 * IT0 + -1 >> 1, 7)), i = 0; i <= _t1; i
              ++) {
         int _t2;
         int j;
         for (_t2 = (__mins_32(dim + -32 * IT1 + -1 >> 1, 15)), j = 0; j <= _t2
                 ; j++) {
            int _t3;
            int k;
            for (_t3 = (__mins_32(dim + -32 * IT2 + -2 >> 1, 15)), k = 0; k <=
                    _t3; k++) {
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
               _t18 = phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2
                  + 1)];
               _t4 = 2.0 * _t18;
               _t17 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t16 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t15 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t14 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1)));
               _t13 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 2)));
               _t12 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  2 * k + (32 * IT2 + 2)));
               _t11 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t10 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  2 * k + (32 * IT2 + 1)));
               _t9 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + -1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t8 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 1)));
               _t7 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2 *
                  k + 32 * IT2));
               _t6 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 1)));
               _t5 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + 32 * IT1]
                  + (2 * k + (32 * IT2 + 1)));
               phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2 + 1)] =
                  _t18 + 1.0 / (_t5 - 1.0 * (double)H * (_t6 * (_t7 - 2.0) + _t8
                  * (_t9 - 2.0) + _t10 * (_t11 - 2.0) + _t12 * (_t13 - 2.0) +
                  _t14 * (_t15 - 2.0) + _t16 * (_t17 - 2.0))) * (*(double*)(rhs
                  [2 * i + 16 * IT0][2 * j + 32 * IT1] + (2 * k + (32 * IT2 + 1
                  ))) - (_t5 * _t18 - 1.0 * (double)H * (_t6 * (_t7 * (_t18 +
                  phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + 32 * IT2]) -
                  _t4) + _t8 * (_t9 * (_t18 + phi[2 * i + 16 * IT0][2 * j + (32
                  * IT1 + -1)][2 * k + (32 * IT2 + 1)]) - _t4) + _t10 * (_t11 *
                  (_t18 + phi[2 * i + (16 * IT0 + -1)][2 * j + 32 * IT1][2 * k +
                  (32 * IT2 + 1)]) - _t4) + _t12 * (_t13 * (_t18 + phi[2 * i +
                  16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2 + 2)]) - _t4) +
                  _t14 * (_t15 * (_t18 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)][2 * k + (32 * IT2 + 1)]) - _t4) + _t16 * (_t17 * (_t18 +
                  phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (32 *
                  IT2 + 1)]) - _t4))));
               _t33 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + 32 *
                  IT2];
               _t19 = 2.0 * _t33;
               _t32 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t31 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t30 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)
                  ] + (2 * k + 32 * IT2));
               _t29 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2
                  )] + (2 * k + 32 * IT2));
               _t28 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t27 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1)));
               _t26 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t25 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t24 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + 32 * IT2));
               _t23 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t22 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + -1)));
               _t21 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t20 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + 32 * IT2] =
                  _t33 + 1.0 / (_t20 - 1.0 * (double)H * (_t21 * (_t22 - 2.0) +
                  _t23 * (_t24 - 2.0) + _t25 * (_t26 - 2.0) + _t27 * (_t28 - 2.0
                  ) + _t29 * (_t30 - 2.0) + _t31 * (_t32 - 2.0))) * (*(double*)
                  (rhs[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + (2 * k + 32 *
                  IT2)) - (_t20 * _t33 - 1.0 * (double)H * (_t21 * (_t22 * (
                  _t33 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (
                  32 * IT2 + -1)]) - _t19) + _t23 * (_t24 * (_t33 + phi[2 * i +
                  16 * IT0][2 * j + 32 * IT1][2 * k + 32 * IT2]) - _t19) + _t25
                  * (_t26 * (_t33 + phi[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)][2 * k + 32 * IT2]) - _t19) + _t27 * (_t28 * (_t33 +
                  phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (32 *
                  IT2 + 1)]) - _t19) + _t29 * (_t30 * (_t33 + phi[2 * i + 16 *
                  IT0][2 * j + (32 * IT1 + 2)][2 * k + 32 * IT2]) - _t19) + _t31
                  * (_t32 * (_t33 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)][2 * k + 32 * IT2]) - _t19))));
               _t48 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + 32 *
                  IT2];
               _t34 = 2.0 * _t48;
               _t47 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t46 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t45 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t44 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t43 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t42 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t41 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + 32 * IT2));
               _t40 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t39 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + -1)] + (2 * k + 32 * IT2));
               _t38 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t37 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + -1)));
               _t36 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t35 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1] + (2 * k + 32 * IT2));
               phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + 32 * IT2] =
                  _t48 + 1.0 / (_t35 - 1.0 * (double)H * (_t36 * (_t37 - 2.0) +
                  _t38 * (_t39 - 2.0) + _t40 * (_t41 - 2.0) + _t42 * (_t43 - 2.0
                  ) + _t44 * (_t45 - 2.0) + _t46 * (_t47 - 2.0))) * (*(double*)
                  (rhs[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + (2 * k + 32 *
                  IT2)) - (_t35 * _t48 - 1.0 * (double)H * (_t36 * (_t37 * (
                  _t48 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (
                  32 * IT2 + -1)]) - _t34) + _t38 * (_t39 * (_t48 + phi[2 * i +
                  (16 * IT0 + 1)][2 * j + (32 * IT1 + -1)][2 * k + 32 * IT2]) -
                  _t34) + _t40 * (_t41 * (_t48 + phi[2 * i + 16 * IT0][2 * j +
                  32 * IT1][2 * k + 32 * IT2]) - _t34) + _t42 * (_t43 * (_t48 +
                  phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (32 *
                  IT2 + 1)]) - _t34) + _t44 * (_t45 * (_t48 + phi[2 * i + (16 *
                  IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k + 32 * IT2]) - _t34) +
                  _t46 * (_t47 * (_t48 + phi[2 * i + (16 * IT0 + 2)][2 * j + 32
                  * IT1][2 * k + 32 * IT2]) - _t34))));
               _t63 = phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k
                  + (32 * IT2 + 1)];
               _t49 = 2.0 * _t63;
               _t62 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + (32 * IT1
                  + 1)] + (2 * k + (32 * IT2 + 1)));
               _t61 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t60 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 2)] + (2 * k + (32 * IT2 + 1)));
               _t59 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 2)] + (2 * k + (32 * IT2 + 1)));
               _t58 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + (32 * IT2 + 2)));
               _t57 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 2)));
               _t56 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t55 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t54 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t53 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t52 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t51 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t50 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + (
                  32 * IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k + (32 *
                  IT2 + 1)] = _t63 + 1.0 / (_t50 - 1.0 * (double)H * (_t51 * (
                  _t52 - 2.0) + _t53 * (_t54 - 2.0) + _t55 * (_t56 - 2.0) + _t57
                  * (_t58 - 2.0) + _t59 * (_t60 - 2.0) + _t61 * (_t62 - 2.0))) *
                  (*(double*)(rhs[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1))) - (_t50 * _t63 - 1.0 * (double
                  )H * (_t51 * (_t52 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j
                  + (32 * IT1 + 1)][2 * k + 32 * IT2]) - _t49) + _t53 * (_t54 *
                  (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k +
                  (32 * IT2 + 1)]) - _t49) + _t55 * (_t56 * (_t63 + phi[2 * i +
                  16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (32 * IT2 + 1)]) -
                  _t49) + _t57 * (_t58 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 *
                  j + (32 * IT1 + 1)][2 * k + (32 * IT2 + 2)]) - _t49) + _t59 *
                  (_t60 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 2)][2 * k + (32 * IT2 + 1)]) - _t49) + _t61 * (_t62 * (_t63
                  + phi[2 * i + (16 * IT0 + 2)][2 * j + (32 * IT1 + 1)][2 * k +
                  (32 * IT2 + 1)]) - _t49))));
            }
            if (- dim + 32 * IT2 >= -31 && (dim + -32 * IT2 >= 3 && (dim + 1 & 1
                   ) == 0)) {
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
               _t78 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -1];
               _t64 = 2.0 * _t78;
               _t77 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               _t76 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t75 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)
                  ] + (dim + -1));
               _t74 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2
                  )] + (dim + -1));
               _t73 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + dim);
               _t72 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + dim);
               _t71 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t70 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t69 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  dim + -1));
               _t68 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t67 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (dim + -2));
               _t66 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t65 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -1] = _t78 +
                  1.0 / (_t65 - 1.0 * (double)H * (_t66 * (_t67 - 2.0) + _t68 *
                  (_t69 - 2.0) + _t70 * (_t71 - 2.0) + _t72 * (_t73 - 2.0) +
                  _t74 * (_t75 - 2.0) + _t76 * (_t77 - 2.0))) * (*(double*)(rhs
                  [2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + (dim + -1)) - (
                  _t65 * _t78 - 1.0 * (double)H * (_t66 * (_t67 * (_t78 + phi[2
                  * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -2]) - _t64) +
                  _t68 * (_t69 * (_t78 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1
                  ][dim + -1]) - _t64) + _t70 * (_t71 * (_t78 + phi[2 * i + (16
                  * IT0 + -1)][2 * j + (32 * IT1 + 1)][dim + -1]) - _t64) + _t72
                  * (_t73 * (_t78 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )][dim]) - _t64) + _t74 * (_t75 * (_t78 + phi[2 * i + 16 * IT0
                  ][2 * j + (32 * IT1 + 2)][dim + -1]) - _t64) + _t76 * (_t77 *
                  (_t78 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][
                  dim + -1]) - _t64))));
               _t93 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -1];
               _t79 = 2.0 * _t93;
               _t92 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t91 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t90 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               _t89 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t88 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + dim);
               _t87 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + dim);
               _t86 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  dim + -1));
               _t85 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t84 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + -1)] + (dim + -1));
               _t83 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t82 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -2));
               _t81 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t80 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1] + (dim + -1));
               phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -1] = _t93 +
                  1.0 / (_t80 - 1.0 * (double)H * (_t81 * (_t82 - 2.0) + _t83 *
                  (_t84 - 2.0) + _t85 * (_t86 - 2.0) + _t87 * (_t88 - 2.0) +
                  _t89 * (_t90 - 2.0) + _t91 * (_t92 - 2.0))) * (*(double*)(rhs
                  [2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + (dim + -1)) - (
                  _t80 * _t93 - 1.0 * (double)H * (_t81 * (_t82 * (_t93 + phi[2
                  * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -2]) - _t79) +
                  _t83 * (_t84 * (_t93 + phi[2 * i + (16 * IT0 + 1)][2 * j + (
                  32 * IT1 + -1)][dim + -1]) - _t79) + _t85 * (_t86 * (_t93 +
                  phi[2 * i + 16 * IT0][2 * j + 32 * IT1][dim + -1]) - _t79) +
                  _t87 * (_t88 * (_t93 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1][dim]) - _t79) + _t89 * (_t90 * (_t93 + phi[2 * i + (16
                  * IT0 + 1)][2 * j + (32 * IT1 + 1)][dim + -1]) - _t79) + _t91
                  * (_t92 * (_t93 + phi[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ][dim + -1]) - _t79))));
            }
         }
      }
   }
   if (dim + -32 * IT2 == 1) {
      int _t94;
      int i;
      for (_t94 = (__mins_32(-8 * IT0 + 16 * IT2, 7)), i = 0; i <= _t94; i++) {
         int _t95;
         int j;
         for (_t95 = (__mins_32(-16 * IT1 + 16 * IT2, 15)), j = 0; j <= _t95; j
                 ++) {
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
            _t110 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2];
            _t96 = 2.0 * _t110;
            _t109 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            _t108 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t107 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)] +
               32 * IT2);
            _t106 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)]
               + 32 * IT2);
            _t105 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               (32 * IT2 + 1));
            _t104 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + (32 * IT2 + 1));
            _t103 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t102 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + 32 * IT2);
            _t101 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + 32 *
               IT2);
            _t100 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + 32 * IT2);
            _t99 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               (32 * IT2 + -1));
            _t98 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               32 * IT2);
            _t97 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2] = _t110 +
               1.0 / (_t97 - 1.0 * (double)H * (_t98 * (_t99 - 2.0) + _t100 * (
               _t101 - 2.0) + _t102 * (_t103 - 2.0) + _t104 * (_t105 - 2.0) +
               _t106 * (_t107 - 2.0) + _t108 * (_t109 - 2.0))) * (*(double*)(
               rhs[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + 32 * IT2) - (
               _t97 * _t110 - 1.0 * (double)H * (_t98 * (_t99 * (_t110 + phi[2 *
               i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2 + -1]) - _t96) +
               _t100 * (_t101 * (_t110 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1
               ][32 * IT2]) - _t96) + _t102 * (_t103 * (_t110 + phi[2 * i + (16
               * IT0 + -1)][2 * j + (32 * IT1 + 1)][32 * IT2]) - _t96) + _t104 *
               (_t105 * (_t110 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][
               32 * IT2 + 1]) - _t96) + _t106 * (_t107 * (_t110 + phi[2 * i + 16
               * IT0][2 * j + (32 * IT1 + 2)][32 * IT2]) - _t96) + _t108 * (
               _t109 * (_t110 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)][32 * IT2]) - _t96))));
            _t125 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2];
            _t111 = 2.0 * _t125;
            _t124 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1] +
               32 * IT2);
            _t123 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t122 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            _t121 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t120 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] +
               (32 * IT2 + 1));
            _t119 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + (32 * IT2 + 1));
            _t118 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + 32 *
               IT2);
            _t117 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t116 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               -1)] + 32 * IT2);
            _t115 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t114 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] +
               (32 * IT2 + -1));
            _t113 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t112 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32 *
               IT1] + 32 * IT2);
            phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2] = _t125 +
               1.0 / (_t112 - 1.0 * (double)H * (_t113 * (_t114 - 2.0) + _t115 *
               (_t116 - 2.0) + _t117 * (_t118 - 2.0) + _t119 * (_t120 - 2.0) +
               _t121 * (_t122 - 2.0) + _t123 * (_t124 - 2.0))) * (*(double*)(
               rhs[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + 32 * IT2) - (
               _t112 * _t125 - 1.0 * (double)H * (_t113 * (_t114 * (_t125 + phi
               [2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2 + -1]) -
               _t111) + _t115 * (_t116 * (_t125 + phi[2 * i + (16 * IT0 + 1)][2
               * j + (32 * IT1 + -1)][32 * IT2]) - _t111) + _t117 * (_t118 * (
               _t125 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1][32 * IT2]) -
               _t111) + _t119 * (_t120 * (_t125 + phi[2 * i + (16 * IT0 + 1)][2
               * j + 32 * IT1][32 * IT2 + 1]) - _t111) + _t121 * (_t122 * (
               _t125 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][32 *
               IT2]) - _t111) + _t123 * (_t124 * (_t125 + phi[2 * i + (16 * IT0
               + 2)][2 * j + 32 * IT1][32 * IT2]) - _t111))));
         }
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t gsrb_mapped_kernel10_red_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{ rocrParseInputArgs(paramc, paramv, depc, depv);
   union __args_gsrb_mapped_kernel10_red_main_4
      gsrb_mapped_kernel10_red_main_args;
   union __args_gsrb_mapped_kernel10_red1_2* allArgs;
   double (* phi)[10][10];
   double const (* alpha)[10][10];
   double const (* beta_i)[10][10];
   double const (* valid)[10][10];
   double const (* beta_j)[10][10];
   double const (* beta_k)[10][10];
   double const (* rhs)[10][10];
   int dim;
   int H;
   int _t1;
   int i;
   for (gsrb_mapped_kernel10_red_main_args = *(union
           __args_gsrb_mapped_kernel10_red_main_4*)rocrArgs(depv),
        dim = gsrb_mapped_kernel10_red_main_args.data.dim,
        _t1 = dim + -1 >> 4,
        H = gsrb_mapped_kernel10_red_main_args.data.H,
        rhs = gsrb_mapped_kernel10_red_main_args.data.rhs,
        beta_k = gsrb_mapped_kernel10_red_main_args.data.beta_k,
        beta_j = gsrb_mapped_kernel10_red_main_args.data.beta_j,
        valid = gsrb_mapped_kernel10_red_main_args.data.valid,
        beta_i = gsrb_mapped_kernel10_red_main_args.data.beta_i,
        alpha = gsrb_mapped_kernel10_red_main_args.data.alpha,
        phi = gsrb_mapped_kernel10_red_main_args.data.phi,
        i = 0;
        i <= _t1; i++) {
      int _t2;
      int j;
      for (_t2 = dim + -1 >> 5, j = 0; j <= _t2; j++) {
         int _t3;
         int k;
         for (_t3 = dim + -1 >> 5, k = 0; k <= _t3; k++) {
            ocrGuid_t _t4;
            union __args_gsrb_mapped_kernel10_red1_2* _t5;
            _t4 = rocrAlloc((void**)&allArgs, 80ul);
            _t5 = allArgs;
            _t5->data.phi = phi;
            _t5->data.alpha = alpha;
            _t5->data.beta_i = beta_i;
            _t5->data.valid = valid;
            _t5->data.beta_j = beta_j;
            _t5->data.beta_k = beta_k;
            _t5->data.rhs = rhs;
            _t5->data.dim = dim;
            _t5->data.H = H;
            _t5->data.IT0 = i;
            _t5->data.IT1 = j;
            _t5->data.IT2 = k;
            rocrCreateTask(0, _t4, (unsigned int)(k + j + i + (j + 2 * i) * (-1
               + dim >> 5) + i * (-1 + dim >> 5) * (-1 + dim >> 5)), dim, H, i,
                j, k);
         }
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

/*
 * Optimization PARSE[-I//usr/include, -I/home/konstantinidis/Work/ocr-db-autodec/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DbSIZE=18, -DH2INV=256.0, -D__RSTREAM_CC__, /home/konstantinidis/HPGMG/xstack-traleika-apps/sandbox/konstantinidis/hpgmg/finite-volume/source/operators/gsrb_mapped_kernel18_red.c] (233ms)
 * Optimization SSA (102ms)
 * Optimization CCP (50ms)
 * Optimization GVNGCM (74ms)
 *   Optimization SimplifyControl (16ms)
 * Optimization OSR (59ms)
 * Optimization DCE (13ms)
 * Optimization Raise[-liftnone] (710ms)
 * Optimization PolyhedralMapperNew[tile=sizes={8-16-16}] (3125ms)
 * Optimization Lower (715ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (14ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization InsertMapperAnnotations (3ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization DCE (52ms)
 * Optimization SyntaxPrepass (16ms)
 * Optimization DCE (37ms)
 * Optimization DeSSA (805ms)
 */
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_1_red_18;
union __args_gsrb_mapped_kernel18_red1_2;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_3_red_18;
union __args_gsrb_mapped_kernel18_red_main_4;
union ocrGuidUnion_t;
struct va_list_t;
struct __anon_1_red_18 {
   double (* phi)[18][18];
   double const (* alpha)[18][18];
   double const (* beta_i)[18][18];
   double const (* valid)[18][18];
   double const (* beta_j)[18][18];
   double const (* beta_k)[18][18];
   double const (* rhs)[18][18];
   int dim;
   int H;
   int IT0;
   int IT1;
   int IT2;
};
union __args_gsrb_mapped_kernel18_red1_2 {
   struct __anon_1_red_18 data;
   double padding[10];
};
struct __anon_3_red_18 {
   double (* phi)[18][18];
   double const (* alpha)[18][18];
   double const (* beta_i)[18][18];
   double const (* valid)[18][18];
   double const (* beta_j)[18][18];
   double const (* beta_k)[18][18];
   double const (* rhs)[18][18];
   int dim;
   int H;
};
union __args_gsrb_mapped_kernel18_red_main_4 {
   struct __anon_3_red_18 data;
   double padding[8];
};
/*
 * Forward declarations of functions
 */
void gsrb_mapped_kernel18_red(int dim, double (* phi)[18][18], double const (*
   Dinv)[18][18], double const (* rhs)[18][18], double a, int H, double const (
   * alpha)[18][18], double const (* beta_i)[18][18], double const (* beta_j)[
   18][18], double const (* beta_k)[18][18], double const (* valid)[18][18]);
static ocrGuid_t gsrb_mapped_kernel18_red1(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
ocrGuid_t rocrAlloc(void** dim, unsigned long k);
static ocrGuid_t gsrb_mapped_kernel18_red_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void rocrExit(void);
/*
 * Definitions of functions
 */
void gsrb_mapped_kernel18_red(int dim, double (* phi)[18][18], double const (*
   Dinv)[18][18], double const (* rhs)[18][18], double a, int H, double const (
   * alpha)[18][18], double const (* beta_i)[18][18], double const (* beta_j)[
   18][18], double const (* beta_k)[18][18], double const (* valid)[18][18])
{
   union __args_gsrb_mapped_kernel18_red_main_4* allArgs;
   rocrDeclareType(gsrb_mapped_kernel18_red_main, 1, (unsigned int)1, 1, (
      unsigned long (*)(va_list))((void*)0l));
   rocrDeclareType(gsrb_mapped_kernel18_red1, 0, (unsigned int)(dim >= 1 ? 1 + (
      -1 + dim >> 4) + (2 + 2 * (-1 + dim >> 4)) * (-1 + dim >> 5) + (1 + (-1 +
      dim >> 4)) * (-1 + dim >> 5) * (-1 + dim >> 5) : 0), 0, (unsigned long (*
      )(va_list))((void*)0l));
   rocrInit();
   if (dim >= 1) {
      ocrGuid_t _t1;
      union __args_gsrb_mapped_kernel18_red_main_4* _t2;
      _t1 = rocrAlloc((void**)&allArgs, 64ul);
      _t2 = allArgs;
      _t2->data.phi = phi;
      _t2->data.alpha = alpha;
      _t2->data.beta_i = beta_i;
      _t2->data.valid = valid;
      _t2->data.beta_j = beta_j;
      _t2->data.beta_k = beta_k;
      _t2->data.rhs = rhs;
      _t2->data.dim = dim;
      _t2->data.H = H;
      rocrExecute(1, _t1);
   }
   rocrExit();
}

static ocrGuid_t gsrb_mapped_kernel18_red1(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{ rocrParseInputArgs(paramc, paramv, depc, depv);
   union __args_gsrb_mapped_kernel18_red1_2 gsrb_mapped_kernel18_red1_args;
   double (* phi)[18][18];
   double const (* alpha)[18][18];
   double const (* beta_i)[18][18];
   double const (* valid)[18][18];
   double const (* beta_j)[18][18];
   double const (* beta_k)[18][18];
   double const (* rhs)[18][18];
   int dim;
   int H;
   int IT0;
   int IT1;
   int IT2;
   double _v1;
   gsrb_mapped_kernel18_red1_args = *(union __args_gsrb_mapped_kernel18_red1_2*
      )rocrArgs(depv);
   IT1 = gsrb_mapped_kernel18_red1_args.data.IT1;
   IT0 = gsrb_mapped_kernel18_red1_args.data.IT0;
   H = gsrb_mapped_kernel18_red1_args.data.H;
   rhs = gsrb_mapped_kernel18_red1_args.data.rhs;
   beta_k = gsrb_mapped_kernel18_red1_args.data.beta_k;
   beta_j = gsrb_mapped_kernel18_red1_args.data.beta_j;
   valid = gsrb_mapped_kernel18_red1_args.data.valid;
   beta_i = gsrb_mapped_kernel18_red1_args.data.beta_i;
   alpha = gsrb_mapped_kernel18_red1_args.data.alpha;
   phi = gsrb_mapped_kernel18_red1_args.data.phi;
   IT2 = gsrb_mapped_kernel18_red1_args.data.IT2;
   dim = gsrb_mapped_kernel18_red1_args.data.dim;
   if (dim + -32 * IT2 >= 2) {
      int _t1;
      int i;
      for (_t1 = (__mins_32(dim + -16 * IT0 + -1 >> 1, 7)), i = 0; i <= _t1; i
              ++) {
         int _t2;
         int j;
         for (_t2 = (__mins_32(dim + -32 * IT1 + -1 >> 1, 15)), j = 0; j <= _t2
                 ; j++) {
            int _t3;
            int k;
            for (_t3 = (__mins_32(dim + -32 * IT2 + -2 >> 1, 15)), k = 0; k <=
                    _t3; k++) {
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
               _t18 = phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2
                  + 1)];
               _t4 = 2.0 * _t18;
               _t17 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t16 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t15 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t14 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1)));
               _t13 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 2)));
               _t12 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  2 * k + (32 * IT2 + 2)));
               _t11 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t10 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  2 * k + (32 * IT2 + 1)));
               _t9 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + -1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t8 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 1)));
               _t7 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2 *
                  k + 32 * IT2));
               _t6 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 1)));
               _t5 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + 32 * IT1]
                  + (2 * k + (32 * IT2 + 1)));
               phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2 + 1)] =
                  _t18 + 1.0 / (_t5 - 1.0 * (double)H * (_t6 * (_t7 - 2.0) + _t8
                  * (_t9 - 2.0) + _t10 * (_t11 - 2.0) + _t12 * (_t13 - 2.0) +
                  _t14 * (_t15 - 2.0) + _t16 * (_t17 - 2.0))) * (*(double*)(rhs
                  [2 * i + 16 * IT0][2 * j + 32 * IT1] + (2 * k + (32 * IT2 + 1
                  ))) - (_t5 * _t18 - 1.0 * (double)H * (_t6 * (_t7 * (_t18 +
                  phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + 32 * IT2]) -
                  _t4) + _t8 * (_t9 * (_t18 + phi[2 * i + 16 * IT0][2 * j + (32
                  * IT1 + -1)][2 * k + (32 * IT2 + 1)]) - _t4) + _t10 * (_t11 *
                  (_t18 + phi[2 * i + (16 * IT0 + -1)][2 * j + 32 * IT1][2 * k +
                  (32 * IT2 + 1)]) - _t4) + _t12 * (_t13 * (_t18 + phi[2 * i +
                  16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2 + 2)]) - _t4) +
                  _t14 * (_t15 * (_t18 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)][2 * k + (32 * IT2 + 1)]) - _t4) + _t16 * (_t17 * (_t18 +
                  phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (32 *
                  IT2 + 1)]) - _t4))));
               _t33 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + 32 *
                  IT2];
               _t19 = 2.0 * _t33;
               _t32 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t31 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t30 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)
                  ] + (2 * k + 32 * IT2));
               _t29 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2
                  )] + (2 * k + 32 * IT2));
               _t28 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t27 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1)));
               _t26 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t25 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t24 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + 32 * IT2));
               _t23 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t22 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + -1)));
               _t21 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t20 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + 32 * IT2] =
                  _t33 + 1.0 / (_t20 - 1.0 * (double)H * (_t21 * (_t22 - 2.0) +
                  _t23 * (_t24 - 2.0) + _t25 * (_t26 - 2.0) + _t27 * (_t28 - 2.0
                  ) + _t29 * (_t30 - 2.0) + _t31 * (_t32 - 2.0))) * (*(double*)
                  (rhs[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + (2 * k + 32 *
                  IT2)) - (_t20 * _t33 - 1.0 * (double)H * (_t21 * (_t22 * (
                  _t33 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (
                  32 * IT2 + -1)]) - _t19) + _t23 * (_t24 * (_t33 + phi[2 * i +
                  16 * IT0][2 * j + 32 * IT1][2 * k + 32 * IT2]) - _t19) + _t25
                  * (_t26 * (_t33 + phi[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)][2 * k + 32 * IT2]) - _t19) + _t27 * (_t28 * (_t33 +
                  phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (32 *
                  IT2 + 1)]) - _t19) + _t29 * (_t30 * (_t33 + phi[2 * i + 16 *
                  IT0][2 * j + (32 * IT1 + 2)][2 * k + 32 * IT2]) - _t19) + _t31
                  * (_t32 * (_t33 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)][2 * k + 32 * IT2]) - _t19))));
               _t48 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + 32 *
                  IT2];
               _t34 = 2.0 * _t48;
               _t47 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t46 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t45 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t44 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t43 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t42 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t41 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + 32 * IT2));
               _t40 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t39 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + -1)] + (2 * k + 32 * IT2));
               _t38 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t37 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + -1)));
               _t36 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t35 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1] + (2 * k + 32 * IT2));
               phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + 32 * IT2] =
                  _t48 + 1.0 / (_t35 - 1.0 * (double)H * (_t36 * (_t37 - 2.0) +
                  _t38 * (_t39 - 2.0) + _t40 * (_t41 - 2.0) + _t42 * (_t43 - 2.0
                  ) + _t44 * (_t45 - 2.0) + _t46 * (_t47 - 2.0))) * (*(double*)
                  (rhs[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + (2 * k + 32 *
                  IT2)) - (_t35 * _t48 - 1.0 * (double)H * (_t36 * (_t37 * (
                  _t48 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (
                  32 * IT2 + -1)]) - _t34) + _t38 * (_t39 * (_t48 + phi[2 * i +
                  (16 * IT0 + 1)][2 * j + (32 * IT1 + -1)][2 * k + 32 * IT2]) -
                  _t34) + _t40 * (_t41 * (_t48 + phi[2 * i + 16 * IT0][2 * j +
                  32 * IT1][2 * k + 32 * IT2]) - _t34) + _t42 * (_t43 * (_t48 +
                  phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (32 *
                  IT2 + 1)]) - _t34) + _t44 * (_t45 * (_t48 + phi[2 * i + (16 *
                  IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k + 32 * IT2]) - _t34) +
                  _t46 * (_t47 * (_t48 + phi[2 * i + (16 * IT0 + 2)][2 * j + 32
                  * IT1][2 * k + 32 * IT2]) - _t34))));
               _t63 = phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k
                  + (32 * IT2 + 1)];
               _t49 = 2.0 * _t63;
               _t62 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + (32 * IT1
                  + 1)] + (2 * k + (32 * IT2 + 1)));
               _t61 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t60 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 2)] + (2 * k + (32 * IT2 + 1)));
               _t59 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 2)] + (2 * k + (32 * IT2 + 1)));
               _t58 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + (32 * IT2 + 2)));
               _t57 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 2)));
               _t56 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t55 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t54 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t53 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t52 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t51 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t50 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + (
                  32 * IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k + (32 *
                  IT2 + 1)] = _t63 + 1.0 / (_t50 - 1.0 * (double)H * (_t51 * (
                  _t52 - 2.0) + _t53 * (_t54 - 2.0) + _t55 * (_t56 - 2.0) + _t57
                  * (_t58 - 2.0) + _t59 * (_t60 - 2.0) + _t61 * (_t62 - 2.0))) *
                  (*(double*)(rhs[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1))) - (_t50 * _t63 - 1.0 * (double
                  )H * (_t51 * (_t52 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j
                  + (32 * IT1 + 1)][2 * k + 32 * IT2]) - _t49) + _t53 * (_t54 *
                  (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k +
                  (32 * IT2 + 1)]) - _t49) + _t55 * (_t56 * (_t63 + phi[2 * i +
                  16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (32 * IT2 + 1)]) -
                  _t49) + _t57 * (_t58 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 *
                  j + (32 * IT1 + 1)][2 * k + (32 * IT2 + 2)]) - _t49) + _t59 *
                  (_t60 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 2)][2 * k + (32 * IT2 + 1)]) - _t49) + _t61 * (_t62 * (_t63
                  + phi[2 * i + (16 * IT0 + 2)][2 * j + (32 * IT1 + 1)][2 * k +
                  (32 * IT2 + 1)]) - _t49))));
            }
            if (- dim + 32 * IT2 >= -31 && (dim + -32 * IT2 >= 3 && (dim + 1 & 1
                   ) == 0)) {
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
               _t78 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -1];
               _t64 = 2.0 * _t78;
               _t77 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               _t76 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t75 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)
                  ] + (dim + -1));
               _t74 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2
                  )] + (dim + -1));
               _t73 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + dim);
               _t72 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + dim);
               _t71 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t70 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t69 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  dim + -1));
               _t68 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t67 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (dim + -2));
               _t66 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t65 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -1] = _t78 +
                  1.0 / (_t65 - 1.0 * (double)H * (_t66 * (_t67 - 2.0) + _t68 *
                  (_t69 - 2.0) + _t70 * (_t71 - 2.0) + _t72 * (_t73 - 2.0) +
                  _t74 * (_t75 - 2.0) + _t76 * (_t77 - 2.0))) * (*(double*)(rhs
                  [2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + (dim + -1)) - (
                  _t65 * _t78 - 1.0 * (double)H * (_t66 * (_t67 * (_t78 + phi[2
                  * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -2]) - _t64) +
                  _t68 * (_t69 * (_t78 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1
                  ][dim + -1]) - _t64) + _t70 * (_t71 * (_t78 + phi[2 * i + (16
                  * IT0 + -1)][2 * j + (32 * IT1 + 1)][dim + -1]) - _t64) + _t72
                  * (_t73 * (_t78 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )][dim]) - _t64) + _t74 * (_t75 * (_t78 + phi[2 * i + 16 * IT0
                  ][2 * j + (32 * IT1 + 2)][dim + -1]) - _t64) + _t76 * (_t77 *
                  (_t78 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][
                  dim + -1]) - _t64))));
               _t93 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -1];
               _t79 = 2.0 * _t93;
               _t92 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t91 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t90 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               _t89 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t88 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + dim);
               _t87 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + dim);
               _t86 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  dim + -1));
               _t85 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t84 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + -1)] + (dim + -1));
               _t83 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t82 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -2));
               _t81 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t80 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1] + (dim + -1));
               phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -1] = _t93 +
                  1.0 / (_t80 - 1.0 * (double)H * (_t81 * (_t82 - 2.0) + _t83 *
                  (_t84 - 2.0) + _t85 * (_t86 - 2.0) + _t87 * (_t88 - 2.0) +
                  _t89 * (_t90 - 2.0) + _t91 * (_t92 - 2.0))) * (*(double*)(rhs
                  [2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + (dim + -1)) - (
                  _t80 * _t93 - 1.0 * (double)H * (_t81 * (_t82 * (_t93 + phi[2
                  * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -2]) - _t79) +
                  _t83 * (_t84 * (_t93 + phi[2 * i + (16 * IT0 + 1)][2 * j + (
                  32 * IT1 + -1)][dim + -1]) - _t79) + _t85 * (_t86 * (_t93 +
                  phi[2 * i + 16 * IT0][2 * j + 32 * IT1][dim + -1]) - _t79) +
                  _t87 * (_t88 * (_t93 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1][dim]) - _t79) + _t89 * (_t90 * (_t93 + phi[2 * i + (16
                  * IT0 + 1)][2 * j + (32 * IT1 + 1)][dim + -1]) - _t79) + _t91
                  * (_t92 * (_t93 + phi[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ][dim + -1]) - _t79))));
            }
         }
      }
   }
   if (dim + -32 * IT2 == 1) {
      int _t94;
      int i;
      for (_t94 = (__mins_32(-8 * IT0 + 16 * IT2, 7)), i = 0; i <= _t94; i++) {
         int _t95;
         int j;
         for (_t95 = (__mins_32(-16 * IT1 + 16 * IT2, 15)), j = 0; j <= _t95; j
                 ++) {
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
            _t110 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2];
            _t96 = 2.0 * _t110;
            _t109 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            _t108 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t107 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)] +
               32 * IT2);
            _t106 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)]
               + 32 * IT2);
            _t105 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               (32 * IT2 + 1));
            _t104 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + (32 * IT2 + 1));
            _t103 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t102 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + 32 * IT2);
            _t101 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + 32 *
               IT2);
            _t100 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + 32 * IT2);
            _t99 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               (32 * IT2 + -1));
            _t98 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               32 * IT2);
            _t97 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2] = _t110 +
               1.0 / (_t97 - 1.0 * (double)H * (_t98 * (_t99 - 2.0) + _t100 * (
               _t101 - 2.0) + _t102 * (_t103 - 2.0) + _t104 * (_t105 - 2.0) +
               _t106 * (_t107 - 2.0) + _t108 * (_t109 - 2.0))) * (*(double*)(
               rhs[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + 32 * IT2) - (
               _t97 * _t110 - 1.0 * (double)H * (_t98 * (_t99 * (_t110 + phi[2 *
               i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2 + -1]) - _t96) +
               _t100 * (_t101 * (_t110 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1
               ][32 * IT2]) - _t96) + _t102 * (_t103 * (_t110 + phi[2 * i + (16
               * IT0 + -1)][2 * j + (32 * IT1 + 1)][32 * IT2]) - _t96) + _t104 *
               (_t105 * (_t110 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][
               32 * IT2 + 1]) - _t96) + _t106 * (_t107 * (_t110 + phi[2 * i + 16
               * IT0][2 * j + (32 * IT1 + 2)][32 * IT2]) - _t96) + _t108 * (
               _t109 * (_t110 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)][32 * IT2]) - _t96))));
            _t125 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2];
            _t111 = 2.0 * _t125;
            _t124 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1] +
               32 * IT2);
            _t123 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t122 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            _t121 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t120 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] +
               (32 * IT2 + 1));
            _t119 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + (32 * IT2 + 1));
            _t118 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + 32 *
               IT2);
            _t117 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t116 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               -1)] + 32 * IT2);
            _t115 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t114 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] +
               (32 * IT2 + -1));
            _t113 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t112 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32 *
               IT1] + 32 * IT2);
            phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2] = _t125 +
               1.0 / (_t112 - 1.0 * (double)H * (_t113 * (_t114 - 2.0) + _t115 *
               (_t116 - 2.0) + _t117 * (_t118 - 2.0) + _t119 * (_t120 - 2.0) +
               _t121 * (_t122 - 2.0) + _t123 * (_t124 - 2.0))) * (*(double*)(
               rhs[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + 32 * IT2) - (
               _t112 * _t125 - 1.0 * (double)H * (_t113 * (_t114 * (_t125 + phi
               [2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2 + -1]) -
               _t111) + _t115 * (_t116 * (_t125 + phi[2 * i + (16 * IT0 + 1)][2
               * j + (32 * IT1 + -1)][32 * IT2]) - _t111) + _t117 * (_t118 * (
               _t125 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1][32 * IT2]) -
               _t111) + _t119 * (_t120 * (_t125 + phi[2 * i + (16 * IT0 + 1)][2
               * j + 32 * IT1][32 * IT2 + 1]) - _t111) + _t121 * (_t122 * (
               _t125 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][32 *
               IT2]) - _t111) + _t123 * (_t124 * (_t125 + phi[2 * i + (16 * IT0
               + 2)][2 * j + 32 * IT1][32 * IT2]) - _t111))));
         }
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t gsrb_mapped_kernel18_red_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{ rocrParseInputArgs(paramc, paramv, depc, depv);
   union __args_gsrb_mapped_kernel18_red_main_4
      gsrb_mapped_kernel18_red_main_args;
   union __args_gsrb_mapped_kernel18_red1_2* allArgs;
   double (* phi)[18][18];
   double const (* alpha)[18][18];
   double const (* beta_i)[18][18];
   double const (* valid)[18][18];
   double const (* beta_j)[18][18];
   double const (* beta_k)[18][18];
   double const (* rhs)[18][18];
   int dim;
   int H;
   int _t1;
   int i;
   for (gsrb_mapped_kernel18_red_main_args = *(union
           __args_gsrb_mapped_kernel18_red_main_4*)rocrArgs(depv),
        dim = gsrb_mapped_kernel18_red_main_args.data.dim,
        _t1 = dim + -1 >> 4,
        H = gsrb_mapped_kernel18_red_main_args.data.H,
        rhs = gsrb_mapped_kernel18_red_main_args.data.rhs,
        beta_k = gsrb_mapped_kernel18_red_main_args.data.beta_k,
        beta_j = gsrb_mapped_kernel18_red_main_args.data.beta_j,
        valid = gsrb_mapped_kernel18_red_main_args.data.valid,
        beta_i = gsrb_mapped_kernel18_red_main_args.data.beta_i,
        alpha = gsrb_mapped_kernel18_red_main_args.data.alpha,
        phi = gsrb_mapped_kernel18_red_main_args.data.phi,
        i = 0;
        i <= _t1; i++) {
      int _t2;
      int j;
      for (_t2 = dim + -1 >> 5, j = 0; j <= _t2; j++) {
         int _t3;
         int k;
         for (_t3 = dim + -1 >> 5, k = 0; k <= _t3; k++) {
            ocrGuid_t _t4;
            union __args_gsrb_mapped_kernel18_red1_2* _t5;
            _t4 = rocrAlloc((void**)&allArgs, 80ul);
            _t5 = allArgs;
            _t5->data.phi = phi;
            _t5->data.alpha = alpha;
            _t5->data.beta_i = beta_i;
            _t5->data.valid = valid;
            _t5->data.beta_j = beta_j;
            _t5->data.beta_k = beta_k;
            _t5->data.rhs = rhs;
            _t5->data.dim = dim;
            _t5->data.H = H;
            _t5->data.IT0 = i;
            _t5->data.IT1 = j;
            _t5->data.IT2 = k;
            rocrCreateTask(0, _t4, (unsigned int)(k + j + i + (j + 2 * i) * (-1
               + dim >> 5) + i * (-1 + dim >> 5) * (-1 + dim >> 5)), dim, H, i,
                j, k);
         }
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

/*
 * Optimization PARSE[-I//usr/include, -I/home/konstantinidis/Work/ocr-db-autodec/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DbSIZE=34, -DH2INV=1024.0, -D__RSTREAM_CC__, /home/konstantinidis/HPGMG/xstack-traleika-apps/sandbox/konstantinidis/hpgmg/finite-volume/source/operators/gsrb_mapped_kernel34_red.c] (231ms)
 * Optimization SSA (103ms)
 * Optimization CCP (50ms)
 * Optimization GVNGCM (74ms)
 *   Optimization SimplifyControl (17ms)
 * Optimization OSR (59ms)
 * Optimization DCE (12ms)
 * Optimization Raise[-liftnone] (707ms)
 * Optimization PolyhedralMapperNew[tile=sizes={8-16-16}] (3117ms)
 * Optimization Lower (591ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (14ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization InsertMapperAnnotations (2ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization DCE (50ms)
 * Optimization SyntaxPrepass (17ms)
 * Optimization DCE (36ms)
 * Optimization DeSSA (816ms)
 */
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_1_red_34;
union __args_gsrb_mapped_kernel34_red1_2;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_3_red_34;
union __args_gsrb_mapped_kernel34_red_main_4;
union ocrGuidUnion_t;
struct va_list_t;
struct __anon_1_red_34 {
   double (* phi)[34][34];
   double const (* alpha)[34][34];
   double const (* beta_i)[34][34];
   double const (* valid)[34][34];
   double const (* beta_j)[34][34];
   double const (* beta_k)[34][34];
   double const (* rhs)[34][34];
   int dim;
   int H;
   int IT0;
   int IT1;
   int IT2;
};
union __args_gsrb_mapped_kernel34_red1_2 {
   struct __anon_1_red_34 data;
   double padding[10];
};
struct __anon_3_red_34 {
   double (* phi)[34][34];
   double const (* alpha)[34][34];
   double const (* beta_i)[34][34];
   double const (* valid)[34][34];
   double const (* beta_j)[34][34];
   double const (* beta_k)[34][34];
   double const (* rhs)[34][34];
   int dim;
   int H;
};
union __args_gsrb_mapped_kernel34_red_main_4 {
   struct __anon_3_red_34 data;
   double padding[8];
};
/*
 * Forward declarations of functions
 */
void gsrb_mapped_kernel34_red(int dim, double (* phi)[34][34], double const (*
   Dinv)[34][34], double const (* rhs)[34][34], double a, int H, double const (
   * alpha)[34][34], double const (* beta_i)[34][34], double const (* beta_j)[
   34][34], double const (* beta_k)[34][34], double const (* valid)[34][34]);
static ocrGuid_t gsrb_mapped_kernel34_red1(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
ocrGuid_t rocrAlloc(void** dim, unsigned long k);
static ocrGuid_t gsrb_mapped_kernel34_red_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void rocrExit(void);
/*
 * Definitions of functions
 */
void gsrb_mapped_kernel34_red(int dim, double (* phi)[34][34], double const (*
   Dinv)[34][34], double const (* rhs)[34][34], double a, int H, double const (
   * alpha)[34][34], double const (* beta_i)[34][34], double const (* beta_j)[
   34][34], double const (* beta_k)[34][34], double const (* valid)[34][34])
{
   union __args_gsrb_mapped_kernel34_red_main_4* allArgs;
   rocrDeclareType(gsrb_mapped_kernel34_red_main, 1, (unsigned int)1, 1, (
      unsigned long (*)(va_list))((void*)0l));
   rocrDeclareType(gsrb_mapped_kernel34_red1, 0, (unsigned int)(dim >= 1 ? 1 + (
      -1 + dim >> 4) + (2 + 2 * (-1 + dim >> 4)) * (-1 + dim >> 5) + (1 + (-1 +
      dim >> 4)) * (-1 + dim >> 5) * (-1 + dim >> 5) : 0), 0, (unsigned long (*
      )(va_list))((void*)0l));
   rocrInit();
   if (dim >= 1) {
      ocrGuid_t _t1;
      union __args_gsrb_mapped_kernel34_red_main_4* _t2;
      _t1 = rocrAlloc((void**)&allArgs, 64ul);
      _t2 = allArgs;
      _t2->data.phi = phi;
      _t2->data.alpha = alpha;
      _t2->data.beta_i = beta_i;
      _t2->data.valid = valid;
      _t2->data.beta_j = beta_j;
      _t2->data.beta_k = beta_k;
      _t2->data.rhs = rhs;
      _t2->data.dim = dim;
      _t2->data.H = H;
      rocrExecute(1, _t1);
   }
   rocrExit();
}

static ocrGuid_t gsrb_mapped_kernel34_red1(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{ rocrParseInputArgs(paramc, paramv, depc, depv);
   union __args_gsrb_mapped_kernel34_red1_2 gsrb_mapped_kernel34_red1_args;
   double (* phi)[34][34];
   double const (* alpha)[34][34];
   double const (* beta_i)[34][34];
   double const (* valid)[34][34];
   double const (* beta_j)[34][34];
   double const (* beta_k)[34][34];
   double const (* rhs)[34][34];
   int dim;
   int H;
   int IT0;
   int IT1;
   int IT2;
   double _v1;
   gsrb_mapped_kernel34_red1_args = *(union __args_gsrb_mapped_kernel34_red1_2*
      )rocrArgs(depv);
   IT1 = gsrb_mapped_kernel34_red1_args.data.IT1;
   IT0 = gsrb_mapped_kernel34_red1_args.data.IT0;
   H = gsrb_mapped_kernel34_red1_args.data.H;
   rhs = gsrb_mapped_kernel34_red1_args.data.rhs;
   beta_k = gsrb_mapped_kernel34_red1_args.data.beta_k;
   beta_j = gsrb_mapped_kernel34_red1_args.data.beta_j;
   valid = gsrb_mapped_kernel34_red1_args.data.valid;
   beta_i = gsrb_mapped_kernel34_red1_args.data.beta_i;
   alpha = gsrb_mapped_kernel34_red1_args.data.alpha;
   phi = gsrb_mapped_kernel34_red1_args.data.phi;
   IT2 = gsrb_mapped_kernel34_red1_args.data.IT2;
   dim = gsrb_mapped_kernel34_red1_args.data.dim;
   if (dim + -32 * IT2 >= 2) {
      int _t1;
      int i;
      for (_t1 = (__mins_32(dim + -16 * IT0 + -1 >> 1, 7)), i = 0; i <= _t1; i
              ++) {
         int _t2;
         int j;
         for (_t2 = (__mins_32(dim + -32 * IT1 + -1 >> 1, 15)), j = 0; j <= _t2
                 ; j++) {
            int _t3;
            int k;
            for (_t3 = (__mins_32(dim + -32 * IT2 + -2 >> 1, 15)), k = 0; k <=
                    _t3; k++) {
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
               _t18 = phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2
                  + 1)];
               _t4 = 2.0 * _t18;
               _t17 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t16 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t15 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t14 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1)));
               _t13 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 2)));
               _t12 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  2 * k + (32 * IT2 + 2)));
               _t11 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t10 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  2 * k + (32 * IT2 + 1)));
               _t9 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + -1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t8 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 1)));
               _t7 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2 *
                  k + 32 * IT2));
               _t6 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 1)));
               _t5 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + 32 * IT1]
                  + (2 * k + (32 * IT2 + 1)));
               phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2 + 1)] =
                  _t18 + 1.0 / (_t5 - 1.0 * (double)H * (_t6 * (_t7 - 2.0) + _t8
                  * (_t9 - 2.0) + _t10 * (_t11 - 2.0) + _t12 * (_t13 - 2.0) +
                  _t14 * (_t15 - 2.0) + _t16 * (_t17 - 2.0))) * (*(double*)(rhs
                  [2 * i + 16 * IT0][2 * j + 32 * IT1] + (2 * k + (32 * IT2 + 1
                  ))) - (_t5 * _t18 - 1.0 * (double)H * (_t6 * (_t7 * (_t18 +
                  phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + 32 * IT2]) -
                  _t4) + _t8 * (_t9 * (_t18 + phi[2 * i + 16 * IT0][2 * j + (32
                  * IT1 + -1)][2 * k + (32 * IT2 + 1)]) - _t4) + _t10 * (_t11 *
                  (_t18 + phi[2 * i + (16 * IT0 + -1)][2 * j + 32 * IT1][2 * k +
                  (32 * IT2 + 1)]) - _t4) + _t12 * (_t13 * (_t18 + phi[2 * i +
                  16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2 + 2)]) - _t4) +
                  _t14 * (_t15 * (_t18 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)][2 * k + (32 * IT2 + 1)]) - _t4) + _t16 * (_t17 * (_t18 +
                  phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (32 *
                  IT2 + 1)]) - _t4))));
               _t33 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + 32 *
                  IT2];
               _t19 = 2.0 * _t33;
               _t32 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t31 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t30 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)
                  ] + (2 * k + 32 * IT2));
               _t29 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2
                  )] + (2 * k + 32 * IT2));
               _t28 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t27 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1)));
               _t26 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t25 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t24 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + 32 * IT2));
               _t23 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t22 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + -1)));
               _t21 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t20 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + 32 * IT2] =
                  _t33 + 1.0 / (_t20 - 1.0 * (double)H * (_t21 * (_t22 - 2.0) +
                  _t23 * (_t24 - 2.0) + _t25 * (_t26 - 2.0) + _t27 * (_t28 - 2.0
                  ) + _t29 * (_t30 - 2.0) + _t31 * (_t32 - 2.0))) * (*(double*)
                  (rhs[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + (2 * k + 32 *
                  IT2)) - (_t20 * _t33 - 1.0 * (double)H * (_t21 * (_t22 * (
                  _t33 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (
                  32 * IT2 + -1)]) - _t19) + _t23 * (_t24 * (_t33 + phi[2 * i +
                  16 * IT0][2 * j + 32 * IT1][2 * k + 32 * IT2]) - _t19) + _t25
                  * (_t26 * (_t33 + phi[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)][2 * k + 32 * IT2]) - _t19) + _t27 * (_t28 * (_t33 +
                  phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (32 *
                  IT2 + 1)]) - _t19) + _t29 * (_t30 * (_t33 + phi[2 * i + 16 *
                  IT0][2 * j + (32 * IT1 + 2)][2 * k + 32 * IT2]) - _t19) + _t31
                  * (_t32 * (_t33 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)][2 * k + 32 * IT2]) - _t19))));
               _t48 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + 32 *
                  IT2];
               _t34 = 2.0 * _t48;
               _t47 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t46 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t45 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t44 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t43 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t42 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t41 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + 32 * IT2));
               _t40 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t39 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + -1)] + (2 * k + 32 * IT2));
               _t38 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t37 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + -1)));
               _t36 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t35 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1] + (2 * k + 32 * IT2));
               phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + 32 * IT2] =
                  _t48 + 1.0 / (_t35 - 1.0 * (double)H * (_t36 * (_t37 - 2.0) +
                  _t38 * (_t39 - 2.0) + _t40 * (_t41 - 2.0) + _t42 * (_t43 - 2.0
                  ) + _t44 * (_t45 - 2.0) + _t46 * (_t47 - 2.0))) * (*(double*)
                  (rhs[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + (2 * k + 32 *
                  IT2)) - (_t35 * _t48 - 1.0 * (double)H * (_t36 * (_t37 * (
                  _t48 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (
                  32 * IT2 + -1)]) - _t34) + _t38 * (_t39 * (_t48 + phi[2 * i +
                  (16 * IT0 + 1)][2 * j + (32 * IT1 + -1)][2 * k + 32 * IT2]) -
                  _t34) + _t40 * (_t41 * (_t48 + phi[2 * i + 16 * IT0][2 * j +
                  32 * IT1][2 * k + 32 * IT2]) - _t34) + _t42 * (_t43 * (_t48 +
                  phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (32 *
                  IT2 + 1)]) - _t34) + _t44 * (_t45 * (_t48 + phi[2 * i + (16 *
                  IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k + 32 * IT2]) - _t34) +
                  _t46 * (_t47 * (_t48 + phi[2 * i + (16 * IT0 + 2)][2 * j + 32
                  * IT1][2 * k + 32 * IT2]) - _t34))));
               _t63 = phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k
                  + (32 * IT2 + 1)];
               _t49 = 2.0 * _t63;
               _t62 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + (32 * IT1
                  + 1)] + (2 * k + (32 * IT2 + 1)));
               _t61 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t60 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 2)] + (2 * k + (32 * IT2 + 1)));
               _t59 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 2)] + (2 * k + (32 * IT2 + 1)));
               _t58 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + (32 * IT2 + 2)));
               _t57 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 2)));
               _t56 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t55 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t54 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t53 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t52 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t51 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t50 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + (
                  32 * IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k + (32 *
                  IT2 + 1)] = _t63 + 1.0 / (_t50 - 1.0 * (double)H * (_t51 * (
                  _t52 - 2.0) + _t53 * (_t54 - 2.0) + _t55 * (_t56 - 2.0) + _t57
                  * (_t58 - 2.0) + _t59 * (_t60 - 2.0) + _t61 * (_t62 - 2.0))) *
                  (*(double*)(rhs[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1))) - (_t50 * _t63 - 1.0 * (double
                  )H * (_t51 * (_t52 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j
                  + (32 * IT1 + 1)][2 * k + 32 * IT2]) - _t49) + _t53 * (_t54 *
                  (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k +
                  (32 * IT2 + 1)]) - _t49) + _t55 * (_t56 * (_t63 + phi[2 * i +
                  16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (32 * IT2 + 1)]) -
                  _t49) + _t57 * (_t58 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 *
                  j + (32 * IT1 + 1)][2 * k + (32 * IT2 + 2)]) - _t49) + _t59 *
                  (_t60 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 2)][2 * k + (32 * IT2 + 1)]) - _t49) + _t61 * (_t62 * (_t63
                  + phi[2 * i + (16 * IT0 + 2)][2 * j + (32 * IT1 + 1)][2 * k +
                  (32 * IT2 + 1)]) - _t49))));
            }
            if (- dim + 32 * IT2 >= -31 && (dim + -32 * IT2 >= 3 && (dim + 1 & 1
                   ) == 0)) {
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
               _t78 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -1];
               _t64 = 2.0 * _t78;
               _t77 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               _t76 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t75 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)
                  ] + (dim + -1));
               _t74 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2
                  )] + (dim + -1));
               _t73 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + dim);
               _t72 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + dim);
               _t71 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t70 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t69 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  dim + -1));
               _t68 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t67 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (dim + -2));
               _t66 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t65 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -1] = _t78 +
                  1.0 / (_t65 - 1.0 * (double)H * (_t66 * (_t67 - 2.0) + _t68 *
                  (_t69 - 2.0) + _t70 * (_t71 - 2.0) + _t72 * (_t73 - 2.0) +
                  _t74 * (_t75 - 2.0) + _t76 * (_t77 - 2.0))) * (*(double*)(rhs
                  [2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + (dim + -1)) - (
                  _t65 * _t78 - 1.0 * (double)H * (_t66 * (_t67 * (_t78 + phi[2
                  * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -2]) - _t64) +
                  _t68 * (_t69 * (_t78 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1
                  ][dim + -1]) - _t64) + _t70 * (_t71 * (_t78 + phi[2 * i + (16
                  * IT0 + -1)][2 * j + (32 * IT1 + 1)][dim + -1]) - _t64) + _t72
                  * (_t73 * (_t78 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )][dim]) - _t64) + _t74 * (_t75 * (_t78 + phi[2 * i + 16 * IT0
                  ][2 * j + (32 * IT1 + 2)][dim + -1]) - _t64) + _t76 * (_t77 *
                  (_t78 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][
                  dim + -1]) - _t64))));
               _t93 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -1];
               _t79 = 2.0 * _t93;
               _t92 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t91 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t90 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               _t89 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t88 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + dim);
               _t87 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + dim);
               _t86 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  dim + -1));
               _t85 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t84 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + -1)] + (dim + -1));
               _t83 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t82 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -2));
               _t81 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t80 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1] + (dim + -1));
               phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -1] = _t93 +
                  1.0 / (_t80 - 1.0 * (double)H * (_t81 * (_t82 - 2.0) + _t83 *
                  (_t84 - 2.0) + _t85 * (_t86 - 2.0) + _t87 * (_t88 - 2.0) +
                  _t89 * (_t90 - 2.0) + _t91 * (_t92 - 2.0))) * (*(double*)(rhs
                  [2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + (dim + -1)) - (
                  _t80 * _t93 - 1.0 * (double)H * (_t81 * (_t82 * (_t93 + phi[2
                  * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -2]) - _t79) +
                  _t83 * (_t84 * (_t93 + phi[2 * i + (16 * IT0 + 1)][2 * j + (
                  32 * IT1 + -1)][dim + -1]) - _t79) + _t85 * (_t86 * (_t93 +
                  phi[2 * i + 16 * IT0][2 * j + 32 * IT1][dim + -1]) - _t79) +
                  _t87 * (_t88 * (_t93 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1][dim]) - _t79) + _t89 * (_t90 * (_t93 + phi[2 * i + (16
                  * IT0 + 1)][2 * j + (32 * IT1 + 1)][dim + -1]) - _t79) + _t91
                  * (_t92 * (_t93 + phi[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ][dim + -1]) - _t79))));
            }
         }
      }
   }
   if (dim + -32 * IT2 == 1) {
      int _t94;
      int i;
      for (_t94 = (__mins_32(-8 * IT0 + 16 * IT2, 7)), i = 0; i <= _t94; i++) {
         int _t95;
         int j;
         for (_t95 = (__mins_32(-16 * IT1 + 16 * IT2, 15)), j = 0; j <= _t95; j
                 ++) {
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
            _t110 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2];
            _t96 = 2.0 * _t110;
            _t109 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            _t108 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t107 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)] +
               32 * IT2);
            _t106 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)]
               + 32 * IT2);
            _t105 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               (32 * IT2 + 1));
            _t104 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + (32 * IT2 + 1));
            _t103 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t102 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + 32 * IT2);
            _t101 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + 32 *
               IT2);
            _t100 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + 32 * IT2);
            _t99 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               (32 * IT2 + -1));
            _t98 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               32 * IT2);
            _t97 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2] = _t110 +
               1.0 / (_t97 - 1.0 * (double)H * (_t98 * (_t99 - 2.0) + _t100 * (
               _t101 - 2.0) + _t102 * (_t103 - 2.0) + _t104 * (_t105 - 2.0) +
               _t106 * (_t107 - 2.0) + _t108 * (_t109 - 2.0))) * (*(double*)(
               rhs[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + 32 * IT2) - (
               _t97 * _t110 - 1.0 * (double)H * (_t98 * (_t99 * (_t110 + phi[2 *
               i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2 + -1]) - _t96) +
               _t100 * (_t101 * (_t110 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1
               ][32 * IT2]) - _t96) + _t102 * (_t103 * (_t110 + phi[2 * i + (16
               * IT0 + -1)][2 * j + (32 * IT1 + 1)][32 * IT2]) - _t96) + _t104 *
               (_t105 * (_t110 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][
               32 * IT2 + 1]) - _t96) + _t106 * (_t107 * (_t110 + phi[2 * i + 16
               * IT0][2 * j + (32 * IT1 + 2)][32 * IT2]) - _t96) + _t108 * (
               _t109 * (_t110 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)][32 * IT2]) - _t96))));
            _t125 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2];
            _t111 = 2.0 * _t125;
            _t124 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1] +
               32 * IT2);
            _t123 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t122 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            _t121 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t120 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] +
               (32 * IT2 + 1));
            _t119 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + (32 * IT2 + 1));
            _t118 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + 32 *
               IT2);
            _t117 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t116 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               -1)] + 32 * IT2);
            _t115 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t114 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] +
               (32 * IT2 + -1));
            _t113 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t112 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32 *
               IT1] + 32 * IT2);
            phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2] = _t125 +
               1.0 / (_t112 - 1.0 * (double)H * (_t113 * (_t114 - 2.0) + _t115 *
               (_t116 - 2.0) + _t117 * (_t118 - 2.0) + _t119 * (_t120 - 2.0) +
               _t121 * (_t122 - 2.0) + _t123 * (_t124 - 2.0))) * (*(double*)(
               rhs[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + 32 * IT2) - (
               _t112 * _t125 - 1.0 * (double)H * (_t113 * (_t114 * (_t125 + phi
               [2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2 + -1]) -
               _t111) + _t115 * (_t116 * (_t125 + phi[2 * i + (16 * IT0 + 1)][2
               * j + (32 * IT1 + -1)][32 * IT2]) - _t111) + _t117 * (_t118 * (
               _t125 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1][32 * IT2]) -
               _t111) + _t119 * (_t120 * (_t125 + phi[2 * i + (16 * IT0 + 1)][2
               * j + 32 * IT1][32 * IT2 + 1]) - _t111) + _t121 * (_t122 * (
               _t125 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][32 *
               IT2]) - _t111) + _t123 * (_t124 * (_t125 + phi[2 * i + (16 * IT0
               + 2)][2 * j + 32 * IT1][32 * IT2]) - _t111))));
         }
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t gsrb_mapped_kernel34_red_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{ rocrParseInputArgs(paramc, paramv, depc, depv);
   union __args_gsrb_mapped_kernel34_red_main_4
      gsrb_mapped_kernel34_red_main_args;
   union __args_gsrb_mapped_kernel34_red1_2* allArgs;
   double (* phi)[34][34];
   double const (* alpha)[34][34];
   double const (* beta_i)[34][34];
   double const (* valid)[34][34];
   double const (* beta_j)[34][34];
   double const (* beta_k)[34][34];
   double const (* rhs)[34][34];
   int dim;
   int H;
   int _t1;
   int i;
   for (gsrb_mapped_kernel34_red_main_args = *(union
           __args_gsrb_mapped_kernel34_red_main_4*)rocrArgs(depv),
        dim = gsrb_mapped_kernel34_red_main_args.data.dim,
        _t1 = dim + -1 >> 4,
        H = gsrb_mapped_kernel34_red_main_args.data.H,
        rhs = gsrb_mapped_kernel34_red_main_args.data.rhs,
        beta_k = gsrb_mapped_kernel34_red_main_args.data.beta_k,
        beta_j = gsrb_mapped_kernel34_red_main_args.data.beta_j,
        valid = gsrb_mapped_kernel34_red_main_args.data.valid,
        beta_i = gsrb_mapped_kernel34_red_main_args.data.beta_i,
        alpha = gsrb_mapped_kernel34_red_main_args.data.alpha,
        phi = gsrb_mapped_kernel34_red_main_args.data.phi,
        i = 0;
        i <= _t1; i++) {
      int _t2;
      int j;
      for (_t2 = dim + -1 >> 5, j = 0; j <= _t2; j++) {
         int _t3;
         int k;
         for (_t3 = dim + -1 >> 5, k = 0; k <= _t3; k++) {
            ocrGuid_t _t4;
            union __args_gsrb_mapped_kernel34_red1_2* _t5;
            _t4 = rocrAlloc((void**)&allArgs, 80ul);
            _t5 = allArgs;
            _t5->data.phi = phi;
            _t5->data.alpha = alpha;
            _t5->data.beta_i = beta_i;
            _t5->data.valid = valid;
            _t5->data.beta_j = beta_j;
            _t5->data.beta_k = beta_k;
            _t5->data.rhs = rhs;
            _t5->data.dim = dim;
            _t5->data.H = H;
            _t5->data.IT0 = i;
            _t5->data.IT1 = j;
            _t5->data.IT2 = k;
            rocrCreateTask(0, _t4, (unsigned int)(k + j + i + (j + 2 * i) * (-1
               + dim >> 5) + i * (-1 + dim >> 5) * (-1 + dim >> 5)), dim, H, i,
                j, k);
         }
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

/*
 * Optimization PARSE[-I//usr/include, -I/home/konstantinidis/Work/ocr-db-autodec/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DbSIZE=66, -DH2INV=4096.0, -D__RSTREAM_CC__, /home/konstantinidis/HPGMG/xstack-traleika-apps/sandbox/konstantinidis/hpgmg/finite-volume/source/operators/gsrb_mapped_kernel66_red.c] (231ms)
 * Optimization SSA (103ms)
 * Optimization CCP (50ms)
 * Optimization GVNGCM (74ms)
 *   Optimization SimplifyControl (16ms)
 * Optimization OSR (58ms)
 * Optimization DCE (12ms)
 * Optimization Raise[-liftnone] (703ms)
 * Optimization PolyhedralMapperNew[tile=sizes={8-16-16}] (3128ms)
 * Optimization Lower (589ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (13ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization InsertMapperAnnotations (3ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization DCE (49ms)
 * Optimization SyntaxPrepass (16ms)
 * Optimization DCE (36ms)
 * Optimization DeSSA (822ms)
 */
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_1_red_66;
union __args_gsrb_mapped_kernel66_red1_2;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_3_red_66;
union __args_gsrb_mapped_kernel66_red_main_4;
union ocrGuidUnion_t;
struct va_list_t;
struct __anon_1_red_66 {
   double (* phi)[66][66];
   double const (* alpha)[66][66];
   double const (* beta_i)[66][66];
   double const (* valid)[66][66];
   double const (* beta_j)[66][66];
   double const (* beta_k)[66][66];
   double const (* rhs)[66][66];
   int dim;
   int H;
   int IT0;
   int IT1;
   int IT2;
};
union __args_gsrb_mapped_kernel66_red1_2 {
   struct __anon_1_red_66 data;
   double padding[10];
};
struct __anon_3_red_66 {
   double (* phi)[66][66];
   double const (* alpha)[66][66];
   double const (* beta_i)[66][66];
   double const (* valid)[66][66];
   double const (* beta_j)[66][66];
   double const (* beta_k)[66][66];
   double const (* rhs)[66][66];
   int dim;
   int H;
};
union __args_gsrb_mapped_kernel66_red_main_4 {
   struct __anon_3_red_66 data;
   double padding[8];
};
/*
 * Forward declarations of functions
 */
void gsrb_mapped_kernel66_red(int dim, double (* phi)[66][66], double const (*
   Dinv)[66][66], double const (* rhs)[66][66], double a, int H, double const (
   * alpha)[66][66], double const (* beta_i)[66][66], double const (* beta_j)[
   66][66], double const (* beta_k)[66][66], double const (* valid)[66][66]);
static ocrGuid_t gsrb_mapped_kernel66_red1(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
ocrGuid_t rocrAlloc(void** dim, unsigned long k);
static ocrGuid_t gsrb_mapped_kernel66_red_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void rocrExit(void);
/*
 * Definitions of functions
 */
void gsrb_mapped_kernel66_red(int dim, double (* phi)[66][66], double const (*
   Dinv)[66][66], double const (* rhs)[66][66], double a, int H, double const (
   * alpha)[66][66], double const (* beta_i)[66][66], double const (* beta_j)[
   66][66], double const (* beta_k)[66][66], double const (* valid)[66][66])
{
   union __args_gsrb_mapped_kernel66_red_main_4* allArgs;
   rocrDeclareType(gsrb_mapped_kernel66_red_main, 1, (unsigned int)1, 1, (
      unsigned long (*)(va_list))((void*)0l));
   rocrDeclareType(gsrb_mapped_kernel66_red1, 0, (unsigned int)(dim >= 1 ? 1 + (
      -1 + dim >> 4) + (2 + 2 * (-1 + dim >> 4)) * (-1 + dim >> 5) + (1 + (-1 +
      dim >> 4)) * (-1 + dim >> 5) * (-1 + dim >> 5) : 0), 0, (unsigned long (*
      )(va_list))((void*)0l));
   rocrInit();
   if (dim >= 1) {
      ocrGuid_t _t1;
      union __args_gsrb_mapped_kernel66_red_main_4* _t2;
      _t1 = rocrAlloc((void**)&allArgs, 64ul);
      _t2 = allArgs;
      _t2->data.phi = phi;
      _t2->data.alpha = alpha;
      _t2->data.beta_i = beta_i;
      _t2->data.valid = valid;
      _t2->data.beta_j = beta_j;
      _t2->data.beta_k = beta_k;
      _t2->data.rhs = rhs;
      _t2->data.dim = dim;
      _t2->data.H = H;
      rocrExecute(1, _t1);
   }
   rocrExit();
}

static ocrGuid_t gsrb_mapped_kernel66_red1(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{ rocrParseInputArgs(paramc, paramv, depc, depv);
   union __args_gsrb_mapped_kernel66_red1_2 gsrb_mapped_kernel66_red1_args;
   double (* phi)[66][66];
   double const (* alpha)[66][66];
   double const (* beta_i)[66][66];
   double const (* valid)[66][66];
   double const (* beta_j)[66][66];
   double const (* beta_k)[66][66];
   double const (* rhs)[66][66];
   int dim;
   int H;
   int IT0;
   int IT1;
   int IT2;
   double _v1;
   gsrb_mapped_kernel66_red1_args = *(union __args_gsrb_mapped_kernel66_red1_2*
      )rocrArgs(depv);
   IT1 = gsrb_mapped_kernel66_red1_args.data.IT1;
   IT0 = gsrb_mapped_kernel66_red1_args.data.IT0;
   H = gsrb_mapped_kernel66_red1_args.data.H;
   rhs = gsrb_mapped_kernel66_red1_args.data.rhs;
   beta_k = gsrb_mapped_kernel66_red1_args.data.beta_k;
   beta_j = gsrb_mapped_kernel66_red1_args.data.beta_j;
   valid = gsrb_mapped_kernel66_red1_args.data.valid;
   beta_i = gsrb_mapped_kernel66_red1_args.data.beta_i;
   alpha = gsrb_mapped_kernel66_red1_args.data.alpha;
   phi = gsrb_mapped_kernel66_red1_args.data.phi;
   IT2 = gsrb_mapped_kernel66_red1_args.data.IT2;
   dim = gsrb_mapped_kernel66_red1_args.data.dim;
   if (dim + -32 * IT2 >= 2) {
      int _t1;
      int i;
      for (_t1 = (__mins_32(dim + -16 * IT0 + -1 >> 1, 7)), i = 0; i <= _t1; i
              ++) {
         int _t2;
         int j;
         for (_t2 = (__mins_32(dim + -32 * IT1 + -1 >> 1, 15)), j = 0; j <= _t2
                 ; j++) {
            int _t3;
            int k;
            for (_t3 = (__mins_32(dim + -32 * IT2 + -2 >> 1, 15)), k = 0; k <=
                    _t3; k++) {
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
               _t18 = phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2
                  + 1)];
               _t4 = 2.0 * _t18;
               _t17 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t16 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t15 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t14 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1)));
               _t13 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 2)));
               _t12 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  2 * k + (32 * IT2 + 2)));
               _t11 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t10 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  2 * k + (32 * IT2 + 1)));
               _t9 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + -1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t8 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 1)));
               _t7 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2 *
                  k + 32 * IT2));
               _t6 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + (32 * IT2 + 1)));
               _t5 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + 32 * IT1]
                  + (2 * k + (32 * IT2 + 1)));
               phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2 + 1)] =
                  _t18 + 1.0 / (_t5 - 1.0 * (double)H * (_t6 * (_t7 - 2.0) + _t8
                  * (_t9 - 2.0) + _t10 * (_t11 - 2.0) + _t12 * (_t13 - 2.0) +
                  _t14 * (_t15 - 2.0) + _t16 * (_t17 - 2.0))) * (*(double*)(rhs
                  [2 * i + 16 * IT0][2 * j + 32 * IT1] + (2 * k + (32 * IT2 + 1
                  ))) - (_t5 * _t18 - 1.0 * (double)H * (_t6 * (_t7 * (_t18 +
                  phi[2 * i + 16 * IT0][2 * j + 32 * IT1][2 * k + 32 * IT2]) -
                  _t4) + _t8 * (_t9 * (_t18 + phi[2 * i + 16 * IT0][2 * j + (32
                  * IT1 + -1)][2 * k + (32 * IT2 + 1)]) - _t4) + _t10 * (_t11 *
                  (_t18 + phi[2 * i + (16 * IT0 + -1)][2 * j + 32 * IT1][2 * k +
                  (32 * IT2 + 1)]) - _t4) + _t12 * (_t13 * (_t18 + phi[2 * i +
                  16 * IT0][2 * j + 32 * IT1][2 * k + (32 * IT2 + 2)]) - _t4) +
                  _t14 * (_t15 * (_t18 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)][2 * k + (32 * IT2 + 1)]) - _t4) + _t16 * (_t17 * (_t18 +
                  phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (32 *
                  IT2 + 1)]) - _t4))));
               _t33 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + 32 *
                  IT2];
               _t19 = 2.0 * _t33;
               _t32 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t31 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t30 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)
                  ] + (2 * k + 32 * IT2));
               _t29 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2
                  )] + (2 * k + 32 * IT2));
               _t28 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t27 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1)));
               _t26 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t25 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t24 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + 32 * IT2));
               _t23 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t22 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + -1)));
               _t21 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (2 * k + 32 * IT2));
               _t20 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + 32 * IT2] =
                  _t33 + 1.0 / (_t20 - 1.0 * (double)H * (_t21 * (_t22 - 2.0) +
                  _t23 * (_t24 - 2.0) + _t25 * (_t26 - 2.0) + _t27 * (_t28 - 2.0
                  ) + _t29 * (_t30 - 2.0) + _t31 * (_t32 - 2.0))) * (*(double*)
                  (rhs[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + (2 * k + 32 *
                  IT2)) - (_t20 * _t33 - 1.0 * (double)H * (_t21 * (_t22 * (
                  _t33 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (
                  32 * IT2 + -1)]) - _t19) + _t23 * (_t24 * (_t33 + phi[2 * i +
                  16 * IT0][2 * j + 32 * IT1][2 * k + 32 * IT2]) - _t19) + _t25
                  * (_t26 * (_t33 + phi[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)][2 * k + 32 * IT2]) - _t19) + _t27 * (_t28 * (_t33 +
                  phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (32 *
                  IT2 + 1)]) - _t19) + _t29 * (_t30 * (_t33 + phi[2 * i + 16 *
                  IT0][2 * j + (32 * IT1 + 2)][2 * k + 32 * IT2]) - _t19) + _t31
                  * (_t32 * (_t33 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)][2 * k + 32 * IT2]) - _t19))));
               _t48 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + 32 *
                  IT2];
               _t34 = 2.0 * _t48;
               _t47 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t46 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t45 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t44 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + 32 * IT2));
               _t43 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t42 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t41 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (2
                  * k + 32 * IT2));
               _t40 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t39 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + -1)] + (2 * k + 32 * IT2));
               _t38 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t37 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + -1)));
               _t36 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + 32 * IT2));
               _t35 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1] + (2 * k + 32 * IT2));
               phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + 32 * IT2] =
                  _t48 + 1.0 / (_t35 - 1.0 * (double)H * (_t36 * (_t37 - 2.0) +
                  _t38 * (_t39 - 2.0) + _t40 * (_t41 - 2.0) + _t42 * (_t43 - 2.0
                  ) + _t44 * (_t45 - 2.0) + _t46 * (_t47 - 2.0))) * (*(double*)
                  (rhs[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + (2 * k + 32 *
                  IT2)) - (_t35 * _t48 - 1.0 * (double)H * (_t36 * (_t37 * (
                  _t48 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (
                  32 * IT2 + -1)]) - _t34) + _t38 * (_t39 * (_t48 + phi[2 * i +
                  (16 * IT0 + 1)][2 * j + (32 * IT1 + -1)][2 * k + 32 * IT2]) -
                  _t34) + _t40 * (_t41 * (_t48 + phi[2 * i + 16 * IT0][2 * j +
                  32 * IT1][2 * k + 32 * IT2]) - _t34) + _t42 * (_t43 * (_t48 +
                  phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k + (32 *
                  IT2 + 1)]) - _t34) + _t44 * (_t45 * (_t48 + phi[2 * i + (16 *
                  IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k + 32 * IT2]) - _t34) +
                  _t46 * (_t47 * (_t48 + phi[2 * i + (16 * IT0 + 2)][2 * j + 32
                  * IT1][2 * k + 32 * IT2]) - _t34))));
               _t63 = phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k
                  + (32 * IT2 + 1)];
               _t49 = 2.0 * _t63;
               _t62 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + (32 * IT1
                  + 1)] + (2 * k + (32 * IT2 + 1)));
               _t61 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t60 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 2)] + (2 * k + (32 * IT2 + 1)));
               _t59 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 2)] + (2 * k + (32 * IT2 + 1)));
               _t58 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + (32 * IT2 + 2)));
               _t57 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 2)));
               _t56 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (2 * k + (32 * IT2 + 1)));
               _t55 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t54 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (2 * k + (32 * IT2 + 1)));
               _t53 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t52 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (2 * k + 32 * IT2));
               _t51 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               _t50 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + (
                  32 * IT1 + 1)] + (2 * k + (32 * IT2 + 1)));
               phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][2 * k + (32 *
                  IT2 + 1)] = _t63 + 1.0 / (_t50 - 1.0 * (double)H * (_t51 * (
                  _t52 - 2.0) + _t53 * (_t54 - 2.0) + _t55 * (_t56 - 2.0) + _t57
                  * (_t58 - 2.0) + _t59 * (_t60 - 2.0) + _t61 * (_t62 - 2.0))) *
                  (*(double*)(rhs[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1
                  )] + (2 * k + (32 * IT2 + 1))) - (_t50 * _t63 - 1.0 * (double
                  )H * (_t51 * (_t52 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j
                  + (32 * IT1 + 1)][2 * k + 32 * IT2]) - _t49) + _t53 * (_t54 *
                  (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][2 * k +
                  (32 * IT2 + 1)]) - _t49) + _t55 * (_t56 * (_t63 + phi[2 * i +
                  16 * IT0][2 * j + (32 * IT1 + 1)][2 * k + (32 * IT2 + 1)]) -
                  _t49) + _t57 * (_t58 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 *
                  j + (32 * IT1 + 1)][2 * k + (32 * IT2 + 2)]) - _t49) + _t59 *
                  (_t60 * (_t63 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 2)][2 * k + (32 * IT2 + 1)]) - _t49) + _t61 * (_t62 * (_t63
                  + phi[2 * i + (16 * IT0 + 2)][2 * j + (32 * IT1 + 1)][2 * k +
                  (32 * IT2 + 1)]) - _t49))));
            }
            if (- dim + 32 * IT2 >= -31 && (dim + -32 * IT2 >= 3 && (dim + 1 & 1
                   ) == 0)) {
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
               _t78 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -1];
               _t64 = 2.0 * _t78;
               _t77 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               _t76 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t75 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)
                  ] + (dim + -1));
               _t74 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2
                  )] + (dim + -1));
               _t73 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + dim);
               _t72 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + dim);
               _t71 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t70 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t69 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  dim + -1));
               _t68 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t67 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)
                  ] + (dim + -2));
               _t66 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )] + (dim + -1));
               _t65 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -1] = _t78 +
                  1.0 / (_t65 - 1.0 * (double)H * (_t66 * (_t67 - 2.0) + _t68 *
                  (_t69 - 2.0) + _t70 * (_t71 - 2.0) + _t72 * (_t73 - 2.0) +
                  _t74 * (_t75 - 2.0) + _t76 * (_t77 - 2.0))) * (*(double*)(rhs
                  [2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + (dim + -1)) - (
                  _t65 * _t78 - 1.0 * (double)H * (_t66 * (_t67 * (_t78 + phi[2
                  * i + 16 * IT0][2 * j + (32 * IT1 + 1)][dim + -2]) - _t64) +
                  _t68 * (_t69 * (_t78 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1
                  ][dim + -1]) - _t64) + _t70 * (_t71 * (_t78 + phi[2 * i + (16
                  * IT0 + -1)][2 * j + (32 * IT1 + 1)][dim + -1]) - _t64) + _t72
                  * (_t73 * (_t78 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1
                  )][dim]) - _t64) + _t74 * (_t75 * (_t78 + phi[2 * i + 16 * IT0
                  ][2 * j + (32 * IT1 + 2)][dim + -1]) - _t64) + _t76 * (_t77 *
                  (_t78 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][
                  dim + -1]) - _t64))));
               _t93 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -1];
               _t79 = 2.0 * _t93;
               _t92 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t91 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t90 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + 1)] + (dim + -1));
               _t89 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 *
                  IT1 + 1)] + (dim + -1));
               _t88 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + dim);
               _t87 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + dim);
               _t86 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + (
                  dim + -1));
               _t85 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t84 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
                  + -1)] + (dim + -1));
               _t83 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t82 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -2));
               _t81 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1
                  ] + (dim + -1));
               _t80 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1] + (dim + -1));
               phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -1] = _t93 +
                  1.0 / (_t80 - 1.0 * (double)H * (_t81 * (_t82 - 2.0) + _t83 *
                  (_t84 - 2.0) + _t85 * (_t86 - 2.0) + _t87 * (_t88 - 2.0) +
                  _t89 * (_t90 - 2.0) + _t91 * (_t92 - 2.0))) * (*(double*)(rhs
                  [2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + (dim + -1)) - (
                  _t80 * _t93 - 1.0 * (double)H * (_t81 * (_t82 * (_t93 + phi[2
                  * i + (16 * IT0 + 1)][2 * j + 32 * IT1][dim + -2]) - _t79) +
                  _t83 * (_t84 * (_t93 + phi[2 * i + (16 * IT0 + 1)][2 * j + (
                  32 * IT1 + -1)][dim + -1]) - _t79) + _t85 * (_t86 * (_t93 +
                  phi[2 * i + 16 * IT0][2 * j + 32 * IT1][dim + -1]) - _t79) +
                  _t87 * (_t88 * (_t93 + phi[2 * i + (16 * IT0 + 1)][2 * j + 32
                  * IT1][dim]) - _t79) + _t89 * (_t90 * (_t93 + phi[2 * i + (16
                  * IT0 + 1)][2 * j + (32 * IT1 + 1)][dim + -1]) - _t79) + _t91
                  * (_t92 * (_t93 + phi[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1
                  ][dim + -1]) - _t79))));
            }
         }
      }
   }
   if (dim + -32 * IT2 == 1) {
      int _t94;
      int i;
      for (_t94 = (__mins_32(-8 * IT0 + 16 * IT2, 7)), i = 0; i <= _t94; i++) {
         int _t95;
         int j;
         for (_t95 = (__mins_32(-16 * IT1 + 16 * IT2, 15)), j = 0; j <= _t95; j
                 ++) {
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
            _t110 = phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2];
            _t96 = 2.0 * _t110;
            _t109 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            _t108 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t107 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)] +
               32 * IT2);
            _t106 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 2)]
               + 32 * IT2);
            _t105 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               (32 * IT2 + 1));
            _t104 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + (32 * IT2 + 1));
            _t103 = *(double*)(valid[2 * i + (16 * IT0 + -1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t102 = *(double*)(beta_k[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + 32 * IT2);
            _t101 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + 32 *
               IT2);
            _t100 = *(double*)(beta_j[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)]
               + 32 * IT2);
            _t99 = *(double*)(valid[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               (32 * IT2 + -1));
            _t98 = *(double*)(beta_i[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] +
               32 * IT2);
            _t97 = _v1 * *(double*)(alpha[2 * i + 16 * IT0][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2] = _t110 +
               1.0 / (_t97 - 1.0 * (double)H * (_t98 * (_t99 - 2.0) + _t100 * (
               _t101 - 2.0) + _t102 * (_t103 - 2.0) + _t104 * (_t105 - 2.0) +
               _t106 * (_t107 - 2.0) + _t108 * (_t109 - 2.0))) * (*(double*)(
               rhs[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)] + 32 * IT2) - (
               _t97 * _t110 - 1.0 * (double)H * (_t98 * (_t99 * (_t110 + phi[2 *
               i + 16 * IT0][2 * j + (32 * IT1 + 1)][32 * IT2 + -1]) - _t96) +
               _t100 * (_t101 * (_t110 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1
               ][32 * IT2]) - _t96) + _t102 * (_t103 * (_t110 + phi[2 * i + (16
               * IT0 + -1)][2 * j + (32 * IT1 + 1)][32 * IT2]) - _t96) + _t104 *
               (_t105 * (_t110 + phi[2 * i + 16 * IT0][2 * j + (32 * IT1 + 1)][
               32 * IT2 + 1]) - _t96) + _t106 * (_t107 * (_t110 + phi[2 * i + 16
               * IT0][2 * j + (32 * IT1 + 2)][32 * IT2]) - _t96) + _t108 * (
               _t109 * (_t110 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)][32 * IT2]) - _t96))));
            _t125 = phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2];
            _t111 = 2.0 * _t125;
            _t124 = *(double*)(valid[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1] +
               32 * IT2);
            _t123 = *(double*)(beta_k[2 * i + (16 * IT0 + 2)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t122 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               1)] + 32 * IT2);
            _t121 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1
               + 1)] + 32 * IT2);
            _t120 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] +
               (32 * IT2 + 1));
            _t119 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + (32 * IT2 + 1));
            _t118 = *(double*)(valid[2 * i + 16 * IT0][2 * j + 32 * IT1] + 32 *
               IT2);
            _t117 = *(double*)(beta_k[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t116 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 +
               -1)] + 32 * IT2);
            _t115 = *(double*)(beta_j[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t114 = *(double*)(valid[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] +
               (32 * IT2 + -1));
            _t113 = *(double*)(beta_i[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1]
               + 32 * IT2);
            _t112 = _v1 * *(double*)(alpha[2 * i + (16 * IT0 + 1)][2 * j + 32 *
               IT1] + 32 * IT2);
            phi[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2] = _t125 +
               1.0 / (_t112 - 1.0 * (double)H * (_t113 * (_t114 - 2.0) + _t115 *
               (_t116 - 2.0) + _t117 * (_t118 - 2.0) + _t119 * (_t120 - 2.0) +
               _t121 * (_t122 - 2.0) + _t123 * (_t124 - 2.0))) * (*(double*)(
               rhs[2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1] + 32 * IT2) - (
               _t112 * _t125 - 1.0 * (double)H * (_t113 * (_t114 * (_t125 + phi
               [2 * i + (16 * IT0 + 1)][2 * j + 32 * IT1][32 * IT2 + -1]) -
               _t111) + _t115 * (_t116 * (_t125 + phi[2 * i + (16 * IT0 + 1)][2
               * j + (32 * IT1 + -1)][32 * IT2]) - _t111) + _t117 * (_t118 * (
               _t125 + phi[2 * i + 16 * IT0][2 * j + 32 * IT1][32 * IT2]) -
               _t111) + _t119 * (_t120 * (_t125 + phi[2 * i + (16 * IT0 + 1)][2
               * j + 32 * IT1][32 * IT2 + 1]) - _t111) + _t121 * (_t122 * (
               _t125 + phi[2 * i + (16 * IT0 + 1)][2 * j + (32 * IT1 + 1)][32 *
               IT2]) - _t111) + _t123 * (_t124 * (_t125 + phi[2 * i + (16 * IT0
               + 2)][2 * j + 32 * IT1][32 * IT2]) - _t111))));
         }
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}

static ocrGuid_t gsrb_mapped_kernel66_red_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{ rocrParseInputArgs(paramc, paramv, depc, depv);
   union __args_gsrb_mapped_kernel66_red_main_4
      gsrb_mapped_kernel66_red_main_args;
   union __args_gsrb_mapped_kernel66_red1_2* allArgs;
   double (* phi)[66][66];
   double const (* alpha)[66][66];
   double const (* beta_i)[66][66];
   double const (* valid)[66][66];
   double const (* beta_j)[66][66];
   double const (* beta_k)[66][66];
   double const (* rhs)[66][66];
   int dim;
   int H;
   int _t1;
   int i;
   for (gsrb_mapped_kernel66_red_main_args = *(union
           __args_gsrb_mapped_kernel66_red_main_4*)rocrArgs(depv),
        dim = gsrb_mapped_kernel66_red_main_args.data.dim,
        _t1 = dim + -1 >> 4,
        H = gsrb_mapped_kernel66_red_main_args.data.H,
        rhs = gsrb_mapped_kernel66_red_main_args.data.rhs,
        beta_k = gsrb_mapped_kernel66_red_main_args.data.beta_k,
        beta_j = gsrb_mapped_kernel66_red_main_args.data.beta_j,
        valid = gsrb_mapped_kernel66_red_main_args.data.valid,
        beta_i = gsrb_mapped_kernel66_red_main_args.data.beta_i,
        alpha = gsrb_mapped_kernel66_red_main_args.data.alpha,
        phi = gsrb_mapped_kernel66_red_main_args.data.phi,
        i = 0;
        i <= _t1; i++) {
      int _t2;
      int j;
      for (_t2 = dim + -1 >> 5, j = 0; j <= _t2; j++) {
         int _t3;
         int k;
         for (_t3 = dim + -1 >> 5, k = 0; k <= _t3; k++) {
            ocrGuid_t _t4;
            union __args_gsrb_mapped_kernel66_red1_2* _t5;
            _t4 = rocrAlloc((void**)&allArgs, 80ul);
            _t5 = allArgs;
            _t5->data.phi = phi;
            _t5->data.alpha = alpha;
            _t5->data.beta_i = beta_i;
            _t5->data.valid = valid;
            _t5->data.beta_j = beta_j;
            _t5->data.beta_k = beta_k;
            _t5->data.rhs = rhs;
            _t5->data.dim = dim;
            _t5->data.H = H;
            _t5->data.IT0 = i;
            _t5->data.IT1 = j;
            _t5->data.IT2 = k;
            rocrCreateTask(0, _t4, (unsigned int)(k + j + i + (j + 2 * i) * (-1
               + dim >> 5) + i * (-1 + dim >> 5) * (-1 + dim >> 5)), dim, H, i,
                j, k);
         }
      }
   }
   rocrFreeInputArgs(paramc, paramv, depc, depv);
   return NULL_GUID;
}


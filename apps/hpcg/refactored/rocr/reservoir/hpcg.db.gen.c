/*
 * Optimization PARSE[-I//usr/include, -I/home/pradelle/work/pca-ocr-db-autodec/rstream/target_spec/amd64/include, -I//usr/include/x86_64-linux-gnu, -I../include, -I/home/pradelle/work/pca-ocr-db-autodec/runtime/codelet/ocr/inc, -D__extension__=, -D__amd64__, -D__x86_64__, -D_GNU_SOURCE=1, -D__DEPRECATED=1, -D__EXCEPTIONS=1, -D__GXX_WEAK__=1, -D__CHAR_BIT__=8, -D__DBL_DENORM_MIN__=4.9406564584124654e-324, -D__DBL_DIG__=15, -D__DBL_EPSILON__=2.2204460492503131e-16, -D__DBL_HAS_INFINITY__=1, -D__DBL_HAS_QUIET_NAN__=1, -D__DBL_MANT_DIG__=53, -D__DBL_MAX_10_EXP__=308, -D__DBL_MAX_EXP__=1024, -D__DBL_MAX__=1.7976931348623157e+308, -D__DBL_MIN_10_EXP__=(-307), -D__DBL_MIN_EXP__=(-1021), -D__DBL_MIN__=2.2250738585072014e-308, -D__DECIMAL_DIG__=21, -D__ELF__=1, -D__FINITE_MATH_ONLY__=0, -D__FLT_DENORM_MIN__=1.40129846e-45F, -D__FLT_DIG__=6, -D__FLT_EPSILON__=1.19209290e-7F, -D__FLT_EVAL_METHOD__=2, -D__FLT_HAS_INFINITY__=1, -D__FLT_HAS_QUIET_NAN__=1, -D__FLT_MANT_DIG__=24, -D__FLT_MAX_10_EXP__=38, -D__FLT_MAX_EXP__=128, -D__FLT_MAX__=3.40282347e+38F, -D__FLT_MIN_10_EXP__=(-37), -D__FLT_MIN_EXP__=(-125), -D__FLT_MIN__=1.17549435e-38F, -D__FLT_RADIX__=2, -D__GNUC_RH_RELEASE__=1, -D__GXX_ABI_VERSION=1002, -D__INTMAX_MAX__=9223372036854775807LL, -D__INTMAX_TYPE__=long long int, -D__INT_MAX__=2147483647, -D__LDBL_DENORM_MIN__=3.64519953188247460253e-4951L, -D__LDBL_DIG__=18, -D__LDBL_EPSILON__=1.08420217248550443401e-19L, -D__LDBL_HAS_INFINITY__=1, -D__LDBL_HAS_QUIET_NAN__=1, -D__LDBL_MANT_DIG__=64, -D__LDBL_MAX_10_EXP__=4932, -D__LDBL_MAX_EXP__=16384, -D__LDBL_MAX__=1.18973149535723176502e+4932L, -D__LDBL_MIN_10_EXP__=(-4931), -D__LDBL_MIN_EXP__=(-16381), -D__LDBL_MIN__=3.36210314311209350626e-4932L, -D__LONG_LONG_MAX__=9223372036854775807LL, -D__LONG_MAX__=2147483647L, -D__NO_INLINE__=1, -D__REGISTER_PREFIX__=, -D__SCHAR_MAX__=127, -D__SHRT_MAX__=32767, -D__UINTMAX_TYPE__=long long unsigned int, -D__USER_LABEL_PREFIX__=, -D__WCHAR_MAX__=2147483647, -D__WINT_TYPE__=unsigned int, -D__gnu_linux__=1, -D__i386=1, -D__i386__=1, -D__linux=1, -D__linux__=1, -D__unix=1, -D__unix__=1, -Di386=1, -Dlinux=1, -Dunix=1, -DOCR_TARGET, --c99, -D__RSTREAM_CC__, ../src/hpcg_precond_fissioned.c] (794ms)
 * Optimization SSA (167ms)
 * Optimization CCP (51ms)
 * Optimization DCE (24ms)
 * Optimization Raise[-liftnone, -stacktrace] (1053ms)
 *   Optimization CCP (19ms)
 *   Optimization DCE (6ms)
 *   Optimization DeSSA (28ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (4ms)
 *   Optimization DeSSA (20ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (7ms)
 *   Optimization DeSSA (17ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (5ms)
 * Optimization PolyhedralMapperNew[-stacktrace, no-simplify-loop, no-spatial-layout, stacktrace=true, scal-dep, threadf=reduce_create=true, as=scopes, datablocks, datablock:default-size=65536, datablock:copy_mode=no_copy, logfile=hpcg_precond_fissioned-mapper.log] (23162ms)
 * Optimization Lower[-stacktrace] (11365ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (4ms)
 *   Optimization SSA (4ms)
 *   Optimization SSA (6ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (12ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (4ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (15ms)
 *   Optimization SSA (7ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (6ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (11ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (5ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (6ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (5ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (6ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (5ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (5ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (6ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (6ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (5ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (5ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (7ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (6ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (13ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (18ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (5ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (26ms)
 *   Optimization SSA (5ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (4ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (12ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (15ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (21ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (4ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (10ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (27ms)
 *   Optimization SSA (21ms)
 *   Optimization SSA (17ms)
 *   Optimization SSA (13ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (11ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (13ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (32ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (4ms)
 *   Optimization SSA (4ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (12ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (7ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (10ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (6ms)
 *   Optimization SSA (5ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (8ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (14ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (3ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
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
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
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
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (2ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
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
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
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
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (5ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (4ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
 *   Optimization InsertMapperAnnotations (3ms)
 *   Optimization InsertMapperAnnotations (3ms)
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
 *   Optimization InsertMapperAnnotations (2ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (3ms)
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
 *   Optimization InsertMapperAnnotations (6ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (1ms)
 *   Optimization InsertMapperAnnotations (17ms)
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
 *   Optimization InsertMapperAnnotations (4ms)
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
 *   Optimization InsertMapperAnnotations (4ms)
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
 *   Optimization InsertMapperAnnotations (1ms)
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
 *   Optimization SSA (1ms)
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
 *   Optimization SSA (1ms)
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
 *   Optimization SSA (1ms)
 *   Optimization SSA (1ms)
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
 *   Optimization SSA (1ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
 *   Optimization SSA (0ms)
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
 *   Optimization DCE (73ms)
 * Optimization SyntaxPrepass (125ms)
 * Optimization OSR (198ms)
 * Optimization DCE (60ms)
 * Optimization DeSSA (962ms)
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
union __args_mapped_step1_noGVars92_50;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_51;
union __args_mapped_step1_noGVars95_52;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_53;
union __args_mapped_step1_noGVars169_54;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_55;
union __args_mapped_step1_noGVars97_56;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_57;
union __args_mapped_step1_noGVars99_58;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_59;
union __args_mapped_step1_noGVars102_60;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_61;
union __args_mapped_step1_noGVars170_62;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_63;
union __args_mapped_step1_noGVars109_64;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_65;
union __args_mapped_step1_noGVars112_66;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_67;
union __args_mapped_step1_noGVars117_68;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_69;
union __args_mapped_step1_noGVars119_70;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_71;
union __args_mapped_step1_noGVars122_72;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_73;
union __args_mapped_step1_noGVars172_74;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_75;
union __args_mapped_step1_noGVars173_76;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_77;
union __args_mapped_step1_noGVars124_78;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_79;
union __args_mapped_step1_noGVars126_80;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_81;
union __args_mapped_step1_noGVars129_82;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_83;
union __args_mapped_step1_noGVars132_84;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_85;
union __args_mapped_step1_noGVars134_86;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_91;
union __args_mapped_step1_noGVars136_92;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_93;
union __args_mapped_step1_noGVars139_94;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_97;
union __args_mapped_step1_noGVars146_98;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_99;
union __args_mapped_step1_noGVars149_100;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_103;
union __args_mapped_step1_noGVars156_104;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_105;
union __args_mapped_step1_noGVars159_106;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_111;
union __args_mapped_step1_noGVars161_112;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_113;
union __args_mapped_step1_noGVars163_114;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_115;
union __args_mapped_step1_noGVars166_116;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_117;
union __args_mapped_step1_noGVars_main_118;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_119;
union __args_mapped_step5_dpbr_noGVars124_120;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_121;
union __args_mapped_step5_dpbr_noGVars127_122;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_123;
union __args_mapped_step5_dpbr_noGVars195_124;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_125;
union __args_mapped_step5_dpbr_noGVars129_126;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_127;
union __args_mapped_step5_dpbr_noGVars131_128;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_129;
union __args_mapped_step5_dpbr_noGVars197_130;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_131;
union __args_mapped_step5_dpbr_noGVars133_132;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_133;
union __args_mapped_step5_dpbr_noGVars135_134;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_135;
union __args_mapped_step5_dpbr_noGVars199_136;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_137;
union __args_mapped_step5_dpbr_noGVars200_138;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_139;
union __args_mapped_step5_dpbr_noGVars201_140;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_141;
union __args_mapped_step5_dpbr_noGVars202_142;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_143;
union __args_mapped_step5_dpbr_noGVars137_144;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_145;
union __args_mapped_step5_dpbr_noGVars139_146;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_147;
union __args_mapped_step5_dpbr_noGVars204_148;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_149;
union __args_mapped_step5_dpbr_noGVars205_150;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_151;
union __args_mapped_step5_dpbr_noGVars143_152;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_153;
union __args_mapped_step5_dpbr_noGVars146_154;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_155;
union __args_mapped_step5_dpbr_noGVars206_156;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_157;
union __args_mapped_step5_dpbr_noGVars207_158;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_159;
union __args_mapped_step5_dpbr_noGVars151_160;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_161;
union __args_mapped_step5_dpbr_noGVars153_162;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_163;
union __args_mapped_step5_dpbr_noGVars156_164;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_165;
union __args_mapped_step5_dpbr_noGVars158_166;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_167;
union __args_mapped_step5_dpbr_noGVars160_168;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_169;
union __args_mapped_step5_dpbr_noGVars163_170;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_171;
union __args_mapped_step5_dpbr_noGVars165_172;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_177;
union __args_mapped_step5_dpbr_noGVars170_178;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_179;
union __args_mapped_step5_dpbr_noGVars173_180;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_185;
union __args_mapped_step5_dpbr_noGVars177_186;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_187;
union __args_mapped_step5_dpbr_noGVars213_188;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_189;
union __args_mapped_step5_dpbr_noGVars214_190;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_197;
union __args_mapped_step5_dpbr_noGVars181_198;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_203;
union __args_mapped_step5_dpbr_noGVars185_204;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_209;
union __args_mapped_step5_dpbr_noGVars187_210;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_211;
union __args_mapped_step5_dpbr_noGVars189_212;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_213;
union __args_mapped_step5_dpbr_noGVars192_214;
union ocrGuidUnion_t;
struct ocrEdtDepStruct_t;
struct __anon_215;
union __args_mapped_step5_dpbr_noGVars_main_216;
union ocrGuidUnion_t;
struct __anon_207;
union __args_mapped_step5_dpbr_noGVars221_208;
struct __anon_205;
union __args_mapped_step5_dpbr_noGVars220_206;
struct __anon_109;
union __args_mapped_step1_noGVars179_110;
struct __anon_107;
union __args_mapped_step1_noGVars178_108;
struct __anon_101;
union __args_mapped_step1_noGVars151_102;
struct __anon_95;
union __args_mapped_step1_noGVars141_96;
struct __anon_89;
union __args_mapped_step1_noGVars175_90;
struct __anon_87;
union __args_mapped_step1_noGVars174_88;
struct __anon_181;
union __args_mapped_step5_dpbr_noGVars211_182;
struct __anon_201;
union __args_mapped_step5_dpbr_noGVars219_202;
struct __anon_195;
union __args_mapped_step5_dpbr_noGVars217_196;
struct __anon_183;
union __args_mapped_step5_dpbr_noGVars212_184;
struct __anon_175;
union __args_mapped_step5_dpbr_noGVars209_176;
struct __anon_173;
union __args_mapped_step5_dpbr_noGVars208_174;
struct __anon_199;
union __args_mapped_step5_dpbr_noGVars218_200;
struct __anon_193;
union __args_mapped_step5_dpbr_noGVars216_194;
struct __anon_191;
union __args_mapped_step5_dpbr_noGVars215_192;
struct __anon_49 {
   double (*restrict* A_l)[27];
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
   long IT0;
   long IT1;
   long IT2;
};
union __args_mapped_step1_noGVars92_50 {
   struct __anon_49 data;
   double padding[8];
};
struct __anon_51 {
   int (*restrict* colInds_l)[27];
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
   long IT0;
   long IT1;
   long IT2;
};
union __args_mapped_step1_noGVars95_52 {
   struct __anon_51 data;
   double padding[8];
};
struct __anon_53 {
   int* _p;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
};
union __args_mapped_step1_noGVars169_54 {
   struct __anon_53 data;
   double padding[5];
};
struct __anon_55 {
   double* _p;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
};
union __args_mapped_step1_noGVars97_56 {
   struct __anon_55 data;
   double padding[5];
};
struct __anon_57 {
   double* b_l;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
   long IT0;
};
union __args_mapped_step1_noGVars99_58 {
   struct __anon_57 data;
   double padding[6];
};
struct __anon_59 {
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
   long IT0;
   long IT1;
};
union __args_mapped_step1_noGVars102_60 {
   struct __anon_59 data;
   double padding[6];
};
struct __anon_61 {
   int* _p;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
};
union __args_mapped_step1_noGVars170_62 {
   struct __anon_61 data;
   double padding[5];
};
struct __anon_63 {
   int*restrict* nnzPerRow_l;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
   long IT0;
   long IT1;
};
union __args_mapped_step1_noGVars109_64 {
   struct __anon_63 data;
   double padding[7];
};
struct __anon_65 {
   double* Ap_l;
   double*restrict* r_l;
   double* p_l;
   double* normr;
   long IT0;
};
union __args_mapped_step1_noGVars112_66 {
   struct __anon_65 data;
   double padding[5];
};
struct __anon_67 {
   int* _p;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
};
union __args_mapped_step1_noGVars117_68 {
   struct __anon_67 data;
   double padding[5];
};
struct __anon_69 {
   double* x;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
   long IT0;
};
union __args_mapped_step1_noGVars119_70 {
   struct __anon_69 data;
   double padding[6];
};
struct __anon_71 {
   double* p_l;
   double*restrict* r_l;
   double* Ap_l;
   double* normr;
   long IT0;
};
union __args_mapped_step1_noGVars122_72 {
   struct __anon_71 data;
   double padding[5];
};
struct __anon_73 {
   int* _p;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
};
union __args_mapped_step1_noGVars172_74 {
   struct __anon_73 data;
   double padding[5];
};
struct __anon_75 {
   int const* nPerLvl_l;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
};
union __args_mapped_step1_noGVars173_76 {
   struct __anon_75 data;
   double padding[5];
};
struct __anon_77 {
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
};
union __args_mapped_step1_noGVars124_78 {
   struct __anon_77 data;
   double padding[4];
};
struct __anon_79 {
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
   long IT0;
};
union __args_mapped_step1_noGVars126_80 {
   struct __anon_79 data;
   double padding[5];
};
struct __anon_81 {
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
   long IT0;
};
union __args_mapped_step1_noGVars129_82 {
   struct __anon_81 data;
   double padding[5];
};
struct __anon_83 {
   double*restrict* r_l;
   double* normr;
   long IT0;
};
union __args_mapped_step1_noGVars132_84 {
   struct __anon_83 data;
   double padding[3];
};
struct __anon_85 {
   double* normr;
};
union __args_mapped_step1_noGVars134_86 {
   struct __anon_85 data;
   double padding[1];
};
struct __anon_91 {
   double* p_l;
   long IT0;
};
union __args_mapped_step1_noGVars136_92 {
   struct __anon_91 data;
   double padding[2];
};
struct __anon_93 {
   long IT0;
};
union __args_mapped_step1_noGVars139_94 {
   struct __anon_93 data;
   double padding[1];
};
struct __anon_97 {
   double* Ap_l;
   long IT0;
};
union __args_mapped_step1_noGVars146_98 {
   struct __anon_97 data;
   double padding[2];
};
struct __anon_99 {
   long IT0;
   long IT1;
};
union __args_mapped_step1_noGVars149_100 {
   struct __anon_99 data;
   double padding[2];
};
struct __anon_103 {
   double*restrict* r_l;
   long IT0;
   long IT1;
};
union __args_mapped_step1_noGVars156_104 {
   struct __anon_103 data;
   double padding[3];
};
struct __anon_105 {
   long IT0;
};
union __args_mapped_step1_noGVars159_106 {
   struct __anon_105 data;
   double padding[1];
};
struct __anon_111 {
   double* normr;
};
union __args_mapped_step1_noGVars161_112 {
   struct __anon_111 data;
   double padding[1];
};
struct __anon_113 {
   long IT0;
   long IT1;
   long IT2;
};
union __args_mapped_step1_noGVars163_114 {
   struct __anon_113 data;
   double padding[3];
};
struct __anon_115 {
   long IT0;
   long IT1;
   long IT2;
};
union __args_mapped_step1_noGVars166_116 {
   struct __anon_115 data;
   double padding[3];
};
struct __anon_117 {
   double (*restrict* A_l)[27];
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr;
   int (*restrict* colInds_l)[27];
   double* b_l;
   int*restrict* nnzPerRow_l;
   double* x;
   int const* nPerLvl_l;
};
union __args_mapped_step1_noGVars_main_118 {
   struct __anon_117 data;
   double padding[10];
};
struct __anon_119 {
   double (*restrict* A_l)[27];
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
   long IT0;
   long IT1;
   long IT2;
};
union __args_mapped_step5_dpbr_noGVars124_120 {
   struct __anon_119 data;
   double padding[10];
};
struct __anon_121 {
   int (*restrict* colInds_l)[27];
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
   long IT0;
   long IT1;
   long IT2;
};
union __args_mapped_step5_dpbr_noGVars127_122 {
   struct __anon_121 data;
   double padding[10];
};
struct __anon_123 {
   int* _p;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars195_124 {
   struct __anon_123 data;
   double padding[7];
};
struct __anon_125 {
   double* _p;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars129_126 {
   struct __anon_125 data;
   double padding[7];
};
struct __anon_127 {
   double*restrict* r_l;
   double* Ap_l;
   double* x;
   double* normr;
   long IT0;
   long IT1;
};
union __args_mapped_step5_dpbr_noGVars131_128 {
   struct __anon_127 data;
   double padding[6];
};
struct __anon_129 {
   int* _p;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars197_130 {
   struct __anon_129 data;
   double padding[7];
};
struct __anon_131 {
   double* _p;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars133_132 {
   struct __anon_131 data;
   double padding[7];
};
struct __anon_133 {
   double* x;
   double* Ap_l;
   double*restrict* r_l;
   double* normr;
   long IT0;
};
union __args_mapped_step5_dpbr_noGVars135_134 {
   struct __anon_133 data;
   double padding[5];
};
struct __anon_135 {
   int* _p;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars199_136 {
   struct __anon_135 data;
   double padding[7];
};
struct __anon_137 {
   double* _p;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars200_138 {
   struct __anon_137 data;
   double padding[7];
};
struct __anon_139 {
   double* rtz;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars201_140 {
   struct __anon_139 data;
   double padding[7];
};
struct __anon_141 {
   double* alpha;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars202_142 {
   struct __anon_141 data;
   double padding[6];
};
struct __anon_143 {
   double* pAp;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
};
union __args_mapped_step5_dpbr_noGVars137_144 {
   struct __anon_143 data;
   double padding[6];
};
struct __anon_145 {
   double* p_l;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
   long IT0;
};
union __args_mapped_step5_dpbr_noGVars139_146 {
   struct __anon_145 data;
   double padding[8];
};
struct __anon_147 {
   int* _p;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars204_148 {
   struct __anon_147 data;
   double padding[7];
};
struct __anon_149 {
   double* _p;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars205_150 {
   struct __anon_149 data;
   double padding[7];
};
struct __anon_151 {
   int*restrict* nnzPerRow_l;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
   long IT0;
   long IT1;
};
union __args_mapped_step5_dpbr_noGVars143_152 {
   struct __anon_151 data;
   double padding[9];
};
struct __anon_153 {
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
   long IT0;
};
union __args_mapped_step5_dpbr_noGVars146_154 {
   struct __anon_153 data;
   double padding[7];
};
struct __anon_155 {
   int* _p;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars206_156 {
   struct __anon_155 data;
   double padding[7];
};
struct __anon_157 {
   int const* nPerLvl_l;
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars207_158 {
   struct __anon_157 data;
   double padding[7];
};
struct __anon_159 {
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars151_160 {
   struct __anon_159 data;
   double padding[6];
};
struct __anon_161 {
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
   long IT0;
};
union __args_mapped_step5_dpbr_noGVars153_162 {
   struct __anon_161 data;
   double padding[7];
};
struct __anon_163 {
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
   long IT0;
};
union __args_mapped_step5_dpbr_noGVars156_164 {
   struct __anon_163 data;
   double padding[7];
};
struct __anon_165 {
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars158_166 {
   struct __anon_165 data;
   double padding[6];
};
struct __anon_167 {
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   long IT0;
};
union __args_mapped_step5_dpbr_noGVars160_168 {
   struct __anon_167 data;
   double padding[5];
};
struct __anon_169 {
   double*restrict* r_l;
   double* normr;
   long IT0;
};
union __args_mapped_step5_dpbr_noGVars163_170 {
   struct __anon_169 data;
   double padding[3];
};
struct __anon_171 {
   double* normr;
};
union __args_mapped_step5_dpbr_noGVars165_172 {
   struct __anon_171 data;
   double padding[1];
};
struct __anon_177 {
   double* Ap_l;
   long IT0;
};
union __args_mapped_step5_dpbr_noGVars170_178 {
   struct __anon_177 data;
   double padding[2];
};
struct __anon_179 {
   long IT0;
   long IT1;
};
union __args_mapped_step5_dpbr_noGVars173_180 {
   struct __anon_179 data;
   double padding[2];
};
struct __anon_185 {
   long IT0;
};
union __args_mapped_step5_dpbr_noGVars177_186 {
   struct __anon_185 data;
   double padding[1];
};
struct __anon_187 {
   double* pAp;
};
union __args_mapped_step5_dpbr_noGVars213_188 {
   struct __anon_187 data;
   double padding[1];
};
struct __anon_189 {
   double* alpha;
};
union __args_mapped_step5_dpbr_noGVars214_190 {
   struct __anon_189 data;
   double padding[1];
};
struct __anon_197 {
   double* x;
   long IT0;
};
union __args_mapped_step5_dpbr_noGVars181_198 {
   struct __anon_197 data;
   double padding[2];
};
struct __anon_203 {
   double*restrict* r_l;
   long IT0;
   long IT1;
};
union __args_mapped_step5_dpbr_noGVars185_204 {
   struct __anon_203 data;
   double padding[3];
};
struct __anon_209 {
   double* normr;
};
union __args_mapped_step5_dpbr_noGVars187_210 {
   struct __anon_209 data;
   double padding[1];
};
struct __anon_211 {
   long IT0;
   long IT1;
   long IT2;
};
union __args_mapped_step5_dpbr_noGVars189_212 {
   struct __anon_211 data;
   double padding[3];
};
struct __anon_213 {
   long IT0;
   long IT1;
   long IT2;
};
union __args_mapped_step5_dpbr_noGVars192_214 {
   struct __anon_213 data;
   double padding[3];
};
struct __anon_215 {
   double (*restrict* A_l)[27];
   double* Ap_l;
   double*restrict* r_l;
   double* x;
   double* normr;
   double* alpha;
   double* pAp;
   int (*restrict* colInds_l)[27];
   double* rtz;
   double* p_l;
   int*restrict* nnzPerRow_l;
   int const* nPerLvl_l;
};
union __args_mapped_step5_dpbr_noGVars_main_216 {
   struct __anon_215 data;
   double padding[12];
};
struct __anon_207 {
};
union __args_mapped_step5_dpbr_noGVars221_208 {
   struct __anon_207 data;
   double padding[1];
};
struct __anon_205 {
};
union __args_mapped_step5_dpbr_noGVars220_206 {
   struct __anon_205 data;
   double padding[1];
};
struct __anon_109 {
};
union __args_mapped_step1_noGVars179_110 {
   struct __anon_109 data;
   double padding[1];
};
struct __anon_107 {
};
union __args_mapped_step1_noGVars178_108 {
   struct __anon_107 data;
   double padding[1];
};
struct __anon_101 {
};
union __args_mapped_step1_noGVars151_102 {
   struct __anon_101 data;
   double padding[1];
};
struct __anon_95 {
};
union __args_mapped_step1_noGVars141_96 {
   struct __anon_95 data;
   double padding[1];
};
struct __anon_89 {
};
union __args_mapped_step1_noGVars175_90 {
   struct __anon_89 data;
   double padding[1];
};
struct __anon_87 {
};
union __args_mapped_step1_noGVars174_88 {
   struct __anon_87 data;
   double padding[1];
};
struct __anon_181 {
};
union __args_mapped_step5_dpbr_noGVars211_182 {
   struct __anon_181 data;
   double padding[1];
};
struct __anon_201 {
};
union __args_mapped_step5_dpbr_noGVars219_202 {
   struct __anon_201 data;
   double padding[1];
};
struct __anon_195 {
};
union __args_mapped_step5_dpbr_noGVars217_196 {
   struct __anon_195 data;
   double padding[1];
};
struct __anon_183 {
};
union __args_mapped_step5_dpbr_noGVars212_184 {
   struct __anon_183 data;
   double padding[1];
};
struct __anon_175 {
};
union __args_mapped_step5_dpbr_noGVars209_176 {
   struct __anon_175 data;
   double padding[1];
};
struct __anon_173 {
};
union __args_mapped_step5_dpbr_noGVars208_174 {
   struct __anon_173 data;
   double padding[1];
};
struct __anon_199 {
};
union __args_mapped_step5_dpbr_noGVars218_200 {
   struct __anon_199 data;
   double padding[1];
};
struct __anon_193 {
};
union __args_mapped_step5_dpbr_noGVars216_194 {
   struct __anon_193 data;
   double padding[1];
};
struct __anon_191 {
};
union __args_mapped_step5_dpbr_noGVars215_192 {
   struct __anon_191 data;
   double padding[1];
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
static ocrGuid_t mapped_step1_noGVars92(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void* rocrArgs(ocrEdtDep_t*);
void mapped_step1_noGVars92_dtenum_1(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars95(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars95_dtenum_2(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars169(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars169_dtenum_3(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars97(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars97_dtenum_4(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars99(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars99_dtenum_5(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars102(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars102_dtenum_6(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars170(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars170_dtenum_7(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars109(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars109_dtenum_8(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars112(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars112_dtenum_9(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars117(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars117_dtenum_10(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars119(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars119_dtenum_11(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars122(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars122_dtenum_12(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars172(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars172_dtenum_13(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars173(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars173_dtenum_14(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars124(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars124_dtenum_15(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars126(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars126_dtenum_16(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars129(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars129_dtenum_17(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars132(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars132_dtenum_18(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars134(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars134_dtenum_19(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars174(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars174_dtenum_20(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars175(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars175_dtenum_21(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars136(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars136_dtenum_22(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars139(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars139_dtenum_23(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars141(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars141_dtenum_24(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars146(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars146_dtenum_25(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars149(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars149_dtenum_26(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars151(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars151_dtenum_27(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars156(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars156_dtenum_28(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars159(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars159_dtenum_29(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars178(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars178_dtenum_30(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars179(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars179_dtenum_31(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars161(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars161_dtenum_32(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars163(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars163_dtenum_33(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step1_noGVars166(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step1_noGVars166_dtenum_34(void* rocrEDTCtx, long* args);
ocrGuid_t rocrAlloc(void**, unsigned long);
unsigned long mapped_step1_noGVars166_count_34(long* args);
unsigned long mapped_step1_noGVars129_count_17(long* args);
unsigned long mapped_step1_noGVars163_count_33(long* args);
unsigned long mapped_step1_noGVars159_count_29(long* args);
unsigned long mapped_step1_noGVars156_count_28(long* args);
unsigned long mapped_step1_noGVars151_count_27(long* args);
unsigned long mapped_step1_noGVars149_count_26(long* args);
unsigned long mapped_step1_noGVars146_count_25(long* args);
unsigned long mapped_step1_noGVars141_count_24(long* args);
unsigned long mapped_step1_noGVars136_count_22(long* args);
unsigned long mapped_step1_noGVars132_count_18(long* args);
unsigned long mapped_step1_noGVars179_count_31(long* args);
unsigned long mapped_step1_noGVars178_count_30(long* args);
unsigned long mapped_step1_noGVars134_count_19(long* args);
unsigned long mapped_step1_noGVars161_count_32(long* args);
unsigned long mapped_step1_noGVars124_count_15(long* args);
unsigned long mapped_step1_noGVars175_count_21(long* args);
unsigned long mapped_step1_noGVars174_count_20(long* args);
unsigned long mapped_step1_noGVars126_count_16(long* args);
unsigned long mapped_step1_noGVars139_count_23(long* args);
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
static ocrGuid_t mapped_step5_dpbr_noGVars124(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars124_dtenum_1(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars127(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars127_dtenum_2(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars195(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars195_dtenum_3(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars129(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars129_dtenum_4(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars131(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars131_dtenum_5(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars197(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars197_dtenum_6(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars133(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars133_dtenum_7(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars135(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars135_dtenum_8(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars199(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars199_dtenum_9(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars200(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars200_dtenum_10(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars201(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars201_dtenum_11(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars202(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars202_dtenum_12(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars137(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars137_dtenum_13(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars139(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars139_dtenum_14(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars204(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars204_dtenum_15(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars205(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars205_dtenum_16(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars143(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars143_dtenum_17(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars146(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars146_dtenum_18(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars206(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars206_dtenum_19(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars207(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars207_dtenum_20(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars151(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars151_dtenum_21(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars153(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars153_dtenum_22(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars156(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars156_dtenum_23(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars158(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars158_dtenum_24(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars160(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars160_dtenum_25(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars163(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars163_dtenum_26(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars165(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars165_dtenum_27(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars208(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars208_dtenum_28(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars209(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars209_dtenum_29(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars170(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars170_dtenum_30(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars173(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars173_dtenum_31(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars211(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars211_dtenum_32(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars212(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars212_dtenum_33(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars177(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars177_dtenum_34(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars213(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars213_dtenum_35(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars214(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars214_dtenum_36(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars215(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars215_dtenum_37(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars216(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars216_dtenum_38(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars217(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars217_dtenum_39(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars181(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars181_dtenum_40(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars218(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars218_dtenum_41(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars219(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars219_dtenum_42(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars185(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars185_dtenum_43(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars220(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars220_dtenum_44(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars221(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars221_dtenum_45(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars187(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars187_dtenum_46(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars189(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars189_dtenum_47(void* rocrEDTCtx, long* args);
static ocrGuid_t mapped_step5_dpbr_noGVars192(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv);
void mapped_step5_dpbr_noGVars192_dtenum_48(void* rocrEDTCtx, long* args);
unsigned long mapped_step5_dpbr_noGVars192_count_48(long* args);
unsigned long mapped_step5_dpbr_noGVars153_count_22(long* args);
unsigned long mapped_step5_dpbr_noGVars189_count_47(long* args);
unsigned long mapped_step5_dpbr_noGVars177_count_34(long* args);
unsigned long mapped_step5_dpbr_noGVars173_count_31(long* args);
unsigned long mapped_step5_dpbr_noGVars170_count_30(long* args);
unsigned long mapped_step5_dpbr_noGVars209_count_29(long* args);
unsigned long mapped_step5_dpbr_noGVars160_count_25(long* args);
unsigned long mapped_step5_dpbr_noGVars156_count_23(long* args);
unsigned long mapped_step5_dpbr_noGVars185_count_43(long* args);
unsigned long mapped_step5_dpbr_noGVars219_count_42(long* args);
unsigned long mapped_step5_dpbr_noGVars218_count_41(long* args);
unsigned long mapped_step5_dpbr_noGVars181_count_40(long* args);
unsigned long mapped_step5_dpbr_noGVars217_count_39(long* args);
unsigned long mapped_step5_dpbr_noGVars216_count_38(long* args);
unsigned long mapped_step5_dpbr_noGVars163_count_26(long* args);
unsigned long mapped_step5_dpbr_noGVars221_count_45(long* args);
unsigned long mapped_step5_dpbr_noGVars220_count_44(long* args);
unsigned long mapped_step5_dpbr_noGVars165_count_27(long* args);
unsigned long mapped_step5_dpbr_noGVars187_count_46(long* args);
unsigned long mapped_step5_dpbr_noGVars212_count_33(long* args);
unsigned long mapped_step5_dpbr_noGVars211_count_32(long* args);
unsigned long mapped_step5_dpbr_noGVars158_count_24(long* args);
unsigned long mapped_step5_dpbr_noGVars215_count_37(long* args);
unsigned long mapped_step5_dpbr_noGVars214_count_36(long* args);
unsigned long mapped_step5_dpbr_noGVars213_count_35(long* args);
unsigned long mapped_step5_dpbr_noGVars151_count_21(long* args);
unsigned long mapped_step5_dpbr_noGVars208_count_28(long* args);
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
    #define __divceils_64(x,y) \
        (int)ceilf(((float)(x)/(float)(y)))
#else
#ifdef RSTREAM_FLOATING_POINT_CEIL_FLOOR
#include <math.h>
static RSTREAM_INLINE long __divceils_64(long x, long y) {
   return (long) ceil((double) x / (double)y);
}
#else
static RSTREAM_INLINE long __divceils_64(long x, long y) {
   long q = x / y;
   return (((x > 0 && y > 0) || (x < 0 && y < 0)) &&
          (x % y) != 0) ? q+1 : q;
}
#endif
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
    #define __divfloors_64(x,y) \
        (int)floorf(((float)(x)/(float)(y)))
#else
#ifdef RSTREAM_FLOATING_POINT_CEIL_FLOOR
static RSTREAM_INLINE long __divfloors_64(long x, long y) {
   return (long) floor((double) x / (double)y);
}
#else
static RSTREAM_INLINE long __divfloors_64(long x, long y) {
   long q = x / y;
   return (((x > 0 && y < 0) || (x < 0 && y > 0)) && \
          (x % y) != 0) ? q-1 : q;
}
#endif
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
   union __args_mapped_step1_noGVars_main_118* allArgs;
   ocrGuid_t _t1;
   union __args_mapped_step1_noGVars_main_118* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, (int)0l);
   _t3 = rocrInit(35u, 15u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, mapped_step1_noGVars_main, 0u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars92, 1u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars95, 2u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars169, 3u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars97, 4u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars99, 5u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars102, 6u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars170, 7u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars109, 8u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars112, 9u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars117, 10u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars119, 11u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars122, 12u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars172, 13u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars173, 14u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars124, 15u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars126, 16u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars129, 17u, (unsigned long)138l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars132, 18u, (unsigned long)9l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars134, 19u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars174, 20u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars175, 21u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars136, 22u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars139, 23u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars141, 24u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars146, 25u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars149, 26u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars151, 27u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars156, 28u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars159, 29u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars178, 30u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars179, 31u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars161, 32u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars163, 33u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars166, 34u, (unsigned long)18l, 0u);
   rocrDeclareDBType(_t3, 0u, 0u);
   rocrDeclareDBType(_t3, 1u, 0u);
   rocrDeclareDBType(_t3, 2u, 1u, 1124864ul, (unsigned long)(__divceils_64(
      1124864l, (long)1124864ul)));
   rocrDeclareDBType(_t3, 3u, 1u, 65536ul, (unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 4u, 0u);
   rocrDeclareDBType(_t3, 5u, 1u, 65536ul, (unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 6u, 2u, 1ul, 65536ul, (unsigned long)(1l >> 0l), (
      unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 7u, 0u);
   rocrDeclareDBType(_t3, 8u, 2u, 1ul, 65536ul, (unsigned long)(1l >> 0l), (
      unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 9u, 1u, 65536ul, (unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 10u, 0u);
   rocrDeclareDBType(_t3, 11u, 0u);
   rocrDeclareDBType(_t3, 12u, 0u);
   rocrDeclareDBType(_t3, 13u, 3u, 1ul, 65536ul, 27ul, (unsigned long)(1l >> 0l
      ), (unsigned long)(1190399l >> 16l), (unsigned long)(__divceils_64(27l, (
      long)27ul)));
   rocrDeclareDBType(_t3, 14u, 3u, 1ul, 65536ul, 27ul, (unsigned long)(1l >> 0l
      ), (unsigned long)(1190399l >> 16l), (unsigned long)(__divceils_64(27l, (
      long)27ul)));
   _t1 = rocrAlloc((void**)&allArgs, 80ul);
   _t2 = allArgs;
   _t2->data.A_l = A;
   _t2->data.r_l = r;
   _t2->data.Ap_l = Ap;
   _t2->data.p_l = p;
   _t2->data.normr = normr_2;
   _t2->data.colInds_l = colInds;
   _t2->data.b_l = b;
   _t2->data.nnzPerRow_l = nnzPerRow;
   _t2->data.x = x_11;
   _t2->data.nPerLvl_l = nPerLvl;
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
   union __args_mapped_step5_dpbr_noGVars_main_216* allArgs;
   ocrGuid_t _t1;
   union __args_mapped_step5_dpbr_noGVars_main_216* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, (int)0l);
   _t3 = rocrInit(49u, 21u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars_main, 0u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars124, 1u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars127, 2u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars195, 3u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars129, 4u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars131, 5u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars197, 6u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars133, 7u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars135, 8u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars199, 9u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars200, 10u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars201, 11u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars202, 12u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars137, 13u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars139, 14u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars204, 15u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars205, 16u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars143, 17u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars146, 18u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars206, 19u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars207, 20u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars151, 21u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars153, 22u, (unsigned long)138l,
       0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars156, 23u, (unsigned long)18l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars158, 24u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars160, 25u, (unsigned long)35l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars163, 26u, (unsigned long)9l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars165, 27u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars208, 28u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars209, 29u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars170, 30u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars173, 31u, (unsigned long)18l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars211, 32u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars212, 33u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars177, 34u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars213, 35u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars214, 36u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars215, 37u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars216, 38u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars217, 39u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars181, 40u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars218, 41u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars219, 42u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars185, 43u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars220, 44u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars221, 45u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars187, 46u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars189, 47u, (unsigned long)18l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars192, 48u, (unsigned long)18l,
      0u);
   rocrDeclareDBType(_t3, 0u, 0u);
   rocrDeclareDBType(_t3, 1u, 0u);
   rocrDeclareDBType(_t3, 2u, 1u, 65536ul, (unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 3u, 2u, 1ul, 65536ul, (unsigned long)(1l >> 0l), (
      unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 4u, 0u);
   rocrDeclareDBType(_t3, 5u, 0u);
   rocrDeclareDBType(_t3, 6u, 1u, 1124864ul, (unsigned long)(__divceils_64(
      1124864l, (long)1124864ul)));
   rocrDeclareDBType(_t3, 7u, 0u);
   rocrDeclareDBType(_t3, 8u, 0u);
   rocrDeclareDBType(_t3, 9u, 0u);
   rocrDeclareDBType(_t3, 10u, 0u);
   rocrDeclareDBType(_t3, 11u, 0u);
   rocrDeclareDBType(_t3, 12u, 1u, 65536ul, (unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 13u, 0u);
   rocrDeclareDBType(_t3, 14u, 0u);
   rocrDeclareDBType(_t3, 15u, 2u, 1ul, 65536ul, (unsigned long)(1l >> 0l), (
      unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 16u, 0u);
   rocrDeclareDBType(_t3, 17u, 0u);
   rocrDeclareDBType(_t3, 18u, 0u);
   rocrDeclareDBType(_t3, 19u, 3u, 1ul, 65536ul, 27ul, (unsigned long)(1l >> 0l
      ), (unsigned long)(1190399l >> 16l), (unsigned long)(__divceils_64(27l, (
      long)27ul)));
   rocrDeclareDBType(_t3, 20u, 3u, 1ul, 65536ul, 27ul, (unsigned long)(1l >> 0l
      ), (unsigned long)(1190399l >> 16l), (unsigned long)(__divceils_64(27l, (
      long)27ul)));
   _t1 = rocrAlloc((void**)&allArgs, 96ul);
   _t2 = allArgs;
   _t2->data.A_l = A;
   _t2->data.Ap_l = Ap;
   _t2->data.r_l = r;
   _t2->data.x = x_22;
   _t2->data.normr = normr_11;
   _t2->data.alpha = alpha;
   _t2->data.pAp = pAp;
   _t2->data.colInds_l = colInds;
   _t2->data.rtz = rtz;
   _t2->data.p_l = p;
   _t2->data.nnzPerRow_l = nnzPerRow;
   _t2->data.nPerLvl_l = nPerLvl;
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
   union __args_mapped_step1_noGVars_main_118* allArgs;
   ocrGuid_t _t1;
   union __args_mapped_step1_noGVars_main_118* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, (int)0l);
   _t3 = rocrInit(35u, 15u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, mapped_step1_noGVars_main, 0u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars92, 1u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars95, 2u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars169, 3u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars97, 4u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars99, 5u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars102, 6u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars170, 7u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars109, 8u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars112, 9u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars117, 10u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars119, 11u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars122, 12u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars172, 13u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars173, 14u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars124, 15u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars126, 16u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars129, 17u, (unsigned long)138l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars132, 18u, (unsigned long)9l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars134, 19u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars174, 20u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars175, 21u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars136, 22u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars139, 23u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars141, 24u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars146, 25u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars149, 26u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars151, 27u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars156, 28u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars159, 29u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars178, 30u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars179, 31u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars161, 32u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars163, 33u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars166, 34u, (unsigned long)18l, 0u);
   rocrDeclareDBType(_t3, 0u, 0u);
   rocrDeclareDBType(_t3, 1u, 0u);
   rocrDeclareDBType(_t3, 2u, 1u, 1124864ul, (unsigned long)(__divceils_64(
      1124864l, (long)1124864ul)));
   rocrDeclareDBType(_t3, 3u, 1u, 65536ul, (unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 4u, 0u);
   rocrDeclareDBType(_t3, 5u, 1u, 65536ul, (unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 6u, 2u, 1ul, 65536ul, (unsigned long)(1l >> 0l), (
      unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 7u, 0u);
   rocrDeclareDBType(_t3, 8u, 2u, 1ul, 65536ul, (unsigned long)(1l >> 0l), (
      unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 9u, 1u, 65536ul, (unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 10u, 0u);
   rocrDeclareDBType(_t3, 11u, 0u);
   rocrDeclareDBType(_t3, 12u, 0u);
   rocrDeclareDBType(_t3, 13u, 3u, 1ul, 65536ul, 27ul, (unsigned long)(1l >> 0l
      ), (unsigned long)(1190399l >> 16l), (unsigned long)(__divceils_64(27l, (
      long)27ul)));
   rocrDeclareDBType(_t3, 14u, 3u, 1ul, 65536ul, 27ul, (unsigned long)(1l >> 0l
      ), (unsigned long)(1190399l >> 16l), (unsigned long)(__divceils_64(27l, (
      long)27ul)));
   _t1 = rocrAlloc((void**)&allArgs, 80ul);
   _t2 = allArgs;
   _t2->data.A_l = A_l;
   _t2->data.r_l = r_l;
   _t2->data.Ap_l = Ap_l;
   _t2->data.p_l = p_l;
   _t2->data.normr = normr_12;
   _t2->data.colInds_l = colInds_l;
   _t2->data.b_l = b_l;
   _t2->data.nnzPerRow_l = nnzPerRow_l;
   _t2->data.x = x_23;
   _t2->data.nPerLvl_l = nPerLvl_l;
   rocrExecute(_t3, mapped_step1_noGVars_main, _t1, &outEvt);
   rocrExit(_t3);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

static ocrGuid_t mapped_step1_noGVars92(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars92_50 mapped_step1_noGVars92_args;
   double* A_l_db_0;
   union __args_mapped_step1_noGVars166_116* allArgs;
   union __args_mapped_step1_noGVars129_82* allArgs_1;
   void* _t1;
   double (*restrict* A_l)[27];
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_13;
   long IT0;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars92_args = *(union __args_mapped_step1_noGVars92_50*)
      rocrArgs(depv);
   IT1 = mapped_step1_noGVars92_args.data.IT1;
   IT0 = mapped_step1_noGVars92_args.data.IT0;
   normr_13 = mapped_step1_noGVars92_args.data.normr;
   p_l = mapped_step1_noGVars92_args.data.p_l;
   Ap_l = mapped_step1_noGVars92_args.data.Ap_l;
   r_l = mapped_step1_noGVars92_args.data.r_l;
   A_l = mapped_step1_noGVars92_args.data.A_l;
   if (mapped_step1_noGVars92_args.data.IT2 == 0l && IT0 == 0l) {
      long _t2;
      ocrGuid_t _t3;
      union __args_mapped_step1_noGVars166_116* _t4;
      long _t5;
      long i;
      long i_1;
      long i_2;
      _t2 = (__mins_64(-65536l * IT1 + 1124863l, 65535l));
      rocrFetchDB(_t1, (void**)&A_l_db_0, 4, 14ul, (unsigned long)0l, (
         unsigned long)IT1, (unsigned long)0l);
      for (i = 0l, i_1 = 0l; i <= _t2; i++, i_1 += 27) {
         long j;
         for (j = 0l; j <= 26l; j++) {
            A_l_db_0[1769472l * IT1 + j + i_1 + -27l * (__divfloors_64(j, 27l))
               + -1769472l * (65536l * IT1 + i >> 16l)] = A_l[0l][i + 65536l *
               IT1][j];
         }
      }
      _t3 = rocrAlloc((void**)&allArgs, 24ul);
      _t4 = allArgs;
      _t4->data.IT0 = 0l;
      _t4->data.IT1 = IT1;
      _t4->data.IT2 = 0l;
      _t5 = (__mins_64(137l, 8l * IT1 + 7l));
      rocrAutoDec(_t1, 34u, (unsigned long)IT1, _t3,
         mapped_step1_noGVars166_count_34, mapped_step1_noGVars166_dtenum_34, (
         void (*)(void*, long*))((void*)0l), 3u, (long)0l, (long)IT1, (long)0l)
         ;
      for (i_2 = 8l * IT1; i_2 <= _t5; i_2++) {
         ocrGuid_t _t6;
         union __args_mapped_step1_noGVars129_82* _t7;
         _t6 = rocrAlloc((void**)&allArgs_1, 40ul);
         _t7 = allArgs_1;
         _t7->data.r_l = r_l;
         _t7->data.Ap_l = Ap_l;
         _t7->data.p_l = p_l;
         _t7->data.normr = normr_13;
         _t7->data.IT0 = i_2;
         rocrAutoDec(_t1, 17u, (unsigned long)i_2, _t6,
            mapped_step1_noGVars129_count_17, mapped_step1_noGVars129_dtenum_17
            , (void (*)(void*, long*))((void*)0l), 1u, (long)i_2);
      }
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars92_dtenum_1(void* rocrEDTCtx, long* args)
{
   long _t1;
   long _t2;
   _t2 = (long)args[1l];
   _t1 = (long)args[0l];
   if ((long)args[2l] == 0l && _t1 == 0l) {
      if (_t2 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 14u, 14155776ul, 2l, 1u, (void (*)(void*, long*
            ))((void*)0l), (unsigned long)0l, (unsigned long)_t2, (
            unsigned long)0l);
      }
      if (_t2 == 17l) {
         rocrEnumDB(rocrEDTCtx, 14u, 14155776ul, 1l, 1u, (void (*)(void*, long*
            ))((void*)0l), (unsigned long)0l, (unsigned long)17l, (
            unsigned long)0l);
      }
   }
}

static ocrGuid_t mapped_step1_noGVars95(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars95_52 mapped_step1_noGVars95_args;
   int* colInds_l_db_0;
   union __args_mapped_step1_noGVars163_114* allArgs;
   union __args_mapped_step1_noGVars129_82* allArgs_1;
   void* _t1;
   int (*restrict* colInds_l)[27];
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_14;
   long IT0;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars95_args = *(union __args_mapped_step1_noGVars95_52*)
      rocrArgs(depv);
   IT1 = mapped_step1_noGVars95_args.data.IT1;
   IT0 = mapped_step1_noGVars95_args.data.IT0;
   normr_14 = mapped_step1_noGVars95_args.data.normr;
   p_l = mapped_step1_noGVars95_args.data.p_l;
   Ap_l = mapped_step1_noGVars95_args.data.Ap_l;
   r_l = mapped_step1_noGVars95_args.data.r_l;
   colInds_l = mapped_step1_noGVars95_args.data.colInds_l;
   if (mapped_step1_noGVars95_args.data.IT2 == 0l && IT0 == 0l) {
      long _t2;
      ocrGuid_t _t3;
      union __args_mapped_step1_noGVars163_114* _t4;
      long _t5;
      long i;
      long i_1;
      long i_2;
      _t2 = (__mins_64(-65536l * IT1 + 1124863l, 65535l));
      rocrFetchDB(_t1, (void**)&colInds_l_db_0, 4, 13ul, (unsigned long)0l, (
         unsigned long)IT1, (unsigned long)0l);
      for (i = 0l, i_1 = 0l; i <= _t2; i++, i_1 += 27) {
         long j;
         for (j = 0l; j <= 26l; j++) {
            colInds_l_db_0[1769472l * IT1 + j + i_1 + -27l * (__divfloors_64(j,
                27l)) + -1769472l * (65536l * IT1 + i >> 16l)] = colInds_l[0l][
               i + 65536l * IT1][j];
         }
      }
      _t3 = rocrAlloc((void**)&allArgs, 24ul);
      _t4 = allArgs;
      _t4->data.IT0 = 0l;
      _t4->data.IT1 = IT1;
      _t4->data.IT2 = 0l;
      _t5 = (__mins_64(137l, 8l * IT1 + 7l));
      rocrAutoDec(_t1, 33u, (unsigned long)IT1, _t3,
         mapped_step1_noGVars163_count_33, mapped_step1_noGVars163_dtenum_33, (
         void (*)(void*, long*))((void*)0l), 3u, (long)0l, (long)IT1, (long)0l)
         ;
      for (i_2 = 8l * IT1; i_2 <= _t5; i_2++) {
         ocrGuid_t _t6;
         union __args_mapped_step1_noGVars129_82* _t7;
         _t6 = rocrAlloc((void**)&allArgs_1, 40ul);
         _t7 = allArgs_1;
         _t7->data.r_l = r_l;
         _t7->data.Ap_l = Ap_l;
         _t7->data.p_l = p_l;
         _t7->data.normr = normr_14;
         _t7->data.IT0 = i_2;
         rocrAutoDec(_t1, 17u, (unsigned long)i_2, _t6,
            mapped_step1_noGVars129_count_17, mapped_step1_noGVars129_dtenum_17
            , (void (*)(void*, long*))((void*)0l), 1u, (long)i_2);
      }
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars95_dtenum_2(void* rocrEDTCtx, long* args)
{
   long _t1;
   long _t2;
   _t2 = (long)args[1l];
   _t1 = (long)args[0l];
   if ((long)args[2l] == 0l && _t1 == 0l) {
      if (_t2 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 13u, 7077888ul, 2l, 1u, (void (*)(void*, long*)
            )((void*)0l), (unsigned long)0l, (unsigned long)_t2, (unsigned long
            )0l);
      }
      if (_t2 == 17l) {
         rocrEnumDB(rocrEDTCtx, 13u, 7077888ul, 1l, 1u, (void (*)(void*, long*)
            )((void*)0l), (unsigned long)0l, (unsigned long)17l, (unsigned long
            )0l);
      }
   }
}

static ocrGuid_t mapped_step1_noGVars169(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars169_54 mapped_step1_noGVars169_args;
   int* _v4_db_0;
   union __args_mapped_step1_noGVars179_110* allArgs;
   union __args_mapped_step1_noGVars132_84* allArgs_1;
   union __args_mapped_step1_noGVars124_78* allArgs_2;
   void* _t1;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_15;
   ocrGuid_t _t2;
   union __args_mapped_step1_noGVars124_78* _t3;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars169_args = *(union __args_mapped_step1_noGVars169_54*)
      rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&_v4_db_0, 1, 11ul);
   *_v4_db_0 = *mapped_step1_noGVars169_args.data._p;
   rocrAutoDec(_t1, 31u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step1_noGVars179_count_31, mapped_step1_noGVars179_dtenum_31, (
      void (*)(void*, long*))((void*)0l), 0u);
   for (normr_15 = mapped_step1_noGVars169_args.data.normr,
        p_l = mapped_step1_noGVars169_args.data.p_l,
        Ap_l = mapped_step1_noGVars169_args.data.Ap_l,
        r_l = mapped_step1_noGVars169_args.data.r_l,
        i = 0l;
        i <= 8l; i++) {
      ocrGuid_t _t4;
      union __args_mapped_step1_noGVars132_84* _t5;
      _t4 = rocrAlloc((void**)&allArgs_1, 24ul);
      _t5 = allArgs_1;
      _t5->data.r_l = r_l;
      _t5->data.normr = normr_15;
      _t5->data.IT0 = i;
      rocrAutoDec(_t1, 18u, (unsigned long)i, _t4,
         mapped_step1_noGVars132_count_18, mapped_step1_noGVars132_dtenum_18, (
         void (*)(void*, long*))((void*)0l), 1u, (long)i);
   }
   _t2 = rocrAlloc((void**)&allArgs_2, 32ul);
   _t3 = allArgs_2;
   _t3->data.r_l = r_l;
   _t3->data.Ap_l = Ap_l;
   _t3->data.p_l = p_l;
   _t3->data.normr = normr_15;
   rocrAutoDec(_t1, 15u, (unsigned long)0l, _t2,
      mapped_step1_noGVars124_count_15, mapped_step1_noGVars124_dtenum_15, (
      void (*)(void*, long*))((void*)0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars169_dtenum_3(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 11u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step1_noGVars97(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars97_56 mapped_step1_noGVars97_args;
   double* local_result_db_0;
   union __args_mapped_step1_noGVars178_108* allArgs;
   union __args_mapped_step1_noGVars134_86* allArgs_1;
   union __args_mapped_step1_noGVars132_84* allArgs_2;
   union __args_mapped_step1_noGVars124_78* allArgs_3;
   void* _t1;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_16;
   ocrGuid_t _t2;
   ocrGuid_t _t3;
   union __args_mapped_step1_noGVars124_78* _t4;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars97_args = *(union __args_mapped_step1_noGVars97_56*)
      rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&local_result_db_0, 1, 10ul);
   *local_result_db_0 = *mapped_step1_noGVars97_args.data._p;
   rocrAutoDec(_t1, 30u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step1_noGVars178_count_30, mapped_step1_noGVars178_dtenum_30, (
      void (*)(void*, long*))((void*)0l), 0u);
   _t2 = rocrAlloc((void**)&allArgs_1, 8ul);
   normr_16 = mapped_step1_noGVars97_args.data.normr;
   allArgs_1->data.normr = normr_16;
   rocrAutoDec(_t1, 19u, (unsigned long)0l, _t2,
      mapped_step1_noGVars134_count_19, mapped_step1_noGVars134_dtenum_19, (
      void (*)(void*, long*))((void*)0l), 0u);
   for (p_l = mapped_step1_noGVars97_args.data.p_l,
        Ap_l = mapped_step1_noGVars97_args.data.Ap_l,
        r_l = mapped_step1_noGVars97_args.data.r_l,
        i = 0l;
        i <= 8l; i++) {
      ocrGuid_t _t5;
      union __args_mapped_step1_noGVars132_84* _t6;
      _t5 = rocrAlloc((void**)&allArgs_2, 24ul);
      _t6 = allArgs_2;
      _t6->data.r_l = r_l;
      _t6->data.normr = normr_16;
      _t6->data.IT0 = i;
      rocrAutoDec(_t1, 18u, (unsigned long)i, _t5,
         mapped_step1_noGVars132_count_18, mapped_step1_noGVars132_dtenum_18, (
         void (*)(void*, long*))((void*)0l), 1u, (long)i);
   }
   _t3 = rocrAlloc((void**)&allArgs_3, 32ul);
   _t4 = allArgs_3;
   _t4->data.r_l = r_l;
   _t4->data.Ap_l = Ap_l;
   _t4->data.p_l = p_l;
   _t4->data.normr = normr_16;
   rocrAutoDec(_t1, 15u, (unsigned long)0l, _t3,
      mapped_step1_noGVars124_count_15, mapped_step1_noGVars124_dtenum_15, (
      void (*)(void*, long*))((void*)0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars97_dtenum_4(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 10u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step1_noGVars99(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars99_58 mapped_step1_noGVars99_args;
   double* b_l_db_0;
   union __args_mapped_step1_noGVars159_106* allArgs;
   union __args_mapped_step1_noGVars129_82* allArgs_1;
   void* _t1;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_17;
   long IT0;
   long _t2;
   ocrGuid_t _t3;
   long _t4;
   double* b_l;
   long i;
   long i_1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars99_args = *(union __args_mapped_step1_noGVars99_58*)
      rocrArgs(depv);
   IT0 = mapped_step1_noGVars99_args.data.IT0;
   _t2 = (__mins_64(-65536l * IT0 + 1124863l, 65535l));
   rocrFetchDB(_t1, (void**)&b_l_db_0, 2, 9ul, (unsigned long)IT0);
   for (normr_17 = mapped_step1_noGVars99_args.data.normr,
        p_l = mapped_step1_noGVars99_args.data.p_l,
        Ap_l = mapped_step1_noGVars99_args.data.Ap_l,
        r_l = mapped_step1_noGVars99_args.data.r_l,
        b_l = mapped_step1_noGVars99_args.data.b_l,
        i = 0l;
        i <= _t2; i++) {
      b_l_db_0[65536l * IT0 + i + -65536l * (65536l * IT0 + i >> 16l)] = b_l[i +
         65536l * IT0];
   }
   _t3 = rocrAlloc((void**)&allArgs, 8ul);
   allArgs->data.IT0 = IT0;
   _t4 = (__mins_64(137l, 8l * IT0 + 7l));
   rocrAutoDec(_t1, 29u, (unsigned long)IT0, _t3,
      mapped_step1_noGVars159_count_29, mapped_step1_noGVars159_dtenum_29, (
      void (*)(void*, long*))((void*)0l), 1u, (long)IT0);
   for (i_1 = 8l * IT0; i_1 <= _t4; i_1++) {
      ocrGuid_t _t5;
      union __args_mapped_step1_noGVars129_82* _t6;
      _t5 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t6 = allArgs_1;
      _t6->data.r_l = r_l;
      _t6->data.Ap_l = Ap_l;
      _t6->data.p_l = p_l;
      _t6->data.normr = normr_17;
      _t6->data.IT0 = i_1;
      rocrAutoDec(_t1, 17u, (unsigned long)i_1, _t5,
         mapped_step1_noGVars129_count_17, mapped_step1_noGVars129_dtenum_17, (
         void (*)(void*, long*))((void*)0l), 1u, (long)i_1);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars99_dtenum_5(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[0l];
   if (_t1 <= 16l) {
      rocrEnumDB(rocrEDTCtx, 9u, 524288ul, 2l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)_t1);
   }
   if (_t1 == 17l) {
      rocrEnumDB(rocrEDTCtx, 9u, 524288ul, 1l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)17l);
   }
}

static ocrGuid_t mapped_step1_noGVars102(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars102_60 mapped_step1_noGVars102_args;
   double* r_l_db_0;
   union __args_mapped_step1_noGVars156_104* allArgs;
   union __args_mapped_step1_noGVars132_84* allArgs_1;
   union __args_mapped_step1_noGVars129_82* allArgs_2;
   void* _t1;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_18;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars102_args = *(union __args_mapped_step1_noGVars102_60*)
      rocrArgs(depv);
   IT1 = mapped_step1_noGVars102_args.data.IT1;
   normr_18 = mapped_step1_noGVars102_args.data.normr;
   p_l = mapped_step1_noGVars102_args.data.p_l;
   Ap_l = mapped_step1_noGVars102_args.data.Ap_l;
   r_l = mapped_step1_noGVars102_args.data.r_l;
   if (mapped_step1_noGVars102_args.data.IT0 == 0l) {
      long _t2;
      ocrGuid_t _t3;
      union __args_mapped_step1_noGVars156_104* _t4;
      long _t5;
      long i;
      long i_1;
      long _t6;
      long i_2;
      _t2 = (__mins_64(-65536l * IT1 + 1124863l, 65535l));
      rocrFetchDB(_t1, (void**)&r_l_db_0, 3, 8ul, (unsigned long)0l, (
         unsigned long)IT1);
      for (i = 0l; i <= _t2; i++) {
         r_l_db_0[65536l * IT1 + i + -65536l * (65536l * IT1 + i >> 16l)] = r_l
            [0l][i + 65536l * IT1];
      }
      _t3 = rocrAlloc((void**)&allArgs, 24ul);
      _t4 = allArgs;
      _t4->data.r_l = r_l;
      _t4->data.IT0 = 0l;
      _t4->data.IT1 = IT1;
      _t5 = (__mins_64(IT1 >> 1l, 8l));
      rocrAutoDec(_t1, 28u, (unsigned long)IT1, _t3,
         mapped_step1_noGVars156_count_28, mapped_step1_noGVars156_dtenum_28, (
         void (*)(void*, long*))((void*)0l), 2u, (long)0l, (long)IT1);
      for (i_1 = (__maxs_64(0l, IT1 + -1l + 1l >> 1l)); i_1 <= _t5; i_1++) {
         ocrGuid_t _t7;
         union __args_mapped_step1_noGVars132_84* _t8;
         _t7 = rocrAlloc((void**)&allArgs_1, 24ul);
         _t8 = allArgs_1;
         _t8->data.r_l = r_l;
         _t8->data.normr = normr_18;
         _t8->data.IT0 = i_1;
         rocrAutoDec(_t1, 18u, (unsigned long)i_1, _t7,
            mapped_step1_noGVars132_count_18, mapped_step1_noGVars132_dtenum_18
            , (void (*)(void*, long*))((void*)0l), 1u, (long)i_1);
      }
      for (_t6 = (__mins_64(137l, 8l * IT1 + 7l)), i_2 = 8l * IT1; i_2 <= _t6;
              i_2++) {
         ocrGuid_t _t9;
         union __args_mapped_step1_noGVars129_82* _t10;
         _t9 = rocrAlloc((void**)&allArgs_2, 40ul);
         _t10 = allArgs_2;
         _t10->data.r_l = r_l;
         _t10->data.Ap_l = Ap_l;
         _t10->data.p_l = p_l;
         _t10->data.normr = normr_18;
         _t10->data.IT0 = i_2;
         rocrAutoDec(_t1, 17u, (unsigned long)i_2, _t9,
            mapped_step1_noGVars129_count_17, mapped_step1_noGVars129_dtenum_17
            , (void (*)(void*, long*))((void*)0l), 1u, (long)i_2);
      }
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars102_dtenum_6(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[1l];
   if ((long)args[0l] == 0l) {
      if (_t1 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 8u, 524288ul, 2l, 1u, (void (*)(void*, long*))(
            (void*)0l), (unsigned long)0l, (unsigned long)_t1);
      }
      if (_t1 == 17l) {
         rocrEnumDB(rocrEDTCtx, 8u, 524288ul, 1l, 1u, (void (*)(void*, long*))(
            (void*)0l), (unsigned long)0l, (unsigned long)17l);
      }
   }
}

static ocrGuid_t mapped_step1_noGVars170(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars170_62 mapped_step1_noGVars170_args;
   int* _v3_db_0;
   union __args_mapped_step1_noGVars151_102* allArgs;
   union __args_mapped_step1_noGVars129_82* allArgs_1;
   union __args_mapped_step1_noGVars124_78* allArgs_2;
   void* _t1;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_19;
   ocrGuid_t _t2;
   union __args_mapped_step1_noGVars124_78* _t3;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars170_args = *(union __args_mapped_step1_noGVars170_62*)
      rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&_v3_db_0, 1, 7ul);
   *_v3_db_0 = *mapped_step1_noGVars170_args.data._p;
   rocrAutoDec(_t1, 27u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step1_noGVars151_count_27, mapped_step1_noGVars151_dtenum_27, (
      void (*)(void*, long*))((void*)0l), 0u);
   for (normr_19 = mapped_step1_noGVars170_args.data.normr,
        p_l = mapped_step1_noGVars170_args.data.p_l,
        Ap_l = mapped_step1_noGVars170_args.data.Ap_l,
        r_l = mapped_step1_noGVars170_args.data.r_l,
        i = 0l;
        i <= 137l; i++) {
      ocrGuid_t _t4;
      union __args_mapped_step1_noGVars129_82* _t5;
      _t4 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t5 = allArgs_1;
      _t5->data.r_l = r_l;
      _t5->data.Ap_l = Ap_l;
      _t5->data.p_l = p_l;
      _t5->data.normr = normr_19;
      _t5->data.IT0 = i;
      rocrAutoDec(_t1, 17u, (unsigned long)i, _t4,
         mapped_step1_noGVars129_count_17, mapped_step1_noGVars129_dtenum_17, (
         void (*)(void*, long*))((void*)0l), 1u, (long)i);
   }
   _t2 = rocrAlloc((void**)&allArgs_2, 32ul);
   _t3 = allArgs_2;
   _t3->data.r_l = r_l;
   _t3->data.Ap_l = Ap_l;
   _t3->data.p_l = p_l;
   _t3->data.normr = normr_19;
   rocrAutoDec(_t1, 15u, (unsigned long)0l, _t2,
      mapped_step1_noGVars124_count_15, mapped_step1_noGVars124_dtenum_15, (
      void (*)(void*, long*))((void*)0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars170_dtenum_7(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 7u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step1_noGVars109(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars109_64 mapped_step1_noGVars109_args;
   int* nnzPerRow_l_db_0;
   union __args_mapped_step1_noGVars149_100* allArgs;
   union __args_mapped_step1_noGVars129_82* allArgs_1;
   void* _t1;
   int*restrict* nnzPerRow_l;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_20;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars109_args = *(union __args_mapped_step1_noGVars109_64*)
      rocrArgs(depv);
   IT1 = mapped_step1_noGVars109_args.data.IT1;
   normr_20 = mapped_step1_noGVars109_args.data.normr;
   p_l = mapped_step1_noGVars109_args.data.p_l;
   Ap_l = mapped_step1_noGVars109_args.data.Ap_l;
   r_l = mapped_step1_noGVars109_args.data.r_l;
   nnzPerRow_l = mapped_step1_noGVars109_args.data.nnzPerRow_l;
   if (mapped_step1_noGVars109_args.data.IT0 == 0l) {
      long _t2;
      ocrGuid_t _t3;
      union __args_mapped_step1_noGVars149_100* _t4;
      long _t5;
      long i;
      long i_1;
      _t2 = (__mins_64(-65536l * IT1 + 1124863l, 65535l));
      rocrFetchDB(_t1, (void**)&nnzPerRow_l_db_0, 3, 6ul, (unsigned long)0l, (
         unsigned long)IT1);
      for (i = 0l; i <= _t2; i++) {
         nnzPerRow_l_db_0[65536l * IT1 + i + -65536l * (65536l * IT1 + i >> 16l
            )] = nnzPerRow_l[0l][i + 65536l * IT1];
      }
      _t3 = rocrAlloc((void**)&allArgs, 16ul);
      _t4 = allArgs;
      _t4->data.IT0 = 0l;
      _t4->data.IT1 = IT1;
      _t5 = (__mins_64(137l, 8l * IT1 + 7l));
      rocrAutoDec(_t1, 26u, (unsigned long)IT1, _t3,
         mapped_step1_noGVars149_count_26, mapped_step1_noGVars149_dtenum_26, (
         void (*)(void*, long*))((void*)0l), 2u, (long)0l, (long)IT1);
      for (i_1 = 8l * IT1; i_1 <= _t5; i_1++) {
         ocrGuid_t _t6;
         union __args_mapped_step1_noGVars129_82* _t7;
         _t6 = rocrAlloc((void**)&allArgs_1, 40ul);
         _t7 = allArgs_1;
         _t7->data.r_l = r_l;
         _t7->data.Ap_l = Ap_l;
         _t7->data.p_l = p_l;
         _t7->data.normr = normr_20;
         _t7->data.IT0 = i_1;
         rocrAutoDec(_t1, 17u, (unsigned long)i_1, _t6,
            mapped_step1_noGVars129_count_17, mapped_step1_noGVars129_dtenum_17
            , (void (*)(void*, long*))((void*)0l), 1u, (long)i_1);
      }
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars109_dtenum_8(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[1l];
   if ((long)args[0l] == 0l) {
      if (_t1 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 6u, 262144ul, 2l, 1u, (void (*)(void*, long*))(
            (void*)0l), (unsigned long)0l, (unsigned long)_t1);
      }
      if (_t1 == 17l) {
         rocrEnumDB(rocrEDTCtx, 6u, 262144ul, 1l, 1u, (void (*)(void*, long*))(
            (void*)0l), (unsigned long)0l, (unsigned long)17l);
      }
   }
}

static ocrGuid_t mapped_step1_noGVars112(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars112_66 mapped_step1_noGVars112_args;
   double* Ap_l_db_0;
   union __args_mapped_step1_noGVars146_98* allArgs;
   union __args_mapped_step1_noGVars129_82* allArgs_1;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* p_l;
   double* normr_21;
   long IT0;
   long _t2;
   ocrGuid_t _t3;
   union __args_mapped_step1_noGVars146_98* _t4;
   long _t5;
   long i;
   long i_1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars112_args = *(union __args_mapped_step1_noGVars112_66*)
      rocrArgs(depv);
   IT0 = mapped_step1_noGVars112_args.data.IT0;
   _t2 = (__mins_64(-65536l * IT0 + 1124863l, 65535l));
   rocrFetchDB(_t1, (void**)&Ap_l_db_0, 2, 5ul, (unsigned long)IT0);
   for (normr_21 = mapped_step1_noGVars112_args.data.normr,
        p_l = mapped_step1_noGVars112_args.data.p_l,
        r_l = mapped_step1_noGVars112_args.data.r_l,
        Ap_l = mapped_step1_noGVars112_args.data.Ap_l,
        i = 0l;
        i <= _t2; i++) {
      Ap_l_db_0[65536l * IT0 + i + -65536l * (65536l * IT0 + i >> 16l)] = Ap_l[
         i + 65536l * IT0];
   }
   _t3 = rocrAlloc((void**)&allArgs, 16ul);
   _t4 = allArgs;
   _t4->data.Ap_l = Ap_l;
   _t4->data.IT0 = IT0;
   _t5 = (__mins_64(137l, 8l * IT0 + 7l));
   rocrAutoDec(_t1, 25u, (unsigned long)IT0, _t3,
      mapped_step1_noGVars146_count_25, mapped_step1_noGVars146_dtenum_25, (
      void (*)(void*, long*))((void*)0l), 1u, (long)IT0);
   for (i_1 = 8l * IT0; i_1 <= _t5; i_1++) {
      ocrGuid_t _t6;
      union __args_mapped_step1_noGVars129_82* _t7;
      _t6 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t7 = allArgs_1;
      _t7->data.r_l = r_l;
      _t7->data.Ap_l = Ap_l;
      _t7->data.p_l = p_l;
      _t7->data.normr = normr_21;
      _t7->data.IT0 = i_1;
      rocrAutoDec(_t1, 17u, (unsigned long)i_1, _t6,
         mapped_step1_noGVars129_count_17, mapped_step1_noGVars129_dtenum_17, (
         void (*)(void*, long*))((void*)0l), 1u, (long)i_1);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars112_dtenum_9(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[0l];
   if (_t1 <= 16l) {
      rocrEnumDB(rocrEDTCtx, 5u, 524288ul, 2l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)_t1);
   }
   if (_t1 == 17l) {
      rocrEnumDB(rocrEDTCtx, 5u, 524288ul, 1l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)17l);
   }
}

static ocrGuid_t mapped_step1_noGVars117(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars117_68 mapped_step1_noGVars117_args;
   int* _v2_db_0;
   union __args_mapped_step1_noGVars141_96* allArgs;
   union __args_mapped_step1_noGVars129_82* allArgs_1;
   union __args_mapped_step1_noGVars124_78* allArgs_2;
   void* _t1;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_22;
   ocrGuid_t _t2;
   union __args_mapped_step1_noGVars124_78* _t3;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars117_args = *(union __args_mapped_step1_noGVars117_68*)
      rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&_v2_db_0, 1, 4ul);
   *_v2_db_0 = *mapped_step1_noGVars117_args.data._p;
   rocrAutoDec(_t1, 24u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step1_noGVars141_count_24, mapped_step1_noGVars141_dtenum_24, (
      void (*)(void*, long*))((void*)0l), 0u);
   for (normr_22 = mapped_step1_noGVars117_args.data.normr,
        p_l = mapped_step1_noGVars117_args.data.p_l,
        Ap_l = mapped_step1_noGVars117_args.data.Ap_l,
        r_l = mapped_step1_noGVars117_args.data.r_l,
        i = 0l;
        i <= 137l; i++) {
      ocrGuid_t _t4;
      union __args_mapped_step1_noGVars129_82* _t5;
      _t4 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t5 = allArgs_1;
      _t5->data.r_l = r_l;
      _t5->data.Ap_l = Ap_l;
      _t5->data.p_l = p_l;
      _t5->data.normr = normr_22;
      _t5->data.IT0 = i;
      rocrAutoDec(_t1, 17u, (unsigned long)i, _t4,
         mapped_step1_noGVars129_count_17, mapped_step1_noGVars129_dtenum_17, (
         void (*)(void*, long*))((void*)0l), 1u, (long)i);
   }
   _t2 = rocrAlloc((void**)&allArgs_2, 32ul);
   _t3 = allArgs_2;
   _t3->data.r_l = r_l;
   _t3->data.Ap_l = Ap_l;
   _t3->data.p_l = p_l;
   _t3->data.normr = normr_22;
   rocrAutoDec(_t1, 15u, (unsigned long)0l, _t2,
      mapped_step1_noGVars124_count_15, mapped_step1_noGVars124_dtenum_15, (
      void (*)(void*, long*))((void*)0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars117_dtenum_10(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 4u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step1_noGVars119(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars119_70 mapped_step1_noGVars119_args;
   double* x_db_0;
   union __args_mapped_step1_noGVars139_94* allArgs;
   union __args_mapped_step1_noGVars126_80* allArgs_1;
   void* _t1;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_23;
   long IT0;
   long _t2;
   ocrGuid_t _t3;
   ocrGuid_t _t4;
   union __args_mapped_step1_noGVars126_80* _t5;
   double* x_24;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars119_args = *(union __args_mapped_step1_noGVars119_70*)
      rocrArgs(depv);
   IT0 = mapped_step1_noGVars119_args.data.IT0;
   _t2 = (__mins_64(-65536l * IT0 + 1124863l, 65535l));
   rocrFetchDB(_t1, (void**)&x_db_0, 2, 3ul, (unsigned long)IT0);
   for (normr_23 = mapped_step1_noGVars119_args.data.normr,
        p_l = mapped_step1_noGVars119_args.data.p_l,
        Ap_l = mapped_step1_noGVars119_args.data.Ap_l,
        r_l = mapped_step1_noGVars119_args.data.r_l,
        x_24 = mapped_step1_noGVars119_args.data.x,
        i = 0l;
        i <= _t2; i++) {
      x_db_0[65536l * IT0 + i + -65536l * (65536l * IT0 + i >> 16l)] = x_24[i +
         65536l * IT0];
   }
   _t3 = rocrAlloc((void**)&allArgs, 8ul);
   allArgs->data.IT0 = IT0;
   rocrAutoDec(_t1, 23u, (unsigned long)IT0, _t3,
      mapped_step1_noGVars139_count_23, mapped_step1_noGVars139_dtenum_23, (
      void (*)(void*, long*))((void*)0l), 1u, (long)IT0);
   _t4 = rocrAlloc((void**)&allArgs_1, 40ul);
   _t5 = allArgs_1;
   _t5->data.r_l = r_l;
   _t5->data.Ap_l = Ap_l;
   _t5->data.p_l = p_l;
   _t5->data.normr = normr_23;
   _t5->data.IT0 = IT0;
   rocrAutoDec(_t1, 16u, (unsigned long)IT0, _t4,
      mapped_step1_noGVars126_count_16, mapped_step1_noGVars126_dtenum_16, (
      void (*)(void*, long*))((void*)0l), 1u, (long)IT0);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars119_dtenum_11(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[0l];
   if (_t1 <= 16l) {
      rocrEnumDB(rocrEDTCtx, 3u, 524288ul, 2l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)_t1);
   }
   if (_t1 == 17l) {
      rocrEnumDB(rocrEDTCtx, 3u, 524288ul, 1l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)17l);
   }
}

static ocrGuid_t mapped_step1_noGVars122(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars122_72 mapped_step1_noGVars122_args;
   double* p_l_db_0;
   union __args_mapped_step1_noGVars136_92* allArgs;
   union __args_mapped_step1_noGVars129_82* allArgs_1;
   union __args_mapped_step1_noGVars126_80* allArgs_2;
   void* _t1;
   double* p_l;
   double*restrict* r_l;
   double* Ap_l;
   double* normr_24;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars122_args = *(union __args_mapped_step1_noGVars122_72*)
      rocrArgs(depv);
   normr_24 = mapped_step1_noGVars122_args.data.normr;
   Ap_l = mapped_step1_noGVars122_args.data.Ap_l;
   r_l = mapped_step1_noGVars122_args.data.r_l;
   p_l = mapped_step1_noGVars122_args.data.p_l;
   if (mapped_step1_noGVars122_args.data.IT0 == 0l) {
      ocrGuid_t _t2;
      union __args_mapped_step1_noGVars136_92* _t3;
      long i;
      long i_1;
      long i_2;
      rocrFetchDB(_t1, (void**)&p_l_db_0, 2, 2ul, (unsigned long)0l);
      for (i = 0l; i <= 1124863l; i++) {
         p_l_db_0[i + -1124864l * (__divfloors_64(i, 1124864l))] = p_l[i];
      }
      _t2 = rocrAlloc((void**)&allArgs, 16ul);
      _t3 = allArgs;
      _t3->data.p_l = p_l;
      _t3->data.IT0 = 0l;
      rocrAutoDec(_t1, 22u, (unsigned long)0l, _t2,
         mapped_step1_noGVars136_count_22, mapped_step1_noGVars136_dtenum_22, (
         void (*)(void*, long*))((void*)0l), 1u, (long)0l);
      for (i_1 = 0l; i_1 <= 137l; i_1++) {
         ocrGuid_t _t4;
         union __args_mapped_step1_noGVars129_82* _t5;
         _t4 = rocrAlloc((void**)&allArgs_1, 40ul);
         _t5 = allArgs_1;
         _t5->data.r_l = r_l;
         _t5->data.Ap_l = Ap_l;
         _t5->data.p_l = p_l;
         _t5->data.normr = normr_24;
         _t5->data.IT0 = i_1;
         rocrAutoDec(_t1, 17u, (unsigned long)i_1, _t4,
            mapped_step1_noGVars129_count_17, mapped_step1_noGVars129_dtenum_17
            , (void (*)(void*, long*))((void*)0l), 1u, (long)i_1);
      }
      for (i_2 = 0l; i_2 <= 17l; i_2++) {
         ocrGuid_t _t6;
         union __args_mapped_step1_noGVars126_80* _t7;
         _t6 = rocrAlloc((void**)&allArgs_2, 40ul);
         _t7 = allArgs_2;
         _t7->data.r_l = r_l;
         _t7->data.Ap_l = Ap_l;
         _t7->data.p_l = p_l;
         _t7->data.normr = normr_24;
         _t7->data.IT0 = i_2;
         rocrAutoDec(_t1, 16u, (unsigned long)i_2, _t6,
            mapped_step1_noGVars126_count_16, mapped_step1_noGVars126_dtenum_16
            , (void (*)(void*, long*))((void*)0l), 1u, (long)i_2);
      }
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars122_dtenum_12(void* rocrEDTCtx, long* args)
{
   if ((long)args[0l] == 0l) {
      rocrEnumDB(rocrEDTCtx, 2u, 8998912ul, 2l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)0l);
   }
}

static ocrGuid_t mapped_step1_noGVars172(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars172_74 mapped_step1_noGVars172_args;
   int* _v1_db_0;
   union __args_mapped_step1_noGVars175_90* allArgs;
   union __args_mapped_step1_noGVars126_80* allArgs_1;
   union __args_mapped_step1_noGVars124_78* allArgs_2;
   void* _t1;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_25;
   ocrGuid_t _t2;
   union __args_mapped_step1_noGVars124_78* _t3;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars172_args = *(union __args_mapped_step1_noGVars172_74*)
      rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&_v1_db_0, 1, 1ul);
   *_v1_db_0 = *mapped_step1_noGVars172_args.data._p;
   rocrAutoDec(_t1, 21u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step1_noGVars175_count_21, mapped_step1_noGVars175_dtenum_21, (
      void (*)(void*, long*))((void*)0l), 0u);
   for (normr_25 = mapped_step1_noGVars172_args.data.normr,
        p_l = mapped_step1_noGVars172_args.data.p_l,
        Ap_l = mapped_step1_noGVars172_args.data.Ap_l,
        r_l = mapped_step1_noGVars172_args.data.r_l,
        i = 0l;
        i <= 17l; i++) {
      ocrGuid_t _t4;
      union __args_mapped_step1_noGVars126_80* _t5;
      _t4 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t5 = allArgs_1;
      _t5->data.r_l = r_l;
      _t5->data.Ap_l = Ap_l;
      _t5->data.p_l = p_l;
      _t5->data.normr = normr_25;
      _t5->data.IT0 = i;
      rocrAutoDec(_t1, 16u, (unsigned long)i, _t4,
         mapped_step1_noGVars126_count_16, mapped_step1_noGVars126_dtenum_16, (
         void (*)(void*, long*))((void*)0l), 1u, (long)i);
   }
   _t2 = rocrAlloc((void**)&allArgs_2, 32ul);
   _t3 = allArgs_2;
   _t3->data.r_l = r_l;
   _t3->data.Ap_l = Ap_l;
   _t3->data.p_l = p_l;
   _t3->data.normr = normr_25;
   rocrAutoDec(_t1, 15u, (unsigned long)0l, _t2,
      mapped_step1_noGVars124_count_15, mapped_step1_noGVars124_dtenum_15, (
      void (*)(void*, long*))((void*)0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars172_dtenum_13(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 1u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step1_noGVars173(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars173_76 mapped_step1_noGVars173_args;
   int* nPerLvl_l_db_0;
   union __args_mapped_step1_noGVars174_88* allArgs;
   union __args_mapped_step1_noGVars124_78* allArgs_1;
   void* _t1;
   ocrGuid_t _t2;
   union __args_mapped_step1_noGVars124_78* _t3;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars173_args = *(union __args_mapped_step1_noGVars173_76*)
      rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&nPerLvl_l_db_0, 1, 0ul);
   *nPerLvl_l_db_0 = *(int*)mapped_step1_noGVars173_args.data.nPerLvl_l;
   rocrAutoDec(_t1, 20u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step1_noGVars174_count_20, mapped_step1_noGVars174_dtenum_20, (
      void (*)(void*, long*))((void*)0l), 0u);
   _t2 = rocrAlloc((void**)&allArgs_1, 32ul);
   _t3 = allArgs_1;
   _t3->data.r_l = mapped_step1_noGVars173_args.data.r_l;
   _t3->data.Ap_l = mapped_step1_noGVars173_args.data.Ap_l;
   _t3->data.p_l = mapped_step1_noGVars173_args.data.p_l;
   _t3->data.normr = mapped_step1_noGVars173_args.data.normr;
   rocrAutoDec(_t1, 15u, (unsigned long)0l, _t2,
      mapped_step1_noGVars124_count_15, mapped_step1_noGVars124_dtenum_15, (
      void (*)(void*, long*))((void*)0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars173_dtenum_14(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 0u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step1_noGVars124(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars124_78 mapped_step1_noGVars124_args;
   int* nPerLvl_l_db_0;
   int* _v1_db_4;
   int* nPerLvl_l_db_1;
   int* _v2_db_5;
   int* nPerLvl_l_db_2;
   int* _v3_db_6;
   double* local_result_db_7;
   int* nPerLvl_l_db_3;
   int* _v4_db_8;
   union __args_mapped_step1_noGVars179_110* allArgs;
   union __args_mapped_step1_noGVars178_108* allArgs_1;
   union __args_mapped_step1_noGVars151_102* allArgs_2;
   union __args_mapped_step1_noGVars141_96* allArgs_3;
   union __args_mapped_step1_noGVars175_90* allArgs_4;
   union __args_mapped_step1_noGVars174_88* allArgs_5;
   union __args_mapped_step1_noGVars134_86* allArgs_6;
   union __args_mapped_step1_noGVars132_84* allArgs_7;
   union __args_mapped_step1_noGVars129_82* allArgs_8;
   union __args_mapped_step1_noGVars126_80* allArgs_9;
   void* _t1;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_26;
   ocrGuid_t _t2;
   long i;
   long i_1;
   long i_2;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars124_args = *(union __args_mapped_step1_noGVars124_78*)
      rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&nPerLvl_l_db_0, 1, 0ul);
   rocrFetchDB(_t1, (void**)&_v1_db_4, 1, 1ul);
   *_v1_db_4 = *nPerLvl_l_db_0;
   rocrFetchDB(_t1, (void**)&nPerLvl_l_db_1, 1, 0ul);
   rocrFetchDB(_t1, (void**)&_v2_db_5, 1, 4ul);
   *_v2_db_5 = *nPerLvl_l_db_1;
   rocrFetchDB(_t1, (void**)&nPerLvl_l_db_2, 1, 0ul);
   rocrFetchDB(_t1, (void**)&_v3_db_6, 1, 7ul);
   *_v3_db_6 = *nPerLvl_l_db_2;
   rocrFetchDB(_t1, (void**)&local_result_db_7, 1, 10ul);
   *local_result_db_7 = 0.0;
   rocrFetchDB(_t1, (void**)&nPerLvl_l_db_3, 1, 0ul);
   rocrFetchDB(_t1, (void**)&_v4_db_8, 1, 11ul);
   *_v4_db_8 = *nPerLvl_l_db_3;
   rocrAutoDec(_t1, 31u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step1_noGVars179_count_31, mapped_step1_noGVars179_dtenum_31, (
      void (*)(void*, long*))((void*)0l), 0u);
   rocrAutoDec(_t1, 30u, (unsigned long)0l, rocrAlloc((void**)&allArgs_1, 8ul),
       mapped_step1_noGVars178_count_30, mapped_step1_noGVars178_dtenum_30, (
      void (*)(void*, long*))((void*)0l), 0u);
   rocrAutoDec(_t1, 27u, (unsigned long)0l, rocrAlloc((void**)&allArgs_2, 8ul),
       mapped_step1_noGVars151_count_27, mapped_step1_noGVars151_dtenum_27, (
      void (*)(void*, long*))((void*)0l), 0u);
   rocrAutoDec(_t1, 24u, (unsigned long)0l, rocrAlloc((void**)&allArgs_3, 8ul),
       mapped_step1_noGVars141_count_24, mapped_step1_noGVars141_dtenum_24, (
      void (*)(void*, long*))((void*)0l), 0u);
   rocrAutoDec(_t1, 21u, (unsigned long)0l, rocrAlloc((void**)&allArgs_4, 8ul),
       mapped_step1_noGVars175_count_21, mapped_step1_noGVars175_dtenum_21, (
      void (*)(void*, long*))((void*)0l), 0u);
   rocrAutoDec(_t1, 20u, (unsigned long)0l, rocrAlloc((void**)&allArgs_5, 8ul),
       mapped_step1_noGVars174_count_20, mapped_step1_noGVars174_dtenum_20, (
      void (*)(void*, long*))((void*)0l), 0u);
   _t2 = rocrAlloc((void**)&allArgs_6, 8ul);
   normr_26 = mapped_step1_noGVars124_args.data.normr;
   allArgs_6->data.normr = normr_26;
   rocrAutoDec(_t1, 19u, (unsigned long)0l, _t2,
      mapped_step1_noGVars134_count_19, mapped_step1_noGVars134_dtenum_19, (
      void (*)(void*, long*))((void*)0l), 0u);
   for (p_l = mapped_step1_noGVars124_args.data.p_l,
        Ap_l = mapped_step1_noGVars124_args.data.Ap_l,
        r_l = mapped_step1_noGVars124_args.data.r_l,
        i = 0l;
        i <= 8l; i++) {
      ocrGuid_t _t3;
      union __args_mapped_step1_noGVars132_84* _t4;
      _t3 = rocrAlloc((void**)&allArgs_7, 24ul);
      _t4 = allArgs_7;
      _t4->data.r_l = r_l;
      _t4->data.normr = normr_26;
      _t4->data.IT0 = i;
      rocrAutoDec(_t1, 18u, (unsigned long)i, _t3,
         mapped_step1_noGVars132_count_18, mapped_step1_noGVars132_dtenum_18, (
         void (*)(void*, long*))((void*)0l), 1u, (long)i);
   }
   for (i_1 = 0l; i_1 <= 137l; i_1++) {
      ocrGuid_t _t5;
      union __args_mapped_step1_noGVars129_82* _t6;
      _t5 = rocrAlloc((void**)&allArgs_8, 40ul);
      _t6 = allArgs_8;
      _t6->data.r_l = r_l;
      _t6->data.Ap_l = Ap_l;
      _t6->data.p_l = p_l;
      _t6->data.normr = normr_26;
      _t6->data.IT0 = i_1;
      rocrAutoDec(_t1, 17u, (unsigned long)i_1, _t5,
         mapped_step1_noGVars129_count_17, mapped_step1_noGVars129_dtenum_17, (
         void (*)(void*, long*))((void*)0l), 1u, (long)i_1);
   }
   for (i_2 = 0l; i_2 <= 17l; i_2++) {
      ocrGuid_t _t7;
      union __args_mapped_step1_noGVars126_80* _t8;
      _t7 = rocrAlloc((void**)&allArgs_9, 40ul);
      _t8 = allArgs_9;
      _t8->data.r_l = r_l;
      _t8->data.Ap_l = Ap_l;
      _t8->data.p_l = p_l;
      _t8->data.normr = normr_26;
      _t8->data.IT0 = i_2;
      rocrAutoDec(_t1, 16u, (unsigned long)i_2, _t7,
         mapped_step1_noGVars126_count_16, mapped_step1_noGVars126_dtenum_16, (
         void (*)(void*, long*))((void*)0l), 1u, (long)i_2);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars124_dtenum_15(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 11u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
   rocrEnumDB(rocrEDTCtx, 10u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
   rocrEnumDB(rocrEDTCtx, 7u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
   rocrEnumDB(rocrEDTCtx, 4u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
   rocrEnumDB(rocrEDTCtx, 1u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
   rocrEnumDB(rocrEDTCtx, 0u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step1_noGVars126(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars126_80 mapped_step1_noGVars126_args;
   int* _v1_db_0;
   double* p_l_db_1;
   double* x_db_2;
   union __args_mapped_step1_noGVars139_94* allArgs;
   union __args_mapped_step1_noGVars136_92* allArgs_1;
   union __args_mapped_step1_noGVars175_90* allArgs_2;
   union __args_mapped_step1_noGVars129_82* allArgs_3;
   void* _t1;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_27;
   long IT0;
   int _p2_l;
   int _p2;
   ocrGuid_t _t2;
   ocrGuid_t _t3;
   union __args_mapped_step1_noGVars136_92* _t4;
   long _t5;
   long i;
   long i_1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars126_args = *(union __args_mapped_step1_noGVars126_80*)
      rocrArgs(depv);
   _p2_l = _p2;
   rocrFetchDB(_t1, (void**)&_v1_db_0, 1, 1ul);
   IT0 = mapped_step1_noGVars126_args.data.IT0;
   rocrFetchDB(_t1, (void**)&x_db_2, 2, 3ul, (unsigned long)IT0);
   for (_t5 = (__mins_64(-65536l * IT0 + 1124863l, 65535l)),
        normr_27 = mapped_step1_noGVars126_args.data.normr,
        p_l = mapped_step1_noGVars126_args.data.p_l,
        Ap_l = mapped_step1_noGVars126_args.data.Ap_l,
        r_l = mapped_step1_noGVars126_args.data.r_l,
        i = 0l;
        i <= _t5; i++) {
      _p2_l = (int)(i + 65536l * IT0) < *_v1_db_0;
      rocrFetchDB(_t1, (void**)&p_l_db_1, 2, 2ul, (unsigned long)((i + (65536l *
         IT0 + 0l)) / 1124864l));
      if (_p2_l) {
         p_l_db_1[65536l * IT0 + i + -1124864l * (__divfloors_64(65536l * IT0 +
            i, 1124864l))] = x_db_2[65536l * IT0 + i + -65536l * (65536l * IT0 +
            i >> 16l)];
      }
   }
   _p2 = _p2_l;
   _t2 = rocrAlloc((void**)&allArgs, 8ul);
   allArgs->data.IT0 = IT0;
   rocrAutoDec(_t1, 23u, (unsigned long)IT0, _t2,
      mapped_step1_noGVars139_count_23, mapped_step1_noGVars139_dtenum_23, (
      void (*)(void*, long*))((void*)0l), 1u, (long)IT0);
   _t3 = rocrAlloc((void**)&allArgs_1, 16ul);
   _t4 = allArgs_1;
   _t4->data.p_l = p_l;
   _t4->data.IT0 = 0l;
   rocrAutoDec(_t1, 22u, (unsigned long)0l, _t3,
      mapped_step1_noGVars136_count_22, mapped_step1_noGVars136_dtenum_22, (
      void (*)(void*, long*))((void*)0l), 1u, (long)0l);
   rocrAutoDec(_t1, 21u, (unsigned long)0l, rocrAlloc((void**)&allArgs_2, 8ul),
       mapped_step1_noGVars175_count_21, mapped_step1_noGVars175_dtenum_21, (
      void (*)(void*, long*))((void*)0l), 0u);
   for (i_1 = 0l; i_1 <= 137l; i_1++) {
      ocrGuid_t _t6;
      union __args_mapped_step1_noGVars129_82* _t7;
      _t6 = rocrAlloc((void**)&allArgs_3, 40ul);
      _t7 = allArgs_3;
      _t7->data.r_l = r_l;
      _t7->data.Ap_l = Ap_l;
      _t7->data.p_l = p_l;
      _t7->data.normr = normr_27;
      _t7->data.IT0 = i_1;
      rocrAutoDec(_t1, 17u, (unsigned long)i_1, _t6,
         mapped_step1_noGVars129_count_17, mapped_step1_noGVars129_dtenum_17, (
         void (*)(void*, long*))((void*)0l), 1u, (long)i_1);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars126_dtenum_16(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[0l];
   if (_t1 <= 16l) {
      rocrEnumDB(rocrEDTCtx, 3u, 524288ul, 2l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)_t1);
   }
   if (_t1 == 17l) {
      rocrEnumDB(rocrEDTCtx, 3u, 524288ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)17l);
   }
   rocrEnumDB(rocrEDTCtx, 2u, 8998912ul, 1l, 1u, (void (*)(void*, long*))((void
      *)0l), (unsigned long)0l);
   rocrEnumDB(rocrEDTCtx, 1u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step1_noGVars129(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars129_82 mapped_step1_noGVars129_args;
   int _p3_l[8192];
   int* _v2_db_0;
   double* Ap_l_db_1;
   int* nnzPerRow_l_db_4;
   int _p5_l[8192];
   int* _v3_db_5;
   double* r_l_db_6;
   double* b_l_db_7;
   double* Ap_l_db_2;
   int* colInds_l_db_8;
   double* A_l_db_9;
   double* p_l_db_10;
   double* Ap_l_db_3;
   int _p3[8192];
   int _p5[8192];
   union __args_mapped_step1_noGVars166_116* allArgs;
   union __args_mapped_step1_noGVars163_114* allArgs_1;
   union __args_mapped_step1_noGVars159_106* allArgs_2;
   union __args_mapped_step1_noGVars156_104* allArgs_3;
   union __args_mapped_step1_noGVars151_102* allArgs_4;
   union __args_mapped_step1_noGVars149_100* allArgs_5;
   union __args_mapped_step1_noGVars146_98* allArgs_6;
   union __args_mapped_step1_noGVars141_96* allArgs_7;
   union __args_mapped_step1_noGVars136_92* allArgs_8;
   union __args_mapped_step1_noGVars132_84* allArgs_9;
   void* _t1;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_28;
   long IT0;
   long _t2;
   long _t3;
   long _t4;
   ocrGuid_t _t5;
   union __args_mapped_step1_noGVars136_92* _t6;
   long _t7;
   long _t8;
   long i;
   long i_1;
   long _t9;
   long i_2;
   long _t10;
   long i_3;
   long i_4;
   long i_5;
   long i_6;
   long i_7;
   long _t11;
   long i_8;
   long i_9;
   long _t12;
   long i_10;
   long _t13;
   long i_11;
   long _t14;
   long i_12;
   long _t15;
   long i_13;
   long i_14;
   long _t16;
   long i_15;
   long i_16;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        mapped_step1_noGVars129_args = *(union __args_mapped_step1_noGVars129_82
           *)rocrArgs(depv),
        IT0 = mapped_step1_noGVars129_args.data.IT0,
        _t8 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)),
        normr_28 = mapped_step1_noGVars129_args.data.normr,
        p_l = mapped_step1_noGVars129_args.data.p_l,
        Ap_l = mapped_step1_noGVars129_args.data.Ap_l,
        r_l = mapped_step1_noGVars129_args.data.r_l,
        i = 0l;
        i <= _t8; i++) {
      _p3_l[i] = _p3[i];
   }
   rocrFetchDB(_t1, (void**)&_v2_db_0, 1, 4ul);
   rocrFetchDB(_t1, (void**)&Ap_l_db_1, 2, 5ul, (unsigned long)(IT0 >> 3l));
   _t2 = (__mins_64(-8192l * IT0 + 1124863l, 8191l));
   rocrFetchDB(_t1, (void**)&_v3_db_5, 1, 7ul);
   for (i_1 = 0l; i_1 <= _t2; i_1++) {
      _p3_l[i_1] = (int)(i_1 + 8192l * IT0) < *_v2_db_0;
      if (_p3_l[i_1]) {
         Ap_l_db_1[8192l * IT0 + i_1 + -65536l * (8192l * IT0 + i_1 >> 16l)] =
            0.0;
      }
      _p5_l[i_1] = (int)(i_1 + 8192l * IT0) < *_v3_db_5;
   }
   for (_t9 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)), i_2 = 0l; i_2 <= _t9
           ; i_2++) {
      _p3[i_2] = _p3_l[i_2];
   }
   for (_t10 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)), i_3 = 0l; i_3 <=
           _t10; i_3++) {
      _p5[i_3] = _p5_l[i_3];
   }
   for (i_4 = 0l,
        i_5 = 0l,
        i_6 = 0l,
        i_7 = 26l;
        i_4 <= 1l;
        i_4++,
        i_5 += 16,
        i_6 += 16,
        i_7 -= 16) {
      int _p4_l;
      int _p4;
      long _t17;
      long j;
      long _t18;
      long j_1;
      long j_2;
      for (_t17 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)), j = 0l; j <= _t17
              ; j++) {
         _p3_l[j] = _p3[j];
      }
      _p4_l = _p4;
      rocrFetchDB(_t1, (void**)&Ap_l_db_3, 2, 5ul, (unsigned long)(IT0 >> 3l));
      rocrFetchDB(_t1, (void**)&nnzPerRow_l_db_4, 3, 6ul, (unsigned long)0l, (
         unsigned long)(IT0 >> 3l));
      rocrFetchDB(_t1, (void**)&colInds_l_db_8, 4, 13ul, (unsigned long)0l, (
         unsigned long)(IT0 >> 3l), (unsigned long)0l);
      rocrFetchDB(_t1, (void**)&A_l_db_9, 4, 14ul, (unsigned long)0l, (
         unsigned long)(IT0 >> 3l), (unsigned long)0l);
      rocrFetchDB(_t1, (void**)&p_l_db_10, 2, 2ul, (unsigned long)0l);
      for (_t18 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)), j_1 = 0l, j_2 =
              0l; j_1 <= _t18; j_1++, j_2 += 27) {
         if (_p3_l[j_1]) {
            long _t19;
            long k;
            long k_1;
            long k_2;
            long k_3;
            for (_t19 = (__mins_64(i_7, 15l)),
                 k = 0l,
                 k_1 = i_6,
                 k_3 = i_6,
                 k_2 = i_6;
                 k <= _t19;
                 k++,
                 k_1++,
                 k_2++,
                 k_3++) {
               _p4_l = (int)k_1 < nnzPerRow_l_db_4[8192l * IT0 + j_1 + -65536l *
                  (8192l * IT0 + j_1 >> 16l)];
               if (_p4_l) {
                  spmvBB(colInds_l_db_8 + (221184l * IT0 + k + j_2 + i_5 + -27l
                     * (__divfloors_64(k_2, 27l)) + -1769472l * (8192l * IT0 +
                     j_1 >> 16l)), A_l_db_9 + (221184l * IT0 + k + j_2 + i_5 +
                     -27l * (__divfloors_64(k_3, 27l)) + -1769472l * (8192l *
                     IT0 + j_1 >> 16l)), p_l_db_10 + 0l, Ap_l_db_3 + (8192l *
                     IT0 + j_1 + -65536l * (8192l * IT0 + j_1 >> 16l)));
               }
            }
         }
      }
      _p4 = _p4_l;
   }
   for (_t11 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)), i_8 = 0l; i_8 <=
           _t11; i_8++) {
      _p5_l[i_8] = _p5[i_8];
   }
   rocrFetchDB(_t1, (void**)&Ap_l_db_2, 2, 5ul, (unsigned long)(IT0 >> 3l));
   rocrFetchDB(_t1, (void**)&r_l_db_6, 3, 8ul, (unsigned long)0l, (
      unsigned long)(IT0 >> 3l));
   _t3 = (__mins_64(-8192l * IT0 + 1124863l, 8191l));
   rocrFetchDB(_t1, (void**)&b_l_db_7, 2, 9ul, (unsigned long)(IT0 >> 3l));
   for (i_9 = 0l; i_9 <= _t3; i_9++) {
      if (_p5_l[i_9]) {
         r_l_db_6[8192l * IT0 + i_9 + -65536l * (8192l * IT0 + i_9 >> 16l)] =
            b_l_db_7[8192l * IT0 + i_9 + -65536l * (8192l * IT0 + i_9 >> 16l)] +
            -1.0 * Ap_l_db_2[8192l * IT0 + i_9 + -65536l * (8192l * IT0 + i_9 >>
            16l)];
      }
   }
   for (_t12 = (__mins_64(IT0 >> 3l, 17l)), i_10 = (__maxs_64(0l, IT0 + -7l + 7l
           >> 3l)); i_10 <= _t12; i_10++) {
      ocrGuid_t _t20;
      union __args_mapped_step1_noGVars166_116* _t21;
      _t20 = rocrAlloc((void**)&allArgs, 24ul);
      _t21 = allArgs;
      _t21->data.IT0 = 0l;
      _t21->data.IT1 = i_10;
      _t21->data.IT2 = 0l;
      rocrAutoDec(_t1, 34u, (unsigned long)i_10, _t20,
         mapped_step1_noGVars166_count_34, mapped_step1_noGVars166_dtenum_34, (
         void (*)(void*, long*))((void*)0l), 3u, (long)0l, (long)i_10, (long)0l
         );
   }
   for (_t13 = (__mins_64(IT0 >> 3l, 17l)), i_11 = (__maxs_64(0l, IT0 + -7l + 7l
           >> 3l)); i_11 <= _t13; i_11++) {
      ocrGuid_t _t22;
      union __args_mapped_step1_noGVars163_114* _t23;
      _t22 = rocrAlloc((void**)&allArgs_1, 24ul);
      _t23 = allArgs_1;
      _t23->data.IT0 = 0l;
      _t23->data.IT1 = i_11;
      _t23->data.IT2 = 0l;
      rocrAutoDec(_t1, 33u, (unsigned long)i_11, _t22,
         mapped_step1_noGVars163_count_33, mapped_step1_noGVars163_dtenum_33, (
         void (*)(void*, long*))((void*)0l), 3u, (long)0l, (long)i_11, (long)0l
         );
   }
   for (_t14 = (__mins_64(IT0 >> 3l, 17l)), i_12 = (__maxs_64(0l, IT0 + -7l + 7l
           >> 3l)); i_12 <= _t14; i_12++) {
      ocrGuid_t _t24;
      _t24 = rocrAlloc((void**)&allArgs_2, 8ul);
      allArgs_2->data.IT0 = i_12;
      rocrAutoDec(_t1, 29u, (unsigned long)i_12, _t24,
         mapped_step1_noGVars159_count_29, mapped_step1_noGVars159_dtenum_29, (
         void (*)(void*, long*))((void*)0l), 1u, (long)i_12);
   }
   for (_t15 = (__mins_64(IT0 >> 3l, 17l)), i_13 = (__maxs_64(0l, IT0 + -7l + 7l
           >> 3l)); i_13 <= _t15; i_13++) {
      ocrGuid_t _t25;
      union __args_mapped_step1_noGVars156_104* _t26;
      _t25 = rocrAlloc((void**)&allArgs_3, 24ul);
      _t26 = allArgs_3;
      _t26->data.r_l = r_l;
      _t26->data.IT0 = 0l;
      _t26->data.IT1 = i_13;
      rocrAutoDec(_t1, 28u, (unsigned long)i_13, _t25,
         mapped_step1_noGVars156_count_28, mapped_step1_noGVars156_dtenum_28, (
         void (*)(void*, long*))((void*)0l), 2u, (long)0l, (long)i_13);
   }
   _t4 = (__mins_64(IT0 >> 3l, 17l));
   rocrAutoDec(_t1, 27u, (unsigned long)0l, rocrAlloc((void**)&allArgs_4, 8ul),
       mapped_step1_noGVars151_count_27, mapped_step1_noGVars151_dtenum_27, (
      void (*)(void*, long*))((void*)0l), 0u);
   for (i_14 = (__maxs_64(0l, IT0 + -7l + 7l >> 3l)); i_14 <= _t4; i_14++) {
      ocrGuid_t _t27;
      union __args_mapped_step1_noGVars149_100* _t28;
      _t27 = rocrAlloc((void**)&allArgs_5, 16ul);
      _t28 = allArgs_5;
      _t28->data.IT0 = 0l;
      _t28->data.IT1 = i_14;
      rocrAutoDec(_t1, 26u, (unsigned long)i_14, _t27,
         mapped_step1_noGVars149_count_26, mapped_step1_noGVars149_dtenum_26, (
         void (*)(void*, long*))((void*)0l), 2u, (long)0l, (long)i_14);
   }
   for (_t16 = (__mins_64(IT0 >> 3l, 17l)), i_15 = (__maxs_64(0l, IT0 + -7l + 7l
           >> 3l)); i_15 <= _t16; i_15++) {
      ocrGuid_t _t29;
      union __args_mapped_step1_noGVars146_98* _t30;
      _t29 = rocrAlloc((void**)&allArgs_6, 16ul);
      _t30 = allArgs_6;
      _t30->data.Ap_l = Ap_l;
      _t30->data.IT0 = i_15;
      rocrAutoDec(_t1, 25u, (unsigned long)i_15, _t29,
         mapped_step1_noGVars146_count_25, mapped_step1_noGVars146_dtenum_25, (
         void (*)(void*, long*))((void*)0l), 1u, (long)i_15);
   }
   rocrAutoDec(_t1, 24u, (unsigned long)0l, rocrAlloc((void**)&allArgs_7, 8ul),
       mapped_step1_noGVars141_count_24, mapped_step1_noGVars141_dtenum_24, (
      void (*)(void*, long*))((void*)0l), 0u);
   _t5 = rocrAlloc((void**)&allArgs_8, 16ul);
   _t6 = allArgs_8;
   _t6->data.p_l = p_l;
   _t6->data.IT0 = 0l;
   _t7 = (__mins_64(IT0 >> 4l, 8l));
   rocrAutoDec(_t1, 22u, (unsigned long)0l, _t5,
      mapped_step1_noGVars136_count_22, mapped_step1_noGVars136_dtenum_22, (
      void (*)(void*, long*))((void*)0l), 1u, (long)0l);
   for (i_16 = (__maxs_64(0l, IT0 + -15l + 15l >> 4l)); i_16 <= _t7; i_16++) {
      ocrGuid_t _t31;
      union __args_mapped_step1_noGVars132_84* _t32;
      _t31 = rocrAlloc((void**)&allArgs_9, 24ul);
      _t32 = allArgs_9;
      _t32->data.r_l = r_l;
      _t32->data.normr = normr_28;
      _t32->data.IT0 = i_16;
      rocrAutoDec(_t1, 18u, (unsigned long)i_16, _t31,
         mapped_step1_noGVars132_count_18, mapped_step1_noGVars132_dtenum_18, (
         void (*)(void*, long*))((void*)0l), 1u, (long)i_16);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars129_dtenum_17(void* rocrEDTCtx, long* args)
{
   long _t1;
   long _t2;
   long _t3;
   long i;
   long _t4;
   long i_1;
   long _t5;
   long i_2;
   long _t6;
   long i_3;
   long i_4;
   long _t7;
   long i_5;
   for (_t1 = (long)args[0l], _t3 = (__mins_64(_t1 >> 3l, 17l)), i = (__maxs_64
           (0l, _t1 + -7l + 7l >> 3l)); i <= _t3; i++) {
      rocrEnumDB(rocrEDTCtx, 14u, 14155776ul, 1l, 0u, (void (*)(void*, long*))(
         (void*)0l), (unsigned long)0l, (unsigned long)i, (unsigned long)0l);
   }
   for (_t4 = (__mins_64(_t1 >> 3l, 17l)), i_1 = (__maxs_64(0l, _t1 + -7l + 7l
           >> 3l)); i_1 <= _t4; i_1++) {
      rocrEnumDB(rocrEDTCtx, 13u, 7077888ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)0l, (unsigned long)i_1, (unsigned long)0l);
   }
   for (_t5 = (__mins_64(_t1 >> 3l, 17l)), i_2 = (__maxs_64(0l, _t1 + -7l + 7l
           >> 3l)); i_2 <= _t5; i_2++) {
      rocrEnumDB(rocrEDTCtx, 9u, 524288ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)i_2);
   }
   for (_t6 = (__mins_64(_t1 >> 3l, 17l)), i_3 = (__maxs_64(0l, _t1 + -7l + 7l
           >> 3l)); i_3 <= _t6; i_3++) {
      rocrEnumDB(rocrEDTCtx, 8u, 524288ul, 1l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)0l, (unsigned long)i_3);
   }
   _t2 = (__mins_64(_t1 >> 3l, 17l));
   rocrEnumDB(rocrEDTCtx, 7u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
   for (i_4 = (__maxs_64(0l, _t1 + -7l + 7l >> 3l)); i_4 <= _t2; i_4++) {
      rocrEnumDB(rocrEDTCtx, 6u, 262144ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)0l, (unsigned long)i_4);
   }
   for (_t7 = (__mins_64(_t1 >> 3l, 17l)), i_5 = (__maxs_64(0l, _t1 + -7l + 7l
           >> 3l)); i_5 <= _t7; i_5++) {
      rocrEnumDB(rocrEDTCtx, 5u, 524288ul, 1l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)i_5);
   }
   rocrEnumDB(rocrEDTCtx, 4u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
   rocrEnumDB(rocrEDTCtx, 2u, 8998912ul, 2l, 0u, (void (*)(void*, long*))((void
      *)0l), (unsigned long)0l);
}

static ocrGuid_t mapped_step1_noGVars132(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars132_84 mapped_step1_noGVars132_args;
   int* _v4_db_0;
   double* r_l_db_1;
   double* r_l_db_2;
   double* local_result_db_3;
   union __args_mapped_step1_noGVars179_110* allArgs;
   union __args_mapped_step1_noGVars178_108* allArgs_1;
   union __args_mapped_step1_noGVars156_104* allArgs_2;
   union __args_mapped_step1_noGVars134_86* allArgs_3;
   union __args_mapped_step1_noGVars132_84* allArgs_4;
   void* _t1;
   double*restrict* r_l;
   double* normr_29;
   long IT0;
   int _p1_l;
   int _p1;
   long _t2;
   ocrGuid_t _t3;
   long i;
   long i_1;
   long i_2;
   long i_3;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars132_args = *(union __args_mapped_step1_noGVars132_84*)
      rocrArgs(depv);
   _p1_l = _p1;
   rocrFetchDB(_t1, (void**)&_v4_db_0, 1, 11ul);
   rocrFetchDB(_t1, (void**)&local_result_db_3, 1, 10ul);
   for (IT0 = mapped_step1_noGVars132_args.data.IT0,
        normr_29 = mapped_step1_noGVars132_args.data.normr,
        r_l = mapped_step1_noGVars132_args.data.r_l,
        i = 0l,
        i_1 = 0l,
        i_2 = 0l;
        i <= 1l; i++, i_1 += 65536, i_2 -= 65536) {
      long _t4;
      long j;
      rocrFetchDB(_t1, (void**)&r_l_db_1, 3, 8ul, (unsigned long)0l, (
         unsigned long)(i + 2l * IT0));
      _t4 = (__mins_64(i_2 + -131072l * IT0 + 1124863l, 65535l));
      rocrFetchDB(_t1, (void**)&r_l_db_2, 3, 8ul, (unsigned long)0l, (
         unsigned long)(i + 2l * IT0));
      for (j = 0l; j <= _t4; j++) {
         _p1_l = (int)(i_1 + (j + 131072l * IT0)) < *_v4_db_0;
         if (_p1_l) {
            double* _t5;
            _t5 = local_result_db_3;
            *_t5 = *_t5 + r_l_db_1[131072l * IT0 + j + i_1 + -65536l * (131072l
               * IT0 + j + i_1 >> 16l)] * r_l_db_2[131072l * IT0 + j + i_1 +
               -65536l * (131072l * IT0 + j + i_1 >> 16l)];
         }
      }
   }
   _p1 = _p1_l;
   rocrAutoDec(_t1, 31u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step1_noGVars179_count_31, mapped_step1_noGVars179_dtenum_31, (
      void (*)(void*, long*))((void*)0l), 0u);
   _t2 = 2l * IT0 + 1l;
   rocrAutoDec(_t1, 30u, (unsigned long)0l, rocrAlloc((void**)&allArgs_1, 8ul),
       mapped_step1_noGVars178_count_30, mapped_step1_noGVars178_dtenum_30, (
      void (*)(void*, long*))((void*)0l), 0u);
   for (i_3 = 2l * IT0; i_3 <= _t2; i_3++) {
      ocrGuid_t _t6;
      union __args_mapped_step1_noGVars156_104* _t7;
      _t6 = rocrAlloc((void**)&allArgs_2, 24ul);
      _t7 = allArgs_2;
      _t7->data.r_l = r_l;
      _t7->data.IT0 = 0l;
      _t7->data.IT1 = i_3;
      rocrAutoDec(_t1, 28u, (unsigned long)i_3, _t6,
         mapped_step1_noGVars156_count_28, mapped_step1_noGVars156_dtenum_28, (
         void (*)(void*, long*))((void*)0l), 2u, (long)0l, (long)i_3);
   }
   _t3 = rocrAlloc((void**)&allArgs_3, 8ul);
   allArgs_3->data.normr = normr_29;
   rocrAutoDec(_t1, 19u, (unsigned long)0l, _t3,
      mapped_step1_noGVars134_count_19, mapped_step1_noGVars134_dtenum_19, (
      void (*)(void*, long*))((void*)0l), 0u);
   if (IT0 <= 7l) {
      ocrGuid_t _t8;
      union __args_mapped_step1_noGVars132_84* _t9;
      long _t10;
      _t8 = rocrAlloc((void**)&allArgs_4, 24ul);
      _t9 = allArgs_4;
      _t9->data.r_l = r_l;
      _t9->data.normr = normr_29;
      _t10 = IT0 + 1l;
      _t9->data.IT0 = _t10;
      rocrAutoDec(_t1, 18u, (unsigned long)(1l + IT0), _t8,
         mapped_step1_noGVars132_count_18, mapped_step1_noGVars132_dtenum_18, (
         void (*)(void*, long*))((void*)0l), 1u, (long)_t10);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars132_dtenum_18(void* rocrEDTCtx, long* args)
{
   long _t1;
   long _t2;
   long i;
   rocrEnumDB(rocrEDTCtx, 11u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
   _t1 = (long)args[0l];
   _t2 = (__mins_64(16l, 2l * _t1 + 1l));
   rocrEnumDB(rocrEDTCtx, 10u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
   for (i = 2l * _t1; i <= _t2; i++) {
      rocrEnumDB(rocrEDTCtx, 8u, 524288ul, 2l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)0l, (unsigned long)i);
   }
   if (_t1 == 8l) {
      rocrEnumDB(rocrEDTCtx, 8u, 524288ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)0l, (unsigned long)17l);
   }
}

static ocrGuid_t mapped_step1_noGVars134(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars134_86 mapped_step1_noGVars134_args;
   double* normr_db_0;
   double* local_result_db_1;
   union __args_mapped_step1_noGVars161_112* allArgs;
   union __args_mapped_step1_noGVars178_108* allArgs_1;
   void* _t1;
   ocrGuid_t _t2;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars134_args = *(union __args_mapped_step1_noGVars134_86*)
      rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&normr_db_0, 1, 12ul);
   rocrFetchDB(_t1, (void**)&local_result_db_1, 1, 10ul);
   *normr_db_0 = *local_result_db_1;
   _t2 = rocrAlloc((void**)&allArgs, 8ul);
   allArgs->data.normr = mapped_step1_noGVars134_args.data.normr;
   rocrAutoDec(_t1, 32u, (unsigned long)0l, _t2,
      mapped_step1_noGVars161_count_32, mapped_step1_noGVars161_dtenum_32, (
      void (*)(void*, long*))((void*)0l), 0u);
   rocrAutoDec(_t1, 30u, (unsigned long)0l, rocrAlloc((void**)&allArgs_1, 8ul),
       mapped_step1_noGVars178_count_30, mapped_step1_noGVars178_dtenum_30, (
      void (*)(void*, long*))((void*)0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars134_dtenum_19(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 12u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
   rocrEnumDB(rocrEDTCtx, 10u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step1_noGVars174(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 0ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars174_dtenum_20(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 0u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step1_noGVars175(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 1ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars175_dtenum_21(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 1u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step1_noGVars136(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars136_92 mapped_step1_noGVars136_args;
   double* p_l_db_0;
   void* _t1;
   double* p_l;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars136_args = *(union __args_mapped_step1_noGVars136_92*)
      rocrArgs(depv);
   p_l = mapped_step1_noGVars136_args.data.p_l;
   if (mapped_step1_noGVars136_args.data.IT0 == 0l) {
      long i;
      rocrFetchDB(_t1, (void**)&p_l_db_0, 2, 2ul, (unsigned long)0l);
      for (i = 0l; i <= 1124863l; i++) {
         p_l[i] = p_l_db_0[i + -1124864l * (__divfloors_64(i, 1124864l))];
      }
      rocrFreeDB(_t1, 2, 2ul, (unsigned long)0l);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars136_dtenum_22(void* rocrEDTCtx, long* args)
{
   if ((long)args[0l] == 0l) {
      rocrEnumDB(rocrEDTCtx, 2u, 8998912ul, 2l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)0l);
   }
}

static ocrGuid_t mapped_step1_noGVars139(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars139_94 mapped_step1_noGVars139_args;
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars139_args = *(union __args_mapped_step1_noGVars139_94*)
      rocrArgs(depv);
   rocrFreeDB(_t1, 2, 3ul, (unsigned long)mapped_step1_noGVars139_args.data.IT0
      );
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars139_dtenum_23(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[0l];
   if (_t1 <= 16l) {
      rocrEnumDB(rocrEDTCtx, 3u, 524288ul, 2l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)_t1);
   }
   if (_t1 == 17l) {
      rocrEnumDB(rocrEDTCtx, 3u, 524288ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)17l);
   }
}

static ocrGuid_t mapped_step1_noGVars141(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 4ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars141_dtenum_24(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 4u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step1_noGVars146(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars146_98 mapped_step1_noGVars146_args;
   double* Ap_l_db_0;
   void* _t1;
   long IT0;
   long _t2;
   double* Ap_l;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars146_args = *(union __args_mapped_step1_noGVars146_98*)
      rocrArgs(depv);
   IT0 = mapped_step1_noGVars146_args.data.IT0;
   _t2 = (__mins_64(-65536l * IT0 + 1124863l, 65535l));
   rocrFetchDB(_t1, (void**)&Ap_l_db_0, 2, 5ul, (unsigned long)IT0);
   for (Ap_l = mapped_step1_noGVars146_args.data.Ap_l, i = 0l; i <= _t2; i++) {
      Ap_l[i + 65536l * IT0] = Ap_l_db_0[65536l * IT0 + i + -65536l * (65536l *
         IT0 + i >> 16l)];
   }
   rocrFreeDB(_t1, 2, 5ul, (unsigned long)IT0);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars146_dtenum_25(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[0l];
   if (_t1 <= 16l) {
      rocrEnumDB(rocrEDTCtx, 5u, 524288ul, 2l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)_t1);
   }
   if (_t1 == 17l) {
      rocrEnumDB(rocrEDTCtx, 5u, 524288ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)17l);
   }
}

static ocrGuid_t mapped_step1_noGVars149(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars149_100 mapped_step1_noGVars149_args;
   void* _t1;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars149_args = *(union __args_mapped_step1_noGVars149_100*)
      rocrArgs(depv);
   IT1 = mapped_step1_noGVars149_args.data.IT1;
   if (mapped_step1_noGVars149_args.data.IT0 == 0l) {
      rocrFreeDB(_t1, 3, 6ul, (unsigned long)0l, (unsigned long)IT1);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars149_dtenum_26(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[1l];
   if ((long)args[0l] == 0l) {
      if (_t1 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 6u, 262144ul, 2l, 0u, (void (*)(void*, long*))(
            (void*)0l), (unsigned long)0l, (unsigned long)_t1);
      }
      if (_t1 == 17l) {
         rocrEnumDB(rocrEDTCtx, 6u, 262144ul, 1l, 0u, (void (*)(void*, long*))(
            (void*)0l), (unsigned long)0l, (unsigned long)17l);
      }
   }
}

static ocrGuid_t mapped_step1_noGVars151(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 7ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars151_dtenum_27(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 7u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step1_noGVars156(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars156_104 mapped_step1_noGVars156_args;
   double* r_l_db_0;
   void* _t1;
   double*restrict* r_l;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars156_args = *(union __args_mapped_step1_noGVars156_104*)
      rocrArgs(depv);
   IT1 = mapped_step1_noGVars156_args.data.IT1;
   r_l = mapped_step1_noGVars156_args.data.r_l;
   if (mapped_step1_noGVars156_args.data.IT0 == 0l) {
      long _t2;
      long i;
      _t2 = (__mins_64(-65536l * IT1 + 1124863l, 65535l));
      rocrFetchDB(_t1, (void**)&r_l_db_0, 3, 8ul, (unsigned long)0l, (
         unsigned long)IT1);
      for (i = 0l; i <= _t2; i++) {
         r_l[0l][i + 65536l * IT1] = r_l_db_0[65536l * IT1 + i + -65536l * (
            65536l * IT1 + i >> 16l)];
      }
      rocrFreeDB(_t1, 3, 8ul, (unsigned long)0l, (unsigned long)IT1);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars156_dtenum_28(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[1l];
   if ((long)args[0l] == 0l) {
      if (_t1 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 8u, 524288ul, 2l, 0u, (void (*)(void*, long*))(
            (void*)0l), (unsigned long)0l, (unsigned long)_t1);
      }
      if (_t1 == 17l) {
         rocrEnumDB(rocrEDTCtx, 8u, 524288ul, 1l, 0u, (void (*)(void*, long*))(
            (void*)0l), (unsigned long)0l, (unsigned long)17l);
      }
   }
}

static ocrGuid_t mapped_step1_noGVars159(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars159_106 mapped_step1_noGVars159_args;
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars159_args = *(union __args_mapped_step1_noGVars159_106*)
      rocrArgs(depv);
   rocrFreeDB(_t1, 2, 9ul, (unsigned long)mapped_step1_noGVars159_args.data.IT0
      );
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars159_dtenum_29(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[0l];
   if (_t1 <= 16l) {
      rocrEnumDB(rocrEDTCtx, 9u, 524288ul, 2l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)_t1);
   }
   if (_t1 == 17l) {
      rocrEnumDB(rocrEDTCtx, 9u, 524288ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)17l);
   }
}

static ocrGuid_t mapped_step1_noGVars178(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 10ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars178_dtenum_30(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 10u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step1_noGVars179(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 11ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars179_dtenum_31(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 11u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step1_noGVars161(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars161_112 mapped_step1_noGVars161_args;
   double* normr_db_0;
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars161_args = *(union __args_mapped_step1_noGVars161_112*)
      rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&normr_db_0, 1, 12ul);
   *mapped_step1_noGVars161_args.data.normr = *normr_db_0;
   rocrFreeDB(_t1, 1, 12ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars161_dtenum_32(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 12u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step1_noGVars163(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars163_114 mapped_step1_noGVars163_args;
   void* _t1;
   long IT0;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars163_args = *(union __args_mapped_step1_noGVars163_114*)
      rocrArgs(depv);
   IT1 = mapped_step1_noGVars163_args.data.IT1;
   IT0 = mapped_step1_noGVars163_args.data.IT0;
   if (mapped_step1_noGVars163_args.data.IT2 == 0l && IT0 == 0l) {
      rocrFreeDB(_t1, 4, 13ul, (unsigned long)0l, (unsigned long)IT1, (
         unsigned long)0l);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars163_dtenum_33(void* rocrEDTCtx, long* args)
{
   long _t1;
   long _t2;
   _t2 = (long)args[1l];
   _t1 = (long)args[0l];
   if ((long)args[2l] == 0l && _t1 == 0l) {
      if (_t2 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 13u, 7077888ul, 2l, 0u, (void (*)(void*, long*)
            )((void*)0l), (unsigned long)0l, (unsigned long)_t2, (unsigned long
            )0l);
      }
      if (_t2 == 17l) {
         rocrEnumDB(rocrEDTCtx, 13u, 7077888ul, 1l, 0u, (void (*)(void*, long*)
            )((void*)0l), (unsigned long)0l, (unsigned long)17l, (unsigned long
            )0l);
      }
   }
}

static ocrGuid_t mapped_step1_noGVars166(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars166_116 mapped_step1_noGVars166_args;
   void* _t1;
   long IT0;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step1_noGVars166_args = *(union __args_mapped_step1_noGVars166_116*)
      rocrArgs(depv);
   IT1 = mapped_step1_noGVars166_args.data.IT1;
   IT0 = mapped_step1_noGVars166_args.data.IT0;
   if (mapped_step1_noGVars166_args.data.IT2 == 0l && IT0 == 0l) {
      rocrFreeDB(_t1, 4, 14ul, (unsigned long)0l, (unsigned long)IT1, (
         unsigned long)0l);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars166_dtenum_34(void* rocrEDTCtx, long* args)
{
   long _t1;
   long _t2;
   _t2 = (long)args[1l];
   _t1 = (long)args[0l];
   if ((long)args[2l] == 0l && _t1 == 0l) {
      if (_t2 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 14u, 14155776ul, 2l, 0u, (void (*)(void*, long*
            ))((void*)0l), (unsigned long)0l, (unsigned long)_t2, (
            unsigned long)0l);
      }
      if (_t2 == 17l) {
         rocrEnumDB(rocrEDTCtx, 14u, 14155776ul, 1l, 0u, (void (*)(void*, long*
            ))((void*)0l), (unsigned long)0l, (unsigned long)17l, (
            unsigned long)0l);
      }
   }
}

unsigned long mapped_step1_noGVars166_count_34(long* args)
{
   long _t1;
   long _t2;
   unsigned long rcNbInputDeps;
   _t2 = (long)args[1l];
   _t1 = (long)args[0l];
   if ((long)args[2l] == 0l) {
      if (_t1 == 0l) {
         long _t3;
         long i;
         for (_t3 = (__mins_64(137l, 8l * _t2 + 7l)), rcNbInputDeps = 0ul + 1ul
                 , i = 8l * _t2; i <= _t3; i++, rcNbInputDeps++) {
         }
      } else {
         rcNbInputDeps = 0ul;
      }
   } else {
      rcNbInputDeps = 0ul;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars129_count_17(long* args)
{
   long _t1;
   unsigned long rcNbInputDeps;
   long _t2;
   long i;
   long _t3;
   long i_1;
   long _t4;
   long i_2;
   long _t5;
   long i_3;
   long _t6;
   long i_4;
   long _t7;
   long i_5;
   long i_6;
   for (_t1 = (long)args[0l],
        _t2 = (__mins_64(_t1 >> 3l, 17l)),
        i = (__maxs_64(0l, _t1 + -7l + 7l >> 3l)),
        rcNbInputDeps = 4ul;
        i <= _t2; i++, rcNbInputDeps++) {
   }
   for (_t3 = (__mins_64(_t1 >> 3l, 17l)), i_1 = (__maxs_64(0l, _t1 + -7l + 7l
           >> 3l)); i_1 <= _t3; i_1++, rcNbInputDeps++) {
   }
   for (_t4 = (__mins_64(_t1 >> 3l, 17l)), i_2 = (__maxs_64(0l, _t1 + -7l + 7l
           >> 3l)); i_2 <= _t4; i_2++, rcNbInputDeps++) {
   }
   for (_t5 = (__mins_64(_t1 >> 3l, 17l)), i_3 = (__maxs_64(0l, _t1 + -7l + 7l
           >> 3l)); i_3 <= _t5; i_3++, rcNbInputDeps++) {
   }
   for (_t6 = (__mins_64(_t1 >> 3l, 17l)), i_4 = (__maxs_64(0l, _t1 + -7l + 7l
           >> 3l)); i_4 <= _t6; i_4++, rcNbInputDeps++) {
   }
   for (_t7 = (__mins_64(_t1 >> 3l, 17l)), i_5 = (__maxs_64(0l, _t1 + -7l + 7l
           >> 3l)); i_5 <= _t7; i_5++, rcNbInputDeps++) {
   }
   for (i_6 = 0l; i_6 <= 17l; i_6++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars163_count_33(long* args)
{
   long _t1;
   long _t2;
   unsigned long rcNbInputDeps;
   _t2 = (long)args[1l];
   _t1 = (long)args[0l];
   if ((long)args[2l] == 0l) {
      if (_t1 == 0l) {
         long _t3;
         long i;
         for (_t3 = (__mins_64(137l, 8l * _t2 + 7l)), rcNbInputDeps = 0ul + 1ul
                 , i = 8l * _t2; i <= _t3; i++, rcNbInputDeps++) {
         }
      } else {
         rcNbInputDeps = 0ul;
      }
   } else {
      rcNbInputDeps = 0ul;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars159_count_29(long* args)
{
   unsigned long rcNbInputDeps;
   long _t1;
   long _t2;
   long i;
   for (_t1 = (long)args[0l],
        _t2 = (__mins_64(137l, 8l * _t1 + 7l)),
        rcNbInputDeps = 0ul + 1ul,
        i = 8l * _t1;
        i <= _t2; i++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars156_count_28(long* args)
{
   long _t1;
   unsigned long rcNbInputDeps;
   _t1 = (long)args[1l];
   if ((long)args[0l] == 0l) {
      long _t2;
      long i;
      long _t3;
      long i_1;
      for (_t2 = (__mins_64(137l, 8l * _t1 + 7l)), rcNbInputDeps = 0ul + 1ul, i
              = 8l * _t1; i <= _t2; i++, rcNbInputDeps++) {
      }
      for (_t3 = (__mins_64(_t1 >> 1l, 8l)), i_1 = (__maxs_64(0l, _t1 + -1l + 1l
              >> 1l)); i_1 <= _t3; i_1++, rcNbInputDeps++) {
      }
   } else {
      rcNbInputDeps = 0ul;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars151_count_27(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 1ul, i = 0l; i <= 137l; i++, rcNbInputDeps
           ++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars149_count_26(long* args)
{
   long _t1;
   unsigned long rcNbInputDeps;
   _t1 = (long)args[1l];
   if ((long)args[0l] == 0l) {
      long _t2;
      long i;
      for (_t2 = (__mins_64(137l, 8l * _t1 + 7l)), rcNbInputDeps = 0ul + 1ul, i
              = 8l * _t1; i <= _t2; i++, rcNbInputDeps++) {
      }
   } else {
      rcNbInputDeps = 0ul;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars146_count_25(long* args)
{
   unsigned long rcNbInputDeps;
   long _t1;
   long _t2;
   long i;
   for (_t1 = (long)args[0l],
        _t2 = (__mins_64(137l, 8l * _t1 + 7l)),
        rcNbInputDeps = 0ul + 1ul,
        i = 8l * _t1;
        i <= _t2; i++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars141_count_24(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 1ul, i = 0l; i <= 137l; i++, rcNbInputDeps
           ++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars136_count_22(long* args)
{
   unsigned long rcNbInputDeps;
   if ((long)args[0l] == 0l) {
      long i;
      long i_1;
      for (rcNbInputDeps = 0ul + 1ul, i = 0l; i <= 17l; i++, rcNbInputDeps++) {
      }
      for (i_1 = 0l; i_1 <= 137l; i_1++, rcNbInputDeps++) {
      }
   } else {
      rcNbInputDeps = 0ul;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars132_count_18(long* args)
{
   long _t1;
   unsigned long rcNbInputDeps;
   unsigned long rcNbInputDeps_1;
   long _t2;
   unsigned long _t3;
   long i;
   long _t4;
   long i_1;
   for (_t1 = (long)args[0l],
        _t2 = 2l * _t1 + 1l,
        _t3 = 0ul + 1ul,
        rcNbInputDeps_1 = _t3 + 3ul,
        i = 2l * _t1,
        rcNbInputDeps = _t3 + 2ul;
        i <= _t2; i++, rcNbInputDeps_1++, rcNbInputDeps++) {
   }
   for (_t4 = (__mins_64(137l, 16l * _t1 + 15l)), i_1 = 16l * _t1; i_1 <= _t4;
           i_1++, rcNbInputDeps++, rcNbInputDeps_1++) {
   }
   if (_t1 >= 1l) {
      rcNbInputDeps = rcNbInputDeps_1;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars179_count_31(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 1ul, i = 0l; i <= 8l; i++, rcNbInputDeps++)
       {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars178_count_30(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 2ul, i = 0l; i <= 8l; i++, rcNbInputDeps++)
       {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars134_count_19(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 1ul, i = 0l; i <= 8l; i++, rcNbInputDeps++)
       {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars161_count_32(long* args)
{
   return 0ul + 1ul;
}

unsigned long mapped_step1_noGVars124_count_15(long* args)
{
   return 0ul + 1ul + 1ul + 1ul + 1ul + 1ul + 1ul;
}

unsigned long mapped_step1_noGVars175_count_21(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 1ul, i = 0l; i <= 17l; i++, rcNbInputDeps++
      ) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step1_noGVars174_count_20(long* args)
{
   return 0ul + 1ul + 1ul;
}

unsigned long mapped_step1_noGVars126_count_16(long* args)
{
   return 0ul + 1ul + 1ul + 1ul + 1ul;
}

unsigned long mapped_step1_noGVars139_count_23(long* args)
{
   return 0ul + 1ul + 1ul;
}

static ocrGuid_t mapped_step1_noGVars_main(unsigned int paramc, unsigned long*
   paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step1_noGVars_main_118 mapped_step1_noGVars_main_args;
   union __args_mapped_step1_noGVars92_50* allArgs;
   union __args_mapped_step1_noGVars95_52* allArgs_1;
   union __args_mapped_step1_noGVars169_54* allArgs_2;
   union __args_mapped_step1_noGVars97_56* allArgs_3;
   union __args_mapped_step1_noGVars99_58* allArgs_4;
   union __args_mapped_step1_noGVars102_60* allArgs_5;
   union __args_mapped_step1_noGVars170_62* allArgs_6;
   union __args_mapped_step1_noGVars109_64* allArgs_7;
   union __args_mapped_step1_noGVars112_66* allArgs_8;
   union __args_mapped_step1_noGVars117_68* allArgs_9;
   union __args_mapped_step1_noGVars119_70* allArgs_10;
   union __args_mapped_step1_noGVars122_72* allArgs_11;
   union __args_mapped_step1_noGVars172_74* allArgs_12;
   union __args_mapped_step1_noGVars173_76* allArgs_13;
   void* _t1;
   double*restrict* r_l;
   double* Ap_l;
   double* p_l;
   double* normr_30;
   int (*restrict* colInds_l)[27];
   double* b_l;
   int*restrict* nnzPerRow_l;
   double* x_25;
   int const* nPerLvl_l;
   int* _t2;
   double* _t3;
   int* _t4;
   int* _t5;
   int* _t6;
   ocrGuid_t _t7;
   union __args_mapped_step1_noGVars169_54* _t8;
   ocrGuid_t _t9;
   union __args_mapped_step1_noGVars97_56* _t10;
   ocrGuid_t _t11;
   union __args_mapped_step1_noGVars170_62* _t12;
   ocrGuid_t _t13;
   union __args_mapped_step1_noGVars117_68* _t14;
   ocrGuid_t _t15;
   union __args_mapped_step1_noGVars122_72* _t16;
   ocrGuid_t _t17;
   union __args_mapped_step1_noGVars172_74* _t18;
   ocrGuid_t _t19;
   union __args_mapped_step1_noGVars173_76* _t20;
   double (*restrict* A_l)[27];
   void* _t21;
   void* _t22;
   void* _t23;
   void* _t24;
   long i;
   long i_1;
   long i_2;
   long i_3;
   long i_4;
   long i_5;
   long i_6;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        mapped_step1_noGVars_main_args = *(union
           __args_mapped_step1_noGVars_main_118*)rocrArgs(depv),
        _t21 = malloc(4ul),
        _t22 = malloc(8ul),
        _t23 = malloc(4ul),
        _t24 = malloc(4ul),
        _t6 = (int*)malloc(4ul),
        _t5 = (int*)_t24,
        _t4 = (int*)_t23,
        _t3 = (double*)_t22,
        _t2 = (int*)_t21,
        nPerLvl_l = mapped_step1_noGVars_main_args.data.nPerLvl_l,
        x_25 = mapped_step1_noGVars_main_args.data.x,
        nnzPerRow_l = mapped_step1_noGVars_main_args.data.nnzPerRow_l,
        b_l = mapped_step1_noGVars_main_args.data.b_l,
        colInds_l = mapped_step1_noGVars_main_args.data.colInds_l,
        normr_30 = mapped_step1_noGVars_main_args.data.normr,
        p_l = mapped_step1_noGVars_main_args.data.p_l,
        Ap_l = mapped_step1_noGVars_main_args.data.Ap_l,
        r_l = mapped_step1_noGVars_main_args.data.r_l,
        A_l = mapped_step1_noGVars_main_args.data.A_l,
        i = 0l;
        i <= 17l; i++) {
      ocrGuid_t _t25;
      union __args_mapped_step1_noGVars92_50* _t26;
      _t25 = rocrAlloc((void**)&allArgs, 64ul);
      _t26 = allArgs;
      _t26->data.A_l = A_l;
      _t26->data.r_l = r_l;
      _t26->data.Ap_l = Ap_l;
      _t26->data.p_l = p_l;
      _t26->data.normr = normr_30;
      _t26->data.IT0 = 0l;
      _t26->data.IT1 = i;
      _t26->data.IT2 = 0l;
      rocrAutoDec(_t1, 1u, (unsigned long)i, _t25, (unsigned long (*)(long*))((
         void*)0l), mapped_step1_noGVars92_dtenum_1, (void (*)(void*, long*))((
         void*)0l), 3u, (long)0l, (long)i, (long)0l);
   }
   for (i_1 = 0l; i_1 <= 17l; i_1++) {
      ocrGuid_t _t27;
      union __args_mapped_step1_noGVars95_52* _t28;
      _t27 = rocrAlloc((void**)&allArgs_1, 64ul);
      _t28 = allArgs_1;
      _t28->data.colInds_l = colInds_l;
      _t28->data.r_l = r_l;
      _t28->data.Ap_l = Ap_l;
      _t28->data.p_l = p_l;
      _t28->data.normr = normr_30;
      _t28->data.IT0 = 0l;
      _t28->data.IT1 = i_1;
      _t28->data.IT2 = 0l;
      rocrAutoDec(_t1, 2u, (unsigned long)i_1, _t27, (unsigned long (*)(long*))
         ((void*)0l), mapped_step1_noGVars95_dtenum_2, (void (*)(void*, long*))
         ((void*)0l), 3u, (long)0l, (long)i_1, (long)0l);
   }
   _t7 = rocrAlloc((void**)&allArgs_2, 40ul);
   _t8 = allArgs_2;
   _t8->data._p = _t2;
   _t8->data.r_l = r_l;
   _t8->data.Ap_l = Ap_l;
   _t8->data.p_l = p_l;
   _t8->data.normr = normr_30;
   rocrAutoDec(_t1, 3u, (unsigned long)0l, _t7, (unsigned long (*)(long*))((
      void*)0l), mapped_step1_noGVars169_dtenum_3, (void (*)(void*, long*))((
      void*)0l), 0u);
   _t9 = rocrAlloc((void**)&allArgs_3, 40ul);
   _t10 = allArgs_3;
   _t10->data._p = _t3;
   _t10->data.r_l = r_l;
   _t10->data.Ap_l = Ap_l;
   _t10->data.p_l = p_l;
   _t10->data.normr = normr_30;
   rocrAutoDec(_t1, 4u, (unsigned long)0l, _t9, (unsigned long (*)(long*))((
      void*)0l), mapped_step1_noGVars97_dtenum_4, (void (*)(void*, long*))((
      void*)0l), 0u);
   for (i_2 = 0l; i_2 <= 17l; i_2++) {
      ocrGuid_t _t29;
      union __args_mapped_step1_noGVars99_58* _t30;
      _t29 = rocrAlloc((void**)&allArgs_4, 48ul);
      _t30 = allArgs_4;
      _t30->data.b_l = b_l;
      _t30->data.r_l = r_l;
      _t30->data.Ap_l = Ap_l;
      _t30->data.p_l = p_l;
      _t30->data.normr = normr_30;
      _t30->data.IT0 = i_2;
      rocrAutoDec(_t1, 5u, (unsigned long)i_2, _t29, (unsigned long (*)(long*))
         ((void*)0l), mapped_step1_noGVars99_dtenum_5, (void (*)(void*, long*))
         ((void*)0l), 1u, (long)i_2);
   }
   for (i_3 = 0l; i_3 <= 17l; i_3++) {
      ocrGuid_t _t31;
      union __args_mapped_step1_noGVars102_60* _t32;
      _t31 = rocrAlloc((void**)&allArgs_5, 48ul);
      _t32 = allArgs_5;
      _t32->data.r_l = r_l;
      _t32->data.Ap_l = Ap_l;
      _t32->data.p_l = p_l;
      _t32->data.normr = normr_30;
      _t32->data.IT0 = 0l;
      _t32->data.IT1 = i_3;
      rocrAutoDec(_t1, 6u, (unsigned long)i_3, _t31, (unsigned long (*)(long*))
         ((void*)0l), mapped_step1_noGVars102_dtenum_6, (void (*)(void*, long*)
         )((void*)0l), 2u, (long)0l, (long)i_3);
   }
   _t11 = rocrAlloc((void**)&allArgs_6, 40ul);
   _t12 = allArgs_6;
   _t12->data._p = _t4;
   _t12->data.r_l = r_l;
   _t12->data.Ap_l = Ap_l;
   _t12->data.p_l = p_l;
   _t12->data.normr = normr_30;
   rocrAutoDec(_t1, 7u, (unsigned long)0l, _t11, (unsigned long (*)(long*))((
      void*)0l), mapped_step1_noGVars170_dtenum_7, (void (*)(void*, long*))((
      void*)0l), 0u);
   for (i_4 = 0l; i_4 <= 17l; i_4++) {
      ocrGuid_t _t33;
      union __args_mapped_step1_noGVars109_64* _t34;
      _t33 = rocrAlloc((void**)&allArgs_7, 56ul);
      _t34 = allArgs_7;
      _t34->data.nnzPerRow_l = nnzPerRow_l;
      _t34->data.r_l = r_l;
      _t34->data.Ap_l = Ap_l;
      _t34->data.p_l = p_l;
      _t34->data.normr = normr_30;
      _t34->data.IT0 = 0l;
      _t34->data.IT1 = i_4;
      rocrAutoDec(_t1, 8u, (unsigned long)i_4, _t33, (unsigned long (*)(long*))
         ((void*)0l), mapped_step1_noGVars109_dtenum_8, (void (*)(void*, long*)
         )((void*)0l), 2u, (long)0l, (long)i_4);
   }
   for (i_5 = 0l; i_5 <= 17l; i_5++) {
      ocrGuid_t _t35;
      union __args_mapped_step1_noGVars112_66* _t36;
      _t35 = rocrAlloc((void**)&allArgs_8, 40ul);
      _t36 = allArgs_8;
      _t36->data.Ap_l = Ap_l;
      _t36->data.r_l = r_l;
      _t36->data.p_l = p_l;
      _t36->data.normr = normr_30;
      _t36->data.IT0 = i_5;
      rocrAutoDec(_t1, 9u, (unsigned long)i_5, _t35, (unsigned long (*)(long*))
         ((void*)0l), mapped_step1_noGVars112_dtenum_9, (void (*)(void*, long*)
         )((void*)0l), 1u, (long)i_5);
   }
   _t13 = rocrAlloc((void**)&allArgs_9, 40ul);
   _t14 = allArgs_9;
   _t14->data._p = _t5;
   _t14->data.r_l = r_l;
   _t14->data.Ap_l = Ap_l;
   _t14->data.p_l = p_l;
   _t14->data.normr = normr_30;
   rocrAutoDec(_t1, 10u, (unsigned long)0l, _t13, (unsigned long (*)(long*))((
      void*)0l), mapped_step1_noGVars117_dtenum_10, (void (*)(void*, long*))((
      void*)0l), 0u);
   for (i_6 = 0l; i_6 <= 17l; i_6++) {
      ocrGuid_t _t37;
      union __args_mapped_step1_noGVars119_70* _t38;
      _t37 = rocrAlloc((void**)&allArgs_10, 48ul);
      _t38 = allArgs_10;
      _t38->data.x = x_25;
      _t38->data.r_l = r_l;
      _t38->data.Ap_l = Ap_l;
      _t38->data.p_l = p_l;
      _t38->data.normr = normr_30;
      _t38->data.IT0 = i_6;
      rocrAutoDec(_t1, 11u, (unsigned long)i_6, _t37, (unsigned long (*)(long*)
         )((void*)0l), mapped_step1_noGVars119_dtenum_11, (void (*)(void*, long
         *))((void*)0l), 1u, (long)i_6);
   }
   _t15 = rocrAlloc((void**)&allArgs_11, 40ul);
   _t16 = allArgs_11;
   _t16->data.p_l = p_l;
   _t16->data.r_l = r_l;
   _t16->data.Ap_l = Ap_l;
   _t16->data.normr = normr_30;
   _t16->data.IT0 = 0l;
   rocrAutoDec(_t1, 12u, (unsigned long)0l, _t15, (unsigned long (*)(long*))((
      void*)0l), mapped_step1_noGVars122_dtenum_12, (void (*)(void*, long*))((
      void*)0l), 1u, (long)0l);
   _t17 = rocrAlloc((void**)&allArgs_12, 40ul);
   _t18 = allArgs_12;
   _t18->data._p = _t6;
   _t18->data.r_l = r_l;
   _t18->data.Ap_l = Ap_l;
   _t18->data.p_l = p_l;
   _t18->data.normr = normr_30;
   rocrAutoDec(_t1, 13u, (unsigned long)0l, _t17, (unsigned long (*)(long*))((
      void*)0l), mapped_step1_noGVars172_dtenum_13, (void (*)(void*, long*))((
      void*)0l), 0u);
   _t19 = rocrAlloc((void**)&allArgs_13, 40ul);
   _t20 = allArgs_13;
   _t20->data.nPerLvl_l = nPerLvl_l;
   _t20->data.r_l = r_l;
   _t20->data.Ap_l = Ap_l;
   _t20->data.p_l = p_l;
   _t20->data.normr = normr_30;
   rocrAutoDec(_t1, 14u, (unsigned long)0l, _t19, (unsigned long (*)(long*))((
      void*)0l), mapped_step1_noGVars173_dtenum_14, (void (*)(void*, long*))((
      void*)0l), 0u);
   rocrAsyncFree(_t2);
   rocrAsyncFree(_t3);
   rocrAsyncFree(_t4);
   rocrAsyncFree(_t5);
   rocrAsyncFree(_t6);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step1_noGVars_async(void* event, double* x_26, double* normr_31,
   int const* nPerLvl_l, double* p_l, double*restrict* r_l, double (*restrict*
   A_l)[27], double* Ap_l, int*restrict* nnzPerRow_l, int (*restrict* colInds_l
   )[27], double* b_l)
{
   union __args_mapped_step1_noGVars_main_118* allArgs;
   ocrGuid_t _t1;
   union __args_mapped_step1_noGVars_main_118* _t2;
   void* _t3;
   _t3 = rocrInit(35u, 15u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, mapped_step1_noGVars_main, 0u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars92, 1u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars95, 2u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars169, 3u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars97, 4u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars99, 5u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars102, 6u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars170, 7u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars109, 8u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars112, 9u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars117, 10u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars119, 11u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars122, 12u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars172, 13u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars173, 14u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars124, 15u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars126, 16u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars129, 17u, (unsigned long)138l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars132, 18u, (unsigned long)9l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars134, 19u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars174, 20u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars175, 21u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars136, 22u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars139, 23u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars141, 24u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars146, 25u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars149, 26u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars151, 27u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars156, 28u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars159, 29u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars178, 30u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars179, 31u, (unsigned long)1l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars161, 32u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step1_noGVars163, 33u, (unsigned long)18l, 0u);
   rocrDeclareType(_t3, mapped_step1_noGVars166, 34u, (unsigned long)18l, 0u);
   rocrDeclareDBType(_t3, 0u, 0u);
   rocrDeclareDBType(_t3, 1u, 0u);
   rocrDeclareDBType(_t3, 2u, 1u, 1124864ul, (unsigned long)(__divceils_64(
      1124864l, (long)1124864ul)));
   rocrDeclareDBType(_t3, 3u, 1u, 65536ul, (unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 4u, 0u);
   rocrDeclareDBType(_t3, 5u, 1u, 65536ul, (unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 6u, 2u, 1ul, 65536ul, (unsigned long)(1l >> 0l), (
      unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 7u, 0u);
   rocrDeclareDBType(_t3, 8u, 2u, 1ul, 65536ul, (unsigned long)(1l >> 0l), (
      unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 9u, 1u, 65536ul, (unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 10u, 0u);
   rocrDeclareDBType(_t3, 11u, 0u);
   rocrDeclareDBType(_t3, 12u, 0u);
   rocrDeclareDBType(_t3, 13u, 3u, 1ul, 65536ul, 27ul, (unsigned long)(1l >> 0l
      ), (unsigned long)(1190399l >> 16l), (unsigned long)(__divceils_64(27l, (
      long)27ul)));
   rocrDeclareDBType(_t3, 14u, 3u, 1ul, 65536ul, 27ul, (unsigned long)(1l >> 0l
      ), (unsigned long)(1190399l >> 16l), (unsigned long)(__divceils_64(27l, (
      long)27ul)));
   _t1 = rocrAlloc((void**)&allArgs, 80ul);
   _t2 = allArgs;
   _t2->data.A_l = A_l;
   _t2->data.r_l = r_l;
   _t2->data.Ap_l = Ap_l;
   _t2->data.p_l = p_l;
   _t2->data.normr = normr_31;
   _t2->data.colInds_l = colInds_l;
   _t2->data.b_l = b_l;
   _t2->data.nnzPerRow_l = nnzPerRow_l;
   _t2->data.x = x_26;
   _t2->data.nPerLvl_l = nPerLvl_l;
   rocrExecute(_t3, mapped_step1_noGVars_main, _t1, event);
   rocrExit(_t3);
}

void mapped_step5_dpbr_noGVars(double* alpha, double* rtz, double* pAp, double*
    x_27, double* normr_32, double* p_l, double (*restrict* A_l)[27], double*
   Ap_l, int const* nPerLvl_l, int*restrict* nnzPerRow_l, int (*restrict*
   colInds_l)[27], double*restrict* r_l)
{
   ocrGuid_t outEvt;
   union __args_mapped_step5_dpbr_noGVars_main_216* allArgs;
   ocrGuid_t _t1;
   union __args_mapped_step5_dpbr_noGVars_main_216* _t2;
   void* _t3;
   ocrEventCreate(&outEvt, OCR_EVENT_STICKY_T, (int)0l);
   _t3 = rocrInit(49u, 21u, ROCR_FLAG_REDUCED_EDT_CREATE);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars_main, 0u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars124, 1u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars127, 2u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars195, 3u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars129, 4u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars131, 5u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars197, 6u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars133, 7u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars135, 8u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars199, 9u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars200, 10u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars201, 11u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars202, 12u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars137, 13u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars139, 14u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars204, 15u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars205, 16u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars143, 17u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars146, 18u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars206, 19u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars207, 20u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars151, 21u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars153, 22u, (unsigned long)138l,
       0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars156, 23u, (unsigned long)18l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars158, 24u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars160, 25u, (unsigned long)35l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars163, 26u, (unsigned long)9l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars165, 27u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars208, 28u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars209, 29u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars170, 30u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars173, 31u, (unsigned long)18l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars211, 32u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars212, 33u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars177, 34u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars213, 35u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars214, 36u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars215, 37u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars216, 38u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars217, 39u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars181, 40u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars218, 41u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars219, 42u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars185, 43u, (unsigned long)18l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars220, 44u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars221, 45u, (unsigned long)1l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars187, 46u, (unsigned long)1l,
      ROCR_EDTTP_ONMASTER);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars189, 47u, (unsigned long)18l,
      0u);
   rocrDeclareType(_t3, mapped_step5_dpbr_noGVars192, 48u, (unsigned long)18l,
      0u);
   rocrDeclareDBType(_t3, 0u, 0u);
   rocrDeclareDBType(_t3, 1u, 0u);
   rocrDeclareDBType(_t3, 2u, 1u, 65536ul, (unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 3u, 2u, 1ul, 65536ul, (unsigned long)(1l >> 0l), (
      unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 4u, 0u);
   rocrDeclareDBType(_t3, 5u, 0u);
   rocrDeclareDBType(_t3, 6u, 1u, 1124864ul, (unsigned long)(__divceils_64(
      1124864l, (long)1124864ul)));
   rocrDeclareDBType(_t3, 7u, 0u);
   rocrDeclareDBType(_t3, 8u, 0u);
   rocrDeclareDBType(_t3, 9u, 0u);
   rocrDeclareDBType(_t3, 10u, 0u);
   rocrDeclareDBType(_t3, 11u, 0u);
   rocrDeclareDBType(_t3, 12u, 1u, 65536ul, (unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 13u, 0u);
   rocrDeclareDBType(_t3, 14u, 0u);
   rocrDeclareDBType(_t3, 15u, 2u, 1ul, 65536ul, (unsigned long)(1l >> 0l), (
      unsigned long)(1190399l >> 16l));
   rocrDeclareDBType(_t3, 16u, 0u);
   rocrDeclareDBType(_t3, 17u, 0u);
   rocrDeclareDBType(_t3, 18u, 0u);
   rocrDeclareDBType(_t3, 19u, 3u, 1ul, 65536ul, 27ul, (unsigned long)(1l >> 0l
      ), (unsigned long)(1190399l >> 16l), (unsigned long)(__divceils_64(27l, (
      long)27ul)));
   rocrDeclareDBType(_t3, 20u, 3u, 1ul, 65536ul, 27ul, (unsigned long)(1l >> 0l
      ), (unsigned long)(1190399l >> 16l), (unsigned long)(__divceils_64(27l, (
      long)27ul)));
   _t1 = rocrAlloc((void**)&allArgs, 96ul);
   _t2 = allArgs;
   _t2->data.A_l = A_l;
   _t2->data.Ap_l = Ap_l;
   _t2->data.r_l = r_l;
   _t2->data.x = x_27;
   _t2->data.normr = normr_32;
   _t2->data.alpha = alpha;
   _t2->data.pAp = pAp;
   _t2->data.colInds_l = colInds_l;
   _t2->data.rtz = rtz;
   _t2->data.p_l = p_l;
   _t2->data.nnzPerRow_l = nnzPerRow_l;
   _t2->data.nPerLvl_l = nPerLvl_l;
   rocrExecute(_t3, mapped_step5_dpbr_noGVars_main, _t1, &outEvt);
   rocrExit(_t3);
   ocrWait(outEvt);
   ocrEventDestroy(outEvt);
}

static ocrGuid_t mapped_step5_dpbr_noGVars124(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars124_120
      mapped_step5_dpbr_noGVars124_args;
   double* A_l_db_0;
   union __args_mapped_step5_dpbr_noGVars192_214* allArgs;
   union __args_mapped_step5_dpbr_noGVars153_162* allArgs_1;
   void* _t1;
   double (*restrict* A_l)[27];
   double* Ap_l;
   double*restrict* r_l;
   double* x_28;
   double* normr_33;
   double* alpha;
   double* pAp;
   long IT0;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars124_args = *(union
      __args_mapped_step5_dpbr_noGVars124_120*)rocrArgs(depv);
   IT1 = mapped_step5_dpbr_noGVars124_args.data.IT1;
   IT0 = mapped_step5_dpbr_noGVars124_args.data.IT0;
   pAp = mapped_step5_dpbr_noGVars124_args.data.pAp;
   alpha = mapped_step5_dpbr_noGVars124_args.data.alpha;
   normr_33 = mapped_step5_dpbr_noGVars124_args.data.normr;
   x_28 = mapped_step5_dpbr_noGVars124_args.data.x;
   r_l = mapped_step5_dpbr_noGVars124_args.data.r_l;
   Ap_l = mapped_step5_dpbr_noGVars124_args.data.Ap_l;
   A_l = mapped_step5_dpbr_noGVars124_args.data.A_l;
   if (mapped_step5_dpbr_noGVars124_args.data.IT2 == 0l && IT0 == 0l) {
      long _t2;
      ocrGuid_t _t3;
      union __args_mapped_step5_dpbr_noGVars192_214* _t4;
      long _t5;
      long i;
      long i_1;
      long i_2;
      _t2 = (__mins_64(-65536l * IT1 + 1124863l, 65535l));
      rocrFetchDB(_t1, (void**)&A_l_db_0, 4, 20ul, (unsigned long)0l, (
         unsigned long)IT1, (unsigned long)0l);
      for (i = 0l, i_1 = 0l; i <= _t2; i++, i_1 += 27) {
         long j;
         for (j = 0l; j <= 26l; j++) {
            A_l_db_0[1769472l * IT1 + j + i_1 + -27l * (__divfloors_64(j, 27l))
               + -1769472l * (65536l * IT1 + i >> 16l)] = A_l[0l][i + 65536l *
               IT1][j];
         }
      }
      _t3 = rocrAlloc((void**)&allArgs, 24ul);
      _t4 = allArgs;
      _t4->data.IT0 = 0l;
      _t4->data.IT1 = IT1;
      _t4->data.IT2 = 0l;
      _t5 = (__mins_64(137l, 8l * IT1 + 7l));
      rocrAutoDec(_t1, 48u, (unsigned long)IT1, _t3,
         mapped_step5_dpbr_noGVars192_count_48,
         mapped_step5_dpbr_noGVars192_dtenum_48, (void (*)(void*, long*))((void
         *)0l), 3u, (long)0l, (long)IT1, (long)0l);
      for (i_2 = 8l * IT1; i_2 <= _t5; i_2++) {
         ocrGuid_t _t6;
         union __args_mapped_step5_dpbr_noGVars153_162* _t7;
         _t6 = rocrAlloc((void**)&allArgs_1, 56ul);
         _t7 = allArgs_1;
         _t7->data.Ap_l = Ap_l;
         _t7->data.r_l = r_l;
         _t7->data.x = x_28;
         _t7->data.normr = normr_33;
         _t7->data.alpha = alpha;
         _t7->data.pAp = pAp;
         _t7->data.IT0 = i_2;
         rocrAutoDec(_t1, 22u, (unsigned long)i_2, _t6,
            mapped_step5_dpbr_noGVars153_count_22,
            mapped_step5_dpbr_noGVars153_dtenum_22, (void (*)(void*, long*))((
            void*)0l), 1u, (long)i_2);
      }
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars124_dtenum_1(void* rocrEDTCtx, long* args)
{
   long _t1;
   long _t2;
   _t2 = (long)args[1l];
   _t1 = (long)args[0l];
   if ((long)args[2l] == 0l && _t1 == 0l) {
      if (_t2 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 20u, 14155776ul, 2l, 1u, (void (*)(void*, long*
            ))((void*)0l), (unsigned long)0l, (unsigned long)_t2, (
            unsigned long)0l);
      }
      if (_t2 == 17l) {
         rocrEnumDB(rocrEDTCtx, 20u, 14155776ul, 1l, 1u, (void (*)(void*, long*
            ))((void*)0l), (unsigned long)0l, (unsigned long)17l, (
            unsigned long)0l);
      }
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars127(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars127_122
      mapped_step5_dpbr_noGVars127_args;
   int* colInds_l_db_0;
   union __args_mapped_step5_dpbr_noGVars189_212* allArgs;
   union __args_mapped_step5_dpbr_noGVars153_162* allArgs_1;
   void* _t1;
   int (*restrict* colInds_l)[27];
   double* Ap_l;
   double*restrict* r_l;
   double* x_29;
   double* normr_34;
   double* alpha;
   double* pAp;
   long IT0;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars127_args = *(union
      __args_mapped_step5_dpbr_noGVars127_122*)rocrArgs(depv);
   IT1 = mapped_step5_dpbr_noGVars127_args.data.IT1;
   IT0 = mapped_step5_dpbr_noGVars127_args.data.IT0;
   pAp = mapped_step5_dpbr_noGVars127_args.data.pAp;
   alpha = mapped_step5_dpbr_noGVars127_args.data.alpha;
   normr_34 = mapped_step5_dpbr_noGVars127_args.data.normr;
   x_29 = mapped_step5_dpbr_noGVars127_args.data.x;
   r_l = mapped_step5_dpbr_noGVars127_args.data.r_l;
   Ap_l = mapped_step5_dpbr_noGVars127_args.data.Ap_l;
   colInds_l = mapped_step5_dpbr_noGVars127_args.data.colInds_l;
   if (mapped_step5_dpbr_noGVars127_args.data.IT2 == 0l && IT0 == 0l) {
      long _t2;
      ocrGuid_t _t3;
      union __args_mapped_step5_dpbr_noGVars189_212* _t4;
      long _t5;
      long i;
      long i_1;
      long i_2;
      _t2 = (__mins_64(-65536l * IT1 + 1124863l, 65535l));
      rocrFetchDB(_t1, (void**)&colInds_l_db_0, 4, 19ul, (unsigned long)0l, (
         unsigned long)IT1, (unsigned long)0l);
      for (i = 0l, i_1 = 0l; i <= _t2; i++, i_1 += 27) {
         long j;
         for (j = 0l; j <= 26l; j++) {
            colInds_l_db_0[1769472l * IT1 + j + i_1 + -27l * (__divfloors_64(j,
                27l)) + -1769472l * (65536l * IT1 + i >> 16l)] = colInds_l[0l][
               i + 65536l * IT1][j];
         }
      }
      _t3 = rocrAlloc((void**)&allArgs, 24ul);
      _t4 = allArgs;
      _t4->data.IT0 = 0l;
      _t4->data.IT1 = IT1;
      _t4->data.IT2 = 0l;
      _t5 = (__mins_64(137l, 8l * IT1 + 7l));
      rocrAutoDec(_t1, 47u, (unsigned long)IT1, _t3,
         mapped_step5_dpbr_noGVars189_count_47,
         mapped_step5_dpbr_noGVars189_dtenum_47, (void (*)(void*, long*))((void
         *)0l), 3u, (long)0l, (long)IT1, (long)0l);
      for (i_2 = 8l * IT1; i_2 <= _t5; i_2++) {
         ocrGuid_t _t6;
         union __args_mapped_step5_dpbr_noGVars153_162* _t7;
         _t6 = rocrAlloc((void**)&allArgs_1, 56ul);
         _t7 = allArgs_1;
         _t7->data.Ap_l = Ap_l;
         _t7->data.r_l = r_l;
         _t7->data.x = x_29;
         _t7->data.normr = normr_34;
         _t7->data.alpha = alpha;
         _t7->data.pAp = pAp;
         _t7->data.IT0 = i_2;
         rocrAutoDec(_t1, 22u, (unsigned long)i_2, _t6,
            mapped_step5_dpbr_noGVars153_count_22,
            mapped_step5_dpbr_noGVars153_dtenum_22, (void (*)(void*, long*))((
            void*)0l), 1u, (long)i_2);
      }
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars127_dtenum_2(void* rocrEDTCtx, long* args)
{
   long _t1;
   long _t2;
   _t2 = (long)args[1l];
   _t1 = (long)args[0l];
   if ((long)args[2l] == 0l && _t1 == 0l) {
      if (_t2 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 19u, 7077888ul, 2l, 1u, (void (*)(void*, long*)
            )((void*)0l), (unsigned long)0l, (unsigned long)_t2, (unsigned long
            )0l);
      }
      if (_t2 == 17l) {
         rocrEnumDB(rocrEDTCtx, 19u, 7077888ul, 1l, 1u, (void (*)(void*, long*)
            )((void*)0l), (unsigned long)0l, (unsigned long)17l, (unsigned long
            )0l);
      }
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars195(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars195_124
      mapped_step5_dpbr_noGVars195_args;
   int* _v6_db_0;
   union __args_mapped_step5_dpbr_noGVars221_208* allArgs;
   union __args_mapped_step5_dpbr_noGVars163_170* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars151_160* allArgs_2;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_30;
   double* normr_35;
   double* alpha;
   double* pAp;
   ocrGuid_t _t2;
   union __args_mapped_step5_dpbr_noGVars151_160* _t3;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars195_args = *(union
      __args_mapped_step5_dpbr_noGVars195_124*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&_v6_db_0, 1, 17ul);
   *_v6_db_0 = *mapped_step5_dpbr_noGVars195_args.data._p;
   rocrAutoDec(_t1, 45u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step5_dpbr_noGVars221_count_45,
      mapped_step5_dpbr_noGVars221_dtenum_45, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (pAp = mapped_step5_dpbr_noGVars195_args.data.pAp,
        alpha = mapped_step5_dpbr_noGVars195_args.data.alpha,
        normr_35 = mapped_step5_dpbr_noGVars195_args.data.normr,
        x_30 = mapped_step5_dpbr_noGVars195_args.data.x,
        r_l = mapped_step5_dpbr_noGVars195_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars195_args.data.Ap_l,
        i = 0l;
        i <= 8l; i++) {
      ocrGuid_t _t4;
      union __args_mapped_step5_dpbr_noGVars163_170* _t5;
      _t4 = rocrAlloc((void**)&allArgs_1, 24ul);
      _t5 = allArgs_1;
      _t5->data.r_l = r_l;
      _t5->data.normr = normr_35;
      _t5->data.IT0 = i;
      rocrAutoDec(_t1, 26u, (unsigned long)i, _t4,
         mapped_step5_dpbr_noGVars163_count_26,
         mapped_step5_dpbr_noGVars163_dtenum_26, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i);
   }
   _t2 = rocrAlloc((void**)&allArgs_2, 48ul);
   _t3 = allArgs_2;
   _t3->data.Ap_l = Ap_l;
   _t3->data.r_l = r_l;
   _t3->data.x = x_30;
   _t3->data.normr = normr_35;
   _t3->data.alpha = alpha;
   _t3->data.pAp = pAp;
   rocrAutoDec(_t1, 21u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars151_count_21,
      mapped_step5_dpbr_noGVars151_dtenum_21, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars195_dtenum_3(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 17u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step5_dpbr_noGVars129(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars129_126
      mapped_step5_dpbr_noGVars129_args;
   double* local_result1_db_0;
   union __args_mapped_step5_dpbr_noGVars220_206* allArgs;
   union __args_mapped_step5_dpbr_noGVars165_172* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars163_170* allArgs_2;
   union __args_mapped_step5_dpbr_noGVars151_160* allArgs_3;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_31;
   double* normr_36;
   double* alpha;
   double* pAp;
   ocrGuid_t _t2;
   ocrGuid_t _t3;
   union __args_mapped_step5_dpbr_noGVars151_160* _t4;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars129_args = *(union
      __args_mapped_step5_dpbr_noGVars129_126*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&local_result1_db_0, 1, 16ul);
   *local_result1_db_0 = *mapped_step5_dpbr_noGVars129_args.data._p;
   rocrAutoDec(_t1, 44u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step5_dpbr_noGVars220_count_44,
      mapped_step5_dpbr_noGVars220_dtenum_44, (void (*)(void*, long*))((void*)
      0l), 0u);
   _t2 = rocrAlloc((void**)&allArgs_1, 8ul);
   normr_36 = mapped_step5_dpbr_noGVars129_args.data.normr;
   allArgs_1->data.normr = normr_36;
   rocrAutoDec(_t1, 27u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars165_count_27,
      mapped_step5_dpbr_noGVars165_dtenum_27, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (pAp = mapped_step5_dpbr_noGVars129_args.data.pAp,
        alpha = mapped_step5_dpbr_noGVars129_args.data.alpha,
        x_31 = mapped_step5_dpbr_noGVars129_args.data.x,
        r_l = mapped_step5_dpbr_noGVars129_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars129_args.data.Ap_l,
        i = 0l;
        i <= 8l; i++) {
      ocrGuid_t _t5;
      union __args_mapped_step5_dpbr_noGVars163_170* _t6;
      _t5 = rocrAlloc((void**)&allArgs_2, 24ul);
      _t6 = allArgs_2;
      _t6->data.r_l = r_l;
      _t6->data.normr = normr_36;
      _t6->data.IT0 = i;
      rocrAutoDec(_t1, 26u, (unsigned long)i, _t5,
         mapped_step5_dpbr_noGVars163_count_26,
         mapped_step5_dpbr_noGVars163_dtenum_26, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i);
   }
   _t3 = rocrAlloc((void**)&allArgs_3, 48ul);
   _t4 = allArgs_3;
   _t4->data.Ap_l = Ap_l;
   _t4->data.r_l = r_l;
   _t4->data.x = x_31;
   _t4->data.normr = normr_36;
   _t4->data.alpha = alpha;
   _t4->data.pAp = pAp;
   rocrAutoDec(_t1, 21u, (unsigned long)0l, _t3,
      mapped_step5_dpbr_noGVars151_count_21,
      mapped_step5_dpbr_noGVars151_dtenum_21, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars129_dtenum_4(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 16u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step5_dpbr_noGVars131(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars131_128
      mapped_step5_dpbr_noGVars131_args;
   double* r_l_db_0;
   union __args_mapped_step5_dpbr_noGVars185_204* allArgs;
   union __args_mapped_step5_dpbr_noGVars163_170* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars160_168* allArgs_2;
   void* _t1;
   double*restrict* r_l;
   double* Ap_l;
   double* x_32;
   double* normr_37;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars131_args = *(union
      __args_mapped_step5_dpbr_noGVars131_128*)rocrArgs(depv);
   IT1 = mapped_step5_dpbr_noGVars131_args.data.IT1;
   normr_37 = mapped_step5_dpbr_noGVars131_args.data.normr;
   x_32 = mapped_step5_dpbr_noGVars131_args.data.x;
   Ap_l = mapped_step5_dpbr_noGVars131_args.data.Ap_l;
   r_l = mapped_step5_dpbr_noGVars131_args.data.r_l;
   if (mapped_step5_dpbr_noGVars131_args.data.IT0 == 0l) {
      long _t2;
      ocrGuid_t _t3;
      union __args_mapped_step5_dpbr_noGVars185_204* _t4;
      long _t5;
      long i;
      long i_1;
      long _t6;
      long i_2;
      _t2 = (__mins_64(-65536l * IT1 + 1124863l, 65535l));
      rocrFetchDB(_t1, (void**)&r_l_db_0, 3, 15ul, (unsigned long)0l, (
         unsigned long)IT1);
      for (i = 0l; i <= _t2; i++) {
         r_l_db_0[65536l * IT1 + i + -65536l * (65536l * IT1 + i >> 16l)] = r_l
            [0l][i + 65536l * IT1];
      }
      _t3 = rocrAlloc((void**)&allArgs, 24ul);
      _t4 = allArgs;
      _t4->data.r_l = r_l;
      _t4->data.IT0 = 0l;
      _t4->data.IT1 = IT1;
      _t5 = (__mins_64(IT1 >> 1l, 8l));
      rocrAutoDec(_t1, 43u, (unsigned long)IT1, _t3,
         mapped_step5_dpbr_noGVars185_count_43,
         mapped_step5_dpbr_noGVars185_dtenum_43, (void (*)(void*, long*))((void
         *)0l), 2u, (long)0l, (long)IT1);
      for (i_1 = (__maxs_64(0l, IT1 + -1l + 1l >> 1l)); i_1 <= _t5; i_1++) {
         ocrGuid_t _t7;
         union __args_mapped_step5_dpbr_noGVars163_170* _t8;
         _t7 = rocrAlloc((void**)&allArgs_1, 24ul);
         _t8 = allArgs_1;
         _t8->data.r_l = r_l;
         _t8->data.normr = normr_37;
         _t8->data.IT0 = i_1;
         rocrAutoDec(_t1, 26u, (unsigned long)i_1, _t7,
            mapped_step5_dpbr_noGVars163_count_26,
            mapped_step5_dpbr_noGVars163_dtenum_26, (void (*)(void*, long*))((
            void*)0l), 1u, (long)i_1);
      }
      for (_t6 = (__mins_64(34l, 2l * IT1 + 1l)), i_2 = 2l * IT1; i_2 <= _t6;
              i_2++) {
         ocrGuid_t _t9;
         union __args_mapped_step5_dpbr_noGVars160_168* _t10;
         _t9 = rocrAlloc((void**)&allArgs_2, 40ul);
         _t10 = allArgs_2;
         _t10->data.Ap_l = Ap_l;
         _t10->data.r_l = r_l;
         _t10->data.x = x_32;
         _t10->data.normr = normr_37;
         _t10->data.IT0 = i_2;
         rocrAutoDec(_t1, 25u, (unsigned long)i_2, _t9,
            mapped_step5_dpbr_noGVars160_count_25,
            mapped_step5_dpbr_noGVars160_dtenum_25, (void (*)(void*, long*))((
            void*)0l), 1u, (long)i_2);
      }
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars131_dtenum_5(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[1l];
   if ((long)args[0l] == 0l) {
      if (_t1 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 15u, 524288ul, 2l, 1u, (void (*)(void*, long*))
            ((void*)0l), (unsigned long)0l, (unsigned long)_t1);
      }
      if (_t1 == 17l) {
         rocrEnumDB(rocrEDTCtx, 15u, 524288ul, 1l, 1u, (void (*)(void*, long*))
            ((void*)0l), (unsigned long)0l, (unsigned long)17l);
      }
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars197(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars197_130
      mapped_step5_dpbr_noGVars197_args;
   int* _v7_db_0;
   union __args_mapped_step5_dpbr_noGVars219_202* allArgs;
   union __args_mapped_step5_dpbr_noGVars160_168* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars151_160* allArgs_2;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_33;
   double* normr_38;
   double* alpha;
   double* pAp;
   ocrGuid_t _t2;
   union __args_mapped_step5_dpbr_noGVars151_160* _t3;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars197_args = *(union
      __args_mapped_step5_dpbr_noGVars197_130*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&_v7_db_0, 1, 14ul);
   *_v7_db_0 = *mapped_step5_dpbr_noGVars197_args.data._p;
   rocrAutoDec(_t1, 42u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step5_dpbr_noGVars219_count_42,
      mapped_step5_dpbr_noGVars219_dtenum_42, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (pAp = mapped_step5_dpbr_noGVars197_args.data.pAp,
        alpha = mapped_step5_dpbr_noGVars197_args.data.alpha,
        normr_38 = mapped_step5_dpbr_noGVars197_args.data.normr,
        x_33 = mapped_step5_dpbr_noGVars197_args.data.x,
        r_l = mapped_step5_dpbr_noGVars197_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars197_args.data.Ap_l,
        i = 0l;
        i <= 34l; i++) {
      ocrGuid_t _t4;
      union __args_mapped_step5_dpbr_noGVars160_168* _t5;
      _t4 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t5 = allArgs_1;
      _t5->data.Ap_l = Ap_l;
      _t5->data.r_l = r_l;
      _t5->data.x = x_33;
      _t5->data.normr = normr_38;
      _t5->data.IT0 = i;
      rocrAutoDec(_t1, 25u, (unsigned long)i, _t4,
         mapped_step5_dpbr_noGVars160_count_25,
         mapped_step5_dpbr_noGVars160_dtenum_25, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i);
   }
   _t2 = rocrAlloc((void**)&allArgs_2, 48ul);
   _t3 = allArgs_2;
   _t3->data.Ap_l = Ap_l;
   _t3->data.r_l = r_l;
   _t3->data.x = x_33;
   _t3->data.normr = normr_38;
   _t3->data.alpha = alpha;
   _t3->data.pAp = pAp;
   rocrAutoDec(_t1, 21u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars151_count_21,
      mapped_step5_dpbr_noGVars151_dtenum_21, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars197_dtenum_6(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 14u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step5_dpbr_noGVars133(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars133_132
      mapped_step5_dpbr_noGVars133_args;
   double* _v2_db_0;
   union __args_mapped_step5_dpbr_noGVars218_200* allArgs;
   union __args_mapped_step5_dpbr_noGVars160_168* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars158_166* allArgs_2;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_34;
   double* normr_39;
   double* alpha;
   double* pAp;
   ocrGuid_t _t2;
   union __args_mapped_step5_dpbr_noGVars158_166* _t3;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars133_args = *(union
      __args_mapped_step5_dpbr_noGVars133_132*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&_v2_db_0, 1, 13ul);
   *_v2_db_0 = *mapped_step5_dpbr_noGVars133_args.data._p;
   rocrAutoDec(_t1, 41u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step5_dpbr_noGVars218_count_41,
      mapped_step5_dpbr_noGVars218_dtenum_41, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (pAp = mapped_step5_dpbr_noGVars133_args.data.pAp,
        alpha = mapped_step5_dpbr_noGVars133_args.data.alpha,
        normr_39 = mapped_step5_dpbr_noGVars133_args.data.normr,
        x_34 = mapped_step5_dpbr_noGVars133_args.data.x,
        r_l = mapped_step5_dpbr_noGVars133_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars133_args.data.Ap_l,
        i = 0l;
        i <= 34l; i++) {
      ocrGuid_t _t4;
      union __args_mapped_step5_dpbr_noGVars160_168* _t5;
      _t4 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t5 = allArgs_1;
      _t5->data.Ap_l = Ap_l;
      _t5->data.r_l = r_l;
      _t5->data.x = x_34;
      _t5->data.normr = normr_39;
      _t5->data.IT0 = i;
      rocrAutoDec(_t1, 25u, (unsigned long)i, _t4,
         mapped_step5_dpbr_noGVars160_count_25,
         mapped_step5_dpbr_noGVars160_dtenum_25, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i);
   }
   _t2 = rocrAlloc((void**)&allArgs_2, 48ul);
   _t3 = allArgs_2;
   _t3->data.Ap_l = Ap_l;
   _t3->data.r_l = r_l;
   _t3->data.x = x_34;
   _t3->data.normr = normr_39;
   _t3->data.alpha = alpha;
   _t3->data.pAp = pAp;
   rocrAutoDec(_t1, 24u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars158_count_24,
      mapped_step5_dpbr_noGVars158_dtenum_24, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars133_dtenum_7(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 13u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step5_dpbr_noGVars135(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars135_134
      mapped_step5_dpbr_noGVars135_args;
   double* x_db_0;
   union __args_mapped_step5_dpbr_noGVars181_198* allArgs;
   union __args_mapped_step5_dpbr_noGVars160_168* allArgs_1;
   void* _t1;
   double* x_35;
   double* Ap_l;
   double*restrict* r_l;
   double* normr_40;
   long IT0;
   long _t2;
   ocrGuid_t _t3;
   union __args_mapped_step5_dpbr_noGVars181_198* _t4;
   long _t5;
   long i;
   long i_1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars135_args = *(union
      __args_mapped_step5_dpbr_noGVars135_134*)rocrArgs(depv);
   IT0 = mapped_step5_dpbr_noGVars135_args.data.IT0;
   _t2 = (__mins_64(-65536l * IT0 + 1124863l, 65535l));
   rocrFetchDB(_t1, (void**)&x_db_0, 2, 12ul, (unsigned long)IT0);
   for (normr_40 = mapped_step5_dpbr_noGVars135_args.data.normr,
        r_l = mapped_step5_dpbr_noGVars135_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars135_args.data.Ap_l,
        x_35 = mapped_step5_dpbr_noGVars135_args.data.x,
        i = 0l;
        i <= _t2; i++) {
      x_db_0[65536l * IT0 + i + -65536l * (65536l * IT0 + i >> 16l)] = x_35[i +
         65536l * IT0];
   }
   _t3 = rocrAlloc((void**)&allArgs, 16ul);
   _t4 = allArgs;
   _t4->data.x = x_35;
   _t4->data.IT0 = IT0;
   _t5 = (__mins_64(34l, 2l * IT0 + 1l));
   rocrAutoDec(_t1, 40u, (unsigned long)IT0, _t3,
      mapped_step5_dpbr_noGVars181_count_40,
      mapped_step5_dpbr_noGVars181_dtenum_40, (void (*)(void*, long*))((void*)
      0l), 1u, (long)IT0);
   for (i_1 = 2l * IT0; i_1 <= _t5; i_1++) {
      ocrGuid_t _t6;
      union __args_mapped_step5_dpbr_noGVars160_168* _t7;
      _t6 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t7 = allArgs_1;
      _t7->data.Ap_l = Ap_l;
      _t7->data.r_l = r_l;
      _t7->data.x = x_35;
      _t7->data.normr = normr_40;
      _t7->data.IT0 = i_1;
      rocrAutoDec(_t1, 25u, (unsigned long)i_1, _t6,
         mapped_step5_dpbr_noGVars160_count_25,
         mapped_step5_dpbr_noGVars160_dtenum_25, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i_1);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars135_dtenum_8(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[0l];
   if (_t1 <= 16l) {
      rocrEnumDB(rocrEDTCtx, 12u, 524288ul, 2l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)_t1);
   }
   if (_t1 == 17l) {
      rocrEnumDB(rocrEDTCtx, 12u, 524288ul, 1l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)17l);
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars199(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars199_136
      mapped_step5_dpbr_noGVars199_args;
   int* _v5_db_0;
   union __args_mapped_step5_dpbr_noGVars217_196* allArgs;
   union __args_mapped_step5_dpbr_noGVars160_168* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars151_160* allArgs_2;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_36;
   double* normr_41;
   double* alpha;
   double* pAp;
   ocrGuid_t _t2;
   union __args_mapped_step5_dpbr_noGVars151_160* _t3;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars199_args = *(union
      __args_mapped_step5_dpbr_noGVars199_136*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&_v5_db_0, 1, 11ul);
   *_v5_db_0 = *mapped_step5_dpbr_noGVars199_args.data._p;
   rocrAutoDec(_t1, 39u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step5_dpbr_noGVars217_count_39,
      mapped_step5_dpbr_noGVars217_dtenum_39, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (pAp = mapped_step5_dpbr_noGVars199_args.data.pAp,
        alpha = mapped_step5_dpbr_noGVars199_args.data.alpha,
        normr_41 = mapped_step5_dpbr_noGVars199_args.data.normr,
        x_36 = mapped_step5_dpbr_noGVars199_args.data.x,
        r_l = mapped_step5_dpbr_noGVars199_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars199_args.data.Ap_l,
        i = 0l;
        i <= 34l; i++) {
      ocrGuid_t _t4;
      union __args_mapped_step5_dpbr_noGVars160_168* _t5;
      _t4 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t5 = allArgs_1;
      _t5->data.Ap_l = Ap_l;
      _t5->data.r_l = r_l;
      _t5->data.x = x_36;
      _t5->data.normr = normr_41;
      _t5->data.IT0 = i;
      rocrAutoDec(_t1, 25u, (unsigned long)i, _t4,
         mapped_step5_dpbr_noGVars160_count_25,
         mapped_step5_dpbr_noGVars160_dtenum_25, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i);
   }
   _t2 = rocrAlloc((void**)&allArgs_2, 48ul);
   _t3 = allArgs_2;
   _t3->data.Ap_l = Ap_l;
   _t3->data.r_l = r_l;
   _t3->data.x = x_36;
   _t3->data.normr = normr_41;
   _t3->data.alpha = alpha;
   _t3->data.pAp = pAp;
   rocrAutoDec(_t1, 21u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars151_count_21,
      mapped_step5_dpbr_noGVars151_dtenum_21, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars199_dtenum_9(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 11u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step5_dpbr_noGVars200(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars200_138
      mapped_step5_dpbr_noGVars200_args;
   double* _v1_db_0;
   union __args_mapped_step5_dpbr_noGVars216_194* allArgs;
   union __args_mapped_step5_dpbr_noGVars160_168* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars158_166* allArgs_2;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_37;
   double* normr_42;
   double* alpha;
   double* pAp;
   ocrGuid_t _t2;
   union __args_mapped_step5_dpbr_noGVars158_166* _t3;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars200_args = *(union
      __args_mapped_step5_dpbr_noGVars200_138*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&_v1_db_0, 1, 10ul);
   *_v1_db_0 = *mapped_step5_dpbr_noGVars200_args.data._p;
   rocrAutoDec(_t1, 38u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step5_dpbr_noGVars216_count_38,
      mapped_step5_dpbr_noGVars216_dtenum_38, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (pAp = mapped_step5_dpbr_noGVars200_args.data.pAp,
        alpha = mapped_step5_dpbr_noGVars200_args.data.alpha,
        normr_42 = mapped_step5_dpbr_noGVars200_args.data.normr,
        x_37 = mapped_step5_dpbr_noGVars200_args.data.x,
        r_l = mapped_step5_dpbr_noGVars200_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars200_args.data.Ap_l,
        i = 0l;
        i <= 34l; i++) {
      ocrGuid_t _t4;
      union __args_mapped_step5_dpbr_noGVars160_168* _t5;
      _t4 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t5 = allArgs_1;
      _t5->data.Ap_l = Ap_l;
      _t5->data.r_l = r_l;
      _t5->data.x = x_37;
      _t5->data.normr = normr_42;
      _t5->data.IT0 = i;
      rocrAutoDec(_t1, 25u, (unsigned long)i, _t4,
         mapped_step5_dpbr_noGVars160_count_25,
         mapped_step5_dpbr_noGVars160_dtenum_25, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i);
   }
   _t2 = rocrAlloc((void**)&allArgs_2, 48ul);
   _t3 = allArgs_2;
   _t3->data.Ap_l = Ap_l;
   _t3->data.r_l = r_l;
   _t3->data.x = x_37;
   _t3->data.normr = normr_42;
   _t3->data.alpha = alpha;
   _t3->data.pAp = pAp;
   rocrAutoDec(_t1, 24u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars158_count_24,
      mapped_step5_dpbr_noGVars158_dtenum_24, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars200_dtenum_10(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 10u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step5_dpbr_noGVars201(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars201_140
      mapped_step5_dpbr_noGVars201_args;
   double* rtz_db_0;
   union __args_mapped_step5_dpbr_noGVars215_192* allArgs;
   union __args_mapped_step5_dpbr_noGVars158_166* allArgs_1;
   void* _t1;
   ocrGuid_t _t2;
   union __args_mapped_step5_dpbr_noGVars158_166* _t3;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars201_args = *(union
      __args_mapped_step5_dpbr_noGVars201_140*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&rtz_db_0, 1, 9ul);
   *rtz_db_0 = *mapped_step5_dpbr_noGVars201_args.data.rtz;
   rocrAutoDec(_t1, 37u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step5_dpbr_noGVars215_count_37,
      mapped_step5_dpbr_noGVars215_dtenum_37, (void (*)(void*, long*))((void*)
      0l), 0u);
   _t2 = rocrAlloc((void**)&allArgs_1, 48ul);
   _t3 = allArgs_1;
   _t3->data.Ap_l = mapped_step5_dpbr_noGVars201_args.data.Ap_l;
   _t3->data.r_l = mapped_step5_dpbr_noGVars201_args.data.r_l;
   _t3->data.x = mapped_step5_dpbr_noGVars201_args.data.x;
   _t3->data.normr = mapped_step5_dpbr_noGVars201_args.data.normr;
   _t3->data.alpha = mapped_step5_dpbr_noGVars201_args.data.alpha;
   _t3->data.pAp = mapped_step5_dpbr_noGVars201_args.data.pAp;
   rocrAutoDec(_t1, 24u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars158_count_24,
      mapped_step5_dpbr_noGVars158_dtenum_24, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars201_dtenum_11(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 9u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars202(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars202_142
      mapped_step5_dpbr_noGVars202_args;
   double* alpha_db_0;
   union __args_mapped_step5_dpbr_noGVars214_190* allArgs;
   union __args_mapped_step5_dpbr_noGVars158_166* allArgs_1;
   void* _t1;
   double* alpha;
   ocrGuid_t _t2;
   ocrGuid_t _t3;
   union __args_mapped_step5_dpbr_noGVars158_166* _t4;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars202_args = *(union
      __args_mapped_step5_dpbr_noGVars202_142*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&alpha_db_0, 1, 8ul);
   alpha = mapped_step5_dpbr_noGVars202_args.data.alpha;
   *alpha_db_0 = *alpha;
   _t2 = rocrAlloc((void**)&allArgs, 8ul);
   allArgs->data.alpha = alpha;
   rocrAutoDec(_t1, 36u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars214_count_36,
      mapped_step5_dpbr_noGVars214_dtenum_36, (void (*)(void*, long*))((void*)
      0l), 0u);
   _t3 = rocrAlloc((void**)&allArgs_1, 48ul);
   _t4 = allArgs_1;
   _t4->data.Ap_l = mapped_step5_dpbr_noGVars202_args.data.Ap_l;
   _t4->data.r_l = mapped_step5_dpbr_noGVars202_args.data.r_l;
   _t4->data.x = mapped_step5_dpbr_noGVars202_args.data.x;
   _t4->data.normr = mapped_step5_dpbr_noGVars202_args.data.normr;
   _t4->data.alpha = alpha;
   _t4->data.pAp = mapped_step5_dpbr_noGVars202_args.data.pAp;
   rocrAutoDec(_t1, 24u, (unsigned long)0l, _t3,
      mapped_step5_dpbr_noGVars158_count_24,
      mapped_step5_dpbr_noGVars158_dtenum_24, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars202_dtenum_12(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 8u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars137(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars137_144
      mapped_step5_dpbr_noGVars137_args;
   double* pAp_db_0;
   union __args_mapped_step5_dpbr_noGVars213_188* allArgs;
   union __args_mapped_step5_dpbr_noGVars158_166* allArgs_1;
   void* _t1;
   double* pAp;
   ocrGuid_t _t2;
   ocrGuid_t _t3;
   union __args_mapped_step5_dpbr_noGVars158_166* _t4;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars137_args = *(union
      __args_mapped_step5_dpbr_noGVars137_144*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&pAp_db_0, 1, 7ul);
   pAp = mapped_step5_dpbr_noGVars137_args.data.pAp;
   *pAp_db_0 = *pAp;
   _t2 = rocrAlloc((void**)&allArgs, 8ul);
   allArgs->data.pAp = pAp;
   rocrAutoDec(_t1, 35u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars213_count_35,
      mapped_step5_dpbr_noGVars213_dtenum_35, (void (*)(void*, long*))((void*)
      0l), 0u);
   _t3 = rocrAlloc((void**)&allArgs_1, 48ul);
   _t4 = allArgs_1;
   _t4->data.Ap_l = mapped_step5_dpbr_noGVars137_args.data.Ap_l;
   _t4->data.r_l = mapped_step5_dpbr_noGVars137_args.data.r_l;
   _t4->data.x = mapped_step5_dpbr_noGVars137_args.data.x;
   _t4->data.normr = mapped_step5_dpbr_noGVars137_args.data.normr;
   _t4->data.alpha = mapped_step5_dpbr_noGVars137_args.data.alpha;
   _t4->data.pAp = pAp;
   rocrAutoDec(_t1, 24u, (unsigned long)0l, _t3,
      mapped_step5_dpbr_noGVars158_count_24,
      mapped_step5_dpbr_noGVars158_dtenum_24, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars137_dtenum_13(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 7u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars139(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars139_146
      mapped_step5_dpbr_noGVars139_args;
   double* p_l_db_0;
   union __args_mapped_step5_dpbr_noGVars177_186* allArgs;
   union __args_mapped_step5_dpbr_noGVars160_168* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars156_164* allArgs_2;
   union __args_mapped_step5_dpbr_noGVars153_162* allArgs_3;
   void* _t1;
   double* p_l;
   double* Ap_l;
   double*restrict* r_l;
   double* x_38;
   double* normr_43;
   double* alpha;
   double* pAp;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars139_args = *(union
      __args_mapped_step5_dpbr_noGVars139_146*)rocrArgs(depv);
   pAp = mapped_step5_dpbr_noGVars139_args.data.pAp;
   alpha = mapped_step5_dpbr_noGVars139_args.data.alpha;
   normr_43 = mapped_step5_dpbr_noGVars139_args.data.normr;
   x_38 = mapped_step5_dpbr_noGVars139_args.data.x;
   r_l = mapped_step5_dpbr_noGVars139_args.data.r_l;
   Ap_l = mapped_step5_dpbr_noGVars139_args.data.Ap_l;
   p_l = mapped_step5_dpbr_noGVars139_args.data.p_l;
   if (mapped_step5_dpbr_noGVars139_args.data.IT0 == 0l) {
      ocrGuid_t _t2;
      long i;
      long i_1;
      long i_2;
      long i_3;
      rocrFetchDB(_t1, (void**)&p_l_db_0, 2, 6ul, (unsigned long)0l);
      for (i = 0l; i <= 1124863l; i++) {
         p_l_db_0[i + -1124864l * (__divfloors_64(i, 1124864l))] = p_l[i];
      }
      _t2 = rocrAlloc((void**)&allArgs, 8ul);
      allArgs->data.IT0 = 0l;
      rocrAutoDec(_t1, 34u, (unsigned long)0l, _t2,
         mapped_step5_dpbr_noGVars177_count_34,
         mapped_step5_dpbr_noGVars177_dtenum_34, (void (*)(void*, long*))((void
         *)0l), 1u, (long)0l);
      for (i_1 = 0l; i_1 <= 34l; i_1++) {
         ocrGuid_t _t3;
         union __args_mapped_step5_dpbr_noGVars160_168* _t4;
         _t3 = rocrAlloc((void**)&allArgs_1, 40ul);
         _t4 = allArgs_1;
         _t4->data.Ap_l = Ap_l;
         _t4->data.r_l = r_l;
         _t4->data.x = x_38;
         _t4->data.normr = normr_43;
         _t4->data.IT0 = i_1;
         rocrAutoDec(_t1, 25u, (unsigned long)i_1, _t3,
            mapped_step5_dpbr_noGVars160_count_25,
            mapped_step5_dpbr_noGVars160_dtenum_25, (void (*)(void*, long*))((
            void*)0l), 1u, (long)i_1);
      }
      for (i_2 = 0l; i_2 <= 17l; i_2++) {
         ocrGuid_t _t5;
         union __args_mapped_step5_dpbr_noGVars156_164* _t6;
         _t5 = rocrAlloc((void**)&allArgs_2, 56ul);
         _t6 = allArgs_2;
         _t6->data.Ap_l = Ap_l;
         _t6->data.r_l = r_l;
         _t6->data.x = x_38;
         _t6->data.normr = normr_43;
         _t6->data.alpha = alpha;
         _t6->data.pAp = pAp;
         _t6->data.IT0 = i_2;
         rocrAutoDec(_t1, 23u, (unsigned long)i_2, _t5,
            mapped_step5_dpbr_noGVars156_count_23,
            mapped_step5_dpbr_noGVars156_dtenum_23, (void (*)(void*, long*))((
            void*)0l), 1u, (long)i_2);
      }
      for (i_3 = 0l; i_3 <= 137l; i_3++) {
         ocrGuid_t _t7;
         union __args_mapped_step5_dpbr_noGVars153_162* _t8;
         _t7 = rocrAlloc((void**)&allArgs_3, 56ul);
         _t8 = allArgs_3;
         _t8->data.Ap_l = Ap_l;
         _t8->data.r_l = r_l;
         _t8->data.x = x_38;
         _t8->data.normr = normr_43;
         _t8->data.alpha = alpha;
         _t8->data.pAp = pAp;
         _t8->data.IT0 = i_3;
         rocrAutoDec(_t1, 22u, (unsigned long)i_3, _t7,
            mapped_step5_dpbr_noGVars153_count_22,
            mapped_step5_dpbr_noGVars153_dtenum_22, (void (*)(void*, long*))((
            void*)0l), 1u, (long)i_3);
      }
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars139_dtenum_14(void* rocrEDTCtx, long* args)
{
   if ((long)args[0l] == 0l) {
      rocrEnumDB(rocrEDTCtx, 6u, 8998912ul, 2l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)0l);
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars204(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars204_148
      mapped_step5_dpbr_noGVars204_args;
   int* _v4_db_0;
   union __args_mapped_step5_dpbr_noGVars212_184* allArgs;
   union __args_mapped_step5_dpbr_noGVars156_164* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars151_160* allArgs_2;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_39;
   double* normr_44;
   double* alpha;
   double* pAp;
   ocrGuid_t _t2;
   union __args_mapped_step5_dpbr_noGVars151_160* _t3;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars204_args = *(union
      __args_mapped_step5_dpbr_noGVars204_148*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&_v4_db_0, 1, 5ul);
   *_v4_db_0 = *mapped_step5_dpbr_noGVars204_args.data._p;
   rocrAutoDec(_t1, 33u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step5_dpbr_noGVars212_count_33,
      mapped_step5_dpbr_noGVars212_dtenum_33, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (pAp = mapped_step5_dpbr_noGVars204_args.data.pAp,
        alpha = mapped_step5_dpbr_noGVars204_args.data.alpha,
        normr_44 = mapped_step5_dpbr_noGVars204_args.data.normr,
        x_39 = mapped_step5_dpbr_noGVars204_args.data.x,
        r_l = mapped_step5_dpbr_noGVars204_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars204_args.data.Ap_l,
        i = 0l;
        i <= 17l; i++) {
      ocrGuid_t _t4;
      union __args_mapped_step5_dpbr_noGVars156_164* _t5;
      _t4 = rocrAlloc((void**)&allArgs_1, 56ul);
      _t5 = allArgs_1;
      _t5->data.Ap_l = Ap_l;
      _t5->data.r_l = r_l;
      _t5->data.x = x_39;
      _t5->data.normr = normr_44;
      _t5->data.alpha = alpha;
      _t5->data.pAp = pAp;
      _t5->data.IT0 = i;
      rocrAutoDec(_t1, 23u, (unsigned long)i, _t4,
         mapped_step5_dpbr_noGVars156_count_23,
         mapped_step5_dpbr_noGVars156_dtenum_23, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i);
   }
   _t2 = rocrAlloc((void**)&allArgs_2, 48ul);
   _t3 = allArgs_2;
   _t3->data.Ap_l = Ap_l;
   _t3->data.r_l = r_l;
   _t3->data.x = x_39;
   _t3->data.normr = normr_44;
   _t3->data.alpha = alpha;
   _t3->data.pAp = pAp;
   rocrAutoDec(_t1, 21u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars151_count_21,
      mapped_step5_dpbr_noGVars151_dtenum_21, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars204_dtenum_15(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 5u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars205(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars205_150
      mapped_step5_dpbr_noGVars205_args;
   double* local_result_db_0;
   union __args_mapped_step5_dpbr_noGVars211_182* allArgs;
   union __args_mapped_step5_dpbr_noGVars158_166* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars156_164* allArgs_2;
   union __args_mapped_step5_dpbr_noGVars151_160* allArgs_3;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_40;
   double* normr_45;
   double* alpha;
   double* pAp;
   ocrGuid_t _t2;
   union __args_mapped_step5_dpbr_noGVars158_166* _t3;
   ocrGuid_t _t4;
   union __args_mapped_step5_dpbr_noGVars151_160* _t5;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars205_args = *(union
      __args_mapped_step5_dpbr_noGVars205_150*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&local_result_db_0, 1, 4ul);
   *local_result_db_0 = *mapped_step5_dpbr_noGVars205_args.data._p;
   rocrAutoDec(_t1, 32u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step5_dpbr_noGVars211_count_32,
      mapped_step5_dpbr_noGVars211_dtenum_32, (void (*)(void*, long*))((void*)
      0l), 0u);
   _t2 = rocrAlloc((void**)&allArgs_1, 48ul);
   _t3 = allArgs_1;
   Ap_l = mapped_step5_dpbr_noGVars205_args.data.Ap_l;
   _t3->data.Ap_l = Ap_l;
   r_l = mapped_step5_dpbr_noGVars205_args.data.r_l;
   _t3->data.r_l = r_l;
   x_40 = mapped_step5_dpbr_noGVars205_args.data.x;
   _t3->data.x = x_40;
   normr_45 = mapped_step5_dpbr_noGVars205_args.data.normr;
   _t3->data.normr = normr_45;
   alpha = mapped_step5_dpbr_noGVars205_args.data.alpha;
   _t3->data.alpha = alpha;
   pAp = mapped_step5_dpbr_noGVars205_args.data.pAp;
   _t3->data.pAp = pAp;
   rocrAutoDec(_t1, 24u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars158_count_24,
      mapped_step5_dpbr_noGVars158_dtenum_24, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (i = 0l; i <= 17l; i++) {
      ocrGuid_t _t6;
      union __args_mapped_step5_dpbr_noGVars156_164* _t7;
      _t6 = rocrAlloc((void**)&allArgs_2, 56ul);
      _t7 = allArgs_2;
      _t7->data.Ap_l = Ap_l;
      _t7->data.r_l = r_l;
      _t7->data.x = x_40;
      _t7->data.normr = normr_45;
      _t7->data.alpha = alpha;
      _t7->data.pAp = pAp;
      _t7->data.IT0 = i;
      rocrAutoDec(_t1, 23u, (unsigned long)i, _t6,
         mapped_step5_dpbr_noGVars156_count_23,
         mapped_step5_dpbr_noGVars156_dtenum_23, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i);
   }
   _t4 = rocrAlloc((void**)&allArgs_3, 48ul);
   _t5 = allArgs_3;
   _t5->data.Ap_l = Ap_l;
   _t5->data.r_l = r_l;
   _t5->data.x = x_40;
   _t5->data.normr = normr_45;
   _t5->data.alpha = alpha;
   _t5->data.pAp = pAp;
   rocrAutoDec(_t1, 21u, (unsigned long)0l, _t4,
      mapped_step5_dpbr_noGVars151_count_21,
      mapped_step5_dpbr_noGVars151_dtenum_21, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars205_dtenum_16(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 4u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars143(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars143_152
      mapped_step5_dpbr_noGVars143_args;
   int* nnzPerRow_l_db_0;
   union __args_mapped_step5_dpbr_noGVars173_180* allArgs;
   union __args_mapped_step5_dpbr_noGVars153_162* allArgs_1;
   void* _t1;
   int*restrict* nnzPerRow_l;
   double* Ap_l;
   double*restrict* r_l;
   double* x_41;
   double* normr_46;
   double* alpha;
   double* pAp;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars143_args = *(union
      __args_mapped_step5_dpbr_noGVars143_152*)rocrArgs(depv);
   IT1 = mapped_step5_dpbr_noGVars143_args.data.IT1;
   pAp = mapped_step5_dpbr_noGVars143_args.data.pAp;
   alpha = mapped_step5_dpbr_noGVars143_args.data.alpha;
   normr_46 = mapped_step5_dpbr_noGVars143_args.data.normr;
   x_41 = mapped_step5_dpbr_noGVars143_args.data.x;
   r_l = mapped_step5_dpbr_noGVars143_args.data.r_l;
   Ap_l = mapped_step5_dpbr_noGVars143_args.data.Ap_l;
   nnzPerRow_l = mapped_step5_dpbr_noGVars143_args.data.nnzPerRow_l;
   if (mapped_step5_dpbr_noGVars143_args.data.IT0 == 0l) {
      long _t2;
      ocrGuid_t _t3;
      union __args_mapped_step5_dpbr_noGVars173_180* _t4;
      long _t5;
      long i;
      long i_1;
      _t2 = (__mins_64(-65536l * IT1 + 1124863l, 65535l));
      rocrFetchDB(_t1, (void**)&nnzPerRow_l_db_0, 3, 3ul, (unsigned long)0l, (
         unsigned long)IT1);
      for (i = 0l; i <= _t2; i++) {
         nnzPerRow_l_db_0[65536l * IT1 + i + -65536l * (65536l * IT1 + i >> 16l
            )] = nnzPerRow_l[0l][i + 65536l * IT1];
      }
      _t3 = rocrAlloc((void**)&allArgs, 16ul);
      _t4 = allArgs;
      _t4->data.IT0 = 0l;
      _t4->data.IT1 = IT1;
      _t5 = (__mins_64(137l, 8l * IT1 + 7l));
      rocrAutoDec(_t1, 31u, (unsigned long)IT1, _t3,
         mapped_step5_dpbr_noGVars173_count_31,
         mapped_step5_dpbr_noGVars173_dtenum_31, (void (*)(void*, long*))((void
         *)0l), 2u, (long)0l, (long)IT1);
      for (i_1 = 8l * IT1; i_1 <= _t5; i_1++) {
         ocrGuid_t _t6;
         union __args_mapped_step5_dpbr_noGVars153_162* _t7;
         _t6 = rocrAlloc((void**)&allArgs_1, 56ul);
         _t7 = allArgs_1;
         _t7->data.Ap_l = Ap_l;
         _t7->data.r_l = r_l;
         _t7->data.x = x_41;
         _t7->data.normr = normr_46;
         _t7->data.alpha = alpha;
         _t7->data.pAp = pAp;
         _t7->data.IT0 = i_1;
         rocrAutoDec(_t1, 22u, (unsigned long)i_1, _t6,
            mapped_step5_dpbr_noGVars153_count_22,
            mapped_step5_dpbr_noGVars153_dtenum_22, (void (*)(void*, long*))((
            void*)0l), 1u, (long)i_1);
      }
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars143_dtenum_17(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[1l];
   if ((long)args[0l] == 0l) {
      if (_t1 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 3u, 262144ul, 2l, 1u, (void (*)(void*, long*))(
            (void*)0l), (unsigned long)0l, (unsigned long)_t1);
      }
      if (_t1 == 17l) {
         rocrEnumDB(rocrEDTCtx, 3u, 262144ul, 1l, 1u, (void (*)(void*, long*))(
            (void*)0l), (unsigned long)0l, (unsigned long)17l);
      }
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars146(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars146_154
      mapped_step5_dpbr_noGVars146_args;
   double* Ap_l_db_0;
   union __args_mapped_step5_dpbr_noGVars170_178* allArgs;
   union __args_mapped_step5_dpbr_noGVars160_168* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars156_164* allArgs_2;
   union __args_mapped_step5_dpbr_noGVars153_162* allArgs_3;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_42;
   double* normr_47;
   double* alpha;
   double* pAp;
   long IT0;
   long _t2;
   ocrGuid_t _t3;
   union __args_mapped_step5_dpbr_noGVars170_178* _t4;
   long _t5;
   ocrGuid_t _t6;
   union __args_mapped_step5_dpbr_noGVars156_164* _t7;
   long _t8;
   long i;
   long i_1;
   long i_2;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars146_args = *(union
      __args_mapped_step5_dpbr_noGVars146_154*)rocrArgs(depv);
   IT0 = mapped_step5_dpbr_noGVars146_args.data.IT0;
   _t2 = (__mins_64(-65536l * IT0 + 1124863l, 65535l));
   rocrFetchDB(_t1, (void**)&Ap_l_db_0, 2, 2ul, (unsigned long)IT0);
   for (pAp = mapped_step5_dpbr_noGVars146_args.data.pAp,
        alpha = mapped_step5_dpbr_noGVars146_args.data.alpha,
        normr_47 = mapped_step5_dpbr_noGVars146_args.data.normr,
        x_42 = mapped_step5_dpbr_noGVars146_args.data.x,
        r_l = mapped_step5_dpbr_noGVars146_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars146_args.data.Ap_l,
        i = 0l;
        i <= _t2; i++) {
      Ap_l_db_0[65536l * IT0 + i + -65536l * (65536l * IT0 + i >> 16l)] = Ap_l[
         i + 65536l * IT0];
   }
   _t3 = rocrAlloc((void**)&allArgs, 16ul);
   _t4 = allArgs;
   _t4->data.Ap_l = Ap_l;
   _t4->data.IT0 = IT0;
   _t5 = (__mins_64(34l, 2l * IT0 + 1l));
   rocrAutoDec(_t1, 30u, (unsigned long)IT0, _t3,
      mapped_step5_dpbr_noGVars170_count_30,
      mapped_step5_dpbr_noGVars170_dtenum_30, (void (*)(void*, long*))((void*)
      0l), 1u, (long)IT0);
   for (i_1 = 2l * IT0; i_1 <= _t5; i_1++) {
      ocrGuid_t _t9;
      union __args_mapped_step5_dpbr_noGVars160_168* _t10;
      _t9 = rocrAlloc((void**)&allArgs_1, 40ul);
      _t10 = allArgs_1;
      _t10->data.Ap_l = Ap_l;
      _t10->data.r_l = r_l;
      _t10->data.x = x_42;
      _t10->data.normr = normr_47;
      _t10->data.IT0 = i_1;
      rocrAutoDec(_t1, 25u, (unsigned long)i_1, _t9,
         mapped_step5_dpbr_noGVars160_count_25,
         mapped_step5_dpbr_noGVars160_dtenum_25, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i_1);
   }
   _t6 = rocrAlloc((void**)&allArgs_2, 56ul);
   _t7 = allArgs_2;
   _t7->data.Ap_l = Ap_l;
   _t7->data.r_l = r_l;
   _t7->data.x = x_42;
   _t7->data.normr = normr_47;
   _t7->data.alpha = alpha;
   _t7->data.pAp = pAp;
   _t7->data.IT0 = IT0;
   _t8 = (__mins_64(137l, 8l * IT0 + 7l));
   rocrAutoDec(_t1, 23u, (unsigned long)IT0, _t6,
      mapped_step5_dpbr_noGVars156_count_23,
      mapped_step5_dpbr_noGVars156_dtenum_23, (void (*)(void*, long*))((void*)
      0l), 1u, (long)IT0);
   for (i_2 = 8l * IT0; i_2 <= _t8; i_2++) {
      ocrGuid_t _t11;
      union __args_mapped_step5_dpbr_noGVars153_162* _t12;
      _t11 = rocrAlloc((void**)&allArgs_3, 56ul);
      _t12 = allArgs_3;
      _t12->data.Ap_l = Ap_l;
      _t12->data.r_l = r_l;
      _t12->data.x = x_42;
      _t12->data.normr = normr_47;
      _t12->data.alpha = alpha;
      _t12->data.pAp = pAp;
      _t12->data.IT0 = i_2;
      rocrAutoDec(_t1, 22u, (unsigned long)i_2, _t11,
         mapped_step5_dpbr_noGVars153_count_22,
         mapped_step5_dpbr_noGVars153_dtenum_22, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i_2);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars146_dtenum_18(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[0l];
   if (_t1 <= 16l) {
      rocrEnumDB(rocrEDTCtx, 2u, 524288ul, 2l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)_t1);
   }
   if (_t1 == 17l) {
      rocrEnumDB(rocrEDTCtx, 2u, 524288ul, 1l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)17l);
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars206(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars206_156
      mapped_step5_dpbr_noGVars206_args;
   int* _v3_db_0;
   union __args_mapped_step5_dpbr_noGVars209_176* allArgs;
   union __args_mapped_step5_dpbr_noGVars153_162* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars151_160* allArgs_2;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_43;
   double* normr_48;
   double* alpha;
   double* pAp;
   ocrGuid_t _t2;
   union __args_mapped_step5_dpbr_noGVars151_160* _t3;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars206_args = *(union
      __args_mapped_step5_dpbr_noGVars206_156*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&_v3_db_0, 1, 1ul);
   *_v3_db_0 = *mapped_step5_dpbr_noGVars206_args.data._p;
   rocrAutoDec(_t1, 29u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step5_dpbr_noGVars209_count_29,
      mapped_step5_dpbr_noGVars209_dtenum_29, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (pAp = mapped_step5_dpbr_noGVars206_args.data.pAp,
        alpha = mapped_step5_dpbr_noGVars206_args.data.alpha,
        normr_48 = mapped_step5_dpbr_noGVars206_args.data.normr,
        x_43 = mapped_step5_dpbr_noGVars206_args.data.x,
        r_l = mapped_step5_dpbr_noGVars206_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars206_args.data.Ap_l,
        i = 0l;
        i <= 137l; i++) {
      ocrGuid_t _t4;
      union __args_mapped_step5_dpbr_noGVars153_162* _t5;
      _t4 = rocrAlloc((void**)&allArgs_1, 56ul);
      _t5 = allArgs_1;
      _t5->data.Ap_l = Ap_l;
      _t5->data.r_l = r_l;
      _t5->data.x = x_43;
      _t5->data.normr = normr_48;
      _t5->data.alpha = alpha;
      _t5->data.pAp = pAp;
      _t5->data.IT0 = i;
      rocrAutoDec(_t1, 22u, (unsigned long)i, _t4,
         mapped_step5_dpbr_noGVars153_count_22,
         mapped_step5_dpbr_noGVars153_dtenum_22, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i);
   }
   _t2 = rocrAlloc((void**)&allArgs_2, 48ul);
   _t3 = allArgs_2;
   _t3->data.Ap_l = Ap_l;
   _t3->data.r_l = r_l;
   _t3->data.x = x_43;
   _t3->data.normr = normr_48;
   _t3->data.alpha = alpha;
   _t3->data.pAp = pAp;
   rocrAutoDec(_t1, 21u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars151_count_21,
      mapped_step5_dpbr_noGVars151_dtenum_21, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars206_dtenum_19(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 1u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars207(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars207_158
      mapped_step5_dpbr_noGVars207_args;
   int* nPerLvl_l_db_0;
   union __args_mapped_step5_dpbr_noGVars208_174* allArgs;
   union __args_mapped_step5_dpbr_noGVars151_160* allArgs_1;
   void* _t1;
   ocrGuid_t _t2;
   union __args_mapped_step5_dpbr_noGVars151_160* _t3;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars207_args = *(union
      __args_mapped_step5_dpbr_noGVars207_158*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&nPerLvl_l_db_0, 1, 0ul);
   *nPerLvl_l_db_0 = *(int*)mapped_step5_dpbr_noGVars207_args.data.nPerLvl_l;
   rocrAutoDec(_t1, 28u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step5_dpbr_noGVars208_count_28,
      mapped_step5_dpbr_noGVars208_dtenum_28, (void (*)(void*, long*))((void*)
      0l), 0u);
   _t2 = rocrAlloc((void**)&allArgs_1, 48ul);
   _t3 = allArgs_1;
   _t3->data.Ap_l = mapped_step5_dpbr_noGVars207_args.data.Ap_l;
   _t3->data.r_l = mapped_step5_dpbr_noGVars207_args.data.r_l;
   _t3->data.x = mapped_step5_dpbr_noGVars207_args.data.x;
   _t3->data.normr = mapped_step5_dpbr_noGVars207_args.data.normr;
   _t3->data.alpha = mapped_step5_dpbr_noGVars207_args.data.alpha;
   _t3->data.pAp = mapped_step5_dpbr_noGVars207_args.data.pAp;
   rocrAutoDec(_t1, 21u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars151_count_21,
      mapped_step5_dpbr_noGVars151_dtenum_21, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars207_dtenum_20(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 0u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars151(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars151_160
      mapped_step5_dpbr_noGVars151_args;
   int* nPerLvl_l_db_0;
   int* _v3_db_5;
   double* local_result_db_6;
   int* nPerLvl_l_db_1;
   int* _v4_db_7;
   int* nPerLvl_l_db_2;
   int* _v5_db_8;
   int* nPerLvl_l_db_3;
   int* _v7_db_9;
   double* local_result1_db_10;
   int* nPerLvl_l_db_4;
   int* _v6_db_11;
   union __args_mapped_step5_dpbr_noGVars221_208* allArgs;
   union __args_mapped_step5_dpbr_noGVars220_206* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars219_202* allArgs_2;
   union __args_mapped_step5_dpbr_noGVars217_196* allArgs_3;
   union __args_mapped_step5_dpbr_noGVars212_184* allArgs_4;
   union __args_mapped_step5_dpbr_noGVars211_182* allArgs_5;
   union __args_mapped_step5_dpbr_noGVars209_176* allArgs_6;
   union __args_mapped_step5_dpbr_noGVars208_174* allArgs_7;
   union __args_mapped_step5_dpbr_noGVars165_172* allArgs_8;
   union __args_mapped_step5_dpbr_noGVars163_170* allArgs_9;
   union __args_mapped_step5_dpbr_noGVars160_168* allArgs_10;
   union __args_mapped_step5_dpbr_noGVars158_166* allArgs_11;
   union __args_mapped_step5_dpbr_noGVars156_164* allArgs_12;
   union __args_mapped_step5_dpbr_noGVars153_162* allArgs_13;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_44;
   double* normr_49;
   double* alpha;
   double* pAp;
   ocrGuid_t _t2;
   ocrGuid_t _t3;
   union __args_mapped_step5_dpbr_noGVars158_166* _t4;
   long i;
   long i_1;
   long i_2;
   long i_3;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars151_args = *(union
      __args_mapped_step5_dpbr_noGVars151_160*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&nPerLvl_l_db_0, 1, 0ul);
   rocrFetchDB(_t1, (void**)&_v3_db_5, 1, 1ul);
   *_v3_db_5 = *nPerLvl_l_db_0;
   rocrFetchDB(_t1, (void**)&local_result_db_6, 1, 4ul);
   *local_result_db_6 = 0.0;
   rocrFetchDB(_t1, (void**)&nPerLvl_l_db_1, 1, 0ul);
   rocrFetchDB(_t1, (void**)&_v4_db_7, 1, 5ul);
   *_v4_db_7 = *nPerLvl_l_db_1;
   rocrFetchDB(_t1, (void**)&nPerLvl_l_db_2, 1, 0ul);
   rocrFetchDB(_t1, (void**)&_v5_db_8, 1, 11ul);
   *_v5_db_8 = *nPerLvl_l_db_2;
   rocrFetchDB(_t1, (void**)&nPerLvl_l_db_3, 1, 0ul);
   rocrFetchDB(_t1, (void**)&_v7_db_9, 1, 14ul);
   *_v7_db_9 = *nPerLvl_l_db_3;
   rocrFetchDB(_t1, (void**)&local_result1_db_10, 1, 16ul);
   *local_result1_db_10 = 0.0;
   rocrFetchDB(_t1, (void**)&nPerLvl_l_db_4, 1, 0ul);
   rocrFetchDB(_t1, (void**)&_v6_db_11, 1, 17ul);
   *_v6_db_11 = *nPerLvl_l_db_4;
   rocrAutoDec(_t1, 45u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step5_dpbr_noGVars221_count_45,
      mapped_step5_dpbr_noGVars221_dtenum_45, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrAutoDec(_t1, 44u, (unsigned long)0l, rocrAlloc((void**)&allArgs_1, 8ul),
       mapped_step5_dpbr_noGVars220_count_44,
      mapped_step5_dpbr_noGVars220_dtenum_44, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrAutoDec(_t1, 42u, (unsigned long)0l, rocrAlloc((void**)&allArgs_2, 8ul),
       mapped_step5_dpbr_noGVars219_count_42,
      mapped_step5_dpbr_noGVars219_dtenum_42, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrAutoDec(_t1, 39u, (unsigned long)0l, rocrAlloc((void**)&allArgs_3, 8ul),
       mapped_step5_dpbr_noGVars217_count_39,
      mapped_step5_dpbr_noGVars217_dtenum_39, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrAutoDec(_t1, 33u, (unsigned long)0l, rocrAlloc((void**)&allArgs_4, 8ul),
       mapped_step5_dpbr_noGVars212_count_33,
      mapped_step5_dpbr_noGVars212_dtenum_33, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrAutoDec(_t1, 32u, (unsigned long)0l, rocrAlloc((void**)&allArgs_5, 8ul),
       mapped_step5_dpbr_noGVars211_count_32,
      mapped_step5_dpbr_noGVars211_dtenum_32, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrAutoDec(_t1, 29u, (unsigned long)0l, rocrAlloc((void**)&allArgs_6, 8ul),
       mapped_step5_dpbr_noGVars209_count_29,
      mapped_step5_dpbr_noGVars209_dtenum_29, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrAutoDec(_t1, 28u, (unsigned long)0l, rocrAlloc((void**)&allArgs_7, 8ul),
       mapped_step5_dpbr_noGVars208_count_28,
      mapped_step5_dpbr_noGVars208_dtenum_28, (void (*)(void*, long*))((void*)
      0l), 0u);
   _t2 = rocrAlloc((void**)&allArgs_8, 8ul);
   normr_49 = mapped_step5_dpbr_noGVars151_args.data.normr;
   allArgs_8->data.normr = normr_49;
   rocrAutoDec(_t1, 27u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars165_count_27,
      mapped_step5_dpbr_noGVars165_dtenum_27, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (pAp = mapped_step5_dpbr_noGVars151_args.data.pAp,
        alpha = mapped_step5_dpbr_noGVars151_args.data.alpha,
        x_44 = mapped_step5_dpbr_noGVars151_args.data.x,
        r_l = mapped_step5_dpbr_noGVars151_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars151_args.data.Ap_l,
        i = 0l;
        i <= 8l; i++) {
      ocrGuid_t _t5;
      union __args_mapped_step5_dpbr_noGVars163_170* _t6;
      _t5 = rocrAlloc((void**)&allArgs_9, 24ul);
      _t6 = allArgs_9;
      _t6->data.r_l = r_l;
      _t6->data.normr = normr_49;
      _t6->data.IT0 = i;
      rocrAutoDec(_t1, 26u, (unsigned long)i, _t5,
         mapped_step5_dpbr_noGVars163_count_26,
         mapped_step5_dpbr_noGVars163_dtenum_26, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i);
   }
   for (i_1 = 0l; i_1 <= 34l; i_1++) {
      ocrGuid_t _t7;
      union __args_mapped_step5_dpbr_noGVars160_168* _t8;
      _t7 = rocrAlloc((void**)&allArgs_10, 40ul);
      _t8 = allArgs_10;
      _t8->data.Ap_l = Ap_l;
      _t8->data.r_l = r_l;
      _t8->data.x = x_44;
      _t8->data.normr = normr_49;
      _t8->data.IT0 = i_1;
      rocrAutoDec(_t1, 25u, (unsigned long)i_1, _t7,
         mapped_step5_dpbr_noGVars160_count_25,
         mapped_step5_dpbr_noGVars160_dtenum_25, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i_1);
   }
   _t3 = rocrAlloc((void**)&allArgs_11, 48ul);
   _t4 = allArgs_11;
   _t4->data.Ap_l = Ap_l;
   _t4->data.r_l = r_l;
   _t4->data.x = x_44;
   _t4->data.normr = normr_49;
   _t4->data.alpha = alpha;
   _t4->data.pAp = pAp;
   rocrAutoDec(_t1, 24u, (unsigned long)0l, _t3,
      mapped_step5_dpbr_noGVars158_count_24,
      mapped_step5_dpbr_noGVars158_dtenum_24, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (i_2 = 0l; i_2 <= 17l; i_2++) {
      ocrGuid_t _t9;
      union __args_mapped_step5_dpbr_noGVars156_164* _t10;
      _t9 = rocrAlloc((void**)&allArgs_12, 56ul);
      _t10 = allArgs_12;
      _t10->data.Ap_l = Ap_l;
      _t10->data.r_l = r_l;
      _t10->data.x = x_44;
      _t10->data.normr = normr_49;
      _t10->data.alpha = alpha;
      _t10->data.pAp = pAp;
      _t10->data.IT0 = i_2;
      rocrAutoDec(_t1, 23u, (unsigned long)i_2, _t9,
         mapped_step5_dpbr_noGVars156_count_23,
         mapped_step5_dpbr_noGVars156_dtenum_23, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i_2);
   }
   for (i_3 = 0l; i_3 <= 137l; i_3++) {
      ocrGuid_t _t11;
      union __args_mapped_step5_dpbr_noGVars153_162* _t12;
      _t11 = rocrAlloc((void**)&allArgs_13, 56ul);
      _t12 = allArgs_13;
      _t12->data.Ap_l = Ap_l;
      _t12->data.r_l = r_l;
      _t12->data.x = x_44;
      _t12->data.normr = normr_49;
      _t12->data.alpha = alpha;
      _t12->data.pAp = pAp;
      _t12->data.IT0 = i_3;
      rocrAutoDec(_t1, 22u, (unsigned long)i_3, _t11,
         mapped_step5_dpbr_noGVars153_count_22,
         mapped_step5_dpbr_noGVars153_dtenum_22, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i_3);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars151_dtenum_21(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 17u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
   rocrEnumDB(rocrEDTCtx, 16u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
   rocrEnumDB(rocrEDTCtx, 14u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
   rocrEnumDB(rocrEDTCtx, 11u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
   rocrEnumDB(rocrEDTCtx, 5u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
   rocrEnumDB(rocrEDTCtx, 4u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
   rocrEnumDB(rocrEDTCtx, 1u, 4ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
   rocrEnumDB(rocrEDTCtx, 0u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars153(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars153_162
      mapped_step5_dpbr_noGVars153_args;
   int _p2_l[8192];
   int* _v3_db_0;
   double* Ap_l_db_1;
   int* nnzPerRow_l_db_3;
   int* colInds_l_db_4;
   double* A_l_db_5;
   double* p_l_db_6;
   double* Ap_l_db_2;
   int _p2[8192];
   union __args_mapped_step5_dpbr_noGVars192_214* allArgs;
   union __args_mapped_step5_dpbr_noGVars189_212* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars177_186* allArgs_2;
   union __args_mapped_step5_dpbr_noGVars173_180* allArgs_3;
   union __args_mapped_step5_dpbr_noGVars170_178* allArgs_4;
   union __args_mapped_step5_dpbr_noGVars209_176* allArgs_5;
   union __args_mapped_step5_dpbr_noGVars160_168* allArgs_6;
   union __args_mapped_step5_dpbr_noGVars156_164* allArgs_7;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_45;
   double* normr_50;
   double* alpha;
   double* pAp;
   long IT0;
   long _t2;
   ocrGuid_t _t3;
   long _t4;
   long _t5;
   long _t6;
   long i;
   long i_1;
   long _t7;
   long i_2;
   long i_3;
   long i_4;
   long i_5;
   long i_6;
   long _t8;
   long i_7;
   long _t9;
   long i_8;
   long i_9;
   long _t10;
   long i_10;
   long i_11;
   long _t11;
   long i_12;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        mapped_step5_dpbr_noGVars153_args = *(union
           __args_mapped_step5_dpbr_noGVars153_162*)rocrArgs(depv),
        IT0 = mapped_step5_dpbr_noGVars153_args.data.IT0,
        _t6 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)),
        pAp = mapped_step5_dpbr_noGVars153_args.data.pAp,
        alpha = mapped_step5_dpbr_noGVars153_args.data.alpha,
        normr_50 = mapped_step5_dpbr_noGVars153_args.data.normr,
        x_45 = mapped_step5_dpbr_noGVars153_args.data.x,
        r_l = mapped_step5_dpbr_noGVars153_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars153_args.data.Ap_l,
        i = 0l;
        i <= _t6; i++) {
      _p2_l[i] = _p2[i];
   }
   rocrFetchDB(_t1, (void**)&_v3_db_0, 1, 1ul);
   _t2 = (__mins_64(-8192l * IT0 + 1124863l, 8191l));
   rocrFetchDB(_t1, (void**)&Ap_l_db_1, 2, 2ul, (unsigned long)(IT0 >> 3l));
   for (i_1 = 0l; i_1 <= _t2; i_1++) {
      _p2_l[i_1] = (int)(i_1 + 8192l * IT0) < *_v3_db_0;
      if (_p2_l[i_1]) {
         Ap_l_db_1[8192l * IT0 + i_1 + -65536l * (8192l * IT0 + i_1 >> 16l)] =
            0.0;
      }
   }
   for (_t7 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)), i_2 = 0l; i_2 <= _t7
           ; i_2++) {
      _p2[i_2] = _p2_l[i_2];
   }
   for (i_3 = 0l,
        i_4 = 0l,
        i_5 = 0l,
        i_6 = 26l;
        i_3 <= 1l;
        i_3++,
        i_4 += 16,
        i_5 += 16,
        i_6 -= 16) {
      int _p3_l;
      int _p3;
      long _t12;
      long j;
      long _t13;
      long j_1;
      long j_2;
      for (_t12 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)), j = 0l; j <= _t12
              ; j++) {
         _p2_l[j] = _p2[j];
      }
      _p3_l = _p3;
      rocrFetchDB(_t1, (void**)&Ap_l_db_2, 2, 2ul, (unsigned long)(IT0 >> 3l));
      rocrFetchDB(_t1, (void**)&nnzPerRow_l_db_3, 3, 3ul, (unsigned long)0l, (
         unsigned long)(IT0 >> 3l));
      rocrFetchDB(_t1, (void**)&colInds_l_db_4, 4, 19ul, (unsigned long)0l, (
         unsigned long)(IT0 >> 3l), (unsigned long)0l);
      rocrFetchDB(_t1, (void**)&A_l_db_5, 4, 20ul, (unsigned long)0l, (
         unsigned long)(IT0 >> 3l), (unsigned long)0l);
      rocrFetchDB(_t1, (void**)&p_l_db_6, 2, 6ul, (unsigned long)0l);
      for (_t13 = (__mins_64(-8192l * IT0 + 1124863l, 8191l)), j_1 = 0l, j_2 =
              0l; j_1 <= _t13; j_1++, j_2 += 27) {
         if (_p2_l[j_1]) {
            long _t14;
            long k;
            long k_1;
            long k_2;
            long k_3;
            for (_t14 = (__mins_64(i_6, 15l)),
                 k = 0l,
                 k_1 = i_5,
                 k_3 = i_5,
                 k_2 = i_5;
                 k <= _t14;
                 k++,
                 k_1++,
                 k_2++,
                 k_3++) {
               _p3_l = (int)k_1 < nnzPerRow_l_db_3[8192l * IT0 + j_1 + -65536l *
                  (8192l * IT0 + j_1 >> 16l)];
               if (_p3_l) {
                  spmvBB(colInds_l_db_4 + (221184l * IT0 + k + j_2 + i_4 + -27l
                     * (__divfloors_64(k_2, 27l)) + -1769472l * (8192l * IT0 +
                     j_1 >> 16l)), A_l_db_5 + (221184l * IT0 + k + j_2 + i_4 +
                     -27l * (__divfloors_64(k_3, 27l)) + -1769472l * (8192l *
                     IT0 + j_1 >> 16l)), p_l_db_6 + 0l, Ap_l_db_2 + (8192l * IT0
                     + j_1 + -65536l * (8192l * IT0 + j_1 >> 16l)));
               }
            }
         }
      }
      _p3 = _p3_l;
   }
   for (_t8 = (__mins_64(IT0 >> 3l, 17l)), i_7 = (__maxs_64(0l, IT0 + -7l + 7l
           >> 3l)); i_7 <= _t8; i_7++) {
      ocrGuid_t _t15;
      union __args_mapped_step5_dpbr_noGVars192_214* _t16;
      _t15 = rocrAlloc((void**)&allArgs, 24ul);
      _t16 = allArgs;
      _t16->data.IT0 = 0l;
      _t16->data.IT1 = i_7;
      _t16->data.IT2 = 0l;
      rocrAutoDec(_t1, 48u, (unsigned long)i_7, _t15,
         mapped_step5_dpbr_noGVars192_count_48,
         mapped_step5_dpbr_noGVars192_dtenum_48, (void (*)(void*, long*))((void
         *)0l), 3u, (long)0l, (long)i_7, (long)0l);
   }
   for (_t9 = (__mins_64(IT0 >> 3l, 17l)), i_8 = (__maxs_64(0l, IT0 + -7l + 7l
           >> 3l)); i_8 <= _t9; i_8++) {
      ocrGuid_t _t17;
      union __args_mapped_step5_dpbr_noGVars189_212* _t18;
      _t17 = rocrAlloc((void**)&allArgs_1, 24ul);
      _t18 = allArgs_1;
      _t18->data.IT0 = 0l;
      _t18->data.IT1 = i_8;
      _t18->data.IT2 = 0l;
      rocrAutoDec(_t1, 47u, (unsigned long)i_8, _t17,
         mapped_step5_dpbr_noGVars189_count_47,
         mapped_step5_dpbr_noGVars189_dtenum_47, (void (*)(void*, long*))((void
         *)0l), 3u, (long)0l, (long)i_8, (long)0l);
   }
   _t3 = rocrAlloc((void**)&allArgs_2, 8ul);
   allArgs_2->data.IT0 = 0l;
   _t4 = (__mins_64(IT0 >> 3l, 17l));
   rocrAutoDec(_t1, 34u, (unsigned long)0l, _t3,
      mapped_step5_dpbr_noGVars177_count_34,
      mapped_step5_dpbr_noGVars177_dtenum_34, (void (*)(void*, long*))((void*)
      0l), 1u, (long)0l);
   for (i_9 = (__maxs_64(0l, IT0 + -7l + 7l >> 3l)); i_9 <= _t4; i_9++) {
      ocrGuid_t _t19;
      union __args_mapped_step5_dpbr_noGVars173_180* _t20;
      _t19 = rocrAlloc((void**)&allArgs_3, 16ul);
      _t20 = allArgs_3;
      _t20->data.IT0 = 0l;
      _t20->data.IT1 = i_9;
      rocrAutoDec(_t1, 31u, (unsigned long)i_9, _t19,
         mapped_step5_dpbr_noGVars173_count_31,
         mapped_step5_dpbr_noGVars173_dtenum_31, (void (*)(void*, long*))((void
         *)0l), 2u, (long)0l, (long)i_9);
   }
   for (_t10 = (__mins_64(IT0 >> 3l, 17l)), i_10 = (__maxs_64(0l, IT0 + -7l + 7l
           >> 3l)); i_10 <= _t10; i_10++) {
      ocrGuid_t _t21;
      union __args_mapped_step5_dpbr_noGVars170_178* _t22;
      _t21 = rocrAlloc((void**)&allArgs_4, 16ul);
      _t22 = allArgs_4;
      _t22->data.Ap_l = Ap_l;
      _t22->data.IT0 = i_10;
      rocrAutoDec(_t1, 30u, (unsigned long)i_10, _t21,
         mapped_step5_dpbr_noGVars170_count_30,
         mapped_step5_dpbr_noGVars170_dtenum_30, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i_10);
   }
   _t5 = (__mins_64(IT0 >> 2l, 34l));
   rocrAutoDec(_t1, 29u, (unsigned long)0l, rocrAlloc((void**)&allArgs_5, 8ul),
       mapped_step5_dpbr_noGVars209_count_29,
      mapped_step5_dpbr_noGVars209_dtenum_29, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (i_11 = (__maxs_64(0l, IT0 + -3l + 3l >> 2l)); i_11 <= _t5; i_11++) {
      ocrGuid_t _t23;
      union __args_mapped_step5_dpbr_noGVars160_168* _t24;
      _t23 = rocrAlloc((void**)&allArgs_6, 40ul);
      _t24 = allArgs_6;
      _t24->data.Ap_l = Ap_l;
      _t24->data.r_l = r_l;
      _t24->data.x = x_45;
      _t24->data.normr = normr_50;
      _t24->data.IT0 = i_11;
      rocrAutoDec(_t1, 25u, (unsigned long)i_11, _t23,
         mapped_step5_dpbr_noGVars160_count_25,
         mapped_step5_dpbr_noGVars160_dtenum_25, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i_11);
   }
   for (_t11 = (__mins_64(IT0 >> 3l, 17l)), i_12 = (__maxs_64(0l, IT0 + -7l + 7l
           >> 3l)); i_12 <= _t11; i_12++) {
      ocrGuid_t _t25;
      union __args_mapped_step5_dpbr_noGVars156_164* _t26;
      _t25 = rocrAlloc((void**)&allArgs_7, 56ul);
      _t26 = allArgs_7;
      _t26->data.Ap_l = Ap_l;
      _t26->data.r_l = r_l;
      _t26->data.x = x_45;
      _t26->data.normr = normr_50;
      _t26->data.alpha = alpha;
      _t26->data.pAp = pAp;
      _t26->data.IT0 = i_12;
      rocrAutoDec(_t1, 23u, (unsigned long)i_12, _t25,
         mapped_step5_dpbr_noGVars156_count_23,
         mapped_step5_dpbr_noGVars156_dtenum_23, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i_12);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars153_dtenum_22(void* rocrEDTCtx, long* args)
{
   long _t1;
   long _t2;
   long _t3;
   long i;
   long _t4;
   long i_1;
   long i_2;
   long _t5;
   long i_3;
   for (_t1 = (long)args[0l], _t3 = (__mins_64(_t1 >> 3l, 17l)), i = (__maxs_64
           (0l, _t1 + -7l + 7l >> 3l)); i <= _t3; i++) {
      rocrEnumDB(rocrEDTCtx, 20u, 14155776ul, 1l, 0u, (void (*)(void*, long*))(
         (void*)0l), (unsigned long)0l, (unsigned long)i, (unsigned long)0l);
   }
   for (_t4 = (__mins_64(_t1 >> 3l, 17l)), i_1 = (__maxs_64(0l, _t1 + -7l + 7l
           >> 3l)); i_1 <= _t4; i_1++) {
      rocrEnumDB(rocrEDTCtx, 19u, 7077888ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)0l, (unsigned long)i_1, (unsigned long)0l);
   }
   _t2 = (__mins_64(_t1 >> 3l, 17l));
   rocrEnumDB(rocrEDTCtx, 6u, 8998912ul, 2l, 0u, (void (*)(void*, long*))((void
      *)0l), (unsigned long)0l);
   for (i_2 = (__maxs_64(0l, _t1 + -7l + 7l >> 3l)); i_2 <= _t2; i_2++) {
      rocrEnumDB(rocrEDTCtx, 3u, 262144ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)0l, (unsigned long)i_2);
   }
   for (_t5 = (__mins_64(_t1 >> 3l, 17l)), i_3 = (__maxs_64(0l, _t1 + -7l + 7l
           >> 3l)); i_3 <= _t5; i_3++) {
      rocrEnumDB(rocrEDTCtx, 2u, 524288ul, 1l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)i_3);
   }
   rocrEnumDB(rocrEDTCtx, 1u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars156(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars156_164
      mapped_step5_dpbr_noGVars156_args;
   int* _v4_db_0;
   double* p_l_db_1;
   double* Ap_l_db_2;
   double* local_result_db_3;
   union __args_mapped_step5_dpbr_noGVars177_186* allArgs;
   union __args_mapped_step5_dpbr_noGVars212_184* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars211_182* allArgs_2;
   union __args_mapped_step5_dpbr_noGVars170_178* allArgs_3;
   union __args_mapped_step5_dpbr_noGVars158_166* allArgs_4;
   union __args_mapped_step5_dpbr_noGVars156_164* allArgs_5;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_46;
   double* normr_51;
   double* alpha;
   double* pAp;
   long IT0;
   int _p4_l;
   int _p4;
   ocrGuid_t _t2;
   ocrGuid_t _t3;
   union __args_mapped_step5_dpbr_noGVars170_178* _t4;
   ocrGuid_t _t5;
   union __args_mapped_step5_dpbr_noGVars158_166* _t6;
   long _t7;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars156_args = *(union
      __args_mapped_step5_dpbr_noGVars156_164*)rocrArgs(depv);
   _p4_l = _p4;
   rocrFetchDB(_t1, (void**)&_v4_db_0, 1, 5ul);
   IT0 = mapped_step5_dpbr_noGVars156_args.data.IT0;
   rocrFetchDB(_t1, (void**)&Ap_l_db_2, 2, 2ul, (unsigned long)IT0);
   rocrFetchDB(_t1, (void**)&local_result_db_3, 1, 4ul);
   for (_t7 = (__mins_64(-65536l * IT0 + 1124863l, 65535l)),
        pAp = mapped_step5_dpbr_noGVars156_args.data.pAp,
        alpha = mapped_step5_dpbr_noGVars156_args.data.alpha,
        normr_51 = mapped_step5_dpbr_noGVars156_args.data.normr,
        x_46 = mapped_step5_dpbr_noGVars156_args.data.x,
        r_l = mapped_step5_dpbr_noGVars156_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars156_args.data.Ap_l,
        i = 0l;
        i <= _t7; i++) {
      _p4_l = (int)(i + 65536l * IT0) < *_v4_db_0;
      rocrFetchDB(_t1, (void**)&p_l_db_1, 2, 6ul, (unsigned long)((i + (65536l *
         IT0 + 0l)) / 1124864l));
      if (_p4_l) {
         double* _t8;
         _t8 = local_result_db_3;
         *_t8 = *_t8 + p_l_db_1[65536l * IT0 + i + -1124864l * (__divfloors_64(
            65536l * IT0 + i, 1124864l))] * Ap_l_db_2[65536l * IT0 + i + -65536l
            * (65536l * IT0 + i >> 16l)];
      }
   }
   _p4 = _p4_l;
   _t2 = rocrAlloc((void**)&allArgs, 8ul);
   allArgs->data.IT0 = 0l;
   rocrAutoDec(_t1, 34u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars177_count_34,
      mapped_step5_dpbr_noGVars177_dtenum_34, (void (*)(void*, long*))((void*)
      0l), 1u, (long)0l);
   rocrAutoDec(_t1, 33u, (unsigned long)0l, rocrAlloc((void**)&allArgs_1, 8ul),
       mapped_step5_dpbr_noGVars212_count_33,
      mapped_step5_dpbr_noGVars212_dtenum_33, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrAutoDec(_t1, 32u, (unsigned long)0l, rocrAlloc((void**)&allArgs_2, 8ul),
       mapped_step5_dpbr_noGVars211_count_32,
      mapped_step5_dpbr_noGVars211_dtenum_32, (void (*)(void*, long*))((void*)
      0l), 0u);
   _t3 = rocrAlloc((void**)&allArgs_3, 16ul);
   _t4 = allArgs_3;
   _t4->data.Ap_l = Ap_l;
   _t4->data.IT0 = IT0;
   rocrAutoDec(_t1, 30u, (unsigned long)IT0, _t3,
      mapped_step5_dpbr_noGVars170_count_30,
      mapped_step5_dpbr_noGVars170_dtenum_30, (void (*)(void*, long*))((void*)
      0l), 1u, (long)IT0);
   _t5 = rocrAlloc((void**)&allArgs_4, 48ul);
   _t6 = allArgs_4;
   _t6->data.Ap_l = Ap_l;
   _t6->data.r_l = r_l;
   _t6->data.x = x_46;
   _t6->data.normr = normr_51;
   _t6->data.alpha = alpha;
   _t6->data.pAp = pAp;
   rocrAutoDec(_t1, 24u, (unsigned long)0l, _t5,
      mapped_step5_dpbr_noGVars158_count_24,
      mapped_step5_dpbr_noGVars158_dtenum_24, (void (*)(void*, long*))((void*)
      0l), 0u);
   if (IT0 <= 16l) {
      ocrGuid_t _t9;
      union __args_mapped_step5_dpbr_noGVars156_164* _t10;
      long _t11;
      _t9 = rocrAlloc((void**)&allArgs_5, 56ul);
      _t10 = allArgs_5;
      _t10->data.Ap_l = Ap_l;
      _t10->data.r_l = r_l;
      _t10->data.x = x_46;
      _t10->data.normr = normr_51;
      _t10->data.alpha = alpha;
      _t10->data.pAp = pAp;
      _t11 = IT0 + 1l;
      _t10->data.IT0 = _t11;
      rocrAutoDec(_t1, 23u, (unsigned long)(1l + IT0), _t9,
         mapped_step5_dpbr_noGVars156_count_23,
         mapped_step5_dpbr_noGVars156_dtenum_23, (void (*)(void*, long*))((void
         *)0l), 1u, (long)_t11);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars156_dtenum_23(void* rocrEDTCtx, long* args)
{
   long _t1;
   rocrEnumDB(rocrEDTCtx, 6u, 8998912ul, 1l, 0u, (void (*)(void*, long*))((void
      *)0l), (unsigned long)0l);
   rocrEnumDB(rocrEDTCtx, 5u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
   rocrEnumDB(rocrEDTCtx, 4u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
   _t1 = (long)args[0l];
   if (_t1 <= 16l) {
      rocrEnumDB(rocrEDTCtx, 2u, 524288ul, 2l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)_t1);
   }
   if (_t1 == 17l) {
      rocrEnumDB(rocrEDTCtx, 2u, 524288ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)17l);
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars158(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars158_166
      mapped_step5_dpbr_noGVars158_args;
   double* pAp_db_0;
   double* local_result_db_2;
   double* alpha_db_3;
   double* rtz_db_6;
   double* pAp_db_1;
   double* alpha_db_4;
   double* _v1_db_7;
   double* alpha_db_5;
   double* _v2_db_8;
   union __args_mapped_step5_dpbr_noGVars218_200* allArgs;
   union __args_mapped_step5_dpbr_noGVars216_194* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars215_192* allArgs_2;
   union __args_mapped_step5_dpbr_noGVars214_190* allArgs_3;
   union __args_mapped_step5_dpbr_noGVars213_188* allArgs_4;
   union __args_mapped_step5_dpbr_noGVars211_182* allArgs_5;
   union __args_mapped_step5_dpbr_noGVars160_168* allArgs_6;
   void* _t1;
   ocrGuid_t _t2;
   ocrGuid_t _t3;
   double* Ap_l;
   double*restrict* r_l;
   double* x_47;
   double* normr_52;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars158_args = *(union
      __args_mapped_step5_dpbr_noGVars158_166*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&pAp_db_0, 1, 7ul);
   rocrFetchDB(_t1, (void**)&local_result_db_2, 1, 4ul);
   *pAp_db_0 = *local_result_db_2;
   rocrFetchDB(_t1, (void**)&pAp_db_1, 1, 7ul);
   rocrFetchDB(_t1, (void**)&alpha_db_3, 1, 8ul);
   rocrFetchDB(_t1, (void**)&rtz_db_6, 1, 9ul);
   *alpha_db_3 = *rtz_db_6 / *pAp_db_1;
   rocrFetchDB(_t1, (void**)&alpha_db_4, 1, 8ul);
   rocrFetchDB(_t1, (void**)&_v1_db_7, 1, 10ul);
   *_v1_db_7 = *alpha_db_4;
   rocrFetchDB(_t1, (void**)&alpha_db_5, 1, 8ul);
   rocrFetchDB(_t1, (void**)&_v2_db_8, 1, 13ul);
   *_v2_db_8 = *alpha_db_5;
   rocrAutoDec(_t1, 41u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step5_dpbr_noGVars218_count_41,
      mapped_step5_dpbr_noGVars218_dtenum_41, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrAutoDec(_t1, 38u, (unsigned long)0l, rocrAlloc((void**)&allArgs_1, 8ul),
       mapped_step5_dpbr_noGVars216_count_38,
      mapped_step5_dpbr_noGVars216_dtenum_38, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrAutoDec(_t1, 37u, (unsigned long)0l, rocrAlloc((void**)&allArgs_2, 8ul),
       mapped_step5_dpbr_noGVars215_count_37,
      mapped_step5_dpbr_noGVars215_dtenum_37, (void (*)(void*, long*))((void*)
      0l), 0u);
   _t2 = rocrAlloc((void**)&allArgs_3, 8ul);
   allArgs_3->data.alpha = mapped_step5_dpbr_noGVars158_args.data.alpha;
   rocrAutoDec(_t1, 36u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars214_count_36,
      mapped_step5_dpbr_noGVars214_dtenum_36, (void (*)(void*, long*))((void*)
      0l), 0u);
   _t3 = rocrAlloc((void**)&allArgs_4, 8ul);
   allArgs_4->data.pAp = mapped_step5_dpbr_noGVars158_args.data.pAp;
   rocrAutoDec(_t1, 35u, (unsigned long)0l, _t3,
      mapped_step5_dpbr_noGVars213_count_35,
      mapped_step5_dpbr_noGVars213_dtenum_35, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrAutoDec(_t1, 32u, (unsigned long)0l, rocrAlloc((void**)&allArgs_5, 8ul),
       mapped_step5_dpbr_noGVars211_count_32,
      mapped_step5_dpbr_noGVars211_dtenum_32, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (normr_52 = mapped_step5_dpbr_noGVars158_args.data.normr,
        x_47 = mapped_step5_dpbr_noGVars158_args.data.x,
        r_l = mapped_step5_dpbr_noGVars158_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars158_args.data.Ap_l,
        i = 0l;
        i <= 34l; i++) {
      ocrGuid_t _t4;
      union __args_mapped_step5_dpbr_noGVars160_168* _t5;
      _t4 = rocrAlloc((void**)&allArgs_6, 40ul);
      _t5 = allArgs_6;
      _t5->data.Ap_l = Ap_l;
      _t5->data.r_l = r_l;
      _t5->data.x = x_47;
      _t5->data.normr = normr_52;
      _t5->data.IT0 = i;
      rocrAutoDec(_t1, 25u, (unsigned long)i, _t4,
         mapped_step5_dpbr_noGVars160_count_25,
         mapped_step5_dpbr_noGVars160_dtenum_25, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars158_dtenum_24(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 13u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
   rocrEnumDB(rocrEDTCtx, 10u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
   rocrEnumDB(rocrEDTCtx, 9u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
   rocrEnumDB(rocrEDTCtx, 8u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
   rocrEnumDB(rocrEDTCtx, 7u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l))
      ;
   rocrEnumDB(rocrEDTCtx, 4u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars160(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars160_168
      mapped_step5_dpbr_noGVars160_args;
   int* _v5_db_0;
   double* x_db_1;
   double* x_db_2;
   double* p_l_db_3;
   double* _v1_db_4;
   int* _v7_db_5;
   double* r_l_db_6;
   double* r_l_db_7;
   double* Ap_l_db_8;
   double* _v2_db_9;
   union __args_mapped_step5_dpbr_noGVars185_204* allArgs;
   union __args_mapped_step5_dpbr_noGVars219_202* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars218_200* allArgs_2;
   union __args_mapped_step5_dpbr_noGVars181_198* allArgs_3;
   union __args_mapped_step5_dpbr_noGVars217_196* allArgs_4;
   union __args_mapped_step5_dpbr_noGVars216_194* allArgs_5;
   union __args_mapped_step5_dpbr_noGVars177_186* allArgs_6;
   union __args_mapped_step5_dpbr_noGVars170_178* allArgs_7;
   union __args_mapped_step5_dpbr_noGVars163_170* allArgs_8;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_48;
   double* normr_53;
   long IT0;
   int _p6_l;
   int _p5_l;
   int _p6;
   int _p5;
   long _t2;
   ocrGuid_t _t3;
   long _t4;
   long _t5;
   long i;
   long _t6;
   long i_1;
   long i_2;
   long i_3;
   long _t7;
   long i_4;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars160_args = *(union
      __args_mapped_step5_dpbr_noGVars160_168*)rocrArgs(depv);
   _p6_l = _p6;
   _p5_l = _p5;
   rocrFetchDB(_t1, (void**)&_v5_db_0, 1, 11ul);
   IT0 = mapped_step5_dpbr_noGVars160_args.data.IT0;
   rocrFetchDB(_t1, (void**)&x_db_1, 2, 12ul, (unsigned long)(IT0 >> 1l));
   rocrFetchDB(_t1, (void**)&x_db_2, 2, 12ul, (unsigned long)(IT0 >> 1l));
   rocrFetchDB(_t1, (void**)&_v1_db_4, 1, 10ul);
   rocrFetchDB(_t1, (void**)&_v7_db_5, 1, 14ul);
   rocrFetchDB(_t1, (void**)&r_l_db_6, 3, 15ul, (unsigned long)0l, (
      unsigned long)(IT0 >> 1l));
   rocrFetchDB(_t1, (void**)&r_l_db_7, 3, 15ul, (unsigned long)0l, (
      unsigned long)(IT0 >> 1l));
   rocrFetchDB(_t1, (void**)&Ap_l_db_8, 2, 2ul, (unsigned long)(IT0 >> 1l));
   rocrFetchDB(_t1, (void**)&_v2_db_9, 1, 13ul);
   for (_t5 = (__mins_64(-32768l * IT0 + 1124863l, 32767l)),
        normr_53 = mapped_step5_dpbr_noGVars160_args.data.normr,
        x_48 = mapped_step5_dpbr_noGVars160_args.data.x,
        r_l = mapped_step5_dpbr_noGVars160_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars160_args.data.Ap_l,
        i = 0l;
        i <= _t5; i++) {
      _p6_l = (int)(i + 32768l * IT0) < *_v5_db_0;
      rocrFetchDB(_t1, (void**)&p_l_db_3, 2, 6ul, (unsigned long)((i + (32768l *
         IT0 + 0l)) / 1124864l));
      if (_p6_l) {
         x_db_1[32768l * IT0 + i + -65536l * (32768l * IT0 + i >> 16l)] = x_db_2
            [32768l * IT0 + i + -65536l * (32768l * IT0 + i >> 16l)] + *
            _v1_db_4 * p_l_db_3[32768l * IT0 + i + -1124864l * (__divfloors_64(
            32768l * IT0 + i, 1124864l))];
      }
      _p5_l = (int)(i + 32768l * IT0) < *_v7_db_5;
      if (_p5_l) {
         r_l_db_6[32768l * IT0 + i + -65536l * (32768l * IT0 + i >> 16l)] =
            r_l_db_7[32768l * IT0 + i + -65536l * (32768l * IT0 + i >> 16l)] + -
             *_v2_db_9 * Ap_l_db_8[32768l * IT0 + i + -65536l * (32768l * IT0 +
            i >> 16l)];
      }
   }
   for (_p6 = _p6_l,
        _p5 = _p5_l,
        _t6 = IT0 >> 1l,
        i_1 = (__maxs_64(0l, IT0 + -1l + 1l >> 1l));
        i_1 <= _t6; i_1++) {
      ocrGuid_t _t8;
      union __args_mapped_step5_dpbr_noGVars185_204* _t9;
      _t8 = rocrAlloc((void**)&allArgs, 24ul);
      _t9 = allArgs;
      _t9->data.r_l = r_l;
      _t9->data.IT0 = 0l;
      _t9->data.IT1 = i_1;
      rocrAutoDec(_t1, 43u, (unsigned long)i_1, _t8,
         mapped_step5_dpbr_noGVars185_count_43,
         mapped_step5_dpbr_noGVars185_dtenum_43, (void (*)(void*, long*))((void
         *)0l), 2u, (long)0l, (long)i_1);
   }
   rocrAutoDec(_t1, 42u, (unsigned long)0l, rocrAlloc((void**)&allArgs_1, 8ul),
       mapped_step5_dpbr_noGVars219_count_42,
      mapped_step5_dpbr_noGVars219_dtenum_42, (void (*)(void*, long*))((void*)
      0l), 0u);
   _t2 = IT0 >> 1l;
   rocrAutoDec(_t1, 41u, (unsigned long)0l, rocrAlloc((void**)&allArgs_2, 8ul),
       mapped_step5_dpbr_noGVars218_count_41,
      mapped_step5_dpbr_noGVars218_dtenum_41, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (i_2 = (__maxs_64(0l, IT0 + -1l + 1l >> 1l)); i_2 <= _t2; i_2++) {
      ocrGuid_t _t10;
      union __args_mapped_step5_dpbr_noGVars181_198* _t11;
      _t10 = rocrAlloc((void**)&allArgs_3, 16ul);
      _t11 = allArgs_3;
      _t11->data.x = x_48;
      _t11->data.IT0 = i_2;
      rocrAutoDec(_t1, 40u, (unsigned long)i_2, _t10,
         mapped_step5_dpbr_noGVars181_count_40,
         mapped_step5_dpbr_noGVars181_dtenum_40, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i_2);
   }
   rocrAutoDec(_t1, 39u, (unsigned long)0l, rocrAlloc((void**)&allArgs_4, 8ul),
       mapped_step5_dpbr_noGVars217_count_39,
      mapped_step5_dpbr_noGVars217_dtenum_39, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrAutoDec(_t1, 38u, (unsigned long)0l, rocrAlloc((void**)&allArgs_5, 8ul),
       mapped_step5_dpbr_noGVars216_count_38,
      mapped_step5_dpbr_noGVars216_dtenum_38, (void (*)(void*, long*))((void*)
      0l), 0u);
   _t3 = rocrAlloc((void**)&allArgs_6, 8ul);
   allArgs_6->data.IT0 = 0l;
   _t4 = IT0 >> 1l;
   rocrAutoDec(_t1, 34u, (unsigned long)0l, _t3,
      mapped_step5_dpbr_noGVars177_count_34,
      mapped_step5_dpbr_noGVars177_dtenum_34, (void (*)(void*, long*))((void*)
      0l), 1u, (long)0l);
   for (i_3 = (__maxs_64(0l, IT0 + -1l + 1l >> 1l)); i_3 <= _t4; i_3++) {
      ocrGuid_t _t12;
      union __args_mapped_step5_dpbr_noGVars170_178* _t13;
      _t12 = rocrAlloc((void**)&allArgs_7, 16ul);
      _t13 = allArgs_7;
      _t13->data.Ap_l = Ap_l;
      _t13->data.IT0 = i_3;
      rocrAutoDec(_t1, 30u, (unsigned long)i_3, _t12,
         mapped_step5_dpbr_noGVars170_count_30,
         mapped_step5_dpbr_noGVars170_dtenum_30, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i_3);
   }
   for (_t7 = (__mins_64(IT0 >> 2l, 8l)), i_4 = (__maxs_64(0l, IT0 + -3l + 3l >>
           2l)); i_4 <= _t7; i_4++) {
      ocrGuid_t _t14;
      union __args_mapped_step5_dpbr_noGVars163_170* _t15;
      _t14 = rocrAlloc((void**)&allArgs_8, 24ul);
      _t15 = allArgs_8;
      _t15->data.r_l = r_l;
      _t15->data.normr = normr_53;
      _t15->data.IT0 = i_4;
      rocrAutoDec(_t1, 26u, (unsigned long)i_4, _t14,
         mapped_step5_dpbr_noGVars163_count_26,
         mapped_step5_dpbr_noGVars163_dtenum_26, (void (*)(void*, long*))((void
         *)0l), 1u, (long)i_4);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars160_dtenum_25(void* rocrEDTCtx, long* args)
{
   long _t1;
   long _t2;
   long _t3;
   long _t4;
   long i;
   long i_1;
   long i_2;
   for (_t1 = (long)args[0l], _t4 = _t1 >> 1l, i = (__maxs_64(0l, _t1 + -1l + 1l
           >> 1l)); i <= _t4; i++) {
      rocrEnumDB(rocrEDTCtx, 15u, 524288ul, 1l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)0l, (unsigned long)i);
   }
   rocrEnumDB(rocrEDTCtx, 14u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
   _t2 = _t1 >> 1l;
   rocrEnumDB(rocrEDTCtx, 13u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
   for (i_1 = (__maxs_64(0l, _t1 + -1l + 1l >> 1l)); i_1 <= _t2; i_1++) {
      rocrEnumDB(rocrEDTCtx, 12u, 524288ul, 1l, 1u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)i_1);
   }
   rocrEnumDB(rocrEDTCtx, 11u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
   rocrEnumDB(rocrEDTCtx, 10u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
   _t3 = _t1 >> 1l;
   rocrEnumDB(rocrEDTCtx, 6u, 8998912ul, 1l, 0u, (void (*)(void*, long*))((void
      *)0l), (unsigned long)0l);
   for (i_2 = (__maxs_64(0l, _t1 + -1l + 1l >> 1l)); i_2 <= _t3; i_2++) {
      rocrEnumDB(rocrEDTCtx, 2u, 524288ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)i_2);
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars163(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars163_170
      mapped_step5_dpbr_noGVars163_args;
   int* _v6_db_0;
   double* r_l_db_1;
   double* r_l_db_2;
   double* local_result1_db_3;
   union __args_mapped_step5_dpbr_noGVars221_208* allArgs;
   union __args_mapped_step5_dpbr_noGVars220_206* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars185_204* allArgs_2;
   union __args_mapped_step5_dpbr_noGVars165_172* allArgs_3;
   union __args_mapped_step5_dpbr_noGVars163_170* allArgs_4;
   void* _t1;
   double*restrict* r_l;
   double* normr_54;
   long IT0;
   int _p1_l;
   int _p1;
   long _t2;
   ocrGuid_t _t3;
   long i;
   long i_1;
   long i_2;
   long i_3;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars163_args = *(union
      __args_mapped_step5_dpbr_noGVars163_170*)rocrArgs(depv);
   _p1_l = _p1;
   rocrFetchDB(_t1, (void**)&_v6_db_0, 1, 17ul);
   rocrFetchDB(_t1, (void**)&local_result1_db_3, 1, 16ul);
   for (IT0 = mapped_step5_dpbr_noGVars163_args.data.IT0,
        normr_54 = mapped_step5_dpbr_noGVars163_args.data.normr,
        r_l = mapped_step5_dpbr_noGVars163_args.data.r_l,
        i = 0l,
        i_1 = 0l,
        i_2 = 0l;
        i <= 1l; i++, i_1 += 65536, i_2 -= 65536) {
      long _t4;
      long j;
      rocrFetchDB(_t1, (void**)&r_l_db_1, 3, 15ul, (unsigned long)0l, (
         unsigned long)(i + 2l * IT0));
      _t4 = (__mins_64(i_2 + -131072l * IT0 + 1124863l, 65535l));
      rocrFetchDB(_t1, (void**)&r_l_db_2, 3, 15ul, (unsigned long)0l, (
         unsigned long)(i + 2l * IT0));
      for (j = 0l; j <= _t4; j++) {
         _p1_l = (int)(i_1 + (j + 131072l * IT0)) < *_v6_db_0;
         if (_p1_l) {
            double* _t5;
            _t5 = local_result1_db_3;
            *_t5 = *_t5 + r_l_db_1[131072l * IT0 + j + i_1 + -65536l * (131072l
               * IT0 + j + i_1 >> 16l)] * r_l_db_2[131072l * IT0 + j + i_1 +
               -65536l * (131072l * IT0 + j + i_1 >> 16l)];
         }
      }
   }
   _p1 = _p1_l;
   rocrAutoDec(_t1, 45u, (unsigned long)0l, rocrAlloc((void**)&allArgs, 8ul),
      mapped_step5_dpbr_noGVars221_count_45,
      mapped_step5_dpbr_noGVars221_dtenum_45, (void (*)(void*, long*))((void*)
      0l), 0u);
   _t2 = 2l * IT0 + 1l;
   rocrAutoDec(_t1, 44u, (unsigned long)0l, rocrAlloc((void**)&allArgs_1, 8ul),
       mapped_step5_dpbr_noGVars220_count_44,
      mapped_step5_dpbr_noGVars220_dtenum_44, (void (*)(void*, long*))((void*)
      0l), 0u);
   for (i_3 = 2l * IT0; i_3 <= _t2; i_3++) {
      ocrGuid_t _t6;
      union __args_mapped_step5_dpbr_noGVars185_204* _t7;
      _t6 = rocrAlloc((void**)&allArgs_2, 24ul);
      _t7 = allArgs_2;
      _t7->data.r_l = r_l;
      _t7->data.IT0 = 0l;
      _t7->data.IT1 = i_3;
      rocrAutoDec(_t1, 43u, (unsigned long)i_3, _t6,
         mapped_step5_dpbr_noGVars185_count_43,
         mapped_step5_dpbr_noGVars185_dtenum_43, (void (*)(void*, long*))((void
         *)0l), 2u, (long)0l, (long)i_3);
   }
   _t3 = rocrAlloc((void**)&allArgs_3, 8ul);
   allArgs_3->data.normr = normr_54;
   rocrAutoDec(_t1, 27u, (unsigned long)0l, _t3,
      mapped_step5_dpbr_noGVars165_count_27,
      mapped_step5_dpbr_noGVars165_dtenum_27, (void (*)(void*, long*))((void*)
      0l), 0u);
   if (IT0 <= 7l) {
      ocrGuid_t _t8;
      union __args_mapped_step5_dpbr_noGVars163_170* _t9;
      long _t10;
      _t8 = rocrAlloc((void**)&allArgs_4, 24ul);
      _t9 = allArgs_4;
      _t9->data.r_l = r_l;
      _t9->data.normr = normr_54;
      _t10 = IT0 + 1l;
      _t9->data.IT0 = _t10;
      rocrAutoDec(_t1, 26u, (unsigned long)(1l + IT0), _t8,
         mapped_step5_dpbr_noGVars163_count_26,
         mapped_step5_dpbr_noGVars163_dtenum_26, (void (*)(void*, long*))((void
         *)0l), 1u, (long)_t10);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars163_dtenum_26(void* rocrEDTCtx, long* args)
{
   long _t1;
   long _t2;
   long i;
   rocrEnumDB(rocrEDTCtx, 17u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
   _t1 = (long)args[0l];
   _t2 = (__mins_64(16l, 2l * _t1 + 1l));
   rocrEnumDB(rocrEDTCtx, 16u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
   for (i = 2l * _t1; i <= _t2; i++) {
      rocrEnumDB(rocrEDTCtx, 15u, 524288ul, 2l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)0l, (unsigned long)i);
   }
   if (_t1 == 8l) {
      rocrEnumDB(rocrEDTCtx, 15u, 524288ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)0l, (unsigned long)17l);
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars165(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars165_172
      mapped_step5_dpbr_noGVars165_args;
   double* normr_db_0;
   double* local_result1_db_1;
   union __args_mapped_step5_dpbr_noGVars187_210* allArgs;
   union __args_mapped_step5_dpbr_noGVars220_206* allArgs_1;
   void* _t1;
   ocrGuid_t _t2;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars165_args = *(union
      __args_mapped_step5_dpbr_noGVars165_172*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&normr_db_0, 1, 18ul);
   rocrFetchDB(_t1, (void**)&local_result1_db_1, 1, 16ul);
   *normr_db_0 = *local_result1_db_1;
   _t2 = rocrAlloc((void**)&allArgs, 8ul);
   allArgs->data.normr = mapped_step5_dpbr_noGVars165_args.data.normr;
   rocrAutoDec(_t1, 46u, (unsigned long)0l, _t2,
      mapped_step5_dpbr_noGVars187_count_46,
      mapped_step5_dpbr_noGVars187_dtenum_46, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrAutoDec(_t1, 44u, (unsigned long)0l, rocrAlloc((void**)&allArgs_1, 8ul),
       mapped_step5_dpbr_noGVars220_count_44,
      mapped_step5_dpbr_noGVars220_dtenum_44, (void (*)(void*, long*))((void*)
      0l), 0u);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars165_dtenum_27(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 18u, 8ul, 2l, 1u, (void (*)(void*, long*))((void*)0l)
      );
   rocrEnumDB(rocrEDTCtx, 16u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step5_dpbr_noGVars208(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 0ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars208_dtenum_28(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 0u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars209(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 1ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars209_dtenum_29(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 1u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars170(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars170_178
      mapped_step5_dpbr_noGVars170_args;
   double* Ap_l_db_0;
   void* _t1;
   long IT0;
   long _t2;
   double* Ap_l;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars170_args = *(union
      __args_mapped_step5_dpbr_noGVars170_178*)rocrArgs(depv);
   IT0 = mapped_step5_dpbr_noGVars170_args.data.IT0;
   _t2 = (__mins_64(-65536l * IT0 + 1124863l, 65535l));
   rocrFetchDB(_t1, (void**)&Ap_l_db_0, 2, 2ul, (unsigned long)IT0);
   for (Ap_l = mapped_step5_dpbr_noGVars170_args.data.Ap_l, i = 0l; i <= _t2; i
           ++) {
      Ap_l[i + 65536l * IT0] = Ap_l_db_0[65536l * IT0 + i + -65536l * (65536l *
         IT0 + i >> 16l)];
   }
   rocrFreeDB(_t1, 2, 2ul, (unsigned long)IT0);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars170_dtenum_30(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[0l];
   if (_t1 <= 16l) {
      rocrEnumDB(rocrEDTCtx, 2u, 524288ul, 2l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)_t1);
   }
   if (_t1 == 17l) {
      rocrEnumDB(rocrEDTCtx, 2u, 524288ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)17l);
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars173(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars173_180
      mapped_step5_dpbr_noGVars173_args;
   void* _t1;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars173_args = *(union
      __args_mapped_step5_dpbr_noGVars173_180*)rocrArgs(depv);
   IT1 = mapped_step5_dpbr_noGVars173_args.data.IT1;
   if (mapped_step5_dpbr_noGVars173_args.data.IT0 == 0l) {
      rocrFreeDB(_t1, 3, 3ul, (unsigned long)0l, (unsigned long)IT1);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars173_dtenum_31(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[1l];
   if ((long)args[0l] == 0l) {
      if (_t1 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 3u, 262144ul, 2l, 0u, (void (*)(void*, long*))(
            (void*)0l), (unsigned long)0l, (unsigned long)_t1);
      }
      if (_t1 == 17l) {
         rocrEnumDB(rocrEDTCtx, 3u, 262144ul, 1l, 0u, (void (*)(void*, long*))(
            (void*)0l), (unsigned long)0l, (unsigned long)17l);
      }
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars211(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 4ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars211_dtenum_32(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 4u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars212(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 5ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars212_dtenum_33(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 5u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars177(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars177_186
      mapped_step5_dpbr_noGVars177_args;
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars177_args = *(union
      __args_mapped_step5_dpbr_noGVars177_186*)rocrArgs(depv);
   if (mapped_step5_dpbr_noGVars177_args.data.IT0 == 0l) {
      rocrFreeDB(_t1, 2, 6ul, (unsigned long)0l);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars177_dtenum_34(void* rocrEDTCtx, long* args)
{
   if ((long)args[0l] == 0l) {
      rocrEnumDB(rocrEDTCtx, 6u, 8998912ul, 2l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)0l);
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars213(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars213_188
      mapped_step5_dpbr_noGVars213_args;
   double* pAp_db_0;
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars213_args = *(union
      __args_mapped_step5_dpbr_noGVars213_188*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&pAp_db_0, 1, 7ul);
   *mapped_step5_dpbr_noGVars213_args.data.pAp = *pAp_db_0;
   rocrFreeDB(_t1, 1, 7ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars213_dtenum_35(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 7u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars214(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars214_190
      mapped_step5_dpbr_noGVars214_args;
   double* alpha_db_0;
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars214_args = *(union
      __args_mapped_step5_dpbr_noGVars214_190*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&alpha_db_0, 1, 8ul);
   *mapped_step5_dpbr_noGVars214_args.data.alpha = *alpha_db_0;
   rocrFreeDB(_t1, 1, 8ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars214_dtenum_36(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 8u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars215(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 9ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars215_dtenum_37(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 9u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l))
      ;
}

static ocrGuid_t mapped_step5_dpbr_noGVars216(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 10ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars216_dtenum_38(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 10u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step5_dpbr_noGVars217(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 11ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars217_dtenum_39(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 11u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step5_dpbr_noGVars181(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars181_198
      mapped_step5_dpbr_noGVars181_args;
   double* x_db_0;
   void* _t1;
   long IT0;
   long _t2;
   double* x_49;
   long i;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars181_args = *(union
      __args_mapped_step5_dpbr_noGVars181_198*)rocrArgs(depv);
   IT0 = mapped_step5_dpbr_noGVars181_args.data.IT0;
   _t2 = (__mins_64(-65536l * IT0 + 1124863l, 65535l));
   rocrFetchDB(_t1, (void**)&x_db_0, 2, 12ul, (unsigned long)IT0);
   for (x_49 = mapped_step5_dpbr_noGVars181_args.data.x, i = 0l; i <= _t2; i++)
       {
      x_49[i + 65536l * IT0] = x_db_0[65536l * IT0 + i + -65536l * (65536l * IT0
         + i >> 16l)];
   }
   rocrFreeDB(_t1, 2, 12ul, (unsigned long)IT0);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars181_dtenum_40(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[0l];
   if (_t1 <= 16l) {
      rocrEnumDB(rocrEDTCtx, 12u, 524288ul, 2l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)_t1);
   }
   if (_t1 == 17l) {
      rocrEnumDB(rocrEDTCtx, 12u, 524288ul, 1l, 0u, (void (*)(void*, long*))((
         void*)0l), (unsigned long)17l);
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars218(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 13ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars218_dtenum_41(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 13u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step5_dpbr_noGVars219(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 14ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars219_dtenum_42(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 14u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step5_dpbr_noGVars185(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars185_204
      mapped_step5_dpbr_noGVars185_args;
   double* r_l_db_0;
   void* _t1;
   double*restrict* r_l;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars185_args = *(union
      __args_mapped_step5_dpbr_noGVars185_204*)rocrArgs(depv);
   IT1 = mapped_step5_dpbr_noGVars185_args.data.IT1;
   r_l = mapped_step5_dpbr_noGVars185_args.data.r_l;
   if (mapped_step5_dpbr_noGVars185_args.data.IT0 == 0l) {
      long _t2;
      long i;
      _t2 = (__mins_64(-65536l * IT1 + 1124863l, 65535l));
      rocrFetchDB(_t1, (void**)&r_l_db_0, 3, 15ul, (unsigned long)0l, (
         unsigned long)IT1);
      for (i = 0l; i <= _t2; i++) {
         r_l[0l][i + 65536l * IT1] = r_l_db_0[65536l * IT1 + i + -65536l * (
            65536l * IT1 + i >> 16l)];
      }
      rocrFreeDB(_t1, 3, 15ul, (unsigned long)0l, (unsigned long)IT1);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars185_dtenum_43(void* rocrEDTCtx, long* args)
{
   long _t1;
   _t1 = (long)args[1l];
   if ((long)args[0l] == 0l) {
      if (_t1 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 15u, 524288ul, 2l, 0u, (void (*)(void*, long*))
            ((void*)0l), (unsigned long)0l, (unsigned long)_t1);
      }
      if (_t1 == 17l) {
         rocrEnumDB(rocrEDTCtx, 15u, 524288ul, 1l, 0u, (void (*)(void*, long*))
            ((void*)0l), (unsigned long)0l, (unsigned long)17l);
      }
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars220(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 16ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars220_dtenum_44(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 16u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step5_dpbr_noGVars221(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   rocrArgs(depv);
   rocrFreeDB(_t1, 1, 17ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars221_dtenum_45(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 17u, 4ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step5_dpbr_noGVars187(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars187_210
      mapped_step5_dpbr_noGVars187_args;
   double* normr_db_0;
   void* _t1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars187_args = *(union
      __args_mapped_step5_dpbr_noGVars187_210*)rocrArgs(depv);
   rocrFetchDB(_t1, (void**)&normr_db_0, 1, 18ul);
   *mapped_step5_dpbr_noGVars187_args.data.normr = *normr_db_0;
   rocrFreeDB(_t1, 1, 18ul);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars187_dtenum_46(void* rocrEDTCtx, long* args)
{
   rocrEnumDB(rocrEDTCtx, 18u, 8ul, 2l, 0u, (void (*)(void*, long*))((void*)0l)
      );
}

static ocrGuid_t mapped_step5_dpbr_noGVars189(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars189_212
      mapped_step5_dpbr_noGVars189_args;
   void* _t1;
   long IT0;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars189_args = *(union
      __args_mapped_step5_dpbr_noGVars189_212*)rocrArgs(depv);
   IT1 = mapped_step5_dpbr_noGVars189_args.data.IT1;
   IT0 = mapped_step5_dpbr_noGVars189_args.data.IT0;
   if (mapped_step5_dpbr_noGVars189_args.data.IT2 == 0l && IT0 == 0l) {
      rocrFreeDB(_t1, 4, 19ul, (unsigned long)0l, (unsigned long)IT1, (
         unsigned long)0l);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars189_dtenum_47(void* rocrEDTCtx, long* args)
{
   long _t1;
   long _t2;
   _t2 = (long)args[1l];
   _t1 = (long)args[0l];
   if ((long)args[2l] == 0l && _t1 == 0l) {
      if (_t2 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 19u, 7077888ul, 2l, 0u, (void (*)(void*, long*)
            )((void*)0l), (unsigned long)0l, (unsigned long)_t2, (unsigned long
            )0l);
      }
      if (_t2 == 17l) {
         rocrEnumDB(rocrEDTCtx, 19u, 7077888ul, 1l, 0u, (void (*)(void*, long*)
            )((void*)0l), (unsigned long)0l, (unsigned long)17l, (unsigned long
            )0l);
      }
   }
}

static ocrGuid_t mapped_step5_dpbr_noGVars192(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars192_214
      mapped_step5_dpbr_noGVars192_args;
   void* _t1;
   long IT0;
   long IT1;
   _t1 = rocrParseInputArgs(paramc, paramv, depc, depv);
   mapped_step5_dpbr_noGVars192_args = *(union
      __args_mapped_step5_dpbr_noGVars192_214*)rocrArgs(depv);
   IT1 = mapped_step5_dpbr_noGVars192_args.data.IT1;
   IT0 = mapped_step5_dpbr_noGVars192_args.data.IT0;
   if (mapped_step5_dpbr_noGVars192_args.data.IT2 == 0l && IT0 == 0l) {
      rocrFreeDB(_t1, 4, 20ul, (unsigned long)0l, (unsigned long)IT1, (
         unsigned long)0l);
   }
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

void mapped_step5_dpbr_noGVars192_dtenum_48(void* rocrEDTCtx, long* args)
{
   long _t1;
   long _t2;
   _t2 = (long)args[1l];
   _t1 = (long)args[0l];
   if ((long)args[2l] == 0l && _t1 == 0l) {
      if (_t2 <= 16l) {
         rocrEnumDB(rocrEDTCtx, 20u, 14155776ul, 2l, 0u, (void (*)(void*, long*
            ))((void*)0l), (unsigned long)0l, (unsigned long)_t2, (
            unsigned long)0l);
      }
      if (_t2 == 17l) {
         rocrEnumDB(rocrEDTCtx, 20u, 14155776ul, 1l, 0u, (void (*)(void*, long*
            ))((void*)0l), (unsigned long)0l, (unsigned long)17l, (
            unsigned long)0l);
      }
   }
}

unsigned long mapped_step5_dpbr_noGVars192_count_48(long* args)
{
   long _t1;
   long _t2;
   unsigned long rcNbInputDeps;
   _t2 = (long)args[1l];
   _t1 = (long)args[0l];
   if ((long)args[2l] == 0l) {
      if (_t1 == 0l) {
         long _t3;
         long i;
         for (_t3 = (__mins_64(137l, 8l * _t2 + 7l)), rcNbInputDeps = 0ul + 1ul
                 , i = 8l * _t2; i <= _t3; i++, rcNbInputDeps++) {
         }
      } else {
         rcNbInputDeps = 0ul;
      }
   } else {
      rcNbInputDeps = 0ul;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars153_count_22(long* args)
{
   long _t1;
   unsigned long rcNbInputDeps;
   long _t2;
   long i;
   long _t3;
   long i_1;
   long _t4;
   long i_2;
   long _t5;
   long i_3;
   for (_t1 = (long)args[0l],
        _t2 = (__mins_64(_t1 >> 3l, 17l)),
        i = (__maxs_64(0l, _t1 + -7l + 7l >> 3l)),
        rcNbInputDeps = 3ul;
        i <= _t2; i++, rcNbInputDeps++) {
   }
   for (_t3 = (__mins_64(_t1 >> 3l, 17l)), i_1 = (__maxs_64(0l, _t1 + -7l + 7l
           >> 3l)); i_1 <= _t3; i_1++, rcNbInputDeps++) {
   }
   for (_t4 = (__mins_64(_t1 >> 3l, 17l)), i_2 = (__maxs_64(0l, _t1 + -7l + 7l
           >> 3l)); i_2 <= _t4; i_2++, rcNbInputDeps++) {
   }
   for (_t5 = (__mins_64(_t1 >> 3l, 17l)), i_3 = (__maxs_64(0l, _t1 + -7l + 7l
           >> 3l)); i_3 <= _t5; i_3++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars189_count_47(long* args)
{
   long _t1;
   long _t2;
   unsigned long rcNbInputDeps;
   _t2 = (long)args[1l];
   _t1 = (long)args[0l];
   if ((long)args[2l] == 0l) {
      if (_t1 == 0l) {
         long _t3;
         long i;
         for (_t3 = (__mins_64(137l, 8l * _t2 + 7l)), rcNbInputDeps = 0ul + 1ul
                 , i = 8l * _t2; i <= _t3; i++, rcNbInputDeps++) {
         }
      } else {
         rcNbInputDeps = 0ul;
      }
   } else {
      rcNbInputDeps = 0ul;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars177_count_34(long* args)
{
   unsigned long rcNbInputDeps;
   if ((long)args[0l] == 0l) {
      long i;
      long i_1;
      long i_2;
      for (rcNbInputDeps = 0ul + 1ul, i = 0l; i <= 137l; i++, rcNbInputDeps++)
         {
      }
      for (i_1 = 0l; i_1 <= 17l; i_1++, rcNbInputDeps++) {
      }
      for (i_2 = 0l; i_2 <= 34l; i_2++, rcNbInputDeps++) {
      }
   } else {
      rcNbInputDeps = 0ul;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars173_count_31(long* args)
{
   long _t1;
   unsigned long rcNbInputDeps;
   _t1 = (long)args[1l];
   if ((long)args[0l] == 0l) {
      long _t2;
      long i;
      for (_t2 = (__mins_64(137l, 8l * _t1 + 7l)), rcNbInputDeps = 0ul + 1ul, i
              = 8l * _t1; i <= _t2; i++, rcNbInputDeps++) {
      }
   } else {
      rcNbInputDeps = 0ul;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars170_count_30(long* args)
{
   long _t1;
   unsigned long rcNbInputDeps;
   long _t2;
   long i;
   long _t3;
   long i_1;
   for (_t1 = (long)args[0l],
        _t2 = (__mins_64(137l, 8l * _t1 + 7l)),
        rcNbInputDeps = 0ul + 2ul,
        i = 8l * _t1;
        i <= _t2; i++, rcNbInputDeps++) {
   }
   for (_t3 = (__mins_64(34l, 2l * _t1 + 1l)), i_1 = 2l * _t1; i_1 <= _t3; i_1
           ++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars209_count_29(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 1ul, i = 0l; i <= 137l; i++, rcNbInputDeps
           ++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars160_count_25(long* args)
{
   long _t1;
   unsigned long rcNbInputDeps;
   long _t2;
   long i;
   long _t3;
   long i_1;
   long _t4;
   long i_2;
   long _t5;
   long i_3;
   for (_t1 = (long)args[0l],
        _t2 = _t1 >> 1l,
        i = (__maxs_64(0l, _t1 + -1l + 1l >> 1l)),
        rcNbInputDeps = 7ul;
        i <= _t2; i++, rcNbInputDeps++) {
   }
   for (_t3 = _t1 >> 1l, i_1 = (__maxs_64(0l, _t1 + -1l + 1l >> 1l)); i_1 <= _t3
           ; i_1++, rcNbInputDeps++) {
   }
   for (_t4 = _t1 >> 1l, i_2 = (__maxs_64(0l, _t1 + -1l + 1l >> 1l)); i_2 <= _t4
           ; i_2++, rcNbInputDeps++) {
   }
   for (_t5 = (__mins_64(137l, 4l * _t1 + 3l)), i_3 = 4l * _t1; i_3 <= _t5; i_3
           ++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars156_count_23(long* args)
{
   long _t1;
   unsigned long rcNbInputDeps;
   unsigned long rcNbInputDeps_1;
   long _t2;
   unsigned long _t3;
   long i;
   for (_t1 = (long)args[0l],
        _t2 = (__mins_64(137l, 8l * _t1 + 7l)),
        _t3 = 0ul + 1ul + 1ul + 1ul + 1ul,
        rcNbInputDeps = _t3 + 1ul,
        i = 8l * _t1,
        rcNbInputDeps_1 = _t3 + 2ul;
        i <= _t2; i++, rcNbInputDeps++, rcNbInputDeps_1++) {
   }
   if (_t1 >= 1l) {
      rcNbInputDeps = rcNbInputDeps_1;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars185_count_43(long* args)
{
   long _t1;
   unsigned long rcNbInputDeps;
   _t1 = (long)args[1l];
   if ((long)args[0l] == 0l) {
      long _t2;
      long i;
      long _t3;
      long i_1;
      for (_t2 = (__mins_64(34l, 2l * _t1 + 1l)), rcNbInputDeps = 0ul + 1ul, i =
              2l * _t1; i <= _t2; i++, rcNbInputDeps++) {
      }
      for (_t3 = (__mins_64(_t1 >> 1l, 8l)), i_1 = (__maxs_64(0l, _t1 + -1l + 1l
              >> 1l)); i_1 <= _t3; i_1++, rcNbInputDeps++) {
      }
   } else {
      rcNbInputDeps = 0ul;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars219_count_42(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 1ul, i = 0l; i <= 34l; i++, rcNbInputDeps++
      ) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars218_count_41(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 1ul, i = 0l; i <= 34l; i++, rcNbInputDeps++
      ) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars181_count_40(long* args)
{
   unsigned long rcNbInputDeps;
   long _t1;
   long _t2;
   long i;
   for (_t1 = (long)args[0l],
        _t2 = (__mins_64(34l, 2l * _t1 + 1l)),
        rcNbInputDeps = 0ul + 1ul,
        i = 2l * _t1;
        i <= _t2; i++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars217_count_39(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 1ul, i = 0l; i <= 34l; i++, rcNbInputDeps++
      ) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars216_count_38(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 1ul, i = 0l; i <= 34l; i++, rcNbInputDeps++
      ) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars163_count_26(long* args)
{
   long _t1;
   unsigned long rcNbInputDeps;
   unsigned long rcNbInputDeps_1;
   long _t2;
   unsigned long _t3;
   long i;
   long _t4;
   long i_1;
   for (_t1 = (long)args[0l],
        _t2 = 2l * _t1 + 1l,
        _t3 = 0ul + 1ul,
        rcNbInputDeps_1 = _t3 + 3ul,
        i = 2l * _t1,
        rcNbInputDeps = _t3 + 2ul;
        i <= _t2; i++, rcNbInputDeps_1++, rcNbInputDeps++) {
   }
   for (_t4 = (__mins_64(34l, 4l * _t1 + 3l)), i_1 = 4l * _t1; i_1 <= _t4; i_1
           ++, rcNbInputDeps++, rcNbInputDeps_1++) {
   }
   if (_t1 >= 1l) {
      rcNbInputDeps = rcNbInputDeps_1;
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars221_count_45(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 1ul, i = 0l; i <= 8l; i++, rcNbInputDeps++)
       {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars220_count_44(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 2ul, i = 0l; i <= 8l; i++, rcNbInputDeps++)
       {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars165_count_27(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 1ul, i = 0l; i <= 8l; i++, rcNbInputDeps++)
       {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars187_count_46(long* args)
{
   return 0ul + 1ul;
}

unsigned long mapped_step5_dpbr_noGVars212_count_33(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 1ul, i = 0l; i <= 17l; i++, rcNbInputDeps++
      ) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars211_count_32(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 2ul, i = 0l; i <= 17l; i++, rcNbInputDeps++
      ) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars158_count_24(long* args)
{
   unsigned long rcNbInputDeps;
   long i;
   for (rcNbInputDeps = 0ul + 1ul + 1ul + 1ul + 1ul + 1ul + 1ul + 1ul, i = 0l;
           i <= 17l; i++, rcNbInputDeps++) {
   }
   return rcNbInputDeps;
}

unsigned long mapped_step5_dpbr_noGVars215_count_37(long* args)
{
   return 0ul + 1ul + 1ul;
}

unsigned long mapped_step5_dpbr_noGVars214_count_36(long* args)
{
   return 0ul + 1ul + 1ul;
}

unsigned long mapped_step5_dpbr_noGVars213_count_35(long* args)
{
   return 0ul + 1ul + 1ul;
}

unsigned long mapped_step5_dpbr_noGVars151_count_21(long* args)
{
   return 0ul + 1ul + 1ul + 1ul + 1ul + 1ul + 1ul + 1ul + 1ul;
}

unsigned long mapped_step5_dpbr_noGVars208_count_28(long* args)
{
   return 0ul + 1ul + 1ul;
}

static ocrGuid_t mapped_step5_dpbr_noGVars_main(unsigned int paramc,
   unsigned long* paramv, unsigned int depc, ocrEdtDep_t* depv)
{
   union __args_mapped_step5_dpbr_noGVars_main_216
      mapped_step5_dpbr_noGVars_main_args;
   union __args_mapped_step5_dpbr_noGVars124_120* allArgs;
   union __args_mapped_step5_dpbr_noGVars127_122* allArgs_1;
   union __args_mapped_step5_dpbr_noGVars195_124* allArgs_2;
   union __args_mapped_step5_dpbr_noGVars129_126* allArgs_3;
   union __args_mapped_step5_dpbr_noGVars131_128* allArgs_4;
   union __args_mapped_step5_dpbr_noGVars197_130* allArgs_5;
   union __args_mapped_step5_dpbr_noGVars133_132* allArgs_6;
   union __args_mapped_step5_dpbr_noGVars135_134* allArgs_7;
   union __args_mapped_step5_dpbr_noGVars199_136* allArgs_8;
   union __args_mapped_step5_dpbr_noGVars200_138* allArgs_9;
   union __args_mapped_step5_dpbr_noGVars201_140* allArgs_10;
   union __args_mapped_step5_dpbr_noGVars202_142* allArgs_11;
   union __args_mapped_step5_dpbr_noGVars137_144* allArgs_12;
   union __args_mapped_step5_dpbr_noGVars139_146* allArgs_13;
   union __args_mapped_step5_dpbr_noGVars204_148* allArgs_14;
   union __args_mapped_step5_dpbr_noGVars205_150* allArgs_15;
   union __args_mapped_step5_dpbr_noGVars143_152* allArgs_16;
   union __args_mapped_step5_dpbr_noGVars146_154* allArgs_17;
   union __args_mapped_step5_dpbr_noGVars206_156* allArgs_18;
   union __args_mapped_step5_dpbr_noGVars207_158* allArgs_19;
   void* _t1;
   double* Ap_l;
   double*restrict* r_l;
   double* x_50;
   double* normr_55;
   double* alpha;
   double* pAp;
   int (*restrict* colInds_l)[27];
   double* rtz;
   double* p_l;
   int*restrict* nnzPerRow_l;
   int const* nPerLvl_l;
   int* _t2;
   double* _t3;
   int* _t4;
   double* _t5;
   int* _t6;
   double* _t7;
   int* _t8;
   double* _t9;
   int* _t10;
   ocrGuid_t _t11;
   union __args_mapped_step5_dpbr_noGVars195_124* _t12;
   ocrGuid_t _t13;
   union __args_mapped_step5_dpbr_noGVars129_126* _t14;
   ocrGuid_t _t15;
   union __args_mapped_step5_dpbr_noGVars197_130* _t16;
   ocrGuid_t _t17;
   union __args_mapped_step5_dpbr_noGVars133_132* _t18;
   ocrGuid_t _t19;
   union __args_mapped_step5_dpbr_noGVars199_136* _t20;
   ocrGuid_t _t21;
   union __args_mapped_step5_dpbr_noGVars200_138* _t22;
   ocrGuid_t _t23;
   union __args_mapped_step5_dpbr_noGVars201_140* _t24;
   ocrGuid_t _t25;
   union __args_mapped_step5_dpbr_noGVars202_142* _t26;
   ocrGuid_t _t27;
   union __args_mapped_step5_dpbr_noGVars137_144* _t28;
   ocrGuid_t _t29;
   union __args_mapped_step5_dpbr_noGVars139_146* _t30;
   ocrGuid_t _t31;
   union __args_mapped_step5_dpbr_noGVars204_148* _t32;
   ocrGuid_t _t33;
   union __args_mapped_step5_dpbr_noGVars205_150* _t34;
   ocrGuid_t _t35;
   union __args_mapped_step5_dpbr_noGVars206_156* _t36;
   ocrGuid_t _t37;
   union __args_mapped_step5_dpbr_noGVars207_158* _t38;
   double (*restrict* A_l)[27];
   void* _t39;
   void* _t40;
   void* _t41;
   void* _t42;
   void* _t43;
   void* _t44;
   void* _t45;
   void* _t46;
   long i;
   long i_1;
   long i_2;
   long i_3;
   long i_4;
   long i_5;
   for (_t1 = rocrParseInputArgs(paramc, paramv, depc, depv),
        mapped_step5_dpbr_noGVars_main_args = *(union
           __args_mapped_step5_dpbr_noGVars_main_216*)rocrArgs(depv),
        _t39 = malloc(4ul),
        _t40 = malloc(8ul),
        _t41 = malloc(4ul),
        _t42 = malloc(8ul),
        _t43 = malloc(4ul),
        _t44 = malloc(8ul),
        _t45 = malloc(4ul),
        _t46 = malloc(8ul),
        _t10 = (int*)malloc(4ul),
        _t9 = (double*)_t46,
        _t8 = (int*)_t45,
        _t7 = (double*)_t44,
        _t6 = (int*)_t43,
        _t5 = (double*)_t42,
        _t4 = (int*)_t41,
        _t3 = (double*)_t40,
        _t2 = (int*)_t39,
        nPerLvl_l = mapped_step5_dpbr_noGVars_main_args.data.nPerLvl_l,
        nnzPerRow_l = mapped_step5_dpbr_noGVars_main_args.data.nnzPerRow_l,
        p_l = mapped_step5_dpbr_noGVars_main_args.data.p_l,
        rtz = mapped_step5_dpbr_noGVars_main_args.data.rtz,
        colInds_l = mapped_step5_dpbr_noGVars_main_args.data.colInds_l,
        pAp = mapped_step5_dpbr_noGVars_main_args.data.pAp,
        alpha = mapped_step5_dpbr_noGVars_main_args.data.alpha,
        normr_55 = mapped_step5_dpbr_noGVars_main_args.data.normr,
        x_50 = mapped_step5_dpbr_noGVars_main_args.data.x,
        r_l = mapped_step5_dpbr_noGVars_main_args.data.r_l,
        Ap_l = mapped_step5_dpbr_noGVars_main_args.data.Ap_l,
        A_l = mapped_step5_dpbr_noGVars_main_args.data.A_l,
        i = 0l;
        i <= 17l; i++) {
      ocrGuid_t _t47;
      union __args_mapped_step5_dpbr_noGVars124_120* _t48;
      _t47 = rocrAlloc((void**)&allArgs, 80ul);
      _t48 = allArgs;
      _t48->data.A_l = A_l;
      _t48->data.Ap_l = Ap_l;
      _t48->data.r_l = r_l;
      _t48->data.x = x_50;
      _t48->data.normr = normr_55;
      _t48->data.alpha = alpha;
      _t48->data.pAp = pAp;
      _t48->data.IT0 = 0l;
      _t48->data.IT1 = i;
      _t48->data.IT2 = 0l;
      rocrAutoDec(_t1, 1u, (unsigned long)i, _t47, (unsigned long (*)(long*))((
         void*)0l), mapped_step5_dpbr_noGVars124_dtenum_1, (void (*)(void*,
         long*))((void*)0l), 3u, (long)0l, (long)i, (long)0l);
   }
   for (i_1 = 0l; i_1 <= 17l; i_1++) {
      ocrGuid_t _t49;
      union __args_mapped_step5_dpbr_noGVars127_122* _t50;
      _t49 = rocrAlloc((void**)&allArgs_1, 80ul);
      _t50 = allArgs_1;
      _t50->data.colInds_l = colInds_l;
      _t50->data.Ap_l = Ap_l;
      _t50->data.r_l = r_l;
      _t50->data.x = x_50;
      _t50->data.normr = normr_55;
      _t50->data.alpha = alpha;
      _t50->data.pAp = pAp;
      _t50->data.IT0 = 0l;
      _t50->data.IT1 = i_1;
      _t50->data.IT2 = 0l;
      rocrAutoDec(_t1, 2u, (unsigned long)i_1, _t49, (unsigned long (*)(long*))
         ((void*)0l), mapped_step5_dpbr_noGVars127_dtenum_2, (void (*)(void*,
         long*))((void*)0l), 3u, (long)0l, (long)i_1, (long)0l);
   }
   _t11 = rocrAlloc((void**)&allArgs_2, 56ul);
   _t12 = allArgs_2;
   _t12->data._p = _t2;
   _t12->data.Ap_l = Ap_l;
   _t12->data.r_l = r_l;
   _t12->data.x = x_50;
   _t12->data.normr = normr_55;
   _t12->data.alpha = alpha;
   _t12->data.pAp = pAp;
   rocrAutoDec(_t1, 3u, (unsigned long)0l, _t11, (unsigned long (*)(long*))((
      void*)0l), mapped_step5_dpbr_noGVars195_dtenum_3, (void (*)(void*, long*)
      )((void*)0l), 0u);
   _t13 = rocrAlloc((void**)&allArgs_3, 56ul);
   _t14 = allArgs_3;
   _t14->data._p = _t3;
   _t14->data.Ap_l = Ap_l;
   _t14->data.r_l = r_l;
   _t14->data.x = x_50;
   _t14->data.normr = normr_55;
   _t14->data.alpha = alpha;
   _t14->data.pAp = pAp;
   rocrAutoDec(_t1, 4u, (unsigned long)0l, _t13, (unsigned long (*)(long*))((
      void*)0l), mapped_step5_dpbr_noGVars129_dtenum_4, (void (*)(void*, long*)
      )((void*)0l), 0u);
   for (i_2 = 0l; i_2 <= 17l; i_2++) {
      ocrGuid_t _t51;
      union __args_mapped_step5_dpbr_noGVars131_128* _t52;
      _t51 = rocrAlloc((void**)&allArgs_4, 48ul);
      _t52 = allArgs_4;
      _t52->data.r_l = r_l;
      _t52->data.Ap_l = Ap_l;
      _t52->data.x = x_50;
      _t52->data.normr = normr_55;
      _t52->data.IT0 = 0l;
      _t52->data.IT1 = i_2;
      rocrAutoDec(_t1, 5u, (unsigned long)i_2, _t51, (unsigned long (*)(long*))
         ((void*)0l), mapped_step5_dpbr_noGVars131_dtenum_5, (void (*)(void*,
         long*))((void*)0l), 2u, (long)0l, (long)i_2);
   }
   _t15 = rocrAlloc((void**)&allArgs_5, 56ul);
   _t16 = allArgs_5;
   _t16->data._p = _t4;
   _t16->data.Ap_l = Ap_l;
   _t16->data.r_l = r_l;
   _t16->data.x = x_50;
   _t16->data.normr = normr_55;
   _t16->data.alpha = alpha;
   _t16->data.pAp = pAp;
   rocrAutoDec(_t1, 6u, (unsigned long)0l, _t15, (unsigned long (*)(long*))((
      void*)0l), mapped_step5_dpbr_noGVars197_dtenum_6, (void (*)(void*, long*)
      )((void*)0l), 0u);
   _t17 = rocrAlloc((void**)&allArgs_6, 56ul);
   _t18 = allArgs_6;
   _t18->data._p = _t5;
   _t18->data.Ap_l = Ap_l;
   _t18->data.r_l = r_l;
   _t18->data.x = x_50;
   _t18->data.normr = normr_55;
   _t18->data.alpha = alpha;
   _t18->data.pAp = pAp;
   rocrAutoDec(_t1, 7u, (unsigned long)0l, _t17, (unsigned long (*)(long*))((
      void*)0l), mapped_step5_dpbr_noGVars133_dtenum_7, (void (*)(void*, long*)
      )((void*)0l), 0u);
   for (i_3 = 0l; i_3 <= 17l; i_3++) {
      ocrGuid_t _t53;
      union __args_mapped_step5_dpbr_noGVars135_134* _t54;
      _t53 = rocrAlloc((void**)&allArgs_7, 40ul);
      _t54 = allArgs_7;
      _t54->data.x = x_50;
      _t54->data.Ap_l = Ap_l;
      _t54->data.r_l = r_l;
      _t54->data.normr = normr_55;
      _t54->data.IT0 = i_3;
      rocrAutoDec(_t1, 8u, (unsigned long)i_3, _t53, (unsigned long (*)(long*))
         ((void*)0l), mapped_step5_dpbr_noGVars135_dtenum_8, (void (*)(void*,
         long*))((void*)0l), 1u, (long)i_3);
   }
   _t19 = rocrAlloc((void**)&allArgs_8, 56ul);
   _t20 = allArgs_8;
   _t20->data._p = _t6;
   _t20->data.Ap_l = Ap_l;
   _t20->data.r_l = r_l;
   _t20->data.x = x_50;
   _t20->data.normr = normr_55;
   _t20->data.alpha = alpha;
   _t20->data.pAp = pAp;
   rocrAutoDec(_t1, 9u, (unsigned long)0l, _t19, (unsigned long (*)(long*))((
      void*)0l), mapped_step5_dpbr_noGVars199_dtenum_9, (void (*)(void*, long*)
      )((void*)0l), 0u);
   _t21 = rocrAlloc((void**)&allArgs_9, 56ul);
   _t22 = allArgs_9;
   _t22->data._p = _t7;
   _t22->data.Ap_l = Ap_l;
   _t22->data.r_l = r_l;
   _t22->data.x = x_50;
   _t22->data.normr = normr_55;
   _t22->data.alpha = alpha;
   _t22->data.pAp = pAp;
   rocrAutoDec(_t1, 10u, (unsigned long)0l, _t21, (unsigned long (*)(long*))((
      void*)0l), mapped_step5_dpbr_noGVars200_dtenum_10, (void (*)(void*, long*
      ))((void*)0l), 0u);
   _t23 = rocrAlloc((void**)&allArgs_10, 56ul);
   _t24 = allArgs_10;
   _t24->data.rtz = rtz;
   _t24->data.Ap_l = Ap_l;
   _t24->data.r_l = r_l;
   _t24->data.x = x_50;
   _t24->data.normr = normr_55;
   _t24->data.alpha = alpha;
   _t24->data.pAp = pAp;
   rocrAutoDec(_t1, 11u, (unsigned long)0l, _t23, (unsigned long (*)(long*))((
      void*)0l), mapped_step5_dpbr_noGVars201_dtenum_11, (void (*)(void*, long*
      ))((void*)0l), 0u);
   _t25 = rocrAlloc((void**)&allArgs_11, 48ul);
   _t26 = allArgs_11;
   _t26->data.alpha = alpha;
   _t26->data.Ap_l = Ap_l;
   _t26->data.r_l = r_l;
   _t26->data.x = x_50;
   _t26->data.normr = normr_55;
   _t26->data.pAp = pAp;
   rocrAutoDec(_t1, 12u, (unsigned long)0l, _t25, (unsigned long (*)(long*))((
      void*)0l), mapped_step5_dpbr_noGVars202_dtenum_12, (void (*)(void*, long*
      ))((void*)0l), 0u);
   _t27 = rocrAlloc((void**)&allArgs_12, 48ul);
   _t28 = allArgs_12;
   _t28->data.pAp = pAp;
   _t28->data.Ap_l = Ap_l;
   _t28->data.r_l = r_l;
   _t28->data.x = x_50;
   _t28->data.normr = normr_55;
   _t28->data.alpha = alpha;
   rocrAutoDec(_t1, 13u, (unsigned long)0l, _t27, (unsigned long (*)(long*))((
      void*)0l), mapped_step5_dpbr_noGVars137_dtenum_13, (void (*)(void*, long*
      ))((void*)0l), 0u);
   _t29 = rocrAlloc((void**)&allArgs_13, 64ul);
   _t30 = allArgs_13;
   _t30->data.p_l = p_l;
   _t30->data.Ap_l = Ap_l;
   _t30->data.r_l = r_l;
   _t30->data.x = x_50;
   _t30->data.normr = normr_55;
   _t30->data.alpha = alpha;
   _t30->data.pAp = pAp;
   _t30->data.IT0 = 0l;
   rocrAutoDec(_t1, 14u, (unsigned long)0l, _t29, (unsigned long (*)(long*))((
      void*)0l), mapped_step5_dpbr_noGVars139_dtenum_14, (void (*)(void*, long*
      ))((void*)0l), 1u, (long)0l);
   _t31 = rocrAlloc((void**)&allArgs_14, 56ul);
   _t32 = allArgs_14;
   _t32->data._p = _t8;
   _t32->data.Ap_l = Ap_l;
   _t32->data.r_l = r_l;
   _t32->data.x = x_50;
   _t32->data.normr = normr_55;
   _t32->data.alpha = alpha;
   _t32->data.pAp = pAp;
   rocrAutoDec(_t1, 15u, (unsigned long)0l, _t31, (unsigned long (*)(long*))((
      void*)0l), mapped_step5_dpbr_noGVars204_dtenum_15, (void (*)(void*, long*
      ))((void*)0l), 0u);
   _t33 = rocrAlloc((void**)&allArgs_15, 56ul);
   _t34 = allArgs_15;
   _t34->data._p = _t9;
   _t34->data.Ap_l = Ap_l;
   _t34->data.r_l = r_l;
   _t34->data.x = x_50;
   _t34->data.normr = normr_55;
   _t34->data.alpha = alpha;
   _t34->data.pAp = pAp;
   rocrAutoDec(_t1, 16u, (unsigned long)0l, _t33, (unsigned long (*)(long*))((
      void*)0l), mapped_step5_dpbr_noGVars205_dtenum_16, (void (*)(void*, long*
      ))((void*)0l), 0u);
   for (i_4 = 0l; i_4 <= 17l; i_4++) {
      ocrGuid_t _t55;
      union __args_mapped_step5_dpbr_noGVars143_152* _t56;
      _t55 = rocrAlloc((void**)&allArgs_16, 72ul);
      _t56 = allArgs_16;
      _t56->data.nnzPerRow_l = nnzPerRow_l;
      _t56->data.Ap_l = Ap_l;
      _t56->data.r_l = r_l;
      _t56->data.x = x_50;
      _t56->data.normr = normr_55;
      _t56->data.alpha = alpha;
      _t56->data.pAp = pAp;
      _t56->data.IT0 = 0l;
      _t56->data.IT1 = i_4;
      rocrAutoDec(_t1, 17u, (unsigned long)i_4, _t55, (unsigned long (*)(long*)
         )((void*)0l), mapped_step5_dpbr_noGVars143_dtenum_17, (void (*)(void*,
          long*))((void*)0l), 2u, (long)0l, (long)i_4);
   }
   for (i_5 = 0l; i_5 <= 17l; i_5++) {
      ocrGuid_t _t57;
      union __args_mapped_step5_dpbr_noGVars146_154* _t58;
      _t57 = rocrAlloc((void**)&allArgs_17, 56ul);
      _t58 = allArgs_17;
      _t58->data.Ap_l = Ap_l;
      _t58->data.r_l = r_l;
      _t58->data.x = x_50;
      _t58->data.normr = normr_55;
      _t58->data.alpha = alpha;
      _t58->data.pAp = pAp;
      _t58->data.IT0 = i_5;
      rocrAutoDec(_t1, 18u, (unsigned long)i_5, _t57, (unsigned long (*)(long*)
         )((void*)0l), mapped_step5_dpbr_noGVars146_dtenum_18, (void (*)(void*,
          long*))((void*)0l), 1u, (long)i_5);
   }
   _t35 = rocrAlloc((void**)&allArgs_18, 56ul);
   _t36 = allArgs_18;
   _t36->data._p = _t10;
   _t36->data.Ap_l = Ap_l;
   _t36->data.r_l = r_l;
   _t36->data.x = x_50;
   _t36->data.normr = normr_55;
   _t36->data.alpha = alpha;
   _t36->data.pAp = pAp;
   rocrAutoDec(_t1, 19u, (unsigned long)0l, _t35, (unsigned long (*)(long*))((
      void*)0l), mapped_step5_dpbr_noGVars206_dtenum_19, (void (*)(void*, long*
      ))((void*)0l), 0u);
   _t37 = rocrAlloc((void**)&allArgs_19, 56ul);
   _t38 = allArgs_19;
   _t38->data.nPerLvl_l = nPerLvl_l;
   _t38->data.Ap_l = Ap_l;
   _t38->data.r_l = r_l;
   _t38->data.x = x_50;
   _t38->data.normr = normr_55;
   _t38->data.alpha = alpha;
   _t38->data.pAp = pAp;
   rocrAutoDec(_t1, 20u, (unsigned long)0l, _t37, (unsigned long (*)(long*))((
      void*)0l), mapped_step5_dpbr_noGVars207_dtenum_20, (void (*)(void*, long*
      ))((void*)0l), 0u);
   rocrAsyncFree(_t2);
   rocrAsyncFree(_t3);
   rocrAsyncFree(_t4);
   rocrAsyncFree(_t5);
   rocrAsyncFree(_t6);
   rocrAsyncFree(_t7);
   rocrAsyncFree(_t8);
   rocrAsyncFree(_t9);
   rocrAsyncFree(_t10);
   rocrFreeInputArgs(_t1, paramc, paramv, depc, depv);
   return NULL_GUID;
}

/*
 * ../src/hpcg_precond_fissioned.c:684.6
 */
void mapped_step5_async(void* event, double* alpha, double rtz, double* pAp,
   double* x_51, double* normr_56)
{
   double rtz_var;
   rtz_var = rtz;
   mapped_step5_dpbr(alpha, &rtz_var, pAp, x_51, normr_56);
}


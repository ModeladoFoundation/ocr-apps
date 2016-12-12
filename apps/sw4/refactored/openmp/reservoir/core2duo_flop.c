// IRAD rights

// definition file of flop counts for clauss and feautrier,
// core 2 duo machines.
// Those were measured, using the advertised flops/sec on the machines.
#define add_flop_f  1
#define add_flop_d  1
#define sub_flop_f  1
#define sub_flop_d  1
#define mul_flop_f  1
#define mul_flop_d  1
#define div_flop_f  1
#define div_flop_d  1
#define sqrt_flop_f  10
#define sqrt_flop_d  16
#define cos_flop_f  10
#define cos_flop_d  16

#define INCLUDE_FLOP_C
#include"flop.c"

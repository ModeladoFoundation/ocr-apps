/**
 * Stencil code provided by D. Richards
 * based on SW4 - Seismic Waves, 4th order
 *
 * FF2 rights
 *
 * core_fn1 represent the core computation while core_fn2 and 3 are some
 * adjustments for boundaries
 */

#include "micro_kernels.h"

#include <stdlib.h>
#include <math.h>

// In the original code, the problem size depends on the function parameters
// and is not defined as it is here.
#define NI 25
#define NJ 25
#define NK 25
#define NC 3

#define float_sw4 double

// original program -osl and -osu options
#define OSU 0
#define OSL 0

static int onesided[6] = {0, 0, 0, 0, 0, 0};
static float_sw4 acof[8][8][6];
static float_sw4 bope[8][6];
static float_sw4 ghcof[6];
static float_sw4 (*lu)[NJ][NI][NC];
static float_sw4 (*u)[NJ][NI][NC];
static float_sw4 (*mu)[NJ][NI];
static float_sw4 (*lambda)[NJ][NI];
static float_sw4 strx[NI];
static float_sw4 stry[NJ];
static float_sw4 strz[NK];

static float_sw4 (*eqs)[NJ][NI][NC];
static float_sw4 (*rho)[NJ][NI];

// Utility functions only there to generate some random input datasets
// converted from the original C++ code

static void get_data(float_sw4 x, float_sw4 y, float_sw4 z, float_sw4 *u,
    float_sw4 *v, float_sw4 *w, float_sw4 *mu, float_sw4 *lambda,
    float_sw4 *rho)
{
    *lambda = cos(x) * pow(sin(3 * y), 2) * cos(z);
    *mu     = sin(3 * x) * sin(y) * sin(z);
    *rho    = x * x * x + 1 + y * y + z * z;
    *u      = cos(x * x) * sin(y * x) * z * z;
    *v      = sin(x) * cos(y * y) * sin(z);
    *w      = cos(x * y) * sin(z * y);
}


static void fg(double x, double y, double z, float_sw4 eqs[3]) {
   double t1;
   double t10;
   double t109;
   double t11;
   double t112;
   double t113;
   double t117;
   double t119;
   double t120;
   double t126;
   double t127;
   double t13;
   double t135;
   double t14;
   double t148;
   double t149;
   double t150;
   double t151;
   double t152;
   double t159;
   double t16;
   double t17;
   double t173;
   double t175;
   double t18;
   double t187;
   double t19;
   double t190;
   double t2;
   double t20;
   double t21;
   double t22;
   double t24;
   double t25;
   double t26;
   double t27;
   double t29;
   double t3;
   double t32;
   double t33;
   double t36;
   double t37;
   double t38;
   double t39;
   double t4;
   double t41;
   double t5;
   double t50;
   double t51;
   double t56;
   double t59;
   double t61;
   double t64;
   double t69;
   double t70;
   double t71;
   double t72;
   double t75;
   double t77;
   double t78;
   double t79;
   double t8;
   double t85;
   double t88;
   double t9;
   double t95;
   double t99;
   {
      t1 = 3.0*x;
      t2 = cos(t1);
      t3 = sin(y);
      t4 = t2*t3;
      t5 = sin(z);
      t8 = sin(x);
      t9 = 3.0*y;
      t10 = sin(t9);
      t11 = t10*t10;
      t13 = cos(z);
      t14 = t8*t11*t13;
      t16 = x*x;
      t17 = sin(t16);
      t18 = t17*x;
      t19 = y*x;
      t20 = sin(t19);
      t21 = z*z;
      t22 = t20*t21;
      t24 = 2.0*t18*t22;
      t25 = cos(t16);
      t26 = cos(t19);
      t27 = t25*t26;
      t29 = t27*y*t21;
      t32 = sin(t1);
      t33 = t32*t3;
      t36 = cos(x);
      t37 = t36*t11;
      t38 = t37*t13;
      t39 = 2.0*t33*t5+t38;
      t41 = t25*t16*t22;
      t50 = t25*t20;
      t51 = y*y;
      t56 = t8*t8;
      t59 = sin(t51);
      t61 = t59*y*t5;
      t64 = t36*t36;
      t69 = z*y;
      t70 = cos(t69);
      t71 = t26*t70;
      t72 = t71*y;
      t75 = t20*t51*t70;
      t77 = cos(y);
      t78 = t32*t77;
      t79 = cos(t51);
      t85 = t5*(t36*t79*t5+t27*x*t21);
      t88 = y*t5;
      t95 = sin(t69);
      t99 = -t20*y*t95+2.0*t50*z;
      t109 = 1/(t16*x+1.0+t51+t21);
      eqs[0] = ((6.0*t4*t5-t14)*(-t24+t29)+t39*(-4.0*t41-2.0*t17*t20*t21-4.0*
        t18*t26*y*t21-t50*t51*t21)+2.0*t56*t11*t13*t61-2.0*t64*t11*t13*t61-t14*
        t72-t38*t75+t78*t85+t33*t5*(-2.0*t36*t59*t88-t41)+t33*t13*t99+t33*t5*
        (-t75+2.0*t50))*t109;
      t112 = t8*t79;
      t113 = t112*t5;
      t117 = 2.0*t17*t16*t26*t21;
      t119 = t50*t19*t21;
      t120 = t27*t21;
      t126 = t36*t10;
      t127 = cos(t9);
      t135 = t39*t8;
      t148 = t20*x;
      t149 = t70*y;
      t150 = t148*t149;
      t151 = t26*t95;
      t152 = t151*t69;
      t159 = -t148*t95+t71*z+t112*t13;
      eqs[1] = (3.0*t4*t85+t33*t5*(-t113-t117-t119+t120)-2.0*(2.0*t78*t5+6.0*
        t126*t13*t127)*t8*t61-4.0*t135*t79*t51*t5-2.0*t135*t59*t5+6.0*t126*t13*
        (-t24+t29+t72)*t127+t37*t13*(-t117-t119+t120-t150-t152+t71)+t33*t13*
        t159+t33*t5*(-t150-t152+t71-t113))*t109;
      t173 = 4.0*t18*t20*z;
      t175 = 2.0*t27*t69;
      t187 = t8*t59;
      t190 = 2.0*t187*y*t13;
      eqs[2] = (3.0*t4*t5*t99+t33*t5*(-t26*t51*t95-t173+t175)+t78*t5*t159+t33*
        t5*(-t26*t16*t95-2.0*t148*t70*z-t151*t21-t190)+(2.0*t33*t13-t37*t5)*
        t26*t149-t39*t26*t95*t51-t37*t5*(-t24+t29-2.0*t187*t88)+t37*t13*(-t173
        +t175-t190))*t109;

      return;
   }
}

// more utility function used in initialization, translated by f2c
// Those functions are GPL-protected (see SW4 code)

/* Subroutine */ static int wavepropbop_4__(double *iop, double *iop2,
	double *bop, double *bop2, double *gh2, double *h__,
	double *s)
{
/* ********************************************************************** */
/* ** */
/* ** SBP operator of order 2/4 for approximating 1st derivative. 5pt stencil. */
/* ** */
/* ********************************************************************** */
/* ** Norm */
    /* Parameter adjustments */
    --h__;
    bop2 -= 5;
    bop -= 5;
    --iop2;
    --iop;

    /* Function Body */
    h__[1] = .35416666666666669;
    h__[2] = 1.2291666666666667;
    h__[3] = .89583333333333337;
    h__[4] = 1.0208333333333333;
/* ** First derivative interior, 4th order */
    iop[1] = .083333333333333329;
    iop[2] = -.66666666666666663;
    iop[3] = 0.;
    iop[4] = .66666666666666663;
    iop[5] = -.083333333333333329;
/* ** First derivative bop, 2nd order */
    bop[5] = -1.411764705882353;
    bop[9] = 1.7352941176470589;
    bop[13] = -.23529411764705882;
    bop[17] = -.088235294117647065;
    bop[21] = 0.;
    bop[25] = 0.;
    bop[6] = -.5;
    bop[10] = 0.;
    bop[14] = .5;
    bop[18] = 0.;
    bop[22] = 0.;
    bop[26] = 0.;
    bop[7] = .093023255813953487;
    bop[11] = -.68604651162790697;
    bop[15] = 0.;
    bop[19] = .68604651162790697;
    bop[23] = -.093023255813953487;
    bop[27] = 0.;
    bop[8] = .030612244897959183;
    bop[12] = 0.;
    bop[16] = -.60204081632653061;
    bop[20] = 0.;
    bop[24] = .65306122448979587;
    bop[28] = -.081632653061224483;
/* ** Second derivative interior, 4th order */
    iop2[1] = -.083333333333333329;
    iop2[2] = 1.3333333333333333;
    iop2[3] = -2.5;
    iop2[4] = 1.3333333333333333;
    iop2[5] = -.083333333333333329;
/* ** Second derivative bop, 2nd order */
    *gh2 = .70588235294117652;
    bop2[5] = -.82352941176470584;
    bop2[9] = -.76470588235294112;
    bop2[13] = 1.1764705882352942;
    bop2[17] = -.29411764705882354;
    bop2[21] = 0.;
    bop2[25] = 0.;
    bop2[6] = 1.;
    bop2[10] = -2.;
    bop2[14] = 1.;
    bop2[18] = 0.;
    bop2[22] = 0.;
    bop2[26] = 0.;
    bop2[7] = -.093023255813953487;
    bop2[11] = 1.3720930232558139;
    bop2[15] = -2.558139534883721;
    bop2[19] = 1.3720930232558139;
    bop2[23] = -.093023255813953487;
    bop2[27] = 0.;
    bop2[8] = -.020408163265306121;
    bop2[12] = 0.;
    bop2[16] = 1.2040816326530612;
    bop2[20] = -2.4081632653061225;
    bop2[24] = 1.3061224489795917;
    bop2[28] = -.081632653061224483;
/* ** boundary derivative, 4th order */
    s[0] = -.25;
    s[1] = -.83333333333333337;
    s[2] = 1.5;
    s[3] = -.5;
    s[4] = .083333333333333329;
    return 0;
} /* wavepropbop_4__ */



/* ----------------------------------------------------------------------- */
/* Subroutine */ static int bopext4th_(double *bop, double *bope)
{
    static int i__, j;
    static double d4a, d4b;

    /* Parameter adjustments */
    bope -= 7;
    bop -= 5;

    /* Function Body */
    for (j = 1; j <= 8; ++j) {
	for (i__ = 1; i__ <= 6; ++i__) {
	    bope[i__ + j * 6] = 0.;
	}
    }
    for (j = 1; j <= 6; ++j) {
	for (i__ = 1; i__ <= 4; ++i__) {
	    bope[i__ + j * 6] = bop[i__ + (j << 2)];
	}
    }
    d4a = .66666666666666663;
    d4b = -.083333333333333329;
    bope[23] = -d4b;
    bope[29] = -d4a;
    bope[41] = d4a;
    bope[47] = d4b;
    bope[30] = -d4b;
    bope[36] = -d4a;
    bope[48] = d4a;
    bope[54] = d4b;
    return 0;
} /* bopext4th_ */

/* ----------------------------------------------------------------------- */
/* Subroutine */ static int varcoeffs4_(double *acof, double *ghcof)
{
/* ** acofs(i,j,k) is coefficient of a(k) in stencil coefficient (i,j) */
/* ** ghcof is coefficient of ghost point, a(1)*ghcof*u(0) in stencil at i=1. */
    /* Parameter adjustments */
    --ghcof;
    acof -= 55;

    /* Function Body */
    ghcof[1] = .70588235294117652;
    ghcof[2] = 0.;
    ghcof[3] = 0.;
    ghcof[4] = 0.;
    ghcof[5] = 0.;
    ghcof[6] = 0.;
    acof[55] = .35986159169550175;
    acof[103] = -1.0166864283333032;
    acof[151] = -.19053503754442955;
    acof[199] = -.0027010804321728693;
    acof[247] = 0.;
    acof[295] = 0.;
    acof[343] = 0.;
    acof[391] = 0.;
    acof[61] = -1.7854671280276817;
    acof[109] = .44711810852932143;
    acof[157] = .68697218969667206;
    acof[205] = 0.;
    acof[253] = 0.;
    acof[301] = 0.;
    acof[349] = 0.;
    acof[397] = 0.;
    acof[67] = 1.0795847750865053;
    acof[115] = .42052895029420795;
    acof[163] = -.56476516735285087;
    acof[211] = .053121248499399761;
    acof[259] = 0.;
    acof[307] = 0.;
    acof[355] = 0.;
    acof[403] = 0.;
    acof[73] = -.35986159169550175;
    acof[121] = .14903936950977381;
    acof[169] = .066048033440462367;
    acof[217] = 0.;
    acof[265] = 0.;
    acof[313] = 0.;
    acof[361] = 0.;
    acof[409] = 0.;
    acof[79] = 0.;
    acof[127] = 0.;
    acof[175] = .0022799817601459188;
    acof[223] = -.057623049219687875;
    acof[271] = 0.;
    acof[319] = 0.;
    acof[367] = 0.;
    acof[415] = 0.;
    acof[85] = 0.;
    acof[133] = 0.;
    acof[181] = 0.;
    acof[229] = .0072028811524609843;
    acof[277] = 0.;
    acof[325] = 0.;
    acof[373] = 0.;
    acof[421] = 0.;
    acof[91] = 0.;
    acof[139] = 0.;
    acof[187] = 0.;
    acof[235] = 0.;
    acof[283] = 0.;
    acof[331] = 0.;
    acof[379] = 0.;
    acof[427] = 0.;
    acof[97] = 0.;
    acof[145] = 0.;
    acof[193] = 0.;
    acof[241] = 0.;
    acof[289] = 0.;
    acof[337] = 0.;
    acof[385] = 0.;
    acof[433] = 0.;
    acof[56] = .70588235294117652;
    acof[104] = .12883064144065193;
    acof[152] = .19794113940412586;
    acof[200] = 0.;
    acof[248] = 0.;
    acof[296] = 0.;
    acof[344] = 0.;
    acof[392] = 0.;
    acof[62] = -.86764705882352944;
    acof[110] = -.38649192432195584;
    acof[158] = -.84819410618011493;
    acof[206] = -.033933163332341212;
    acof[254] = 0.;
    acof[302] = 0.;
    acof[350] = 0.;
    acof[398] = 0.;
    acof[68] = .11764705882352941;
    acof[116] = .38649192432195584;
    acof[164] = .60693548211558968;
    acof[212] = .10744920751114793;
    acof[260] = 0.;
    acof[308] = 0.;
    acof[356] = 0.;
    acof[404] = 0.;
    acof[74] = .044117647058823532;
    acof[122] = -.12883064144065193;
    acof[170] = .038946796692662086;
    acof[218] = -.11874864253939653;
    acof[266] = 0.;
    acof[314] = 0.;
    acof[362] = 0.;
    acof[410] = 0.;
    acof[80] = 0.;
    acof[128] = 0.;
    acof[176] = .004370687967737351;
    acof[224] = .050882315874714097;
    acof[272] = 0.;
    acof[320] = 0.;
    acof[368] = 0.;
    acof[416] = 0.;
    acof[86] = 0.;
    acof[134] = 0.;
    acof[182] = 0.;
    acof[230] = -.0056497175141242938;
    acof[278] = 0.;
    acof[326] = 0.;
    acof[374] = 0.;
    acof[422] = 0.;
    acof[92] = 0.;
    acof[140] = 0.;
    acof[188] = 0.;
    acof[236] = 0.;
    acof[284] = 0.;
    acof[332] = 0.;
    acof[380] = 0.;
    acof[428] = 0.;
    acof[98] = 0.;
    acof[146] = 0.;
    acof[194] = 0.;
    acof[242] = 0.;
    acof[290] = 0.;
    acof[338] = 0.;
    acof[386] = 0.;
    acof[434] = 0.;
    acof[57] = -.13132694938440492;
    acof[105] = .16625563151166362;
    acof[153] = -.22327925220926662;
    acof[201] = .02100142382534409;
    acof[249] = 0.;
    acof[297] = 0.;
    acof[345] = 0.;
    acof[393] = 0.;
    acof[63] = .16142270861833105;
    acof[111] = .53030287290686962;
    acof[159] = .83277194057720438;
    acof[207] = .14743030798041229;
    acof[255] = 0.;
    acof[303] = 0.;
    acof[351] = 0.;
    acof[399] = 0.;
    acof[69] = -.02188782489740082;
    acof[117] = -.87332612872082316;
    acof[165] = -1.1586403084760135;
    acof[213] = -.9155941987395283;
    acof[261] = -.044771396930865369;
    acof[309] = 0.;
    acof[357] = 0.;
    acof[405] = 0.;
    acof[75] = -.0082079343365253077;
    acof[123] = .17676762430228987;
    acof[171] = .71208180405748023;
    acof[219] = .62311674264080974;
    acof[267] = .18082581869957284;
    acof[315] = 0.;
    acof[363] = 0.;
    acof[411] = 0.;
    acof[81] = 0.;
    acof[129] = 0.;
    acof[177] = -.16293418394940454;
    acof[225] = .1197742482606888;
    acof[273] = -.1343141907925961;
    acof[321] = 0.;
    acof[369] = 0.;
    acof[417] = 0.;
    acof[87] = 0.;
    acof[135] = 0.;
    acof[183] = 0.;
    acof[231] = .0042714760322733747;
    acof[279] = -.0017402309761113748;
    acof[327] = 0.;
    acof[375] = 0.;
    acof[423] = 0.;
    acof[93] = 0.;
    acof[141] = 0.;
    acof[189] = 0.;
    acof[237] = 0.;
    acof[285] = 0.;
    acof[333] = 0.;
    acof[381] = 0.;
    acof[429] = 0.;
    acof[99] = 0.;
    acof[147] = 0.;
    acof[195] = 0.;
    acof[243] = 0.;
    acof[291] = 0.;
    acof[339] = 0.;
    acof[387] = 0.;
    acof[435] = 0.;
    acof[58] = -.043217286914765909;
    acof[106] = .051707536360533775;
    acof[154] = .022914623846691025;
    acof[202] = 0.;
    acof[250] = 0.;
    acof[298] = 0.;
    acof[346] = 0.;
    acof[394] = 0.;
    acof[64] = .053121248499399761;
    acof[112] = -.1551226090816013;
    acof[160] = .046895122548307411;
    acof[208] = -.14298305938417133;
    acof[256] = 0.;
    acof[304] = 0.;
    acof[352] = 0.;
    acof[400] = 0.;
    acof[70] = -.0072028811524609843;
    acof[118] = .1551226090816013;
    acof[166] = .62488811376472753;
    acof[214] = .54681673333785341;
    acof[262] = .15868388171595169;
    acof[310] = 0.;
    acof[358] = 0.;
    acof[406] = 0.;
    acof[76] = -.0027010804321728693;
    acof[124] = -.051707536360533775;
    acof[172] = -.88007927423461074;
    acof[220] = -.78255184370853226;
    acof[268] = -.80258225739275302;
    acof[316] = -.040816326530612242;
    acof[364] = 0.;
    acof[412] = 0.;
    acof[82] = 0.;
    acof[130] = 0.;
    acof[178] = .18538141407488481;
    acof[226] = .49658572494018965;
    acof[274] = .47605164514785508;
    acof[322] = .16326530612244897;
    acof[370] = 0.;
    acof[418] = 0.;
    acof[88] = 0.;
    acof[136] = 0.;
    acof[184] = 0.;
    acof[232] = -.11786755518533944;
    acof[280] = .16784673052894628;
    acof[328] = -.12244897959183673;
    acof[376] = 0.;
    acof[424] = 0.;
    acof[94] = 0.;
    acof[142] = 0.;
    acof[190] = 0.;
    acof[238] = 0.;
    acof[286] = 0.;
    acof[334] = 0.;
    acof[382] = 0.;
    acof[430] = 0.;
    acof[100] = 0.;
    acof[148] = 0.;
    acof[196] = 0.;
    acof[244] = 0.;
    acof[292] = 0.;
    acof[340] = 0.;
    acof[388] = 0.;
    acof[436] = 0.;
    acof[59] = 0.;
    acof[107] = 0.;
    acof[155] = 8.0749354005167954e-4;
    acof[203] = -.020408163265306121;
    acof[251] = 0.;
    acof[299] = 0.;
    acof[347] = 0.;
    acof[395] = 0.;
    acof[65] = 0.;
    acof[113] = 0.;
    acof[161] = .0053723039603438272;
    acof[209] = .062542846596002746;
    acof[257] = 0.;
    acof[305] = 0.;
    acof[353] = 0.;
    acof[401] = 0.;
    acof[71] = 0.;
    acof[119] = 0.;
    acof[167] = -.14596187312134157;
    acof[215] = .10729776406686706;
    acof[263] = -.12032312925170068;
    acof[311] = 0.;
    acof[359] = 0.;
    acof[407] = 0.;
    acof[77] = 0.;
    acof[125] = 0.;
    acof[173] = .18924352686811158;
    acof[221] = .50693126087644358;
    acof[269] = .48596938775510207;
    acof[317] = .16666666666666666;
    acof[365] = 0.;
    acof[413] = 0.;
    acof[83] = 0.;
    acof[131] = 0.;
    acof[179] = -.049461451247165535;
    acof[227] = -.8172197173443021;
    acof[275] = -.73596938775510201;
    acof[323] = -.83333333333333337;
    acof[371] = -.041666666666666664;
    acof[419] = 0.;
    acof[89] = 0.;
    acof[137] = 0.;
    acof[185] = 0.;
    acof[233] = .16085600907029479;
    acof[281] = .49532312925170069;
    acof[329] = .5;
    acof[377] = .16666666666666666;
    acof[425] = 0.;
    acof[95] = 0.;
    acof[143] = 0.;
    acof[191] = 0.;
    acof[239] = 0.;
    acof[287] = -.125;
    acof[335] = .16666666666666666;
    acof[383] = -.125;
    acof[431] = 0.;
    acof[101] = 0.;
    acof[149] = 0.;
    acof[197] = 0.;
    acof[245] = 0.;
    acof[293] = 0.;
    acof[341] = 0.;
    acof[389] = 0.;
    acof[437] = 0.;
    acof[60] = 0.;
    acof[108] = 0.;
    acof[156] = 0.;
    acof[204] = .0025510204081632651;
    acof[252] = 0.;
    acof[300] = 0.;
    acof[348] = 0.;
    acof[396] = 0.;
    acof[66] = 0.;
    acof[114] = 0.;
    acof[162] = 0.;
    acof[210] = -.0069444444444444441;
    acof[258] = 0.;
    acof[306] = 0.;
    acof[354] = 0.;
    acof[402] = 0.;
    acof[72] = 0.;
    acof[120] = 0.;
    acof[168] = 0.;
    acof[216] = .0038265306122448979;
    acof[264] = -.0015589569160997733;
    acof[312] = 0.;
    acof[360] = 0.;
    acof[408] = 0.;
    acof[78] = 0.;
    acof[126] = 0.;
    acof[174] = 0.;
    acof[222] = -.12032312925170068;
    acof[270] = .171343537414966;
    acof[318] = -.125;
    acof[366] = 0.;
    acof[414] = 0.;
    acof[84] = 0.;
    acof[132] = 0.;
    acof[180] = 0.;
    acof[228] = .16085600907029479;
    acof[276] = .49532312925170069;
    acof[324] = .5;
    acof[372] = .16666666666666666;
    acof[420] = 0.;
    acof[90] = 0.;
    acof[138] = 0.;
    acof[186] = 0.;
    acof[234] = -.039965986394557826;
    acof[282] = -.83177437641723351;
    acof[330] = -.75;
    acof[378] = -.83333333333333337;
    acof[426] = -.041666666666666664;
    acof[96] = 0.;
    acof[144] = 0.;
    acof[192] = 0.;
    acof[240] = 0.;
    acof[288] = .16666666666666666;
    acof[336] = .5;
    acof[384] = .5;
    acof[432] = .16666666666666666;
    acof[102] = 0.;
    acof[150] = 0.;
    acof[198] = 0.;
    acof[246] = 0.;
    acof[294] = 0.;
    acof[342] = -.125;
    acof[390] = .16666666666666666;
    acof[438] = -.125;
/* ** 129 non-zero out of 384. */
    return 0;
} /* varcoeffs4_ */

// end of GPL portion

void initialize_once() {
    lu = malloc(NC * NK * NJ * NI * sizeof(float_sw4));
    u = malloc(NC * NK * NJ * NI * sizeof(float_sw4));
    eqs = malloc(NC * NK * NJ * NI * sizeof(float_sw4));
    mu = malloc(NK * NJ * NI * sizeof(float_sw4));
    rho = malloc(NK * NJ * NI * sizeof(float_sw4));
    lambda = malloc(NK * NJ * NI * sizeof(float_sw4));

#if OSU
    onesided[4] = 1;
#endif

#if OSL
    onesided[5] = 1;
#endif
}

void initialize() {
    int i, j, k, c;

    double m_iop[5];
    double m_iop2[5];
    double bop[24];
    double m_bop2[24];
    double gh2;
    double m_hnorm[4];
    double m_sbop[5];

    double h = 1.0 / (NI - 1);

    for(k = 0; k < NK; k++) {
        for(j = 0; j < NJ; j++) {
            for(i = 0; i < NI; i++) {
                get_data(i * h, j * h, k * h, &u[k][j][i][0],
                         &u[k][j][i][1], &u[k][j][i][2], &mu[k][j][i],
                         &lambda[k][j][i], &rho[k][j][i]);
                fg(i * h, j * h, k * h, &eqs[k][j][i][0]);

                for (c = 0; c < NC; c++) {
                    lu[k][j][i][c] = 0.0;
                }
            }
        }
    }
    for(i = 0; i < NI; i++) {
        strx[i] = 1;
    }
    for(j = 0; j < NJ; j++) {
        stry[j] = 1;
    }
    for(k = 0; k < NK; k++) {
        strz[k] = 1;
    }

    varcoeffs4_((double *) acof, (double *) ghcof);
    wavepropbop_4__(m_iop, m_iop2, bop, m_bop2, &gh2, m_hnorm, m_sbop);
    bopext4th_((double *) bop, (double *) bope);
}

/*
in the original code, the arrays are flat and the following macros are defined:
#define mu(i,j,k)     a_mu[base+i+ni*(j)+nij*(k)]
#define la(i,j,k) a_lambda[base+i+ni*(j)+nij*(k)]
#define u(c,i,j,k)   a_u[base3+c+3*(i)+nic*(j)+nijc*(k)]
#define lu(c,i,j,k) a_lu[base3+c+3*(i)+nic*(j)+nijc*(k)]
#define strx(i) a_strx[i-ifirst0]
#define stry(j) a_stry[j-jfirst0]
#define strz(k) a_strz[k-kfirst0]
#define acof(i,j,k) a_acof[(i-1)+6*(j-1)+48*(k-1)]
#define bope(i,j) a_bope[i-1+6*(j-1)]
#define ghcof(i) a_ghcof[i-1]
*/

#define mu(i,j,k)     a_mu[k][j][base+i]
#define la(i,j,k) a_lambda[k][j][base+i]
#define u(c,i,j,k)   a_u[k][j][i][base3+c]
#define lu(c,i,j,k) a_lu[k][j][i][base3+c]
#define strx(i) a_strx[i-ifirst]
#define stry(j) a_stry[j-jfirst]
#define strz(k) a_strz[k-kfirst]
#define acof(i,j,k) a_acof[k-1][j-1][i-1]
#define bope(i,j) a_bope[j-1][i-1]
#define ghcof(i) a_ghcof[i-1]

// core stencil function
#pragma rstream inline
void core_fn1(int ifirst, int ilast, int jfirst, int jlast, int kfirst,
    int klast, int nk, int* onesided, float_sw4 (*a_acof)[8][6],
    float_sw4 (*a_bope)[6], float_sw4 *a_ghcof, float_sw4 (*a_lu)[NJ][NI][NC],
    float_sw4 (*a_u)[NJ][NI][NC], float_sw4 (*a_mu)[NJ][NI],
    float_sw4 (*a_lambda)[NJ][NI], float_sw4 h, float_sw4* a_strx,
    float_sw4* a_stry, float_sw4* a_strz, int k1, int k2, int base, int base3)
{
   int i, j, k;
   float_sw4 mux1, mux2, mux3, mux4, muy1, muy2, muy3, muy4, muz1, muz2, muz3, muz4;
   float_sw4 r1, r2, r3, cof;

   const float_sw4 a1   = 0;
   const float_sw4 i6   = 1.0/6;
   const float_sw4 i144 = 1.0/144;
   const float_sw4 tf   = 0.75;

   cof = 1.0/(h*h);

   for( k= k1; k <= k2 ; k++ )
   {
      for( j=jfirst+2; j <= jlast-2 ; j++ )
      {
         for( i=ifirst+2; i <= ilast-2 ; i++ )
         {

            /* from inner_loop_4a, 28x3 = 84 ops */
            mux1 = mu(i-1,j,k)*strx(i-1)-
                tf*(mu(i,j,k)*strx(i)+mu(i-2,j,k)*strx(i-2));
            mux2 = mu(i-2,j,k)*strx(i-2)+mu(i+1,j,k)*strx(i+1)+
                3*(mu(i,j,k)*strx(i)+mu(i-1,j,k)*strx(i-1));
            mux3 = mu(i-1,j,k)*strx(i-1)+mu(i+2,j,k)*strx(i+2)+
                3*(mu(i+1,j,k)*strx(i+1)+mu(i,j,k)*strx(i));
            mux4 = mu(i+1,j,k)*strx(i+1)-
                tf*(mu(i,j,k)*strx(i)+mu(i+2,j,k)*strx(i+2));

            muy1 = mu(i,j-1,k)*stry(j-1)-
                tf*(mu(i,j,k)*stry(j)+mu(i,j-2,k)*stry(j-2));
            muy2 = mu(i,j-2,k)*stry(j-2)+mu(i,j+1,k)*stry(j+1)+
                3*(mu(i,j,k)*stry(j)+mu(i,j-1,k)*stry(j-1));
            muy3 = mu(i,j-1,k)*stry(j-1)+mu(i,j+2,k)*stry(j+2)+
                3*(mu(i,j+1,k)*stry(j+1)+mu(i,j,k)*stry(j));
            muy4 = mu(i,j+1,k)*stry(j+1)-
                tf*(mu(i,j,k)*stry(j)+mu(i,j+2,k)*stry(j+2));

            muz1 = mu(i,j,k-1)*strz(k-1)-
                tf*(mu(i,j,k)*strz(k)+mu(i,j,k-2)*strz(k-2));
            muz2 = mu(i,j,k-2)*strz(k-2)+mu(i,j,k+1)*strz(k+1)+
                3*(mu(i,j,k)*strz(k)+mu(i,j,k-1)*strz(k-1));
            muz3 = mu(i,j,k-1)*strz(k-1)+mu(i,j,k+2)*strz(k+2)+
                3*(mu(i,j,k+1)*strz(k+1)+mu(i,j,k)*strz(k));
            muz4 = mu(i,j,k+1)*strz(k+1)-
                tf*(mu(i,j,k)*strz(k)+mu(i,j,k+2)*strz(k+2));
            /* xx, yy, and zz derivatives:*/
            /* 75 ops */
            r1 = i6*( strx(i)*( (2*mux1+la(i-1,j,k)*strx(i-1)-
                                 tf*(la(i,j,k)*strx(i)+la(i-2,j,k)*strx(i-2)))*
                                (u(1,i-2,j,k)-u(1,i,j,k))+
                                (2*mux2+la(i-2,j,k)*strx(i-2)+la(i+1,j,k)*strx(i+1)+
                                 3*(la(i,j,k)*strx(i)+la(i-1,j,k)*strx(i-1)))*
                                (u(1,i-1,j,k)-u(1,i,j,k))+
                                (2*mux3+la(i-1,j,k)*strx(i-1)+la(i+2,j,k)*strx(i+2)+
                                 3*(la(i+1,j,k)*strx(i+1)+la(i,j,k)*strx(i)))*
                                (u(1,i+1,j,k)-u(1,i,j,k))+
                                (2*mux4+ la(i+1,j,k)*strx(i+1)-
                                 tf*(la(i,j,k)*strx(i)+la(i+2,j,k)*strx(i+2)))*
                                (u(1,i+2,j,k)-u(1,i,j,k)) ) + stry(j)*(
                                    muy1*(u(1,i,j-2,k)-u(1,i,j,k)) +
                                    muy2*(u(1,i,j-1,k)-u(1,i,j,k)) +
                                    muy3*(u(1,i,j+1,k)-u(1,i,j,k)) +
                                    muy4*(u(1,i,j+2,k)-u(1,i,j,k)) ) + strz(k)*(
                                        muz1*(u(1,i,j,k-2)-u(1,i,j,k)) +
                                        muz2*(u(1,i,j,k-1)-u(1,i,j,k)) +
                                        muz3*(u(1,i,j,k+1)-u(1,i,j,k)) +
                                        muz4*(u(1,i,j,k+2)-u(1,i,j,k)) ) );

            /* 75 ops */
            r2 = i6*( strx(i)*(mux1*(u(2,i-2,j,k)-u(2,i,j,k)) +
                               mux2*(u(2,i-1,j,k)-u(2,i,j,k)) +
                               mux3*(u(2,i+1,j,k)-u(2,i,j,k)) +
                               mux4*(u(2,i+2,j,k)-u(2,i,j,k)) ) + stry(j)*(
                                   (2*muy1+la(i,j-1,k)*stry(j-1)-
                                    tf*(la(i,j,k)*stry(j)+la(i,j-2,k)*stry(j-2)))*
                                   (u(2,i,j-2,k)-u(2,i,j,k))+
                                   (2*muy2+la(i,j-2,k)*stry(j-2)+la(i,j+1,k)*stry(j+1)+
                                    3*(la(i,j,k)*stry(j)+la(i,j-1,k)*stry(j-1)))*
                                   (u(2,i,j-1,k)-u(2,i,j,k))+
                                   (2*muy3+la(i,j-1,k)*stry(j-1)+la(i,j+2,k)*stry(j+2)+
                                    3*(la(i,j+1,k)*stry(j+1)+la(i,j,k)*stry(j)))*
                                   (u(2,i,j+1,k)-u(2,i,j,k))+
                                   (2*muy4+la(i,j+1,k)*stry(j+1)-
                                    tf*(la(i,j,k)*stry(j)+la(i,j+2,k)*stry(j+2)))*
                                   (u(2,i,j+2,k)-u(2,i,j,k)) ) + strz(k)*(
                                       muz1*(u(2,i,j,k-2)-u(2,i,j,k)) +
                                       muz2*(u(2,i,j,k-1)-u(2,i,j,k)) +
                                       muz3*(u(2,i,j,k+1)-u(2,i,j,k)) +
                                       muz4*(u(2,i,j,k+2)-u(2,i,j,k)) ) );

            /* 75 ops */
            r3 = i6*( strx(i)*(mux1*(u(3,i-2,j,k)-u(3,i,j,k)) +
                               mux2*(u(3,i-1,j,k)-u(3,i,j,k)) +
                               mux3*(u(3,i+1,j,k)-u(3,i,j,k)) +
                               mux4*(u(3,i+2,j,k)-u(3,i,j,k))  ) + stry(j)*(
                                   muy1*(u(3,i,j-2,k)-u(3,i,j,k)) +
                                   muy2*(u(3,i,j-1,k)-u(3,i,j,k)) +
                                   muy3*(u(3,i,j+1,k)-u(3,i,j,k)) +
                                   muy4*(u(3,i,j+2,k)-u(3,i,j,k)) ) + strz(k)*(
                                       (2*muz1+la(i,j,k-1)*strz(k-1)-
                                        tf*(la(i,j,k)*strz(k)+la(i,j,k-2)*strz(k-2)))*
                                       (u(3,i,j,k-2)-u(3,i,j,k))+
                                       (2*muz2+la(i,j,k-2)*strz(k-2)+la(i,j,k+1)*strz(k+1)+
                                        3*(la(i,j,k)*strz(k)+la(i,j,k-1)*strz(k-1)))*
                                       (u(3,i,j,k-1)-u(3,i,j,k))+
                                       (2*muz3+la(i,j,k-1)*strz(k-1)+la(i,j,k+2)*strz(k+2)+
                                        3*(la(i,j,k+1)*strz(k+1)+la(i,j,k)*strz(k)))*
                                       (u(3,i,j,k+1)-u(3,i,j,k))+
                                       (2*muz4+la(i,j,k+1)*strz(k+1)-
                                        tf*(la(i,j,k)*strz(k)+la(i,j,k+2)*strz(k+2)))*
                                       (u(3,i,j,k+2)-u(3,i,j,k)) ) );


            /* Mixed derivatives: */
            /* 29ops /mixed derivative */
            /* 116 ops for r1 */
            /*   (la*v_y)_x */
            r1 = r1 + strx(i)*stry(j)*
                i144*( la(i-2,j,k)*(u(2,i-2,j-2,k)-u(2,i-2,j+2,k)+
                                    8*(-u(2,i-2,j-1,k)+u(2,i-2,j+1,k)))
                       - 8*(la(i-1,j,k)*(u(2,i-1,j-2,k)-u(2,i-1,j+2,k)+
                                         8*(-u(2,i-1,j-1,k)+u(2,i-1,j+1,k))) )+8*(
                                             la(i+1,j,k)*(u(2,i+1,j-2,k)-u(2,i+1,j+2,k)+
                                                          8*(-u(2,i+1,j-1,k)+u(2,i+1,j+1,k))) ) - (
                                                              la(i+2,j,k)*(u(2,i+2,j-2,k)-u(2,i+2,j+2,k)+
                                                                           8*(-u(2,i+2,j-1,k)+u(2,i+2,j+1,k))) ))
                /*   (la*w_z)_x */
                + strx(i)*strz(k)*
                i144*( la(i-2,j,k)*(u(3,i-2,j,k-2)-u(3,i-2,j,k+2)+
                                    8*(-u(3,i-2,j,k-1)+u(3,i-2,j,k+1)))
                       - 8*(la(i-1,j,k)*(u(3,i-1,j,k-2)-u(3,i-1,j,k+2)+
                                         8*(-u(3,i-1,j,k-1)+u(3,i-1,j,k+1))) )+8*(
                                             la(i+1,j,k)*(u(3,i+1,j,k-2)-u(3,i+1,j,k+2)+
                                                          8*(-u(3,i+1,j,k-1)+u(3,i+1,j,k+1))) ) - (
                                                              la(i+2,j,k)*(u(3,i+2,j,k-2)-u(3,i+2,j,k+2)+
                                                                           8*(-u(3,i+2,j,k-1)+u(3,i+2,j,k+1))) ))
                /*   (mu*v_x)_y */
                + strx(i)*stry(j)*
                i144*( mu(i,j-2,k)*(u(2,i-2,j-2,k)-u(2,i+2,j-2,k)+
                                    8*(-u(2,i-1,j-2,k)+u(2,i+1,j-2,k)))
                       - 8*(mu(i,j-1,k)*(u(2,i-2,j-1,k)-u(2,i+2,j-1,k)+
                                         8*(-u(2,i-1,j-1,k)+u(2,i+1,j-1,k))) )+8*(
                                             mu(i,j+1,k)*(u(2,i-2,j+1,k)-u(2,i+2,j+1,k)+
                                                          8*(-u(2,i-1,j+1,k)+u(2,i+1,j+1,k))) ) - (
                                                              mu(i,j+2,k)*(u(2,i-2,j+2,k)-u(2,i+2,j+2,k)+
                                                                           8*(-u(2,i-1,j+2,k)+u(2,i+1,j+2,k))) ))
                /*   (mu*w_x)_z */
                + strx(i)*strz(k)*
                i144*( mu(i,j,k-2)*(u(3,i-2,j,k-2)-u(3,i+2,j,k-2)+
                                    8*(-u(3,i-1,j,k-2)+u(3,i+1,j,k-2)))
                       - 8*(mu(i,j,k-1)*(u(3,i-2,j,k-1)-u(3,i+2,j,k-1)+
                                         8*(-u(3,i-1,j,k-1)+u(3,i+1,j,k-1))) )+8*(
                                             mu(i,j,k+1)*(u(3,i-2,j,k+1)-u(3,i+2,j,k+1)+
                                                          8*(-u(3,i-1,j,k+1)+u(3,i+1,j,k+1))) ) - (
                                                              mu(i,j,k+2)*(u(3,i-2,j,k+2)-u(3,i+2,j,k+2)+
                                                                           8*(-u(3,i-1,j,k+2)+u(3,i+1,j,k+2))) )) ;

            /* 116 ops for r2 */
            /*   (mu*u_y)_x */
            r2 = r2 + strx(i)*stry(j)*
                i144*( mu(i-2,j,k)*(u(1,i-2,j-2,k)-u(1,i-2,j+2,k)+
                                    8*(-u(1,i-2,j-1,k)+u(1,i-2,j+1,k)))
                       - 8*(mu(i-1,j,k)*(u(1,i-1,j-2,k)-u(1,i-1,j+2,k)+
                                         8*(-u(1,i-1,j-1,k)+u(1,i-1,j+1,k))) )+8*(
                                             mu(i+1,j,k)*(u(1,i+1,j-2,k)-u(1,i+1,j+2,k)+
                                                          8*(-u(1,i+1,j-1,k)+u(1,i+1,j+1,k))) ) - (
                                                              mu(i+2,j,k)*(u(1,i+2,j-2,k)-u(1,i+2,j+2,k)+
                                                                           8*(-u(1,i+2,j-1,k)+u(1,i+2,j+1,k))) ))
                /* (la*u_x)_y */
                + strx(i)*stry(j)*
                i144*( la(i,j-2,k)*(u(1,i-2,j-2,k)-u(1,i+2,j-2,k)+
                                    8*(-u(1,i-1,j-2,k)+u(1,i+1,j-2,k)))
                       - 8*(la(i,j-1,k)*(u(1,i-2,j-1,k)-u(1,i+2,j-1,k)+
                                         8*(-u(1,i-1,j-1,k)+u(1,i+1,j-1,k))) )+8*(
                                             la(i,j+1,k)*(u(1,i-2,j+1,k)-u(1,i+2,j+1,k)+
                                                          8*(-u(1,i-1,j+1,k)+u(1,i+1,j+1,k))) ) - (
                                                              la(i,j+2,k)*(u(1,i-2,j+2,k)-u(1,i+2,j+2,k)+
                                                                           8*(-u(1,i-1,j+2,k)+u(1,i+1,j+2,k))) ))
                /* (la*w_z)_y */
                + stry(j)*strz(k)*
                i144*( la(i,j-2,k)*(u(3,i,j-2,k-2)-u(3,i,j-2,k+2)+
                                    8*(-u(3,i,j-2,k-1)+u(3,i,j-2,k+1)))
                       - 8*(la(i,j-1,k)*(u(3,i,j-1,k-2)-u(3,i,j-1,k+2)+
                                         8*(-u(3,i,j-1,k-1)+u(3,i,j-1,k+1))) )+8*(
                                             la(i,j+1,k)*(u(3,i,j+1,k-2)-u(3,i,j+1,k+2)+
                                                          8*(-u(3,i,j+1,k-1)+u(3,i,j+1,k+1))) ) - (
                                                              la(i,j+2,k)*(u(3,i,j+2,k-2)-u(3,i,j+2,k+2)+
                                                                           8*(-u(3,i,j+2,k-1)+u(3,i,j+2,k+1))) ))
                /* (mu*w_y)_z */
                + stry(j)*strz(k)*
                i144*( mu(i,j,k-2)*(u(3,i,j-2,k-2)-u(3,i,j+2,k-2)+
                                    8*(-u(3,i,j-1,k-2)+u(3,i,j+1,k-2)))
                       - 8*(mu(i,j,k-1)*(u(3,i,j-2,k-1)-u(3,i,j+2,k-1)+
                                         8*(-u(3,i,j-1,k-1)+u(3,i,j+1,k-1))) )+8*(
                                             mu(i,j,k+1)*(u(3,i,j-2,k+1)-u(3,i,j+2,k+1)+
                                                          8*(-u(3,i,j-1,k+1)+u(3,i,j+1,k+1))) ) - (
                                                              mu(i,j,k+2)*(u(3,i,j-2,k+2)-u(3,i,j+2,k+2)+
                                                                           8*(-u(3,i,j-1,k+2)+u(3,i,j+1,k+2))) )) ;
            /* 116 ops for r3 */
            /*  (mu*u_z)_x */
            r3 = r3 + strx(i)*strz(k)*
                i144*( mu(i-2,j,k)*(u(1,i-2,j,k-2)-u(1,i-2,j,k+2)+
                                    8*(-u(1,i-2,j,k-1)+u(1,i-2,j,k+1)))
                       - 8*(mu(i-1,j,k)*(u(1,i-1,j,k-2)-u(1,i-1,j,k+2)+
                                         8*(-u(1,i-1,j,k-1)+u(1,i-1,j,k+1))) )+8*(
                                             mu(i+1,j,k)*(u(1,i+1,j,k-2)-u(1,i+1,j,k+2)+
                                                          8*(-u(1,i+1,j,k-1)+u(1,i+1,j,k+1))) ) - (
                                                              mu(i+2,j,k)*(u(1,i+2,j,k-2)-u(1,i+2,j,k+2)+
                                                                           8*(-u(1,i+2,j,k-1)+u(1,i+2,j,k+1))) ))
                /* (mu*v_z)_y */
                + stry(j)*strz(k)*
                i144*( mu(i,j-2,k)*(u(2,i,j-2,k-2)-u(2,i,j-2,k+2)+
                                    8*(-u(2,i,j-2,k-1)+u(2,i,j-2,k+1)))
                       - 8*(mu(i,j-1,k)*(u(2,i,j-1,k-2)-u(2,i,j-1,k+2)+
                                         8*(-u(2,i,j-1,k-1)+u(2,i,j-1,k+1))) )+8*(
                                             mu(i,j+1,k)*(u(2,i,j+1,k-2)-u(2,i,j+1,k+2)+
                                                          8*(-u(2,i,j+1,k-1)+u(2,i,j+1,k+1))) ) - (
                                                              mu(i,j+2,k)*(u(2,i,j+2,k-2)-u(2,i,j+2,k+2)+
                                                                           8*(-u(2,i,j+2,k-1)+u(2,i,j+2,k+1))) ))
                /*   (la*u_x)_z */
                + strx(i)*strz(k)*
                i144*( la(i,j,k-2)*(u(1,i-2,j,k-2)-u(1,i+2,j,k-2)+
                                    8*(-u(1,i-1,j,k-2)+u(1,i+1,j,k-2)))
                       - 8*(la(i,j,k-1)*(u(1,i-2,j,k-1)-u(1,i+2,j,k-1)+
                                         8*(-u(1,i-1,j,k-1)+u(1,i+1,j,k-1))) )+8*(
                                             la(i,j,k+1)*(u(1,i-2,j,k+1)-u(1,i+2,j,k+1)+
                                                          8*(-u(1,i-1,j,k+1)+u(1,i+1,j,k+1))) ) - (
                                                              la(i,j,k+2)*(u(1,i-2,j,k+2)-u(1,i+2,j,k+2)+
                                                                           8*(-u(1,i-1,j,k+2)+u(1,i+1,j,k+2))) ))
                /* (la*v_y)_z */
                + stry(j)*strz(k)*
                i144*( la(i,j,k-2)*(u(2,i,j-2,k-2)-u(2,i,j+2,k-2)+
                                    8*(-u(2,i,j-1,k-2)+u(2,i,j+1,k-2)))
                       - 8*(la(i,j,k-1)*(u(2,i,j-2,k-1)-u(2,i,j+2,k-1)+
                                         8*(-u(2,i,j-1,k-1)+u(2,i,j+1,k-1))) )+8*(
                                             la(i,j,k+1)*(u(2,i,j-2,k+1)-u(2,i,j+2,k+1)+
                                                          8*(-u(2,i,j-1,k+1)+u(2,i,j+1,k+1))) ) - (
                                                              la(i,j,k+2)*(u(2,i,j-2,k+2)-u(2,i,j+2,k+2)+
                                                                           8*(-u(2,i,j-1,k+2)+u(2,i,j+1,k+2))) )) ;

            /* 9 ops */
            lu(1,i,j,k) = a1*lu(1,i,j,k) + cof*r1;
            lu(2,i,j,k) = a1*lu(2,i,j,k) + cof*r2;
            lu(3,i,j,k) = a1*lu(3,i,j,k) + cof*r3;
         }
      }
   }
}

#pragma rstream inline
void core_fn2(int ifirst, int ilast, int jfirst, int jlast, int kfirst,
    int klast, int nk, int* onesided, float_sw4 (*a_acof)[8][6],
    float_sw4 (*a_bope)[6], float_sw4 *a_ghcof, float_sw4 (*a_lu)[NJ][NI][NC],
    float_sw4 (*a_u)[NJ][NI][NC], float_sw4 (*a_mu)[NJ][NI],
    float_sw4 (*a_lambda)[NJ][NI], float_sw4 h, float_sw4* a_strx,
    float_sw4* a_stry, float_sw4* a_strz, int k1, int k2, int base, int base3)
{
   int i, j, k, q, m;
   float_sw4 mux1, mux2, mux3, mux4, muy1, muy2, muy3, muy4;
   float_sw4 r1, r2, r3, cof, mucof, mu1zz, mu2zz, mu3zz;
   float_sw4 lap2mu, u3zip2, u3zip1, u3zim1, u3zim2, lau3zx, mu3xz, u3zjp2, u3zjp1, u3zjm1, u3zjm2;
   float_sw4 lau3zy, mu3yz, mu1zx, mu2zy, u1zip2, u1zip1, u1zim1, u1zim2;
   float_sw4 u2zjp2, u2zjp1, u2zjm1, u2zjm2, lau1xz, lau2yz;

   const float_sw4 a1   = 0;
   const float_sw4 i6   = 1.0/6;
   const float_sw4 i12  = 1.0/12;
   const float_sw4 i144 = 1.0/144;
   const float_sw4 tf   = 0.75;

      for( k=1 ; k<= 6 ; k++ )
         /* the centered stencil can be used in the x- and y-directions */
         for( j=jfirst+2; j<=jlast-2; j++ )
            for( i=ifirst+2; i<=ilast-2; i++ )
            {
               /* from inner_loop_4a */
               mux1 = mu(i-1,j,k)*strx(i-1)-
                   tf*(mu(i,j,k)*strx(i)+mu(i-2,j,k)*strx(i-2));
               mux2 = mu(i-2,j,k)*strx(i-2)+mu(i+1,j,k)*strx(i+1)+
                   3*(mu(i,j,k)*strx(i)+mu(i-1,j,k)*strx(i-1));
               mux3 = mu(i-1,j,k)*strx(i-1)+mu(i+2,j,k)*strx(i+2)+
                   3*(mu(i+1,j,k)*strx(i+1)+mu(i,j,k)*strx(i));
               mux4 = mu(i+1,j,k)*strx(i+1)-
                   tf*(mu(i,j,k)*strx(i)+mu(i+2,j,k)*strx(i+2));

               muy1 = mu(i,j-1,k)*stry(j-1)-
                   tf*(mu(i,j,k)*stry(j)+mu(i,j-2,k)*stry(j-2));
               muy2 = mu(i,j-2,k)*stry(j-2)+mu(i,j+1,k)*stry(j+1)+
                   3*(mu(i,j,k)*stry(j)+mu(i,j-1,k)*stry(j-1));
               muy3 = mu(i,j-1,k)*stry(j-1)+mu(i,j+2,k)*stry(j+2)+
                   3*(mu(i,j+1,k)*stry(j+1)+mu(i,j,k)*stry(j));
               muy4 = mu(i,j+1,k)*stry(j+1)-
                   tf*(mu(i,j,k)*stry(j)+mu(i,j+2,k)*stry(j+2));

               r1 = i6*(strx(i)*((2*mux1+la(i-1,j,k)*strx(i-1)-
                                  tf*(la(i,j,k)*strx(i)+la(i-2,j,k)*strx(i-2)))*
                                 (u(1,i-2,j,k)-u(1,i,j,k))+
                                 (2*mux2+la(i-2,j,k)*strx(i-2)+la(i+1,j,k)*strx(i+1)+
                                  3*(la(i,j,k)*strx(i)+la(i-1,j,k)*strx(i-1)))*
                                 (u(1,i-1,j,k)-u(1,i,j,k))+
                                 (2*mux3+la(i-1,j,k)*strx(i-1)+la(i+2,j,k)*strx(i+2)+
                                  3*(la(i+1,j,k)*strx(i+1)+la(i,j,k)*strx(i)))*
                                 (u(1,i+1,j,k)-u(1,i,j,k))+
                                 (2*mux4+ la(i+1,j,k)*strx(i+1)-
                                  tf*(la(i,j,k)*strx(i)+la(i+2,j,k)*strx(i+2)))*
                                 (u(1,i+2,j,k)-u(1,i,j,k)) ) + stry(j)*(
                                     + muy1*(u(1,i,j-2,k)-u(1,i,j,k)) +
                                     muy2*(u(1,i,j-1,k)-u(1,i,j,k)) +
                                     muy3*(u(1,i,j+1,k)-u(1,i,j,k)) +
                                     muy4*(u(1,i,j+2,k)-u(1,i,j,k)) ) );

               /* (mu*uz)_z can not be centered */
               /* second derivative (mu*u_z)_z at grid point z_k */
               /* averaging the coefficient, */
               /* leave out the z-supergrid stretching strz, since it will */
               /* never be used together with the sbp-boundary operator */
               mu1zz = 0;
               mu2zz = 0;
               mu3zz = 0;
               for( q=1; q <= 8; q ++ )
               {
                  lap2mu= 0;
                  mucof = 0;
                  for( m=1 ; m<=8; m++ )
                  {
                     mucof  += acof(k,q,m)*mu(i,j,m);
                     lap2mu += acof(k,q,m)*(la(i,j,m)+2*mu(i,j,m));
                  }
                  mu1zz += mucof*u(1,i,j,q);
                  mu2zz += mucof*u(2,i,j,q);
                  mu3zz += lap2mu*u(3,i,j,q);
               }


               /* ghost point only influences the first point (k=1) because ghcof(k)=0 for k>=2*/
               r1 = r1 + (mu1zz + ghcof(k)*mu(i,j,1)*u(1,i,j,0));

               r2 = i6*(strx(i)*(mux1*(u(2,i-2,j,k)-u(2,i,j,k)) +
                                 mux2*(u(2,i-1,j,k)-u(2,i,j,k)) +
                                 mux3*(u(2,i+1,j,k)-u(2,i,j,k)) +
                                 mux4*(u(2,i+2,j,k)-u(2,i,j,k)) )+ stry(j)*(
                                     (2*muy1+la(i,j-1,k)*stry(j-1)-
                                      tf*(la(i,j,k)*stry(j)+la(i,j-2,k)*stry(j-2)))*
                                     (u(2,i,j-2,k)-u(2,i,j,k))+
                                     (2*muy2+la(i,j-2,k)*stry(j-2)+la(i,j+1,k)*stry(j+1)+
                                      3*(la(i,j,k)*stry(j)+la(i,j-1,k)*stry(j-1)))*
                                     (u(2,i,j-1,k)-u(2,i,j,k))+
                                     (2*muy3+la(i,j-1,k)*stry(j-1)+la(i,j+2,k)*stry(j+2)+
                                      3*(la(i,j+1,k)*stry(j+1)+la(i,j,k)*stry(j)))*
                                     (u(2,i,j+1,k)-u(2,i,j,k))+
                                     (2*muy4+la(i,j+1,k)*stry(j+1)-
                                      tf*(la(i,j,k)*stry(j)+la(i,j+2,k)*stry(j+2)))*
                                     (u(2,i,j+2,k)-u(2,i,j,k)) ) );

               /* ghost point only influences the first point (k=1) because ghcof(k)=0 for k>=2 */
               r2 = r2 + (mu2zz + ghcof(k)*mu(i,j,1)*u(2,i,j,0));

               r3 = i6*(strx(i)*(mux1*(u(3,i-2,j,k)-u(3,i,j,k)) +
                                 mux2*(u(3,i-1,j,k)-u(3,i,j,k)) +
                                 mux3*(u(3,i+1,j,k)-u(3,i,j,k)) +
                                 mux4*(u(3,i+2,j,k)-u(3,i,j,k))  ) + stry(j)*(
                                     muy1*(u(3,i,j-2,k)-u(3,i,j,k)) +
                                     muy2*(u(3,i,j-1,k)-u(3,i,j,k)) +
                                     muy3*(u(3,i,j+1,k)-u(3,i,j,k)) +
                                     muy4*(u(3,i,j+2,k)-u(3,i,j,k)) ) );
               /* ghost point only influences the first point (k=1) because ghcof(k)=0 for k>=2 */
               r3 = r3 + (mu3zz + ghcof(k)*(la(i,j,1)+2*mu(i,j,1))*
                          u(3,i,j,0));

               /* cross-terms in first component of rhs */
               /*   (la*v_y)_x */
               r1 = r1 + strx(i)*stry(j)*(
                   i144*( la(i-2,j,k)*(u(2,i-2,j-2,k)-u(2,i-2,j+2,k)+
                                       8*(-u(2,i-2,j-1,k)+u(2,i-2,j+1,k)))
                          - 8*(la(i-1,j,k)*(u(2,i-1,j-2,k)-u(2,i-1,j+2,k)+
                                            8*(-u(2,i-1,j-1,k)+u(2,i-1,j+1,k))) )+8*(
                                                la(i+1,j,k)*(u(2,i+1,j-2,k)-u(2,i+1,j+2,k)+
                                                             8*(-u(2,i+1,j-1,k)+u(2,i+1,j+1,k))) ) - (
                                                                 la(i+2,j,k)*(u(2,i+2,j-2,k)-u(2,i+2,j+2,k)+
                                                                              8*(-u(2,i+2,j-1,k)+u(2,i+2,j+1,k))) ))
                   /*   (mu*v_x)_y */
                   + i144*( mu(i,j-2,k)*(u(2,i-2,j-2,k)-u(2,i+2,j-2,k)+
                                         8*(-u(2,i-1,j-2,k)+u(2,i+1,j-2,k)))
                            - 8*(mu(i,j-1,k)*(u(2,i-2,j-1,k)-u(2,i+2,j-1,k)+
                                              8*(-u(2,i-1,j-1,k)+u(2,i+1,j-1,k))) )+8*(
                                                  mu(i,j+1,k)*(u(2,i-2,j+1,k)-u(2,i+2,j+1,k)+
                                                               8*(-u(2,i-1,j+1,k)+u(2,i+1,j+1,k))) ) - (
                                                                   mu(i,j+2,k)*(u(2,i-2,j+2,k)-u(2,i+2,j+2,k)+
                                                                                8*(-u(2,i-1,j+2,k)+u(2,i+1,j+2,k))) )) );
               /*   (la*w_z)_x: NOT CENTERED */
               u3zip2=0;
               u3zip1=0;
               u3zim1=0;
               u3zim2=0;
               for( q=1 ; q <=8 ; q++ )
               {
                  u3zip2 += bope(k,q)*u(3,i+2,j,q);
                  u3zip1 += bope(k,q)*u(3,i+1,j,q);
                  u3zim1 += bope(k,q)*u(3,i-1,j,q);
                  u3zim2 += bope(k,q)*u(3,i-2,j,q);
               }
               lau3zx= i12*(-la(i+2,j,k)*u3zip2 + 8*la(i+1,j,k)*u3zip1
                            -8*la(i-1,j,k)*u3zim1 +   la(i-2,j,k)*u3zim2);
               r1 = r1 + strx(i)*lau3zx;
               /*   (mu*w_x)_z: NOT CENTERED */
               mu3xz=0;
               for( q=1 ; q<=8 ; q++ )
                  mu3xz += bope(k,q)*( mu(i,j,q)*i12*
                                       (-u(3,i+2,j,q) + 8*u(3,i+1,j,q)
                                        -8*u(3,i-1,j,q) + u(3,i-2,j,q)) );
               r1 = r1 + strx(i)*mu3xz;

               /* cross-terms in second component of rhs */
               /*   (mu*u_y)_x */
               r2 = r2 + strx(i)*stry(j)*(
                   i144*( mu(i-2,j,k)*(u(1,i-2,j-2,k)-u(1,i-2,j+2,k)+
                                       8*(-u(1,i-2,j-1,k)+u(1,i-2,j+1,k)))
                          - 8*(mu(i-1,j,k)*(u(1,i-1,j-2,k)-u(1,i-1,j+2,k)+
                                            8*(-u(1,i-1,j-1,k)+u(1,i-1,j+1,k))) )+8*(
                                                mu(i+1,j,k)*(u(1,i+1,j-2,k)-u(1,i+1,j+2,k)+
                                                             8*(-u(1,i+1,j-1,k)+u(1,i+1,j+1,k))) ) - (
                                                                 mu(i+2,j,k)*(u(1,i+2,j-2,k)-u(1,i+2,j+2,k)+
                                                                              8*(-u(1,i+2,j-1,k)+u(1,i+2,j+1,k))) ))
                   /* (la*u_x)_y  */
                   + i144*( la(i,j-2,k)*(u(1,i-2,j-2,k)-u(1,i+2,j-2,k)+
                                         8*(-u(1,i-1,j-2,k)+u(1,i+1,j-2,k)))
                            - 8*(la(i,j-1,k)*(u(1,i-2,j-1,k)-u(1,i+2,j-1,k)+
                                              8*(-u(1,i-1,j-1,k)+u(1,i+1,j-1,k))) )+8*(
                                                  la(i,j+1,k)*(u(1,i-2,j+1,k)-u(1,i+2,j+1,k)+
                                                               8*(-u(1,i-1,j+1,k)+u(1,i+1,j+1,k))) ) - (
                                                                   la(i,j+2,k)*(u(1,i-2,j+2,k)-u(1,i+2,j+2,k)+
                                                                                8*(-u(1,i-1,j+2,k)+u(1,i+1,j+2,k))) )) );
               /* (la*w_z)_y : NOT CENTERED */
               u3zjp2=0;
               u3zjp1=0;
               u3zjm1=0;
               u3zjm2=0;
               for( q=1 ; q <=8 ; q++ )
               {
                  u3zjp2 += bope(k,q)*u(3,i,j+2,q);
                  u3zjp1 += bope(k,q)*u(3,i,j+1,q);
                  u3zjm1 += bope(k,q)*u(3,i,j-1,q);
                  u3zjm2 += bope(k,q)*u(3,i,j-2,q);
               }
               lau3zy= i12*(-la(i,j+2,k)*u3zjp2 + 8*la(i,j+1,k)*u3zjp1
                            -8*la(i,j-1,k)*u3zjm1 + la(i,j-2,k)*u3zjm2);

               r2 = r2 + stry(j)*lau3zy;

               /* (mu*w_y)_z: NOT CENTERED */
               mu3yz=0;
               for(  q=1 ; q <=8 ; q++ )
                  mu3yz += bope(k,q)*( mu(i,j,q)*i12*
                                       (-u(3,i,j+2,q) + 8*u(3,i,j+1,q)
                                        -8*u(3,i,j-1,q) + u(3,i,j-2,q)) );

               r2 = r2 + stry(j)*mu3yz;

               /* No centered cross terms in r3 */
               /*  (mu*u_z)_x: NOT CENTERED */
               u1zip2=0;
               u1zip1=0;
               u1zim1=0;
               u1zim2=0;
               for(  q=1 ; q <=8 ; q++ )
               {
                  u1zip2 += bope(k,q)*u(1,i+2,j,q);
                  u1zip1 += bope(k,q)*u(1,i+1,j,q);
                  u1zim1 += bope(k,q)*u(1,i-1,j,q);
                  u1zim2 += bope(k,q)*u(1,i-2,j,q);
               }
               mu1zx= i12*(-mu(i+2,j,k)*u1zip2 + 8*mu(i+1,j,k)*u1zip1
                           -8*mu(i-1,j,k)*u1zim1 + mu(i-2,j,k)*u1zim2);
               r3 = r3 + strx(i)*mu1zx;

               /* (mu*v_z)_y: NOT CENTERED */
               u2zjp2=0;
               u2zjp1=0;
               u2zjm1=0;
               u2zjm2=0;
               for(  q=1 ; q <=8 ; q++ )
               {
                  u2zjp2 += bope(k,q)*u(2,i,j+2,q);
                  u2zjp1 += bope(k,q)*u(2,i,j+1,q);
                  u2zjm1 += bope(k,q)*u(2,i,j-1,q);
                  u2zjm2 += bope(k,q)*u(2,i,j-2,q);
               }
               mu2zy= i12*(-mu(i,j+2,k)*u2zjp2 + 8*mu(i,j+1,k)*u2zjp1
                           -8*mu(i,j-1,k)*u2zjm1 + mu(i,j-2,k)*u2zjm2);
               r3 = r3 + stry(j)*mu2zy;

               /*   (la*u_x)_z: NOT CENTERED */
               lau1xz=0;
               for(  q=1 ; q <=8 ; q++ )
                  lau1xz += bope(k,q)*( la(i,j,q)*i12*
                                        (-u(1,i+2,j,q) + 8*u(1,i+1,j,q)
                                         -8*u(1,i-1,j,q) + u(1,i-2,j,q)) );
               r3 = r3 + strx(i)*lau1xz;

               /* (la*v_y)_z: NOT CENTERED */
               lau2yz=0;
               for(  q=1 ; q <=8 ; q++ )
                  lau2yz += bope(k,q)*( la(i,j,q)*i12*
                                        (-u(2,i,j+2,q) + 8*u(2,i,j+1,q)
                                         -8*u(2,i,j-1,q) + u(2,i,j-2,q)) );
               r3 = r3 + stry(j)*lau2yz;

               lu(1,i,j,k) = a1*lu(1,i,j,k) + cof*r1;
               lu(2,i,j,k) = a1*lu(2,i,j,k) + cof*r2;
               lu(3,i,j,k) = a1*lu(3,i,j,k) + cof*r3;
            }
}

#pragma rstream inline
void core_fn3(int ifirst, int ilast, int jfirst, int jlast, int kfirst,
    int klast, int nk, int* onesided, float_sw4 (*a_acof)[8][6],
    float_sw4 (*a_bope)[6], float_sw4 *a_ghcof, float_sw4 (*a_lu)[NJ][NI][NC],
    float_sw4 (*a_u)[NJ][NI][NC], float_sw4 (*a_mu)[NJ][NI],
    float_sw4 (*a_lambda)[NJ][NI], float_sw4 h, float_sw4* a_strx,
    float_sw4* a_stry, float_sw4* a_strz, int k1, int k2, int base, int base3)
{
   int i, j, k, qb, mb, kb;
   float_sw4 mux1, mux2, mux3, mux4, muy1, muy2, muy3, muy4;
   float_sw4 r1, r2, r3, cof, mucof, mu1zz, mu2zz, mu3zz;
   float_sw4 lap2mu, u3zip2, u3zip1, u3zim1, u3zim2, lau3zx, mu3xz, u3zjp2, u3zjp1, u3zjm1, u3zjm2;
   float_sw4 lau3zy, mu3yz, mu1zx, mu2zy, u1zip2, u1zip1, u1zim1, u1zim2;
   float_sw4 u2zjp2, u2zjp1, u2zjm1, u2zjm2, lau1xz, lau2yz;

   const float_sw4 a1   = 0;
   const float_sw4 i6   = 1.0/6;
   const float_sw4 i12  = 1.0/12;
   const float_sw4 i144 = 1.0/144;
   const float_sw4 tf   = 0.75;

   for(  k = nk-5 ; k <= nk ; k++ )
      {
         for(  j=jfirst+2; j<=jlast-2; j++ )
         {
            for(  i=ifirst+2; i<=ilast-2; i++ )
            {
               /* from inner_loop_4a */
               mux1 = mu(i-1,j,k)*strx(i-1)-
                   tf*(mu(i,j,k)*strx(i)+mu(i-2,j,k)*strx(i-2));
               mux2 = mu(i-2,j,k)*strx(i-2)+mu(i+1,j,k)*strx(i+1)+
                   3*(mu(i,j,k)*strx(i)+mu(i-1,j,k)*strx(i-1));
               mux3 = mu(i-1,j,k)*strx(i-1)+mu(i+2,j,k)*strx(i+2)+
                   3*(mu(i+1,j,k)*strx(i+1)+mu(i,j,k)*strx(i));
               mux4 = mu(i+1,j,k)*strx(i+1)-
                   tf*(mu(i,j,k)*strx(i)+mu(i+2,j,k)*strx(i+2));

               muy1 = mu(i,j-1,k)*stry(j-1)-
                   tf*(mu(i,j,k)*stry(j)+mu(i,j-2,k)*stry(j-2));
               muy2 = mu(i,j-2,k)*stry(j-2)+mu(i,j+1,k)*stry(j+1)+
                   3*(mu(i,j,k)*stry(j)+mu(i,j-1,k)*stry(j-1));
               muy3 = mu(i,j-1,k)*stry(j-1)+mu(i,j+2,k)*stry(j+2)+
                   3*(mu(i,j+1,k)*stry(j+1)+mu(i,j,k)*stry(j));
               muy4 = mu(i,j+1,k)*stry(j+1)-
                   tf*(mu(i,j,k)*stry(j)+mu(i,j+2,k)*stry(j+2));

               /* xx, yy, and zz derivatives: */
               /* note that we could have introduced intermediate variables for the average of lambda  */
               /* in the same way as we did for mu */
               r1 = i6*(strx(i)*((2*mux1+la(i-1,j,k)*strx(i-1)-
                                  tf*(la(i,j,k)*strx(i)+la(i-2,j,k)*strx(i-2)))*
                                 (u(1,i-2,j,k)-u(1,i,j,k))+
                                 (2*mux2+la(i-2,j,k)*strx(i-2)+la(i+1,j,k)*strx(i+1)+
                                  3*(la(i,j,k)*strx(i)+la(i-1,j,k)*strx(i-1)))*
                                 (u(1,i-1,j,k)-u(1,i,j,k))+
                                 (2*mux3+la(i-1,j,k)*strx(i-1)+la(i+2,j,k)*strx(i+2)+
                                  3*(la(i+1,j,k)*strx(i+1)+la(i,j,k)*strx(i)))*
                                 (u(1,i+1,j,k)-u(1,i,j,k))+
                                 (2*mux4+ la(i+1,j,k)*strx(i+1)-
                                  tf*(la(i,j,k)*strx(i)+la(i+2,j,k)*strx(i+2)))*
                                 (u(1,i+2,j,k)-u(1,i,j,k)) ) + stry(j)*(
                                     + muy1*(u(1,i,j-2,k)-u(1,i,j,k)) +
                                     muy2*(u(1,i,j-1,k)-u(1,i,j,k)) +
                                     muy3*(u(1,i,j+1,k)-u(1,i,j,k)) +
                                     muy4*(u(1,i,j+2,k)-u(1,i,j,k)) ) );

               /* all indices ending with 'b' are indices relative to the boundary, going into the domain (1,2,3,...)*/
               kb = nk-k+1;
               /* all coefficient arrays (acof, bope, ghcof) should be indexed with these indices */
               /* all solution and material property arrays should be indexed with (i,j,k) */

               /* (mu*uz)_z can not be centered */
               /* second derivative (mu*u_z)_z at grid point z_k */
               /* averaging the coefficient */
               mu1zz = 0;
               mu2zz = 0;
               mu3zz = 0;
               for(  qb=1; qb <= 8 ; qb++ )
               {
                  mucof = 0;
                  lap2mu = 0;
                  for(  mb=1; mb <= 8; mb++ )
                  {
                     mucof  += acof(kb,qb,mb)*mu(i,j,nk-mb+1);
                     lap2mu += acof(kb,qb,mb)*(2*mu(i,j,nk-mb+1)+la(i,j,nk-mb+1));
                  }
                  mu1zz += mucof*u(1,i,j,nk-qb+1);
                  mu2zz += mucof*u(2,i,j,nk-qb+1);
                  mu3zz += lap2mu*u(3,i,j,nk-qb+1);
               }
               /* computing the second derivative */
               /* ghost point only influences the first point (k=1) because ghcof(k)=0 for k>=2*/
               r1 = r1 + (mu1zz + ghcof(kb)*mu(i,j,nk)*u(1,i,j,nk+1));

               r2 = i6*(strx(i)*(mux1*(u(2,i-2,j,k)-u(2,i,j,k)) +
                                 mux2*(u(2,i-1,j,k)-u(2,i,j,k)) +
                                 mux3*(u(2,i+1,j,k)-u(2,i,j,k)) +
                                 mux4*(u(2,i+2,j,k)-u(2,i,j,k)) )+ stry(j)*(
                                     (2*muy1+la(i,j-1,k)*stry(j-1)-
                                      tf*(la(i,j,k)*stry(j)+la(i,j-2,k)*stry(j-2)))*
                                     (u(2,i,j-2,k)-u(2,i,j,k))+
                                     (2*muy2+la(i,j-2,k)*stry(j-2)+la(i,j+1,k)*stry(j+1)+
                                      3*(la(i,j,k)*stry(j)+la(i,j-1,k)*stry(j-1)))*
                                     (u(2,i,j-1,k)-u(2,i,j,k))+
                                     (2*muy3+la(i,j-1,k)*stry(j-1)+la(i,j+2,k)*stry(j+2)+
                                      3*(la(i,j+1,k)*stry(j+1)+la(i,j,k)*stry(j)))*
                                     (u(2,i,j+1,k)-u(2,i,j,k))+
                                     (2*muy4+la(i,j+1,k)*stry(j+1)-
                                      tf*(la(i,j,k)*stry(j)+la(i,j+2,k)*stry(j+2)))*
                                     (u(2,i,j+2,k)-u(2,i,j,k)) ) );

               /* (mu*vz)_z can not be centered */
               /* second derivative (mu*v_z)_z at grid point z_k */
               /* averaging the coefficient: already done above */
               r2 = r2 + (mu2zz + ghcof(kb)*mu(i,j,nk)*u(2,i,j,nk+1));

               r3 = i6*(strx(i)*(mux1*(u(3,i-2,j,k)-u(3,i,j,k)) +
                                 mux2*(u(3,i-1,j,k)-u(3,i,j,k)) +
                                 mux3*(u(3,i+1,j,k)-u(3,i,j,k)) +
                                 mux4*(u(3,i+2,j,k)-u(3,i,j,k))  ) + stry(j)*(
                                     muy1*(u(3,i,j-2,k)-u(3,i,j,k)) +
                                     muy2*(u(3,i,j-1,k)-u(3,i,j,k)) +
                                     muy3*(u(3,i,j+1,k)-u(3,i,j,k)) +
                                     muy4*(u(3,i,j+2,k)-u(3,i,j,k)) ) );
               r3 = r3 + (mu3zz + ghcof(kb)*(la(i,j,nk)+2*mu(i,j,nk))*
                          u(3,i,j,nk+1));

               /* cross-terms in first component of rhs */
               /*   (la*v_y)_x */
               r1 = r1 + strx(i)*stry(j)*(
                   i144*( la(i-2,j,k)*(u(2,i-2,j-2,k)-u(2,i-2,j+2,k)+
                                       8*(-u(2,i-2,j-1,k)+u(2,i-2,j+1,k)))
                          - 8*(la(i-1,j,k)*(u(2,i-1,j-2,k)-u(2,i-1,j+2,k)+
                                            8*(-u(2,i-1,j-1,k)+u(2,i-1,j+1,k))) )+8*(
                                                la(i+1,j,k)*(u(2,i+1,j-2,k)-u(2,i+1,j+2,k)+
                                                             8*(-u(2,i+1,j-1,k)+u(2,i+1,j+1,k))) ) - (
                                                                 la(i+2,j,k)*(u(2,i+2,j-2,k)-u(2,i+2,j+2,k)+
                                                                              8*(-u(2,i+2,j-1,k)+u(2,i+2,j+1,k))) ))
                   /*   (mu*v_x)_y */
                   + i144*( mu(i,j-2,k)*(u(2,i-2,j-2,k)-u(2,i+2,j-2,k)+
                                         8*(-u(2,i-1,j-2,k)+u(2,i+1,j-2,k)))
                            - 8*(mu(i,j-1,k)*(u(2,i-2,j-1,k)-u(2,i+2,j-1,k)+
                                              8*(-u(2,i-1,j-1,k)+u(2,i+1,j-1,k))) )+8*(
                                                  mu(i,j+1,k)*(u(2,i-2,j+1,k)-u(2,i+2,j+1,k)+
                                                               8*(-u(2,i-1,j+1,k)+u(2,i+1,j+1,k))) ) - (
                                                                   mu(i,j+2,k)*(u(2,i-2,j+2,k)-u(2,i+2,j+2,k)+
                                                                                8*(-u(2,i-1,j+2,k)+u(2,i+1,j+2,k))) )) );
               /*   (la*w_z)_x: NOT CENTERED */
               u3zip2=0;
               u3zip1=0;
               u3zim1=0;
               u3zim2=0;
               for(  qb=1; qb <= 8 ; qb++ )
               {
                  u3zip2 -= bope(kb,qb)*u(3,i+2,j,nk-qb+1);
                  u3zip1 -= bope(kb,qb)*u(3,i+1,j,nk-qb+1);
                  u3zim1 -= bope(kb,qb)*u(3,i-1,j,nk-qb+1);
                  u3zim2 -= bope(kb,qb)*u(3,i-2,j,nk-qb+1);
               }
               lau3zx= i12*(-la(i+2,j,k)*u3zip2 + 8*la(i+1,j,k)*u3zip1
                            -8*la(i-1,j,k)*u3zim1 + la(i-2,j,k)*u3zim2);
               r1 = r1 + strx(i)*lau3zx;

               /*   (mu*w_x)_z: NOT CENTERED */
               mu3xz=0;
               for(  qb=1; qb <= 8 ; qb++ )
                  mu3xz -= bope(kb,qb)*( mu(i,j,nk-qb+1)*i12*
                                         (-u(3,i+2,j,nk-qb+1) + 8*u(3,i+1,j,nk-qb+1)
                                          -8*u(3,i-1,j,nk-qb+1) + u(3,i-2,j,nk-qb+1)) );

               r1 = r1 + strx(i)*mu3xz;

               /* cross-terms in second component of rhs */
               /*   (mu*u_y)_x */
               r2 = r2 + strx(i)*stry(j)*(
                   i144*( mu(i-2,j,k)*(u(1,i-2,j-2,k)-u(1,i-2,j+2,k)+
                                       8*(-u(1,i-2,j-1,k)+u(1,i-2,j+1,k)))
                          - 8*(mu(i-1,j,k)*(u(1,i-1,j-2,k)-u(1,i-1,j+2,k)+
                                            8*(-u(1,i-1,j-1,k)+u(1,i-1,j+1,k))) )+8*(
                                                mu(i+1,j,k)*(u(1,i+1,j-2,k)-u(1,i+1,j+2,k)+
                                                             8*(-u(1,i+1,j-1,k)+u(1,i+1,j+1,k))) ) - (
                                                                 mu(i+2,j,k)*(u(1,i+2,j-2,k)-u(1,i+2,j+2,k)+
                                                                              8*(-u(1,i+2,j-1,k)+u(1,i+2,j+1,k))) ))
                   /* (la*u_x)_y */
                   + i144*( la(i,j-2,k)*(u(1,i-2,j-2,k)-u(1,i+2,j-2,k)+
                                         8*(-u(1,i-1,j-2,k)+u(1,i+1,j-2,k)))
                            - 8*(la(i,j-1,k)*(u(1,i-2,j-1,k)-u(1,i+2,j-1,k)+
                                              8*(-u(1,i-1,j-1,k)+u(1,i+1,j-1,k))) )+8*(
                                                  la(i,j+1,k)*(u(1,i-2,j+1,k)-u(1,i+2,j+1,k)+
                                                               8*(-u(1,i-1,j+1,k)+u(1,i+1,j+1,k))) ) - (
                                                                   la(i,j+2,k)*(u(1,i-2,j+2,k)-u(1,i+2,j+2,k)+
                                                                                8*(-u(1,i-1,j+2,k)+u(1,i+1,j+2,k))) )) );
               /* (la*w_z)_y : NOT CENTERED */
               u3zjp2=0;
               u3zjp1=0;
               u3zjm1=0;
               u3zjm2=0;
               for(  qb=1; qb <= 8 ; qb++ )
               {
                  u3zjp2 -= bope(kb,qb)*u(3,i,j+2,nk-qb+1);
                  u3zjp1 -= bope(kb,qb)*u(3,i,j+1,nk-qb+1);
                  u3zjm1 -= bope(kb,qb)*u(3,i,j-1,nk-qb+1);
                  u3zjm2 -= bope(kb,qb)*u(3,i,j-2,nk-qb+1);
               }
               lau3zy= i12*(-la(i,j+2,k)*u3zjp2 + 8*la(i,j+1,k)*u3zjp1
                            -8*la(i,j-1,k)*u3zjm1 + la(i,j-2,k)*u3zjm2);
               r2 = r2 + stry(j)*lau3zy;

               /* (mu*w_y)_z: NOT CENTERED */
               mu3yz=0;
               for(  qb=1; qb <= 8 ; qb++ )
                  mu3yz -= bope(kb,qb)*( mu(i,j,nk-qb+1)*i12*
                                         (-u(3,i,j+2,nk-qb+1) + 8*u(3,i,j+1,nk-qb+1)
                                          -8*u(3,i,j-1,nk-qb+1) + u(3,i,j-2,nk-qb+1)) );
               r2 = r2 + stry(j)*mu3yz;

               /* No centered cross terms in r3 */
               /*  (mu*u_z)_x: NOT CENTERED */
               u1zip2=0;
               u1zip1=0;
               u1zim1=0;
               u1zim2=0;
               for(  qb=1; qb <= 8 ; qb++ )
               {
                  u1zip2 -= bope(kb,qb)*u(1,i+2,j,nk-qb+1);
                  u1zip1 -= bope(kb,qb)*u(1,i+1,j,nk-qb+1);
                  u1zim1 -= bope(kb,qb)*u(1,i-1,j,nk-qb+1);
                  u1zim2 -= bope(kb,qb)*u(1,i-2,j,nk-qb+1);
               }
               mu1zx= i12*(-mu(i+2,j,k)*u1zip2 + 8*mu(i+1,j,k)*u1zip1
                           -8*mu(i-1,j,k)*u1zim1 + mu(i-2,j,k)*u1zim2);
               r3 = r3 + strx(i)*mu1zx;

               /* (mu*v_z)_y: NOT CENTERED */
               u2zjp2=0;
               u2zjp1=0;
               u2zjm1=0;
               u2zjm2=0;
               for(  qb=1; qb <= 8 ; qb++ )
               {
                  u2zjp2 -= bope(kb,qb)*u(2,i,j+2,nk-qb+1);
                  u2zjp1 -= bope(kb,qb)*u(2,i,j+1,nk-qb+1);
                  u2zjm1 -= bope(kb,qb)*u(2,i,j-1,nk-qb+1);
                  u2zjm2 -= bope(kb,qb)*u(2,i,j-2,nk-qb+1);
               }
               mu2zy= i12*(-mu(i,j+2,k)*u2zjp2 + 8*mu(i,j+1,k)*u2zjp1
                           -8*mu(i,j-1,k)*u2zjm1 + mu(i,j-2,k)*u2zjm2);
               r3 = r3 + stry(j)*mu2zy;

               /*   (la*u_x)_z: NOT CENTERED */
               lau1xz=0;
               for(  qb=1; qb <= 8 ; qb++ )
                  lau1xz -= bope(kb,qb)*( la(i,j,nk-qb+1)*i12*
                                          (-u(1,i+2,j,nk-qb+1) + 8*u(1,i+1,j,nk-qb+1)
                                           -8*u(1,i-1,j,nk-qb+1) + u(1,i-2,j,nk-qb+1)) );
               r3 = r3 + strx(i)*lau1xz;

               /* (la*v_y)_z: NOT CENTERED */
               lau2yz=0;
               for(  qb=1; qb <= 8 ; qb++ )
               {
                  lau2yz -= bope(kb,qb)*( la(i,j,nk-qb+1)*i12*
                                          (-u(2,i,j+2,nk-qb+1) + 8*u(2,i,j+1,nk-qb+1)
                                           -8*u(2,i,j-1,nk-qb+1) + u(2,i,j-2,nk-qb+1)) );
               }
               r3 = r3 + stry(j)*lau2yz;

               lu(1,i,j,k) = a1*lu(1,i,j,k) + cof*r1;
               lu(2,i,j,k) = a1*lu(2,i,j,k) + cof*r2;
               lu(3,i,j,k) = a1*lu(3,i,j,k) + cof*r3;
            }
         }
      }
}

#undef mu
#undef la
#undef u
#undef lu
#undef strx
#undef stry
#undef strz


#pragma rstream map
void mapped_fn1(int ifirst, int ilast, int jfirst, int jlast, int kfirst,
    int klast, int nk, int* onesided, float_sw4 (*a_acof)[8][6],
    float_sw4 (*a_bope)[6], float_sw4 *a_ghcof, float_sw4 (*a_lu)[NJ][NI][NC],
    float_sw4 (*a_u)[NJ][NI][NC], float_sw4 (*a_mu)[NJ][NI],
    float_sw4 (*a_lambda)[NJ][NI], float_sw4 h, float_sw4* a_strx,
    float_sw4* a_stry, float_sw4* a_strz, int k1, int k2, int base, int base3)
{
    core_fn1(ifirst, ilast, jfirst, jlast, kfirst, klast, nk, onesided, a_acof,
        a_bope, a_ghcof, a_lu, a_u, a_mu, a_lambda, h, a_strx, a_stry, a_strz,
        k1, k2, base, base3);
}

//#pragma rstream map
void mapped_fn2(int ifirst, int ilast, int jfirst, int jlast, int kfirst,
    int klast, int nk, int* onesided, float_sw4 (*a_acof)[8][6],
    float_sw4 (*a_bope)[6], float_sw4 *a_ghcof, float_sw4 (*a_lu)[NJ][NI][NC],
    float_sw4 (*a_u)[NJ][NI][NC], float_sw4 (*a_mu)[NJ][NI],
    float_sw4 (*a_lambda)[NJ][NI], float_sw4 h, float_sw4* a_strx,
    float_sw4* a_stry, float_sw4* a_strz, int k1, int k2, int base, int base3)
{
    core_fn2(ifirst, ilast, jfirst, jlast, kfirst, klast, nk, onesided, a_acof,
        a_bope, a_ghcof, a_lu, a_u, a_mu, a_lambda, h, a_strx, a_stry, a_strz,
        k1, k2, base, base3);
}

//#pragma rstream map
void mapped_fn3(int ifirst, int ilast, int jfirst, int jlast, int kfirst,
    int klast, int nk, int* onesided, float_sw4 (*a_acof)[8][6],
    float_sw4 (*a_bope)[6], float_sw4 *a_ghcof, float_sw4 (*a_lu)[NJ][NI][NC],
    float_sw4 (*a_u)[NJ][NI][NC], float_sw4 (*a_mu)[NJ][NI],
    float_sw4 (*a_lambda)[NJ][NI], float_sw4 h, float_sw4* a_strx,
    float_sw4* a_stry, float_sw4* a_strz, int k1, int k2, int base, int base3)
{
    core_fn3(ifirst, ilast, jfirst, jlast, kfirst, klast, nk, onesided, a_acof,
        a_bope, a_ghcof, a_lu, a_u, a_mu, a_lambda, h, a_strx, a_stry, a_strz,
        k1, k2, base, base3);
}

void kernel() {
    int k1, k2;

    int ifirst = 0;
    int ilast = NI - 1;
    int jfirst = 0;
    int jlast = NJ - 1;
    int kfirst = 0;
    int klast = NK - 1;

    const int ni    = ilast-ifirst+1;
    const int nij   = ni*(jlast-jfirst+1);
    const int base  = -(ifirst+ni*jfirst+nij*kfirst);
    const int base3 = 3*base-1;

    double h = 1.0 / (NI - 1);

    k1 = kfirst + 2;
    if (onesided[4] == 1) {
        k1 = 7;
    }

    k2 = klast - 2;
    if (onesided[5] == 1) {
        k2 = NK - 6;
    }

    mapped_fn1(ifirst, ilast, jfirst, jlast, kfirst, klast, NK, onesided, acof,
        bope, ghcof, lu, u, mu, lambda, h, strx, stry, strz, k1, k2, base,
        base3);
    if (onesided[4] == 1) {
        mapped_fn2(ifirst, ilast, jfirst, jlast, kfirst, klast, NK, onesided,
            acof, bope, ghcof, lu, u, mu, lambda, h, strx, stry, strz, k1, k2,
            base, base3);
    }
    if (onesided[5] == 1) {
        mapped_fn3(ifirst, ilast, jfirst, jlast, kfirst, klast, NK, onesided,
            acof, bope, ghcof, lu, u, mu, lambda, h, strx, stry, strz, k1, k2,
            base, base3);
    }
}

static void unmapped_kernel(float_sw4 (*lu)[NJ][NI][NC]) {
    int k1, k2;

    int ifirst = 0;
    int ilast = NI - 1;
    int jfirst = 0;
    int jlast = NJ - 1;
    int kfirst = 0;
    int klast = NK - 1;

    const int ni    = ilast-ifirst+1;
    const int nij   = ni*(jlast-jfirst+1);
    const int base  = -(ifirst+ni*jfirst+nij*kfirst);
    const int base3 = 3*base-1;

    double h = 1.0 / (NI - 1);

    k1 = kfirst + 2;
    if (onesided[4] == 1) {
        k1 = 7;
    }

    k2 = klast - 2;
    if (onesided[5] == 1) {
        k2 = NK - 6;
    }

    core_fn1(ifirst, ilast, jfirst, jlast, kfirst, klast, NK, onesided, acof,
        bope, ghcof, lu, u, mu, lambda, h, strx, stry, strz, k1, k2, base,
        base3);
    if (onesided[4] == 1) {
        core_fn2(ifirst, ilast, jfirst, jlast, kfirst, klast, NK, onesided,
            acof, bope, ghcof, lu, u, mu, lambda, h, strx, stry, strz, k1, k2,
            base, base3);
    }
    if (onesided[5] == 1) {
        core_fn3(ifirst, ilast, jfirst, jlast, kfirst, klast, NK, onesided,
            acof, bope, ghcof, lu, u, mu, lambda, h, strx, stry, strz, k1, k2,
            base, base3);
    }
}

void show() {
}

int check() {
    int i, j, k, c;

    float_sw4 (*lu2)[NJ][NI][NC] =
        calloc(NK * NJ * NI * NC,  sizeof(float_sw4));
    unmapped_kernel(lu2);

    for (k = 0; k < NK; k++) {
        for (j = 0; j < NJ; j++) {
            for (i = 0; i < NI; i++) {
                for (c = 0; c < NC; c++) {
                    if (!fequal(lu[k][j][i][c], lu2[k][j][i][c])) {
                        return 1;
                    }
                }
            }
        }
    }

    return 0;
}

double flops_per_trial() {
    return 666 * (NI - 4) * (NJ - 4) * (NK - 4);
}

int nb_samples = NI * NJ * NK * NC;
char const * function_name = "rhs4sg";

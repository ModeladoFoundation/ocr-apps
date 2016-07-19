#include "test.h"
 one_line_type sinf_vec[] = {
{31, 0,123,__LINE__, 0xbfedd343, 0xb0aef9e0, 0xbff33333, 0x33333333},			/* -0.93203=f(-1.20000)*/
{31, 0,123,__LINE__, 0xbfedb532, 0xde4934c0, 0xbff30a3d, 0x70a3d70a},			/* -0.92836=f(-1.19000)*/
{31, 0,123,__LINE__, 0xbfed965f, 0x4d4b79b0, 0xbff2e147, 0xae147ae1},			/* -0.92460=f(-1.18000)*/
{31, 0,123,__LINE__, 0xbfed76c9, 0xecfd00f0, 0xbff2b851, 0xeb851eb8},			/* -0.92075=f(-1.17000)*/
{31, 0,123,__LINE__, 0xbfed5673, 0x708e95c0, 0xbff28f5c, 0x28f5c28f},			/* -0.91680=f(-1.16000)*/
{31, 0,123,__LINE__, 0xbfed355c, 0xae3d9410, 0xbff26666, 0x66666666},			/* -0.91276=f(-1.15000)*/
{11, 0,123,__LINE__, 0xbfed1386, 0x898f05a0, 0xbff23d70, 0xa3d70a3d},			/* -0.90863=f(-1.14000)*/
{11, 0,123,__LINE__, 0xbfecf0f1, 0xd7ca8350, 0xbff2147a, 0xe147ae14},			/* -0.90441=f(-1.13000)*/
{11, 0,123,__LINE__, 0xbfeccd9f, 0x72b28700, 0xbff1eb85, 0x1eb851eb},			/* -0.90010=f(-1.12000)*/
{11, 0,123,__LINE__, 0xbfeca990, 0x532842a0, 0xbff1c28f, 0x5c28f5c2},			/* -0.89569=f(-1.11000)*/
{11, 0,123,__LINE__, 0xbfec84c5, 0x525ed360, 0xbff19999, 0x99999999},			/* -0.89120=f(-1.10000)*/
{11, 0,123,__LINE__, 0xbfec5f3f, 0x71376ab0, 0xbff170a3, 0xd70a3d70},			/* -0.88662=f(-1.09000)*/
{11, 0,123,__LINE__, 0xbfec38ff, 0xa2d0aa60, 0xbff147ae, 0x147ae147},			/* -0.88195=f(-1.08000)*/
{11, 0,123,__LINE__, 0xbfec1206, 0xde7229c0, 0xbff11eb8, 0x51eb851e},			/* -0.87720=f(-1.07000)*/
{11, 0,123,__LINE__, 0xbfebea55, 0xfbd69100, 0xbff0f5c2, 0x8f5c28f5},			/* -0.87235=f(-1.06000)*/
{11, 0,123,__LINE__, 0xbfebc1ee, 0x52738ea0, 0xbff0cccc, 0xcccccccc},			/* -0.86742=f(-1.05000)*/
{11, 0,123,__LINE__, 0xbfeb98d0, 0xb193c800, 0xbff0a3d7, 0x0a3d70a3},			/* -0.86240=f(-1.04000)*/
{11, 0,123,__LINE__, 0xbfeb6efe, 0x42186dc0, 0xbff07ae1, 0x47ae147a},			/* -0.85729=f(-1.03000)*/
{11, 0,123,__LINE__, 0xbfeb4478, 0x06f72b40, 0xbff051eb, 0x851eb851},			/* -0.85210=f(-1.02000)*/
{11, 0,123,__LINE__, 0xbfeb193f, 0x17c982a0, 0xbff028f5, 0xc28f5c28},			/* -0.84683=f(-1.01000)*/
{13, 0,123,__LINE__, 0xbfeaed54, 0x90000000, 0xbfefffff, 0xfffffffe},			/* -0.84147=f(-0.01000)*/
{13, 0,123,__LINE__, 0xbfeac0b9, 0x8ee23980, 0xbfefae14, 0x7ae147ac},			/* -0.83602=f(-0.99000)*/
{13, 0,123,__LINE__, 0xbfea936f, 0x3f65d980, 0xbfef5c28, 0xf5c28f5a},			/* -0.83049=f(-0.98000)*/
{13, 0,123,__LINE__, 0xbfea6576, 0xc0808a20, 0xbfef0a3d, 0x70a3d708},			/* -0.82488=f(-0.97000)*/
{13, 0,123,__LINE__, 0xbfea36d1, 0x310cb938, 0xbfeeb851, 0xeb851eb6},			/* -0.81919=f(-0.96000)*/
{13, 0,123,__LINE__, 0xbfea077f, 0xee1ec208, 0xbfee6666, 0x66666664},			/* -0.81341=f(-0.95000)*/
{13, 0,123,__LINE__, 0xbfe9d784, 0x1642a610, 0xbfee147a, 0xe147ae12},			/* -0.80755=f(-0.94000)*/
{13, 0,123,__LINE__, 0xbfe9a6de, 0xdf4f5690, 0xbfedc28f, 0x5c28f5c0},			/* -0.80161=f(-0.93000)*/
{13, 0,123,__LINE__, 0xbfe97591, 0x8a4ef7c0, 0xbfed70a3, 0xd70a3d6e},			/* -0.79560=f(-0.92000)*/
{13, 0,123,__LINE__, 0xbfe9439d, 0x63187a70, 0xbfed1eb8, 0x51eb851c},			/* -0.78950=f(-0.91000)*/
{13, 0,123,__LINE__, 0xbfe91103, 0x8e759800, 0xbfeccccc, 0xccccccca},			/* -0.78332=f(-0.90000)*/
{13, 0,123,__LINE__, 0xbfe8ddc5, 0x8f9d4eb0, 0xbfec7ae1, 0x47ae1478},			/* -0.77707=f(-0.89000)*/
{13, 0,123,__LINE__, 0xbfe8a9e4, 0x92e0bad0, 0xbfec28f5, 0xc28f5c26},			/* -0.77073=f(-0.88000)*/
{14, 0,123,__LINE__, 0xbfe87561, 0xf591ade8, 0xbfebd70a, 0x3d70a3d4},			/* -0.76432=f(-0.87000)*/
{14, 0,123,__LINE__, 0xbfe8403f, 0x112aef10, 0xbfeb851e, 0xb851eb82},			/* -0.75784=f(-0.86000)*/
{14, 0,123,__LINE__, 0xbfe80a7d, 0x3f1d07b8, 0xbfeb3333, 0x33333330},			/* -0.75128=f(-0.85000)*/
{12, 0,123,__LINE__, 0xbfe7d41d, 0xc35bbdac, 0xbfeae147, 0xae147ade},			/* -0.74464=f(-0.84000)*/
{12, 0,123,__LINE__, 0xbfe79d22, 0x3a3be0b4, 0xbfea8f5c, 0x28f5c28c},			/* -0.73793=f(-0.83000)*/
{12, 0,123,__LINE__, 0xbfe7658b, 0xee7f0040, 0xbfea3d70, 0xa3d70a3a},			/* -0.73114=f(-0.82000)*/
{12, 0,123,__LINE__, 0xbfe72d5c, 0x4dc8d870, 0xbfe9eb85, 0x1eb851e8},			/* -0.72428=f(-0.81000)*/
{12, 0,123,__LINE__, 0xbfe6f494, 0xc5bd2530, 0xbfe99999, 0x99999996},			/* -0.71735=f(-0.80000)*/
{12, 0,123,__LINE__, 0xbfe6bb36, 0xd0d6ac78, 0xbfe947ae, 0x147ae144},			/* -0.71035=f(-0.79000)*/
{12, 0,123,__LINE__, 0xbfe68143, 0xcf973f00, 0xbfe8f5c2, 0x8f5c28f2},			/* -0.70327=f(-0.78000)*/
{12, 0,123,__LINE__, 0xbfe646bd, 0x60246e20, 0xbfe8a3d7, 0x0a3d70a0},			/* -0.69613=f(-0.77000)*/
{12, 0,123,__LINE__, 0xbfe60ba4, 0xf91e6ca0, 0xbfe851eb, 0x851eb84e},			/* -0.68892=f(-0.76000)*/
{14, 0,123,__LINE__, 0xbfe5cffc, 0x18000000, 0xbfe7ffff, 0xfffffffc},			/* -0.68163=f(-0.75000)*/
{14, 0,123,__LINE__, 0xbfe593c4, 0x4095d910, 0xbfe7ae14, 0x7ae147aa},			/* -0.67428=f(-0.74000)*/
{14, 0,123,__LINE__, 0xbfe556ff, 0x058df020, 0xbfe75c28, 0xf5c28f58},			/* -0.66686=f(-0.73000)*/
{14, 0,123,__LINE__, 0xbfe519ad, 0xf0a07ac0, 0xbfe70a3d, 0x70a3d706},			/* -0.65938=f(-0.72000)*/
{14, 0,123,__LINE__, 0xbfe4dbd2, 0x770615a0, 0xbfe6b851, 0xeb851eb4},			/* -0.65183=f(-0.71000)*/
{14, 0,123,__LINE__, 0xbfe49d6e, 0x634f5ccc, 0xbfe66666, 0x66666662},			/* -0.64421=f(-0.70000)*/
{14, 0,123,__LINE__, 0xbfe45e83, 0x390f2640, 0xbfe6147a, 0xe147ae10},			/* -0.63653=f(-0.69000)*/
{15, 0,123,__LINE__, 0xbfe41f12, 0x8df369ac, 0xbfe5c28f, 0x5c28f5be},			/* -0.62879=f(-0.68000)*/
{13, 0,123,__LINE__, 0xbfe3df1e, 0x0843b82c, 0xbfe570a3, 0xd70a3d6c},			/* -0.62098=f(-0.67000)*/
{13, 0,123,__LINE__, 0xbfe39ea7, 0x461eacec, 0xbfe51eb8, 0x51eb851a},			/* -0.61311=f(-0.66000)*/
{13, 0,123,__LINE__, 0xbfe35daf, 0xd7e1ea68, 0xbfe4cccc, 0xccccccc8},			/* -0.60518=f(-0.65000)*/
{13, 0,123,__LINE__, 0xbfe31c39, 0x978dbc50, 0xbfe47ae1, 0x47ae1476},			/* -0.59719=f(-0.64000)*/
{13, 0,123,__LINE__, 0xbfe2da46, 0x185db328, 0xbfe428f5, 0xc28f5c24},			/* -0.58914=f(-0.63000)*/
{15, 0,123,__LINE__, 0xbfe297d7, 0x0dadda58, 0xbfe3d70a, 0x3d70a3d2},			/* -0.58103=f(-0.62000)*/
{15, 0,123,__LINE__, 0xbfe254ee, 0x2997adc4, 0xbfe3851e, 0xb851eb80},			/* -0.57286=f(-0.61000)*/
{15, 0,123,__LINE__, 0xbfe2118d, 0x233ee698, 0xbfe33333, 0x3333332e},			/* -0.56464=f(-0.60000)*/
{14, 0,123,__LINE__, 0xbfe1cdb5, 0x961530b0, 0xbfe2e147, 0xae147adc},			/* -0.55636=f(-0.59000)*/
{14, 0,123,__LINE__, 0xbfe18969, 0x782c89a0, 0xbfe28f5c, 0x28f5c28a},			/* -0.54802=f(-0.58000)*/
{14, 0,123,__LINE__, 0xbfe144aa, 0x6a1ac606, 0xbfe23d70, 0xa3d70a38},			/* -0.53963=f(-0.57000)*/
{15, 0,123,__LINE__, 0xbfe0ff7a, 0x33c62e08, 0xbfe1eb85, 0x1eb851e6},			/* -0.53118=f(-0.56000)*/
{15, 0,123,__LINE__, 0xbfe0b9da, 0x9761d676, 0xbfe19999, 0x99999994},			/* -0.52268=f(-0.55000)*/
{15, 0,123,__LINE__, 0xbfe073cd, 0x5ceda0c6, 0xbfe147ae, 0x147ae142},			/* -0.51413=f(-0.54000)*/
{16, 0,123,__LINE__, 0xbfe02d54, 0x3472f658, 0xbfe0f5c2, 0x8f5c28f0},			/* -0.50553=f(-0.53000)*/
{10, 0,123,__LINE__, 0xbfdfcce2, 0x4659bec0, 0xbfe0a3d7, 0x0a3d709e},			/* -0.49688=f(-0.52000)*/
{10, 0,123,__LINE__, 0xbfdf3e4b, 0xbf702bb0, 0xbfe051eb, 0x851eb84c},			/* -0.48817=f(-0.51000)*/
{15, 0,123,__LINE__, 0xbfdeaee8, 0x74000000, 0xbfdfffff, 0xfffffff4},			/* -0.47942=f(-0.50000)*/
{15, 0,123,__LINE__, 0xbfde1ebc, 0x146f6410, 0xbfdf5c28, 0xf5c28f50},			/* -0.47062=f(-0.49000)*/
{14, 0,123,__LINE__, 0xbfdd8dca, 0x3446fd1e, 0xbfdeb851, 0xeb851eac},			/* -0.46177=f(-0.48000)*/
{14, 0,123,__LINE__, 0xbfdcfc16, 0xc1ba02f4, 0xbfde147a, 0xe147ae08},			/* -0.45288=f(-0.47000)*/
{15, 0,123,__LINE__, 0xbfdc69a5, 0x5a913d00, 0xbfdd70a3, 0xd70a3d64},			/* -0.44394=f(-0.46000)*/
{13, 0,123,__LINE__, 0xbfdbd679, 0xa20713e0, 0xbfdccccc, 0xccccccc0},			/* -0.43496=f(-0.45000)*/
{13, 0,123,__LINE__, 0xbfdb4297, 0x983de70c, 0xbfdc28f5, 0xc28f5c1c},			/* -0.42593=f(-0.44000)*/
{15, 0,123,__LINE__, 0xbfdaae02, 0xe739baca, 0xbfdb851e, 0xb851eb78},			/* -0.41687=f(-0.43000)*/
{16, 0,123,__LINE__, 0xbfda18bf, 0x3fee2934, 0xbfdae147, 0xae147ad4},			/* -0.40776=f(-0.42000)*/
{14, 0,123,__LINE__, 0xbfd982d0, 0xaefc9748, 0xbfda3d70, 0xa3d70a30},			/* -0.39860=f(-0.41000)*/
{15, 0,123,__LINE__, 0xbfd8ec3a, 0xee3b0eba, 0xbfd99999, 0x9999998c},			/* -0.38941=f(-0.40000)*/
{16, 0,123,__LINE__, 0xbfd85501, 0xbcf248f0, 0xbfd8f5c2, 0x8f5c28e8},			/* -0.38018=f(-0.39000)*/
{12, 0,123,__LINE__, 0xbfd7bd29, 0x305cc1f8, 0xbfd851eb, 0x851eb844},			/* -0.37092=f(-0.38000)*/
{16, 0,123,__LINE__, 0xbfd724b5, 0x11af1908, 0xbfd7ae14, 0x7ae147a0},			/* -0.36161=f(-0.37000)*/
{15, 0,123,__LINE__, 0xbfd68ba9, 0x4863ef1c, 0xbfd70a3d, 0x70a3d6fc},			/* -0.35227=f(-0.36000)*/
{14, 0,123,__LINE__, 0xbfd5f209, 0x9f07a579, 0xbfd66666, 0x66666658},			/* -0.34289=f(-0.35000)*/
{16, 0,123,__LINE__, 0xbfd557da, 0x420be3a7, 0xbfd5c28f, 0x5c28f5b4},			/* -0.33348=f(-0.34000)*/
{15, 0,123,__LINE__, 0xbfd4bd1f, 0x05aa7438, 0xbfd51eb8, 0x51eb8510},			/* -0.32404=f(-0.33000)*/
{17, 0,123,__LINE__, 0xbfd421db, 0xc1691024, 0xbfd47ae1, 0x47ae146c},			/* -0.31456=f(-0.32000)*/
{17, 0,123,__LINE__, 0xbfd38614, 0xab6112da, 0xbfd3d70a, 0x3d70a3c8},			/* -0.30505=f(-0.31000)*/
{15, 0,123,__LINE__, 0xbfd2e9cd, 0xa20244ca, 0xbfd33333, 0x33333324},			/* -0.29552=f(-0.30000)*/
{16, 0,123,__LINE__, 0xbfd24d0a, 0x87533748, 0xbfd28f5c, 0x28f5c280},			/* -0.28595=f(-0.29000)*/
{17, 0,123,__LINE__, 0xbfd1afcf, 0x9b4da7b2, 0xbfd1eb85, 0x1eb851dc},			/* -0.27635=f(-0.28000)*/
{17, 0,123,__LINE__, 0xbfd11220, 0xc67b04c1, 0xbfd147ae, 0x147ae138},			/* -0.26673=f(-0.27000)*/
{16, 0,123,__LINE__, 0xbfd07401, 0xf2b1ea4c, 0xbfd0a3d7, 0x0a3d7094},			/* -0.25708=f(-0.26000)*/
{17, 0,123,__LINE__, 0xbfcfaaee, 0xd5000000, 0xbfcfffff, 0xffffffe0},			/* -0.24740=f(-0.25000)*/
{16, 0,123,__LINE__, 0xbfce6d0a, 0x1c1c3aea, 0xbfceb851, 0xeb851e98},			/* -0.23770=f(-0.24000)*/
{17, 0,123,__LINE__, 0xbfcd2e5e, 0x1c8fab8c, 0xbfcd70a3, 0xd70a3d50},			/* -0.22797=f(-0.23000)*/
{13, 0,123,__LINE__, 0xbfcbeef2, 0xc0616f0d, 0xbfcc28f5, 0xc28f5c08},			/* -0.21822=f(-0.22000)*/
{17, 0,123,__LINE__, 0xbfcaaed0, 0x54555088, 0xbfcae147, 0xae147ac0},			/* -0.20845=f(-0.21000)*/
{16, 0,123,__LINE__, 0xbfc96dff, 0x295b7fc9, 0xbfc99999, 0x99999978},			/* -0.19866=f(-0.20000)*/
{17, 0,123,__LINE__, 0xbfc82c87, 0x378cfdf0, 0xbfc851eb, 0x851eb830},			/* -0.18885=f(-0.19000)*/
{15, 0,123,__LINE__, 0xbfc6ea70, 0xf8b799c8, 0xbfc70a3d, 0x70a3d6e8},			/* -0.17902=f(-0.18000)*/
{17, 0,123,__LINE__, 0xbfc5a7c4, 0x6be43b5b, 0xbfc5c28f, 0x5c28f5a0},			/* -0.16918=f(-0.17000)*/
{19, 0,123,__LINE__, 0xbfc46489, 0xf3df6afa, 0xbfc47ae1, 0x47ae1458},			/* -0.15931=f(-0.16000)*/
{19, 0,123,__LINE__, 0xbfc320c9, 0xf69a2036, 0xbfc33333, 0x33333310},			/* -0.14943=f(-0.15000)*/
{18, 0,123,__LINE__, 0xbfc1dc8c, 0x7e2f9215, 0xbfc1eb85, 0x1eb851c8},			/* -0.13954=f(-0.14000)*/
{18, 0,123,__LINE__, 0xbfc097d9, 0xf7594c28, 0xbfc0a3d7, 0x0a3d7080},			/* -0.12963=f(-0.13000)*/
{19, 0,123,__LINE__, 0xbfbea575, 0x83c9a1ff, 0xbfbeb851, 0xeb851e71},			/* -0.11971=f(-0.12000)*/
{18, 0,123,__LINE__, 0xbfbc1a6e, 0x4150ae4c, 0xbfbc28f5, 0xc28f5be2},			/* -0.10977=f(-0.11000)*/
{19, 0,123,__LINE__, 0xbfb98eae, 0xd1e3abb3, 0xbfb99999, 0x99999953},			/* -0.09983=f(-0.00100)*/
{20, 0,123,__LINE__, 0xbfb70247, 0xe4c25fad, 0xbfb70a3d, 0x70a3d6c4},			/* -0.08987=f(-0.09000)*/
{20, 0,123,__LINE__, 0xbfb4754a, 0x0d687f19, 0xbfb47ae1, 0x47ae1435},			/* -0.07991=f(-0.08000)*/
{20, 0,123,__LINE__, 0xbfb1e7c6, 0x432c884f, 0xbfb1eb85, 0x1eb851a6},			/* -0.06994=f(-0.07000)*/
{20, 0,123,__LINE__, 0xbfaeb39a, 0x2206e3a0, 0xbfaeb851, 0xeb851e2d},			/* -0.05996=f(-0.06000)*/
{20, 0,123,__LINE__, 0xbfa996de, 0xa9628476, 0xbfa99999, 0x9999990e},			/* -0.04997=f(-0.05000)*/
{16, 0,123,__LINE__, 0xbfa4797b, 0x5d5caf2b, 0xbfa47ae1, 0x47ae13ef},			/* -0.03998=f(-0.04000)*/
{18, 0,123,__LINE__, 0xbf9eb723, 0xe613009a, 0xbf9eb851, 0xeb851da0},			/* -0.02999=f(-0.03000)*/
{ 2, 0,123,__LINE__, 0xbf947a87, 0xc5f7cfa2, 0xbf947ae1, 0x47ae1362},			/* -0.01999=f(-0.02000)*/
{ 2, 0,123,__LINE__, 0xbf847aca, 0xe167df6e, 0xbf847ae1, 0x47ae1249},			/* -0.00999=f(-0.00010)*/
{64, 0,123,__LINE__, 0x3cd19000, 0x00000000, 0x3cd19000, 0x00000000},			/* 9.74915e-16=f(9.74915e-16)*/
{26, 0,123,__LINE__, 0x3f847aca, 0xe167df6e, 0x3f847ae1, 0x47ae16ad},			/* 0.00999=f(0.01000)*/
{25, 0,123,__LINE__, 0x3f947a87, 0xc5f7cfa2, 0x3f947ae1, 0x47ae1594},			/* 0.01999=f(0.02000)*/
{20, 0,123,__LINE__, 0x3f9eb723, 0xe613009a, 0x3f9eb851, 0xeb851fd2},			/* 0.02999=f(0.03000)*/
{22, 0,123,__LINE__, 0x3fa4797b, 0x5d5caf2b, 0x3fa47ae1, 0x47ae1508},			/* 0.03998=f(0.04000)*/
{20, 0,123,__LINE__, 0x3fa996de, 0xa9628476, 0x3fa99999, 0x99999a27},			/* 0.04997=f(0.05000)*/
{20, 0,123,__LINE__, 0x3faeb39a, 0x2206e3a0, 0x3faeb851, 0xeb851f46},			/* 0.05996=f(0.06000)*/
{20, 0,123,__LINE__, 0x3fb1e7c6, 0x432c884f, 0x3fb1eb85, 0x1eb85232},			/* 0.06994=f(0.07000)*/
{20, 0,123,__LINE__, 0x3fb4754a, 0x0d687f19, 0x3fb47ae1, 0x47ae14c1},			/* 0.07991=f(0.08000)*/
{20, 0,123,__LINE__, 0x3fb70247, 0xe4c25fad, 0x3fb70a3d, 0x70a3d750},			/* 0.08987=f(0.09000)*/
{19, 0,123,__LINE__, 0x3fb98eae, 0xd1e3abb3, 0x3fb99999, 0x999999df},			/* 0.09983=f(0.10000)*/
{18, 0,123,__LINE__, 0x3fbc1a6e, 0x4150ae4c, 0x3fbc28f5, 0xc28f5c6e},			/* 0.10977=f(0.11000)*/
{19, 0,123,__LINE__, 0x3fbea575, 0x83c9a1ff, 0x3fbeb851, 0xeb851efd},			/* 0.11971=f(0.12000)*/
{18, 0,123,__LINE__, 0x3fc097d9, 0xf7594c28, 0x3fc0a3d7, 0x0a3d70c6},			/* 0.12963=f(0.13000)*/
{18, 0,123,__LINE__, 0x3fc1dc8c, 0x7e2f9215, 0x3fc1eb85, 0x1eb8520e},			/* 0.13954=f(0.14000)*/
{19, 0,123,__LINE__, 0x3fc320c9, 0xf69a2036, 0x3fc33333, 0x33333356},			/* 0.14943=f(0.15000)*/
{19, 0,123,__LINE__, 0x3fc46489, 0xf3df6afa, 0x3fc47ae1, 0x47ae149e},			/* 0.15931=f(0.16000)*/
{17, 0,123,__LINE__, 0x3fc5a7c4, 0x6be43b5b, 0x3fc5c28f, 0x5c28f5e6},			/* 0.16918=f(0.17000)*/
{15, 0,123,__LINE__, 0x3fc6ea70, 0xf8b799c8, 0x3fc70a3d, 0x70a3d72e},			/* 0.17902=f(0.18000)*/
{17, 0,123,__LINE__, 0x3fc82c87, 0x378cfdf0, 0x3fc851eb, 0x851eb876},			/* 0.18885=f(0.19000)*/
{16, 0,123,__LINE__, 0x3fc96dff, 0x295b7fc9, 0x3fc99999, 0x999999be},			/* 0.19866=f(0.20000)*/
{17, 0,123,__LINE__, 0x3fcaaed0, 0x54555088, 0x3fcae147, 0xae147b06},			/* 0.20845=f(0.21000)*/
{13, 0,123,__LINE__, 0x3fcbeef2, 0xc0616f0d, 0x3fcc28f5, 0xc28f5c4e},			/* 0.21822=f(0.22000)*/
{17, 0,123,__LINE__, 0x3fcd2e5e, 0x1c8fab8c, 0x3fcd70a3, 0xd70a3d96},			/* 0.22797=f(0.23000)*/
{16, 0,123,__LINE__, 0x3fce6d0a, 0x1c1c3aea, 0x3fceb851, 0xeb851ede},			/* 0.23770=f(0.24000)*/
{11, 0,123,__LINE__, 0x3fcfaaee, 0xd5000000, 0x3fd00000, 0x00000013},			/* 0.24740=f(0.25000)*/
{16, 0,123,__LINE__, 0x3fd07401, 0xf2b1ea4c, 0x3fd0a3d7, 0x0a3d70b7},			/* 0.25708=f(0.26000)*/
{17, 0,123,__LINE__, 0x3fd11220, 0xc67b04c1, 0x3fd147ae, 0x147ae15b},			/* 0.26673=f(0.27000)*/
{17, 0,123,__LINE__, 0x3fd1afcf, 0x9b4da7b2, 0x3fd1eb85, 0x1eb851ff},			/* 0.27635=f(0.28000)*/
{16, 0,123,__LINE__, 0x3fd24d0a, 0x87533748, 0x3fd28f5c, 0x28f5c2a3},			/* 0.28595=f(0.29000)*/
{15, 0,123,__LINE__, 0x3fd2e9cd, 0xa20244ca, 0x3fd33333, 0x33333347},			/* 0.29552=f(0.30000)*/
{17, 0,123,__LINE__, 0x3fd38614, 0xab6112da, 0x3fd3d70a, 0x3d70a3eb},			/* 0.30505=f(0.31000)*/
{17, 0,123,__LINE__, 0x3fd421db, 0xc1691024, 0x3fd47ae1, 0x47ae148f},			/* 0.31456=f(0.32000)*/
{15, 0,123,__LINE__, 0x3fd4bd1f, 0x05aa7438, 0x3fd51eb8, 0x51eb8533},			/* 0.32404=f(0.33000)*/
{16, 0,123,__LINE__, 0x3fd557da, 0x420be3a7, 0x3fd5c28f, 0x5c28f5d7},			/* 0.33348=f(0.34000)*/
{14, 0,123,__LINE__, 0x3fd5f209, 0x9f07a579, 0x3fd66666, 0x6666667b},			/* 0.34289=f(0.35000)*/
{15, 0,123,__LINE__, 0x3fd68ba9, 0x4863ef1c, 0x3fd70a3d, 0x70a3d71f},			/* 0.35227=f(0.36000)*/
{16, 0,123,__LINE__, 0x3fd724b5, 0x11af1908, 0x3fd7ae14, 0x7ae147c3},			/* 0.36161=f(0.37000)*/
{12, 0,123,__LINE__, 0x3fd7bd29, 0x305cc1f8, 0x3fd851eb, 0x851eb867},			/* 0.37092=f(0.38000)*/
{16, 0,123,__LINE__, 0x3fd85501, 0xbcf248f0, 0x3fd8f5c2, 0x8f5c290b},			/* 0.38018=f(0.39000)*/
{15, 0,123,__LINE__, 0x3fd8ec3a, 0xee3b0eba, 0x3fd99999, 0x999999af},			/* 0.38941=f(0.40000)*/
{14, 0,123,__LINE__, 0x3fd982d0, 0xaefc9748, 0x3fda3d70, 0xa3d70a53},			/* 0.39860=f(0.41000)*/
{16, 0,123,__LINE__, 0x3fda18bf, 0x3fee2934, 0x3fdae147, 0xae147af7},			/* 0.40776=f(0.42000)*/
{15, 0,123,__LINE__, 0x3fdaae02, 0xe739baca, 0x3fdb851e, 0xb851eb9b},			/* 0.41687=f(0.43000)*/
{13, 0,123,__LINE__, 0x3fdb4297, 0x983de70c, 0x3fdc28f5, 0xc28f5c3f},			/* 0.42593=f(0.44000)*/
{13, 0,123,__LINE__, 0x3fdbd679, 0xa20713e0, 0x3fdccccc, 0xcccccce3},			/* 0.43496=f(0.45000)*/
{15, 0,123,__LINE__, 0x3fdc69a5, 0x5a913d00, 0x3fdd70a3, 0xd70a3d87},			/* 0.44394=f(0.46000)*/
{14, 0,123,__LINE__, 0x3fdcfc16, 0xc1ba02f4, 0x3fde147a, 0xe147ae2b},			/* 0.45288=f(0.47000)*/
{14, 0,123,__LINE__, 0x3fdd8dca, 0x3446fd1e, 0x3fdeb851, 0xeb851ecf},			/* 0.46177=f(0.48000)*/
{15, 0,123,__LINE__, 0x3fde1ebc, 0x146f6410, 0x3fdf5c28, 0xf5c28f73},			/* 0.47062=f(0.49000)*/
{15, 0,123,__LINE__, 0x3fdeaee8, 0x74000000, 0x3fe00000, 0x0000000b},			/* 0.47942=f(0.50000)*/
{10, 0,123,__LINE__, 0x3fdf3e4b, 0xbf702bb0, 0x3fe051eb, 0x851eb85d},			/* 0.48817=f(0.51000)*/
{10, 0,123,__LINE__, 0x3fdfcce2, 0x4659bec0, 0x3fe0a3d7, 0x0a3d70af},			/* 0.49688=f(0.52000)*/
{16, 0,123,__LINE__, 0x3fe02d54, 0x3472f658, 0x3fe0f5c2, 0x8f5c2901},			/* 0.50553=f(0.53000)*/
{15, 0,123,__LINE__, 0x3fe073cd, 0x5ceda0c6, 0x3fe147ae, 0x147ae153},			/* 0.51413=f(0.54000)*/
{15, 0,123,__LINE__, 0x3fe0b9da, 0x9761d676, 0x3fe19999, 0x999999a5},			/* 0.52268=f(0.55000)*/
{15, 0,123,__LINE__, 0x3fe0ff7a, 0x33c62e08, 0x3fe1eb85, 0x1eb851f7},			/* 0.53118=f(0.56000)*/
{14, 0,123,__LINE__, 0x3fe144aa, 0x6a1ac606, 0x3fe23d70, 0xa3d70a49},			/* 0.53963=f(0.57000)*/
{14, 0,123,__LINE__, 0x3fe18969, 0x782c89a0, 0x3fe28f5c, 0x28f5c29b},			/* 0.54802=f(0.58000)*/
{14, 0,123,__LINE__, 0x3fe1cdb5, 0x961530b0, 0x3fe2e147, 0xae147aed},			/* 0.55636=f(0.59000)*/
{15, 0,123,__LINE__, 0x3fe2118d, 0x233ee698, 0x3fe33333, 0x3333333f},			/* 0.56464=f(0.60000)*/
{15, 0,123,__LINE__, 0x3fe254ee, 0x2997adc4, 0x3fe3851e, 0xb851eb91},			/* 0.57286=f(0.61000)*/
{15, 0,123,__LINE__, 0x3fe297d7, 0x0dadda58, 0x3fe3d70a, 0x3d70a3e3},			/* 0.58103=f(0.62000)*/
{13, 0,123,__LINE__, 0x3fe2da46, 0x185db328, 0x3fe428f5, 0xc28f5c35},			/* 0.58914=f(0.63000)*/
{13, 0,123,__LINE__, 0x3fe31c39, 0x978dbc50, 0x3fe47ae1, 0x47ae1487},			/* 0.59719=f(0.64000)*/
{13, 0,123,__LINE__, 0x3fe35daf, 0xd7e1ea68, 0x3fe4cccc, 0xccccccd9},			/* 0.60518=f(0.65000)*/
{13, 0,123,__LINE__, 0x3fe39ea7, 0x461eacec, 0x3fe51eb8, 0x51eb852b},			/* 0.61311=f(0.66000)*/
{13, 0,123,__LINE__, 0x3fe3df1e, 0x0843b82c, 0x3fe570a3, 0xd70a3d7d},			/* 0.62098=f(0.67000)*/
{15, 0,123,__LINE__, 0x3fe41f12, 0x8df369ac, 0x3fe5c28f, 0x5c28f5cf},			/* 0.62879=f(0.68000)*/
{14, 0,123,__LINE__, 0x3fe45e83, 0x390f2640, 0x3fe6147a, 0xe147ae21},			/* 0.63653=f(0.69000)*/
{14, 0,123,__LINE__, 0x3fe49d6e, 0x634f5ccc, 0x3fe66666, 0x66666673},			/* 0.64421=f(0.70000)*/
{14, 0,123,__LINE__, 0x3fe4dbd2, 0x770615a0, 0x3fe6b851, 0xeb851ec5},			/* 0.65183=f(0.71000)*/
{14, 0,123,__LINE__, 0x3fe519ad, 0xf0a07ac0, 0x3fe70a3d, 0x70a3d717},			/* 0.65938=f(0.72000)*/
{14, 0,123,__LINE__, 0x3fe556ff, 0x058df020, 0x3fe75c28, 0xf5c28f69},			/* 0.66686=f(0.73000)*/
{14, 0,123,__LINE__, 0x3fe593c4, 0x4095d910, 0x3fe7ae14, 0x7ae147bb},			/* 0.67428=f(0.74000)*/
{14, 0,123,__LINE__, 0x3fe5cffc, 0x18000000, 0x3fe80000, 0x0000000d},			/* 0.68163=f(0.75000)*/
{12, 0,123,__LINE__, 0x3fe60ba4, 0xf91e6ca0, 0x3fe851eb, 0x851eb85f},			/* 0.68892=f(0.76000)*/
{12, 0,123,__LINE__, 0x3fe646bd, 0x60246e20, 0x3fe8a3d7, 0x0a3d70b1},			/* 0.69613=f(0.77000)*/
{12, 0,123,__LINE__, 0x3fe68143, 0xcf973f00, 0x3fe8f5c2, 0x8f5c2903},			/* 0.70327=f(0.78000)*/
{12, 0,123,__LINE__, 0x3fe6bb36, 0xd0d6ac78, 0x3fe947ae, 0x147ae155},			/* 0.71035=f(0.79000)*/
{12, 0,123,__LINE__, 0x3fe6f494, 0xc5bd2530, 0x3fe99999, 0x999999a7},			/* 0.71735=f(0.80000)*/
{12, 0,123,__LINE__, 0x3fe72d5c, 0x4dc8d870, 0x3fe9eb85, 0x1eb851f9},			/* 0.72428=f(0.81000)*/
{12, 0,123,__LINE__, 0x3fe7658b, 0xee7f0040, 0x3fea3d70, 0xa3d70a4b},			/* 0.73114=f(0.82000)*/
{12, 0,123,__LINE__, 0x3fe79d22, 0x3a3be0b4, 0x3fea8f5c, 0x28f5c29d},			/* 0.73793=f(0.83000)*/
{12, 0,123,__LINE__, 0x3fe7d41d, 0xc35bbdac, 0x3feae147, 0xae147aef},			/* 0.74464=f(0.84000)*/
{14, 0,123,__LINE__, 0x3fe80a7d, 0x3f1d07b8, 0x3feb3333, 0x33333341},			/* 0.75128=f(0.85000)*/
{14, 0,123,__LINE__, 0x3fe8403f, 0x112aef10, 0x3feb851e, 0xb851eb93},			/* 0.75784=f(0.86000)*/
{14, 0,123,__LINE__, 0x3fe87561, 0xf591ade8, 0x3febd70a, 0x3d70a3e5},			/* 0.76432=f(0.87000)*/
{13, 0,123,__LINE__, 0x3fe8a9e4, 0x92e0bad0, 0x3fec28f5, 0xc28f5c37},			/* 0.77073=f(0.88000)*/
{13, 0,123,__LINE__, 0x3fe8ddc5, 0x8f9d4eb0, 0x3fec7ae1, 0x47ae1489},			/* 0.77707=f(0.89000)*/
{13, 0,123,__LINE__, 0x3fe91103, 0x8e759800, 0x3feccccc, 0xccccccdb},			/* 0.78332=f(0.90000)*/
{13, 0,123,__LINE__, 0x3fe9439d, 0x63187a70, 0x3fed1eb8, 0x51eb852d},			/* 0.78950=f(0.91000)*/
{13, 0,123,__LINE__, 0x3fe97591, 0x8a4ef7c0, 0x3fed70a3, 0xd70a3d7f},			/* 0.79560=f(0.92000)*/
{13, 0,123,__LINE__, 0x3fe9a6de, 0xdf4f5690, 0x3fedc28f, 0x5c28f5d1},			/* 0.80161=f(0.93000)*/
{13, 0,123,__LINE__, 0x3fe9d784, 0x1642a610, 0x3fee147a, 0xe147ae23},			/* 0.80755=f(0.94000)*/
{13, 0,123,__LINE__, 0x3fea077f, 0xee1ec208, 0x3fee6666, 0x66666675},			/* 0.81341=f(0.95000)*/
{13, 0,123,__LINE__, 0x3fea36d1, 0x310cb938, 0x3feeb851, 0xeb851ec7},			/* 0.81919=f(0.96000)*/
{13, 0,123,__LINE__, 0x3fea6576, 0xc0808a20, 0x3fef0a3d, 0x70a3d719},			/* 0.82488=f(0.97000)*/
{13, 0,123,__LINE__, 0x3fea936f, 0x3f65d980, 0x3fef5c28, 0xf5c28f6b},			/* 0.83049=f(0.98000)*/
{13, 0,123,__LINE__, 0x3feac0b9, 0x8ee23980, 0x3fefae14, 0x7ae147bd},			/* 0.83602=f(0.99000)*/
{13, 0,123,__LINE__, 0x3feaed54, 0x90000000, 0x3ff00000, 0x00000007},			/* 0.84147=f(1.00000)*/
{11, 0,123,__LINE__, 0x3feb193f, 0x17c982a0, 0x3ff028f5, 0xc28f5c30},			/* 0.84683=f(1.01000)*/
{11, 0,123,__LINE__, 0x3feb4478, 0x06f72b40, 0x3ff051eb, 0x851eb859},			/* 0.85210=f(1.02000)*/
{11, 0,123,__LINE__, 0x3feb6efe, 0x42186dc0, 0x3ff07ae1, 0x47ae1482},			/* 0.85729=f(1.03000)*/
{11, 0,123,__LINE__, 0x3feb98d0, 0xb193c800, 0x3ff0a3d7, 0x0a3d70ab},			/* 0.86240=f(1.04000)*/
{11, 0,123,__LINE__, 0x3febc1ee, 0x52738ea0, 0x3ff0cccc, 0xccccccd4},			/* 0.86742=f(1.05000)*/
{11, 0,123,__LINE__, 0x3febea55, 0xfbd69100, 0x3ff0f5c2, 0x8f5c28fd},			/* 0.87235=f(1.06000)*/
{11, 0,123,__LINE__, 0x3fec1206, 0xde7229c0, 0x3ff11eb8, 0x51eb8526},			/* 0.87720=f(1.07000)*/
{11, 0,123,__LINE__, 0x3fec38ff, 0xa2d0aa60, 0x3ff147ae, 0x147ae14f},			/* 0.88195=f(1.08000)*/
{11, 0,123,__LINE__, 0x3fec5f3f, 0x71376ab0, 0x3ff170a3, 0xd70a3d78},			/* 0.88662=f(1.09000)*/
{11, 0,123,__LINE__, 0x3fec84c5, 0x525ed360, 0x3ff19999, 0x999999a1},			/* 0.89120=f(1.10000)*/
{11, 0,123,__LINE__, 0x3feca990, 0x532842a0, 0x3ff1c28f, 0x5c28f5ca},			/* 0.89569=f(1.11000)*/
{11, 0,123,__LINE__, 0x3feccd9f, 0x72b28700, 0x3ff1eb85, 0x1eb851f3},			/* 0.90010=f(1.12000)*/
{11, 0,123,__LINE__, 0x3fecf0f1, 0xd7ca8350, 0x3ff2147a, 0xe147ae1c},			/* 0.90441=f(1.13000)*/
{11, 0,123,__LINE__, 0x3fed1386, 0x898f05a0, 0x3ff23d70, 0xa3d70a45},			/* 0.90863=f(1.14000)*/
{11, 0,123,__LINE__, 0x3fed355c, 0xae3d9410, 0x3ff26666, 0x6666666e},			/* 0.91276=f(1.15000)*/
{11, 0,123,__LINE__, 0x3fed5673, 0x708e95c0, 0x3ff28f5c, 0x28f5c297},			/* 0.91680=f(1.16000)*/
{11, 0,123,__LINE__, 0x3fed76c9, 0xecfd00f0, 0x3ff2b851, 0xeb851ec0},			/* 0.92075=f(1.17000)*/
{11, 0,123,__LINE__, 0x3fed965f, 0x4d4b79b0, 0x3ff2e147, 0xae147ae9},			/* 0.92460=f(1.18000)*/
{11, 0,123,__LINE__, 0x3fedb532, 0xde4934c0, 0x3ff30a3d, 0x70a3d712},			/* 0.92836=f(1.19000)*/
{ 0, 0,123,__LINE__, 0x3fefffff, 0xff660711, 0xc012d97c, 0x7f3321d2},			/* 1.00000=f(-4.71238)*/
{11, 0,123,__LINE__, 0xbfefffff, 0xf488d2e8, 0xbff921fb, 0x54442d18},			/* -1.00000=f(-1.57079)*/
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x00000000, 0x00000000},			/* 0.00000=f(0.00000)*/
{11, 0,123,__LINE__, 0x3fefffff, 0xf488d2e8, 0x3ff921fb, 0x54442d18},			/* 1.00000=f(1.57079)*/
{ 7, 0,123,__LINE__, 0xbe7777a5, 0xcffffff8, 0x400921fb, 0x54442d18},			/* -8.74228e-08=f(3.14159)*/
{ 0, 0,123,__LINE__, 0xbfefffff, 0xff660711, 0x4012d97c, 0x7f3321d2},			/* -1.00000=f(4.71238)*/
{11, 0,123,__LINE__, 0x3fef9df4, 0xb1e4642c, 0xc03e0000, 0x00000000},			/* 0.98803=f(-30.0000)*/
{ 0, 0,123,__LINE__, 0x3f9a4718, 0xcc369d44, 0xc03c4ccc, 0xcccccccd},			/* 0.02566=f(-28.3000)*/
{ 0, 0,123,__LINE__, 0xbfefd421, 0x492e7c00, 0xc03a9999, 0x9999999a},			/* -0.99464=f(-26.6000)*/
{ 0, 0,123,__LINE__, 0x3fcd85d3, 0xcf182eb3, 0xc038e666, 0x66666667},			/* 0.23064=f(-24.9000)*/
{11, 0,123,__LINE__, 0x3feded3d, 0x8b45e97f, 0xc0373333, 0x33333334},			/* 0.93520=f(-23.2000)*/
{ 0, 0,123,__LINE__, 0xbfde2f57, 0x4d706576, 0xc0358000, 0x00000001},			/* -0.47163=f(-21.5000)*/
{11, 0,123,__LINE__, 0xbfea099b, 0xf0641080, 0xc033cccc, 0xccccccce},			/* -0.81367=f(-19.8000)*/
{11, 0,123,__LINE__, 0x3fe5cd52, 0x799b4b25, 0xc0321999, 0x9999999b},			/* 0.68131=f(-18.1000)*/
{ 0, 0,123,__LINE__, 0x3fe46b5d, 0x881b4460, 0xc0306666, 0x66666668},			/* 0.63810=f(-16.4000)*/
{11, 0,123,__LINE__, 0xbfeb105c, 0x3d517bfe, 0xc02d6666, 0x6666666a},			/* -0.84574=f(-14.7000)*/
{10, 0,123,__LINE__, 0xbfdae402, 0xf37793e0, 0xc02a0000, 0x00000004},			/* -0.42016=f(-13.0000)*/
{11, 0,123,__LINE__, 0x3fee8753, 0x653e5af1, 0xc0269999, 0x9999999e},			/* 0.95401=f(-11.3000)*/
{ 0, 0,123,__LINE__, 0x3fc65058, 0xbd1b5eb4, 0xc0233333, 0x33333338},			/* 0.17432=f(-9.60000)*/
{ 0, 0,123,__LINE__, 0xbfeff753, 0xd002012d, 0xc01f9999, 0x999999a3},			/* -0.99894=f(-7.90000)*/
{ 0, 0,123,__LINE__, 0x3fb5455c, 0x0c1effcb, 0xc018cccc, 0xccccccd6},			/* 0.08308=f(-6.20000)*/
{11, 0,123,__LINE__, 0x3fef47ed, 0x3d6ef59a, 0xc0120000, 0x00000009},			/* 0.97753=f(-4.50000)*/
{10, 0,123,__LINE__, 0xbfd57072, 0x5348b244, 0xc0066666, 0x66666678},			/* -0.33498=f(-2.80000)*/
{11, 0,123,__LINE__, 0xbfec84c5, 0x525ed360, 0xbff19999, 0x999999bd},			/* -0.89120=f(-1.10000)*/
{15, 0,123,__LINE__, 0x3fe2118d, 0x233ee698, 0x3fe33333, 0x333332ec},			/* 0.56464=f(0.60000)*/
{ 0, 0,123,__LINE__, 0x3fe7dcd1, 0x3c5edd01, 0x40026666, 0x66666654},			/* 0.74570=f(2.30000)*/
{ 0, 0,123,__LINE__, 0xbfe837b9, 0xdd343ee8, 0x400fffff, 0xffffffee},			/* -0.75680=f(4.00000)*/
{11, 0,123,__LINE__, 0xbfe19f37, 0x9f04b0ee, 0x4016cccc, 0xccccccc4},			/* -0.55068=f(5.70000)*/
{11, 0,123,__LINE__, 0x3fecc237, 0x682e8000, 0x401d9999, 0x99999991},			/* 0.89870=f(7.40000)*/
{ 0, 0,123,__LINE__, 0x3fd46c1a, 0xb721a161, 0x40223333, 0x3333332f},			/* 0.31909=f(9.10000)*/
{ 0, 0,123,__LINE__, 0xbfef63d4, 0x692d42ca, 0x40259999, 0x99999995},			/* -0.98093=f(10.8000)*/
{ 0, 0,123,__LINE__, 0xbfb0fa7b, 0xb96c5598, 0x4028ffff, 0xfffffffb},			/* -0.06632=f(12.5000)*/
{ 0, 0,123,__LINE__, 0x3fefefd5, 0x9dd228df, 0x402c6666, 0x66666661},			/* 0.99802=f(14.2000)*/
{10, 0,123,__LINE__, 0xbfc86e07, 0xcd3622f3, 0x402fcccc, 0xccccccc7},			/* -0.19085=f(15.9000)*/
{ 0, 0,123,__LINE__, 0xbfee5cef, 0x21612ad9, 0x40319999, 0x99999997},			/* -0.94884=f(17.6000)*/
{ 0, 0,123,__LINE__, 0x3fdbdd01, 0xcfebc4e0, 0x40334ccc, 0xccccccca},			/* 0.43536=f(19.3000)*/
{11, 0,123,__LINE__, 0x3feac5e2, 0xa3fc3b2e, 0x4034ffff, 0xfffffffd},			/* 0.83665=f(21.0000)*/
{11, 0,123,__LINE__, 0xbfe4d4af, 0x48b00aed, 0x4036b333, 0x33333330},			/* -0.65096=f(22.7000)*/
{ 0, 0,123,__LINE__, 0xbfe567b6, 0xfbf31f85, 0x40386666, 0x66666663},			/* -0.66891=f(24.4000)*/
{11, 0,123,__LINE__, 0x3fea58be, 0x10063200, 0x403a1999, 0x99999996},			/* 0.82333=f(26.1000)*/
{ 0, 0,123,__LINE__, 0x3fdd3b58, 0xe939af6e, 0x403bcccc, 0xccccccc9},			/* 0.45674=f(27.8000)*/
{ 0, 0,123,__LINE__, 0xbfee1ced, 0x68d0d25a, 0x403d7fff, 0xfffffffc},			/* -0.94103=f(29.5000)*/
0,};
test_sinf(m)   {run_vector_1(m,sinf_vec,(char *)(sinf),"sinf","ff");   }
/* Copyright (C) 2015 Reservoir Labs, Inc. All rights reserved. */

// PERFECT rights

#ifndef ROCR_MACROS_H
#define ROCR_MACROS_H

// build a macro name as the expansion of the provided terms
#define EXCAT(t, ...) t ## __VA_ARGS__

// force the expansion of an expression
#define EXPAND(...) __VA_ARGS__

// arithmetic increment
#define INC_0 1
#define INC_1 2
#define INC_2 3
#define INC_3 4
#define INC_4 5
#define INC_5 6
#define INC_6 7
#define INC_7 8
#define INC_8 9
#define INC_9 10
#define INC_10 11
#define INC_11 12
#define INC_12 13
#define INC_13 14
#define INC_14 15
#define INC_15 16
#define INC_16 17
#define INC_17 18
#define INC_18 19
#define INC_19 20
#define INC_20 21
#define INC_21 22
#define INC_22 23
#define INC_23 24
#define INC_24 25
#define INC_25 26
#define INC_26 27
#define INC_27 28
#define INC_28 29
#define INC_29 30
#define INC_30 31
#define INC_31 32
#define INC_32 33
#define INC_33 34
#define INC_34 35
#define INC_35 36
#define INC_36 37
#define INC_37 38
#define INC_38 39
#define INC_39 40
#define INC_40 41
#define INC_41 42
#define INC_42 43
#define INC_43 44
#define INC_44 45
#define INC_45 46
#define INC_46 47
#define INC_47 48
#define INC_48 49
#define INC_49 50
#define INC_50 51
#define INC_51 52
#define INC_52 53
#define INC_53 54
#define INC_54 55
#define INC_55 56
#define INC_56 57
#define INC_57 58
#define INC_58 59
#define INC_59 60
#define INC_60 61
#define INC_61 62
#define INC_62 63
#define INC_63 64
#define INC(v) EXCAT(INC_, v)

// utility function to build an expression as the folding of the arguments
// transformed by a macro
//
// For instance
// #define fn(i, arg) (arg+i)
// BUILDEXP(*, 3, fn, a, b ,c) generates (a+0)*(b+1)*(c+2)
//
#define BUILDEXP(sep, cnt, fn, arg, ...) EXCAT(BUILDEXP_, cnt) (sep, 0, fn, arg, __VA_ARGS__)


#define BUILDEXP_0(sep, cnt, fn, ...)
#define BUILDEXP_1(sep, cnt, fn, arg, ...) fn(cnt, arg)
#define BUILDEXP_2(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_1(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_3(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_2(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_4(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_3(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_5(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_4(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_6(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_5(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_7(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_6(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_8(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_7(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_9(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_8(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_10(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_9(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_11(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_10(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_12(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_11(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_13(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_12(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_14(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_13(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_15(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_14(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_16(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_15(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_17(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_16(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_18(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_17(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_19(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_18(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_20(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_19(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_21(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_20(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_22(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_21(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_23(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_22(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_24(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_23(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_25(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_24(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_26(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_25(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_27(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_26(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_28(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_27(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_29(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_28(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_30(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_29(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_31(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_30(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_32(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_31(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_33(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_32(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_34(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_33(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_35(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_34(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_36(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_35(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_37(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_36(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_38(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_37(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_39(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_38(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_40(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_39(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_41(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_40(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_42(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_41(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_43(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_42(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_44(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_43(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_45(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_44(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_46(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_45(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_47(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_46(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_48(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_47(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_49(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_48(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_50(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_49(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_51(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_50(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_52(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_51(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_53(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_52(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_54(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_53(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_55(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_54(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_56(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_55(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_57(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_56(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_58(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_57(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_59(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_58(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_60(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_59(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_61(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_60(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_62(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_61(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_63(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_62(sep, INC(cnt), fn, __VA_ARGS__)
#define BUILDEXP_64(sep, cnt, fn, arg, ...) fn(cnt, arg) sep BUILDEXP_63(sep, INC(cnt), fn, __VA_ARGS__)

#endif


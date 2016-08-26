#ifndef INCLUSION_INTEGER_LOG2_H
#include "integer_log2.h"
#endif

#include <limits.h>

#if ULONG_MAX == 4294967295UL
    //unsigned long is 32 bits
    const unsigned int tab32[32] = {
         0,  9,  1, 10, 13, 21,  2, 29,
        11, 14, 16, 18, 22, 25,  3, 30,
         8, 12, 20, 28, 15, 17, 24,  7,
        19, 27, 23,  6, 26,  5,  4, 31};
#else
    //We are in 64 bits territory, hopefully...
    const unsigned int tab32[32] = {
         0,  9,  1, 10, 13, 21,  2, 29,
        11, 14, 16, 18, 22, 25,  3, 30,
         8, 12, 20, 28, 15, 17, 24,  7,
        19, 27, 23,  6, 26,  5,  4, 31};

    const unsigned long tab64[64] = {
        63,  0, 58,  1, 59, 47, 53,  2,
        60, 39, 48, 27, 54, 33, 42,  3,
        61, 51, 37, 40, 49, 18, 28, 20,
        55, 30, 34, 11, 43, 14, 22,  4,
        62, 57, 46, 52, 38, 26, 32, 41,
        50, 36, 17, 19, 29, 10, 13, 21,
        56, 45, 25, 31, 35, 16,  9, 12,
        44, 24, 15,  8, 23,  7,  6,  5};
#endif // ULONG_MAX

unsigned long ulong_log2(unsigned long value)
{
#   if ULONG_MAX == 4294967295UL
        //unsigned long is 32 bits
        unsigned int x = (unsigned int)in;
        x = uint_log2(x);
        return ((unsigned long) x);
#   else
        //We are in 64 bits territory, hopefully...
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        value |= value >> 32;

        return tab64[((unsigned long)((value - (value >> 1))*0x07EDD5E59A4E28C2)) >> 58];
#   endif
}

unsigned int uint_log2(unsigned int value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return tab32[(unsigned int)(value*0x07C4ACDD) >> 27];
}

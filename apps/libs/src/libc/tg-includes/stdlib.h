#ifndef _STDLIB_H
#define _STDLIB_H

#include <ocr.h>

inline s64 __attribute__((always_inline)) atoi(char *str) {
    s64 retval = 0;
    s64 sign = 1;
    u64 index = 0;

    if(str[index]=='-') {
        sign = -1;
        index++;
    }

    while(str[index] >= '0' && str[index] <= '9') {
        retval = retval*10;
        retval += (str[index]-'0');
        index++;
    }

    return retval * sign;
}

inline double __attribute__((always_inline)) atof(char *str) {
    double retval = 0;
    u8 sign = 0;
    u64 index = 0;

    if(str[index]=='-') {
        sign = 1;
        index++;
    }

    while(str[index] >= '0' && str[index] <= '9') {
        retval *= 10;
        retval += (str[index]-'0');
        index++;
    }

    if(str[index]=='.') {
        ++index;
        double dec = 0.1;
        while(str[index] >= '0' && str[index] <= '9') {
            retval += (str[index]-'0')*dec;
            index++;
        }
    }

    if(str[index]=='e' || str[index]=='E') {
        ++index;
        double exp = str[index] == '-' ? 10 : 0.1;
        int pow = 0;
        while(str[index] >= '0' && str[index] <= '9') {
            pow *= 10;
            pow += (str[index]-'0');
            index++;
        }
        while(pow) {
            --pow;
            retval *= exp;
        }
    }

    return sign ? -retval : retval;
}

#endif //_STDLIB_H

#ifndef INCLUSION_INTEGER_LOG2_H
#define INCLUSION_INTEGER_LOG2_H
//2016Aug11:  Taken from
//              http://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
//            Key features: branch-free, CPU-abstract DeBruijn-like algorithm
//2016Aug11: Tested to be correct for all values in value=[1,UINT_MAX]
unsigned long ulong_log2(unsigned long value);
unsigned int uint_log2(unsigned int value);
#endif // INCLUSION_INTEGER_LOG2_H

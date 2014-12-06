#ifndef _STRINGS_H
#define _STRINGS_H

inline void __attribute__((always_inline)) bzero(void *s, size_t n) {
    ASSERT(s!=NULL);
    char* ptr = (char*)s;
    while(n>0) ptr[--n] = 0;
}

#endif //_STRINGS_H

#ifndef _STRING_H
#define _STRING_H

#include <ocr.h>
#include <misc.h>

/* TODO: Should probably be blocked and use hal_memCopy for optimization (see bug #140) */
inline char* __attribute__((always_inline)) strcpy(char *dest, const char *src) {
    ASSERT(dest!=NULL && src!=NULL);
    while(*src != '\0') {
      *dest=*src;
      ++dest; ++src;
    }
    *dest=*src;
    return dest;
}

/* TODO: Should probably be blocked and use hal_memCopy for optimization (see bug #140) */
inline char* __attribute__((always_inline)) strncpy(char *dest, const char *src, size_t n) {
    ASSERT(dest!=NULL && src!=NULL);
    while(*src != '\0' && --n>0) {
      *dest=*src;
      ++dest; ++src;
    }
    *dest=*src;
    return dest;
}

inline size_t __attribute__((always_inline)) strlen(const char *s) {
    if(s==NULL) return 0;
    u64 n;
    for(n=0; *s!='\0'; ++s,++n);
    return n;
}


inline void* __attribute__((always_inline)) memcpy(void *dest, const void *src, size_t n) {
    ASSERT(dest!=NULL && src!=NULL);

    hal_memCopy(dest, src, n, false);
    return dest;
}

inline int __attribute__((always_inline)) strcmp(const char *s1, const char *s2) {
    ASSERT(s1!=NULL && s2!=NULL);
    while(*s1 != '\0' && *s2 != '\0' && *s1 == *s2) {
      ++s1; ++s2;
    }
    return *s1-*s2;
}

inline int __attribute__((always_inline)) strcasecmp(const char *s1, const char *s2) {
    ASSERT(s1!=NULL && s2!=NULL);
    while(*s1 != '\0' && *s2 != '\0' &&
          (*s1 == *s2 || (*s1>*s2 ? *s1>='a' && *s1<='z' && *s1-*s2==32 :
                          *s2>='a' && *s2<='z' && *s2-*s1==32))) {
      ++s1; ++s2;
    }
    char a1 = *s1>='A' && *s1<='Z' ? *s1+32 : *s1;
    char a2 = *s2>='A' && *s2<='Z' ? *s2+32 : *s2;
    return a1-a2;
}

#endif //_STRING_H

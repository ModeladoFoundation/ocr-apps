#ifndef _STRING_H
#define _STRING_H

#include <ocr.h>

inline int __attribute__((always_inline)) strcmp(const char *s1, const char *s2)
{
  ASSERT(s1!=NULL && s2!=NULL);
  while(*s1 != '\0' && *s2 != '\0' && *s1 == *s2) {
    ++s1; ++s2;
  }
  return *s1-*s2;
}

#endif //_STRING_H

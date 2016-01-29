#ifdef TG_ARCH
#include "../tg/stdio.h"
#include "../tg/stdlib.h"
// #include "../tg/stdint.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#endif
#ifdef OCR
#include <crlibm.h>
#else
#include <math.h>
#define pow_rn(arg1,arg2) pow((arg1),(arg2))
#define log_rn(arg1) log(arg1)
#define sin_rn(arg1) sin(arg1)
#define cos_rn(arg1) cos(arg1)
#define tan_rn(arg1) tan(arg1)
#define atan_rn(arg1) atan(arg1)
#endif

#ifdef OCR
#include "ocr.h"
#endif

#ifdef OCR
ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
#else
int main (int argc, char *argv[])
#endif
{

#ifdef OCR
#ifdef TG_ARCH
  PRINTF("test's mainEDT enter\n");
#else
  printf("test's mainEDT enter\n");
#endif
#else
  printf("test's main enter\n");
#endif
  double arg1   = 1.0/3.0;
  double arg2   = 1.0/5.0;
  double result = 1.0/7.0;

#if 1
  result = pow_rn(arg1,arg2);
#ifdef TG_ARCH
  PRINTF("pow_rn(0x%16.16llx,%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg1,*(unsigned long long *)&arg2,*(unsigned long long *)&result);
#else
  printf("pow_rn(0x%16.16llx,%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg1,*(unsigned long long *)&arg2,*(unsigned long long *)&result);
  printf("pow_rn(%25.16e,%25.16e)=%25.16e\n",arg1,arg2,result);
#endif
#endif

#if 1
  result = log_rn(arg1);
#ifdef TG_ARCH
  PRINTF("log_rn(0x%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg1,*(unsigned long long *)&result);
#else
  printf("log_rn(0x%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg1,*(unsigned long long *)&result);
  printf("log_rn(%25.16e)=%25.16e\n",arg1,result);
#endif
#endif

#if 1
  result = sin_rn(arg1);
#ifdef TG_ARCH
  PRINTF("sin_rn(0x%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg1,*(unsigned long long *)&result);
#else
  printf("sin_rn(0x%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg1,*(unsigned long long *)&result);
  printf("sin_rn(%25.16e)=%25.16e\n",arg1,result);
#endif
#endif

#if 1
  result = cos_rn(arg1);
#ifdef TG_ARCH
  PRINTF("cos_rn(0x%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg1,*(unsigned long long *)&result);
#else
  printf("cos_rn(0x%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg1,*(unsigned long long *)&result);
  printf("cos_rn(%25.16e)=%25.16e\n",arg1,result);
#endif
#endif

#if 1
  result = tan_rn(arg1);
#ifdef TG_ARCH
  PRINTF("tan_rn(0x%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg1,*(unsigned long long *)&result);
#else
  printf("tan_rn(0x%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg1,*(unsigned long long *)&result);
  printf("tan_rn(%25.16e)=%25.16e\n",arg1,result);
#endif
#endif

#if 1
  result = atan_rn(arg1);
#ifdef TG_ARCH
  PRINTF("atan_rn(0x%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg1,*(unsigned long long *)&result);
#else
  printf("atan_rn(0x%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg1,*(unsigned long long *)&result);
  printf("atan_rn(%25.16e)=%25.16e\n",arg1,result);
#endif
#endif

#if 1

  arg2   = pow_rn(2.0,120.0);
  result = sin_rn(arg2);
#ifdef TG_ARCH
  PRINTF("sin_rn(0x%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg2,*(unsigned long long *)&result);
#else
  printf("sin_rn(0x%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg2,*(unsigned long long *)&result);
  printf("sin_rn(%25.16e)=%25.16e\n",arg2,result);
#endif
  arg1 = result;

  arg2   = pow_rn(2.0,120.0);
  result = cos_rn(arg2);
#ifdef TG_ARCH
  PRINTF("cos_rn(0x%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg2,*(unsigned long long *)&result);
#else
  printf("cos_rn(0x%16.16llx)=0x%16.16llx\n",*(unsigned long long *)&arg2,*(unsigned long long *)&result);
  printf("cos_rn(%25.16e)=%25.16e\n",arg2,result);
#endif
  arg2 = result;

  result = pow_rn(arg1,2.0) + pow_rn(arg2,2.0);
#ifdef TG_ARCH
  PRINTF("sin_rn^2+cos_rn^2=0x%16.16llx\n",*(unsigned long long *)&result);
#else
  printf("sin_rn^2+cos_rn^2=0x%16.16llx\n",*(unsigned long long *)&result);
  printf("sin_rn^2+cos_rn^2=%25.16e\n",result);
#endif

#endif

  printf("test's mainEDT return\n");
  exit(0);
#ifdef OCR
  return NULL_GUID;
#endif
}

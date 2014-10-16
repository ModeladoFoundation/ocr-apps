#include <math.h>
#include <ocr.h>

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  PRINTF("Test floor\n");
  double x;
  x=1;         VERIFY(x==floor(x), "floor integral test\n");
  x=0;         VERIFY(x==floor(x), "floor zero test\n");
  x=-x;        VERIFY(x==floor(x), "floor -zero test\n");
  NaN(x);      VERIFY(isNaN(floor(x)), "floor nan test\n");
  pINF(x);     VERIFY(x==floor(x), "floor inf test\n");
  nINF(x);     VERIFY(x==floor(x), "floor -inf test\n");
  x=1e20+0.1;  VERIFY(1e20==floor(x), "floor pos test\n");
  x=-1e20+0.1; VERIFY(-1e20==floor(x), "floor neg test\n");
  x=1+0.1; VERIFY(1==floor(x), "floor 1.1 test\n");
  x=1-1.1; VERIFY(-1==floor(x), "floor -0.1 test\n");

  PRINTF("Test sqrt\n");
  x=1;         VERIFY(x==sqrt(x), "sqrt one test\n");
  x=4;         VERIFY(2==sqrt(x), "sqrt four test\n");
  x=0;         VERIFY(x==sqrt(x), "sqrt zero test\n");
  x=-x;        VERIFY(x==sqrt(x), "sqrt -zero test\n");
  NaN(x);      VERIFY(isNaN(sqrt(x)), "sqrt nan test\n");
  pINF(x);     VERIFY(x==sqrt(x), "sqrt inf test\n");
  nINF(x);     VERIFY(isNaN(sqrt(x)), "sqrt -inf test\n");
  x=-0.1;      VERIFY(isNaN(sqrt(x)), "sqrt neg test\n");

  PRINTF("Test log\n");
  x=1;         VERIFY(0==log(x), "log one test\n");
  x=4;         VERIFY(1.3862<=log(x)&&1.3863>=log(x), "log four test\n");
  NaN(x);      VERIFY(isNaN(log(x)), "log nan test\n");
  pINF(x);     VERIFY(x==log(x), "log inf test\n");
  nINF(x);     VERIFY(isNaN(log(x)), "log -inf test\n");
  x=0;         VERIFY(isNaN(log(x)), "log zero test\n");
  x=-x;        VERIFY(isNaN(log(x)), "log -zero test\n");

  double y;
  PRINTF("Test pow\n");
  x=2;y=2;     VERIFY(4==pow(x,y), "pow 2^2 test\n");
  x=3;y=3;     VERIFY(27==pow(x,y), "pow 3^3 test\n");
  x=5;y=5.5;   VERIFY(6987.67<=pow(x,y)&&6987.73>=pow(x,y), "pow 5^5.5 test\n");

  PRINTF("Done\n");
  ocrShutdown();
  return NULL_GUID;
}

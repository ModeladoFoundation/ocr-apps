#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, char **argv) {
  uint64_t t_u64,d_u64,e_u64;
  double   t_dbl,d_dbl,e_dbl;
  int ret_val = -1;

// N-1 lines

  for(;;) {
    ret_val = scanf ("time = %lx, dt=%lx, e(0)=%lx\n", &t_u64,&d_u64,&e_u64);
//  printf("time = %16.16lx, dt=%16.16lx, e(0)=%16.16lx\n",  t_u64, d_u64, e_u64);fflush(stdout);

    if( ret_val != 3 ) break;

    t_dbl = *(double *)&t_u64;
    d_dbl = *(double *)&d_u64;
    e_dbl = *(double *)&e_u64;

    printf("time = %e, dt=%e, e(0)=%e\n", t_dbl,d_dbl,e_dbl);fflush(stdout);
  } // for(;;)

// Nth, i.e. last line

  ret_val = scanf("   Final Origin Energy = %16.16lx \n", &e_u64);
//printf("   Final Origin Energy = %16.16lx \n", e_u64);fflush(stdout);

  if( ret_val == EOF ) exit(1);

  e_dbl = *(double *)&e_u64;
  printf("   Final Origin Energy = %12.6e \n", e_dbl);fflush(stdout);

  if ( feof(stdin) == 0 ) exit(1);
  else exit(0);
} // main()

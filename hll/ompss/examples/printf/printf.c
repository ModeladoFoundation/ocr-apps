
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ompss_user_main( int argc, char* argv[] )
{
  char* text = malloc( 50 * sizeof(char) );
  #pragma omp task out( text[0;50] )
  {
    const char* src = "This is an OmpSs task";
    int src_size = strlen(src) + 1;
    strncpy( text, src, src_size );
  }

  #pragma omp task in( text[0;50] )
  PRINTF("Hello, world! %s\n", text);

  #pragma omp taskwait
  free( text );

  return 0;
}


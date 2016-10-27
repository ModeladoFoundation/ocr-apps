
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _OMPSS
#define PRINTF printf
#define ompss_user_main main
#endif

char text[50];

static inline int min( int a, int b )
{
    return a < b? a: b;
}

#pragma omp task out( [size]text )
void produce( char* text, int size )
{
   const char* src = "This is an OmpSs task";
   int src_size = strlen(src) + 1;
   strncpy( text, src, min(size,src_size) );
}

#pragma omp task in( [size]additional_text )
void say_hello( const char* additional_text, int size )
{
   PRINTF("Hello, world! %s\n", additional_text);
}

void print_args( int argc, char* argv[] )
{
   for( int arg = 0; arg < argc; ++arg )
      PRINTF("%s\n",argv[arg]);
}

int ompss_user_main( int argc, char* argv[] )
{
   PRINTF("Starting ompss_user_main. Arguments passed are:\n");
   print_args(argc, argv);

   char* text = malloc( 50 * sizeof(char) );
   produce( text, 50 );

   say_hello(text, 50 );

   #pragma omp taskwait
   free( text );

   return 0;
}



#include <stdio.h>

#pragma omp task
void say_hello( const char* additional_text )
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

   const char* text = "This is an OmpSs task";
   say_hello(text);

   #pragma omp taskwait

   return 0;
}


/// \file
/// A parser for command line arguments.
///
/// A general purpose command line parser that uses getopt_long() to parse
/// the command line.
///
/// \author Sriram Swaminarayan
/// \date July 24, 2007

#include "cmdLineParser.h"

#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include "mytype.h"

#define nextOption(o) ((MyOption*) o->next)

static void dupString(char* d, const char* s)
{
   if ( ! s ) s = "";
   strcpy(d, s);
}

static void myOptionAlloc( MyOption* o,
   const char* longOption, const char shortOption,
   int has_arg, const char type, void* dataPtr, int dataSize, const char* help)
{
   static int iBase=129;
   #ifndef TG_ARCH
      memset(o, 0, sizeof(MyOption));
   #else
     u32 m;
     for(m=0; m<sizeof(MyOption); ++m) ((char*)o)[m]='\0';
   #endif

   dupString(o->help,help);
   dupString(o->longArg,longOption);
   if(shortOption) o->shortArg[0] = (unsigned char)shortOption;
   else
   {
      o->shortArg[0] = iBase;
      iBase++;
   }
   o->argFlag = has_arg;
   o->type = type;
   o->ptr = dataPtr;
   o->sz = dataSize;
}

static MyOption* lastOption(MyOption* o)
{
   if ( ! o) return o;
   while(nextOption(o)) o = nextOption(o);
   return o;
}

static MyOption* findOption(MyOption* o, unsigned char shortArg)
{
   while(o)
   {
      if (o->shortArg[0] == shortArg) return o;
      o = nextOption(o);
   }
   return o;
}

int addArg(MyOption* o, MyOption** myargs, const char* longOption, const char shortOption,
           int has_arg, const char type, void* dataPtr, int dataSize,
           const char* help)
{
   MyOption* p;
   myOptionAlloc(o, longOption,shortOption,has_arg,type,dataPtr,dataSize, help);
   if ( ! o ) return 1;
   if ( ! *myargs) *myargs = o;
   else
   {
      p = lastOption(*myargs);
      p->next = (void *)o;
   }
   return 0;
}

void printArgs(MyOption* myargs)
{
   MyOption* o = myargs;
   char s[4096];
   unsigned char *shortArg;
   PRINTF("\n"
      "  Arguments are: \n");
   while(o)
   {
      if(o->shortArg[0]<0xFF) shortArg = o->shortArg;
      else shortArg = (unsigned char *) "---";
      PRINTF("  --\%\%-%20s",o->longArg);
      PRINTF(" -%c  arg=%1d type=%c  %s\n",shortArg[0],o->argFlag,o->type,o->help);
      o = nextOption(o);

   }
   PRINTF("\n\n");
   return;
}

void processArgs(MyOption* myargs, int argc, char** argv)
{
   MyOption* o;
   int n=0;
   int i;
   struct option* opts;
   char* sArgs;
   int c;

   if ( ! myargs) return;
   o = myargs;
   while(o)
   {n++,o=nextOption(o);}

   o = myargs;

   ocrDBK_t DBK_sArgs, DBK_opts;
   ocrDbCreate( &DBK_sArgs, (void**) &sArgs, sizeof(char)*2*(n+2), 0, NULL_HINT, NO_ALLOC );
   ocrDbCreate( &DBK_opts, (void**) &opts, sizeof(struct option)*(n+2), 0, NULL_HINT, NO_ALLOC );

   for (i=0; i<n; i++)
   {
      opts[i].name = o->longArg;
      opts[i].has_arg = o->argFlag;
      opts[i].flag    = 0;
      opts[i].val     = o->shortArg[0];

      strcat(sArgs,(char*) o->shortArg);
      if(o->argFlag) strcat(sArgs,":");
      o = nextOption(o);
   }

   while(1)
   {

      int option_index = 0;

      c = getopt_long (argc, argv, sArgs, opts, &option_index);
      if ( c == -1) break;
      o = findOption(myargs,c);
      if ( ! o )
      {
         PRINTF("\n\n"
            "    invalid switch : -%c in getopt()\n"
            "\n\n",
            c);
         break;
      }
      if(! o->argFlag)
      {
         int* i = (int*)o->ptr;
         *i = 1;
      }
      else
      {
         switch(o->type)
         {
            case 'i':
               sscanf(optarg,"%d",(int*)o->ptr);
               break;
            case 'f':
               sscanf(optarg,"%f",(float*)o->ptr);
               break;
            case 'd':
               sscanf(optarg,"%lf",(double*)o->ptr);
               break;
            case 's':
               strncpy((char*)o->ptr,(char*)optarg,o->sz);
               ((char*)o->ptr)[o->sz-1] = '\0';
               break;
            case 'c':
               sscanf(optarg,"%c",(char*)o->ptr);
               break;
            default:
               PRINTF("\n\n"
                  "    invalid type : %c in getopt()\n"
                  "    valid values are 'e', 'z'. 'i','d','f','s', and 'c'\n"
                  "\n\n",
                  c);
         }
      }
   }

   ocrDbDestroy(DBK_sArgs);
   ocrDbDestroy(DBK_opts);

   return;
}

#ifndef INCLUSION_TAILRECURSION_H
#include "tailrecursion.h"
#endif

Err_t initializeIterate(Iterate_t * io_iterate, ocrGuid_t in_whereToGoWhenFalse)
{
    Err_t err=0;
    while(!err){
        if( ! io_iterate) {err=__LINE__; IFEB;}

        io_iterate->whereToGoWhenFalse = in_whereToGoWhenFalse;

        io_iterate->increment = 1;
        io_iterate->begin = 0;
        io_iterate->end = 5;
        //current will be incremented before comparison.  See condition() below.
        io_iterate->current = io_iterate->begin - io_iterate->increment;

        break;  //  while not erri:
    }
    return err;
}
int condition(Iterate_t * io_iterate) //Return 1 upon success; zero otherwise.
{
    io_iterate->current += io_iterate->increment;

    PRINTF("TESTIO> CONDITION: Checking iteration %ld in [%ld, %ld[ by step of %ld",
           io_iterate->current,
           io_iterate->begin,
           io_iterate->end,
           io_iterate->increment
           );

    if(io_iterate->current < io_iterate->end){
        PRINTF(" : True\n");
        return 1; //Process current iteration
    }else{
        PRINTF(" : False\n");
        return 0; //All done.
    }
}
Err_t trueClause(const Iterate_t * in_iterate)
{
    Err_t err=0;
    while(!err){
        if( ! in_iterate) {err=__LINE__; IFEB;}

        PRINTF("TESTIO> Performing iteration %ld in [%ld, %ld[\n",
               in_iterate->current,
               in_iterate->begin,
               in_iterate->end
               );

        break;  //  while not erri:
    }
    return err;
}
Err_t falseClause(const Iterate_t * in_iterate)
{
    Err_t err=0;
    while(!err){
        if( ! in_iterate) {err=__LINE__; IFEB;}

        PRINTF("TESTIO> Iterations reached the end.\n");

        break;  //  while not erri:
    }
    return err;
}

Err_t validateWork(int * in_work)
{
    Err_t err=0;
    while(!err){
        if( !in_work ) {err=__LINE__; break;}
        long sum = 0;
        u64 i;
        for(i=1; i<= in_work[0]; ++i){
            sum += in_work[i];
        }
        if( sum != in_work[0] ) {err=__LINE__; break;}
        break;  //  while not erri:
    }
    return err;
}

Err_t concludeIteration(int * in_work)
{
    Err_t err=0;
    while(!err){
        if(!in_work){
            PRINTF("TESTIO> Concluding the iterations.\n");
        }else{
            err = validateWork(in_work); IFEB;
            PRINTF("TESTIO> Concluding: Work is ok.\n");
        }
        break;  //  while not erri:
    }
    return err;
}

Err_t initializeWork(u64 in_workLength, int * io_work)
{
    Err_t err=0;
    while(!err){
        if( in_workLength == 0 || !io_work) {err=__LINE__; break;}

        io_work[0] = in_workLength - 1; //offsets are in [1, io_work[0] ]
        u64 i;
        for(i=1; i<= io_work[0]; ++i){
            io_work[i] = 1;
        }

        break;  //  while not erri:
    }
    return err;
}

Err_t trueClauseWithWork(const Iterate_t * in_iterate, int * in_work)
{
    Err_t err=0;
    while(!err){
        if( !in_iterate ) {err=__LINE__; break;}
        if( !in_work ) {err=__LINE__; break;}

        if(in_iterate->begin > in_iterate->current){
            err=__LINE__; break;
        }

        if(in_iterate->current >= in_iterate->end ){
            err=__LINE__; break;
        }

        PRINTF("TESTIO> IF-THEN: Performing iteration %ld in [%ld, %ld[",
               in_iterate->current,
               in_iterate->begin,
               in_iterate->end
               );

        err = validateWork(in_work); IFEB;
        PRINTF(" : Work is ok.\n");
        break;  //  while not erri:
    }
    return err;
}

Err_t falseClauseWithWork(const Iterate_t * in_iterate, int * in_work)
{
    Err_t err=0;
    while(!err){
        if( !in_iterate ) {err=__LINE__; break;}
        if( !in_work ) {err=__LINE__; break;}
        if(in_iterate->current != in_iterate->end ){
            err=__LINE__; break;
        }

        err = validateWork(in_work); IFEB;
        PRINTF("TESTIO> ELSE: Work is ok.\n");
        break;
    }
    return err;
}

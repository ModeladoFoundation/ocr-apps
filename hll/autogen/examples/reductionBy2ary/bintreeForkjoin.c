#ifndef INCLUSION_BINTREE_FORKJOIN_H
#include "bintreeForkjoin.h"
#endif

#include <stdio.h> //sprintf

#include "integer_log2.h"

#define XMEMSET(SRC, CHARC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)SRC+xmIT)=CHARC;}
#define XMEMCPY(DEST, SRC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)DEST+xmIT)=*((char*)SRC+xmIT);}

void get_children_indices(BTindex_t in_parent, BTindex_t * o_child1, BTindex_t * o_child2)
{
    *o_child1 = 2*in_parent + 1;
    *o_child2 = 2*in_parent + 2;
}
BTindex_t get_parent_index(BTindex_t in_index)
{
    if(in_index == 0) return NOTA_BTindex;
    return ((in_index-1)/2);
}
unsigned long get_level(BTindex_t in_index)
{
    //+1 as the first index starts at zero.
    //So the first level will have id  = zero.
    return ulong_log2(in_index+1);
}

Value_t add(Value_t one, Value_t two)
{
    return (one + two);
}
void add_zero(Value_t * io)
{
    *io = 0;
}

void print_iterate(int in_edtType, ocrGuid_t in_thisEDT, TFJiterate_t * it, const char * in_text)
{
    const char * defaultt= "";

    const char * t;
    if(!in_text || in_text[0]=='\0'){
        t = defaultt;
    } else {
        t = in_text;
    }

    if(ocrGuidIsEq(in_thisEDT, NULL_GUID)){
        PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" ITERATE dex=%5lu lohi=%5u,%5u  %s\n",
                in_edtType,
                GUIDA(in_thisEDT),
                it->btindex,
                it->low, it->hi,
                t
               );
    }else {
        PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" ITERATE dex=%5lu lohi=%5u,%5u  F=%p  Togo="GUIDF" %s\n",
                in_edtType,
                GUIDA(in_thisEDT),
                it->btindex,
                it->low, it->hi,
                (void*)it->funcp,
                GUIDA(it->whereToGoWhenFalse),
                t
               );
    }
}
void print_work(int in_edtType, ocrGuid_t in_thisEDT, Work_t w, const char * in_text)
{
    const char * defaultt= "";

    const char * t;
    if(!in_text || in_text[0]=='\0'){
        t = defaultt;
    } else {
        t = in_text;
    }

    PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" WORK dex=%lu res=%lu %s\n",
            in_edtType,
            GUIDA(in_thisEDT),
            w.btindex,
            (unsigned long) w.result,
            t
           );
}

void copy_iterate(TFJiterate_t * in, TFJiterate_t * out)
{
    XMEMCPY(out, in, sizeof(TFJiterate_t));
}

void copy_work(Work_t w, Work_t * out)
{
    XMEMCPY(out, &w, sizeof(Work_t));
}

Err_t setupBtForkJoin(int in_edtType, ocrGuid_t in_thisEDT,
                      TFJiterate_t * io_iterate,
                      ocrGuid_t in_whereToGoWhenFalse,
                      testFunctPtr_t in_funcp,
                      testFunctPtr_zero_t in_func_zerop,
                      GlobalData_t * in_globals,
                      unsigned long * o_refvalue
                      )
{
    Err_t err=0;
    while(!err){
        if( ! io_iterate) {err=__LINE__; IFEB;}

        GUID_ASSIGN_VALUE(io_iterate->whereToGoWhenFalse, in_whereToGoWhenFalse);

        io_iterate->btindex = 0;
        //I've tested the following lo-hi setups: 1-1, 1-2, 1-3, 1-4, 1-5
        io_iterate->low =   1;
        io_iterate->hi  = in_globals->overall_mpi_count;
        io_iterate->funcp = in_funcp;
        io_iterate->fzerop = in_func_zerop;

        print_iterate(in_edtType, in_thisEDT, io_iterate, "setupBtForkJoin");

        in_func_zerop(o_refvalue);

        unsigned int i;
        for(i=io_iterate->low; i<=io_iterate->hi; ++i){
            Value_t x = i;
            *o_refvalue = in_funcp(x, *o_refvalue);
        }

        int diff = io_iterate->hi - io_iterate->low;
        if(diff <0){
            err = __LINE__; IFEB;
        }

        break;  //  while not err
    }
    return err;
}

int conditionBtFork(int in_edtType, ocrGuid_t in_thisEDT, TFJiterate_t * io_iterate) //Return 1 upon success; zero otherwise.
{
    int x = 1;
    if(io_iterate->low >= io_iterate->hi) x = 0;

    if(x){
        print_iterate(in_edtType, in_thisEDT, io_iterate, "CONDITION True");
    } else {
        print_iterate(in_edtType, in_thisEDT, io_iterate, "CONDITION False");
    }
    return x;
}

Err_t btForkThen(int in_edtType, ocrGuid_t in_thisEDT,
                 TFJiterate_t * in_iterate,
                 ocrGuid_t in_whenDone,
                 TFJiterate_t * o_iterate,
                 ocrGuid_t * o_gDone)
{
    Err_t err=0;
    while(!err){
        if( ! in_iterate) {err=__LINE__; IFEB;}

        print_iterate(in_edtType, in_thisEDT, in_iterate, "btForkThen");

        if(in_iterate->low == in_iterate->hi){
            //This cannot be split any further
            err = __LINE__; IFEB;
        }

        copy_iterate(in_iterate, o_iterate);
        GUID_ASSIGN_VALUE(*o_gDone, in_iterate->whereToGoWhenFalse);

        GUID_ASSIGN_VALUE(o_iterate->whereToGoWhenFalse, in_whenDone);

        print_iterate(in_edtType, in_thisEDT, o_iterate, "btForkThen new iterate");

        break;  //  while not err
    }
    return err;
}

Err_t btForkFOR(int in_edtType, ocrGuid_t in_thisEDT, int in_foliationIndex, TFJiterate_t * in_iterate, TFJiterate_t * o_iterate)
{
    Err_t err=0;
    while(!err){
        if( ! in_iterate) {err=__LINE__; IFEB;}

        char text[256];
        sprintf(text, "btForkFOR initial (i=%d)", in_foliationIndex);
        print_iterate(in_edtType, in_thisEDT, in_iterate, text);

        if(in_iterate->low == in_iterate->hi){
            //This cannot be split any further
            err = __LINE__; IFEB;
        }

        copy_iterate(in_iterate, o_iterate);

        unsigned long mid = in_iterate->low;
        mid += in_iterate->hi;
        mid/=2;

        BTindex_t mine, other;
        if(in_foliationIndex == 0){
            //This is the left side
            o_iterate->hi = mid;
            get_children_indices(in_iterate->btindex, &mine, &other);
            o_iterate->btindex = mine;
        } else if(in_foliationIndex == 1){
            //This is the right side
            o_iterate->low = mid +1;
            get_children_indices(in_iterate->btindex, &other, &mine);
            o_iterate->btindex = mine;
        } else {
            err = __LINE__; IFEB; //This code will need to be revised if used with in_foliationIndex != 2
        }

        GUID_ASSIGN_VALUE(o_iterate->whereToGoWhenFalse, in_iterate->whereToGoWhenFalse);

        sprintf(text, "btForkFOR final (i=%d)", in_foliationIndex);
        print_iterate(in_edtType, in_thisEDT, o_iterate, text);

        break;  //  while not err
    }
    return err;
}

Err_t btForkElse(int in_edtType, ocrGuid_t in_thisEDT,
                 TFJiterate_t * in_iterate, ocrGuid_t in_whenDone,
                 TFJiterate_t * o_iterate, ocrGuid_t * o_gDone)
{
    Err_t err=0;
    while(!err){
        if( ! in_iterate) {err=__LINE__; IFEB;}

        print_iterate(in_edtType, in_thisEDT, in_iterate, "btForkElse initial");

        if(in_iterate->low != in_iterate->hi){
            err = __LINE__; IFEB;
        }

        copy_iterate(in_iterate, o_iterate);
        GUID_ASSIGN_VALUE(*o_gDone, in_iterate->whereToGoWhenFalse);

        GUID_ASSIGN_VALUE(o_iterate->whereToGoWhenFalse, in_whenDone);

        print_iterate(in_edtType, in_thisEDT, o_iterate, "btForkElse new iterate");

        break;  //  while not err
    }
    return err;
}

Err_t transitionBTFork(int in_edtType, ocrGuid_t in_thisEDT, TFJiterate_t * in_iterate, Work_t * o_work, Work_t * o_work2)
{
    Err_t err=0;
    while(!err){
        if( ! in_iterate) {err=__LINE__; IFEB;}
        if( in_iterate->low != in_iterate->hi) {err=__LINE__; IFEB;}

        Value_t zero;
        in_iterate->fzerop(&zero);

        o_work2->btindex = in_iterate->btindex;
        o_work2->funcp = in_iterate->funcp;
        o_work2->result = zero;

        o_work->btindex = in_iterate->btindex;
        o_work->funcp = in_iterate->funcp;

        Value_t x;
        x = in_iterate->low;
        o_work->result = o_work->funcp(x, zero);

        print_iterate(in_edtType, in_thisEDT, in_iterate, "Transition");
        print_work(in_edtType, in_thisEDT,       *o_work, "Transition");
        print_work(in_edtType, in_thisEDT,      *o_work2, "Transition2");

        break;  //  while not err
    }
    return err;
}

unsigned int btCalculateJoinIndex(BTindex_t in_index)
{
    return (2 - (1 & in_index)); //Odd index -> 1; even index -> 2'
}

Err_t joinOperationIFTHEN(int in_edtType, ocrGuid_t in_thisEDT, Work_t in_left, Work_t in_right, Work_t * o_work)
{
    Err_t err=0;
    while(!err){
        copy_work(in_left,o_work);
        o_work->btindex = get_parent_index(in_left.btindex);
        o_work->result = o_work->funcp(in_left.result, in_right.result);
        print_work( in_edtType, in_thisEDT, *o_work, "JoinIFTHEN");
        break;  //  while not err
    }
    return err;
}

Err_t joinOperationELSE(int in_edtType, ocrGuid_t in_thisEDT, Work_t in_left, Work_t in_right, Work_t * o_work)
{
    Err_t err=0;
    while(!err){
        copy_work(in_left,o_work);
        o_work->btindex = get_parent_index(in_left.btindex);
        o_work->result = o_work->funcp(in_left.result, in_right.result);
        print_work(in_edtType, in_thisEDT, *o_work, "JoinELSE");
        break;  //  while not err
    }
    return err;
}

Err_t concludeBtForkJoin(int in_edtType, ocrGuid_t in_thisEDT, Value_t in_reference, Value_t in_result)
{
    Err_t err=0;
    while(!err){
        if(in_reference == in_result){
            PRINTF("TESTIO> Concluding: TaskTYPE=%d TaskID="GUIDF" Work is ok.\n", in_edtType, GUIDA(in_thisEDT));
        }else{
            PRINTF("TESTIO> Concluding: TaskTYPE=%d TaskID="GUIDF" ERROR: ref=%lu  work=%lu.\n",
                    in_edtType, in_thisEDT,
                   (unsigned long) in_reference,
                   (unsigned long) in_result
                  );
            err = __LINE__; IFEB;
        }
        break;  //  while not err
    }
    return err;
}

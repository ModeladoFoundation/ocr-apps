#ifndef INCLUSION_BINTREE_FORKJOIN_H
#include "treeForkJoin.h"
#endif

//No longer needed> #include <stdio.h> //sprintf
//Removed for runs on TG> #include <string.h> //memcpy

#include "integer_log2.h"

//#define TREE_FORKJOIN_VERBOSE  //Define this in order to get activity messages
                               //to be outputted; otherwise do not define.


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

TChecksum_t treeFJ_checksum_add(TChecksum_t one, TChecksum_t two)
{
    return (one + two);
}
void treeFJ_checksum_add_zero(TChecksum_t * io)
{
    *io = 0;
}

void print_iterate(int in_edtType, ocrGuid_t in_thisEDT,
                   TFJiterate_t * it, const char * in_text)
{
#ifdef TREE_FORKJOIN_VERBOSE
    const char * defaultt= "";

    const char * t;
    if(!in_text || in_text[0]=='\0'){
        t = defaultt;
    } else {
        t = in_text;
    }

    if(ocrGuidIsEq(in_thisEDT, NULL_GUID)){
        //This printf does not output EDT guids in order to make the output
        //independent of the underlying value of the guids.
        PRINTF("TESTIO> TREEFORKJOIN TaskTYPE=%d TaskID="GUIDF" ITERATE dex=%5lu lohi=%5u,%5u  %s\n",
                in_edtType,
                GUIDA(in_thisEDT),
                it->btindex,
                it->low, it->hi,
                t
               );
    }else {
        PRINTF("TESTIO> TREEFORKJOIN TaskTYPE=%d TaskID="GUIDF" ITERATE dex=%5lu lohi=%5u,%5u  Togo="GUIDF" %s\n",
                in_edtType,
                GUIDA(in_thisEDT),
                it->btindex,
                it->low, it->hi,
                GUIDA(it->whereToGoWhenFalse),
                t
               );
    }
#endif // TREE_FORKJOIN_VERBOSE
}
void print_TChecksum_work(int in_edtType, ocrGuid_t in_thisEDT, TChecksum_work_t w,
                          const char * in_text)
{
#ifdef TREE_FORKJOIN_VERBOSE
    const char * defaultt= "";

    const char * t;
    if(!in_text || in_text[0]=='\0'){
        t = defaultt;
    } else {
        t = in_text;
    }

    PRINTF("TESTIO> TREEFORKJOIN TaskTYPE=%d TaskID="GUIDF" WORK dex=%lu res=%lu %s\n",
            in_edtType,
            GUIDA(in_thisEDT),
            w.btindex,
            (unsigned long) w.result,
            t
           );
#endif // TREE_FORKJOIN_VERBOSE
}

void copy_TFJiterate(TFJiterate_t * in, TFJiterate_t * out)
{
    //Removed for runs on TG> memcpy(out, in, sizeof(TFJiterate_t));
    out->btindex = in->btindex;
    out->hi      = in->hi;
    out->low     = in->low;
    out->whereToGoWhenFalse = in->whereToGoWhenFalse;
}

void copy_TChecksum_work(TChecksum_work_t w, TChecksum_work_t * out)
{
    //Removed for runs on TG> memcpy(out, &w, sizeof(TChecksum_work_t));
    out->btindex = w.btindex;
    out->result  = w.result;
}

Err_t setupBtForkJoin(int in_edtType, ocrGuid_t in_thisEDT,
                      TFJiterate_t * io_iterate,
                      ocrGuid_t in_whereToGoWhenFalse,
                      SPMD_GlobalData_t * in_SPMDglobalData,
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
        io_iterate->hi  = in_SPMDglobalData->overall_mpi_count;

        print_iterate(in_edtType, in_thisEDT, io_iterate, "setupBtForkJoin");

        treeFJ_checksum_add_zero(o_refvalue);

        unsigned int i;
        for(i=io_iterate->low; i<=io_iterate->hi; ++i){
            TChecksum_t x = i;
            *o_refvalue = treeFJ_checksum_add(x, *o_refvalue);
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

        copy_TFJiterate(in_iterate, o_iterate);
        GUID_ASSIGN_VALUE(*o_gDone, in_iterate->whereToGoWhenFalse);

        GUID_ASSIGN_VALUE(o_iterate->whereToGoWhenFalse, in_whenDone);

        print_iterate(in_edtType, in_thisEDT, o_iterate, "btForkThen new iterate");

        break;  //  while not err
    }
    return err;
}

char * myUtoA(unsigned int in, char *io)
{
    if(!io) return 0;
    if(in == 0){
        io[0] = '0';
        io[1] = '\0';
        return io;
    }

    int i = 0;
    while(in != 0)
    {
        unsigned int rem = in % 10;
        io[i++] = rem + '0';
        in = in/10;
    }

    io[i] = '\0';

    //Reverse the sring
    int start = 0;
    int end = i -1;
    while (start < end)
    {
        char c = *(io+start);
        *(io+start) = *(io+end);
        *(io+end) = c;
        start++;
        end--;
    }

    return io;
}

void makeBtForkFOR_text(const char * in_header, unsigned int in_foliationNb, const char * in_footer, char * io_buf)
{
    char ntext[256];
    myUtoA( in_foliationNb, ntext );
    char * p = io_buf;

    const char * q = in_header;
    while( !q && *q !='\0'){
        *p = *q;
        ++p;
        ++q;
    }

    q = ntext;
    while( !q && *q !='\0'){
        *p = *q;
        ++p;
        ++q;
    }

    q = in_footer;
    while( !q && *q !='\0'){
        *p = *q;
        ++p;
        ++q;
    }

}

Err_t btForkFOR(int in_edtType, ocrGuid_t in_thisEDT, int in_foliationIndex,
                TFJiterate_t * in_iterate, TFJiterate_t * o_iterate)
{
    Err_t err=0;
    while(!err){
        if( ! in_iterate) {err=__LINE__; IFEB;}
        if(in_foliationIndex <0 || 2<=in_foliationIndex) {
            //This has only been tested for Nfoliation equals to 2.
            err = __LINE__; IFEB;
        }

        char foliationTxt[256];
        makeBtForkFOR_text("btForkFOR initial (i=", in_foliationIndex, ")", foliationTxt);
        print_iterate(in_edtType, in_thisEDT, in_iterate, foliationTxt);

        if(in_iterate->low == in_iterate->hi){
            //This cannot be split any further
            err = __LINE__; IFEB;
        }

        copy_TFJiterate(in_iterate, o_iterate);

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

        makeBtForkFOR_text("btForkFOR final (i=", in_foliationIndex, ")", foliationTxt);
        print_iterate(in_edtType, in_thisEDT, o_iterate, foliationTxt);

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

        copy_TFJiterate(in_iterate, o_iterate);
        GUID_ASSIGN_VALUE(*o_gDone, in_iterate->whereToGoWhenFalse);

        GUID_ASSIGN_VALUE(o_iterate->whereToGoWhenFalse, in_whenDone);

        print_iterate(in_edtType, in_thisEDT, o_iterate, "btForkElse new iterate");

        break;  //  while not err
    }
    return err;
}

Err_t transitionBTFork(int in_edtType, ocrGuid_t in_thisEDT, TFJiterate_t * in_iterate,
                       TChecksum_work_t * o_TChecksum_work, TChecksum_work_t * o_TChecksum_work2)
{
    Err_t err=0;
    while(!err){
        if( ! in_iterate) {err=__LINE__; IFEB;}
        if( in_iterate->low != in_iterate->hi) {err=__LINE__; IFEB;}

        TChecksum_t zero;
        treeFJ_checksum_add_zero(&zero);

        o_TChecksum_work2->btindex = in_iterate->btindex;
        o_TChecksum_work2->result = zero;

        o_TChecksum_work->btindex = in_iterate->btindex;

        TChecksum_t x;
        x = in_iterate->low;
        o_TChecksum_work->result = treeFJ_checksum_add(x, zero);

        print_iterate(in_edtType, in_thisEDT, in_iterate, "Transition");
        print_TChecksum_work(in_edtType, in_thisEDT,       *o_TChecksum_work, "Transition");
        print_TChecksum_work(in_edtType, in_thisEDT,      *o_TChecksum_work2, "Transition2");

        break;  //  while not err
    }
    return err;
}

unsigned int btCalculateJoinIndex(BTindex_t in_index)
{
    return (2 - (1 & in_index)); //Odd index -> 1; even index -> 2'
}

Err_t joinOperationIFTHEN(int in_edtType, ocrGuid_t in_thisEDT,
                          TChecksum_work_t in_left, TChecksum_work_t in_right,
                          TChecksum_work_t * o_TChecksum_work)
{
    Err_t err=0;
    while(!err){
        copy_TChecksum_work(in_left,o_TChecksum_work);
        o_TChecksum_work->btindex = get_parent_index(in_left.btindex);
        o_TChecksum_work->result = treeFJ_checksum_add(in_left.result, in_right.result);
        print_TChecksum_work( in_edtType, in_thisEDT, *o_TChecksum_work, "JoinIFTHEN");
        break;  //  while not err
    }
    return err;
}

Err_t joinOperationELSE(int in_edtType, ocrGuid_t in_thisEDT,
                        TChecksum_work_t in_left, TChecksum_work_t in_right,
                        TChecksum_work_t * o_TChecksum_work)
{
    Err_t err=0;
    while(!err){
        copy_TChecksum_work(in_left,o_TChecksum_work);
        o_TChecksum_work->btindex = get_parent_index(in_left.btindex);
        o_TChecksum_work->result = treeFJ_checksum_add(in_left.result, in_right.result);
        print_TChecksum_work(in_edtType, in_thisEDT, *o_TChecksum_work, "JoinELSE");
        break;  //  while not err
    }
    return err;
}

Err_t concludeBtForkJoin(int in_edtType, ocrGuid_t in_thisEDT,
                         TChecksum_t in_reference, TChecksum_t in_result)
{
    Err_t err=0;
    while(!err){
        if(in_reference == in_result){
//DEV#           ifdef TREE_FORKJOIN_VERBOSE
                PRINTF("TESTIO> TREEFORKJOIN Concluding: TaskTYPE=%d TaskID="GUIDF" Work is ok.\n",
                       in_edtType, GUIDA(in_thisEDT));
//DEV#           endif
        }else{
#           ifdef TREE_FORKJOIN_VERBOSE
                PRINTF("TESTIO> TREEFORKJOIN Concluding: TaskTYPE=%d TaskID="GUIDF" ERROR: ref=%lu  work=%lu.\n",
                    in_edtType, in_thisEDT,
                   (unsigned long) in_reference,
                   (unsigned long) in_result
                  );
#           endif
            err = __LINE__; IFEB;
        }
        break;  //  while not err
    }
    return err;
}

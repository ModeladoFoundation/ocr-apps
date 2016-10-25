//2016Oct24: This code is working to test how one traverse a binary tree
//           when not using a power of two.
#include <stdio.h>

#include "integer_log2.h"

unsigned long reduct2ary_get_parent_index(unsigned long in_index)
{
    if(in_index == 0) return -1;
    return ((in_index-1)/2);
}

void reduct2ary_get_children_indices(unsigned long in_index, unsigned long * o_left, unsigned long * o_right)
{
    *o_left  = 2*in_index + 1;
    *o_right = 2*in_index + 2;
}

unsigned long reduc2ary_calculate_leafID(unsigned long in_rankCount, unsigned long in_rankID)
{
    //If rankCount is 4, so the initial rankID are 0,1,2,3 ,
    //then the leafID will be 3,4,5,6 , which are the number of the leaves
    //of binary tree with 4 leaves, and rooted at zero.
    //Likewise, if rankCount is 3, so the initial rankID are 0,1,2 ,
    //the the new leafID are 3,4,5 .
    unsigned long dex = 1;
    while( dex < in_rankCount) dex <<=1;  //Get lowest power of 2 bigger or equal than rankCount.
                                         // 2 because we are on a binary (2-ary) tree.
    dex = dex -1 + in_rankID;
    return dex;
}

int reduc2ary_is_smaller_than_itsmax_on_that_level(unsigned long in_rankCount, unsigned long in_leafid) //Returns 1 for yes; zero otherwise.
{
    unsigned long minleaf = reduc2ary_calculate_leafID(in_rankCount,0);
    unsigned long maxleaf = reduc2ary_calculate_leafID(in_rankCount, in_rankCount - 1);

    if(minleaf <= in_leafid && in_leafid < maxleaf) return 1;
    if(in_leafid == maxleaf) return 0;

    while(in_leafid < minleaf){
        minleaf = reduct2ary_get_parent_index(minleaf);
        maxleaf = reduct2ary_get_parent_index(maxleaf);
        if(minleaf <= in_leafid && in_leafid < maxleaf) return 1;
        if(in_leafid == maxleaf) return 0;
    }

    if(in_leafid == maxleaf) return 0;
    return 1;
}

unsigned long reduc2ary_parent_for_maxleaf(unsigned long in_topminleaf, unsigned long in_topmaxleaf, unsigned long in_leafid)
{
    unsigned long min = in_topminleaf, max = in_topmaxleaf, leaf = in_leafid;

    if(leaf > max) return 0;  //This should be an error.

    while( leaf < max && max !=0) {
        max = reduct2ary_get_parent_index(max);
        min = reduct2ary_get_parent_index(min);
    }

    if( ! (min <= leaf && leaf <= max) ){
        printf("ERROR: %d\n", __LINE__);
        return 0;
    }

    //printf("DBG> min=%lu  max=%lu  leaf=%lu\n", min,max, leaf);

    unsigned long left, right;
    unsigned long parent = reduct2ary_get_parent_index(leaf);
    reduct2ary_get_children_indices(parent, &left, &right);

    int bothKigsInrange = min <= left  && left <= right;
    bothKigsInrange    &= min <= right && right <= max;

    while( ! bothKigsInrange ){
        parent = reduct2ary_get_parent_index(parent);
        if(parent == 0 ){
            break;
        }
        reduct2ary_get_children_indices(parent, &left, &right);
        max = reduct2ary_get_parent_index(max);
        min = reduct2ary_get_parent_index(min);
        bothKigsInrange  = min <= left && left <= right;
        bothKigsInrange &= min <= right && right <= max;
    }

    return parent;
}

unsigned long reduc2ary_find_parent(unsigned long in_rankCount, unsigned long in_rankID)
{
    unsigned long parent;
    unsigned long topminleaf = reduc2ary_calculate_leafID(in_rankCount,0);
    unsigned long topmaxleaf = reduc2ary_calculate_leafID(in_rankCount, in_rankCount - 1);

    int lessThanMax = reduc2ary_is_smaller_than_itsmax_on_that_level(in_rankCount, in_rankID);
    if( ! lessThanMax){
        if( in_rankID == 0 ) lessThanMax = 1; //The root is always ok.
    }

    if(lessThanMax) {
        parent = reduct2ary_get_parent_index(in_rankID);
    }else {
        parent = reduc2ary_parent_for_maxleaf(topminleaf, topmaxleaf, in_rankID);
    }

    return parent;
}

void f2(void)
{
    unsigned int rankCount = 16;

    unsigned int rc;
    for(rc=1; rc<=rankCount; ++rc)
//    for(rc=1; rc<=5; ++rc)
//    unsigned int RC=31; for(rc = RC; rc < RC+1; ++rc)
    {
        unsigned long topminleaf = reduc2ary_calculate_leafID(rc,0);
        unsigned long topmaxleaf = reduc2ary_calculate_leafID(rc, rc - 1);

        printf("========= rankCount=%u  topminleaf=%lu  topmaxleaf=%lu\n", rc, topminleaf, topmaxleaf);

        unsigned int lf;
        for(lf=0; lf<=topmaxleaf; ++lf){

//            if(lf!=13) continue;

            unsigned long parent;

            parent = reduc2ary_find_parent(rc, lf);

            printf("leafid=%u", lf);
            printf("\tparent=%lu\n", parent);

            //printf("\n");

        }
    }
}

int main()
{
    //f1();
    f2();
    return 0;
}

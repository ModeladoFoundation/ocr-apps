#ifndef REDUCTION2ARY_INDICES_H
#include "reduction2ary_indices.h"
#endif

unsigned long reduct2ary_get_parent_index(unsigned long in_index)
{
    if(in_index == 0) return NOTA_reduc2ary_parentIndex;
    return ((in_index-1)/2);
}

void reduct2ary_get_children_indices(unsigned long in_index, unsigned long * o_left, unsigned long * o_right)
{
    *o_left  = 2*in_index + 1;
    *o_right = 2*in_index + 2;
}

unsigned long reduc2ary_calculate_leafID(unsigned long in_edtCount, unsigned long in_edtID)
{
    //If edtCount is 4, so the initial edtID are 0,1,2,3 ,
    //then the leafID will be 3,4,5,6 , which are the number of the leaves
    //of binary tree with 4 leaves, and rooted at zero.
    //Likewise, if edtCount is 3, so the initial edtID are 0,1,2 ,
    //the the new leafID are 3,4,5 .
    unsigned long dex = 1;
    while( dex < in_edtCount) dex <<=1;  //Get lowest power of 2 bigger or equal than edtCount.
                                         // 2 because we are on a binary (2-ary) tree.
    dex = dex -1 + in_edtID;
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
        //This is a sanity check, put in during DEV.
        //It should always pass.
        PRINTF("ERROR: reduc2ary_parent_for_maxleaf: %d\n", __LINE__);
        return 0;
    }

    //PRINTF("DBG> min=%lu  max=%lu  leaf=%lu\n", min,max, leaf);

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

unsigned int reduc2ary_select_slot2use(unsigned long leaf, unsigned long parent)
{
    // 0 for even leaf, 1 for odd leaf
    // For a pure binary tree, that is sufficient.

    unsigned int distance = 0;
    unsigned long f = leaf;
    while(f != parent){
        ++distance;
        f = reduct2ary_get_parent_index(f);
    }

    if( distance ==1){
        //This is the normal distance between a leaf and its parent.
        return (leaf & 1);
    }

    //If the distance between the parent is more than 1,
    //then the current leaf is on the max range size for its level,
    //and it had to skip a few levels to get to a proper parent.

    //Because the left hand side child, i.e. the min range, of the
    //is always an odd leaf number, the right hand side, the one
    //which jump at least one level, needs to be even.
    return 0;
}

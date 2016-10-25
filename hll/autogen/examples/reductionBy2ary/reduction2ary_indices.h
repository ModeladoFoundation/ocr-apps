#ifndef REDUCTION2ARY_INDICES_H
#define REDUCTION2ARY_INDICES_H

//USe reduc2ary_find_parent in order to get the correct
// index of the parent of an EDT/rank.
#define NOTA_reduc2ary_parentIndex ((unsigned long)(-1))      //See bintreeForkJoin::BTindex_t for details.
#define ROOT_reduc2ary_parentIndex 0
unsigned long reduct2ary_get_parent_index(unsigned long in_index);  //See bintreeForkJoin::BTindex_t for details.

void reduct2ary_get_children_indices(unsigned long in_index, unsigned long * o_left, unsigned long * o_right);

#define NOTA_reduc2ary_leafID ((unsigned long)(-1))
unsigned long reduc2ary_calculate_leafID(unsigned long in_edtCount, unsigned long in_edtID);

int reduc2ary_is_smaller_than_itsmax_on_that_level(unsigned long in_rankCount, unsigned long in_leafid); //Returns 1 for yes; zero otherwise.
unsigned long reduc2ary_parent_for_maxleaf(unsigned long in_topminleaf, unsigned long in_topmaxleaf, unsigned long in_leafid);
unsigned long reduc2ary_find_parent(unsigned long in_rankCount, unsigned long in_rankID);

unsigned int reduc2ary_select_slot2use(unsigned long leaf, unsigned long parent);

#endif // REDUCTION2ARY_INDICES_H

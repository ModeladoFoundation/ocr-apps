#define HAB_C_BLK_SIZE (16)

#define FINISH \
  finish {

#define END_FINISH \
  } // finish

#define FOR_OUTER_0xNx1(index,len) \
    Index_t index ## _len = (len); \
    Index_t index ## _blk = (HAB_C_BLK_SIZE); \
    for (Index_t index ## _out=0; index ## _out < index ## _len; index ## _out+= index ## _blk) { \
      Index_t index ## _end = (index ## _out + index ## _blk) < index ## _len?( index ## _out + index ## _blk) : index ## _len;

#define END_FOR_OUTER(index) \
  } // for index ## _out

#define FOR_INNER(index) \
  for (Index_t index = index ## _out; index < index ## _end; index += 1) {

#define END_FOR_INNER(index) \
  } // for index

#define ASYNC_IN_3(three,two,one, ...) \
  async IN( three ## _out, three ## _end, two ## _out, two ## _end, one ## _out, one ## _end, __VA_ARGS__ ) {

#define END_ASYNC_IN_3(three,two,one) \
  }

#define ASYNC_IN_2(two, one, ...) \
  async IN( two ## _out, two ## _end, one ## _out, one ## _end, __VA_ARGS__ ) {

#define END_ASYNC_IN_2(two,one) \
  }

#define ASYNC_IN_1(one, ...) \
  async IN( one ## _out, one ## _end, __VA_ARGS__ ) {

#define END_ASYNC_IN_1(one) \
  }

#define ASYNC_IN_1_INOUT(out, one, ... ) \
  async IN( one ## _out, one ## _end, __VA_ARGS__ ) INOUT out {

#define END_ASYNC_IN_1_INOUT(one) \
  }


#if 1
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdint.h>
//
//#include "hc.h"
//
//#include "RAG_habanaro.h"
//
//#define EIGHT 8
//
//double *global_node;
//
//typedef int Index_t;
//
//int main() {
//
//  global_node = (double *)malloc(EIGHT*EIGHT*sizeof(double));
//
//  FINISH
//
//    Index_t edgeElems = EIGHT, edgeNodes = (EIGHT+1);
//
//    Index_t dimE = edgeElems, dimEdimE = edgeElems*edgeElems;
//    Index_t dimN = edgeNodes, dimNdimN = edgeNodes*edgeNodes;
//
//    FOR_OUTER_0xNx1(pln,edgeElems)
//      FOR_OUTER_0xNx1(row,edgeElems)
//        FOR_OUTER_0xNx1(col,edgeElems)
//          ASYNC_IN_3(pln,row,col,               // INDEX
//                   global_node,                 // GLOBAL
//                   dimE,dimEdimE,dimN,dimNdimN) // LOCAL
//            FOR_INNER(pln)
//              Index_t pln_nidx = pln*dimNdimN;
//              Index_t pln_zidx = pln*dimEdimE;
//              FOR_INNER(row)
//                Index_t pln_row_nidx = pln_nidx + row*dimN;
//                Index_t pln_row_zidx = pln_zidx + row*dimE;
//                FOR_INNER(col)
//                  Index_t nidx = pln_row_nidx+col;
//                  Index_t zidx = pln_row_zidx+col;
//                  Index_t *localNode = (Index_t *)&global_node[EIGHT*zidx] ;
//                  localNode[0] = nidx                       ;
//                  localNode[1] = nidx                   + 1 ;
//                  localNode[2] = nidx            + dimN + 1 ;
//                  localNode[3] = nidx            + dimN     ;
//                  localNode[4] = nidx + dimNdimN            ;
//                  localNode[5] = nidx + dimNdimN        + 1 ;
//                  localNode[6] = nidx + dimNdimN + dimN + 1 ;
//                  localNode[7] = nidx + dimNdimN + dimN     ;
//                END_FOR_INNER(col)
//              END_FOR_INNER(row)
//            END_FOR_INNER(pln)
//          END_ASYNC_IN_3(pln,row,col)
//        END_FOR_OUTER(col)
//      END_FOR_OUTER(col)
//    END_FOR_OUTER(pln)
//  END_FINISH
//  return 0;
//}
#else
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdint.h>
//
//#include "hc.h"
//
//#define EIGHT 8
//double *global_node;
//
//typedef int Index_t;
//
//int main() {
//
//  global_node = (double *)malloc(EIGHT*EIGHT*sizeof(double));
//
//  finish {
//
//    Index_t edgeElems = EIGHT, edgeNodes = (EIGHT+1);
//
//    Index_t dimE = edgeElems, dimEdimE = edgeElems*edgeElems;
//    Index_t dimN = edgeNodes, dimNdimN = edgeNodes*edgeNodes;
//
//    Index_t pln_len = (edgeElems); Index_t pln_blk = ((BLK_SIZE));
//    for (Index_t pln_out=0; pln_out < pln_len; pln_out+= pln_blk) {
//      Index_t pln_end = (pln_out + pln_blk) < pln_len?( pln_out + pln_blk) : pln_len;
//
//      Index_t row_len = (edgeElems); Index_t row_blk = ((BLK_SIZE));
//      for (Index_t row_out=0; row_out < row_len; row_out+= row_blk) {
//        Index_t row_end = (row_out + row_blk) < row_len?( row_out + row_blk) : row_len;
//
//        Index_t col_len = (edgeElems); Index_t col_blk = ((BLK_SIZE));
//        for (Index_t col_out=0; col_out < col_len; col_out+= col_blk) {
//          Index_t col_end = (col_out + col_blk) < col_len?( col_out + col_blk) : col_len;
//
//          async IN( pln_out, pln_end, row_out, row_end, col_out, col_end, // INDEX
//                    global_node,                                          // GLOBAL
//                    dimE,dimEdimE,dimN,dimNdimN ) {                       // LOCAL
//            for (Index_t pln = pln_out; pln < pln_end; pln += 1) {
//              Index_t pln_nidx = pln*dimNdimN;
//              Index_t pln_zidx = pln*dimEdimE;
//              for (Index_t row = row_out; row < row_end; row += 1) {
//                Index_t pln_row_nidx = pln_nidx + row*dimN;
//                Index_t pln_row_zidx = pln_zidx + row*dimE;
//                for (Index_t col = col_out; col < col_end; col += 1) {
//                  Index_t nidx = pln_row_nidx+col;
//                  Index_t zidx = pln_row_zidx+col;
//                  Index_t *localNode = (Index_t *)&global_node[EIGHT*zidx] ;
//                  localNode[0] = nidx ;
//                  localNode[1] = nidx + 1 ;
//                  localNode[2] = nidx + dimN + 1 ;
//                  localNode[3] = nidx + dimN ;
//                  localNode[4] = nidx + dimNdimN ;
//                  localNode[5] = nidx + dimNdimN + 1 ;
//                  localNode[6] = nidx + dimNdimN + dimN + 1 ;
//                  localNode[7] = nidx + dimNdimN + dimN ;
//                } // for(col)
//              } // for(row)
//            } // for(pln)
//          } // async
//        } // for(col_out)
//      } // for(row_out)
//    } // for(pln_out)
//  } // finish
//  return 0;
//}
#endif

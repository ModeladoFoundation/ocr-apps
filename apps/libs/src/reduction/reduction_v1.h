/*
 Author: Chandra S Martha
 Copyright Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/
#ifndef _REDUCTION_V1_H
#define _REDUCTION_V1_H

#include "ocr.h"

//ARITY must be a power of 2; Could be relaxed by changing the tree implementation
#ifndef ARITY
#define ARITY 8
#endif

//nParticipants:    IN
//      Number of participants in the overall reduction
//PTR_EVT_inputs:   INOUT
//      The user provides a pointer to "N" events to store input events for the reduction
//      This function basically stores "N" input events to the reduction operation in the provided pointer and returns an output event
//      which will be satisfied by the whole reduction result
//      The function sets up the appropriate "ARITY"-ary reduction tree in the whole process
//FNC_reduction:    IN
//      Pointer to EDT function implementing the reduction for atmost "ARITY" input datablocks
//
//RETURN:
//  Output event for the reduction result
//  Any EDT that depends on the returned output event will be able to access the final reduction result

ocrGuid_t ocrLibRed_setup_tree_serial( u32 nParticipants, ocrGuid_t* PTR_EVT_inputs, ocrGuid_t (*FNC_reduction) (u32, u64*, u32, ocrEdtDep_t*) );
extern ocrGuid_t FNC_reduction_double(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

#endif

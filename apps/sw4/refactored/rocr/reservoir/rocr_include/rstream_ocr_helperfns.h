/* Copyright (C) 2016 Reservoir Labs, Inc. All rights reserved. */

// QP6 rights

/**
 * Helper functions for the user when using R-Stream with OCR.
 *
 * @author Benoit Pradelle <pradelle@reservoir.com>
 */

#ifndef RSTREAM_OCR_HELPERFNS_H
#define RSTREAM_OCR_HELPERFNS_H

/**
 * Converts a position in local datablock buffer into a position into the
 * global dataset.
 *
 * @param offset Offset in the local DB buffer (in number of elements from the
 * origin)
 * @param globalIdx [output] corresponding index along every data dimension in
 * the global dataset. Must point to a memory area large enough to hold
 * {@code dims} entries. The indices are generated from left to right.
 *
 * @param dims The number of data dimensions, as provided in the DB
 * initialization and finalization callbacks.
 * @param coord The current DB coordinates, as provided in the DB
 * initialization and finalization callbacks.
 * @param dimSz The number of elements along every data dimension, as provided
 * in the DB initialization and finalization callbacks.
 * @param ... Size of the global array along every dimension, as unsigned ints,
 * sorted from left to right: if the array is A[10][5], then it must be passed
 * as rocr_globalIdx(..., 10, 5).
 *
 * @return 1 if the value returned in {@code globalIdx} is inside the global
 * array, 0 otherwise.
 */
int rocr_globalIdx(unsigned long offset, unsigned long globalIdx[],
    unsigned int dims, unsigned long coord[], unsigned long dimSz[], ...);

#endif

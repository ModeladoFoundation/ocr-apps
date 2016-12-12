/* Copyright (C) 2016 Reservoir Labs, Inc. All rights reserved. */

#ifndef ROCR_ELECTION_H
#define ROCR_ELECTION_H

//
// ______________________________ Public types ________________________________
//


/**
 * Signature of a function that elects one particular EDT. Elections are useful
 * to pick one EDT among a set of EDTs for instance to assign it a particular
 * role.
 *
 * @param election opaque structure in which the election result is stored
 * @param coords coordinates of the EDT for which the election is performed.
 */
typedef void (*rocrElectionFn)(void *election, long *coords);

/**
 * Defines an election result, i.e. the coordinates of the elected EDT.
 */
typedef struct {
    /** Type identifier of the elected EDT */
    unsigned int type;

    /** Id of the elected EDT */
    unsigned long id;
} RocrElection;


//
// ________________________________ Callbacks _________________________________
//


/**
 * Callback used to elect an EDT. The provided private structure will be updated
 * with the id of the elected EDT. How this information is used depends on the
 * context. In the initial implementation, they are intended to select one EDT
 * creating a successor EDT or a datablock.
 *
 * This is currently implemented as a macro to allow an early exit to be
 * performed once an EDT has been elected.
 *
 * @param election (void*) structure updated with the election result
 * @param edtTp (uint) type of the elected EDT
 * @param edtId (ulong) id of the elected EDT
 */
#define rocrElect(election, edtTp, edtId) do {\
        RocrElection *_rocr_elec = election;\
        _rocr_elec->type = edtTp;\
        _rocr_elec->id = edtId;\
        return;\
    } while (0)

#endif

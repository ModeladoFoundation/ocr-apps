/*
Author: Brian R Nickerson
Copyright Intel Corporation 2016

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/


#ifndef __DBCOMMHEADER_H__
#define __DBCOMMHEADER_H__

#include <ocr.h>

// This include file specifies a struct that is placed at the top of any datablock that is passed from one EDT to another, for the case where the receiving EDT is a parent acting as a "collector"
// of inputs from multiple children, and a request for the parent to perform some sort of service, e.g. AllReduce.  The children must all agree on what service they want (e.g. they must ALL be
// asking for an AllReduce at the same time), but the parent is prepared to offer lots of different services.

typedef enum {
   Operation_Checksum,            // First time:  record the "golden" checksum;  subsequently compare against it.
   Operation_Plot,                // Aggregate the contributions of blocks to the plot file output.
   Operation_Profile,             // Aggregate the performance profile contributions of dying children to the aggregate.
} DataExchangeOpcode_t;

   DataExchangeOpcode_t  serviceOpcode;           // What general action is the parent to perform.  E.g.  dataExchange__CheckSum

typedef struct DbCommHeader_t DbCommHeader_t;

//#include "control.h"
//#include "object.h"
//#include <ocr.h>
//#include "continuationcloner.h"

typedef struct DbCommHeader_t {
// Description of operation being solicited from the parent.  While each operand/result has storage for all these details, the parent only needs to pry the information loose from one of them to
// figure out what service is being requested.  Which one is rather arbitrary, so long as the source code on both the sending and receiving side know which to query.
   DataExchangeOpcode_t  serviceOpcode;           // What general action is the parent to perform.  E.g.  dataExchange__CheckSum
   int                   timeStep;
   ocrGuid_t             atBat_Event;             // The event that carries THIS datablock from its producer (e.g. a child) to its consumer (e.g. a parent). The parent destroys this event.
   ocrGuid_t             onDeck_Event;            // The event that, in the next iteration, will carry whichever type of datablock this one is, i.e. the operand1 or result1 of this iteration.
//   int                   opcodeDetail;            // Specifics.  E.g. Perform reduction sum on doubles.
//   int                   operationLength;         // How much.  E.g. for checksum, we need to do a reduction sum on num_vars elements.
//   int                   subjectOffset;           // Offset into this datablock for where the operand/result starts.
//   int                   subjectDbIndex;          // Index of datablock (with respect to the array of datablocks catalogued in the Meta_t datablock).
} DbCommHeader_t;

#define sizeof_DbCommHeader_t (sizeof(DbCommHeader_t))

#endif // __DBCOMMHEADER_H__


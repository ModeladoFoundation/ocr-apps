// TODO: FIXME:  Adjust copyright notice
// ************************************************************************
//
// miniAMR: stencil computations with boundary exchange and AMR.
//
// Copyright (2014) Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
// retains certain rights in this software.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// Questions? Contact Courtenay T. Vaughan (ctvaugh@sandia.gov)
//                    Richard F. Barrett (rfbarre@sandia.gov)
//
// ************************************************************************


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <proto.h>
#include "meta.h"
//#include <mpi.h>


double timer(void)
{
   return(wtime());
}


// Gaskets for      Dynamic memory storage blocks

void gasket__ocrDbCreate(ocrGuid_t * block_dblk, void ** block_base, int size, char * file, const char * func, int line) {
   ocrDbCreate (block_dblk, block_base, size, 0, NULL_GUID, NO_ALLOC);
//printf ("In %s, called from %s in %s line %d, Allocation size = %d, address is at %p, guid = 0x%lx\n", __func__, func, file, line, size, *block_base, (unsigned long long) (*block_dblk));
#ifndef RELAX_DATABLOCK_SEASONING
   *block_base = NULL;              // We have to "season" datablocks by passing them to a continuation clone.  NULLify the pointer.  It will become valid in the clone.
#endif
} // gasket__ocrDbCreate


#if 0
//#define PRINT_GASKET_DEBUG_MESSAGES   // TODO

#include "block.h"
#include "comm.h"
#include "proto.h"
#include "timer.h"

void *ma_malloc(Globals_t * glbl, size_t size, char *file, int line)
{
   void *ptr;

   ptr = (void *) malloc(size);

   if (ptr == NULL) {
      printf("NULL pointer from malloc call in %s at %d\n", file, line);
      exit(-1);
   }

   if (glbl == NULL) {
      glbl = ptr;          // Very first call per rank is to allocate Global_t into glbl.
      counter_malloc = 0;  // Set glbl to the ptr so that counter_malloc and size_malloc
      size_malloc = 0.0;   // (which are actually in that struct) can be written.
   }

   counter_malloc++;

   size_malloc += (double) size;

   return(ptr);
}


// All of the following "gasket__..." functions are to fit malloc and MPI service functions into
// either their non-OCR variant (i.e. the original reference versions of the code), or the non-OCR
// variant in a fashion that unit tests the EDT-continuation logic needed for the OCR version, or
// else the OCR version.



// Gaskets for      Dynamic memory storage blocks

void gasket__ma_malloc(Globals_t * const glbl, Dbmeta_t * blockmeta, void ** pblock, size_t size, char * file, int line)
{
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/
   continuationOpcode = SeasoningOneOrMoreDbCreates;
#if defined(BUILD_REFERENCE_VERSION)
   *pblock = ma_malloc(glbl, size, file, line);
#elif defined(BUILD_OCR_PREPARATORY_REFERENCE_VERSION)
// Enforce that the datablock pointer is NOT valid until the continuation EDT.  In variant of the reference version that unit
// tests the OCR version, continuation EDT is immitated by crawling up the calling stack and back down.  In the course of doing
// that, the pointer is made valid ("seasoned") by the top-level control routine.  We don't return here, but rather, the caller
// of this function has to do a SUSPEND operation before attempting to utilize the pointer.  At the caller's option, it can do
// multiple mallocs before seasoning their pointers, though they must be into different datablocks.
   *pblock = ((void *) (((unsigned long long) ma_malloc(glbl, size, file, line)) ^ 0xBAD0000000000001LL));  // Ptr valid after "seasoning", which imitates what transfering control to a continuation EDT does.
   blockmeta->guid = -9999;
   blockmeta->size = size;
#elif defined(BUILD_OCR_VERSION)
   ocrDbCreate( &(blockmeta->guid), pblock, size, 0, NULL_GUID, NO_ALLOC );    // Create the datablock to share with all "ranks".
   blockmeta->size = size;
   *pblock = NULL;          // Not valid until "seasoned" by passing control to a continuation EDT.

   counter_malloc++;

   size_malloc += (double) size;

#else
ERROR: build version is undefined!
#endif
}


void gasket__free(Dbmeta_t * blockMeta, void ** blockAddr, char * file, int line)
{
#if defined(BUILD_OCR_VERSION)
   ocrDbDestroy(blockMeta->guid);
   *blockAddr = NULL;
   blockMeta->guid = NULL_GUID;
   blockMeta->size = 0;
#else
   free (*blockAddr);
   *blockAddr = NULL;
   blockMeta->guid = 0;
   blockMeta->size = 0;
#endif
}


// Gaskets for      Receiving communications from a sender.

int gasket__mpi_Recv (        // See the argument list in API for MPI_Recv
   Globals_t    * const glbl,
   void         * const buf,
   int            const count,
   MPI_Datatype   const datatype,
   int            const source,
   int            const tag,
   MPI_Comm       const comm,
   int            const indexOfCommunicator,
   MPI_Status   * const status,
   char         * const file,
   int            const line)
{
#if defined(BUILD_REFERENCE_VERSION)
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d; source is %d\n", my_pe, __func__, file, line, source); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d; source is %d\n", my_pe, __func__, file, line, source); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/
   return MPI_Recv(buf, count, datatype, source, tag, comm, status);
#else
   typedef struct {
      Frame_Header_t myFrame;
      int result;
      MPI_Request req;
      MPI_Status status;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__gasket__mpi_Wait__for_Recv_t;

#define result (lcl->result)
#define req    (lcl->req)
#define status (lcl->status)

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__gasket__mpi_Wait__for_Recv_t)

#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d; source is %d\n", my_pe, __func__, file, line, source); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d; source is %d\n", my_pe, __func__, file, line, source); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/
   result = gasket__mpi_Irecv(glbl, buf, count, datatype, source, tag, comm, indexOfCommunicator, &req, file, line + 1000000);
   CALL_SUSPENDABLE_CALLEE(1)
   result = gasket__mpi_Wait__for_Irecv(glbl, buf, count, datatype, source, comm, indexOfCommunicator, &req, &status, file, line + 1000000);
   DEBRIEF_SUSPENDABLE_FUNCTION(-9999)
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(result)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  result
#undef  req
#undef  status
#endif
}


int gasket__mpi_Irecv (       // See the argument list in API for MPI_Irecv
   Globals_t    * const glbl,
   void         * const buf,
   int            const count,
   MPI_Datatype   const datatype,
   int            const source,
   int            const tag,
   MPI_Comm       const comm,
   int            const indexOfCommunicator,
   MPI_Request  * const req,
   char         * const file,
   int            const line)
{
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d; source is %d\n", my_pe, __func__, file, line, source); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d; source is %d\n", my_pe, __func__, file, line, source); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/
#if defined(BUILD_REFERENCE_VERSION) || defined(BUILD_OCR_PREPARATORY_REFERENCE_VERSION)
   return MPI_Irecv(buf, count, datatype, source, tag, comm, req);
#else
   return 0;
#endif
}


int gasket__mpi_Wait__for_Irecv (
   Globals_t    * const glbl,
   void         * const buf,        // See this argument in API for MPI_Irecv
   int            const count,      // See this argument in API for MPI_Irecv
   MPI_Datatype   const datatype,   // See this argument in API for MPI_Irecv
   int            const source_arg,
   MPI_Comm       const comm,
   int            const indexOfCommunicator,
   MPI_Request  * const req,        // See this argument in API for MPI_Wait
   MPI_Status   * const status,
   char         * const file,
   int            const line)     // See this argument in API for MPI_Wait
{
#if defined(BUILD_REFERENCE_VERSION)
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d; source is %d\n", my_pe, __func__, file, line, source_arg); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d; source is %d\n", my_pe, __func__, file, line, source_arg); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/
   return MPI_Wait(req, status);
#else
   typedef struct {
      Frame_Header_t myFrame;
      int result;
      int sizeof_datatype;
      int source;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__gasket__mpi_Wait__for_Irecv_t;

#define result (lcl->result)
#define sizeof_datatype (lcl->sizeof_datatype)
#define source (lcl->source)

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__gasket__mpi_Wait__for_Irecv_t)
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d; source is %d\n", my_pe, __func__, file, line, source_arg); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d; source is %d\n", my_pe, __func__, file, line, source_arg); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/

   source = source_arg;

#if defined(BUILD_OCR_PREPARATORY_REFERENCE_VERSION)
   continuationOpcode = ReceivingACommunication;
   continuationDetail = source;
   //CALL_SUSPENDABLE_CALLEE(1)
//printf ("pe = %d, %s line %d calls %s, glbl = 0x%p, buf = 0x%p, count = %d, req = 0x%p, status = 0x%p\n", my_pe, file, line, __func__, glbl, buf, count, req, status); fflush(stdout);
   result = MPI_Wait(req, status);
   //DEBRIEF_SUSPENDABLE_FUNCTION(-9999)
#elif defined(BUILD_OCR_VERSION)
   if (comm != MPI_COMM_WORLD) {
      // Hackish code for OCR version.  Fortunately, when the communicator isn't MPI_COMM_WORLD, there is only one other case in this application, and we can derive the PEs that make up the communicator
      // from other data that we have lying around.

      int grpSize = np[indexOfCommunicator];                    // Number of PEs in this communicator.
      int myRank  = my_pe % grpSize;                            // My rank, relative to just this communicator;
      int offsetFromGroupRootToItsRankInWorld = my_pe - myRank; // World communicator's rank for the root rank of this communicator.
      source += offsetFromGroupRootToItsRankInWorld;            // Rank of source in the MPI_COMM_WORLD communicator.
//printf ("pe = %3d> %30s called from %20s line %7d: source = %d  --  indexOfCommunicator = %d, grpSize = %d, myRank = %d, offsetFromGroupRootToItsRankInWorld = %d\n", my_pe, __func__, file, line, source, indexOfCommunicator, grpSize, myRank, offsetFromGroupRootToItsRankInWorld);
   }
   continuationOpcode = ReceivingACommunication;
   continuationDetail = source;
   SUSPEND__RESUME_IN_CONTINUATION_EDT(0)

   switch (datatype) {
   case MPI_DOUBLE:
      sizeof_datatype = sizeof(double);
      break;
   case MPI_INTEGER:
      sizeof_datatype = sizeof(int);
      break;
   case MPI_BYTE:
      sizeof_datatype = sizeof(char);
      break;
   default:
      printf ("gasket__mpi_Wait__for_Irecv called for a MPI_Datatype not yet supported. 0x%lx / %ld\n", (unsigned long long) datatype, (unsigned long long) datatype); fflush(stdout);
      *((int *) 0) = 123;
   }
   dbmeta__newResource.size = sizeof_datatype * count;
//printf ("pe = %d, %s line %d calls %s, newResource of size %d being inducted from 0x%p to 0x%p, count=%d, sizeof_datatype = %d\n", my_pe, file, line, __func__, dbmeta__newResource.size, newResource, buf, count, sizeof_datatype); fflush(stdout);
   memcpy (buf, newResource, sizeof_datatype * count);
//printf ("pe = %d, %s line %d calls %s, About to call gasket_free.  newResource.guid = 0x%lx (0x%lx)\n", my_pe, file, line, __func__, (unsigned long long) dbmeta__newResource.guid, dbmeta__newResource.guid); fflush(stdout); fflush(stdout);
   gasket__free(&dbmeta__newResource, (void **) &newResource, __FILE__, __LINE__);
//printf ("pe = %d, %s line %d calls %s, back from gasket__free\n", my_pe, file, line, __func__); fflush(stdout);
   ocrEventDestroy(resource);
#else
ERROR: define a BUILD...VERSION symbol in the Makefile.new file
#endif
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(result)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  result
#undef  sizeof_datatype
#undef  source
#endif
}


// Gaskets for      Sending communications to a receiver.

int gasket__mpi_Send (        // See the argument list in API for MPI_Send
   Globals_t    * const glbl,
   const void   * const buf,
   int            const count,
   MPI_Datatype   const datatype,
   int            const dest,
   int            const tag,
   MPI_Comm       const comm,
   int            const indexOfCommunicator,
   char         * const file,
   int            const line)
{
#if defined(BUILD_REFERENCE_VERSION)
//send_cnt++;
//if ((send_cnt & 0) == 0) { printf ("pe = %d, %s line %d calls %s, send_cnt = %6d, isend_cnt = %6d, total = %6d, dest = %d\n", my_pe, file, line, __func__, send_cnt, isend_cnt, send_cnt+isend_cnt, dest); fflush(stdout); }
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d; dest = %d\n", my_pe, __func__, file, line, dest); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d; dest = %d\n", my_pe, __func__, file, line, dest); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/
   return MPI_Send(buf, count, datatype, dest, tag, comm);
#else
   typedef struct {
      Frame_Header_t myFrame;
      int result;
      MPI_Status status;
      MPI_Request req;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__gasket__mpi_Send_t;

#define result (lcl->result)
#define status (lcl->status)
#define req    (lcl->req)

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__gasket__mpi_Send_t)
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d; dest = %d\n", my_pe, __func__, file, line, dest); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d; dest = %d\n", my_pe, __func__, file, line, dest); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/

   CALL_SUSPENDABLE_CALLEE(1)
   result = gasket__mpi_Isend(glbl, buf, count, datatype, dest, tag, comm, indexOfCommunicator, &req, file, line + 1000000);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)
   result = gasket__mpi_Wait__for_Isend(glbl, &req, &status, file, line + 1000000);
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(result)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  result
#undef  status
#undef  req
#endif
}

int gasket__mpi_Isend (       // See the argument list in API for MPI_Isend
   Globals_t    * const glbl,
   const void   * const buf,
   int            const count,
   MPI_Datatype   const datatype,
   int            const dest_arg,
   int            const tag,
   MPI_Comm       const comm,
   int            const indexOfCommunicator,
   MPI_Request  * const req,
   char *         const file,
   int            const line)
{
#if defined(BUILD_REFERENCE_VERSION)
//isend_cnt++;
//if ((isend_cnt & 0) == 0) { printf ("pe = %d, %s line %d calls %s, send_cnt = %6d, isend_cnt = %6d, total = %6d, dest = %d\n", my_pe, file, line, __func__, send_cnt, isend_cnt, send_cnt+isend_cnt, dest_arg); fflush(stdout); }
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d; dest = %d\n", my_pe, __func__, file, line, dest_arg); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d; dest = %d\n", my_pe, __func__, file, line, dest_arg); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/
   return MPI_Isend(buf, count, datatype, dest_arg, tag, comm, req);
#else
   typedef struct {
      Frame_Header_t myFrame;
      int result;
      int sizeof_datatype;
      MPI_Status   status;
      int dest;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__gasket__mpi_Isend_t;

#define result (lcl->result)
#define sizeof_datatype (lcl->sizeof_datatype)
#define status (lcl->status)
#define dest   (lcl->dest)

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__gasket__mpi_Isend_t)
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d; dest = %d\n", my_pe, __func__, file, line, dest_arg); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d; dest = %d\n", my_pe, __func__, file, line, dest_arg); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/

   dest = dest_arg;

#if defined(BUILD_OCR_PREPARATORY_REFERENCE_VERSION)
//isend_cnt++;
//if ((isend_cnt & 0) == 0) { printf ("pe = %d, %s line %d calls %s, send_cnt = %6d, isend_cnt = %6d, total = %6d, dest = %d\n", my_pe, file, line, __func__, send_cnt, isend_cnt, send_cnt+isend_cnt, dest); fflush(stdout); }
   MPI_Type_size(datatype, &sizeof_datatype);
   gasket__ma_malloc(glbl, &dbmeta__tmpDb7, (void *) &tmpDb7, count*sizeof_datatype, file, line+1000000);
   SUSPEND__RESUME_IN_CONTINUATION_EDT(9999)
//printf ("pe = %d, Isend is back from seasoning the malloc.  Proceeding to send\n", my_pe); fflush(stdout);
   memcpy (tmpDb7, buf, count*sizeof_datatype);
   result = MPI_Isend(tmpDb7, count, datatype, dest, tag, comm, req);
   //result = MPI_Isend(buf, count, datatype, dest, tag, comm, req);
   result = MPI_Wait(req, &status);
   gasket__free(&dbmeta__tmpDb7, (void **) &tmpDb7, __FILE__, __LINE__);
#elif defined(BUILD_OCR_VERSION)
   if (comm != MPI_COMM_WORLD) {
      // Hackish code for OCR version.  Fortunately, when the communicator isn't MPI_COMM_WORLD, there is only one other case in this application, and we can derive the PEs that make up the communicator
      // from other data that we have lying around.

      int grpSize = np[indexOfCommunicator];                    // Number of PEs in this communicator.
      int myRank  = my_pe % grpSize;                            // My rank, relative to just this communicator;
      int offsetFromGroupRootToItsRankInWorld = my_pe - myRank; // World communicator's rank for the root rank of this communicator.
      dest += offsetFromGroupRootToItsRankInWorld;              // Rank of dest in the MPI_COMM_WORLD communicator.
//printf ("pe = %3d> %30s called from %20s line %7d: dest = %d  --  indexOfCommunicator = %d, grpSize = %d, myRank = %d, offsetFromGroupRootToItsRankInWorld = %d\n", my_pe, __func__, file, line, dest, indexOfCommunicator, grpSize, myRank, offsetFromGroupRootToItsRankInWorld);
   }
isend_cnt++;
//if ((isend_cnt & 0) == 0) { printf ("pe = %d, %s line %d calls %s, send_cnt = %6d, isend_cnt = %6d, total = %6d, dest = %d\n", my_pe, file, line, __func__, send_cnt, isend_cnt, send_cnt+isend_cnt, dest); fflush(stdout); }
   switch (datatype) {
   case MPI_DOUBLE:
      sizeof_datatype = sizeof(double);
      break;
   case MPI_INTEGER:
      sizeof_datatype = sizeof(int);
      break;
   case MPI_BYTE:
      sizeof_datatype = sizeof(char);
      break;
   default:
      printf ("gasket__mpi_Isend called for a MPI_Datatype not yet supported. 0x%lx / %ld\n", (unsigned long long) datatype, (unsigned long long) datatype); fflush(stdout);
      *((int *) 0) = 123;
   }
   gasket__ma_malloc(glbl, &dbmeta__tmpDb7, (void *) &tmpDb7, count*sizeof_datatype, file, line+1000000);
   SUSPEND__RESUME_IN_CONTINUATION_EDT(9999)
//printf ("pe = %d, Isend is back from seasoning the malloc.  Proceeding to send\n", my_pe); fflush(stdout);
   memcpy (tmpDb7, buf, count*sizeof_datatype);

   ocrGuid_t event;
//printf ("pe = %d, %s line %d, Sending communication %d to receiver pe %d (GUID index is %d)\n", my_pe, __FILE__, __LINE__, isend_cnt, dest, (((dest*num_pes+my_pe)*GUID_ROUND_ROBIN_SPAN)+guid_toggle__send[dest]));fflush(stdout);
   ocrGuidFromIndex(&event, shared->labeledGuidRange, (((dest*num_pes+my_pe)*GUID_ROUND_ROBIN_SPAN)+guid_toggle__send[dest]));
   guid_toggle__send[dest] = (guid_toggle__send[dest] + 1) & (GUID_ROUND_ROBIN_SPAN - 1);
#define DEFAULT_LG_PROPS GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG
   ocrEventCreate(&event, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS);
   ocrDbRelease(dbmeta__tmpDb7.guid);
   ocrEventSatisfy(event, dbmeta__tmpDb7.guid);
   tmpDb7 = NULL;
   dbmeta__tmpDb7.guid = NULL_GUID;
   dbmeta__tmpDb7.size = 0;
#else
ERROR: Build type unknown
#endif
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(result)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  result
#undef  sizeof_datatype
#undef  status
#undef  dest

#endif
}

int gasket__mpi_Wait__for_Isend(
   Globals_t    * const glbl,
   MPI_Request  * const req,        // See this argument in API for MPI_Wait
   MPI_Status   * const status,
   char         * const file,
   int            const line)     // See this argument in API for MPI_Wait
{
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/
#if defined(BUILD_REFERENCE_VERSION)
   return MPI_Wait(req, status);
#else
   return 0;
#endif
}


// Gaskets for      aggregate functions:  Broadcast, Allreduce, Alltoall

int gasket__mpi_Bcast (
   Globals_t    * const glbl,
   void         * const buffer,
   int            const count,
   MPI_Datatype   const datatype,
   int            const root,
   MPI_Comm       const comm,
   char         * const file,
   int            const line)
{
#if defined(BUILD_REFERENCE_VERSION)
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/
   return MPI_Bcast (buffer, count, datatype, root, comm);
#else
   typedef struct {
      Frame_Header_t myFrame;
      MPI_Group grp;
      int grpSize;
      int myRank;
      int result;
      int i;
      MPI_Status status;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__gasket__mpi_Bcast_t;

#define grp         (lcl->grp)
#define grpSize     (lcl->grpSize)
#define myRank      (lcl->myRank)
#define result      (lcl->result)
#define i           (lcl->i)
#define status      (lcl->status)

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__gasket__mpi_Bcast_t)
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/

#ifdef BUILD_OCR_VERSION
   if (comm != MPI_COMM_WORLD) {
      printf ("Error:  gasket__mpi_Bcast is only able to handle MPI_COMM_WORLD communicatior\n");fflush(stdout);
      *((int *) 0) = 123;
      ocrShutdown();
   }
   grpSize = num_pes;
   myRank  = my_pe;
#else
   MPI_Comm_group(comm, &grp);
   MPI_Group_size(grp, &grpSize);
   MPI_Comm_rank(comm, &myRank);
#endif

   if (myRank == root) {
      for (i = 0; i < grpSize; i++) {
         if (i == root) continue;
         CALL_SUSPENDABLE_CALLEE(1)
         result = gasket__mpi_Send (glbl, buffer, count, datatype, i, 103, comm, -99999999, file, line+1000000);
         DEBRIEF_SUSPENDABLE_FUNCTION(0)
      }
   } else {
      CALL_SUSPENDABLE_CALLEE(1)
      result = gasket__mpi_Recv (glbl, buffer, count, datatype, 0, 103, comm, -99999999, &status, file, line+1000000);
      DEBRIEF_SUSPENDABLE_FUNCTION(0)
   }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(result)
   SUSPENDABLE_FUNCTION_EPILOGUE
#endif
#undef  grp
#undef  grpSize
#undef  myRank
#undef  result
#undef  i
#undef  status
}


int gasket__mpi_Allreduce (     // See API for MPI_Allreduce
   Globals_t    * const glbl,
   const void   * const sendbuf,
   void         * const recvbuf,
   int            const count,
   MPI_Datatype   const datatype,
   MPI_Op         const op,
   MPI_Comm       const comm_arg,
   int            const indexOfCommunicator,
   char         * const file,
   int            const line)
{
#if defined(BUILD_REFERENCE_VERSION)
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/
   return MPI_Allreduce (sendbuf, recvbuf, count, datatype, op, comm_arg);
#else

   typedef struct {
      Frame_Header_t myFrame;
      int i;
      int elementNum;
      double scratch[count];
      MPI_Group grp;
      int grpSize;
      int myRank;
      MPI_Status status;
      int offsetFromGroupRootToItsRankInWorld;
      MPI_Comm comm;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__gasket__mpi_Allreduce_t;

#define i           (lcl->i)
#define elementNum  (lcl->elementNum)
#define scratch     (lcl->scratch)
#define grp         (lcl->grp)
#define grpSize     (lcl->grpSize)
#define myRank      (lcl->myRank)
#define status      (lcl->status)
#define offsetFromGroupRootToItsRankInWorld (lcl->offsetFromGroupRootToItsRankInWorld)
#define comm        (lcl->comm)

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__gasket__mpi_Allreduce_t)
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/

   comm = comm_arg;

   switch (datatype) {
   case MPI_DOUBLE:
   case MPI_INTEGER:
      break;
   default:
      printf ("gasket__mpi_Allreduce called for a MPI_Datatype not yet supported.\n");fflush(stdout);
      *((int *) 0) = 123;
   }
   switch (op) {
   case MPI_SUM:
   case MPI_MIN:
   case MPI_MAX:
   break;
   default:
      printf ("gasket__mpi_Allreduce called for a MPI_Op not yet supported.\n");fflush(stdout);
      *((int *) 0) = 123;
   }

#ifdef BUILD_OCR_VERSION
   if (comm == MPI_COMM_WORLD) {
      grpSize = num_pes;
      myRank  = my_pe;
      offsetFromGroupRootToItsRankInWorld = 0;
//printf ("<<<< pe = %d, %s line %d, called from %s line %d, grpSize = %d, indexOfCommunicator = %d, myRank = %d, offset = %d, count = %d\n", my_pe, __FILE__, __LINE__, file, line, grpSize, indexOfCommunicator, myRank, offsetFromGroupRootToItsRankInWorld, count);
   } else {
      // Hackish code for OCR version.  Fortunately, when the communicator isn't MPI_COMM_WORLD, there is only one other case in this application, and we can derive the PEs that make up the communicator
      // from other data that we have lying around.

      comm = MPI_COMM_WORLD;                                // Do this just so that other checks we do in gasket__mpi routines don't gripe.
      grpSize = np[indexOfCommunicator];                    // Number of PEs in this communicator.
      myRank  = my_pe % grpSize;                            // My rank, relative to just this communicator;
      offsetFromGroupRootToItsRankInWorld = my_pe - myRank; // World communicator's rank for the root rank of this communicator.
//printf ("<<<< pe = %d, %s line %d, called from %s line %d, grpSize = %d, indexOfCommunicator = %d, myRank = %d, offset = %d, count = %d\n", my_pe, __FILE__, __LINE__, file, line, grpSize, indexOfCommunicator, myRank, offsetFromGroupRootToItsRankInWorld, count);
   }
#else
   MPI_Comm_group(comm, &grp);
   MPI_Group_size(grp, &grpSize);
   MPI_Comm_rank(comm, &myRank);
   offsetFromGroupRootToItsRankInWorld = 0;
#endif

   if (myRank == 0) {
      switch (datatype) {
      case MPI_DOUBLE:
         for (elementNum = 0; elementNum < count; elementNum++) ((double *) recvbuf)[elementNum] = ((double *) sendbuf)[elementNum];
         break;
      case MPI_INTEGER:
         for (elementNum = 0; elementNum < count; elementNum++) ((int *) recvbuf)[elementNum] = ((int *) sendbuf)[elementNum];
//printf ("**** pe = %d, %s at %d, RECVBUF = %d, %d, %d, %d, %d\n", my_pe, __FILE__, __LINE__, ((int *) recvbuf)[0], ((int *) recvbuf)[1], ((int *) recvbuf)[2], ((int *) recvbuf)[3], ((int *) recvbuf)[4]); fflush(stdout);
         break;
      }

      for (i = 1; i < grpSize; i++) {
//printf ("**** pe = %d, %s at %d, Expecting %d elements back from AllReduce partner %d\n", my_pe, __FILE__, __LINE__, count, i+offsetFromGroupRootToItsRankInWorld); fflush(stdout);
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Recv (glbl, scratch, count, datatype, i+offsetFromGroupRootToItsRankInWorld, 101, comm, -99999999, &status, file, line+1000000);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)

         switch(datatype) {
         case MPI_DOUBLE:
            switch(op) {
            case MPI_SUM:
               for (elementNum = 0; elementNum < count; elementNum++) ((double *) recvbuf)[elementNum] += ((double *) scratch)[elementNum];
               break;
            case MPI_MIN:
               for (elementNum = 0; elementNum < count; elementNum++)
                  ((double *) recvbuf)[elementNum] = ((double *) scratch)[elementNum] < ((double *) recvbuf)[elementNum] ? ((double *) scratch)[elementNum] : ((double *) recvbuf)[elementNum];
               break;
            case MPI_MAX:
               for (elementNum = 0; elementNum < count; elementNum++)
                  ((double *) recvbuf)[elementNum] = ((double *) scratch)[elementNum] > ((double *) recvbuf)[elementNum] ? ((double *) scratch)[elementNum] : ((double *) recvbuf)[elementNum];
               break;
            }
            break;
         case MPI_INTEGER:
            switch(op) {
            case MPI_SUM:
               for (elementNum = 0; elementNum < count; elementNum++) ((int *) recvbuf)[elementNum] += ((int *) scratch)[elementNum];
//printf ("**** pe = %d, %s at %d, RECVBUF = %d, %d, %d, %d, %d\n", my_pe, __FILE__, __LINE__, ((int *) recvbuf)[0], ((int *) recvbuf)[1], ((int *) recvbuf)[2], ((int *) recvbuf)[3], ((int *) recvbuf)[4]); fflush(stdout);
               break;
            case MPI_MIN:
               for (elementNum = 0; elementNum < count; elementNum++)
                  ((int *) recvbuf)[elementNum] = ((int *) scratch)[elementNum] < ((int *) recvbuf)[elementNum] ? ((int *) scratch)[elementNum] : ((int *) recvbuf)[elementNum];
               break;
            case MPI_MAX:
               for (elementNum = 0; elementNum < count; elementNum++)
                  ((int *) recvbuf)[elementNum] = ((int *) scratch)[elementNum] > ((int *) recvbuf)[elementNum] ? ((int *) scratch)[elementNum] : ((int *) recvbuf)[elementNum];
               break;
            }
            break;
         }
      }
      for (i = 1; i < grpSize; i++) {
//printf ("pe = %d, %s at line %d  calling send/isend\n", my_pe, __FILE__, __LINE__);fflush(stdout);
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Send (glbl, recvbuf, count, datatype, i+offsetFromGroupRootToItsRankInWorld, 102, comm, -99999999, file, line+1000000);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      }
      //gasket__free(scratch);   // Commented out because scratch is now on the stack (i.e. the application-managed stack).
   } else {
//printf ("pe = %d, %s at line %d  calling send/isend\n", my_pe, __FILE__, __LINE__);
//if (0 || datatype == MPI_INTEGER) printf ("**** pe = %d, %s at %d, SENDBUF = %d, %d, %d, %d, %d\n", my_pe, __FILE__, __LINE__, ((int *) sendbuf)[0], ((int *) sendbuf)[1], ((int *) sendbuf)[2], ((int *) sendbuf)[3], ((int *) sendbuf)[4]); fflush(stdout);
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Send (glbl, sendbuf, count, datatype, 0+offsetFromGroupRootToItsRankInWorld, 101, comm, -99999999, file, line+1000000);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Recv (glbl, recvbuf, count, datatype, 0+offsetFromGroupRootToItsRankInWorld, 102, comm, -99999999, &status, file, line+1000000);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
   }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(0)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  elementNum
#undef  scratch
#undef  grp
#undef  grpSize
#undef  myRank
#undef  status
#undef  offsetFromGroupRootToItsRankInWorld
#undef  comm
#endif
}

int gasket__mpi_Alltoall (      // See API for MPI_Alltoall
   Globals_t    * const glbl,
   const void   * const sendbuf,
   int            const sendcount,
   MPI_Datatype   const sendtype,
   void         * const recvbuf,
   int            const recvcount,
   MPI_Datatype   const recvtype,
   MPI_Comm       const comm,
   char         * const file,
   int            const line)
{
#if defined(BUILD_REFERENCE_VERSION)
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/
   return MPI_Alltoall (sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
#else

   typedef struct {
      Frame_Header_t myFrame;
      int i;
      int j;
      MPI_Group grp;
      int grpSize;
      int myRank;
      MPI_Status status;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__gasket__mpi_Alltoall_t;

#define i           (lcl->i)
#define j           (lcl->j)
#define grp         (lcl->grp)
#define grpSize     (lcl->grpSize)
#define myRank      (lcl->myRank)
#define status      (lcl->status)

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__gasket__mpi_Alltoall_t)
#ifdef PRINT_GASKET_DEBUG_MESSAGES
if (line <= 1000000) {
    printf ("pe = %3d, %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
} else {
    printf ("pe = %3d+ %30s called from %20s line %7d\n", my_pe, __func__, file, line); fflush(stdout);
}
#endif /*PRINT_GASKET_DEBUG_MESSAGES*/

   if (sendtype != MPI_INTEGER || recvtype != MPI_INTEGER) {
      printf ("gasket__mpi_Alltoall called for a MPI_Datatype not yet supported.\n");fflush(stdout);
      *((int *) 0) = 123;
   }
   if (sendcount != recvcount) {
      printf ("gasket__mpi_Alltoall called with sendcount != recvcount\n");fflush(stdout); // I don't know why they wouldn't always be equal!
      *((int *) 0) = 123;
   }

#ifdef BUILD_OCR_VERSION
   if (comm != MPI_COMM_WORLD) {
      printf ("Error:  gasket__mpi_Alltoall is only able to handle MPI_COMM_WORLD communicatior\n");fflush(stdout);
      *((int *) 0) = 123;
      ocrShutdown();
   }
   grpSize = num_pes;
   myRank  = my_pe;
#else
   MPI_Comm_group(comm, &grp);
   MPI_Group_size(grp, &grpSize);
   MPI_Comm_rank(comm, &myRank);
#endif

   // Send out my contribution to all other ranks in the communicator.
   CALL_SUSPENDABLE_CALLEE(1)
   for (i = 0; i < grpSize; i++) {
      if (i != myRank) {
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Send (glbl, &(((int *) sendbuf)[i*sendcount]), sendcount, sendtype, i, 103, comm, -99999999, file, line+1000000);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      }
   }
   DEBRIEF_SUSPENDABLE_FUNCTION(;)
   // Receive the contributions or all other ranks in the communicator.
   for (i = 0; i < grpSize; i++) {
      if (i == myRank) {
         if (sendbuf != recvbuf) {
            for (j = 0; j < sendcount; j++) {
               ((int *) recvbuf)[i*recvcount+j] = ((int *) sendbuf)[i*sendcount+j];
            }
         }
      } else {
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Recv(glbl, &(((int *) recvbuf)[i*recvcount]), recvcount, recvtype, i, 103, comm, -99999999, &status, file, line+1000000);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      }
   }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(0)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  grp
#undef  grpSize
#undef  myRank
#undef  status
#endif
}


#endif

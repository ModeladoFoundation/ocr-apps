/*
Author: Brian R Nickerson
Copyright Intel Corporation 2016

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

#ifndef __CLONE_H__
#define __CLONE_H__

#include <stdio.h>
#include <ocr.h>

typedef int DbSize_t;
#define DbSize(dep)                (dbSize[&dep-depv])

typedef struct {
   int  serialNum;          // Serial number of the datablock into which this pointer points.
   int  ptrOffset;          // Offset within the datablock.
} PtrAdjustmentRecord_t;
#define sizeof_PtrAdjustmentRecord_t (sizeof(PtrAdjustmentRecord_t))


// This is the cloner-managed stack, and related metadata.
typedef struct {
   char *                               tos;                              // Pointer to top of stack.  This is the stack of activation records.
   PtrAdjustmentRecord_t *              topPtrAdjRec;
   int                                  topPtrAdjRecOffset;               // Used to recover topPtrAdjRec upon crawl-out/crawl-in
   int                                  cloneNum;                         // Serial number of the clone.  First one is one.
   int                                  returnCanary;                     // Canary trap variable for detecting a return through a return statement.  Need to return via macro!

   enum  {
      SeasoningOneOrMoreDbCreates,      // Did one or more ocrDbCreates (and no other operation that triggers a clone) and now need to "season" the datablock(s)
      ReceivingACommunication,          // A remote EDT is sending us something.
      Fork,                             // Classic fork operation, but with number and/or kind of prongs being context specific.  (If need to implement multiple kinds, provide details in a fashion TBD.)
      Join,                             // Classic join operation, but with number and/or kind of prongs being context specific.  (If need to implement multiple kinds, provide details in a fashion TBD.)
      OpcodeUnspecified,                // Coding error:  at the point where cloning operation is triggered, need to provide an opcode.
      Shutdown,                         // Shutdown.
      Special,                          // Special case.  Perform an operation not generic enough to belong in this generalized API.  (If need to implement multiple kinds, provide details in a fashion TBD.)
      MassParallelismDone}              // We are collecting the results of a OpenMP-like fan-out operation.
   cloningOpcode;


#define                                 SIZEOFSTACK 1024
   char                                 stack[SIZEOFSTACK+64];            // Stack space, including a little extra "slop" space.
   int                                  numberOfDatablocks;               // Number of datablocks the cloner has to rationalize pointers against.
} Clone_t;


// This is a frame header upon the above stack, one per activation record, followed by local variables.
typedef struct {
   union {
      void * junk;                                     // This is merely a totally portable way to assure that the Frame_Header_t is aligned to an 8-byte boundary.
      struct {
         short resumption_case_num;                    // Zero on initial entry to a function, and upon its final return.
         short my_size;                                // Includes "my" frame header, but NOT my callee's frame header.
         short caller_size;                            // "my_size" of my caller, (so I can pop my frame back to his).
         short validate_callers_prep_for_suspension;   // Caller sets this to one to indicate it is prepared for callee's suspension.
      };
   };
} Frame_Header_t;
#define sizeof_Frame_Header_t (sizeof(Frame_Header_t))


void CastPointerToDbIndexAndOffset(Clone_t * cloningState, ocrEdtDep_t * depv, DbSize_t * dbSize, void * ptrToCast, int * dbIndex, int * offset, char * filename, const char * funcname, int linenum, int ptrnum);
void CapturePointerBaseAndOffset(Clone_t * cloningState, ocrEdtDep_t * depv, DbSize_t * dbSize, void * ptrToCapture, char * filename, const char * funcname, int linenum, int ptrnum);
void RestorePointerBaseAndOffset(Clone_t * cloningState, ocrEdtDep_t * depv, void ** ptrToRestore, char * filename, const char * funcname, int linenum, int ptrnum);

// A suspendable function starts with this macro, which obtains (or re-obtains) the stack frame, generates the switch statement boilerplate, and the case 0 label (initial call)
#define SUSPENDABLE_FUNCTION_PROLOGUE(baseStruct, frameName) \
   Clone_t * cloningState = &(baseStruct->cloningState); \
   cloningState->tos += ((Frame_Header_t *) cloningState->tos)->my_size;                /* Here, "my_size" is actually my caller's frame size.  Advance TOS to MY frame header. */ \
   cloningState->returnCanary  = 1;       /* Set "return canary trap" "live" so it will scream if we attempt to return to our callee through a return statement.  Must return through macro. */ \
   if ((((unsigned long long) cloningState->tos) + sizeof(frameName)) >= ((unsigned long long) &cloningState->stack[SIZEOFSTACK])) { \
      printf ("Stack overflow in Global_t block, detected in SUSPENDABLE_FUNCTION_PROLOGUE.  Increase SIZEOFSTACK and try again.\n"); fflush(stdout); \
      ocrShutdown(); \
   } \
   frameName * lcl = (frameName *) cloningState->tos;                 /* Carve out my frame. */ \
   if (lcl->myFrame.validate_callers_prep_for_suspension == 0) { /* If caller didn't prepare, we cannot suspend correctly. */ \
      printf ("CANARY TRAP!!!  Caller to %s (in %s) is NOT prepared to propagate suspension back to the root.\n", __func__, __FILE__); fflush(stdout); \
      *((int *) 123) = 456; \
      ocrShutdown(); \
   } \
   lcl->myFrame.validate_callers_prep_for_suspension = 0;   /* Done with this consistency check.  Prep for next time */ \
   if (lcl->myFrame.resumption_case_num == 0) {             /* Initial activation of this function instance (i.e. NOT a resumption). */ \
      lcl->myFrame.my_size = offsetof(frameName, calleeFrame); /* Initialize size of my frame. */ \
      lcl->calleeFrame.resumption_case_num = 0;             /* Initialize for my callee's initial activation. */ \
      lcl->calleeFrame.caller_size = lcl->myFrame.my_size;  /* Initialize for my callee's initial activation. */ \
      lcl->calleeFrame.validate_callers_prep_for_suspension = 0; /* Set "canary trap" for possibility that we are NOT prepared for calling a callee that might suspend activation back to me */ \
      { \
         int pointerCounter; \
         for (pointerCounter = 0; pointerCounter < sizeof(lcl->pointers)/sizeof(void *); pointerCounter ++) { \
            ((void **) (&(lcl->pointers)))[pointerCounter] = NULL; \
         } \
      } \
   } else { \
      if (lcl->myFrame.my_size != offsetof(frameName, calleeFrame)) { \
         printf ("%s at line %d, My stack frame size is wrong upon resumption of a previous activation.\n", __FILE__, __LINE__); fflush(stdout); \
         ocrShutdown(); \
      } \
      int pointerCounter; \
      for (pointerCounter = sizeof(lcl->pointers)/sizeof(void *); --pointerCounter >= 0; ) { \
         RestorePointerBaseAndOffset(cloningState, depv, &(((void **) (&(lcl->pointers)))[pointerCounter]), __FILE__, __func__, __LINE__, pointerCounter); \
      } \
   } \
   do { /* Create an opening bracket that is closed by the Eplilogue.  It is of the form do { ... } while (false); so only executes the do-loop once, but the form makes it more clear if there is a failure to match the prologue with the epilogue. */ \
   switch (lcl->myFrame.resumption_case_num) { \
   case 0: ;


// Caller has to precede the call to a suspendable function with this macro.  Otherwise a "canary trap" is triggered in the SUSPENDABLE_FUNCTION_ENTRY_SEQUENCE of
// the callee, to report that the call site probably has not been modified yet to convey a possible suspension of the callee back up the calling chain to the root.
#define CALL_SUSPENDABLE_CALLEE \
   do { /* Create an opening bracket that is closed by the Debrief.  It is of the form do { ... } while (false); so only executes the do-loop once, but the form makes it more clear if there is a failure to match the Calling macro with the debriefing macro. */ \
      case __COUNTER__+1: ; \
         lcl->calleeFrame.validate_callers_prep_for_suspension = 1;  /* Assure callee that it can suspend activation back to me and I will unwind. */


// Caller has to immediately follow the call to a suspendable function with this macro, to effect unwinding when a suspension operation needs to be perpetuated back to the root.
// If the function is of type void, use a semi-colon for the argument to the macro.
#define DEBRIEF_SUSPENDABLE_FUNCTION(suspensionReturnValue) \
      if (cloningState->returnCanary == 1) {  /* If the return canary is "live" the callee must have used a return statement.  We need it to use the macro! */ \
         printf ("ERROR: %s at line %d: Canary trap on RETURN from the callee.  Change callee to return through the NORMAL_RETURN_SEQUENCE macro.\n", __FILE__, __LINE__); fflush(stdout); \
         printf ("ERROR: Other possibility is that callee is not (yet) a SUSPENDABLE function, but it is being identified as such by the caller.\n"); fflush(stdout); \
         printf ("ERROR: And another possibility is that you commented out the call, but forgot to comment out the surrounding macros.\n"); fflush(stdout); \
         *((int *) 123) = 456; \
         ocrShutdown(); \
      } \
      if(lcl->calleeFrame.resumption_case_num != 0) {          /* If the callee suspended, we have to do likewise, so that activations unwind all the way up to the root. Prep for eventual reactivation. */ \
         lcl->myFrame.resumption_case_num = __COUNTER__;       /* Inform my caller that I am NOT done, and when it resumes me, this is the case number I need to return to. */ \
         cloningState->tos -= ((Frame_Header_t *) cloningState->tos)->caller_size;     /* Retreat TOS to my caller's frame header. */  \
         { \
            int pointerCounter; \
            for (pointerCounter = 0; pointerCounter < sizeof(lcl->pointers)/sizeof(void *); pointerCounter ++) { \
               CapturePointerBaseAndOffset(cloningState, depv, meta->dbSize, ((void **) (&(lcl->pointers)))[pointerCounter], __FILE__, __func__, __LINE__, pointerCounter); \
               ((void **) (&(lcl->pointers)))[pointerCounter] = NULL; \
            } \
         } \
         return suspensionReturnValue; \
      } \
      cloningState->returnCanary = 1;       /* Reset "return canary trap" "live" so it will scream if we attempt to return to our callee through a return statement.  Must return through macro. */ \
   } while (0);  /* This is the closing bracket to match up with the opening bracket expanded in the CALL_SUSPENDABLE_CALLEE macro */

// A suspendable function must change all normal return instructions to this macro instead.  It pops the stack frame, tells the caller its a normal return, and preps for the next initial activation of
// the caller's next callee.
// If the function is of type void, use a semi-colon for the argument to the macro.
#define SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(normalReturnValue) \
   lcl->myFrame.resumption_case_num = 0;             /* Inform my caller that I am done, and set up for caller's next callee. */ \
   cloningState->tos -= ((Frame_Header_t *) cloningState->tos)->caller_size;     /* Retreat TOS to my caller's frame header. */ \
   cloningState->returnCanary = 2;       /* Set "return canary trap" "dead" so caller will know it regained control by way of it's callee's use of a macro. */ \
   return normalReturnValue;

// Trigger suspension of the current EDT, unwinding the stack back to the root EDT function (so that it can "return" and thus terminate the EDT).  The current EDT should have created a clone EDT,
// and when that EDT's events are satisfied, it will crawl back through the stack to the current point, and carry-on herefrom.
//
// This is like an expansion of CALL_SUSPENDABLE_CALLEE immediately followed by DEBRIEF..., but without a function call.  We are NOT trying to call a SUSPENDABLE function, but rather, we called something
// that requires subsequent code only to proceed after we've received a dependency.  Examples are:
//
// 1) the reference code did a malloc, and then used the new block.  We instead do an ocrDbCreate.  Good form is for the creator of a datablock to NOT write to it, but rather to feed it to a child EDT
// for first access.  (The reason has to do with the future implementation of resiliency.)  So we need to follow the ocrDbCreate with an ocrEdtCreate of an EDT that will receive that datablock.
//
// 2) the reference code did an MPI_recv.  We have to do that by receiving that message block as an EDT.  The call gets replaced by this SUSPEND macro, and when the clone EDT RESUMEs at this point,
// we need to take the datablock that satisfied our EDT's dependency and stitch it into where the reference version's MPI_recv call received its message.
// If the function is of type void, use a semi-colon for the argument to the macro.
#define SUSPEND__RESUME_IN_CLONE_EDT(suspensionReturnValue) \
   cloningState->topPtrAdjRec = (PtrAdjustmentRecord_t *) (((unsigned long long) &(lcl->calleeFrame)) + sizeof(Frame_Header_t)); /* Use this as cursor for adding Pointer Adjustment Records as we crawl out. */ \
   lcl->myFrame.resumption_case_num = __COUNTER__+1;   /* Inform my caller that I am NOT done, and when it resumes me, this is the case number I need to return to. */ \
   cloningState->tos -= ((Frame_Header_t *) cloningState->tos)->caller_size;     /* Retreat TOS to my caller's frame header. */  \
   { \
      int pointerCounter; \
      for (pointerCounter = 0; pointerCounter < sizeof(lcl->pointers)/sizeof(void *); pointerCounter ++) { \
         CapturePointerBaseAndOffset(cloningState, depv, meta->dbSize, ((void **) (&(lcl->pointers)))[pointerCounter], __FILE__, __func__, __LINE__, pointerCounter); \
         ((void **) (&(lcl->pointers)))[pointerCounter] = NULL; \
      } \
   } \
   cloningState->returnCanary = 2;       /* Set "return canary trap" "dead" so caller will know it regained control by way of it's callee's use of a macro. */ \
   return suspensionReturnValue; \
   case __COUNTER__: ;

// A suspendable function has to finish (lexically) with the following boilerplate.
#define SUSPENDABLE_FUNCTION_EPILOGUE \
      printf ("ERROR, in %s of file %s you should NOT let the code flow into the EPILOGUE.  Use the SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE macro before epilogue.\n", __func__, __FILE__); fflush(stdout); \
      *((int *) 123) = 456; \
      ocrShutdown(); \
   default: \
      printf ("ERROR, resumption_case_num = %d is NOT defined for function %s in file %s\n", lcl->myFrame.resumption_case_num, __func__, __FILE__); fflush(stdout); \
      *((int *) 123) = 456; \
      ocrShutdown(); \
   } /* switch */ \
   } while (0);  /* This is the closing bracket to match up with the opening bracket expanded in the SUSPENDABLE_FUNCTION_PROLOGUE macro */

#endif // __CLONE_H__

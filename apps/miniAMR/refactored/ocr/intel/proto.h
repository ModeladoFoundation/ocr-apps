// TODO: FIXME:  adjust this copyright stuff
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

#ifndef __PROTO_H__
#define __PROTO_H__

#include "control.h"
#include "meta.h"
#include "profile.h"
#include <stdio.h>
//#include "block.h"


// root.c          This is the "root progenitor", i.e. the "Adam"-level parent of the entire mesh.
// **************************************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// mainEdt
//
// This is the EDT that the OCR startup instantiates.  It creates only ONE instance of this EDT.  The name "mainEdt" is essentially just a generic name made necessary by the fact that
// the OCR startup doesn't know the context of what will be done at the very start.  To make this more clear, we will simply make this function call a function whose name imlies what we
// need to do first:  rootProgenitorLaunch_Func.

ocrGuid_t mainEdt (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);


// **************************************************************************************************************************************************************************************************************
// rootProgenitorLaunch_Func  -- "Thunked" to by mainEdt, i.e. called as a function directly from mainEdt, NOT created as an EDT therefrom.
// -- Topology:  mainEdt --> rPLaunch(create datablocks) --> rPInit(initialize contents of datablocks, create children) --> rPClone(steady state: perform service requests of children; clone again)
//
// rootProgenitorLaunch_Func does the following:
//   * Process the command-line arguments only sufficiently to figure out how many objects will be modeled to move through the mesh.  Creates a "scratch" datablock for same.
//   * Creates a "control" datablock for an instance of the shared struct of parsed command line arguments and derived control variables.
//   * Creates a "wrapup" EDT, which will gain control when the parseCmdLine finish EDT terminates.  It will shut down the application.
//   * Passes control to the EDT that starts at rootProgenitorInit_Func.

ocrGuid_t rootProgenitorLaunch_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

typedef struct {
   ocrEdtDep_t argv_Dep;                    // OCR startup sends us a datablock comprised of the argc argument count followed by the arguments themselves.
} rootProgenitorLaunch_Deps_t;
#define sizeof_rootProgenitorLaunch_Deps_t (sizeof(rootProgenitorLaunch_Deps_t))

// rootProgenitorLaunch_Func receives a vacuous parameter list.  Don't bother to define a rootProgenitorLaunch_Params_t struct typedef.


// **************************************************************************************************************************************************************************************************************
// rootProgenitorInit_Func
// -- Topology:  mainEdt --> rPLaunch(create datablocks) --> rPInit(initialize contents of datablocks, create children) --> rPClone(steady state: perform service requests of children; clone again)
//
// This EDT does the following:
//   * Parses the command line into control_t and allScratchObjects_t.
//   * Creates one instance of blockLaunch for each unrefined block in the problem mesh, of which there are npx*npy*npz of them.  These are the "children".  Identify position of each by parameters thereto.
//   * Creates the first in the series of rootProgenitorContinuation clones, to await the first service request from the children.

ocrGuid_t rootProgenitorInit_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

typedef struct {
   ocrEdtDep_t argv_Dep;                // Passed in from mainEdt, OCR startup sends us a datablock comprised of the argc argument count followed by the arguments themselves.
   ocrEdtDep_t meta_Dep;                // My "meta data" mostly comprised of the continuation cloning stack and related support.
   ocrEdtDep_t control_Dep;             // mainEdt also provides the space into which to write the parsed arguments and derived control parameters.  These become READ-ONLY during driveblock processing.
   ocrEdtDep_t scratchAllObjects_Dep;   // mainEdt provides scratch space for writing the descriptions of the object(s) listed in the command line.
} rootProgenitorInit_Deps_t;
#define sizeof_rootProgenitorInit_Deps_t  (sizeof(rootProgenitorInit_Deps_t))
#define countof_rootProgenitorInit_Deps_t (sizeof_rootProgenitorInit_Deps_t / depsCountDivisor)

typedef struct {
   u64 dummy_Prm;                             // No parameters to pass to rootProgenitorInit_Func.
} rootProgenitorInit_Params_t;
#define sizeof_rootProgenitorInit_Params_t  (sizeof(rootProgenitorInit_Params_t))
#define countof_rootProgenitorInit_Params_t (sizeof_rootProgenitorInit_Params_t / paramsCountDivisor)


// **************************************************************************************************************************************************************************************************************
// rootProgenitorContinuation_Func
// -- Topology:  mainEdt --> rPLaunch(create datablocks) --> rPInit(initialize contents of datablocks, create children) --> rPClone(steady state: perform service requests of children; clone again)
//
// Provide services to the top-level blocks (i.e. the blocks that are totally unrefined, or the parents of refined blocks descended from them.)

ocrGuid_t rootProgenitorContinuation_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

typedef struct rootProgenitorContinuation_DepsAnnex_t {
   ocrEdtDep_t serviceRequestOperand_Dep;       // INPUT datablock to the rootProgenitor EDT for the service being requested by the child.  Initially generic, made specific by the opcode it carries.
} rootProgenitorContinuation_DepsAnnex_t;       // Size of the annex struct determined at run time as npx*npy*npz, and is invariant after initial determination.
typedef struct {
         // CAUTION:  These must agree in number and order with the guids unioned with the "datablock" array in the RootProgenitorMeta_t struct typedef in meta.h
   union {
      ocrEdtDep_t dependence[0];
      struct {
         ocrEdtDep_t meta_Dep;
         ocrEdtDep_t control_Dep;
         ocrEdtDep_t goldenChecksum_Dep;
         ocrEdtDep_t scratchChecksum_Dep;
#ifndef ALLOW_DATABLOCK_REWRITES
         ocrEdtDep_t metaClone_Dep;                       // Storage for the read/write "meta" data that tracks and controls progress of the algorithm.
#endif
         rootProgenitorContinuation_DepsAnnex_t annex[0]; // Size of the annex struct determined at run time as npx*npy*npz, and is invariant after initial determination.
      };
   };
} rootProgenitorContinuation_Deps_t;
#define sizeof_rootProgenitorContinuation_DepsAnnex_t       (sizeof(rootProgenitorContinuation_DepsAnnex_t)*control->npx*control->npy*control->npz)
#define sizeof_rootProgenitorContinuation_Deps_t            (sizeof(rootProgenitorContinuation_Deps_t)+sizeof_rootProgenitorContinuation_DepsAnnex_t)
#define countof_rootProgenitorContinuation_Deps_t           (sizeof_rootProgenitorContinuation_Deps_t / depsCountDivisor)
#define countof_rootProgenitorContinuation_AnnexDeps_t      (control->npx*control->npy*control->npz)

typedef struct {
   ocrGuid_t rootProgenitorContinuation_Template;
   int       isFirstChecksum;                   // True until we record the "golden" checksum, to which all subsequent checksums are compared.
} rootProgenitorContinuation_Params_t;
#define sizeof_rootProgenitorContinuation_Params_t  (sizeof(rootProgenitorContinuation_Params_t))
#define countof_rootProgenitorContinuation_Params_t (sizeof_rootProgenitorContinuation_Params_t / paramsCountDivisor)

//
// **************************************************************************************************************************************************************************************************************
// rootProgenitorContinuation_SoupToNuts
//
// This is the top-level function in the calling topology of the several functions that do the actual processing of the root progenitor's service duties.
//
void rootProgenitorContinuation_SoupToNuts(RootProgenitorMeta_t * meta, rootProgenitorContinuation_Params_t * myParams);


// ***************************************************************************************************************************************************************************************
// wrapup_Func

ocrGuid_t wrapup_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);
// One instance of this EDT is created by mainEdt, and gains control after the finish EDT parseCmdLine and ALL the topology below it terminates.


typedef struct {
   ocrEdtDep_t triggerEvent_Dep;
} wrapup_Deps_t;
#define sizeof_wrapup_Deps_t  (sizeof(wrapup_Deps_t))
#define countof_wrapup_Deps_t (sizeof_wrapup_Deps_t / depsCountDivisor)

typedef struct {
   u64 dummy_Prm;                           // wrapup doesn't really need any params, but give it one anyway.
} wrapup_Params_t;
#define sizeof_wrapup_Params_t  (sizeof(wrapup_Params_t))
#define countof_wrapup_Params_t (sizeof_wrapup_Params_t / paramsCountDivisor)


// ***************************************************************************************************************************************************************************************
// print_help_message
void print_help_message();



// block.c         This is a block of the mesh, at some refinement level.
// ***************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// **************************************************************************************************************************************************************************************************************
// blockLaunch_Func
//
// This just creates the blockInit EDT and the datablocks that it will need to initialize:  control_dblk, allObjects_dblk, and meta_dblk.  It then passes control to the blockInit EDT.
ocrGuid_t blockLaunch_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

typedef struct {
   ocrEdtDep_t control_Dep;
   ocrEdtDep_t allObjects_Dep;
   ocrEdtDep_t block_Dep;
} blockLaunch_Deps_t;
#define sizeof_blockLaunch_Deps_t  (sizeof(blockLaunch_Deps_t))
#define countof_blockLaunch_Deps_t (sizeof_blockLaunch_Deps_t / depsCountDivisor)

typedef struct {
   ocrGuid_t  blockLaunch_Template;
   ocrGuid_t  blockInit_Template;
   ocrGuid_t  blockContinuation_Template;
} blockTemplates_t;

typedef struct {
   blockTemplates_t template;
   int              refinementLevel;
   int              xPos;
   int              yPos;
   int              zPos;
   ocrGuid_t        conveyOperand_Event;   // Event that child satisfies with Operand_dblk in order to obtain a service from the parent. (Applicable first time; thereafter, child provides next event to parent.)
} blockLaunch_Params_t;
#define sizeof_blockLaunch_Params_t  (sizeof(blockLaunch_Params_t))
#define countof_blockLaunch_Params_t (sizeof_blockLaunch_Params_t / paramsCountDivisor)


// **************************************************************************************************************************************************************************************************************
// blockInit_Func
//
// This initializes control_dblk, allObjects_dblk, and meta_dblk, then creates all other datablocks that will be needed to process the block, then passes control to the blockContinuation EDT.
ocrGuid_t blockInit_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

typedef struct {
   ocrEdtDep_t initialControl_Dep;      // Parent sends us the Control_dblk.  The underlying content is read-only, but we copy it anyway so that we are sure that it resides in our policy domain.
   ocrEdtDep_t initialAllObjects_Dep;   // Parent sends us its AllObjects_dblk.  All siblings will get their own copy, and even though we will track object movement identically, it is best to have our own copy.
   ocrEdtDep_t initialBlock_Dep;        // Parent sends us his Block_dblk, which we use to build our refined block. (Exception, rootProgenitor sends NULL_GUID, meaning we manufacture our own initial blocks)
   ocrEdtDep_t meta_Dep;                // My "meta data" about block location, refinement level, guids and pointers of other datablocks I and my clones will use, and my stack for managing continuation cloning.
   ocrEdtDep_t control_Dep;             // My copy of the above.
   ocrEdtDep_t block_Dep;               // Space into which one octant of the parentBlock can be refined.
   ocrEdtDep_t allObjects_Dep;          // My copy of the above.
} blockInit_Deps_t;
#define sizeof_blockInit_Deps_t  (sizeof(blockInit_Deps_t))
#define countof_blockInit_Deps_t (sizeof_blockInit_Deps_t / depsCountDivisor)

typedef blockLaunch_Params_t blockInit_Params_t;     // The parameter list for blockInit is identical to that of blockLaunch.
#define sizeof_blockInit_Params_t  sizeof_blockLaunch_Params_t
#define countof_blockInit_Params_t countof_blockLaunch_Params_t



// **************************************************************************************************************************************************************************************************************
// blockContinuation_Func
//
// This is the code that performs the algorithm on a block at the leaf of the refinement level depth tree.  It advances the algorithm as far as it can, to such point where a communication is needed from
// another EDT (such as a halo exchange from a neighbor, or such as the results of a service requested of the parent), at which time it clones, and the clone continues where the predecessor left off.  This
// particular function just runs the loop that drives the continuation cloning logic.  It calls blockContinuation_SoupToNuts, which is the top function of the calling topology of functions that do the work.
ocrGuid_t blockContinuation_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

typedef struct {
         // CAUTION:  These must agree in number and order with the guids unioned with the "datablock" array in the BlockMeta_t struct typedef in meta.h
   union {
      ocrEdtDep_t dependence[0];
      struct {
         ocrEdtDep_t meta_Dep;                      // Storage for the read/write "meta" data that tracks and controls progress of the algorithm.
         ocrEdtDep_t control_Dep;                   // The parsed and derived controls.  This is READ-ONLY to this function, and its subsequent clones!  It is shared among all instances.
         ocrEdtDep_t block_Dep;                     // Storage for the block proper.
         ocrEdtDep_t allObjects_Dep;                // The object(s) being modeled as they migrate through the mesh domain.
         ocrEdtDep_t profile_Dep;                   // Performance metrics.
#ifndef ALLOW_DATABLOCK_REWRITES
         ocrEdtDep_t metaClone_Dep;                 // Storage for the read/write "meta" data that tracks and controls progress of the algorithm.
         ocrEdtDep_t blockClone_Dep;                // Storage for the block proper.
         ocrEdtDep_t allObjectsClone_Dep;           // The object(s) being modeled as they migrate through the mesh domain.
         ocrEdtDep_t profileClone_Dep;              // Performance metrics.
#endif
         ocrEdtDep_t checksum_Dep;                  // Storage for accumulating the block's contributiton to a checksum.
         ocrEdtDep_t fullFace_Dep[2];               // Storage for full faces, to exchange between neighbors.
         ocrEdtDep_t qrtrFace_Dep[2][2][2];         // Storage for quarter faces, to exchange between neighbors.
      };
   };
} blockContinuation_Deps_t;
#define sizeof_blockContinuation_Deps_t  (sizeof(blockContinuation_Deps_t))
#define countof_blockContinuation_Deps_t (sizeof_blockContinuation_Deps_t / depsCountDivisor)

typedef struct {
   blockTemplates_t template;
} blockContinuation_Params_t;
#define sizeof_blockContinuation_Params_t  (sizeof(blockContinuation_Params_t))
#define countof_blockContinuation_Params_t (sizeof_blockContinuation_Params_t / paramsCountDivisor)


// **************************************************************************************************************************************************************************************************************
// blockContinuation_SoupToNuts
//
// This is the top-level function in the calling topology of the several functions that do the actual processing of a block.
//
void blockContinuation_SoupToNuts(BlockMeta_t * meta);


// **************************************************************************************************************************************************************************************************************

// init.c
void init (BlockMeta_t * meta);

// checksum.c
void checksum (BlockMeta_t * meta);

// profile.c
void report_profile_results(Control_t * control, Profile_t * profile);
void profile_report_perf_2and4 (FILE * fp, Control_t * control, Profile_t * profile, char * version);
void calculate_results(Profile_t * profile);
void init_profile(BlockMeta_t * const meta);


// refine.c
void refine(BlockMeta_t * meta, int const ts);
//int refine_level(Globals_t * const glbl);
//void reset_all(Globals_t * const glbl);
//void reset_neighbors(Globals_t * const glbl);
//void redistribute_blocks(Globals_t * const glbl, double * const tp, double * const tm, double * const tu, double * const time, int * const num_moved, int const num_split);


// util.c
double timer(void);
void gasket__ocrDbCreate (ocrGuid_t  * guid,     // Guid of created datablock.
                          void      ** addr,     // Address of created datablock.  (Set to NULL if RELAX_DATABLOCK_SEASONING is NOT set, forcing caller to "season" by continuation-cloning the EDT.
                          int          size,     // Size of requested datablock in bytes.
                          char       * file,     // File of calling site.
                          const char * func,     // Function of calling site.
                          int          line);    // Line number of calling site.



#endif // __PROTO_H__

#if 0
#if defined(BUILD_REFERENCE_VERSION)
   #if defined(BUILD_OCR_PREPARATORY_REFERENCE_VERSION)
      ERROR:  In Makefile.new exactly ONE BUILD...VERSION symbol must be defined.
   #elif defined(BUILD_OCR_VERSION)
      ERROR:  In Makefile.new exactly ONE BUILD...VERSION symbol must be defined.
   #endif
#elif defined(BUILD_OCR_PREPARATORY_REFERENCE_VERSION)
   #if defined(BUILD_OCR_VERSION)
      ERROR:  In Makefile.new exactly ONE BUILD...VERSION symbol must be defined.
   #endif
#elif defined(BUILD_OCR_VERSION)
#else
   ERROR:  In Makefile.new exactly ONE BUILD...VERSION symbol must be defined.
#endif

#include <stddef.h>

// main.c
void driver__soup_to_nuts(Globals_t * const glbl, int const tmp_my_pe);
void print_help_message(Globals_t * const glbl);
void allocate(Globals_t * const glbl);
void deallocate(Globals_t * const glbl);
int check_input(Globals_t * const glbl);

// block.c
void split_blocks(Globals_t * const glbl);
void consolidate_blocks(Globals_t * const glbl);
void add_sorted_list(Globals_t * const glbl, int, int, int);
void del_sorted_list(Globals_t * const glbl, int, int);
int find_sorted_list(Globals_t * const glbl, int, int);

// check_sum.c
double check_sum(Globals_t * const glbl, int const var);

// comm_block.c
void comm_proc(Globals_t * const glbl);

// comm.c
void comm(Globals_t * const glbl, int const start, int const num_comm, int const stage);
void pack_face(Globals_t * const glbl, double *, int, int, int, int, int);
void unpack_face(Globals_t * const glbl, double *, int, int, int, int, int);
void on_proc_comm(Globals_t * const glbl, int, int, int, int, int);
void on_proc_comm_diff(Globals_t * const glbl, int, int, int, int, int, int, int);
void apply_bc(Globals_t * const glbl, int, void * /* Cast this to cell * */, int, int);

// comm_parent.c
void comm_parent(Globals_t * const glbl);
void comm_parent_reverse(Globals_t * const glbl);
void comm_parent_unrefine(Globals_t * const glbl);
void comm_parent_proc(Globals_t * const glbl);
void add_par_list(Globals_t * const glbl, Par_Comm_t * const pc, int const nparent, int const block, int const child, int const pe, int const sort);
void del_par_list(Globals_t * const glbl, Par_Comm_t * const pc, int const nparent, int const block, int const child, int const pe);

// comm_refine.c
void comm_refine(Globals_t * const glbl);
void comm_reverse_refine(Globals_t * const glbl);
void comm_refine_unrefine(Globals_t * const glbl);

// comm_util.c
void add_comm_list(Globals_t * const glbl, int const dir, int const block_f, int const pe, int const fcase, int const pos, int const pos1);
void del_comm_list(Globals_t * const glbl, int, int, int, int);
void zero_comm_list(Globals_t * const glbl);
void check_buff_size(Globals_t * const glbl);
void update_comm_list(Globals_t * const glbl);

// driver.c
void driver(Globals_t * const glbl);

// init.c
void init(Globals_t * const glbl);

// move.c
void move(Globals_t * const glbl);
void check_objects(Globals_t * const glbl);
int check_block(Globals_t * const glbl, double cor[3][2]);

// pack.c
void pack_block(Globals_t * const glbl, int);
void unpack_block(Globals_t * const glbl, int);

// plot.c
void plot(Globals_t * const glbl, int const ts);

// profile.c
void profile(Globals_t * const glbl);
void calculate_results(Globals_t * const glbl);
void init_profile(Globals_t * const glbl);

// rcb.c
void load_balance(Globals_t * const glbl);
void exchange(Globals_t * const glbl, double * const tp, double * const tm, double * const tu);
void sort(Globals_t * const glbl, int const div, int const fact, int const dir);
int factor(Globals_t * const glbl, int *);
int find_dir(Globals_t * const glbl, int, int, int, int);
void move_dots(Globals_t * const glbl, int const div, int const fact);
void move_dots_back(Globals_t * const glbl);
void move_blocks(Globals_t * const glbl, double * const tp, double * const tm, double * const tu);


// stencil.c
void stencil_calc(Globals_t * const glbl, int);

// target.c
int reduce_blocks(Globals_t * const glbl);
void add_blocks(Globals_t * const glbl);
void zero_refine(Globals_t * const glbl);

// util.c
double timer(void);
void *ma_malloc        (Globals_t * glbl, size_t, char *, int);
void gasket__ma_malloc(Globals_t * const glbl, Dbmeta_t * gblock, void ** pblock, size_t, char *, int);
void gasket__free(Dbmeta_t * blockMeta, void ** blockAddr, char * file, int line);
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
   int            const line);
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
   int            const line);
int gasket__mpi_Wait__for_Irecv (
   Globals_t    * const glbl,
   void         * const buf,        // See this argument in API for MPI_Irecv
   int            const count,      // See this argument in API for MPI_Irecv
   MPI_Datatype   const datatype,   // See this argument in API for MPI_Irecv
   int            const source,
   MPI_Comm       const comm,
   int            const indexOfCommunicator,
   MPI_Request  * const req,        // See this argument in API for MPI_Wait
   MPI_Status   * const status,
   char *         const file,
   int            const line);    // See this argument in API for MPI_Wait
int gasket__mpi_Send (        // See the argument list in API for MPI_Send
   Globals_t    * const glbl,
   const void   * const buf,
   int            const count,
   MPI_Datatype   const datatype,
   int            const dest,
   int            const tag,
   MPI_Comm       const comm,
   int            const indexOfCommunicator,
   char *         const file,
   int            const line);
int gasket__mpi_Isend (       // See the argument list in API for MPI_Isend
   Globals_t    * const glbl,
   const void   * const buf,
   int            const count,
   MPI_Datatype   const datatype,
   int            const dest,
   int            const tag,
   MPI_Comm       const comm,
   int            const indexOfCommunicator,
   MPI_Request  * const req,
   char *         const file,
   int            const line);
int gasket__mpi_Wait__for_Isend(
   Globals_t    * const glbl,
   MPI_Request  * const req,        // See this argument in API for MPI_Wait
   MPI_Status   * const status,
   char *         const file,
   int            const line);    // See this argument in API for MPI_Wait


int gasket__mpi_Bcast (
   Globals_t    * const glbl,
   void         * const buffer,
   int            const count,
   MPI_Datatype   const datatype,
   int            const root,
   MPI_Comm       const comm,
   char *         const file,
   int            const line);
int gasket__mpi_Allreduce (     // See API for MPI_Allreduce
   Globals_t    * const glbl,
   const void   * const sendbuf,
   void         * const recvbuf,
   int            const count,
   MPI_Datatype   const datatype,
   MPI_Op         const op,
   MPI_Comm       const comm,
   int            const indexOfCommunicator,
   char *         const file,
   int            const line);
int gasket__mpi_Alltoall (      // See API for MPI_Alltoall
   Globals_t    * const glbl,
   const void   * const sendbuf,
   int            const sendcount,
   MPI_Datatype   const sendtype,
   void         * const recvbuf,
   int            const recvcount,
   MPI_Datatype   const recvtype,
   MPI_Comm       const comm,
   char *         const file,
   int            const line);

//#define TRACE { printf("pe = %3d, %30s TRACE in %23s line %7d %s\n", my_pe, __func__, __FILE__, __LINE__, my_pe >= 64 ? "*****" : " "); fflush(stdout); }
#define TRACE
#define TRACErcb { printf("pe = %3d, %30s %23s line %7d TRACE %s\n", my_pe, __func__, __FILE__, __LINE__, my_pe >= 64 ? "*****" : " "); fflush(stdout); }
#endif

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

// refine.c
void refine(Globals_t * const glbl, int const ts);
int refine_level(Globals_t * const glbl);
void reset_all(Globals_t * const glbl);
void reset_neighbors(Globals_t * const glbl);
void redistribute_blocks(Globals_t * const glbl, double * const tp, double * const tm, double * const tu, double * const time, int * const num_moved, int const num_split);

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
void CapturePointerBaseAndOffset(Globals_t * const glbl, void *  ptrToCapture, char * filename, const char * funcname, int linenum, int ptrnum);
void RestorePointerBaseAndOffset(Globals_t * const glbl, void ** ptrToRestore, char * filename, const char * funcname, int linenum, int ptrnum);

//#define TRACE { printf("pe = %3d, %30s TRACE in %23s line %7d %s\n", my_pe, __func__, __FILE__, __LINE__, my_pe >= 64 ? "*****" : " "); fflush(stdout); }
#define TRACE
#define TRACErcb { printf("pe = %3d, %30s %23s line %7d TRACE %s\n", my_pe, __func__, __FILE__, __LINE__, my_pe >= 64 ? "*****" : " "); fflush(stdout); }

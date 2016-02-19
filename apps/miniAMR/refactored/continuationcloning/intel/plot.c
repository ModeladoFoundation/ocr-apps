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
#include <mpi.h>

#include "block.h"
#include "comm.h"
#include "proto.h"

// Write block information (level and center) to plot file.
void plot(Globals_t * const glbl, int const ts)
{

   OBTAIN_ACCESS_TO_blocks

   typedef struct {
      Frame_Header_t myFrame;
      int i, j, n, total_num_blocks, plot_buf_size, size;
      char fname[20];
      MPI_Status status;
      struct {
         Block_t *bp;
         FILE *fp;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__plot_t;

#define i                 (lcl->i)
#define j                 (lcl->j)
#define n                 (lcl->n)
#define total_num_blocks  (lcl->total_num_blocks)
#define plot_buf_size     (lcl->plot_buf_size)
#define size              (lcl->size)
#define fname             (lcl->fname)
#define bp                (lcl->pointers.bp)
#define fp                (lcl->pointers.fp)
#define status            (lcl->status)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__plot_t)
   TRACE
   fp = NULL;
   bp = NULL;

// Economize on datablocks.  Alias plot_buf onto tmpDb1.  Rework code to destroy it upon completion.
// Also, file handles (pointers) are opaque, and I cannot make the crawl-out--continuation-edt--crawl-back-in
// logic work correctly if a file is open;  I cannot assure that the file pointer is accurate.  Therefore
// I close the file at every continuation-edt crawl, and reopen it for append access.
#define dbmeta__plot_buf dbmeta__tmpDb1
#define         plot_buf         tmpDb1

   if (!my_pe) {
      fname[0] = 'p';
      fname[1] = 'l';
      fname[2] = 'o';
      fname[3] = 't';
      fname[4] = '.';
      for (n = 1, j = 0; n < num_tsteps; j++, n *= 10) ;
      for (n = 1, i = 0; i <= j; i++, n *= 10)
         fname[5+j-i] = (char) ('0' + (ts/n)%10);
      fname[6+j] = '\0';
      fp = fopen(fname, "w");

      total_num_blocks = 0;
      for (i = 0; i <= num_refine; i++)
         total_num_blocks += num_blocks[i];
      fprintf(fp, "%d %d %d %d %d\n", total_num_blocks, num_refine,
                                      npx*init_block_x, npy*init_block_y,
                                      npz*init_block_z);
      plot_buf_size = 0;
      fprintf(fp, "%d\n", num_active);
      for (n = 0; n < max_active_block; n++)
         if ((bp = &blocks[n])->number >= 0)
            fprintf(fp, "%d %d %d %d\n", bp->level, bp->cen[0],
                                         bp->cen[1], bp->cen[2]);
         bp = NULL;
      fclose(fp);
      fp = NULL;
      for (i = 1; i < num_pes; i++) {
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Send(glbl, &size, 1, MPI_INTEGER, i, 1, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Recv(glbl, &size, 1, MPI_INTEGER, i, 2, MPI_COMM_WORLD, -9999, &status, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         //if (size > plot_buf_size) {
         //   if (i != 1)
         //      gasket__free(&dbmeta__plot_buf, (void **) &plot_buf, __FILE__, __LINE__);
            plot_buf_size = size;
            gasket__ma_malloc(glbl, &dbmeta__plot_buf, (void *) &plot_buf, 4*plot_buf_size*sizeof(int), __FILE__, __LINE__);
            SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.
         //}
         gasket__mpi_Irecv(glbl, plot_buf, 4*size, MPI_INTEGER, i, 3, MPI_COMM_WORLD, -9999, &request[0], __FILE__, __LINE__);
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Send(glbl, &size, 1, MPI_INTEGER, i, 4, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Wait__for_Irecv(glbl, plot_buf, 4*size, MPI_INTEGER, i, MPI_COMM_WORLD, -9999, &request[0], &status, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         fp = fopen(fname, "a");
         fprintf(fp, "%d\n", size);
         for (n = j = 0; j < size; j++, n += 4)
            fprintf(fp, "%d %d %d %d\n", plot_buf[n], plot_buf[n+1], plot_buf[n+2], plot_buf[n+3]);
         fclose(fp);
         fp = NULL;
         gasket__free(&dbmeta__plot_buf, (void **) &plot_buf, __FILE__, __LINE__);
      }
   } else {
      total_num_blocks = 0;
      for (i = 0; i <= num_refine; i++)
         total_num_blocks += local_num_blocks[i];
      gasket__ma_malloc(glbl, &dbmeta__plot_buf, (void *) &plot_buf, 4*total_num_blocks*sizeof(int), __FILE__, __LINE__);
      SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.
      for (i = n = 0; n < max_active_block; n++)
         if ((bp = &blocks[n])->number >= 0) {
            plot_buf[i++] = bp->level;
            plot_buf[i++] = bp->cen[0];
            plot_buf[i++] = bp->cen[1];
            plot_buf[i++] = bp->cen[2];
         }
         bp = NULL;
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Recv(glbl, &size, 1, MPI_INTEGER, 0, 1, MPI_COMM_WORLD, -9999, &status, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Send(glbl, &total_num_blocks, 1, MPI_INTEGER, 0, 2, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Recv(glbl, &size, 1, MPI_INTEGER, 0, 4, MPI_COMM_WORLD, -9999, &status, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Send(glbl, plot_buf, 4*total_num_blocks, MPI_INTEGER, 0, 3, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      /*TODO*/gasket__free(&dbmeta__plot_buf, (void **) &plot_buf, __FILE__, __LINE__);
   }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  n
#undef  total_num_blocks
#undef  plot_buf_size
#undef  size
#undef  fname
#undef  bp
#undef  fp
#undef  status
}

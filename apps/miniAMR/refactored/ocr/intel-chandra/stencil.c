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

#include "block.h"
#include "proto.h"


_OCR_TASK_FNC_( FNC_calc )
{
    calcPRM_t* calcPRM = (calcPRM_t*) paramv;

    u64 istart = calcPRM->istart;
    u64 ts = calcPRM->ts;
    u64 istage = calcPRM->istage;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_array = depv[_idep++].guid;
    ocrDBK_t DBK_work = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    double* array      = depv[_idep++].ptr;
    double* work       = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    int ilevel = PTR_rankH->ilevel;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d istage %d istart %d\n", __func__, ilevel, PTR_rankH->myRank, ts, istage, istart ));

    bp->array = array;
    bp->work = work;

    int x_block_size = PTR_cmd->x_block_size;
    int y_block_size = PTR_cmd->y_block_size;
    int z_block_size = PTR_cmd->z_block_size;

    int n, i, j, k, var;
    double sb, sm, sf;

    var = istart;

    if (PTR_cmd->stencil == 7) {
            for (i = 1; i <= x_block_size; i++)
                for (j = 1; j <= y_block_size; j++)
                    for (k = 1; k <= z_block_size; k++)
                    WORK(bp,i,j,k) =
                           (IN(bp,var,i-1,j  ,k  ) +
                            IN(bp,var,i  ,j-1,k  ) +
                            IN(bp,var,i  ,j  ,k-1) +
                            IN(bp,var,i  ,j  ,k  ) +
                            IN(bp,var,i  ,j  ,k+1) +
                            IN(bp,var,i  ,j+1,k  ) +
                            IN(bp,var,i+1,j  ,k  ))/7.0;

            for (i = 1; i <= x_block_size; i++)
                for (j = 1; j <= y_block_size; j++)
                    for (k = 1; k <= z_block_size; k++) {
                        IN(bp,var,i,j,k) = WORK(bp,i,j,k);
                        //if( PTR_rankH->myRank==0 && ilevel == 2)
                        //DEBUG_PRINTF(( "WORK block number %d ilevel %d stage %d ts %d i %d j %d k %d var %d %f\n", PTR_rankH->myRank, ilevel, istage, ts, i, j, k, var, WORK(bp,i,j,k) ));
                     }
    }
    else {
            for (i = 1; i <= x_block_size; i++)
                for (j = 1; j <= y_block_size; j++)
                    for (k = 1; k <= z_block_size; k++) {
                         sb = IN(bp,var,i-1,j-1,k-1) +
                              IN(bp,var,i-1,j-1,k  ) +
                              IN(bp,var,i-1,j-1,k+1) +
                              IN(bp,var,i-1,j  ,k-1) +
                              IN(bp,var,i-1,j  ,k  ) +
                              IN(bp,var,i-1,j  ,k+1) +
                              IN(bp,var,i-1,j+1,k-1) +
                              IN(bp,var,i-1,j+1,k  ) +
                              IN(bp,var,i-1,j+1,k+1);
                         sm = IN(bp,var,i  ,j-1,k-1) +
                              IN(bp,var,i  ,j-1,k  ) +
                              IN(bp,var,i  ,j-1,k+1) +
                              IN(bp,var,i  ,j  ,k-1) +
                              IN(bp,var,i  ,j  ,k  ) +
                              IN(bp,var,i  ,j  ,k+1) +
                              IN(bp,var,i  ,j+1,k-1) +
                              IN(bp,var,i  ,j+1,k  ) +
                              IN(bp,var,i  ,j+1,k+1);
                         sf = IN(bp,var,i+1,j-1,k-1) +
                              IN(bp,var,i+1,j-1,k  ) +
                              IN(bp,var,i+1,j-1,k+1) +
                              IN(bp,var,i+1,j  ,k-1) +
                              IN(bp,var,i+1,j  ,k  ) +
                              IN(bp,var,i+1,j  ,k+1) +
                              IN(bp,var,i+1,j+1,k-1) +
                              IN(bp,var,i+1,j+1,k  ) +
                              IN(bp,var,i+1,j+1,k+1);
                         WORK(bp,i,j,k) = (sb + sm + sf)/27.0;
                    }

            for (i = 1; i <= x_block_size; i++)
                for (j = 1; j <= y_block_size; j++)
                    for (k = 1; k <= z_block_size; k++)
                        IN(bp,var,i,j,k) = WORK(bp,i,j,k);
    }

    return NULL_GUID;
}

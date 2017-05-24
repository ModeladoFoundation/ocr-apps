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

#include <math.h>

#include "block.h"
#include "proto.h"

// Generate check sum for a variable over all active blocks.
double check_sum( ocrDBK_t DBK_rankH, rankH_t* PTR_rankH, int ts, int istage, int var, ocrDBK_t DBK_gridSum_in, double* sum )
{
    int n, in, i, j, k;
    double gsum, block_sum, t1, t2, t3;

    block *bp = &PTR_rankH->blockH;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    int x_block_size = PTR_cmd->x_block_size;
    int y_block_size = PTR_cmd->y_block_size;
    int z_block_size = PTR_cmd->z_block_size;

    sum[0] = 0.0;
    for (i = 1; i <= x_block_size; i++)
        for (j = 1; j <= y_block_size; j++)
            for (k = 1; k <= z_block_size; k++)
                sum[0] += IN(bp,var,i,j,k);

    s32 _idep, _paramc, _depc;

    int istart = var;
    int r = CHECKSUM_RED_HANDLE_LB + (istart)%2;
    redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];

    ocrEVT_t redUpIEVT = PTR_redObjects->upIEVT;

    ocrDbRelease( DBK_gridSum_in );
    ocrEventSatisfy( redUpIEVT, DBK_gridSum_in ); //All blocks provide partial sums

    int phase = var;
    reducePRM_t reducePRM = {-1, ts, phase, r};
    ocrGuid_t reduceAllUpEDT, reduceAllUpOEVT, reduceAllUpOEVTS;

    ocrEdtCreate( &reduceAllUpEDT, PTR_rankTemplateH->TML_reduceAllUp, //FNC_reduceAllUp
                  EDT_PARAM_DEF, (u64*) &reducePRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, &reduceAllUpOEVT );
    createEventHelper(&reduceAllUpOEVTS, 1);
    ocrAddDependence( reduceAllUpOEVT, reduceAllUpOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, reduceAllUpEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( redUpIEVT, reduceAllUpEDT, _idep++, DB_MODE_RW );

    DEBUG_PRINTF(("CHECKSUM block number %d level %d stage %d ts %d var %d sum %f\n", PTR_rankH->myRank, bp->level, istage, ts, var, *sum));

    return *sum;
}

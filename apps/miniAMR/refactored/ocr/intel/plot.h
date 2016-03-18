// ************************************************************************
//
// miniAMR: stencil computations with boundary exchange and AMR.
//
// Copyright (2014) Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
// retains certain rights in this software.
//
// Portions Copyright (2016) Intel Corporation.
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

#ifndef __PLOT_H__
#define __PLOT_H__

typedef struct Plot_t Plot_t;

#include "dbcommheader.h"
#include "control.h"

typedef struct Plot_Annex_t {
   unsigned int level:4;
   unsigned int cenx:20;
   unsigned int ceny:20;
   unsigned int cenz:20;
} Plot_Annex_t;

typedef struct Plot_t {
   DbCommHeader_t  dbCommHeader;
   int numBlocks;
   int ts;
   Plot_Annex_t annex[0];
} Plot_t;

#define sizeof_Plot_t(numBlocks) (sizeof(Plot_t) + sizeof(Plot_Annex_t) * numBlocks)

#endif // __PLOT_H__

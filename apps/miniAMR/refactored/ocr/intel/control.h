// TODO: FIXME:  insert copyright notice

#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <ocr.h>

// These are the "control variables" that are parsed from the command line or otherwise derived therefrom.  When
// passed in to the EDT that processes a block ("DriveBlock_Func"), the contents of this datablock are READ-ONLY,
// and it is shared among all instances of that EDT.

typedef struct {
   ocrGuid_t driveBlock_Template;
   int x_block_size;
   int y_block_size;
   int z_block_size;
   int npx;
   int npy;
   int npz;
   int num_refine;
   int block_change;
   int uniform_refine;
   int refine_freq;
   int num_vars;
   int comm_vars;
   int num_tsteps;
   int stages_per_ts;
   int checksum_freq;
   int stencil;
   int error_tol;
   int report_diffusion;
   int report_perf;
   int plot_freq;
   int code;
   int permute;
   int refine_ghost;
   int num_objects;
} Control_t;

#define sizeof_Control_t (sizeof(Control_t))

#endif // __CONTROL_H__

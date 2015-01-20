#include "Stencil1D.h"

ocrGuid_t mainEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {

    // Create a new graph context
    Stencil1DCtx *context = Stencil1D_create();

    // Set up arguments for new graph instantiation
    Stencil1DArgs args = {
        /* DONE: initialize custom arguments
         * Note that you should define the members of
         * this struct by editing Stencil1D_defs.h.
         */
    };

    // DONE: initialize graph context parameters
    // int NX;
    // int NITER;
    context->NX = atoi(OCR_MAIN_ARGV(1));
    context->NITER = atoi(OCR_MAIN_ARGV(2));

    // Launch the graph for execution
    Stencil1D_launch(&args, context);

    // Exit when the graph execution completes
    CNC_SHUTDOWN_ON_FINISH(context);

    return NULL_GUID;
}

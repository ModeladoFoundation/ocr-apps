#include "Stencil1D_context.h"

int updateStep_step::execute( const updateStep_tag_t & tag, Stencil1D_context & ctxt ) const
{
    Stencil1D_context * ctx = &ctxt;
    const cnc_tag_t v = tag.v;const cnc_tag_t t = tag.t;

    int NX( ctxt.NX );
   int NITER( ctxt.NITER );


    // Get input items

    float  center_handle;

    ctxt.val.get( val_key_t(v, t ), center_handle );


    float  left_handle;

    ctxt.val.get( val_key_t(v - 1, t ), left_handle );


    float  right_handle;

    ctxt.val.get( val_key_t(v + 1, t ), right_handle );


    // DONE: here goes your computation
    // creating output handles and assign values

    float  new_handle /* = TODO: computation/initialization */;
    new_handle = center_handle + 0.5 * ( right_handle - left_handle );



    // TODO: create output

    ctxt.val.put( val_key_t(v, t + 1 ), new_handle );

    return CnC::CNC_Success;
}

/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/
#ifndef _CNC_STENCIL1D_H_INCLUDED_
#define _CNC_STENCIL1D_H_INCLUDED_

#ifdef _DIST_
# include <cnc/dist_cnc.h>
#else // _DIST_
# include <cnc/cnc.h>
#endif // _DIST_
# include <cnc/debug.h>

struct Stencil1D_context;

/*********************************************************************************\
 * tag/step combo collection, needed as long cncocr doesn't support tag-collections
\*********************************************************************************/
template< typename Tag, typename Step >
class tagged_step_collection : public CnC::tag_collection< Tag >, public CnC::step_collection< Step >
{
public:
    template< typename Ctxt >
    tagged_step_collection( Ctxt & ctxt, const std::string & name )
      : CnC::tag_collection< Tag >( ctxt, name ), CnC::step_collection< Step >( ctxt, name )
    {
        this->prescribes( *this, ctxt );
    }
};


/*********************************************************************************\
 * type of tag-components
\*********************************************************************************/
typedef int cnc_tag_t;


/*********************************************************************************\
 * tag/key types for items
\*********************************************************************************/
// *********************************************************
// item-collection val

// *********************************************************
// item-collection val
typedef struct val_key {
    val_key( const cnc_tag_t v_=-1, const cnc_tag_t t_=-1 ) : v( v_ ), t( t_ ) {}
    cnc_tag_t v, t;
    bool operator==(const val_key & o) const {
        return v == o.v && t == o.t;
    }
} val_key_t;

template <> struct cnc_hash< val_key_t >
{ size_t operator()(const val_key_t& tt) const {
    return ( (tt.v) + (tt.t << ((3*2)-1)) );
} };

#ifdef _DIST_
CNC_BITWISE_SERIALIZABLE( val_key_t );
#endif



/*********************************************************************************\
 * types of control-tags
 * and step declarations
\*********************************************************************************/
// *********************************************************
// Step updateStep

// *********************************************************
// item-collection updateStep
typedef struct updateStep_tag {
    updateStep_tag( const cnc_tag_t v_=-1, const cnc_tag_t t_=-1 ) : v( v_ ), t( t_ ) {}
    cnc_tag_t v, t;
    bool operator==(const updateStep_tag & o) const {
        return v == o.v && t == o.t;
    }
} updateStep_tag_t;

template <> struct cnc_hash< updateStep_tag_t >
{ size_t operator()(const updateStep_tag_t& tt) const {
    return ( (tt.v) + (tt.t << ((3*2)-1)) );
} };

#ifdef _DIST_
CNC_BITWISE_SERIALIZABLE( updateStep_tag_t );
#endif

struct updateStep_step { int execute( const updateStep_tag_t &, Stencil1D_context & ) const; };



/*********************************************************************************\
 * CnC context/graph
\*********************************************************************************/
struct Stencil1D_context : public CnC::context< Stencil1D_context >
{
    // tag/step-collections
    tagged_step_collection< updateStep_tag_t, updateStep_step > updateStep;


    // item-collections
    CnC::item_collection< val_key_t, float  > val;

    // other parameters
    int NX;
    int NITER;

    // constructor
    Stencil1D_context( int _NX = -1, int _NITER = -1 ) :
        // initialize tag/step-collections
        updateStep( *this, "updateStep" ),
        // initialize item-collections
         val( *this, "val" )
        // initialize other parameters
        , NX( _NX )
   , NITER( _NITER )

    {
        // data and control relationships
        // updateStep
        updateStep.consumes( val );
        updateStep.consumes( val );
        updateStep.consumes( val );
        updateStep.produces( val );
        // CnC::debug::trace_all( *this );
    }

#ifdef _DIST_
    virtual void serialize( CnC::serializer & ser )
    {
        ser & NX & NITER;
    }
#endif
 };

#endif // _CNC_STENCIL1D_H_INCLUDED_

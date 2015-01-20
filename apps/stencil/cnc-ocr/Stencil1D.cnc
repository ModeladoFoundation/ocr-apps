////////////////////////////////////////////////////////////////////////////////
// Author: Gabriele Jost (gabriele.jost@intel.cnc)
////////////////////////////////////////////////////////////////////////////////
// simple stencil update: u(i, t + 1) = u (i, t) + 0.5 * ( u(i-1, t) + u(i+1, t))
// to update the valu in each iteration we need the value of the previous iteration and  left and right (west, east)
//of the previous iteration
// Output of the program is a 1D sequence af values after NITER time steps. The intermediate values are not needed
// Number of values NX·
////////////////////////////////////////////////////////////////////////////////
// context declaraion to hold NX, NITER
$context {
    int NX;
    int NITER;
};


// item collection declarations
[ float *val : v, t ];  // val, time t


// update step at iteration t produces NX values at iteration t + 1
( updateStep: v, t )
    <- [ center @ val: v, t ],
       [ left @ val: v - 1, t ],
       [ right @ val: v + 1, t]
    -> [ new @ val: v, t + 1 ];


// Write graph inputs and start steps
( $init: () )
    -> [ val: $range(1, #NX-1), 0 ],
       [ val: 0, $rangeTo(#NITER) ],
       [ val: #NX-1, $rangeTo(#NITER) ],
       ( updateStep: $range(1, #NX-1), $range(0, #NITER) );


( $finalize: ())
    <- [ results @ val: $range(#NX), #NITER ];

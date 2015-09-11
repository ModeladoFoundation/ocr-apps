/******************************************************************************
// * CnC version of HPGMG
// *
// * Authors:
// *   v.CnC: Ellen Porter (ellen.porter@pnnl.gov)
// *          Luke Rodriguez (luke.rodriguez@pnnl.gov)
// *          Pacific Northwest National Lab
// *   HPGMG: Samuel Williams (SWWilliams@lbl.gov)
// *          Lawrence Berkeley National Lab
// *
//****************************************************************************/


/******************************************************************************
// * Graph parameters */
$context {

    int num_blocks;      // per dimension at level 0
    int num_cells;       // per dimension at level 0
    int num_levels;
    int num_cycles;
    int num_smooth_cycles;

    int restrict_blocks[NUM_LEVELS];
    int num_blocks_per_level[NUM_LEVELS]; // for each level, per dimension
    int num_cells_per_level [NUM_LEVELS];
    double dominant_eigenvalue_of_DinvA[NUM_LEVELS];

};

/******************************************************************************
//* Item collection declarations */

// produced by initialize, these do not change through execution
[ double* beta_x : level, i, j, k ]; // x face centered coefficient
[ double* beta_y : level, i, j, k ]; // y face centered coefficient
[ double* beta_z : level, i, j, k ]; // z face centered coefficient
[ double* dinv   : level, i, j, k ]; // cell-centered relaxation parameter
[ double* alpha  : level, i, j, k ]; // cell-centered coefficient
[ double* u_true : level, i, j, k ]; // exact solution used to generate f

// produced by residual
[ double* f      : cycle, level, i, j, k ]; // right hand side of equation

// produced by smooth, exchange boundaries, apply bc, restrict_grid, interpolate, solve
[ double* u      : id, cycle, segment, level, i, j, k ];

/******************************************************************************
//* HPGMG CnC steps */
( exchange_boundaries_apply_bc : u_id, cycle, segment, level, i, j, k )
<- [ u_in_13  @ u : u_id*2+1, cycle, segment, level, i,   j,   k   ],
   [ u_in_4   @ u : u_id*2+1, cycle, segment, level, i  , j  , k-1 ] $when(                                           k>0             ),
   [ u_in_10  @ u : u_id*2+1, cycle, segment, level, i  , j-1, k   ] $when(                      j>0                                  ),
   [ u_in_12  @ u : u_id*2+1, cycle, segment, level, i-1, j  , k   ] $when( i>0                                                       ),
   [ u_in_14  @ u : u_id*2+1, cycle, segment, level, i+1, j  , k   ] $when( i<#num_blocks-1                                           ),
   [ u_in_16  @ u : u_id*2+1, cycle, segment, level, i  , j+1, k   ] $when(                      j<#num_blocks-1                      ),
   [ u_in_22  @ u : u_id*2+1, cycle, segment, level, i  , j  , k+1 ] $when(                                           k<#num_blocks-1 )
-> [ u_out    @ u : u_id*2+2, cycle, segment, level, i  , j  , k   ];

( smooth : u_id, cycle, segment, level, i, j, k )
<- [ u_in_0 @ u :             u_id*2  , cycle  , segment, level, i, j, k ] $when(u_id != 0),
   [ u_in_1 @ u :             u_id*2+2, cycle  , segment, level, i, j, k ],
   [ beta_x_in @ beta_x :                                 level, i, j, k ],
   [ beta_y_in @ beta_y :                                 level, i, j, k ],
   [ beta_z_in @ beta_z :                                 level, i, j, k ],
   [ f_in      @ f      :               cycle  ,          level, i, j, k ] $when(level != 0),
   [ f_in_init @ f      :               0      ,          0    , i, j, k ] $when(level == 0),
   [ dinv_in   @ dinv   :                                 level, i, j, k ]
-> [ u_out  @ u :             u_id*2+3, cycle  , segment, level, i, j, k ] $when(level != 0 || segment != 1 || u_id != #num_smooth_cycles - 1),
   [ u_out_next_segment @ u : 1       , cycle+1, 0      , level, i, j, k ] $when(level == 0 && segment == 1 && u_id == #num_smooth_cycles - 1);

( compute_residaul  : cycle, level, i, j, k )
<-  [ u_in      @ u : #num_smooth_cycles*2+2, cycle, 0, level, i, j, k ],
    [ f_in      @ f :                         cycle,    level, i, j, k ] $when(level != 0),
    [ f_in_init @ f :                         0    ,    level, i, j, k ] $when(level == 0),
    [ beta_x_in @ beta_x :                              level, i, j, k ],
    [ beta_y_in @ beta_y :                              level, i, j, k ],
    [ beta_z_in @ beta_z :                              level, i, j, k ]
->  [ u_out     @ u : 0                    ,  cycle, 1, level, i, j, k ];

( iterative_solve       : cycle )
<- [ f_in @ f           :                         cycle,    #num_levels-1, $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ beta_x_in @ beta_x :                                   #num_levels-1, $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ beta_y_in @ beta_y :                                   #num_levels-1, $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ beta_z_in @ beta_z :                                   #num_levels-1, $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ dinv_in @ dinv     :                                   #num_levels-1, $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ]
-> [ u_out @ u          : #num_smooth_cycles*2+1, cycle, 1, #num_levels-1, $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ];

( restrict_grid       : cycle, level, i, j, k )
<-  [ u_in_0 @ u : 0, cycle, 1, level  , i  , j  , k   ],
    [ u_in_1 @ u : 0, cycle, 1, level  , i+1, j  , k   ] $when( i<#num_blocks-1                                           ),
    [ u_in_2 @ u : 0, cycle, 1, level  , i  , j+1, k   ] $when(                      j<#num_blocks-1                      ),
    [ u_in_3 @ u : 0, cycle, 1, level  , i+1, j+1, k   ] $when( i<#num_blocks-1  &&  j<#num_blocks-1                      ),
    [ u_in_4 @ u : 0, cycle, 1, level  , i  , j  , k+1 ] $when(                                           k<#num_blocks-1 ),
    [ u_in_5 @ u : 0, cycle, 1, level  , i+1, j  , k+1 ] $when( i<#num_blocks-1  &&                       k<#num_blocks-1 ),
    [ u_in_6 @ u : 0, cycle, 1, level  , i  , j+1, k+1 ] $when(                      j<#num_blocks-1  &&  k<#num_blocks-1 ),
    [ u_in_7 @ u : 0, cycle, 1, level  , i+1, j+1, k+1 ] $when( i<#num_blocks-1  &&  j<#num_blocks-1  &&  k<#num_blocks-1 )
->  [ f_out  @ f :    cycle,    level+1, i, j, k ];

( interpolate    : cycle, level, i, j, k )
<-  [ u_in_previous_level_0 @ u : #num_smooth_cycles*2+1, cycle, 1, level+1, i, j, k ],
    [ u_in_previous_level_1 @ u : #num_smooth_cycles*2+1, cycle, 1, level+1, i-i%2, j-j%2, k-k%2 ],
    [ u_in_previous_segment @ u : #num_smooth_cycles*2+2, cycle, 0, level  , i  , j  , k   ]
->  [ u_out                 @ u : 1,                      cycle, 1, level  , i  ,   j, k   ];


/******************************************************************************
//* Input output relationships from environment */

( $initialize: () )
   // instance, cycle, segment, level, block i, block j, block k
-> [ beta_x :                               $range(0, #num_levels)  , $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ beta_y :                               $range(0, #num_levels)  , $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ beta_z :                               $range(0, #num_levels)  , $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ dinv   :                               $range(0, #num_levels)  , $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ alpha  :                               $range(0, #num_levels)  , $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ u_true :                               $range(0, #num_levels)  , $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ f      :    0                     ,    0                       , $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ u      : 1, 0                     , 0, 0                       , $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ u      : 1, $range(0, #num_cycles), 0, $range(1, #num_levels-1), $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],

   (exchange_boundaries_apply_bc : $range(0, #num_smooth_cycles+1), $range(0, #num_cycles), 0           , $range(0, #num_levels-1), $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks)),
   (exchange_boundaries_apply_bc : $range(0, #num_smooth_cycles  ), $range(0, #num_cycles), 1           , $range(0, #num_levels-1), $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks)),
   (smooth                       : $range(0, #num_smooth_cycles  ), $range(0, #num_cycles), $range(0, 2), $range(0, #num_levels-1), $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks)),
   (compute_residaul             :                                  $range(0, #num_cycles),               $range(0, #num_levels-1), $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks)),
   (restrict_grid                     :                                  $range(0, #num_cycles),               $range(0, #num_levels-1), $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks)),
   (interpolate                  :                                  $range(0, #num_cycles),               $range(0, #num_levels-1), $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks)),
   (iterative_solve              :                                  $range(0, #num_cycles));

( $finalize: () )
<- [ u      : 1, #num_cycles, 0, 0, $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ beta_x :                    0, $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ beta_y :                    0, $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ beta_z :                    0, $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ dinv   :                    0, $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ],
   [ f      :    0          ,    0, $range(0, #num_blocks), $range(0, #num_blocks), $range(0, #num_blocks) ];
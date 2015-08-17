
$context {
    u32 dim; // board side dimension (or number of queens)
    u32 n;   // desired number of solutions
};

// cheat and keep a global solutions count
[ u64 solutions[4]: i ];

// board is 4 8x8 sections (so max of 16x16)
// arranged like this:
// b00 b01
// b10 b11
// so to get square r=9, c=3, you check r=1, c=3 of b10
( placeQueen: row, nw, ne, sw, se )
 -> ( placeQueen: 0, 0, 0, 0, 0 ),
    [ solutions: i ];

( $initialize: () )
 -> ( placeQueen: 0, 0, 0, 0, 0 );

( $finalize: () )
 <- [ solutions: $range(#n) ];


[ void *done: () ];

( stoker: i, j )
 -> ( quencher: i, j, $rangeTo(Y) ),
    ( stoker: i, j+1 ) $when(j<X);

( quencher: i, j, k )
 -> [ done: () ] $when(i==0 && j==X && k==Y);

( $initialize: () )
 -> ( stoker: $range(Z), 0 );

( $finalize: () )
 <- [ done: () ];

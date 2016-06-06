// Raw program with the __data keyword
// and __attribute__ that specifies where the datablock can be placed
// Similarly quickSort is decorated with __task keyword
#include <stdio.h>
#include <stdlib.h>

#define LEN 128

int partition( int a[], int l, int r) {
    int pivot, i, j, t;

    pivot = a[l];
    i = l;
    j = r+1;

    while( 1 ) {
        do ++i;
        while( a[i] <= pivot && i <= r );
        do --j;
        while( a[j] > pivot );
        if( i >= j )
            break;
        t = a[i];
        a[i] = a[j];
        a[j] = t;
    }
    t = a[l];
    a[l] = a[j];
    a[j] = t;

    return j;
}

void quickSort( int a[], int l, int r)
{
    int j;

    if( l < r ) {
        // divide and conquer
        j = partition( a, l, r);
        quickSort( a, l, j-1);
        quickSort( a, j+1, r);
    }
}

int main()
{
    int a[LEN];

    int i;
    for(i = 0; i<LEN; i++)
        a[i] = rand();

    printf("Before\n");
    for ( i = 0; i < LEN; i++ )
        printf("%02x ", a[i]);
    printf("\n");

    quickSort( a, 0, LEN-1);

    printf("After\n");
    for( i = 0; i < LEN; i++ )
        printf("%02x ", a[i]);
    printf("\n");

    return 0;
}

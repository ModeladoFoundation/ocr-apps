#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "ocr.h"

//
// When testing and we want to avoid stdio mallocs/frees
//
#ifdef INTERNAL_NAMES
#define malloc(n) mALLOc(n)
#define calloc(n) cALLOc(n)
#define free(a) fREe(a)
#endif // INTERNAL_NAMES

struct mallinfo
{
  int arena;    /* non-mmapped space allocated from system */
  int ordblks;  /* number of free chunks */
  int smblks;   /* number of fastbin blocks */
  int hblks;    /* number of mmapped regions */
  int hblkhd;   /* space in mmapped regions */
  int usmblks;  /* maximum total allocated space */
  int fsmblks;  /* space available in freed fastbin blocks */
  int uordblks; /* total allocated space */
  int fordblks; /* total free space */
  int keepcost; /* top-most, releasable (via malloc_trim) space */
};

struct mallstats {
    uint64_t max_system_size;
    uint64_t max_regions;
    uint64_t max_region_size;
    uint64_t heaps;
};

extern void  mall_stats( struct mallstats *ms );
extern int   mallarinfo( uint64_t arenaNo, struct mallinfo *mi );

extern void *malloc( size_t );
extern void *calloc( size_t );
extern void  free( void * );
extern void *realloc( void *, size_t );
extern void *memalign( size_t, size_t );
extern void  dump_arenas();

#define printf( ... )
static void print_arena_info( uint64_t arenaNo, struct mallinfo *mi )
{
    printf("Arena %d: %d b, %d fc, %d sb, %d hb, %d hbb, %d ub, %d all, %d free\n",
            arenaNo, mi->arena, mi->ordblks,
            mi->smblks, mi->hblks, mi->hblkhd,
            mi->usmblks, mi->uordblks, mi->fordblks );
}

#if MALLOC_DEBUG > 0
static void print_info( void )
{
    struct mallinfo mi;

    for( int i = 0 ; mallarinfo( i, & mi ) == 0 ; i++ )
        print_arena_info( i, & mi );

    struct mallstats ms;

    mall_stats( & ms );
    printf("Totals: %d max size, %d max regions, %d max_region size, %d heaps\n",
        ms.max_system_size, ms.max_regions, ms.max_region_size, ms.heaps );
}
#else

#define print_info()
#define dump_arenas()

#endif

//
// size at which mmap'd chunks should be created
//
#define DEFAULT_MMAP_THRESHOLD (128 * 1024)

int
main( int argc, char **argv )
{
    void * allocations[100];
    int nallocs = 0;

    {
        printf("**** Alloc 1\n");
        void * a = malloc( 1 );
        print_info();
        printf("**** Alloc 2\n");
        void * b = malloc( DEFAULT_MMAP_THRESHOLD );
        print_info();
        printf("**** Free 2\n");
        free( b );
        print_info();
        printf("**** Free 1\n");
        free( a );
        print_info();
        // return 0;
    }
    dump_arenas();

    memset( allocations, 0, sizeof(allocations) );

    printf("*** Allocating ***\n");
    for( size_t i = 1 ; i < 1024 * 1024 ; i <<= 1 ) {
        void * a = malloc( i );
        allocations[nallocs++] = a;
        printf("\n%d: malloc 0x%08x : 0x%016llx\n", nallocs-1, i, a );
        dump_arenas();
        print_info();
    }
    printf("*** Freeing ***\n");
    for( int i = 0 ; i < nallocs ; i++ ) {
        printf("\n%d: free 0x%016llx\n", i, allocations[i] );
        dump_arenas();
        free( allocations[i] );
        print_info();
    }
    printf("*** Final State ***\n");
    dump_arenas();
    return 0;
}

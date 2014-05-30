#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "pil_mem.h"
#include "pil.h"

#ifdef PIL2AFL
#include <unistd.h>
#include "codelet.h"
#include "rmd_afl_all.h"
#endif /* PIL2AFL */

#ifdef PIL2OCR
#include "ocr.h"
//#include "ocr-runtime.h"
#endif /* PIL2OCR */

#define PIL_MEM_DEBUG 0


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
TODO TODO TODO: this bit arithmetic assumes a 64 bit environment! TODO TODO TODO
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*
 * Allocates a block of size bytes of memory, returning the id of the new
 * memory block.
 *
 * The content of the newly allocated block of memory is not initialized,
 * remaining with indeterminate values.
 */
void pil_alloc(gpp_t *g, size_t size)
{
#if PIL_MEM_DEBUG
	printf("calling pil_alloc()");
#endif

	assert (sizeof(void *) == 8); // TODO: currently only works with 64 bit backends

#if defined PIL2C || defined PIL2SWARM
	g->ptr = malloc(size);
	assert (g->ptr != NULL);

	// check to make sure that the LSB of the pointer isn't a 1. GUIDs are negative!
	uint64_t id = (uint64_t) g->ptr;

#if PIL_MEM_DEBUG
	printf(" with C backend\n");
	printf("ptr:  %p\n", g->ptr);
	printf(" id after cast:   %8lx\n", id);
#endif

	uint64_t lsb = id & 0x1;
	if (lsb == 1)
	{
		// TODO: call sendmail to padua. This was his idea.
		printf("ERROR: allocated a memory block with lsb of 1!\n");
		exit(EXIT_FAILURE);
	}
	g->guid = (guid_t)(id | 0x1); // make the lsb be a 1
#endif /* PIL2C */

#ifdef PIL2AFL
#if PIL_MEM_DEBUG
	printf(" with AFL backend\n");
#endif
	u64 retval;
	rmd_guid_t db_guid;
	rmd_location_t loc;
	loc.type = RMD_LOC_TYPE_RELATIVE;
	loc.data.relative.level = RMD_LOCATION_BLOCK;
	retval = rmd_db_alloc(&db_guid, size, BLOCK_LOCAL, &loc);
	assert (0 == GET_STATUS(retval));
	printf("allocated data-block (%ld) for db_guid in pil_alloc\n", db_guid.data);
	//void *ptr = GET_ADDRESS(retval);
	uint64_t id = db_guid.data;
	ret_id = (void *) id;
	rmd_db_release(db_guid);
#endif /* PIL2AFL */
#ifdef PIL2OCR
	ocrDbCreate(&g->guid, (void **) &g->ptr, size, /*flags=*/DB_PROP_NONE, /*affinity=*/NULL_GUID, NO_ALLOC);
#if PIL_MEM_DEBUG
	printf("allocated data-block (%ld) for db_guid (%ld) in pil_alloc\n", g->guid, (long int)g->ptr);
#endif
	//ocrDbRelease(db_guid);
#endif /* PIL2OCR */

#if PIL_MEM_DEBUG
	printf("g->guid: %p\n", g->guid);
	printf("leaving pil_alloc()\n\n");
#endif
}

/* 
 * Return a pointer to the memory block's contents. Find the address of the
 * memory block with the corresponding id.
 */
void *pil_mem(void *id)
{
#if PIL_MEM_DEBUG
	printf("calling pil_mem()");
#endif

	void *ptr = NULL;
	// check the LSB of the id. if it is a 1, turn it into an address. else, it is an address.
	uint64_t lsb = ((uint64_t) id) & 0x1;
	if (lsb != 1)
	{
#if 0
		uint64_t check_bits = (((uint64_t) id) & 0x00007F0000000000);
		//printf("check_bits: %lx\n", check_bits);
		if (check_bits != 0x00007F0000000000) // TODO: this is a bad bad hack
		{
			printf("p: %lx\n", (uint64_t)id);
			printf("ERROR: called pil_mem with not an id: %p\n", id);
			//exit(EXIT_FAILURE);
		}
#endif
		return id;
	}

#if defined PIL2C || defined PIL2SWARM
	uint64_t int_id = (uint64_t)id;
	uint64_t real_id = int_id & 0xFFFFFFFFFFFFFFFE;
	ptr = (void *) real_id;

#if PIL_MEM_DEBUG
	printf(" with C backend\n");
	printf("id: %p\n", id);
	printf("int_id: %lx\n", int_id);
	printf("real_id: %lx\n", real_id);
	fflush(stdout);
#endif

#endif /* PIL2C */

#ifdef PIL2AFL
	rmd_guid_t guid;
	guid.data = (u64) id;
	ptr = RMD_DB_ACQUIRE(guid, 0);
	//rmd_db_mem((void **)&ptr, guid);
#endif /* PIL2AFL */

#ifdef PIL2OCR
#if 0
	ocrGuid_t guid;
	guid = (u64) id;
	ocrDbAcquire(guid, (void **) &ptr, /*flags=*/0xdead);
#endif
#endif /* PIL2OCR */

	return ptr;
}

/*
 * when you access memory, you register the data block to that codelet. you
 * have to release a datablock before you can free it.
 */
void pil_release(void *id)
{
#ifdef PIL2AFL
	rmd_guid_t guid;
	guid.data = (u64) id;
	printf("releasing data-block (%ld) in pil_release\n", guid.data);
	assert(rmd_db_release(guid) == 0);
#endif /* PIL2AFL */
#ifdef PIL2OCR
	ocrGuid_t guid;
	guid = (u64) id;
	printf("releasing data-block (%ld) in pil_release\n", guid);
	fprintf(stderr, "WARNING: PIL implemented hack for pil_releae to avoid OCR bug. Not releasing.\n");
	//ocrDbRelease(guid);
#endif /* PIL2OCR */
}

/*
 * Deallocate space in memory A block of memory previously allocated using a
 * call to pil_alloc is deallocated, making it available again for further
 * allocations.
 *
 * Notice that this function leaves the value of id unchanged, hence it still
 * refers to the same (now invalid) memory block.
*/
void pil_free(gpp_t g)
{
#if PIL_MEM_DEBUG
	printf("calling pil_free()\n");
#endif

#if defined PIL2C || defined PIL2SWARM
	void *ptr = g.ptr;

#if PIL_MEM_DEBUG
	printf("ptr: %p\n", ptr);
#endif

	free(ptr);
#endif /* PIL2C */

#ifdef PIL2AFL
	rmd_guid_t guid;
	guid.data = (u64) id;
	printf("freeing data-block (%ld) in pil_free\n", guid.data);
	rmd_db_free(guid);
#endif /* PIL2AFL */

#ifdef PIL2OCR
	//printf("freeing data-block (%ld) in pil_free\n", guid);
/*
	fprintf(stderr, "WARNING: PIL implemented hack for pil_free to avoid OCR bug. Not freeing\n");
	ocrDbDestroy(g.guid);
*/
	g.ptr = NULL;
#endif /* PIL2OCR */
}

void *pil_realloc(void *ptr, size_t newsize)
{
	printf("ERROR: pil_realloc not implemented!\n");
	exit(EXIT_FAILURE);
	return NULL;
}

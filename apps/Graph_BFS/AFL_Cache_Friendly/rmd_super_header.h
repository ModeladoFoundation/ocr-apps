#include "db_alloc_public.h"
#include "db_alloc_pthrhack.h"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"

const int nCEs = 0;
const int nXEs = 8;
const int nBlocks = 1;
const int nChips = 1;

#define engineHeap 32768
#define blockHeap 524288
#define DRAM 268435456

#define ANY LOCAL | BLOCK_LOCAL | ON_CHIP | OFF_CHIP

typedef struct db_node {
	data_block_t db;
	void *ptr;
	struct db_node *next;
	struct db_node *prev;
} db_node;

db_node *db_head = NULL;
data_block_t memory_pool;


uint64_t locate(size_t size);
void* db_quick_malloc(size_t size);
void* db_malloc(size_t size, uint64_t alloc_flag, agentid_t location_id);
void db_free_one(void *ptr); //horrible naming
void db_free_all();

//really, what with locate(size_t size) and the fact that I currently only use
//COREID_SELF, I could outright only take a size_t value and have the same signature as malloc
void* db_malloc(size_t size, uint64_t alloc_flag, agentid_t location_id)
{
	//printf("size: %luK\n", size/1000);
	//allocation
	void *ret;
	data_block_t block;
	DB_ALLOC(&block, size + sizeof(db_node), alloc_flag, location_id);
	DB_MEM(&ret, block);
	ret = (char *) ret + sizeof(db_node);

	//bookkeeping
	db_node *meta;		
	meta = ret - sizeof(db_node);
	meta->db = block;
	meta->ptr = ret;
	meta->next = db_head;
	meta->prev = NULL;
	db_head = meta;

	return (void *) ret;
}

//calls locate to judge location
void* db_quick_malloc(size_t size)
{
	return db_malloc(size, locate(size), COREID_SELF);
}

//What I would like to do is call locate from within db_malloc and db_free
//and have it make the allocation decisions for me, with the ability to force a decision.
//I will do this after I finish my naive version.
//need real bookkeeping
int global_total_local = engineHeap; 
int global_total_block = blockHeap; 

uint64_t locate(size_t size)
{

	uint64_t mode;
/*
	if(size < engineHeap && size > global_total_local)
	{
		mode = LOCAL;
		global_total_local -= size;
	}
	//if(size < engineHeap / numThreads) // a hard (but very important) one
	else if(size < blockHeap && size < global_total_block) 
	{
		mode = BLOCK_LOCAL;
		global_total_block -= size;
	}
	//and others whose work/reward ratio depends significantly on market price
	//else if(size < blockHeap*nBlocks)
	//	mode = ON_CHIP;
	else
		//mode = ANY;
*/
		mode = OFF_CHIP;

	return mode;
}

//add size back in if bookkeeping ever created
void db_free_one(void *ptr)
{
	//currently untested, but works in use
	db_node *meta = ptr - sizeof(db_node);
	if(meta->prev != NULL) meta->prev->next = meta->next;
	if(meta->next != NULL) meta->next->prev = meta->prev;
	if(meta == db_head) db_head = db_head->next;
	DB_RELEASE(meta->db);
	DB_FREE(meta->db);
}

void db_free_all()
{
	while(db_head != NULL)
	{
		db_node *temp = db_head;
		db_head = db_head->next;
		DB_RELEASE(temp->db);
		DB_FREE(temp->db);
	}
}

agentid_t find_core_id(int thread_id)
{
	return gen_core_id(thread_id % nChips, thread_id % nBlocks, thread_id % (nXEs + nCEs));
}

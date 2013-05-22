//*************************************************************************
//-------------------------------------------------------------------------
//
//         ---- The Unbalanced Tree Search (UTS) Benchmark ----
//  
//  Copyright (c) 2010 See AUTHORS file for copyright holders
//
//  This file is part of the unbalanced tree search benchmark.  This
//  project is licensed under the MIT Open Source license.  See the LICENSE
//  file for copyright and licensing information.
//
//  UTS is a collaborative project between researchers at the University of
//  Maryland, the University of North Carolina at Chapel Hill, and the Ohio
//  State University.  See AUTHORS file for more information.
//-------------------------------------------------------------------------
//*************************************************************************

//**********************************************************
// Recursive depth-first implementation                    *
//**********************************************************

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef RAG_SIM
#include <string.h>

#include "xe-codelet.h"
#include "xe-memory.h"
#include "xe-console.h"
#include "xe-global.h"
#endif

#ifdef RAG_AFL
#include <string.h>

#include "codelet.h"
#include "rmd_afl_all.h"

#define xe_printf printf
#endif

#ifndef RMD_DB_MEM
#define RMD_DB_MEM(x,y) \
do { \
	u64 __retval__; \
	int __err__; \
	__retval__ = rmd_db_acquire((y),(0)); \
	__err__ = GET_STATUS(__retval__);        \
	if (__err__ != 0) { \
		xe_printf("db_acquire ERROR arg=%ld (%s) %s:%d\n", y.data, strerror(__err__), __FILE__, __LINE__); \
		exit(__err__); \
	} \
	__err__ = GET_ACCESS_MODE(__retval__); \
	*(x) = (void*)GET_ADDRESS(__retval__);  \
} while(0)
#endif

rmd_guid_t NULL_GUID = { .data = 0 };
#ifdef RAG_BLOCK
rmd_location_t    allocLoc = {	.type                     = RMD_LOC_TYPE_RELATIVE,
        			.data.relative.identifier = 0,
				.data.relative.level      = RMD_LOCATION_BLOCK   };
#else
rmd_location_t    allocLoc = {	.type                     = RMD_LOC_TYPE_RELATIVE,
        			.data.relative.identifier = 0,
				.data.relative.level      = RMD_LOCATION_CORE    };
#endif
#ifndef LOCAL
const int LOCAL = 0;
#endif

#define ALLOC_LOC (&allocLoc)


#ifdef RAG_SIM
#ifdef assert
#undef assert
#endif
#ifndef DEBUG
#define assert(x)
#else
#define assert(x) if(!(x)) {xe_printf("ASSERT () failed %s:%d\n",__FILE__, __LINE__); exit(x); }
#endif
#else
#include <assert.h>
#endif

#include "uts.h"

#define GET_NUM_THREADS  (1)

// _clg => codelet guid
// _scg => scheduled codelet guid
// _dbg => data block guid
// _dbp => data block pointer
// _lcl => on stack data

//**********************************************************
//        codelet Parallel Recursive Search Exection       *
//**********************************************************

//**********************************************************
//  Global state                                           *
//**********************************************************

counter_t nNodes  = 0;
counter_t nLeaves = 0;
counter_t maxTreeDepth = 0;

typedef struct {
  counter_t maxdepth, size, leaves;
} Result;

#ifdef RAG_AFL
#define REM_LDX_ADDR(out_var,in_ptr,type) memcpy(&out_var,in_ptr,sizeof(type))
#define REM_STX_ADDR(out_ptr,in_var,type) memcpy(out_ptr,&in_var,sizeof(type))
#define REM_ST64_ADDR(out_ptr,in_var) memcpy(out_ptr,&in_var,8*sizeof(char))
#endif

void parTreeSearch(rmd_guid_t main_post_scg, rmd_guid_t pTS_body_clg, int depth, Node *parent);

//-----------------------------------------------------------//

rmd_guid_t main_pre_clg,   main_pre_scg;
rmd_guid_t pTS_return_clg, pTS_return_scg;

typedef struct    main_pre_param_struct   {
			int argc; char **argv; } main_pre_param_t;

typedef struct    main_post_param0_struct { 
			double t1; } main_post_param0_t;

typedef struct    main_post_param1_struct {
			Result r;  } main_post_param1_t;

typedef struct    pTS_body_async_param_struct   {
			rmd_guid_t main_post_scg; rmd_guid_t pTS_body_clg; int slot; Node parent; } pTS_body_async_param_t;

rmd_guid_t pTS_body_async_codelet   (uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db);

typedef struct    pTS_finish_return_param_struct {
			rmd_guid_t main_post_scg; int slot; Result r; } pTS_finish_return_param_t;

rmd_guid_t pTS_finish_codelet( uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db);

rmd_guid_t pTS_return_codelet( uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db);

//----------------------------------------------------------------------//

rmd_guid_t pTS_return_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
#ifdef TRACE
xe_printf("enter return_codelet arg %ld n_db %d db[0] %ld db_ptr[0] %ld\n",arg,n_db,db[0].data,(uint64_t)db_ptr[0]);
#endif
	int retval=-1;
	assert(n_db == 1);
	pTS_finish_return_param_t pTS_return_arg_lcl;
	REM_LDX_ADDR(pTS_return_arg_lcl,db_ptr[0],pTS_finish_return_param_t);
	rmd_guid_t main_post_scg = pTS_return_arg_lcl.main_post_scg;
#ifdef DEBUG
xe_printf("return_codelet main_post_scg %ld\n",main_post_scg.data);
#endif
// create a db for main_post's argument
	main_post_param1_t *main_post_arg1_dbp;
	rmd_guid_t          main_post_arg1_dbg;
	RMD_DB_ALLOC(&main_post_arg1_dbg, sizeof(main_post_param1_t),  LOCAL, ALLOC_LOC);
	RMD_DB_MEM  (&main_post_arg1_dbp, main_post_arg1_dbg);
#ifdef RAG_BLOCK
	REM_STX_ADDR(main_post_arg1_dbp + offsetof(main_post_param1_t, r), pTS_return_arg_lcl.r, Result);
#else
	main_post_arg1_dbp->r    = pTS_return_arg_lcl.r;
#endif
// provide the arguments to main_post
	retval = rmd_codelet_satisfy(
				main_post_scg,		// rmd_guid_t scheduled codelet's guid
				main_post_arg1_dbg,	// rmd_guid_t db guid
				1);			// int dep_slot
#ifdef DEBUG
xe_printf("return_codelet satisfy %ld slot 1 with dbg %ld (retval=%d)\n",main_post_scg.data,main_post_arg1_dbg.data,retval);
#endif
	assert(retval==0);
	RMD_DB_FREE(db[0]);				// rmd_guid_t db guid
  	RMD_DB_RELEASE(main_post_arg1_dbg);		// rmd_guid_t db guid
#ifdef TRACE
xe_printf("leave return_codelet\n");
#endif
	return NULL_GUID;
}

rmd_guid_t pTS_finish_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)
{
	int retval=-1;
#ifdef TRACE
xe_printf("enter finish_codelet arg %ld n_db %d (more)\n",arg,n_db);
#endif
#ifdef DEBUG
xe_printf("enter finish_codelet line 1 db[0] = %ld db_ptr[0] = %ld (cont.)\n",db[0].data,(uint64_t)db_ptr[0]);
#endif
	rmd_guid_t arg_guid = { .data = arg};
#ifdef DEBUG
xe_printf("enter finish_codelet line 2 db[1] = %ld db_ptr[1] = %ld (cont.)\n",db[1].data,(uint64_t)db_ptr[1]);
#endif
	assert(n_db > 1);
#ifdef DEBUG
xe_printf("finish_codelet line 3\n");
#endif
	int numChildren = n_db-1;	
#ifdef DEBUG
xe_printf("finish_codelet line 4 numChildren %d\n",numChildren);
#endif
 	pTS_finish_return_param_t out_lcl;
#ifdef DEBUG
xe_printf("finish_codelet line 5\n");
#endif
	REM_LDX_ADDR(out_lcl,db_ptr[numChildren],pTS_finish_return_param_t);
#ifdef DEBUG
xe_printf("finish_codelet line 6\n");
#endif
	int slot = out_lcl.slot;
#ifdef DEBUG
xe_printf("finish_codelet slot %d\n",slot);
#endif

	for(int i=0; i<numChildren; i++) {
 		pTS_finish_return_param_t in_lcl;
		REM_LDX_ADDR(in_lcl,db_ptr[i],pTS_finish_return_param_t);
		if( (out_lcl.r.maxdepth) < (in_lcl.r.maxdepth) )
			out_lcl.r.maxdepth = in_lcl.r.maxdepth;
		out_lcl.r.size       += in_lcl.r.size;
		out_lcl.r.leaves     += in_lcl.r.leaves;
		RMD_DB_FREE(db[i]);
	} // for numChildren
// prepare to return our answer 
	pTS_finish_return_param_t pTS_finish_arg_lcl, *pTS_finish_arg_dbp;
	rmd_guid_t pTS_finish_arg_dbg;
	RMD_DB_ALLOC(&pTS_finish_arg_dbg, sizeof(pTS_finish_return_param_t), LOCAL, ALLOC_LOC);
	RMD_DB_MEM  (&pTS_finish_arg_dbp, pTS_finish_arg_dbg);
	pTS_finish_arg_lcl.main_post_scg  = out_lcl.main_post_scg;
	pTS_finish_arg_lcl.slot           = out_lcl.slot;
	pTS_finish_arg_lcl.r              = out_lcl.r;
#ifdef RAG_BLOCK
	REM_STX_ADDR(pTS_finish_arg_dbp,pTS_finish_arg_lcl, pTS_finish_return_param_t);
#else
	*pTS_finish_arg_dbp       = pTS_finish_arg_lcl;
#endif
// and let our parent's completion know we're done with parTreeSearch()
	retval = rmd_codelet_satisfy(
				arg_guid,		// rmd_guid_t scheduled codelet's guid
				pTS_finish_arg_dbg,	// rmd_guid_t db guid
				slot);			// int dep_slot
#ifdef DEBUG
xe_printf("finish_codelet satisfy caller %ld slot %d with dbg %ld (retval=%d)\n",arg_guid.data,slot,pTS_finish_arg_dbg.data,retval);
#endif
	assert(retval==0);
	RMD_DB_RELEASE(pTS_finish_arg_dbg);

// The app is done with the answers from parTreeSearch()
	RMD_DB_FREE(db[numChildren]);
#ifdef TRACE
xe_printf("leave finish_codelet\n");
#endif
	return NULL_GUID;
}

rmd_guid_t pTS_body_async_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)
{
	int retval=-1;
#ifdef TRACE
xe_printf("enter body_codelet arg %ld n_db %d db[0] %ld db_ptr[0] %ld \n",arg,n_db,db[0].data,(uint64_t)db_ptr[0]);
#endif
	rmd_guid_t arg_guid = { .data = arg };
	assert(n_db == 1);
	assert(arg != 0);
	pTS_body_async_param_t pTS_body_arg_lcl;
	REM_LDX_ADDR(pTS_body_arg_lcl,db_ptr[0],pTS_body_async_param_t);
	rmd_guid_t pTS_body_clg  = pTS_body_arg_lcl.pTS_body_clg;
	rmd_guid_t main_post_scg = pTS_body_arg_lcl.main_post_scg;
#ifdef DEBUG
xe_printf("body_codelet main_post_scg %ld\n",pTS_body_arg_lcl.main_post_scg.data);
xe_printf("body_codelet pTS_body_clg  %ld\n",pTS_body_clg.data);
#endif
	int slot    = pTS_body_arg_lcl.slot;
#ifdef DEBUG
xe_printf("body_codelet slot %d\n",slot);
#endif
	Node *parent = &pTS_body_arg_lcl.parent;
	counter_t parentHeight = parent->height;
#ifdef DEBUG
xe_printf("body_codelet parentHeight %lld\n",parentHeight);
#endif
	int numChildren = uts_numChildren(parent);
	int childType   = uts_childType(parent);
	Result r = { parentHeight, 1, 0 };
//
// record number of children in parent
	parent->numChildren = numChildren;
#ifdef DEBUG
xe_printf("body_codelet numChildren %d\n",numChildren);
#endif

	rmd_guid_t pTS_finish_clg;
	rmd_guid_t pTS_finish_scg;

	if (numChildren == 0) {
#ifdef DEBUG
xe_printf("body_codelel L\n");
#endif
		pTS_finish_return_param_t pTS_leaf_arg_lcl, *pTS_leaf_arg_dbp;
		rmd_guid_t pTS_leaf_arg_dbg;
		r.leaves = 1;
		RMD_DB_ALLOC(&pTS_leaf_arg_dbg, sizeof(pTS_finish_return_param_t), LOCAL, ALLOC_LOC);
		RMD_DB_MEM  (&pTS_leaf_arg_dbp, pTS_leaf_arg_dbg);
		pTS_leaf_arg_lcl.main_post_scg  = main_post_scg;
		pTS_leaf_arg_lcl.slot   = slot;
		pTS_leaf_arg_lcl.r= r;
#ifdef RAG_BLOCK
		REM_STX_ADDR(pTS_leaf_arg_dbp, pTS_leaf_arg_lcl, pTS_finish_return_param_t);
#else
		*pTS_leaf_arg_dbp = pTS_leaf_arg_lcl;
#endif
		retval = rmd_codelet_satisfy(
				arg_guid,		// rmd_guid_t scheduled codelet's guid
				pTS_leaf_arg_dbg,	// rmd_guid_t db guid
				slot);			// int dep_slot
#ifdef DEBUG
xe_printf("body_codelel L satisfy %ld slot %d with dbg %ld (retval=%d)\n",arg_guid.data,slot,pTS_leaf_arg_dbg.data,retval);
#endif
		assert(retval==0);
		RMD_DB_RELEASE(pTS_leaf_arg_dbg);		// rmd_guid_t db guid
	} else { // numChildren != 0
#ifdef DEBUG
xe_printf("body_codelel R\n");
#endif
// create codelet for finish codelet
		retval = rmd_codelet_create(
			&pTS_finish_clg,	// rmd_guid_t *new_guid
			pTS_finish_codelet,	// rmd_codelet_ptr func_ptr
			0,			// size_t code_size
			-200,			// uinit64_t default_arg
			numChildren+1,		// int n_dep
			1,			// int buffer_in
			false,			// bool gen_out
			0);			// uint64_t prop
#ifdef DEBUG
xe_printf("body_codelel R create finish_clg %ld (retval=%d)\n",pTS_finish_clg.data,retval);
#endif
		assert(retval==0);
// create the completion codelet and pass it the slot argument as a dependency
		pTS_finish_return_param_t pTS_finish_arg_lcl, *pTS_finish_arg_dbp;
		rmd_guid_t pTS_finish_arg_dbg;
		retval = rmd_codelet_sched(
			&pTS_finish_scg,		// rmd_guid_t* scheduled codelet's guid
			arg_guid.data,			// uint64_t arg
			pTS_finish_clg);		// rmd_guid_t created codelet's guid
#ifdef DEBUG
xe_printf("body_codelel R sched finish_scg %ld (retval=%d)\n",pTS_finish_scg.data,retval);
#endif
		assert(retval==0);

// create the async codelet for parTreeSearch(depth+1,child[i])
		for(int i=0;i<numChildren;i++) {
			pTS_body_async_param_t pTS_async_arg_lcl, *pTS_async_arg_dbp;
			rmd_guid_t pTS_async_arg_dbg, pTS_async_scg;
			RMD_DB_ALLOC(&pTS_async_arg_dbg, sizeof(pTS_body_async_param_t), LOCAL, ALLOC_LOC);
			RMD_DB_MEM  (&pTS_async_arg_dbp, pTS_async_arg_dbg);
			pTS_async_arg_lcl.main_post_scg      = main_post_scg;
			pTS_async_arg_lcl.pTS_body_clg       = pTS_body_clg;
			pTS_async_arg_lcl.slot               = i;
		        pTS_async_arg_lcl.parent.type        = childType;
			pTS_async_arg_lcl.parent.height      = parentHeight + 1;
			pTS_async_arg_lcl.parent.numChildren = -1;    // not yet determined
			for (int j = 0; j < computeGranularity; j++) {
				rng_spawn(parent->state.state, pTS_async_arg_lcl.parent.state.state, i);
			} // for j
#ifdef RAG_BLOCK
		        REM_STX_ADDR(pTS_async_arg_dbp, pTS_async_arg_lcl, pTS_body_async_param_t);
#else
		        *pTS_async_arg_dbp = pTS_async_arg_lcl;
#endif
			retval = rmd_codelet_sched(
				&pTS_async_scg,		// rmd_guid_t* scheduled codelet's guid
				pTS_finish_scg.data,	// uint64_t arg
				pTS_body_clg);		// rmd_guid_t created codelet's guid
#ifdef DEBUG
xe_printf("body_codelel R sched async_scg %ld %ld (retval=%d)\n",pTS_async_scg.data,pTS_finish_scg.data,retval);
#endif
			assert(retval==0);
			retval = rmd_codelet_satisfy(
				pTS_async_scg,		// rmd_guid_t scheduled codelet's guid
				pTS_async_arg_dbg,	// rmd_guid_t db guid
				0);			// int dep_slot
#ifdef DEBUG
xe_printf("body_codelel R satisfy %ld slot %d with dbg %ld (retval=%d)\n",pTS_async_scg.data,slot,pTS_async_arg_dbg.data,retval);
#endif
			assert(retval==0);
			RMD_DB_RELEASE(pTS_async_arg_dbg);	// rmd_guid_t db guid
		} // for numChildren

		RMD_DB_ALLOC(&pTS_finish_arg_dbg, sizeof(pTS_finish_return_param_t), LOCAL, ALLOC_LOC);
		RMD_DB_MEM  (&pTS_finish_arg_dbp, pTS_finish_arg_dbg);
// slot, depth, parent, r //
		pTS_finish_arg_lcl.main_post_scg  = main_post_scg;
		pTS_finish_arg_lcl.slot           = slot;
		pTS_finish_arg_lcl.r              = r;
#ifdef RAG_BLOCK
		REM_STX_ADDR(pTS_finish_arg_dbp, pTS_finish_arg_lcl, pTS_finish_return_param_t);
#else
		*pTS_finish_arg_dbp = pTS_finish_arg_lcl;
#endif
		retval = rmd_codelet_satisfy(
			pTS_finish_scg,			// rmd_guid_t scheduled codelet's guid
			pTS_finish_arg_dbg,		// rmd_guid_t db guid
			numChildren);			// int dep_slog
#ifdef DEBUG
xe_printf("body_codelel F satisfy %ld slot %d with dbg %ld (retval=%d)\n",pTS_finish_scg.data,numChildren,pTS_finish_arg_dbg.data,retval);
#endif
		assert(retval==0);
		RMD_DB_RELEASE(pTS_finish_arg_dbg);	// rmd_guid_t db guid
	} // if numChildren
	RMD_DB_FREE(db[0]);				// rmd_guid_t db guid
#ifdef TRACE
xe_printf("leave body_codelet\n");
#endif
	return NULL_GUID;
}
//----------------------------------------------------------------//

void parTreeSearch(rmd_guid_t main_post_scg, rmd_guid_t pTS_body_clg, int depth, Node *root) {
	int retval=-1;
	rmd_guid_t pTS_body_scg, pTS_body_arg_dbg;

#ifdef TRACE
xe_printf("enter parTreeSearch (main_post = %ld) (body = %ld) (depth = %d) (root = %d %d)\n",main_post_scg.data, pTS_body_clg.data,depth, root->height, root->numChildren);
#endif
	rmd_guid_t pTS_return_clg;
	retval = rmd_codelet_create(
		&pTS_return_clg,	// rmd_guid_t *new_guid
		pTS_return_codelet,	// rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		-202,			// uinit64_t default_arg
		1,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
#ifdef DEBUG
xe_printf("parTreeSearch create pTS_return_clg %ld (retval=%d)\n",pTS_return_clg.data,retval);
#endif
	assert(retval==0);
// create an instance for pTS_return
	rmd_guid_t pTS_return_scg;
	retval = rmd_codelet_sched(
				&pTS_return_scg,	// rmd_guid_t* scheduled codelet's guid
				0,			// uint64_t arg
				pTS_return_clg);	// rmd_guid_t created codelet's guid
#ifdef DEBUG
xe_printf("parTreeSearch sched pTS_return_scg %ld (retval=%d)\n",pTS_return_scg.data,retval);
#endif
	assert(retval==0);
// create a db for parTreeSearch's argument and return value
	pTS_body_async_param_t pTS_body_arg_lcl, *pTS_body_arg_dbp;
	RMD_DB_ALLOC(&pTS_body_arg_dbg, sizeof(pTS_body_async_param_t),  LOCAL, ALLOC_LOC);
	RMD_DB_MEM  (&pTS_body_arg_dbp, pTS_body_arg_dbg);
	pTS_body_arg_lcl.main_post_scg = main_post_scg;
	pTS_body_arg_lcl.pTS_body_clg  = pTS_body_clg;
	pTS_body_arg_lcl.slot          = 0;
	pTS_body_arg_lcl.parent        = *root;
#ifdef RAG_BLOCK
	REM_STX_ADDR(pTS_body_arg_dbp, pTS_body_arg_lcl,pTS_body_async_param_t);
#else
	*pTS_body_arg_dbp = pTS_body_arg_lcl;
#endif

// create a new instance of pTS_body
	retval = rmd_codelet_sched(
			&pTS_body_scg,		// rmd_guid_t* scheduled codelet's guid
			pTS_return_scg.data,	// uint64_t arg
			pTS_body_clg);		// rmd_guid_t created codelet's guid
#ifdef DEBUG
xe_printf("parTreeSearch sched pTS_body_scg %ld %ld (retval=%d)\n",pTS_body_scg.data,pTS_return_scg.data,retval);
#endif
	assert(retval==0);
// pass the argument to pTS_body
	retval = rmd_codelet_satisfy(
			pTS_body_scg,		// rmd_guid_t scheduled codelet's guid
			pTS_body_arg_dbg,	// rmd_guid_t db guid
			0);			// int dep_slot
#ifdef DEBUG
xe_printf("parTreeSearch satisfy pTS_body_scg %ld slot 0 with dbg %ld (retval=%d)\n",pTS_body_scg.data,pTS_body_arg_dbg.data,retval);
#endif
	assert(retval==0);
	RMD_DB_RELEASE(pTS_body_arg_dbg);	// rmd_guid_t db guid
#ifdef TRACE
xe_printf("leave parTreeSearch\n");
#endif
}

void main_pre(rmd_guid_t main_post_scg, rmd_guid_t pTS_body_clg, int argc, char **argv) {
	int retval=-1;
	static Node root;
#ifdef TRACE
xe_printf("enter main_pre\n");
#endif

#ifdef DEBUG
xe_printf("line 1 main_pre argc = %d\n",argc);
#endif
	uts_parseParams(argc, argv);
#ifdef DEBUG
xe_printf("line 2 main_pre\n");
#endif
	uts_printParams();
#ifdef DEBUG
xe_printf("line 3 main_pre\n");
#endif
	uts_initRoot(&root, type);
#ifdef DEBUG
xe_printf("line 4 main_pre\n");
#endif
	double t1 = uts_wctime();
// create a db for main_post arguments passed from main_pre
	main_post_param0_t *main_post_arg0_dbp;
	rmd_guid_t main_post_arg0_dbg;
#ifdef DEBUG
xe_printf("line 5 main_pre\n");
#endif
	RMD_DB_ALLOC(&main_post_arg0_dbg, sizeof(main_post_param0_t),  LOCAL, ALLOC_LOC);
#ifdef DEBUG
xe_printf("line 6 main_pre\n");
#endif
	RMD_DB_MEM  (&main_post_arg0_dbp, main_post_arg0_dbg);
#ifdef DEBUG
xe_printf("line 7 main_pre\n");
#endif
#ifdef DEBUG
xe_printf("line 8 main_pre\n");
#endif
#ifdef RAG_BLOCK
    REM_ST64_ADDR(&main_post_arg0_dbp->t1, t1);
#else
	main_post_arg0_dbp->t1 = t1;
#endif
#ifdef DEBUG
xe_printf("line 9 main_pre\n");
#endif
	retval = rmd_codelet_satisfy(
			main_post_scg,		// rmd_guid_t scheduled codelet's guid
			main_post_arg0_dbg,	// rmd_guid_t db guid
			0);			// int dep_slot
#ifdef DEBUG
xe_printf("line 10 main_pre satisfy main_post %ld slot 0 with dbg %ld (retval=%d)\n",main_post_scg.data,main_post_arg0_dbg.data,retval);
#endif
	assert(retval==0);
#ifdef DEBUG
xe_printf("line 11 main_pre\n");
#endif
	RMD_DB_RELEASE(main_post_arg0_dbg);		// rmd_guid_t  db guid
#ifdef DEBUG
xe_printf("line 12 main_pre\n");
#endif
	parTreeSearch(main_post_scg,pTS_body_clg,0, &root);
#ifdef DEBUG
xe_printf("line 13 main_pre\n");
#endif
#ifdef TRACE
xe_printf("leave main_pre\n");
#endif
	return;
}

void main_post(double t1, Result r) {
	double t2 = uts_wctime();

	maxTreeDepth = r.maxdepth;
	nNodes  = r.size;
	nLeaves = r.leaves;
#ifdef TRACE
xe_printf("enter main_post\n");
#endif
#ifdef RAG_SIM
	if(t1 < 0) t1 = 0;
	if(t2 <= t1) t2 = t1+123; // RAG avoid divide by zero
#endif
	uts_showStats(GET_NUM_THREADS, 0, t2-t1, nNodes, nLeaves, maxTreeDepth);
#ifdef TRACE
xe_printf("leave main_post\n");
#endif
}

rmd_guid_t main_post_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
	assert(n_db == 2);
#ifdef TRACE
xe_printf("enter main_post_codelet arg  %ld db[0] %ld db_ptr[0] %ld\n",arg,db[0].data,(uint64_t)db_ptr[0]);
xe_printf("enter main_post_codelet n_db %d db[1] %ld db_ptr[1] %ld\n",n_db,db[1].data,(uint64_t)db_ptr[1]);
#endif
	main_post_param0_t main_post_arg0_lcl;
        REM_LDX_ADDR(main_post_arg0_lcl,db_ptr[0],main_post_param0_t);
	main_post_param1_t main_post_arg1_lcl;
        REM_LDX_ADDR(main_post_arg1_lcl,db_ptr[1],main_post_param1_t);
	main_post(main_post_arg0_lcl.t1, main_post_arg1_lcl.r);
	rmd_complete();
	RMD_DB_FREE(db[0]);
	RMD_DB_FREE(db[1]);
#ifdef TRACE
xe_printf("leave main_post_codelet\n");
#endif
	return NULL_GUID;
}

rmd_guid_t main_pre_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
	int retval=-1;
#ifdef TRACE 
xe_printf("enter main_pre_codelet arg %ld n_db %d db[0] %ld db_ptr[0] %ld\n",arg,n_db,db[0].data,(uint64_t)db_ptr[0]);
#endif
	assert(n_db == 1);
#ifdef DEBUG
xe_printf("line 1 main_pre_codelet\n");
#endif
	main_pre_param_t main_pre_arg_lcl;
        REM_LDX_ADDR(main_pre_arg_lcl,db_ptr[0],main_pre_param_t);
#ifdef DEBUG
xe_printf("line 2.0 main_pre_codelet\n");
xe_printf("db[0].data = %ld\n",db[0].data);
xe_printf("line 2.1 main_pre_codelet\n");
xe_printf("db_ptr[0] = %ld\n",(uint64_t)db_ptr[0]);
xe_printf("line 2.2 main_pre_codelet\n");
#endif
#ifdef DEBUG
xe_printf("line 3 main_pre_codelet\n");
#endif
	int   argc  = main_pre_arg_lcl.argc;
#ifdef DEBUG
xe_printf("line 4 main_pre_codelet argc = %d (00)\n",argc);
#endif
	char **argv = main_pre_arg_lcl.argv;
#ifdef DEBUG
xe_printf("line 5 main_pre_codelet\n");
#endif
// create an codelet guid for main_post
	rmd_guid_t main_post_clg;
	retval = rmd_codelet_create(
		&main_post_clg,		// rmd_guid_t *new_guid
		main_post_codelet,	// rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		-203,			// uinit64_t default_arg
		2,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
#ifdef DEBUG
xe_printf("main_pre_codelet create main_post_clg %ld (retval=%d)\n",main_post_clg.data,retval);
#endif
	assert(retval==0);
// create codelet for pTS codelet
	rmd_guid_t pTS_body_clg;
	retval = rmd_codelet_create(
		&pTS_body_clg,		// rmd_guid_t *new_guid
		pTS_body_async_codelet,	// rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		-204,			// uinit64_t default_arg
		1,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
#ifdef DEBUG
xe_printf("main_pre_codelet create pTS_body_clg %ld (retval=%d)\n",pTS_body_clg.data,retval);
#endif
	assert(retval==0);
// create a scheduled instance for main_post
	rmd_guid_t main_post_scg;
	retval = rmd_codelet_sched(
				&main_post_scg,		// rmd_guid_t scheduled codelet's guid
				0,			// uint64_t arg
				main_post_clg);		// rmd_guid_t created codelet's guid
#ifdef DEBUG
xe_printf("sched main_post %d\n",retval);
#endif
	main_pre(main_post_scg,pTS_body_clg,argc, argv);
#ifdef DEBUG
xe_printf("line 6 main_pre_codelet\n");
#endif
	RMD_DB_FREE(db[0]);
#ifdef TRACE
xe_printf("leave main_pre_codelet\n");
#endif
	return NULL_GUID;
}

// main codelet for rmd's runtime to start

rmd_guid_t main_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)
{
	int retval=-1;
#if defined(RAG_SIM) || defined(RAG_AFL)
// create argc
	int   argc = 0;
	char **argv = NULL;
#endif
#ifdef TRACE
	xe_printf("enter main_codelet arg %ld n_db %d db[0] %ld db_ptr[0] %ld\n",arg,n_db,db[0].data,(uint64_t)db_ptr[0]);
#endif

// create codelets
	retval = rmd_codelet_create(
		&main_pre_clg,		// rmd_guid_t *new_guid
		main_pre_codelet,	// rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		-201,			// uinit64_t default_arg
		1,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
#ifdef DEBUG
xe_printf("main_codelet create main_pre_clg %ld (retval=%d)\n",main_pre_clg.data,retval);
#endif
	assert(retval==0);
// create a db for main_pre's argument
        main_pre_param_t main_pre_arg_lcl, *main_pre_arg_dbp;
        rmd_guid_t main_pre_arg_dbg;
	RMD_DB_ALLOC(&main_pre_arg_dbg, sizeof(main_pre_param_t),  LOCAL, ALLOC_LOC);
#ifdef DEBUG
xe_printf("main_codelet alloc main_pre_arg_dbg %ld (retval=%d)\n",main_pre_arg_dbg.data,retval);
#endif
	RMD_DB_MEM  (&main_pre_arg_dbp, main_pre_arg_dbg);
#ifdef DEBUG
xe_printf("main_codelet acquire main_pre_arg_dbg %ld (retval=%d)\n",(uint64_t)main_pre_arg_dbp,retval);
#endif
	main_pre_arg_lcl.argc = argc;
	main_pre_arg_lcl.argv = argv;
#ifdef RAG_BLOCK
	REM_STX_ADDR(main_pre_arg_dbp,main_pre_arg_lcl,main_pre_param_t);
#else
	*main_pre_arg_dbp = main_pre_arg_lcl;
#endif

// create an instance for main_pre
	retval = rmd_codelet_sched(
				&main_pre_scg,		// rmd_guid_t* scheduled codelet's guid
				0,			// uint64_t arg
				main_pre_clg);		// rmd_guid_t created codelet's guid
#ifdef DEBUG
xe_printf("main_codelet sched main_pre_scg %ld (retval=%d)\n",main_pre_scg.data,retval);
#endif
	assert(retval==0);
// provide the arguments to main_pre
	retval = rmd_codelet_satisfy(
				main_pre_scg,		// rmd_guid_t scheduled codelet's guid
				main_pre_arg_dbg,	// rmd_guid_t db guid
				0);			// int dep_slot
#ifdef DEBUG
xe_printf("main_codelet satisfy main_pre %ld slot 0 with dbg %ld (retval=%d)\n",main_pre_scg.data,main_pre_arg_dbg.data,retval);
#endif
	assert(retval==0);
// release DB main_pre_arg
	RMD_DB_RELEASE(main_pre_arg_dbg);		// rmd_guid_t db guid
#ifdef TRACE
xe_printf("leave main_codelet\n");
#endif
	return NULL_GUID;
}
//**********************************************************
//  UTS Implementation Hooks                               *
//**********************************************************

int  impl_paramsToStr(char *strBuf, int ind) { 
  strcat(strBuf+ind,"Execution strategy:  Codelet Parallel Recursive Search");
  ind = strlen(strBuf);
  return ind;
}

// Not using UTS command line params, return non-success
int  impl_parseParam(char *param, char *value) { return 1; }

void impl_helpMessage() {
  xe_printf("   none.\n");
}

void impl_abort(int err) {
  exit(err);
}

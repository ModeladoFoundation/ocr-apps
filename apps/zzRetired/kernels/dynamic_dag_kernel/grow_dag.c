#ifndef __AFL__
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#endif

#include <stdio.h>
#include <string.h>             /* for strdup */
#include <strings.h>    /* for strcasecmp */
#include <stdlib.h>             /* for free */
#include <assert.h>
#include <stdint.h>
/*#include <cprops/hashtable.h> */


#include "grow_dag.h"
#include "int_vec.h"

/* routines from dynamic_graph managment */
uint8_t add_node(struct  graphtree_idx* tree, uint32_t srcid);
uint8_t add_edge(struct graphtree_idx* tree, uint32_t srcid, uint32_t desid);

int max_depth = 10000;

nref nodecounter;
nref edgecounter;

struct IntVec* fanoutcredit; /* vector of fanoutcredit; grows dynamically */
struct Vec_IntVec nref_2_depth_iv;  /* nodes at given depth; grow dynamically */
int totalfanoutcredit;
int maxDeg;


#ifndef __AFL__
const gsl_rng_type* T;
gsl_rng* rng;
#endif


int intcmp(int* a, int* b)
{
  if (*a < *b) return -1;
  if (*a == *b) return 0;
  return 1;
}


void grow_dag_init(int d)
{

  maxDeg = d;
  totalfanoutcredit = 0;
  nodecounter = 0;
  edgecounter = 0;

  nref_2_depth_iv.num_elements = 0;
  nref_2_depth_iv.num_allocated = 0;
  nref_2_depth_iv.arr = NULL;
  fanoutcredit = construct_IntVec();

#ifndef __AFL__
  gsl_rng_env_setup();
  T = gsl_rng_default;
  rng = gsl_rng_alloc(T);
#endif
}


uint32_t genRand(uint32_t M)
{
  double r = drand48();
  double x = M * r;
  uint32_t y = (uint32_t)x;
  if(y < M)
    return y;
  else
    return 0;
}


nref findMatch_c()
{
  if(totalfanoutcredit == 0)
    {
      //std::cout<<"SKIP "<<std::endl;
      return -1;
    }
  while(1)
    {
      int i;
      for(i = nref_2_depth_iv.num_elements-1; i >= 0; --i)
	{
	  if(!nref_2_depth_iv.arr[i]) continue;
	  if(nref_2_depth_iv.arr[i]->num_elements == 0) continue;
	  double rand_val = drand48();
	  if(rand_val > 0.35) continue; //chose a lower depth
	  struct IntVec* nodes_at_depth = nref_2_depth_iv.arr[i];
	  assert(nodes_at_depth->num_elements > 0);

	  int choosen_pos = genRand(nodes_at_depth->num_elements);
	  nref choosen_parent = nodes_at_depth->arr[choosen_pos];
	  int parent_fanout_remcredit = fanoutcredit->arr[choosen_parent];
	  assert(parent_fanout_remcredit > 0);

	  --parent_fanout_remcredit;
	  --totalfanoutcredit;
	  //remove from inverted_idx if the quota is met
	  if(parent_fanout_remcredit == 0)
	    {
	      nodes_at_depth->arr[choosen_pos] = nodes_at_depth->arr[nodes_at_depth->num_elements - 1];
	      --nodes_at_depth->num_elements;
	    }
	  fanoutcredit->arr[choosen_parent] = parent_fanout_remcredit;
	  return choosen_parent;

	}
    }
}



int addNode_c(struct graphtree_idx* gptr, int fanout, int fanin)
{
  if(fanout == 0 && fanin == 0) return -1;
  if(totalfanoutcredit < fanin) return -1;

  add_int(fanoutcredit, fanout);
  totalfanoutcredit += fanout;
  uint32_t newid = nodecounter;  ++nodecounter;
  add_node(gptr, newid);
  //  printf("Node-Add: %d\n",newid);

  if(fanin  == 0) /* insert node at depth 0 */
    {
      int zdepth = 0;


      if(nref_2_depth_iv.num_elements <= zdepth)
	{
	  int i;
	  for(i = nref_2_depth_iv.num_elements; i <= zdepth; ++i)
	    {
	      struct IntVec* avec = construct_IntVec();
	      add_intvec(&nref_2_depth_iv, avec);
	    }
	}


      add_int(nref_2_depth_iv.arr[zdepth], newid);
      return 0;
    }
  int mydepth;
  int i;


  for(i = 0; i < fanin; ++i)
    {

      nref src = findMatch_c();
      if(src !=-1){
	mydepth = add_edge(gptr, src, newid);
	//printf("Edge-add: %d,%d\n",src, newid);
	edgecounter++;
      }
      else
	{
	assert(0);
	}

    }

  assert(mydepth < max_depth);


  if(fanout > 0)
    {
      if(nref_2_depth_iv.num_elements <= mydepth)
	{
	  int i;
	  for(i = nref_2_depth_iv.num_elements; i <= mydepth; ++i)
	    {
	      struct IntVec* avec = construct_IntVec();

	      add_intvec(&nref_2_depth_iv, avec);
	    }
	}
      add_int(nref_2_depth_iv.arr[mydepth], newid);
    }
  return mydepth;
}


/* generate a node with its fanin,fanout sampled  */
/* from a distribution, currently using binomail(n,p) */

void boot_strap_new_node(struct graphtree_idx* gptr)
{
  #ifndef __AFL__
  int fanout= gsl_ran_binomial(rng, .5, maxDeg);
  #else
  int fanout= drand48() * maxDeg;
  #endif

 addNode_c(gptr, fanout,0);
}

void addNewNode(struct graphtree_idx* gptr)
{

  int maxFanIn = maxDeg;
  while(1)
    {
      #ifndef __AFL__
      int fanin = gsl_ran_binomial(rng, .5, maxFanIn);
      int fanout= gsl_ran_binomial(rng, .5, maxDeg);
      #else
      int fanin = drand48() * maxDeg;
      int fanout = drand48() * maxDeg;
      #endif

      int retval =  addNode_c(gptr,fanout, fanin);
      if(retval != -1)
	return;
      if(maxFanIn > 0)
	maxFanIn = maxFanIn - 1;
    }
}

void free_dag_generator()
{
  /* free random generator */


  /* free graph data structures */

  /* free dag-generator */
  free_Vec_IntVec(&nref_2_depth_iv);
  free(fanoutcredit->arr);
  free(fanoutcredit);
  #ifndef __AFL__
  gsl_rng_free(rng);
  #endif
}


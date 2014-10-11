#include <iostream>
#include <stdint.h>


#define  NodeSet set<nref>
#define foreach BOOST_FOREACH



#include "grow_dag.h"
#include "NaivePatternQuery_Impl.h"


#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>



using namespace NaivePatternQuery_Impl;


void assignLabelRandom()
{
  const NodeSet& allNodes = bg->allNodes();
  ridx_node_label_map.clear();
  ridx_node_label_map.resize(max_labels + 1);
  foreach(nref n, allNodes)
    {
      int random_label = genRand(max_labels + 1);
      ridx_node_label_map[random_label].insert(n);
    }
}

void buildQueryRandom()
{

  aquery.clear();
  aquery.resize(num_qnodes);
  for(int i  = 0 ; i < num_qnodes; ++i)
    {
      aquery[i] = genRand(max_labels + 1);

    }

}

/* Return 1 if the difference is negative, otherwise 0.  */
int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}



int main(int argc, char** argv)
{
  struct timeval tvBegin, tvEnd, tvDiff;
  build_dag(10000);
  time_t t1;
  (void) time(&t1);
  srand48((long) t1);
  int run_sz  = 100;
  int counter  = 0;
  while(counter < run_sz)
    {
      gettimeofday(&tvBegin, NULL);
      max_labels = 5 + genRand(400); //number of labels range 5 -- 100
      int qsz = 2 + genRand(5); //size of query node

      init_pq_run(qsz, max_labels);
      std::cout<<"Run Parameters"<<std::endl;
      std::cout<<"   num_labels  ="<<max_labels<<std::endl;
      std::cout<<"   num_queries ="<<qsz<<std::endl;
      assignLabelRandom();
      buildQueryRandom();

      build_ridx_node_pqmash_map();
      build_nref_cpgnode_map();
      build_cpg();
      std::ofstream ofs("allPaths.txt");
      //AllPathsOptimized(ofs);
      //draw_cpg();
      clear();
      gettimeofday(&tvEnd, NULL);
      timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
      printf("Elapsed-Time = %ld.%06ld\n\n", tvDiff.tv_sec, tvDiff.tv_usec);
      ++counter;
    }
  free_dag_generator();
  delete NaivePatternQuery_Impl::bg;
}

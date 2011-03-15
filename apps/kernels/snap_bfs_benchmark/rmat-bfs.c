#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "graph_defs.h"
#include "graph_gen.h"

#include "utils.h"





int main(int argc, char** argv) {
  graph_t* g;
  long src = -1;
  int est_diameter;
  long num_vertices_visited;
  double elapsed_time;
  srand48(time(0));
  g = (graph_t *) malloc(sizeof(graph_t));
  read_SNAP_graph(g, argv[1]);
  
  /* fprintf(stdout, "Number of vertices     : %ld\n", g->n); */
  /* if (g->undirected) */
  /*   fprintf(stdout, "Number of edges        : %ld\n\n", g->m/2); */
  /* else */
  /*   fprintf(stdout, "Number of edges        : %ld\n\n", g->m); */

  int counter =0 ;
  elapsed_time = get_seconds();
  while(counter < 500)
    {
  if (src == -1)
    src = lrand48() % g->n;
  
  est_diameter = 100;

  num_vertices_visited = BFS_parallel_frontier_expansion(g, src, est_diameter);
  ++counter;
    }
  free_graph(g);
  free(g);
  elapsed_time = get_seconds() - elapsed_time;
  elapsed_time = elapsed_time/500;
  printf("%lf\n",elapsed_time);
  /* fprintf(stdout, "  Breadth-first search from vertex %ld\n", src);  */
  /* fprintf(stdout, "  Number of vertices visited: %ld\n\n",  */
  /*           num_vertices_visited); */

  
  
}

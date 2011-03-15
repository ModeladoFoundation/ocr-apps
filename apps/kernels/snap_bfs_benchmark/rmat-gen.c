#include "graph_defs.h"
#include "graph_gen.h"
#include "utils.h"

int main(int argc, char* argv[])
{
 
  graph_t *g;
  double elapsed_time;
  srand48(time(0));
  elapsed_time = get_seconds();
  g = (graph_t *) alloca (sizeof (graph_t));
  assert(g != NULL);
  graph_gen(g, argv[1], "rmat");
  elapsed_time = get_seconds() - elapsed_time;
  printf("%lf\n",elapsed_time);
  FILE *fp = fopen (argv[2], "w");
  save_undir_unwgt_graph(fp, g);
  free_graph(g);
  return 0;
}

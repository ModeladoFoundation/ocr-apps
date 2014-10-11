#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include "graph_defs.h"


void build_snap_graph(graph_t* G, char* prefix, int np) { /* np <-- num_process */


    long i, j;
    long undirected;
    long n, m=0;
    long offset;
    long step;
    long u, v;
    attr_id_t *src;
    attr_id_t *dest;
    attr_id_t *degree;

    /* set undirected, num_nodes and num_edges */
    int t =0;
    char elfn[200];
    FILE* f;
    sprintf(elfn, "%s_num_nodes.gr",prefix);
    f = fopen(elfn,"r");
    int nn;
    int ne;
    fscanf(f, "%d", &nn);
    fclose(f);
    for(; t < np; t++)
      {
	sprintf(elfn, "%s%d.gr",prefix,t);
	f = fopen(elfn,"r");
	fscanf(f, "%d", &ne);
	m += ne;
	fclose(f);
      }


    G->undirected = 1;
    G->n = nn;
    G->m= m;
    G->weight_type = 0;


    n = G->n;


    src = (attr_id_t *) malloc (m * sizeof(attr_id_t));
    dest = (attr_id_t *) malloc(m * sizeof(attr_id_t));
    degree = (attr_id_t *) calloc(n, sizeof(attr_id_t));

    assert(src != NULL);
    assert(dest != NULL);
    assert(degree != NULL);

    /* set src[i] and des[i] for each edge */
    i = 0;
    for(t=0; t < np; t++) /* read all edges */
      {
	int sid;
	int did;
	sprintf(elfn, "%s%d.gr",prefix,t);
	f = fopen(elfn,"r");
	fscanf(f, "%d", &ne);
	while(fscanf(f, "%d %d", &sid, &did) !=EOF )
	  {
	    src[i] = sid;
	    dest[i] = did;
	    ++i;
	  }
	fclose(f);
      }

    for (i=0; i<m; i++) {
        degree[src[i]]++;
	degree[dest[i]]++;
    }




    /* Update graph data structure */

    G->endV = (attr_id_t *) calloc(2*m, sizeof(attr_id_t));
    G->edge_id = (attr_id_t *) calloc(2*m, sizeof(attr_id_t));


    assert(G->endV != NULL);
    assert(G->edge_id != NULL);

    G->numEdges = (attr_id_t *) malloc((n+1)*sizeof(attr_id_t));
    assert(G->numEdges != NULL);



    G->m = 2*m;

    /* compressed representation */
    G->numEdges[0] = 0;
    for (i=1;i<=G->n;i++) {
        G->numEdges[i] = G->numEdges[i-1] + degree[i-1];
    }

    for (i=0; i<m; i++) {
        u = src[i];
        v = dest[i];
        offset = degree[u]--;
        G->endV[G->numEdges[u]+offset-1] = v;
        G->edge_id[G->numEdges[u]+offset-1] = i;



	offset = degree[v]--;
	G->endV[G->numEdges[v]+offset-1] = u;
	G->edge_id[G->numEdges[v]+offset-1] = i;

    }
    G->zero_indexed = 1;

    free(src);
    free(dest);
    free(degree);
}


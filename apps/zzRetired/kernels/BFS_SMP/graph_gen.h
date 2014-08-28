

typedef struct graph_ttag
{
  uint32_t n;
  uint32_t m;
  Vec_IntVec adj_store;
  uint32_t (*deg) (uint32_t);


} graph_t;



double a;
double b;
double c;
double d;

void rmat_gen_pair(uint32_t n,void (*vis)(uint32_t, uint32_t))
{
  uint32_t u = 0, v = 0;
  uint32_t step = n/2;
  a = 0.57;
  b= 0.19;
  c = 0.19;
  d = 1.0 -a -b -c;
  long double limit = log2l((long double)n);

  for(unsigned int j = 0; j < limit; ++j) {

	double p = drand48();


	if (p < a)
	  ;
	else if (p >= a && p < a + b)
	  v += step;
	else if (p >= a + b && p < a + b + c)
	  u += step;
	else { // p > a + b + c && p < a + b + c + d
	  u += step;
	  v += step;
	}

	step /= 2;

	// 0.2 and 0.9 are hardcoded in the reference SSCA implementation.
	// The maximum change in any given value should be less than 10%
	a *= 0.9 + 0.2 * drand48();
	b *= 0.9 + 0.2 * drand48();
	c *= 0.9 + 0.2 * drand48();
	d *= 0.9 + 0.2 * drand48();


	double S = a + b + c + d;

	a /= S; b /= S; c /= S;
	// d /= S;
	// Ensure all values add up to 1, regardless of floating point errors
	d = 1. - a - b - c;
      }

  vis(u,v);
}



void gen_edges(void (*vis)(uint32_t, uint32_t),  uint32_t n, uint32_t m)
{
  for(int i = 0; i < m; ++i)
    {
      rmat_gen_pair(n, vis);
    }

}

void init_adjlist(graph_t* G)
{
  reserve_VecOfIntVec(&G->adj_store, G->n);
  for(uint32_t i = 0; i < G->n; ++i)
    {
      IntVec* adj_vec =  construct_IntVec();
      add_intvec(&G->adj_store, adj_vec);
    }
}


void graph_stats(graph_t* G)
{
  uint32_t i = 0;
  for(; i < G->n; i++)
    {
      fprintf(stderr, "%u\n",G->adj_store.arr[i]->num_elements);
    }
}

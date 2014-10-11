#include "pil_nw.h"


pil_nwID_t pil_get_nwID()
{
	pil_nwID_t id = 0;
#ifdef DISTSWARM
	id = nw_getNodeID();
#endif /* DISTSWARM */
	return id;
}

pil_nwID_t pil_get_nwCount()
{
	pil_nwID_t id = 0;
#ifdef DISTSWARM
	id = nw_getNodeCount();
#endif /* DISTSWARM */
	return id;
}
#if 0
void pil_barrier_all()
{
#ifdef DISTSWARM
	printf("pil_barrier_all()\n"); fflush(stdout);
	nw_barrierAll(GRAPH_BARRIER_0, swarm_cargs(bar));
#endif /* DISTSWARM */
}
#endif

void pil_send(void *source_ptr, size_t num, int target_node, int TARGET_CODELET)
{
#if 0
#ifdef DISTSWARM
	nw_netBuffer_t *nb;
	nb = nw_netBuffer_new(NULL, num)
	//void * memcpy ( void * destination, const void * source, size_t num );
	memcpy(nb->p, source_ptr, num);
	nw_call(target_node, TARGET_CODELET, nw_NetBuffer_to_swarm_Trasferable(nb), NULL, NULL);
#endif /* DISTSWARM */
#endif
}

#ifndef PIL_NW_H
#define PIL_NW_H
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifdef PIL2SWARM
#include <swarm/nw_api.h>
#endif

#ifdef DISTSWARM
//barrierID_t const GRAPH_BARRIER_0 = 0;
#endif /* DISTSWARM */

typedef uint32_t pil_nwID_t;

pil_nwID_t pil_get_nwID(void);
pil_nwID_t pil_get_nwCount(void);
void pil_barrier_all(void);
void pil_send(void *, size_t, int, int);

#endif /* PIL_NW_H */

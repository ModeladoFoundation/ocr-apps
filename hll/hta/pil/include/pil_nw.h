#ifndef PIL_NW_H
#define PIL_NW_H
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <semaphore.h>

#ifdef PIL2SWARM
#include <swarm/nw_api.h>
#endif

#include "pil.h"

#ifdef DISTSWARM
//barrierID_t const GRAPH_BARRIER_0 = 0;
#endif /* DISTSWARM */

// types
typedef uint32_t pil_nwID_t;

struct _pil_communication_buffer {
	void *ptr;
	int volatile full;
	size_t size;
	struct _pil_communication_buffer *next;
};

struct _pil_send_buffer {
	struct _pil_communication_buffer *reader;
	struct _pil_communication_buffer *writer;
	size_t max_buffer_size;
	sem_t mutex;
};

// variables defined in the *.c file
extern int _pil_barrier_counter;
extern sem_t _pil_turnstile;
extern sem_t _pil_turnstile2;
extern sem_t _pil_mutex;

#if (CIRC_BUF_SIZE == 1)
extern struct _pil_communication_buffer **_pil_send_buf;
#else
extern struct _pil_send_buffer **_pil_send_buf;
#endif

// function headers
void pil_nw_init();
void pil_nw_cleanup();
pil_nwID_t pil_get_nwID(void);
pil_nwID_t pil_get_nwCount(void);
void pil_barrier_all(int);
void pil_send(int, int, gpp_t, size_t, size_t, int);
void pil_recv(int, int, gpp_t *, size_t, size_t, int);
uint8_t pil_recv_get_buf_status(int, int);
uint8_t pil_send_get_buf_status(int, int);

#endif /* PIL_NW_H */

#ifndef __COMM_H__
#define __COMM_H__
#include "pil.h"
#include "HTA.h"

void comm_bcast(int pid, int src_bcast, void* data, size_t size);
void comm_recv(int pid, gpp_t buf, int src, size_t size, size_t offset);
void comm_send(int pid, gpp_t buf, int dest, size_t size, size_t offset);
void comm_sendrecv(int pid, gpp_t buf, int target, size_t size);
void comm_allreduce(int pid, ReduceOp fr_op, void* data, void* result, HTA_SCALAR_TYPE stype);
void comm_allgatherv(int pid, void* sendptr, size_t send_size, size_t send_offset, void* recvptr, size_t* recv_sizes, size_t* recv_offsets);
void comm_alltoallv(int pid, void* sendptr, size_t* send_sizes, size_t* send_offsets, void* recvptr, size_t* recv_sizes, size_t* recv_offsets);
int comm_can_send(int source, int dest);
int comm_can_recv(int source, int dest);
#endif

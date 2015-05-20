#include <stdio.h>
#include "Comm.h"
#include "pil.h"

int comm_can_send(int source, int dest) {
    return pil_send_get_buf_status(source, dest) == 0; // not full
}

int comm_can_recv(int source, int dest) {
    return pil_recv_get_buf_status(dest, source) == 1; // not empty
}

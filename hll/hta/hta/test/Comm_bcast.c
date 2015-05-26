#include <stdio.h>
#include "test.h"
#include "Comm.h"
#include "HTA.h"
#include "HTA_operations.h"
#include "timer.h"

#define MAX_ITER (5)

int hta_main(int argc, char** argv, int pid)
{
    int err = SUCCESS;
    int np = HTA_get_num_processes();

    SET_SPMD_PID(pid);
    MASTER_PRINTF("number of processes = %d\n", np); fflush(stdout);
    if(pid == -1) {
        printf("this is fork-join mode, skip the test\n");
        return 0;
    }
    for(int i = 0; i < MAX_ITER; i++) {
        double val = 0.0f;
        int src = i % np;

        // initialization
        if(pid == src)
            val = src;  // only source assigns value to val

        MASTER_EXEC_START
        hta_timer_start(0);
        MASTER_EXEC_END

        comm_bcast(pid, i % np, &val, sizeof(double));

        MASTER_EXEC_START
        hta_timer_stop(0);
        printf("time spent in comm_bcast is %.2lf\n", timer_get_sec(0));
        MASTER_EXEC_END

        // verify
        if(val != src) {
            err = ERR_UNMATCH;
        }
    }

    if(err != SUCCESS) {
        printf("pid %d detects unmatched communication results\n", pid);
    }

    int all_err = SUCCESS;
    comm_allreduce(pid, REDUCE_MAX, &err, &all_err, HTA_SCALAR_TYPE_INT32);
    assert(all_err==SUCCESS);
    return 0;
}

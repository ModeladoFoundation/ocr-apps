#ifndef __SPMD_MACRO_H__
#define __SPMD_MACRO_H__

// supply whichever HTA to acquire self pid
// since the pid is stored locally, it should
// always be called before using the other
// macros in this file
#define SET_SPMD_PID(id) \
    int __SPMD_pid = id;

#define SINGLE_EXEC_START(id) \
    if(__SPMD_pid == id || __SPMD_pid == -1) {

#define SINGLE_EXEC_END \
    }

#define SINGLE_EXEC_SYNC_START(id) \
    HTA_barrier(__SPMD_pid); \
    SINGLE_EXEC_START(id)

#define SINGLE_EXEC_SYNC_END \
    SINGLE_EXEC_END \
    HTA_barrier(__SPMD_pid);

// For process 0
#define MASTER_EXEC_START \
    SINGLE_EXEC_START(0)

#define MASTER_EXEC_END \
    }

// These macros should not be used in benchmarks
// where execution time is relevant
#define MASTER_EXEC_SYNC_START \
    SINGLE_EXEC_SYNC_START(0)

#define MASTER_EXEC_SYNC_END \
    SINGLE_EXEC_SYNC_END

#define MASTER_PRINTF(...) \
    if(__SPMD_pid == 0 || __SPMD_pid == -1) printf(__VA_ARGS__);

#endif

#ifndef __ce_xe_intf_h__
#define __ce_xe_intf_h__

/////////////////////// Interface requests ////////////////////////
//
// Input:
//  alarm code XE_MSG_READY
//  arg0 (r2) = request # < 32 | request data len in bytes
//              build with CE_REQ_MAKE()
//
//  arg1 (r3) = request data va in MR format
//              The request data holds the incoming and outgoing request
//              arguments - guids, mem addrs, lengths, flags, ...
// Output:
//  arg0 (r2) = return status code
//              Return values such as fds are written into the request data.
//              Return value is either 0 (success) or errno.
//
#define CE_REQ_MAKE( t, l )    (((t) << 32) | ((l) & ((1L << 32) - 1)))

#define CE_REQ_TYPE( r )  ((ce_request_type)((r) >> 32))
#define CE_REQ_LEN( r )   ((r) & ((1L << 32) - 1))
//
// XE registers to hold val1 and val2
//
#define XE_ARG0_REG 2   // R2
#define XE_ARG1_REG 3   // R3
//
// Request types
//
typedef enum {
    CE_REQTYPE_FIRST    = 0,
    CE_REQTYPE_MEMALLOC = CE_REQTYPE_FIRST,
    CE_REQTYPE_MEMFREE,
    CE_REQTYPE_GETCWD,
    CE_REQTYPE_CHDIR,
    CE_REQTYPE_CHMOD,
    CE_REQTYPE_CHOWN,
    CE_REQTYPE_LINK,
    CE_REQTYPE_SYMLINK,
    CE_REQTYPE_UNLINK,
    CE_REQTYPE_FILEOPEN,
    CE_REQTYPE_FILECLOSE,
    CE_REQTYPE_FILEREAD,
    CE_REQTYPE_FILEWRITE,
    CE_REQTYPE_FILELSEEK,
    CE_REQTYPE_FILESTAT,
    CE_REQTYPE_FILEFSTAT,
    CE_REQTYPE_GETTIMEOFDAY,
    CE_REQTYPE_GETHOSTNAME,
    CE_REQTYPE_ISATTY,
    CE_REQTYPE_MKDIR,
    CE_REQTYPE_RMDIR,
    CE_REQTYPE_READLINK,
    CE_REQTYPE_GETPID,
    CE_REQTYPE_CLONE,
    CE_REQTYPE_WAITPID,
    CE_REQTYPE_FINISH,
    CE_REQTYPE_KILLALL,
    CE_REQTYPE_CANCEL,
    CE_REQTYPE_SUSPEND,
    CE_REQTYPE_RESUME,
    CE_REQTYPE_DETACH,
    CE_REQTYPE_WAITALL,
    CE_REQTYPE_LAST
} ce_request_type;

//
// memory region preferred.
//
typedef enum {
    MREQ_LOCAL, MREQ_BLOCK, MREQ_GLOBAL, MREQ_ANY, MREQ_LAST = MREQ_ANY
} mreq_region;

///////////////////////////////// Request structures ///////////////////////////
//
// Allocate some memory
// If an allocation is to ANY, it will start at LOCAL and promote as necessary
// to the next layer until it does or can't succeed.
// If a specific region is provided, no promotion will occur.
// 'va' is set to the MR format start of the allocated memory
// returns ENOMEM if it doesn't succeed
//
// XXX Do we need to add a 'flags' member to indicate whether
// the allocation is private (freed on finish) or shared?
//
typedef struct {
    uint64_t  region;       // in - region preference (global, block, local) (Not implemented)
    uint64_t  va;           // out
    uint64_t  len : 56,     // in/out - size requested and actually provided
              private : 1,  // in - private use (reapable on finish) (Not implemented)
              promote : 1,  // in - promote region if alloc fails (Not implemented)
              unused : 6;
} memalloc_req;

//
// Free previously allocated memory
// Only an XE can free its own allocated local memory (?)
//
typedef struct {
    uint64_t  va;    // in
} memfree_req;

//
// Get the CWD
//
typedef struct {
    uint64_t buf_ptr; // in/out - writing cwd to *buf_ptr
    uint64_t size;    // in
} getcwd_req;

//
// Change the CWD
//
typedef struct {
    uint64_t path_ptr; // in
    uint64_t path_len; // in (not including terminal '\0')
} chdir_req;

//
// Change the permissions of a file
//
typedef struct {
    uint64_t path_ptr; // in
    uint64_t path_len; // in (not including terminal '\0')
    uint64_t mode;     // in
} chmod_req;

//
// Change the ownership of a file
//
typedef struct {
    uint64_t path_ptr; // in
    uint64_t path_len; // in (not including terminal '\0')
    uint64_t owner;    // in
    uint64_t group;    // in
} chown_req;

//
// Create a new name for a file
//
typedef struct {
    uint64_t oldpath_ptr; // in
    uint64_t oldpath_len; // in (not including terminal '\0')
    uint64_t newpath_ptr; // in
    uint64_t newpath_len; // in (not including terminal '\0')
} link_req;

//
// Create a symlink for a file
//
typedef struct {
    uint64_t oldpath_ptr; // in
    uint64_t oldpath_len; // in (not including terminal '\0')
    uint64_t newpath_ptr; // in
    uint64_t newpath_len; // in (not including terminal '\0')
} symlink_req;

//
// Remove a name for a file
//
typedef struct {
    uint64_t path_ptr; // in
    uint64_t path_len; // in (not including terminal '\0')
} unlink_req;

//
// File open
//
typedef struct {
    uint64_t fname_ptr; // in
    uint64_t fname_len; // in (len of fname not including terminal '\0')
    uint64_t flags;     // in
    uint64_t mode;      // in
    uint64_t fd;        // out
} fileopen_req;

//
// File close
//
typedef struct {
    uint64_t fd;        // in
} fileclose_req;

//
// read
//
typedef struct {
    uint64_t fd;        // in
    uint64_t buf;       // in - address to read into
    uint64_t len;       // in / out
} fileread_req;

//
// write
//
typedef struct {
    uint64_t fd;        // in
    uint64_t buf;       // in
    uint64_t len;       // in / out
} filewrite_req;

//
// lseek
//
typedef struct {
    uint64_t fd;        // in
    uint64_t offset;    // in/out
    uint64_t whence;    // in
} filelseek_req;

//
// stat
//
typedef struct {
    uint64_t fname_ptr; // in
    uint64_t fname_len; // in (len of fname not including terminal '\0')
    uint64_t stat_ptr;  // in/out - writing stat structure to location
    uint64_t stat_len;  // in (paranoia)
} filestat_req;

//
// fstat
//
typedef struct {
    uint64_t fd;        // in
    uint64_t stat_ptr;  // in/out - writing stat structure to location
    uint64_t stat_len;  // in (paranoia)
} filefstat_req;

//
// gettimeofday
//
typedef struct {
    uint64_t timeval_ptr;   // in/out - writing timeval to location
    uint64_t timezone_ptr;  // ignored
} gettimeofday_req;

//
// gethostname
//
typedef struct {
    uint64_t hname_ptr; // in/out - writing hostname to location
    uint64_t size;      // in  - max len of hname buffer
} gethostname_req;

//
// Test if a file descriptor refers to a terminal
//
typedef struct {
    uint64_t fd;  // in
} isatty_req;

//
// Make directory
//
typedef struct {
    uint64_t path_ptr; // in
    uint64_t path_len; // in (not including terminal '\0')
    uint64_t mode;     // in
} mkdir_req;

//
// Remove a directory
//
typedef struct {
    uint64_t path_ptr; // in
    uint64_t path_len; // in (not including terminal '\0')
} rmdir_req;

//
// Get the value of a symbolic link
//
typedef struct {
    uint64_t path_ptr;  // in
    uint64_t path_len;  // in
    uint64_t buf_ptr;   // in/out - link value written to location
    uint64_t size;      // in
} readlink_req;

//
// Return the caller's PID
//
typedef struct {
    uint64_t pid;  // out
} getpid_req;

//
// Spin up another XE with shared resources
//
typedef struct {
    uint64_t entry_fn_ptr;  // in - initial PC
    uint64_t arg1;          // in - first arg (R0), thread entry fn for pthreads
    uint64_t arg2;          // in - second arg (R1)
    uint64_t stack_top;     // in - use this stack, 0 means CE allocate
    uint64_t pid;           // out - which XE is the victim
} clone_req;

//
// Wait for an XE to be done
//
typedef struct {
    uint64_t pid;         // in
    uint64_t ret;         // out
    uint64_t block;       // in
} waitpid_req;

//
// Automatically clean XE
//
typedef struct {
    uint64_t pid;           // in
} detach_req;

//
// Stop the XE running.
//
typedef struct {
    uint64_t ret;         // in
} finish_req;

//
// Redirect an XE to a cancellation routine
//
typedef struct {
    uint64_t pid;           // in
    uint64_t cancel_fn_ptr; // in
    uint64_t force_async;   // in
} cancel_req;

//
// Suspend an XE, waiting for a resume
//
typedef struct {
    uint64_t timeout; // in
    uint64_t sec;     // in
    uint64_t nsec;    // in
} suspend_req;

//
// Resume a suspended XE
//
typedef struct {
    uint64_t pid;           // in
} resume_req;


#endif // __ce_xe_intf_h__

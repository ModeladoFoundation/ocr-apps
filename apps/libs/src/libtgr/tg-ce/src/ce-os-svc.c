#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>

#include <tg-arch.h>
#include <tg-console.h>
#include <tg-remap.h>

#include "tgr-ce.h"
#include "util.h"
#include "ce-os-svc.h"

#define newlib_stat stat

#include "ce-os.h"  // from the fsim source

//
// Initialize for intercept
//
void ce_os_svc_init()
{
    remap_intercept();
}

//
// XXX The low level interface should be factored out to call common setup
// and execute fns to more easily virtualize the implementation
//
//
// This is nominally opaque to the syscall fns below
//
typedef struct {
    OSReqBuffer * osreq;
    size_t max_size;
} OSReq;

OSReq * ce_os_init_req( uint64_t type, size_t max_size )
{
    return NULL;
}

uint8_t * ce_os_copy_to_req( OSReq * req, off_t offset, uint8_t * from_buf, size_t len )
{
    return NULL;
}

int ce_os_copy_from_req( OSReq * req, off_t offset, uint8_t * to_buf, size_t len )
{
    return 0;
}

int ce_os_execute_req( OSReq * req )
{
    return 0;
}

///////////////////////////// Syscall Interfaces ///////////////////////////////
//
// interfaces to the intercept 'device'
//
int ce_os_getcwd( char * fname, size_t max_size )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_GETCWD;
    osreq->req_size = sizeof(getcwd_req);

    getcwd_req * req = (getcwd_req *) osreq->req_data;
    req->size = max_size;
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "getcwd status = 0x%llx\n", value);

    if (value == 0)
        memcpy( fname, req->buffer, max_size );

    return value;
}

int ce_os_chdir( const char * fname )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_CHDIR;
    osreq->req_size = sizeof(chdir_req);

    chdir_req * req = (chdir_req *) osreq->req_data;
    req->size = strlen(fname);
    memcpy( req->buffer, fname, req->size + 1 );
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "chdir status = 0x%llx\n", value);

    return value;
}

int ce_os_chmod( const char * fname, u64 mode )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_CHMOD;
    osreq->req_size = sizeof(chmod_req);

    chmod_req * req = (chmod_req *) osreq->req_data;
    req->mode = mode;
    req->size = strlen(fname);
    memcpy( req->buffer, fname, req->size + 1 );
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "chmod status = 0x%llx\n", value);

    return value;
}


int ce_os_chown( const char * fname, u64 owner, u64 group )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_CHOWN;
    osreq->req_size = sizeof(chown_req);

    chown_req * req = (chown_req *) osreq->req_data;
    req->owner = owner;
    req->group = group;
    req->size = strlen(fname);
    memcpy( req->buffer, fname, req->size + 1 );
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "chown status = 0x%llx\n", value);

    return value;
}

int ce_os_link( const char * oldpath, const char * newpath )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_LINK;
    osreq->req_size = sizeof(link_req);

    link_req * req = (link_req *) osreq->req_data;
    req->old_size = strlen(oldpath);
    req->new_size = strlen(newpath);
    memcpy( req->buffer, oldpath, req->old_size + 1 );
    memcpy( req->buffer + req->old_size + 1, newpath, req->new_size + 1 );
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "link status = 0x%llx\n", value);

    return value;
}

int ce_os_symlink( const char * oldpath, const char * newpath )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_SYMLINK;
    osreq->req_size = sizeof(symlink_req);

    symlink_req * req = (symlink_req *) osreq->req_data;
    req->old_size = strlen(oldpath);
    req->new_size = strlen(newpath);
    memcpy( req->buffer, oldpath, req->old_size + 1 );
    memcpy( req->buffer + req->old_size + 1, newpath, req->new_size + 1 );
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "symlink status = 0x%llx\n", value);

    return value;
}

int ce_os_unlink( const char * fname )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_UNLINK;
    osreq->req_size = sizeof(unlink_req);

    unlink_req * req = (unlink_req *) osreq->req_data;
    req->size = strlen(fname);
    memcpy( req->buffer, fname, req->size + 1 );
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "unlink status = 0x%llx\n", value);

    return value;
}

//
// fileopen is the only call that returns a value (resulting fd)
// instead of just status
//
int ce_os_fileopen( const char * fname, u64 flags, u64 mode )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_FILEOPEN;
    osreq->req_size = sizeof(fileopen_req);

    fileopen_req * req = (fileopen_req *) osreq->req_data;
    req->flags = flags;
    req->mode = mode;
    req->size = strlen(fname);
    memcpy( req->buffer, fname, req->size + 1 );
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "fileopen status = 0x%llx\n", value);

    return (value == 0) ? req->fd : value;
}

int ce_os_fileclose( int fd )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_FILECLOSE;
    osreq->req_size = sizeof(fileclose_req);

    fileclose_req * req = (fileclose_req *) osreq->req_data;
    req->fd = fd;
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "fileclose status = 0x%llx\n", value);

    return value;
}

//
// File read - check out the addr of 'buf' and see if it is a block
// local address (we can just use that directly) or a socket level resource
// which will require sequential reads into block (L2) followed by dma to
// the final destination.
//
ssize_t ce_os_fileread( int fd, void * buf, size_t count )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_FILEREAD;
    osreq->req_size = sizeof(fileread_req);

    fileread_req * req = (fileread_req *) osreq->req_data;
    req->fd = fd;
    req->addr = (uint64_t) buf;
    req->len = count;
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "fileread status = 0x%llx\n", value);

    if( value == 0 )
        value = req->len;
    return value;
}

ssize_t ce_os_filewrite( int fd, void * buf, size_t count )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_FILEWRITE;
    osreq->req_size = sizeof(filewrite_req);

    filewrite_req * req = (filewrite_req *) osreq->req_data;
    req->fd = fd;
    req->addr = (uint64_t) buf;
    req->len = count;
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "filewrite status = 0x%llx\n", value);

    if( value == 0 )
        value = req->len;
    return value;
}

//
// This is different from the std form in that we return the offset via
// the incoming offset reference.
//
int ce_os_filelseek( int fd, off_t * offset, int whence )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_FILELSEEK;
    osreq->req_size = sizeof(filelseek_req);

    filelseek_req * req = (filelseek_req *) osreq->req_data;
    req->fd = fd;
    req->offset = *offset;
    req->whence = whence;
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "filelseek status = 0x%llx\n", value);

    *offset = req->offset;

    return value;
}

int ce_os_filestat( const char * fname, struct stat * stat )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_FILESTAT;
    osreq->req_size = sizeof(filestat_req);

    filestat_req * req = (filestat_req *) osreq->req_data;
    req->size = strlen(fname);
    memcpy( req->buffer, fname, req->size + 1);
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "filestat status = 0x%llx\n", value);

    if ( value == 0 )
        memcpy( stat, & req->stat, sizeof(*stat) );

    return value;
}

int ce_os_filefstat( int fd, struct stat * stat )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_FILEFSTAT;
    osreq->req_size = sizeof(filefstat_req);

    filefstat_req * req = (filefstat_req *) osreq->req_data;
    req->fd = fd;
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "filefstat status = 0x%llx\n", value);

    if ( value == 0 )
        memcpy( stat, & req->stat, sizeof(*stat) );

    return value;
}

int ce_os_gettimeofday( struct timeval * tv )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_GETTIMEOFDAY;
    osreq->req_size = sizeof(gettimeofday_req);

    gettimeofday_req * req = (gettimeofday_req *) osreq->req_data;
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "gettimeofday status = 0x%llx\n", value);

    if ( value == 0 )
        memcpy( tv, & req->timeval, sizeof(*tv) );

    return value;
}

int ce_os_gethostname( char * buf, size_t max_size )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_GETHOSTNAME;
    osreq->req_size = sizeof(gethostname_req) + max_size;

    gethostname_req * req = (gethostname_req *) osreq->req_data;
    req->size = max_size;
    //
    // reading triggers execution
    //
    u64 value = osreq->status;

    if (value == 0)
        memcpy( buf, req->buffer, max_size );
    // ce_print("OS", "gethostname status = 0x%llx\n", value);

    return value;
}

int ce_os_mkdir( const char * dirname, u64 mode )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_MKDIR;
    osreq->req_size = sizeof(mkdir_req);

    mkdir_req * req = (mkdir_req *) osreq->req_data;
    req->size = strlen(dirname);
    req->mode = mode;
    memcpy( req->buffer, dirname, req->size + 1);
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "mkdir status = 0x%llx\n", value);

    return value;
}

int ce_os_rmdir( const char * dirname )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_RMDIR;
    osreq->req_size = sizeof(rmdir_req);

    rmdir_req * req = (rmdir_req *) osreq->req_data;
    req->size = strlen(dirname);
    memcpy( req->buffer, dirname, req->size + 1);
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "rmdir status = 0x%llx\n", value);

    return value;
}

int ce_os_isatty( int fd )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_ISATTY;
    osreq->req_size = sizeof(isatty_req);

    isatty_req * req = (isatty_req *) osreq->req_data;
    req->fd = fd;
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS",  "isatty status = 0x%llx\n", value);

    return value;
}

int ce_os_readlink( const char * linkpath, char * linkvalue, size_t max_size )
{
    //
    // setup our request
    //
    volatile OSReqBuffer * osreq = (OSReqBuffer *) INTERCEPT_START;
    osreq->status = 0;
    osreq->req_type = CE_REQTYPE_READLINK;
    osreq->req_size = sizeof(readlink_req);

    readlink_req * req = (readlink_req *) osreq->req_data;
    req->lname_size = strlen(linkpath);
    req->buf_size = max_size;
    memcpy( req->buffer, linkpath, req->lname_size + 1 );
    //
    // reading triggers execution
    //
    u64 value = osreq->status;
    // ce_print("OS", "readlink status = 0x%llx\n", value);

    if( value == 0 )
        memcpy( linkvalue, req->buffer, max_size );

    return value;
}


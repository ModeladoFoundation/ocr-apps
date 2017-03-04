#include "tg-types.h"
#include "tg-console.h"

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>


#include "memory.h"
#include "tgr-ce.h"
#include "ce-xe-intf.h"
#include "ce-os-svc.h"
#include "util.h"
#include "mem-util.h"

#include "xe-abi.h"

//
// XXX Unimplemented methods
//
int return_mem( xe_info * xei, uint64_t va );
int alloc_stack_mem( xe_info *xei, int len );

//
// This is an interface for XEs that provides access to
// memory and file resources via the CE.
// Note that this implementation is single threaded.
//
static mem_type mem_type_map[] = {
    Mem_L1,  // MREQ_LOCAL
    Mem_L2,  // MREQ_BLOCK
    Mem_IPM, // MREQ_GLOBAL
    Mem_ANY, // MREQ_ANY
 };

///////////////////////  Interfaces ///////////////////////
//
// Allocate some memory
//
static int do_memalloc_req( xe_info * xei, void * params )
{
    memalloc_req *  req = params;
    //
    // round len up to 8 byte boundary
    //
    uint64_t len = (req->len + 7) & ~7UL;
    //
    // validate
    //
    if( req->region < 0 || req->region > MREQ_LAST ) {
        ce_error( "MSG", "XE 0x%lx memalloc err: invalid region %d\n",
                    xei->id.all, req->region);
        return EINVAL;
    }
    //
    // Get some memory
    //
    int region  = (req->region == MREQ_ANY) ? MREQ_LOCAL : req->region;
    int promote = (req->region == MREQ_ANY) ? 1 : req->promote;

    do {
        //
        // If this succeeds, the seg will be queued on xei
        //
        mem_seg * mem = xe_alloc_mem( xei, mem_type_map[ region ], len );

        if( mem != NULL ) {
            ce_vprint( "MSG", "XE%x memalloc va 0x%lx, len 0x%lx\n",
                    xei->id.all, mem->va, mem->len );
            mem->private = req->private;
            //
            // set our 'outs' for write-back to the XE
            //
            req->va = mem->va;
            req->len = mem->len;

            return 0;
        }
    } while( promote && ++region <= MREQ_GLOBAL );
    //
    // didn't succeed, give the client the bad news
    //
    ce_error( "MSG", "XE 0x%lx memalloc err: can't alloc %d bytes of mem\n",
                xei->id.all, req->len);

    return ENOMEM;
}

//
// Free previously allocated memory - should have a resource instance
//
static int do_memfree_req( xe_info * xei, void * params )
{
    memfree_req * req = params;

    if( return_mem( xei, req->va ) ) {
        ce_error( "MSG", "XE 0x%lx memfree err: trying to free unallocated mem 0x%lx\n",
                 xei->id.all, req->va);
        return EINVAL;
    }
    return 0;
}

//
// Get the CWD
//
static int do_getcwd_req( xe_info * xei, void * params )
{
    getcwd_req * req = params;

    char * path = (char *) validate_xe_addr( xei, req->buf_ptr, req->size );

    if( path == NULL ) {
        ce_error( "MSG", "XE 0x%lx getcwd err: invalid path addr\n", xei->id.all );
        return EINVAL;
    }
    int status = ce_os_getcwd( path, (size_t) req->size );

    if( status < 0 ) {
        ce_error( "MSG", "XE 0x%lx getcwd err: failed (%d)\n", xei->id.all, status );
    }
    return status;
}

//
// Change the CWD
//
static int do_chdir_req( xe_info * xei, void * params )
{
    chdir_req * req = params;

    char * path = (char *) validate_xe_addr( xei, req->path_ptr, req->path_len + 1 );

    if( path == NULL ) {
        ce_error( "MSG", "XE 0x%lx chdir err: invalid path addr\n", xei->id.all );
        return EINVAL;
    }
    path[req->path_len] = '\0';

    int status = ce_os_chdir( path );

    if( status < 0 ) {
        ce_error( "MSG", "XE 0x%lx chdir err: failed to chdir to `%s' (%d)\n",
                    xei->id.all, path, status );
    }
    return status;
}

//
// Change the permissions of a file
//
static int do_chmod_req( xe_info * xei, void * params )
{
    chmod_req * req = params;

    char * path = (char *) validate_xe_addr( xei, req->path_ptr, req->path_len + 1 );

    if( path == NULL ) {
        ce_error( "MSG", "XE 0x%lx chmod err: invalid path addr\n", xei->id.all );
        return EINVAL;
    }
    path[req->path_len] = '\0';

    int status = ce_os_chmod( path, (mode_t) req->mode );

    if( status < 0 ) {
        ce_error( "MSG", "XE 0x%lx chmod err: failed to chmod `%s' to %u (%d)\n",
                    xei->id.all, path, req->mode, status);
    }
    return status;
}

//
// Change the ownership of a file
//
static int do_chown_req( xe_info * xei, void * params )
{
    chown_req * req = params;

    char * path = (char *) validate_xe_addr( xei, req->path_ptr, req->path_len + 1 );

    if( path == NULL ) {
        ce_error( "MSG", "XE 0x%lx chown err: invalid path addr\n", xei->id.all );
        return EINVAL;
    }
    path[req->path_len] = '\0';

    int status = ce_os_chown( path, (uid_t) req->owner, (gid_t) req->group );

    if( status < 0 ) {
        ce_error( "MSG", "XE 0x%lx chown err: failed to chown `%s' to %u:%u (%d)\n", xei->id.all,
                 path, req->owner, req->group, status);
    }
    return status;
}

//
// Create a new name for a file
//
static int do_link_req( xe_info * xei, void * params )
{
    link_req * req = params;

    char * opath = (char *) validate_xe_addr( xei, req->oldpath_ptr, req->oldpath_len + 1 );

    if( opath == NULL ) {
        ce_error( "MSG", "XE 0x%lx link err: invalid oldpath addr\n", xei->id.all );
        return EINVAL;
    }
    opath[req->oldpath_len] = '\0';

    char * npath = (char *) validate_xe_addr( xei, req->newpath_ptr, req->newpath_len + 1 );

    if( npath == NULL ) {
        ce_error( "MSG", "XE 0x%lx link err: invalid newpath addr\n", xei->id.all );
        return EINVAL;
    }
    npath[req->newpath_len] = '\0';

    int status = ce_os_link( opath, npath );

    if( status < 0 ) {
        ce_error( "MSG", "XE 0x%lx link err: failed to link `%s' to `%s' (%d)\n",
                    xei->id.all, opath, npath, status );
    }
    return status;

}

//
// Create a symlink for a file
//
static int do_symlink_req( xe_info * xei, void * params )
{
    symlink_req * req = params;

    char * opath = (char *) validate_xe_addr( xei, req->oldpath_ptr, req->oldpath_len + 1 );

    if( opath == NULL ) {
        ce_error( "MSG", "XE 0x%lx symlink err: invalid oldpath addr\n", xei->id.all );
        return EINVAL;
    }
    opath[req->oldpath_len] = '\0';

    char * npath = (char *) validate_xe_addr( xei, req->newpath_ptr, req->newpath_len + 1 );

    if( npath == NULL ) {
        ce_error( "MSG", "XE 0x%lx symlink err: invalid newpath addr\n", xei->id.all );
        return EINVAL;
    }
    npath[req->newpath_len] = '\0';

    int status = ce_os_symlink( opath, npath );

    if( status < 0 ) {
        ce_error( "MSG", "XE 0x%lx symlink err: failed to symlink `%s' to `%s' (%d)\n",
                    xei->id.all, opath, npath, status );
    }
    return status;
}

//
// Remove a name for a file
//
static int do_unlink_req( xe_info * xei, void * params )
{
    unlink_req * req = params;

    char * path = (char *) validate_xe_addr( xei, req->path_ptr, req->path_len + 1 );

    if( path == NULL ) {
        ce_error( "MSG", "XE 0x%lx unlink err: invalid path addr %p\n",
                    xei->id.all, req->path_ptr );
        return EINVAL;
    }
    path[req->path_len] = '\0';

    int status = ce_os_unlink(path);

    if( status < 0 ) {
        ce_error( "MSG", "XE 0x%lx unlink err: failed to unlink `%s' (%d)\n", xei->id.all,
                 path, status );
    }
    return status;
}

//
// File open
//
static int do_fileopen_req( xe_info * xei, void * params )
{
    fileopen_req * req = params;

    char * fname = (char *) validate_xe_addr( xei, req->fname_ptr, req->fname_len + 1 );

    if( fname == NULL ) {
        ce_error( "MSG", "XE 0x%lx open err: invalid path addr %p\n", xei->id.all, req->fname_ptr );
        return EINVAL;
    }
    fname[req->fname_len] = '\0';

    int fd = ce_os_fileopen( fname, req->flags, req->mode );

    if( fd < 0 ) {
        ce_error( "MSG", "XE 0x%lx fileopen err: failed (%d)\n", xei->id.all, fd );
        return fd;
    } else
        req->fd = fd;

    return 0;
}

//
// File close
//
static int do_fileclose_req( xe_info * xei, void * params )
{
    fileclose_req * req = params;

    ce_vprint( "MSG", "XE 0x%lx closing fd %d\n", xei->id.all, req->fd);

    int status = ce_os_fileclose( req->fd );

    return status;
}
//
// read
//
static int do_fileread_req( xe_info * xei, void * params )
{
    fileread_req * req = params;
    //
    // pass to CE OS SVC
    //
    void * va = (void *) validate_xe_addr( xei, req->buf, req->len );

    if( va == NULL ) {
        ce_error( "MSG", "XE 0x%lx fileread err: invalid buf addr\n", xei->id.all );
        return EINVAL;
    }
    ssize_t didread = ce_os_fileread( req->fd, va, req->len );

    if( didread < 0 ) {
        ce_error( "MSG", "XE 0x%lx fileread err: read from fd %d failed (%d)\n",
                xei->id.all, req->fd, didread );
        return didread;
    } else {
        req->len = didread;
        return 0;
    }
}
//
// write
//
static int do_filewrite_req( xe_info * xei, void * params )
{
    filewrite_req * req = params;
    //
    // repeatedly write until the count is exhausted
    //
    void * va = (void *) validate_xe_addr( xei, req->buf, req->len );

    if( va == NULL ) {
        ce_error( "MSG", "XE 0x%lx filewrite err: invalid buf addr\n", xei->id.all );
        return EINVAL;
    }
    ssize_t didwrite = ce_os_filewrite( req->fd, va, req->len );

    if( didwrite < 0 ) {
        ce_error( "MSG", "XE 0x%lx filewrite err: write to fd %d failed (%d)\n",
                xei->id.all, req->fd, didwrite );
        return didwrite;
    } else {
        req->len = didwrite;
        return 0;
    }
}

//
// lseek
//
static int do_filelseek_req( xe_info * xei, void * params )
{
    filelseek_req * req = params;

    off_t offset = req->offset;

    int status = ce_os_filelseek( req->fd, & offset, req->whence );

    if( status ) {
        ce_error( "MSG", "XE 0x%lx filelseek err: seek fd %d failed (%d)\n",
                xei->id.all, req->fd, status );
        return status;
    }
    req->offset = offset;

    return 0;
}

//
// stat
//
static int do_filestat_req( xe_info * xei, void * params )
{
    filestat_req * req = params;

    char * fname = (char *) validate_xe_addr( xei, req->fname_ptr, req->fname_len + 1 );

    if( fname == NULL ) {
        ce_error( "MSG", "XE 0x%lx filestat err: invalid filename addr\n", xei->id.all );
        return EINVAL;
    }
    fname[req->fname_len] = '\0';

    if( req->stat_len != sizeof(struct stat) ) {
        ce_error( "MSG", "XE 0x%lx filestat err: stat struct wrong size - %d\n",
                        xei->id.all, req->stat_len);
        return EINVAL;
    }
    struct stat * statb;

    statb = (void *) validate_xe_addr( xei, req->stat_ptr, req->stat_len );

    if( statb == NULL ) {
        ce_error( "MSG", "XE 0x%lx filestat err: invalid stat addr\n", xei->id.all );
        return EINVAL;
    }
    int status = ce_os_filestat( (const char *) fname, statb );

    if( status < 0 ) {
        ce_error( "MSG", "XE 0x%lx filestat err: stat of file '%s' failed\n",
                    xei->id.all, fname );
    }
    return status;
}
//
// fstat
//
static int do_filefstat_req( xe_info * xei, void * params )
{
    filefstat_req * req = params;

    struct stat * statb;

    statb = (void *) validate_xe_addr( xei, req->stat_ptr, req->stat_len );

    if( statb == NULL ) {
        ce_error( "MSG", "XE 0x%lx filefstat err: invalid stat addr\n", xei->id.all );
        return EINVAL;
    }
    if( req->stat_len != sizeof(struct stat) ) {
        ce_error( "MSG", "XE 0x%lx filefstat err: fstat struct wrong size - %d\n",
                xei->id.all, req->stat_len);
        return EINVAL;
    }
    int status = ce_os_filefstat( req->fd, statb );

    if( status < 0 ) {
        ce_error( "MSG", "XE 0x%lx filefstat err: fstat of fd %d failed\n", xei->id.all, req->fd );
        return status;
    }
    return status;
}

//
// gettimeofday
//
static int do_gettimeofday_req( xe_info * xei, void * params )
{
    gettimeofday_req * req = params;

    struct timeval * tv;

    tv = (void *) validate_xe_addr( xei, req->timeval_ptr, sizeof(struct timeval) );

    if( tv == NULL ) {
        ce_error( "MSG", "XE 0x%lx gettimeofday err: invalid timeval addr\n", xei->id.all );
        return EINVAL;
    }

    int status = ce_os_gettimeofday( tv );

    if( status < 0 ) {
        ce_error( "MSG", "XE 0x%lx gettimeofday err: failed (%d)\n", xei->id.all, status );
    }
    return status;
}

//
// Get hostname
//
static int do_gethostname_req( xe_info * xei, void * params )
{
    gethostname_req * req = params;

    char * buf = (char *) validate_xe_addr( xei, req->hname_ptr, req->size );

    if( buf == NULL ) {
        ce_error( "MSG", "XE 0x%lx gethostname err: invalid buf addr %p\n",
                xei->id.all, req->hname_ptr );
        return EINVAL;
    }
    int status = ce_os_gethostname( buf, (size_t) req->size );

    if( status < 0 ) {
        ce_error( "MSG", "XE 0x%lx gethostname err: failed (%d)\n", xei->id.all, status );
    }
    return status;
}

//
// Test if a file descriptor refers to a terminal
//
static int do_isatty_req( xe_info * xei, void * params )
{
    isatty_req * req = params;

    return ce_os_isatty( (int)req->fd );
}

//
// Make directory
//
static int do_mkdir_req( xe_info * xei, void * params )
{
    mkdir_req * req = params;

    char * str = (char *) validate_xe_addr( xei, req->path_ptr, req->path_len + 1 );

    if( str == NULL ) {
        ce_error( "MSG", "XE 0x%lx mkdir err: invalid path addr\n", xei->id.all );
        return EINVAL;
    }
    str[req->path_len] = '\0';

    int status = ce_os_mkdir( str, req->mode );

    if( status < 0 ) {
        ce_error( "MSG", "XE 0x%lx mkdir err: failed to create `%s' (%d)\n", xei->id.all,
                 req->path_ptr, status );
    }
    return status;
}

//
// Remove a directory
//
static int do_rmdir_req( xe_info * xei, void * params )
{
    rmdir_req * req = params;

    char * str = (char *) validate_xe_addr( xei, req->path_ptr, req->path_len + 1 );

    if( str == NULL ) {
        ce_error( "MSG", "XE 0x%lx rmdir err: invalid path addr\n", xei->id.all );
        return EINVAL;
    }
    str[req->path_len] = '\0';

    int status = ce_os_rmdir( str );

    if( status < 0 ) {
        ce_error( "MSG", "XE 0x%lx rmdir err: failed to remove `%s' (%d)\n", xei->id.all,
                 req->path_ptr, status );
    }
    return status;
}

//
// Get the value of a symbolic link
//
static int do_readlink_req( xe_info * xei, void * params )
{
    readlink_req * req = params;

    size_t size = (size_t) req->size;
    char * buffer = (char *) validate_xe_addr( xei, req->buf_ptr, req->size );

    if( buffer == NULL ) {
        ce_error( "MSG", "XE 0x%lx readlink err: invalid buffer addr\n", xei->id.all );
        return EINVAL;
    }

    char * str = (char *) validate_xe_addr( xei, req->path_ptr, req->path_len + 1 );

    if( str == NULL ) {
        ce_error( "MSG", "XE 0x%lx readlink err: invalid path addr\n", xei->id.all );
        return EINVAL;
    }
    str[req->path_len] = '\0';

    ssize_t read_count = ce_os_readlink( str, buffer, size );

    if( read_count < 0 ) {
        ce_error( "MSG", "XE 0x%lx readlink err: readlink failed\n", xei->id.all );
    }
    return read_count;
}

//
// Return the caller's XE id (pid)
//
static int do_getpid_req( xe_info * xei, void * params )
{
    getpid_req * req = params;

    req->pid = xei->id.all;

    return 0;
}

//
// Select an idle XE, initialize it's meta and actual (stack, regs) state
// and start it running at the supplied entry point with the
// supplied arguments.
// Returns EAGAIN if no XEs are available.
//
static int do_clone_req( xe_info * xei, void * params )
{
    clone_req * req = params;

    ce_print("MSG", "clone entry\n");
    //
    // Select a victim
    //
    xe_info * new_xei = xe_get_info_by_state( xei->block, NULL, XE_UNUSED );

    if( new_xei == NULL ) {
        ce_error( "MSG", "XE 0x%lx clone err: Out of free XEs\n", xei->id.all );
        return EAGAIN;
    }
    //
    // Initialize our new XE
    //
    new_xei->blocked = false;
    new_xei->detached = false;
    new_xei->resumes = 0;
    new_xei->use_state = XE_RUNNING;
    new_xei->waiting_xei = NULL;
    new_xei->waiting_for = NULL;
    //
    // Init the stack state
    //
    uint64_t stack_top = req->stack_top;

    if( ! stack_top ) {
        if( ! new_xei->stack ) {
            size_t stack_size = xei->block->ce->config->xe_stack_size;

            new_xei->stack = xe_alloc_mem( new_xei, Mem_ANY, stack_size );
            if( new_xei->stack == NULL ) {
                ce_error( "MSG", "XE 0x%lx clone err: Can't get stack\n", xei->id.all );
                new_xei->use_state = XE_UNUSED;
                return ENOMEM;
            }
            new_xei->stack_top = new_xei->stack->va + new_xei->stack->len;
        }
        stack_top = new_xei->stack_top;
    }
    ce_print( "MSG", "XE 0x%lx: Cloned XE 0x%lx, entry %p, stack top %p\n",
                xei->id.all, new_xei->id.all, req->entry_fn_ptr, stack_top);
    xe_set_sp( new_xei, stack_top );
    xe_set_ra( new_xei, 0UL );
    //
    // Start the XE at the entry (pthreads - trampoline) func. This func
    // takes two parameters: for pthreads a function and an argument for
    // that function.
    //
    xe_set_pc( new_xei, req->entry_fn_ptr );
    xe_set_reg( new_xei, 0, req->arg1 );  // Argument 1 (fn)
    xe_set_reg( new_xei, 1, req->arg2 );  // Argument 2 (arg)
    //
    // We are go for launch...
    //
    xe_continue( new_xei );

    xei->block->running++;

    req->pid = new_xei->id.all;

    ce_vprint( "MSG", "XE 0x%lx: Cloned XE 0x%lx, entry %p\n",
                xei->id.all, new_xei->id.all, req->entry_fn_ptr);
    return 0;
}

//
// Wait for an XE to be done (executed a 'finish' call).
// We can't wait for an unused or detached XE.
// A finished XE is transitioned to unused.
// The wait can be either blocking or non-blocking. If this is a blocking wait
// we register our wait with the victim and let it complete our syscall.
//
static int do_waitpid_req( xe_info * xei, void * params )
{
    waitpid_req * req = params;
    xe_info *     wait_xei = xe_get_info_by_id( xei->block, (id_tuple) req->pid );

    if( wait_xei ) {
        //
        // Check if XE is in_use.
        //
        if( wait_xei->use_state != XE_RUNNING &&
            wait_xei->use_state != XE_FINISHED ) {
            ce_error( "MSG", "XE 0x%lx waitpid err: XE 0x%lx is unused.\n",
                        xei->id.all, req->pid );
            return ESRCH;
        }
        //
        // Check if XE is detached
        //
        if( wait_xei->detached ) {
            ce_error( "MSG", "XE 0x%lx waitpid err: XE 0x%lx is detached.\n",
                        xei->id.all, req->pid );
            return EPERM;
        }
        //
        // Check if the XE is waiting to be reaped
        //
        if( wait_xei->use_state == XE_FINISHED ) {
            wait_xei->use_state = XE_UNUSED;
            req->ret = wait_xei->exit_code;
            ce_vprint( "MSG", "XE 0x%lx: reaped XE %lx.\n", xei->id.all, req->pid );
        //
        // still running, block for it?
        //
        } else if( req->block ) {
            wait_xei->waiting_xei = xei;
            xei->waiting_for = wait_xei;
            xei->wait_req_addr = params;
            xei->blocked = true;
            ce_vprint( "MSG", "XE 0x%lx: waiting for XE 0x%lx.\n",
                        xei->id.all, req->pid );
        //
        // The victim is not ready to be reaped and we are not going to wait for it.
        //
        } else {
            return EAGAIN;
        }
    } else {
        ce_error( "MSG", "XE 0x%lx waitpid err: Could not find XE 0x%lx\n",
                    xei->id.all, req->pid);
        return ESRCH;
    }
    return 0;
}

//
// Automatically reap an XE
//
static int do_detach_req( xe_info * xei , void * params )
{
    detach_req * req = params;
    xe_info *    detach_xei = xe_get_info_by_id( xei->block, (id_tuple) req->pid );

    if( detach_xei ) {
        //
        // Check if XE is in_use.
        //
        if( detach_xei->use_state != XE_RUNNING &&
            detach_xei->use_state != XE_FINISHED) {
            ce_error( "MSG", "XE 0x%lx detach err: XE %lx is unused.\n",
                        xei->id.all, req->pid );
            return ESRCH;
        }
        //
        // Check if this XE is already being waited for
        //
        if( detach_xei->waiting_xei != NULL ) {
            ce_error( "MSG", "XE 0x%lx detach err: XE %d being waited for.\n",
                     xei->id.all, detach_xei->id.all );
            return EPERM;
        }
        detach_xei->detached = true;
        //
        // if the victim has already finished, just transition to unused.
        //
        if (detach_xei->use_state == XE_FINISHED) {
            detach_xei->use_state = XE_UNUSED;
            ce_vprint( "MSG", "XE 0x%lx: reaped XE %d.\n", xei->id.all, detach_xei->id.all );
        //
        // a simple detach
        //
        } else {
            ce_vprint( "MSG", "XE 0x%lx: detached XE %d.\n", xei->id.all, detach_xei->id.all );
        }
    } else {
        ce_error( "MSG", "XE 0x%lx detach err: Could not find XE %ld \n",
                    xei->id.all, req->pid );
        return ESRCH;
    }
    return 0;
}

//
// This XE has completed it's processing
// Notify any blocked waiter. We enter FINISHED state until we've been
// waited for, unless we've previously been detached.
// If detached or waited for, we enter UNUSED state and are available
// for allocation.
//
static int do_finish_req( xe_info * xei , void * params )
{
    finish_req * req = params;

    xei->exit_code = req->ret;
    //
    // This blocking call will actually never return. The XE will have its
    // PC changed and then be unblocked when another XE 'clones' it.
    //
    xei->blocked = true;
    //
    // If someone is waiting to reap us, complete their wait
    //
    if( xei->waiting_xei != NULL ) {
        waitpid_req  waiting_req = {xei->id.all, req->ret};
        xe_info *    waiting_xei = xei->waiting_xei;

        xei->waiting_xei = NULL;
        waiting_xei->blocked = false;
        ce_vprint( "MSG", "XE 0x%lx finishing, waking XE 0x%lx, wait_req 0x%lx.\n",
                xei->id.all, waiting_xei->id.all, waiting_xei->wait_req_addr );
        //
        // Since we are being waited for, we may set our state to be unused.
        //
        xei->use_state = XE_UNUSED;
        //
        // We must write-back the return value for the waiting XE and unblock it.
        // We've already validated the address, so just write it out and continue
        //
        memcpy( waiting_xei->wait_req_addr, & waiting_req, sizeof(waiting_req) );

        xe_resume( waiting_xei, 0 );
    //
    // we've been 'backgrounded'. Just quietly fade away ...
    //
    } else if( xei->detached ) {
        xei->use_state = XE_UNUSED;
        xei->detached = false;
        ce_vprint( "MSG", "XE 0x%lx: reaped because it was detached.\n", xei->id.all );
    //
    // transition to FINISHED so someone can reap us
    //
    } else {
        xei->use_state = XE_FINISHED;
        ce_vprint( "MSG", "XE 0x%lx has finished.\n", xei->id.all );
    }
    //
    // if only the initial XE running, complete an outstanding waitall
    //
    if( --xei->block->running == 1 && xei->block->doing_waitall ) {
        xe_info * xei0 = xei->block->xes;
        //
        // First reap all finished XEs
        //
        for( xei = xei0 ; (xei = xe_get_next_info( xei->block, xei )) != NULL ; ) {
            if( xei->use_state == XE_FINISHED ) {
                xei->blocked = false;
                xei->detached = false;
                xei->use_state = XE_UNUSED;
            }
        }
        ce_vprint( "MSG", "waitall satisfied, resuming XE 0x%lx.\n", xei0->id.all );
        xei0->blocked = false;
        xei0->block->doing_waitall = false;
        xe_resume( xei0, 0 );
    } else
        ce_vprint( "MSG", "finish - %d still running\n", xei->block->running );

    return 0;
}

//
// Terminate all XE's, except for the calling one.
//
static int do_killall_req( xe_info * xei , void * params )
{
    block_info * bi = xei->block;
    xe_info *    kill_xei = bi->xes;

    ce_vprint( "MSG", "XE 0x%lx: killall\n", xei->id.all );

    for( ; kill_xei < bi->xes + bi->xe_count ; kill_xei++ ) {
        if( kill_xei != xei )
            xe_terminated( kill_xei );
    }
    return 0;
}

//
// Wait for all other XE's to finish
//
static int do_waitall_req( xe_info * xei , void * params )
{
    if( xei->id.agent != 1) {
        ce_error( "MSG", "XE 0x%lx: Attempted waitall with non-main XE.\n", xei->id.all );
        return EPERM;
    }

    if( xei->block->running > 1 ) {
        xei->blocked = true;
        xei->block->doing_waitall = true;
    }
    return 0;
}

//
// Redirect an XE to a cancellation routine
//
static int do_cancel_req( xe_info * xei , void * params )
{
    cancel_req * req = params;
    xe_info * cancel_xei = xe_get_info_by_id( xei->block, (id_tuple) req->pid );

    if( cancel_xei ) {
        //
        // Check if XE is in_use.
        //
        if( cancel_xei->use_state != XE_RUNNING &&
            cancel_xei->use_state != XE_FINISHED ) {
            ce_error( "MSG", "XE 0x%lx err: XE %ld is not cancelable.\n",
                        xei->id.all, req->pid );
            return ESRCH;
        }
        xe_stop( cancel_xei );
        xe_set_pc( cancel_xei, req->cancel_fn_ptr );
        xe_set_ra( cancel_xei, 0UL );
        //
        // Just in case it is blocked on an alarm
        //
        cancel_xei->blocked = 0;

        if( cancel_xei->waiting_for ) {
            //
            // This xe is being canceled, and can no longer wait to cleanup
            //
            cancel_xei->waiting_for->waiting_xei = NULL;
        }
        xe_continue( cancel_xei );
    } else {
        ce_error( "MSG", "XE 0x%lx err: Could not find cancel XE %ld \n",
                    cancel_xei->id.all, req->pid );
        return ESRCH;
    }
    return 0;
}

//
// Suspend the requesting XE, waiting for a resume
//
static int do_suspend_req( xe_info * xei , void * params )
{
    suspend_req * req = params;

    if( xei->resumes-- == 0 ) {
        xei->blocked = true;

        if (req->timeout) {
            xei->suspend_time.tv_sec = req->sec;
            xei->suspend_time.tv_nsec = req->nsec;
            //
            // Get the current time to check if already timed out
            // XXX change to use CE time
            //
            struct timespec ts;
            ce_gettime( &ts );

            if( ! EARLIER(ts, xei->suspend_time) ) {
                xei->blocked = false;
                xei->resumes = 0;
                return ETIMEDOUT;
            }
        } else {
            xei->suspend_time.tv_sec = ((time_t)-1);
        }
    }
    return 0;
}

//
// Resume a suspended XE
//
static int do_resume_req( xe_info * xei , void * params )
{
    resume_req * req = params;
    xe_info * resume_xei = xe_get_info_by_id( xei->block, (id_tuple) req->pid );

    if( resume_xei ) {
        //
        // Check if XE is in_use.
        //
        if (resume_xei->use_state != XE_RUNNING ) {
            ce_error( "MSG", "XE 0x%lx resume err: XE 0x%lx is not running.\n",
                         xei->id.all, req->pid );
            return ESRCH;
        }
        //
        // If suspended, resume
        //
        if( ++resume_xei->resumes == 0 ) {
            xe_resume( resume_xei, 0 );
        }
    } else {
        ce_error( "MSG", "XE 0x%lx err: Could not find resume XE with id 0x%lx \n",
                    xei->id.all, req->pid );
        return ESRCH;
    }
    return 0;
}

///////////////////////  Main Interface ///////////////////////
//
// This table validates and vectors our requests to the appropriate handler.
// It is in ce_request_type enum order and indexed by type, not searched!
//
static struct req_methods {
    const char *    name;
    ce_request_type type;
    uint64_t        len;
    int             write_back;
    int             (*req_method)( xe_info *, void * );

} ReqMethods[] = {
    { "memalloc",     CE_REQTYPE_MEMALLOC,     sizeof(memalloc_req),     1, do_memalloc_req },
    { "memfree",      CE_REQTYPE_MEMFREE,      sizeof(memfree_req),      0, do_memfree_req },
    { "getcwd",       CE_REQTYPE_GETCWD,       sizeof(getcwd_req),       0, do_getcwd_req },
    { "chdir",        CE_REQTYPE_CHDIR,        sizeof(chdir_req),        0, do_chdir_req },
    { "chmod",        CE_REQTYPE_CHMOD,        sizeof(chmod_req),        0, do_chmod_req },
    { "chown",        CE_REQTYPE_CHOWN,        sizeof(chown_req),        0, do_chown_req },
    { "link",         CE_REQTYPE_LINK,         sizeof(link_req),         0, do_link_req },
    { "symlink",      CE_REQTYPE_SYMLINK,      sizeof(symlink_req),      0, do_symlink_req },
    { "unlink",       CE_REQTYPE_UNLINK,       sizeof(unlink_req),       0, do_unlink_req },
    { "open",         CE_REQTYPE_FILEOPEN,     sizeof(fileopen_req),     1, do_fileopen_req },
    { "close",        CE_REQTYPE_FILECLOSE,    sizeof(fileclose_req),    0, do_fileclose_req },
    { "read",         CE_REQTYPE_FILEREAD,     sizeof(fileread_req),     1, do_fileread_req },
    { "write",        CE_REQTYPE_FILEWRITE,    sizeof(filewrite_req),    1, do_filewrite_req },
    { "lseek",        CE_REQTYPE_FILELSEEK,    sizeof(filelseek_req),    1, do_filelseek_req },
    { "stat",         CE_REQTYPE_FILESTAT,     sizeof(filestat_req),     0, do_filestat_req },
    { "fstat",        CE_REQTYPE_FILEFSTAT,    sizeof(filefstat_req),    0, do_filefstat_req },
    { "gettimeofday", CE_REQTYPE_GETTIMEOFDAY, sizeof(gettimeofday_req), 0, do_gettimeofday_req },
    { "gethostname",  CE_REQTYPE_GETHOSTNAME,  sizeof(gethostname_req),  0, do_gethostname_req },
    { "isatty",       CE_REQTYPE_ISATTY,       sizeof(isatty_req),       0, do_isatty_req },
    { "mkdir",        CE_REQTYPE_MKDIR,        sizeof(mkdir_req),        0, do_mkdir_req },
    { "rmdir",        CE_REQTYPE_RMDIR,        sizeof(rmdir_req),        0, do_rmdir_req },
    { "readlink",     CE_REQTYPE_READLINK,     sizeof(readlink_req),     0, do_readlink_req },
    { "getpid",       CE_REQTYPE_GETPID,       sizeof(getpid_req),       1, do_getpid_req },
    { "clone",        CE_REQTYPE_CLONE,        sizeof(clone_req),        1, do_clone_req },
    { "waitpid",      CE_REQTYPE_WAITPID,      sizeof(waitpid_req),      1, do_waitpid_req },
    { "finish",       CE_REQTYPE_FINISH,       sizeof(finish_req),       1, do_finish_req },
    { "killall",      CE_REQTYPE_KILLALL,      0,                        0, do_killall_req },
    { "cancel",       CE_REQTYPE_CANCEL,       sizeof(cancel_req),       0, do_cancel_req },
    { "suspend",      CE_REQTYPE_SUSPEND,      sizeof(suspend_req),      0, do_suspend_req },
    { "resume",       CE_REQTYPE_RESUME,       sizeof(resume_req),       0, do_resume_req },
    { "detach",       CE_REQTYPE_DETACH,       sizeof(detach_req),       0, do_detach_req },
    { "waitall",      CE_REQTYPE_WAITALL,      0,                        0, do_waitall_req }
};


#define CE_REQ_MAX_PARAMS   8  // overkill
//
// Dispatch an XE request msg
// This is called by the tgr run loop after reading the args
// It will write back the status and continue the XE (if not blocked)
//
int ce_xe_msg_handler( xe_info * xei, uint64_t arg0, uint64_t arg1, uint64_t *status )
{
    ce_request_type type = CE_REQ_TYPE( arg0 );
    size_t len  = CE_REQ_LEN( arg0 );
    void * params = NULL;

    //
    // first do a bit of validation
    //
    if( type < CE_REQTYPE_FIRST && type >= CE_REQTYPE_LAST ) {
        ce_error( "MSG", "XE 0x%lx err: unknown req type %d\n", xei->id.all, type );
        *status = ENOSYS;
        return 1;
    }
    ce_vprint("MSG", "XE 0x%lx - arg0 0x%lx, arg1 %p, type %s (%d), len 0x%lx\n",
            xei->id.all, arg0, (void *) arg1, ReqMethods[type].name, type, len );

    if( len != ReqMethods[type].len  ) {
        ce_error( "MSG", "XE 0x%lx err: req %d (%s) length mismatch - %d != %d\n",
                 xei->id.all, type, ReqMethods[type].name, len, ReqMethods[type].len );
        *status = EINVAL;
        return 1;
    }
    if( len ) {
        params = (void *) validate_xe_addr( xei, arg1, len );
        if( params == NULL ) {
            ce_error( "MSG", "XE 0x%lx err: req address %p invalid\n",
                    xei->id.all, (void *) arg1 );
            *status = EINVAL;
            return 1;
        }
    }
    //
    // status will be pushed into an XE reg by our caller
    //
    *status = ReqMethods[type].req_method( xei, params );

    ce_vprint( "MSG", "XE 0x%lx status: 0x%lx\n", xei->id.all, *status );

    return 0;
}


#include "tgr.h"

s8 tgr_open(int* fd, const char* file, int flags, mode_t mode)
{
    fileopen_req req;

    req.flags = flags;
    req.mode = mode;
    req.fd = 0;

    req.fname_ptr = (uint64_t) file;

    const char *p = file;
    while( *p )
		p++;
    req.fname_len = p - file;

    int status = send_req(CE_REQTYPE_FILEOPEN, & req, sizeof(req));

    if (!status)
		*fd = req.fd;

    RETURN(status)
}

s8 tgr_close(int fd)
{
    fileclose_req req;

    req.fd = fd;

    int status = send_req(CE_REQTYPE_FILECLOSE, & req, sizeof(req));

    RETURN(status)
}

/* unimplemented */
s8 tgr_poll(struct pollfd * fds , s32 nfds, s32 timeout)
{
    ERRNO = ENOSYS;
    return -1;
}

s8 tgr_read(int fd, ssize_t *readCount, void* ptr, size_t len)
{
    fileread_req req;

    req.fd = fd;
    req.buf = (uint64_t) ptr;
    req.len = (u64) len;

    int status = send_req(CE_REQTYPE_FILEREAD, & req, sizeof(req));

    if (status)
		*readCount = 0;
    else
		*readCount = (s32)req.len;

    RETURN(status);
}

s8 tgr_write(int fd, ssize_t *wroteCount, const void* ptr, size_t len)
{
    filewrite_req req;

    req.fd = fd;
    req.buf = (uint64_t) ptr;
    req.len = len;

    int status = send_req(CE_REQTYPE_FILEWRITE, & req, sizeof(req));

    if (!status)
		*wroteCount = len;

    RETURN(status)
}

s8 tgr_fstat(int fd, struct stat* st)
{
    filefstat_req req;

    req.fd = fd;
    req.stat_ptr = (uint64_t) st;
    req.stat_len = sizeof(*st);

    int status = send_req(CE_REQTYPE_FILEFSTAT, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_stat(const char* file, struct stat* st)
{
    filestat_req req;

    req.fname_ptr = (uint64_t) file;

    const char *p = file;
    while( *p )
		p++;
    req.fname_len = p - file;

    req.stat_ptr = (uint64_t) st;
    req.stat_len = sizeof(*st);

    int status = send_req(CE_REQTYPE_FILESTAT, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_lseek(int fd, off_t * offset, int whence)
{
    filelseek_req req;

    req.fd = fd;
    req.offset = (uint64_t) *offset;
    req.whence = whence;

    int status = send_req(CE_REQTYPE_FILELSEEK, & req, sizeof(req));

    if(!status)
		*offset = req.offset;

    RETURN(status)
}

s8 tgr_link(const char* existing, const char* link)
{
    link_req req;

    const char *p = existing;
    while( *p )
		p++;
    req.oldpath_ptr = (uint64_t)existing;
    req.oldpath_len = (uint64_t)(p - existing);

    p = link;
    while( *p )
		p++;
    req.newpath_ptr = (uint64_t)link;
    req.newpath_len = (uint64_t)(p - link);

    int status = send_req(CE_REQTYPE_LINK, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_symlink(const char* path1, const char* path2)
{
    symlink_req req;

    const char *p = path1;
    while( *p )
		p++;
    req.oldpath_ptr = (uint64_t)path1;
    req.oldpath_len = (uint64_t)(p - path1);

    p = path2;
    while( *p )
		p++;
    req.newpath_ptr = (uint64_t)path2;
    req.newpath_len = (uint64_t)(p - path2);

    int status = send_req(CE_REQTYPE_SYMLINK, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_readlink (const char *path, char *buf, size_t bufsize)
{
    readlink_req req;

    const char *p = path;
    while( *p )
		p++;
    req.path_ptr = (uint64_t)path;
    req.path_len = (uint64_t)(p - path);

    req.buf_ptr = (uint64_t)buf;
    req.size = (uint64_t)bufsize;

    int status = send_req(CE_REQTYPE_READLINK, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_unlink(const char* name)
{
    unlink_req req;

    const char *p = name;
    while( *p )
		p++;
    req.path_ptr = (uint64_t)name;
    req.path_len = (uint64_t)(p - name);

    int status = send_req(CE_REQTYPE_UNLINK, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_chown(const char* path, uid_t owner, gid_t group)
{
    chown_req req;

    const char *p = path;
    while( *p )
		p++;
    req.path_ptr = (uint64_t)path;
    req.path_len = (uint64_t)(p - path);
    req.owner = (uint64_t)owner;
    req.group = (uint64_t)group;

    int status = send_req(CE_REQTYPE_CHOWN, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_chmod(const char* path, mode_t mode)
{
    chmod_req req;

    const char *p = path;
    while( *p )
		p++;
    req.path_ptr = (uint64_t)path;
    req.path_len = (uint64_t)(p - path);
    req.mode = (uint64_t)mode;

    int status = send_req(CE_REQTYPE_CHMOD, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_mkdir(const char *path, mode_t mode)
{
    mkdir_req req;

    const char *p = path;
    while( *p )
		p++;
    req.path_ptr = (uint64_t)path;
    req.path_len = (uint64_t)(p - path);

    req.mode = (uint64_t)mode;

    int status = send_req(CE_REQTYPE_MKDIR, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_rmdir(const char *path)
{
    rmdir_req req;

    const char *p = path;
    while( *p )
		p++;
    req.path_ptr = (uint64_t)path;
    req.path_len = (uint64_t)(p - path);

    int status = send_req(CE_REQTYPE_RMDIR, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_chdir(const char* path)
{
    chdir_req req;

    const char *p = path;
    while( *p )
		p++;
    req.path_ptr = (uint64_t)path;
    req.path_len = (uint64_t)(p - path);

    int status = send_req(CE_REQTYPE_CHDIR, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_getcwd(char* buf, size_t bufSize)
{
    getcwd_req req;

    req.buf_ptr = (uint64_t)buf;
    req.size = bufSize;

    int status = send_req(CE_REQTYPE_GETCWD, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_isatty(int fd)
{
    isatty_req req;

    req.fd = fd;

    int status = send_req(CE_REQTYPE_ISATTY, & req, sizeof(req));

    RETURN(status)
}


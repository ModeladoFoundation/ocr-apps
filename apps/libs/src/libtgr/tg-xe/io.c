#include <errno.h>

#include "tgr.h"

s8 tgr_open(u64* fd, const char* file, s32 flags, s32 mode)
{
    struct {
        uint64_t fname_ptr; // in
        uint64_t fname_len; // in (len of fname not including terminal '\0')
        uint64_t flags;     // in
        uint64_t mode;      // in
        uint64_t fd;        // out
    } req;
    req.flags = flags;
    req.mode = mode;
    req.fd = 0;

    req.fname_ptr = (uint64_t) file;

    const char *p = file;
    while( *p ) p++;
    req.fname_len = p - file;

    int status = send_req(CE_REQTYPE_FILEOPEN, & req, sizeof(req));

    if (!status) *fd = req.fd;

    RETURN(status)
}

s8 tgr_pipe(u64 pipefd[2])
{
    struct {
        uint64_t pipefd_read;  // out
        uint64_t pipefd_write; // out
    } req;

    req.pipefd_read = pipefd[0];
    req.pipefd_write = pipefd[1];

    int status = send_req(CE_REQTYPE_PIPE, & req, sizeof(req));

    if (!status) {
        pipefd[0] = req.pipefd_read;
        pipefd[1] = req.pipefd_write;
    }

    RETURN(status)
}

/* unimplemented */
s8 tgr_poll(struct pollfd * fds , s32 nfds, s32 timeout)
{
    ERRNO = ENOSYS;
    return -1;
}

s8 tgr_read(u64 fd, s32 *readCount, char* ptr, u32 len)
{
    struct {
        uint64_t fd;        // in
        uint64_t buf;       // in
        uint64_t len;       // in / out
    } req;
    req.fd = fd;
    req.buf = (uint64_t) ptr;
    req.len = (u64) len;

    int status = send_req(CE_REQTYPE_FILEREAD, & req, sizeof(req));

    if (status) *readCount = 0;
    else *readCount = (s32)req.len;

    RETURN(status);
}

s8 tgr_write(u64 fd, s32 *wroteCount, const char* ptr, s32 len)
{
    struct {
        uint64_t fd;        // in
        uint64_t buf;       // in
        uint64_t len;       // in / out
    } req;
    req.fd = fd;
    req.buf = (uint64_t) ptr;
    req.len = len;

    int status = send_req(CE_REQTYPE_FILEWRITE, & req, sizeof(req));

    if (!status) *wroteCount = len;

    RETURN(status)
}

s8 tgr_close(u64 fd)
{
    struct {
        uint64_t fd; // in
    } req;
    req.fd = fd;

    int status = send_req(CE_REQTYPE_FILECLOSE, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_isatty(u64 file)
{
    struct {
        uint64_t fd; // in
    } req;
    req.fd = file;

    int status = send_req(CE_REQTYPE_ISATTY, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_fstat(u64 fd, struct stat* st)
{
    struct {
        uint64_t fd;        // in
        uint64_t stat_ptr;  // in
        uint64_t stat_len;  // in (paranoia)
    } req;
    req.fd = fd;
    req.stat_ptr = (uint64_t) st;
    req.stat_len = sizeof(*st);

    int status = send_req(CE_REQTYPE_FILEFSTAT, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_stat(const char* file, struct stat* st)
{
    struct {
        uint64_t fname_ptr; // in
        uint64_t fname_len; // in (len of fname not including terminal '\0')
        uint64_t stat_ptr;  // in
        uint64_t stat_len;  // in (paranoia)
    } req;
    req.fname_ptr = (uint64_t) file;

    const char *p = file;
    while( *p ) p++;
    req.fname_len = p - file;

    req.stat_ptr = (uint64_t) st;
    req.stat_len = sizeof(*st);

    int status = send_req(CE_REQTYPE_FILESTAT, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_lseek(u64 fd, s64 * offset, s32 whence)
{
    struct {
        uint64_t fd;     // in
        uint64_t offset; // in/out
        uint64_t whence; // in
    } req;
    req.fd = fd;
    req.offset = (uint64_t) *offset;
    req.whence = whence;

    int status = send_req(CE_REQTYPE_FILELSEEK, & req, sizeof(req));

    if(!status) *offset = req.offset;

    RETURN(status)
}

s8 tgr_link(const char* existing, const char* link)
{
    struct {
        uint64_t oldpath_ptr;        // in
        uint64_t oldpath_len;        // in (len not including terminal '\0')
        uint64_t newpath_ptr;        // in
        uint64_t newpath_len;        // in (len not including terminal '\0')
    } req;

    const char *p = existing;
    while( *p ) p++;

    req.oldpath_ptr = (uint64_t)existing;
    req.oldpath_len = (uint64_t)(p - existing);

    p = link;
    while( *p ) p++;

    req.newpath_ptr = (uint64_t)link;
    req.newpath_len = (uint64_t)(p - link);

    int status = send_req(CE_REQTYPE_LINK, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_symlink(const char* path1, const char* path2)
{
    struct {
        uint64_t oldpath_ptr;        // in
        uint64_t oldpath_len;        // in (len not including terminal '\0')
        uint64_t newpath_ptr;        // in
        uint64_t newpath_len;        // in (len not including terminal '\0')
    } req;

    const char *p = path1;
    while( *p ) p++;

    req.oldpath_ptr = (uint64_t)path1;
    req.oldpath_len = (uint64_t)(p - path1);

    p = path2;
    while( *p ) p++;

    req.newpath_ptr = (uint64_t)path2;
    req.newpath_len = (uint64_t)(p - path2);

    int status = send_req(CE_REQTYPE_SYMLINK, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_readlink (const char *path, char *buf, size_t bufsize)
{
    struct {
        uint64_t path_ptr;        // in
        uint64_t path_len;        // in (len not including terminal '\0')
        uint64_t buf_ptr;         // in
        uint64_t size;            // in
    } req;

    const char *p = path;
    while( *p ) p++;

    req.path_ptr = (uint64_t)path;
    req.path_len = (uint64_t)(p - path);

    req.buf_ptr = (uint64_t)buf;
    req.size = (uint64_t)bufsize;

    int status = send_req(CE_REQTYPE_READLINK, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_unlink(const char* name)
{
    struct {
        uint64_t path_ptr;        // in
        uint64_t path_len;        // in (len not including terminal '\0')
    } req;

    const char *p = name;
    while( *p ) p++;

    req.path_ptr = (uint64_t)name;
    req.path_len = (uint64_t)(p - name);

    int status = send_req(CE_REQTYPE_UNLINK, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_chown(const char* path, uid_t owner, gid_t group)
{
    struct {
        uint64_t path_ptr;        // in
        uint64_t path_len;        // in (len not including terminal '\0')
        uint64_t owner;           // in
        uint64_t group;           // in
    } req;

    const char *p = path;
    while( *p ) p++;

    req.path_ptr = (uint64_t)path;
    req.path_len = (uint64_t)(p - path);
    req.owner = (uint64_t)owner;
    req.group = (uint64_t)group;

    int status = send_req(CE_REQTYPE_CHOWN, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_chmod(const char* path, mode_t mode)
{
    struct {
        uint64_t path_ptr;        // in
        uint64_t path_len;        // in (len not including terminal '\0')
        uint64_t mode;            // in
    } req;

    const char *p = path;
    while( *p ) p++;

    req.path_ptr = (uint64_t)path;
    req.path_len = (uint64_t)(p - path);
    req.mode = (uint64_t)mode;

    int status = send_req(CE_REQTYPE_CHMOD, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_mkdir(const char *path, s32 mode)
{
    struct {
        uint64_t path_ptr;        // in
        uint64_t path_len;        // in (len not including terminal '\0')
        uint64_t mode;            // in
    } req;

    const char *p = path;
    while( *p ) p++;

    req.path_ptr = (uint64_t)path;
    req.path_len = (uint64_t)(p - path);

    req.mode = (uint64_t)mode;

    int status = send_req(CE_REQTYPE_MKDIR, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_chdir(const char* path)
{
    struct {
        uint64_t path_ptr;        // in
        uint64_t path_len;        // in (len not including terminal '\0')
    } req;

    const char *p = path;
    while( *p ) p++;

    req.path_ptr = (uint64_t)path;
    req.path_len = (uint64_t)(p - path);

    int status = send_req(CE_REQTYPE_CHDIR, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_getcwd(char* buf, u64 bufSize)
{
    struct {
        uint64_t buf_ptr;     // in
        uint64_t size;        // in
    } req;
    req.buf_ptr = (uint64_t)buf;
    req.size = bufSize;

    int status = send_req(CE_REQTYPE_GETCWD, & req, sizeof(req));

    RETURN(status)
}

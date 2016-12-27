#ifndef _TGR_H_
#define _TGR_H_

#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/poll.h>

typedef __int8_t    s8;
typedef __uint8_t   u8;
typedef __uint16_t  u16;
typedef __uint32_t  u32;
typedef __int32_t   s32;
typedef __uint64_t  u64;
typedef __int64_t   s64;

#define _NOARGS void

_BEGIN_STD_C

// Return the size of the tgr reent structure. (implementation specific.)
u8 tgr_sizeof_reent(_NOARGS);

// Modify argc & argv. Initialize runtime.
s8 tgr_init(s32 * argc, char ** argv[], char ** env[]);

// kill all XE's (except for calling one). Do any cleanup needed by runtime.
void tgr_fini(void);

/**
 * @brief Opens a file descriptor
 *
 * This call creates a file 'descriptor' for future file operations. This call supports
 * both regular files as well as stdout/stderr/stdin "files".
 *
 * @param[out] fd       Descriptor returned if successful
 * @param[in] file      Name of the file to open
 * @param[in] flags     Flags have the same meaning as for open()
 * @param[in] mode      Mode has the same meaning as for open()
 * @return 0 on success or -1 on failure. 'descriptor' will not
 * be usable if this call fails.
 */
s8 tgr_open(int* fd, const char* file, int flags, mode_t mode);

/**
 * @brief Opens a pipe
 *
 * This creates a unidirectional pipe which can be used for interprocess communication. It returns
 * two descriptors, one of which may be read from, the other of which may be written to.
 *
 * @param[out] pipefd   An array of two descriptors. The first one is for reading, the second for writing.
 * @return 0 on success or -1 on failure. 'descriptor' will not
 * be usable if this call fails.
 */
s8 tgr_pipe(u64 pipefd[2]);

/**
 * @brief Waits for an event on a fd
 *
 * This waits for one of a set of file descriptors to becore ready to perform I/O.
 *
 * @param[in,out] fds      Array of pollfd structs describing what to poll for. Also holds results.
 * @param[in]     nfds     The number of pollfd structs
 * @param[in]     timeout  milliseconds to block waiting for a fd to become ready. May be 0.
 * @return 0 on success or -1 on failure.
 */
s8 tgr_poll(struct pollfd * fds , s32 nfds, s32 timeout);


/**
 * @brief Reads from a file
 *
 * @param[in]  fd             Descriptor for the file to read from
 * @param[out] readCount      Number of bytes read from the file
 * @param[out] ptr            Buffer to read the file into
 * @param[in]  len            Maximum number of bytes to read
 * @return 0 on success or -1 on failure. 'readCount' will not
 * be accurate if this call fails.
 */
s8 tgr_read(int fd, ssize_t* readCount, void* ptr, size_t len);

/**
 * @brief Writes to a file
 *
 * This call writes data to an open file.
 *
 * @param[in]  fd             Descriptor for the file to write to
 * @param[out] wroteCount     Number of bytes written to the file
 * @param[out] ptr            Buffer to read the data to write from
 * @param[in]  len            Maximum number of bytes to write
 * @return 0 on success or -1 on failure. 'wroteCount' will not
 * be accurate if this call fails.
 */
s8 tgr_write(int fd, ssize_t* wroteCount, const void* ptr, size_t len);

/**
 * @brief Closes an open file
 *
 * This call closes an open file.
 * @note A subsequent open on the same file may return a different descriptor. Similarly,
 * the same descriptor may be reused for a different file. In other words, the file
 * descriptor is only unique for a file while that file is open.
 *
 * @param[in]  fd         Descriptor for the file to close
 * @return 0 on success or -1 on failure.
 */
s8 tgr_close(int fd);

/**
 * @brief Obtains information on an open file
 *
 * This call gets information about a file for which a descriptor is known (open file).
 *
 * @param[in]  fd         Descriptor for the file
 * @param[out] st         Structure containing information about the file
 * @return 0 on success or -1 on failure. 'st' will not have
 * reliable data if this call fails.
 */
s8 tgr_fstat(int fd, struct stat* st);

/**
 * @brief Obtains information on a closed file
 *
 * This call gets information about a file for which there is no descriptor.
 *
 * @param[in]  file       Name of the file to get information about
 * @param[out] st         Structure containing information about the file
 * @return 0 on success or -1 on failure. 'st' will not have
 * reliable data if this call fails.
 */
s8 tgr_stat(const char* file, struct stat* st);

/**
 * @brief Seeks in an open file
 *
 * This call moves the "pointer" in an open file.
 *
 * @param[in]  fd        Descriptor of the file to seek into
 * @param[in]  offset    Offset to seek (depends on whence)
 * @param[in]  whence    Seek mode to use
 * @return resulting pointer offset from begining of file on success or
 * -1 on failure.
 */
s8 tgr_lseek(int fd, off_t* offset, int whence);

/**
 * @brief Links 'link' with the file pointed to by 'existing'
 *
 * @param[in]  existing       Path to an existing file
 * @param[in]  new            New directory entry (path) to create referring to 'existing'
 * @return 0 on success or -1 on failure.
 */
s8 tgr_link(const char* existing, const char* link);

/**
 * @brief Unlinks (deletes) a file
 *
 * @todo Define the behavior for various types of "files" (file, directory, symlink, etc.)
 * @param[in]  name           Path to an existing file to delete
 * @return 0 on success or -1 on falure.
 */
s8 tgr_unlink(const char* name);

/**
 * @brief Creates a symlink
 *
 * This call creates a symbolic link called 'path2' that contains the string pointed to by 'path1'
 * ('path2' is the name of the symbolic link created, 'path1' is the string contained in the symbolic link)
 *
 * @param[in]  path1          Path to link to
 * @param[in]  path2          Path of link
 * @return 0 on success or -1 on falure.
 */
s8 tgr_symlink(const char* path1, const char* path2);

/**
 * @brief Change owner/group information on a file.
 *
 * @param[in]  path           Path to change the owner/group of
 * @param[in]  owner          Owner to set for 'path'
 * @param[in]  group          Group to set for 'path'
 * @return 0 on success or -1 on falure.
 */
s8 tgr_chown(const char* path, uid_t owner, gid_t group);

/**
 * @brief Change permissions for a file
 *
 * @param[in]  path           Path to change the mode of
 * @param[in]  mode           Permission/mode for the file
 * @return 0 on success or -1 on falure.
 */
s8 tgr_chmod(const char* path, mode_t mode);

/**
 * @brief Change the current working directory
 *
 * @param[in]  path           Path to the directory to change to
 * @return 0 on success or -1 on falure.
 */
s8 tgr_chdir(const char* path);

/**
 * @brief Get the current working direction
 *
 * @param[out] buf            Buffer to store the path information in
 * @param[in]  bufSize        Size of 'buf' in bytes
 * @return 0 on success or -1 on falure. If return value is -1
 * then the data in buf is undefined.
 */
s8 tgr_getcwd(char* buf, size_t bufSize);

/**
 * @brief Request the creation of a data-block
 *
 * On successful allocation, the returned memory location will
 * be 8 byte aligned.
 *
 * @param addr      On successful creation, contains the 64 bit address to use to access the data-block
 * @param len       Size in bytes of the block to allocate.
 *
 * @return a status code on failure or 0 on success:
 *     + 0: successful
 *     + ENXIO:  location does not exist
 *     + ENOMEM: allocation failed because of insufficient memory or too constraining constraints
 *     + EINVAL: invalid arguments (flags or something else)
 *     + EBUSY : the agent that is needed to process this request is busy. Retry is possible.
 *     + EPERM : trying to allocate in an area of memory that is not allowed (such as another core's SPAD)
 */
s8 tgr_mmap(void** addr, size_t len);

/**
 * Request for the freeing of a data-block
 *
 * Accessing the data-block by pointer after it has been freed is undefined.
 *
 * @param addr      Contains the 64 bit address of the data-block
 * @param len       Size in bytes of the block to be freed
 *
 * @return A status code
 *      - 0: successful
 *      - EPERM: data-block cannot be freed because it was already freed.
 *      - EINVAL: addr does not refer to a valid data-block or len is invalid.
 */
s8 tgr_munmap(void* addr, size_t len);

s64 tgr_getpagesize(_NOARGS);

s8 tgr_isatty (int fd);
s8 tgr_readlink (const char *path, char *buf, size_t bufsize);
s8 tgr_mkdir(const char *path, mode_t mode);
s8 tgr_rmdir(const char *path);
s8 tgr_gettimeofday (struct timeval* ptimeval, void* ptimezone);
s8 tgr_gethostname (char* hostname, size_t maxlen);

void tgr_exit (s64 retval) __attribute__((noreturn));
s8 tgr_times (struct tms* buf, clock_t* ticks);

s8 tgr_clone (s64 (*fn)(void *), void *arg, pid_t* pid);
s8 tgr_kill (pid_t pid, s32 sig);
s8 tgr_killall ();
s8 tgr_cancel_pid(pid_t pid, void (*cancel_fn)(void));
s8 tgr_waitpid (pid_t pid, s64 *status, s8 block);
s8 tgr_detach (pid_t pid);
s8 tgr_waitall (_NOARGS);

s8 tgr_resume (pid_t pid);
s8 tgr_queue_resumes (s8 enable);
s8 tgr_suspend (const struct timespec* abstime);

pid_t tgr_getpid (_NOARGS);
pid_t tgr_getppid (_NOARGS);


_END_STD_C

#endif // _TGR_H_

#ifndef _OCR_H_
#define _OCR_H_

#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
//#include <dirent.h>

typedef __uint8_t   u8;
typedef __uint16_t  u16;
typedef __uint32_t  u32;
typedef __int32_t   s32;
typedef __uint64_t  u64;
typedef __int64_t   s64;
typedef __intptr_t  ocrGuid_t;
typedef enum {
    NO_ALLOC = 0 /**< No allocation is possible with the data block */
} ocrInDbAllocator_t;

#define NULL_GUID ((ocrGuid_t)0x0)

#define _NOARGS void
/**
 * @brief Data-structure holding configuration elements for the runtime
 *
 * Members of the struct can be filled by calling ocrParseArgs or be manually set.
 */
typedef struct _ocrConfig_t {
    int userArgc;
    char ** userArgv;
    const char * iniFile;
} ocrConfig_t;

/**
 * @brief Opens a file handle
 *
 * This call returns a handle 'handle' for future file operations. This call supports
 * both regular files as well as stdout/stderr/stdin "files".
 *
 * @param[in]  legacyContext  Legacy context for this call. Since this call maintains state across
 *                            invocations, it implies a legacy context
 * @param[out] handle         Handle returned if successful
 * @param[in] file            Name of the file to open
 * @param[in] flags           Flags have the same meaning as for open()
 * @param[in] mode            Mode has the same meaning as for open()
 * @return 0 on success or a non-zero error code on failure. 'handle' will not
 * be usable if this call returns a non-zero value
 */
u8 ocrUSalOpen(ocrGuid_t legacyContext, ocrGuid_t* handle, const char* file, s32 flags, s32 mode);

/**
 * @brief Reads from a file
 *
 * This call reads data from an open file.
 *
 * @param[in]  legacyContext  Legacy context for this call. Since this call maintains state across
 *                            invocations, it implies a legacy context
 * @param[in]  handle         Handle for the file to read from (returned by ocrUSalOpen())
 * @param[out] readCount      Number of bytes read from the file
 * @param[out] ptr            Buffer to read the file into
 * @param[in] len             Maximum number of bytes to read
 * @return 0 on success or a non-zero error code on failure. 'readCount' will not
 * be accurate if this call returns a non-zero value
 */
u8 ocrUSalRead(ocrGuid_t legacyContext, ocrGuid_t handle, s32 *readCount, char* ptr, s32 len);

/**
 * @brief Writes to a file
 *
 * This call writes data to an open file.
 *
 * @param[in]  legacyContext  Legacy context for this call. Since this call maintains state across
 *                            invocations, it implies a legacy context
 * @param[in]  handle         Handle for the file to write to (returned by ocrUSalOpen())
 * @param[out] wroteCount     Number of bytes written to the file
 * @param[out] ptr            Buffer to read the data to write from
 * @param[in] len             Maximum number of bytes to write
 * @return 0 on success or a non-zero error code on failure. 'wroteCount' will not
 * be accurate if this call returns a non-zero value
 */
u8 ocrUSalWrite(ocrGuid_t legacyContext, ocrGuid_t handle, s32 *wroteCount, const char* ptr, s32 len);

/**
 * @brief Closes an open file
 *
 * This call closes an open file.
 * @note A subsequent open on the same file may return a different handle. Similarly,
 * the same handle may be reused for a different file. In other words, the file
 * handle is only unique for a file while that file is open.
 *
 * @param[in]  legacyContext  Legacy context for this call. Since this call maintains state across
 *                            invocations, it implies a legacy context
 * @param[in]  handle         Handle for the file to close (returned by ocrUSalOpen())
 * @return 0 on success or a non-zero error code on failure.
 */
u8 ocrUSalClose(ocrGuid_t legacyContext, ocrGuid_t handle);

/**
 * @brief Obtains information on an open file
 *
 * This call gets information about a file for which a handle is known (open file).
 *
 * @param[in]  legacyContext  Legacy context for this call. Since this call maintains state across
 *                            invocations, it implies a legacy context
 * @param[in]  handle         Handle for the file (returned by ocrUSalOpen())
 * @param[out] st             Structure containing information about the file
 * @return 0 on success or a non-zero error code on failure. 'st' will not have
 * reliable data if a non-zero value is returned.
 */
u8 ocrUSalFStat(ocrGuid_t legacyContext, ocrGuid_t handle, struct stat* st);

/**
 * @brief Obtains information on a closed file
 *
 * This call gets information about a file for which there is no handle.
 *
 * @param[in]  legacyContext  Legacy context for this call. Since this call maintains state across
 *                            invocations, it implies a legacy context
 * @param[in]  file           Name of the file to get information about
 * @param[out] st             Structure containing information about the file
 * @return 0 on success or a non-zero error code on failure. 'st' will not have
 * reliable data if a non-zero value is returned.
 */
u8 ocrUSalStat(ocrGuid_t legacyContext, const char* file, struct stat* st);

/**
 * @brief Seeks in an open file
 *
 * This call moves the "pointer" in an open file.
 * Supported modes for 'whence' are:
 *     - SEEK_CUR
 *     - SEEK_SET
 *     - SEEK_END
 * @todo Do we need to support SEEK_DATA and SEEK_HOLE and seeking
 * past the end of a file?
 *
 * @param[in]  legacyContext  Legacy context for this call. Since this call maintains state across
 *                            invocations, it implies a legacy context
 * @param[in]  handle         Handle of the file to seek into
 * @param[in]  offset         Offset to seek (depends on whence)
 * @param[in]  whence         Seek mode to use
 * @return 0 on success or a non-zero error code on failure.
 */
s64 ocrUSalLseek(ocrGuid_t legacyContext, ocrGuid_t handle, s64 offset, s32 whence);

/**
 * @brief Links 'link' with the file pointed to by 'existing'
 *
 * @todo Define the behavior for various types of "files" (file, directory, symlink, etc.)
 * @param[in]  legacyContext  Legacy context for this call. Since this call maintains state across
 *                            invocations, it implies a legacy context
 * @param[in]  existing       Path to an existing file
 * @param[in]  new            New directory entry (path) to create referring to 'existing'
 * @return 0 on success or a non-zero error code
 *
 * @todo Currently not supported
 */
u8 ocrUSalLink(ocrGuid_t legacyContext, const char* existing, const char* link);

/**
 * @brief Unlinks (deletes) a file
 *
 * @todo Define the behavior for various types of "files" (file, directory, symlink, etc.)
 * @param[in]  legacyContext  Legacy context for this call. Since this call maintains state across
 *                            invocations, it implies a legacy context
 * @param[in]  name           Path to an existing file to delete
 * @return 0 on success or a non-zero error code
 *
 * @todo Currently not supported
 */
u8 ocrUSalUnlink(ocrGuid_t legacyContext, const char* name);

/**
 * @brief Creates a symlink
 *
 * This call creates a symbolic link called 'path2' that contains the string pointed to by 'path1'
 * ('path2' is the name of the symbolic link created, 'path1' is the string contained in the symbolic link)
 *
 * @param[in]  legacyContext  Legacy context for this call. Since this call maintains state across
 *                            invocations, it implies a legacy context
 * @param[in]  path1          Path to link to
 * @param[in]  path2          Path of link
 * @return 0 on success or a non-zero error code
 *
 * @todo Currently not supported
 */
u8 ocrUSalSymlink(ocrGuid_t legacyContext, const char* path1, const char* path2);

/**
 * @brief Change owner/group information on a file.
 *
 *
 * @param[in]  legacyContext  Legacy context for this call. Since this call maintains state across
 *                            invocations, it implies a legacy context
 * @param[in]  path           Path to change the owner/group of
 * @param[in]  owner          Owner to set for 'path'
 * @param[in]  group          Group to set for 'path'
 * @return 0 on success or a non-zero error code
 *
 * @todo Currently not supported
 */
u8 ocrUSalChown(ocrGuid_t legacyContext, const char* path, uid_t owner, gid_t group);

/**
 * @brief Change permissions for a file
 *
 *
 * @param[in]  legacyContext  Legacy context for this call. Since this call maintains state across
 *                            invocations, it implies a legacy context
 * @param[in]  path           Path to change the mode of
 * @param[in]  mode           Permission/mode for the file
 * @return 0 on success or a non-zero error code
 *
 * @todo Currently not supported
 */
u8 ocrUSalChmod(ocrGuid_t legacyContext, const char* path, mode_t mode);

/**
 * @brief Change the current working directory
 *
 *
 * @param[in]  legacyContext  Legacy context for this call. Since this call maintains state across
 *                            invocations, it implies a legacy context
 * @param[in]  path           Path to the directory to change to
 * @return 0 on success or a non-zero error code
 *
 * @todo Currently not supported
 */
u8 ocrUSalChdir(ocrGuid_t legacyContext, const char* path);

/**
 * @brief Get the current working direction
 *
 *
 * @param[in]  legacyContext  Legacy context for this call. Since this call maintains state across
 *                            invocations, it implies a legacy context
 * @param[out] buf            Buffer to store the path information in
 * @param[in]  bufSize        Size of 'buf' in bytes
 * @return 0 on success or a non-zero error code
 *
 * @todo Currently not supported
 */
u8 ocrUSalGetcwd(ocrGuid_t legacyContext, char* buf, u64 bufSize);

/**
 * @brief Parses the arguments passed to main and extracts the
 * relevant information to initialize OCR
 *
 * This should be called prior to ocrInit() to populate the
 * #ocrConfig_t variable needed by ocrInit().
 *
 * @param[in] argc           The number of elements in argv
 * @param[in] argv           Array of char * argumetns.
 * @param[in,out] ocrConfig  Pointer to an ocrConfig ocrParseArgs will populate. ocrConfig
 *                           needs to have already been allocated
 */
void ocrParseArgs(int argc, const char* argv[], ocrConfig_t * ocrConfig);

/**
 * @brief Bring up the OCR runtime
 *
 * This function needs to be called to bring up the runtime. It
 * should be called once for each runtime that needs to be brought
 * up.
 *
 * @param[out] legacyContext Returns the ID of the legacy context
 *                           created
 * @param[in]  ocrConfig     Configuration parameters to bring up the
 *                           runtime
 */
void ocrInit(ocrGuid_t *legacyContext, ocrConfig_t * ocrConfig);

/**
 * @brief Prepares to tear down the OCR runtime
 *
 * This call prepares the runtime to be torn-down. This call
 * will only return after the OCR program completes (ie: after
 * the program calls ocrShutdown()).
 *
 * @param[in] legacyContext Legacy context to finalize. This
 *                          value is obtained from ocrInit()
 * @return the status code of the OCR program:
 *      - 0: clean shutdown, no errors
 *      - non-zero: user provided error code to ocrAbort()
 */
u8 ocrFinalize(ocrGuid_t legacyContext);

/**
 * @brief Launch an OCR "procedure" from a legacy sequential section of code
 *
 * This call requires the sequential code to have already called ocrInit().
 * This call will *NOT* trigger ocrFinalize(). This call is non-blocking.
 *
 * @param[out] handle             Handle to use in ocrBlockProgressUntilComplete
 * @param[in]  finishEdtTemplate  Template of the EDT to execute. This must be
 *                                the template of a finish EDT
 * @param[in]  paramc             Number of parameters to pass to the EDT
 * @param[in]  paramv             Array of parameters to pass. Will be
 *                                passed by value
 * @param[in]  depc               Number of data-blocks to pass in
 * @param[in]  depv               GUID for these data-blocks (created using
 *                                ocrDbCreate())
 * @param[in]  legacyContext      Legacy context this is called from. This needs
 *                                to be remembered from ocrInit().
 * @return 0 on success or an error code on failure (from ocr-errors.h)
 */
u8 ocrSpawnOCR(ocrGuid_t* handle, ocrGuid_t finishEdtTemplate, u64 paramc, u64* paramv,
               u64 depc, ocrGuid_t depv, ocrGuid_t legacyContext);

/**
 * @brief Waits for the result of an OCR procedure launched with ocrSpawnOCR
 *
 * This call requires the sequential code to have already called ocrInit().
 * This call will *NOT* trigger a call to ocrFinalize(). This call is (obviously)
 * blocking.
 * @param[in]  handle    Event to wait on (usually 'handle' from ocrSpawnOCR)
 * @param[out] guid      GUID of the data-block returned.
 * @param[out] result    Returns a void* pointer to the data-block
 * @param[out] size      Returns the size of the data-block
 * @return 0 on success or an error code on failure (from ocr-errors.h)
 */
u8 ocrLegacyBlockProgress(ocrGuid_t handle, ocrGuid_t* guid, void** result, u64* size);

/**
 * @brief Restores a legacy context and blocks until the legacy code is complete. This
 * will restore the heap and globals. The EDT's stack will be used as the stack for the
 * legacy code.
 *
 * @param[in] func   Function to execute. The signature is generic and the
 *                   programmer is responsible for writing the appropriate
 *                   wrapper needed
 * @return 0 on success or an error code on failure (from ocr-errors.h)
 */
u8 ocrLaunchAndBlockOnLegacy(void* (*func)(void* arg));

/**
 * @brief Request the creation of a data-block
 *
 * On successful allocation, the returned memory location will
 * be 8 byte aligned. ocrDbCreate also implicitly registers the calling EDT
 * with the data-block.
 *
 * @param db        On successful allocation, contains the GUID for the newly allocated data-block.
 *                  Will be NULL if the call fails
 * @param addr      On successful creation, contains the 64 bit address to use to access the data-block
 * @param len       Size in bytes of the block to allocate.
 * @param flags     If OCR_DB_CREATE_NO_ACQUIRE, the DB will be created but not
 *          acquired (addr will be NULL). Future use are reserved
 * @param affinity  GUID to indicate the affinity of this created data-block
 * @param location  Used as input to determine where to allocate memory and on successful allocation
 *                  will contain the location of the actual allocation. Can be NULL
 * @param allocator Allocator to use to allocate within the data-block. Can be NULL
 *
 * @return a status code on failure or 0 on success:
 *     + 0: successful
 *     + ENXIO:  location does not exist
 *     + ENOMEM: allocation failed because of insufficient memory or too constraining constraints
 *     + EINVAL: invalid arguments (flags or something else)
 *     + EBUSY : the agent that is needed to process this request is busy. Retry is possible.
 *     + EPERM : trying to allocate in an area of memory that is not allowed (such as another core's SPAD)
 *
 * @warning The 'flag' and 'allocator' parameter are ignored and the 'location' parameter
 *          is not fully implemented
 */
u8 ocrDbCreate(ocrGuid_t *db, u64* addr, u64 len, u16 flags, ocrGuid_t affinity, ocrInDbAllocator_t allocator);

/**
 * @brief Request for the freeing of a data-block
 *
 * The EDT does not need to be registered with the data-block to destroy/free it.
 * ocrDbDestroy will request freeing of the DB but the DB will only be freed
 * once all other EDTs registered to it unregister or die.
 *
 * Note that if the EDT is registered with the DB, this call implicitly
 * unregisters the EDT from the DB.
 *
 * Once a DB has been marked as 'to-be-destroyed' by this call, the following
 * operations will result in an error:
 *      - registering with the DB (will return EPERM)
 *      - re-allocating the DB (will return EPERM or EACCES if the EDT is not
 *      registered with the DB)
 *      - re-freeing the DB (will return EPERM)
 * The following operations will produce undefined behavior:
 *      - accessing the actual location of the DB (through a pointer)
 *
 * @param db  Used as input to determine the DB to be freed
 * @return A status code
 *      - 0: successful
 *      - EPERM: DB cannot be freed because it was already freed
 *      - EINVAL: db does not refer to a valid data-block
 */
u8 ocrDbDestroy(ocrGuid_t db);

/**
 * @brief Returns an affinity the currently executing EDT is affinitized to
 *
 * An EDT may have multiple affinities. The programmer should rely on
 * ocrAffinityCount() and ocrAffinityGet() to query all affinities.
 *
 * @param[out] affinity    One affinity GUID for the currently executing EDT
 * @return a status code
 *      - 0: successful
 */
u8 ocrAffinityGetCurrent(ocrGuid_t * affinity);

/**
 * @brief Type of memory to associate with a legacy context
 */
typedef enum _ocrLegacyContextMemory_t {
    LEGACY_MEM_NONE = 0x0,
    LEGACY_MEM_STACK,
    LEGACY_MEM_HEAP,
    LEGACY_MEM_BSS,
    LEGACY_MEM_DATA,
    LEGACY_MEM_CONST,
    LEGACY_MEM_TEXT,
    LEGACY_MEM_TLS
} ocrLegacyContextMemory_t;

/**
 * @brief Associate the data-block 'db' with a legacy context
 *
 * This informs the runtime of the memory the legacy context is
 * using. This association will be maintained to allow OCR to still
 * provide some of the benefits of OCR (resilience for example).
 *
 * @param[in] legacyContext  Legacy context ID to modify. This information
 *                           is obtained through the ocrInit() call
 * @param[in] db             GUID of the data-block created to contain
 *                           the chunk of memory. This is obtained using
 *                           ocrDbCreate().
 * @param[in] type           Type of memory. See ocrLegacyContextMemory_t.
 * @return 0 on success or an error code on failure
 */
u8 ocrLegacyContextAssociateMemory(ocrGuid_t legacyContext, ocrGuid_t db, ocrLegacyContextMemory_t type);

/**
 * @brief Dis-associate the data-block 'db' with a legacy context
 *
 * This informs the runtime that a data-block is no longer being
 * used for a particular legacy context.
 *
 * @note This call does not free the data-block, nor does it grant or
 * remove access to it.
 *
 * @param[in] legacyContext  Legacy context ID to modify. This information
 *                           is obtained through the ocrInit() call
 * @param[in] db             GUID of the data-block created to contain
 *                           the chunk of memory. This is obtained using
 *                           ocrDbCreate().
 * @param[in] type           Type of memory. See ocrLegacyContextMemory_t. Must match
 *                           the type passed when associating the DB to the legacy
 *                           context.
 * @return 0 on success or an error code on failure
 */
u8 ocrLegacyContextRemoveMemory(ocrGuid_t legacyContext, ocrGuid_t db, ocrLegacyContextMemory_t type);


void ocrShutdown(void);
void ocrAbort(void);

u8 ocrIsAtty (s32 file);
s64 ocrReadlink (ocrGuid_t legacyContext, const char *path, char *buf, size_t bufsize);
//s32 ocrGetdents (u32 fd, struct dirent *dirp, u32 count);

u8 ocrFork (_NOARGS);
u8 ocrEvecve (char *name, char **argv, char **env);
u8 ocrGetPID (_NOARGS);
u8 ocrKill (s32 pid, s32 sig);
u8 ocrWait (s32 *status);

u8 ocrGetTimeofDay (struct timeval  *ptimeval, void *ptimezone);
u8 ocrTimes (struct tms *buf, clock_t *ticks);

#endif // _OCR_H_

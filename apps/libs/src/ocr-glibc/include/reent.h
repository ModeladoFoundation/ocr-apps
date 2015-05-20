#ifndef _REENT_H_
#define _REENT_H_
/*
 * module structure to hold static OCR related info
 */
struct _reent {
    /*
     * Hold OCR Guid mappings, stack, and context info
     */
    struct _ocr_reent {
        int64_t  stack_Guid;    /* used to store OCR Guid for stack */
        uint64_t stack_start;   /* user stack low addr */
        int64_t  stack_size;    /* user stack size in bytes */
        int64_t  legacyContext; /* used to store OCR Guid for legacy ctx*/
        int64_t * fileGuidMap;  /* used to store fd to OCR Guid map */
        int64_t   fileGuidSize; /* max number of entries in fileGuidMap */
    } _ocr;
    #define OCR_FILE_MAP_MAX    32  /* limits the number of open files */
};

extern struct _reent *_get_reent();
#define _REENT  _get_reent()

#endif // _REENT_H_

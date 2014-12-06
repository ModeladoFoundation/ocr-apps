#ifndef _STDIO_H
#define _STDIO_H

#include <misc.h>
#include <ocr.h>

// Currently, support for one file only
// Absolutely NO synchronization of I/O, use at your own risk

#define EBADF 9
#define FILE u64

#ifndef EOF
# define EOF (-1)
#endif

// A magic value that FSim relies on to know if the file is valid.
// See ss/xe-sim/src/sim-main.c
#define FILE_WRITE_MAGIC 0x5a5a000000000000

static u64 read_fp = 0;
static u64 write_fp = 0;

static inline FILE __attribute__((always_inline)) *fopen(const char* filename, const char* mode) {
    if(mode[0] == 'r') return &read_fp;
    else if(mode[0] == 'w') return &write_fp;
    else return NULL;
}

static inline u32 __attribute__((always_inline)) fclose(FILE *fp) {
    if(fp == NULL) return EBADF;
    else if(fp == &write_fp) {
        // Write the size
        *(u64 *)(BLOB_START) = (*fp)|FILE_WRITE_MAGIC;
        *fp = 0;
    }
    fp = NULL;
    return 0;
}

inline u32 __attribute__((always_inline)) fread(void *ptr, u32 size, u32 nmemb, FILE *fp) {
    hal_memCopy(ptr, (BLOB_START+(*fp)*sizeof(u8)), size*nmemb, 0);
    *fp += size*nmemb;
    ASSERT(*fp <= *(u64 *)(BLOB_START));
    return size*nmemb;
}

inline char* __attribute__((always_inline)) fgets(char *s, int size, FILE *stream) {
    char tmp = 0; int i = 0;
    while(tmp != '\n' && tmp != EOF && --size) {
        tmp = *(char*)stream; stream += sizeof(char);
        if(tmp != EOF) s[i] = tmp;
        ++i;
    }
    s[i] = '\0';
    return s;
}

inline u32 __attribute__((always_inline)) fwrite(const void *ptr, u32 size, u32 nmemb, FILE *fp) {
    hal_memCopy((BLOB_START+sizeof(u64)+(*fp)*sizeof(u8)), ptr, size*nmemb, 0);
    *fp += size*nmemb;
    return size*nmemb;
}

inline u32 __attribute__((always_inline)) ftell(FILE *stream) {
    return (u32)(*stream);
}

#endif //_STDIO_H

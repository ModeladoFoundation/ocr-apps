/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#ifndef MPI_OCR_HASHTABLE_H_
#define MPI_OCR_HASHTABLE_H_

struct _hashtable_entry_struct;

/**
 * @brief hash function to be used for key hashing.
 *
 * @param key         the key to hash
 * @param nbBuckets   number of buckets in the table
 */
typedef u32 (*hashFct)(u64 key, u32 nbBuckets);

/**
 * @brief Function to be used to deallocate remnant
 *        entries on hashtable destruction.
 *
 * @param key       the key of the entry to destroy
 * @param value     the value of the entry to destroy
 */
typedef void (*deallocFct)(u64 key, void * value, void * deallocParam);

typedef struct _hashtable_s {
    u32 nbBuckets;
    struct _hashtable_entry_struct ** table;
    /** @brief hashing function to determine bucket. */
    hashFct hashing;
} hashtable_t;

void * mpiHashtableGet(hashtable_t * hashtable, u64 key);
bool mpiHashtablePut(hashtable_t * hashtable, u64 key, void * value);
void * mpiHashtableTryPut(hashtable_t * hashtable, u64 key, void * value);
bool mpiHashtableRemove(hashtable_t * hashtable, u64 key, void ** value);
void mpiDestructHashtable(hashtable_t * hashtable, deallocFct entryDeallocator,
                        void * deallocatorParam);
hashtable_t * mpiNewHashtable(u32 nbBuckets, hashFct hashing);
hashtable_t * mpiNewHashtableModulo(u32 nbBuckets);


#endif /* MPI_OCR_HASHTABLE_H_ */

/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#include "mpi_ocr.h"
#include "mpi_ocr_hashtable.h"
#include <ocr.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>


#define HASH_TABLE_SIZE 8

/* Hashtable entries */
typedef struct _hashtable_entry_struct {
    u64 key;
    void * value;
    struct _hashtable_entry_struct * nxt;
} hashtable_entry;


/**
 * @brief Print the hashtable entries.
 */
static void printHashtable(hashtable_t * hashtable)
{
    // go over each bucket and display entries
    u32 i = 0;
    while(i < hashtable->nbBuckets) {
        hashtable_entry * bucketHead = (hashtable_entry *)hashtable->table[i];
        printf("Bucket %d:\n", i);
        u32 j=0;
        while (bucketHead != NULL) {
            hashtable_entry * next = bucketHead->nxt;
            printf("\tEntry #%d: key=%p, value=%p\n", j, bucketHead->key,
                   bucketHead->value); fflush(stdout);
            j++;
            bucketHead = next;
        }
        i++;
    }
}


/**
 * @brief find the entry and the previous entry for 'key'.
 */
static hashtable_entry * hashtableFindEntryAndPrev(hashtable_t * hashtable,
                                                  u64 key,
                                                  hashtable_entry** prev)
{
    u32 bucket = hashtable->hashing(key, hashtable->nbBuckets);
    hashtable_entry * current = (hashtable_entry *)hashtable->table[bucket];
    *prev = current;
    while(current != NULL) {
        if (current->key == key) {
            return current;
        }
        *prev = current;
        current = current->nxt;
    }
    *prev = NULL;
    return NULL;
}

/**
 * @brief find hashtable entry for 'key'.
 */
static hashtable_entry * hashtableFindEntry(hashtable_t * hashtable,
                                           u64 key)
{
    u32 bucket = hashtable->hashing(key, hashtable->nbBuckets);
    hashtable_entry * current = (hashtable_entry *)hashtable->table[bucket];
    while(current != NULL) {
        if (current->key == key) {
            return current;
        }
        current = current->nxt;
    }
    return NULL;
}

/**
 * @brief Initialize the hashtable.
 */
static void hashtableInit(hashtable_t * hashtable, u32 nbBuckets,
                          hashFct hashing)
{
    hashtable_entry ** table = (void*)malloc(nbBuckets*sizeof(hashtable_entry*));

    u32 i;
    for (i=0; i < nbBuckets; i++) {
        table[i] = NULL;
    }
    hashtable->table = table;
    hashtable->nbBuckets = nbBuckets;
    hashtable->hashing = hashing;
}

/**
 * @brief Create a new hashtable instance that uses the specified hashing function.
 */
hashtable_t * mpiNewHashtable(u32 nbBuckets, hashFct hashing)
{
    hashtable_t * hashtable = malloc(sizeof(hashtable_t));
    hashtableInit(hashtable, nbBuckets, hashing);
    return hashtable;
}

/**
 * @brief Destruct the hashtable and all its entries (do not deallocate keys and values pointers).
 */
void mpiDestructHashtable(hashtable_t * hashtable, deallocFct entryDeallocator,
                       void * deallocatorParam)
{
    // go over each bucket and deallocate entries
    u32 i = 0;
    while(i < hashtable->nbBuckets) {
        hashtable_entry * bucketHead = (hashtable_entry *)hashtable->table[i];
        while (bucketHead != NULL) {
            hashtable_entry * next = bucketHead->nxt;
            if (entryDeallocator != NULL) {
                entryDeallocator(bucketHead->key, bucketHead->value, deallocatorParam);
            }
            free(bucketHead);
            bucketHead = next;
        }
        i++;
    }
    free(hashtable->table);
    free(hashtable);
}


/**
 * @brief hash function.
 *
 * This implementation is meant to be used with monotonically generated guids.
 * Modulo more or less ensure buckets' load is balanced.
 */
static u32 hashModulo(u64 key, u32 nbBuckets) {
    return (key % nbBuckets);
}

/**
 * @brief Create a new hashtable.
 */
hashtable_t * mpiNewHashtableModulo(u32 numRanks)
{
    u32 nbBuckets;

    if (numRanks <= HASH_TABLE_SIZE)
        nbBuckets = numRanks;
    else
        nbBuckets = numRanks%HASH_TABLE_SIZE;

    return mpiNewHashtable(nbBuckets, hashModulo);
}


/**
 * @brief get the value associated with a key
 */
void * mpiHashtableGet(hashtable_t * hashtable, u64 key)
{
    hashtable_entry * entry = hashtableFindEntry(hashtable, key);
    return (entry == NULL) ? 0x0: entry->value;
}

/**
 * @brief Attempt to insert the key if absent.
 * Return the current value associated with the key
 * if present in the table, otherwise returns the value
 * passed as parameter.
 */
void * mpiHashtableTryPut(hashtable_t * hashtable, u64 key, void * value)
{
    hashtable_entry * entry = hashtableFindEntry(hashtable, key);
    if (entry == NULL) {
        mpiHashtablePut(hashtable, key, value);
        return value;
    } else {
        return entry->value;
    }
}

/**
 * @brief Put a key associated with a given value in the map.
 */
bool mpiHashtablePut(hashtable_t * hashtable, u64 key, void * value)
{
    u32 bucket = hashtable->hashing(key, hashtable->nbBuckets);
    hashtable_entry * newHead = (hashtable_entry *)malloc(sizeof(hashtable_entry));
    newHead->key = key;
    newHead->value = value;
    hashtable_entry * oldHead = hashtable->table[bucket];
    newHead->nxt = oldHead;
    hashtable->table[bucket] = newHead;

    // debugging!
    //printHashtable(hashtable);

    return true;
}

/**
 * @brief Removes a key from the table.
 * If 'value' is not NULL, fill-in the pointer with the entry's value
 * associated with 'key'.  If the hashtable implementation allows for
 * NULL values entries, check the returned boolean.
 * Returns true if entry has been found and removed.
 */
bool mpiHashtableRemove(hashtable_t * hashtable, u64 key, void ** value) {
    hashtable_entry * prev;
    hashtable_entry * entry = hashtableFindEntryAndPrev(hashtable, key, &prev);
    if (entry != NULL) {
        assert(prev != NULL);
        assert(key == entry->key);
        if (entry == prev) {
            // entry is bucket's head
            u32 bucket = hashtable->hashing(key, hashtable->nbBuckets);
            hashtable->table[bucket] = entry->nxt;
        } else {
            prev->nxt = entry->nxt;
        }
        if (value != NULL) {
            *value = entry->value;
        }
        free(entry);
        return true;
    }
    return false;
}



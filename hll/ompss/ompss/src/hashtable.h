
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "common.h"
#include "dependences.h"
#include "hashtable_decl.h"

#include <string.h>

// FIXME look for an optimum value
#define DEFAULT_TABLE_SIZE 128U

/*! \file hashtable.h
 *  This file implements a hash table specialized to track
 *  data dependences.
 *  Key values are plain memory addresses.
 *  Values are data_dependency_t structures.
 */

/*! \brief Computes an u32 hash from a key.
 *  Keys are plain memory addresses.
 *  This is a very simplistic hash function
 *  which only shifts 4 bits left the address value.
 *  The most significant bits are also discarded
 *  to make it fit into a 32bit value.
 */
static inline u32 hash_funct( key_t key )
{
    return (u32)( (((u64)key) >> 4) & ((u32)~0) );
}

/*! \brief Compares two keys equivalency.
 */
static inline u8 compare_funct( key_t key1, key_t key2 )
{
    return key1 == key2;
}

static inline struct _ht_bucket* newHashBucket( key_t key )
{
    struct _ht_bucket* new_bucket = (struct _ht_bucket*)malloc( sizeof(struct _ht_bucket) +
                                                              + sizeof(value_t) );
    new_bucket->next = NULL;
    new_bucket->key = key;

    initializeDataDependency( &new_bucket->value );
    return new_bucket;
}

static inline void destructHashBucket( struct _ht_bucket** bucket )
{
    struct _ht_bucket* this_bucket = *bucket;
    *bucket = this_bucket->next;

    freeDataDependency( &this_bucket->value );
    free( this_bucket );
}

/* \brief Retrieves a value reference for a given key
 *
 * Search the hash table for a {key,value} pair which matches
 * with a given key. If the pair did not exist, it is created.
 *
 * \param[in] table the table that is going to be queried
 * \param[in] key the key used to perform the search
 * \returns a refrence to the value
 */
static inline value_t* hashTableGet( hash_table_t* table, key_t key )
{
    u32 pos = hash_funct( key ) % table->size;

    struct _ht_bucket* last = NULL;
    struct _ht_bucket* bucket = table->data[pos];
    while( bucket != NULL && !compare_funct( key, bucket->key ) ) {
        last = bucket;
        bucket = bucket->next;
    }

    if( bucket == NULL ) {
        bucket = newHashBucket( key );
        if( last != NULL )
            last->next = bucket;
        else
            table->data[pos] = bucket;
    }
    return &bucket->value;
}

/*! \brief Removes an element from the hash table
 *  Removes a {key,value} pair matching a given key
 *  passed as input.
 */
static inline void hashTableRemove( hash_table_t* table, key_t key )
{
    u32 pos = hash_funct( key ) % table->size;

    struct _ht_bucket* last = NULL;
    struct _ht_bucket* bucket = table->data[pos];
    while( bucket != NULL && !compare_funct( key, bucket->key ) ) {
        last = bucket;
        bucket = bucket->next;
    }

    if( bucket != NULL ) {
        destructHashBucket( &last->next );
    }
}

static inline void newHashTable( hash_table_t* table )
{
    table->data = (struct _ht_bucket**)malloc( DEFAULT_TABLE_SIZE * sizeof( struct _ht_bucket* ) );
    table->size = DEFAULT_TABLE_SIZE;

    // Initialize table to NULL pointer values
    for( u32 b = 0; b < table->size; ++b )
        table->data[b] = NULL;
}

static inline void destructHashTable( hash_table_t* table )
{
    // Destroy internal datastructures memory
    for( u32 p = 0; p < table->size; ++p ) {
        while( table->data[p] != NULL ) {
            destructHashBucket( &table->data[p] );
        }
    }

    // Deallocate table
    if( table->size != 0 ) {
        free( table->data );
    }
    table->size = 0;
}

#endif // HASHTABLE_H


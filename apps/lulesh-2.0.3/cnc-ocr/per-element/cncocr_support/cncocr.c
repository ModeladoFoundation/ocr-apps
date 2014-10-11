#include "cncocr_internal.h"


#include <string.h>
#define CNC_MEMCMP memcmp

#define DEPS_BUCKET 3 /* must be >=3 */

#define CNC_GETTER 1
#define CNC_PUTTER 2

#define SINGLE_ASSIGNMENT_ENFORCED 1
#define SKIP_SINGLE_ASSIGNMENT 0

/* The structure to hold an item in the item collection */
typedef struct ItemCollEntry {
    ocrGuid_t event; /* The event representing the data item. Data will be put through the event when it is satisfied */
    struct ItemCollEntry * volatile nxt; /* The next bucket in the hashtable */
    char creator; /* Who created this entry (could be from a Put or a Get)*/
    unsigned char tag[]; /* Tags are byte arrays, with a known length for each item collection */
} ItemCollectionEntry;



/* Compare byte-by-byte for tag equality */
static bool _equals(u8 *tag1, u8 *tag2, u32 length) {
    return CNC_MEMCMP(tag1, tag2, length) == 0;
}

/* Hash function implementation. Fast and pretty good */
static u64 _hash_function(u8 *str, u32 length) {
    u64 hash = 5381;
    u32 c;
    while (length-- > 0) {
        c = *str++;
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

/* get an entry from the item collection, or create and insert one
 * (atomically) if it doesn't exist
 * The creator parameter CNC_PUTTER/CNC_GETTER ensures that multiple
 * puts are not allowed
 */
static ItemCollectionEntry * _allocateEntryIfAbsent(
        ItemCollectionEntry * volatile * hashmap, unsigned char * tag,
        int length, char creator, bool isSingleAssignment) {
    int index = (_hash_function(tag, length)) % CNC_TABLE_SIZE;
    ItemCollectionEntry * volatile current = hashmap[index];
    ItemCollectionEntry * volatile * currentLocation = &hashmap[index];
    ItemCollectionEntry * volatile head = current;
    ItemCollectionEntry * volatile tail = NULL;

    ItemCollectionEntry * entry = NULL;

    while (1) {
        /* traverse the buckets in the table to get to the last one */
        while (current != tail) {
            if (_equals(current->tag, tag, length)) {
                /* deallocate the entry we eagerly allocated in a previous iteration of the outer while(1) loop */
                if (entry != NULL){
                    ocrEventDestroy(entry->event);
                    free(entry);
                }

                // XXX - PutIfAbsent is kind of broken here if creator is GET
                // but using IDEM events still gives the correct behavior
                if ((current->creator == CNC_PUTTER) && (creator == CNC_PUTTER)) {
                    if (isSingleAssignment) {
                        ASSERT(!"Single assignment rule violated in item collection put");
                    }
                    return NULL;
                }
                return current; /* just return the table entry if it already has the tag */
            }
            current = current->nxt;
        }

        /* allocate a new entry if this is the first time we are going to try and insert a new entry to the end of the bucket list */
        if (entry == NULL) {
            entry = (ItemCollectionEntry *) malloc(sizeof(ItemCollectionEntry)+length);
            entry->creator = creator;
            memcpy(entry->tag, tag, length);
            ocrEventCreate(&(entry->event), OCR_EVENT_IDEM_T, true);
        }
        entry->nxt=head;

        /* try to insert the new entry into the _first_ position in a bucket of the table */
        if (__sync_bool_compare_and_swap(currentLocation, head, entry)) {
            return entry;
        }

        /* CAS failed, which means that someone else inserted the new entry into the table while we were trying to do so, we need to try again */
        current = hashmap[index]; //do not update tail anymore if deletes are inserted.
        tail = head;
        head = current;
    }

    ASSERT(!"Arrived at the end of allocateEntryIfAbsent in DataDriven.c"); /* we should never get here */
    return NULL;
}

/* Putting an item into the hashmap */
bool _cncPut(ocrGuid_t item, unsigned char *tag, int tagLength, ItemCollectionEntry ** hashmap, bool isSingleAssignment) {

    ASSERT(tag != NULL && "Put - ERROR================\n");

    /* allocateEntryIfAbsent checks for multiple puts using the "CNC_PUTTER" parameter */
    ItemCollectionEntry * entry = _allocateEntryIfAbsent(hashmap, tag, tagLength, CNC_PUTTER, isSingleAssignment);

    /* the returned placeholder can be NULL only when isSingleAssignment is false.
       in which case, the item was Put previously, so the current Put returns */
    if(entry == NULL && !isSingleAssignment)
        return false;
    /* Now, we have the correct placeholder (either inserted by us or by a Get function) */

    /* Inserting the item now boils down to just satisfying the event with a data block of the item. */
    ocrEventSatisfy(entry->event, item);

    return true;
}

/* Get GUID from an item tag */
void _cncGet(unsigned char *tag, int tagLength, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, ItemCollectionEntry **hashmap) {
    ItemCollectionEntry * entry = _allocateEntryIfAbsent(hashmap, tag, tagLength, CNC_GETTER, SINGLE_ASSIGNMENT_ENFORCED);
    ocrAddDependence(entry->event, destination, slot, mode);
}

/* Put a singleton item */
bool _cncPutSingleton(ocrGuid_t item, ocrGuid_t collection, bool isSingleAssignment) {
    ocrEventSatisfy(collection, item);
    return true;
}

/* Get GUID for singleton item */
void _cncGetSingleton(ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, ocrGuid_t collection) {
    ocrAddDependence(collection, destination, slot, mode);
}

static ocrGuid_t _shutdownEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocrShutdown();
    return NULL_GUID;
}

void cncAutomaticShutdown(ocrGuid_t doneEvent) {
    ocrGuid_t shutdownEdtGuid, templGuid;
    ocrEdtTemplateCreate(&templGuid, _shutdownEdt, 0, 1);
    ocrEdtCreate(&shutdownEdtGuid, templGuid,
        /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
        /*depc=*/EDT_PARAM_DEF, /*depv=*/&doneEvent,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);
    ocrEdtTemplateDestroy(templGuid);
}

ocrEdtDep_t _cncRangedInputAlloc(u32 n, u32 dims[], size_t itemSize) {
    u32 i, j, k;
    ///////////////////////////////////////
    // Figure out how much memory we need
    ///////////////////////////////////////
    u32 sum = 0, product = 1;
    for (i=0; i<n-1; i++) {
        product *= dims[i];
        sum += sizeof(void*) * product;
    }
    product *= dims[i];
    sum += itemSize * product;
    ///////////////////////////////////////
    // Allocate a datablock
    ///////////////////////////////////////
    ocrEdtDep_t block;
    CNC_CREATE_ITEM(&block.guid, &block.ptr, sum);
    ///////////////////////////////////////
    // Set up the internal pointers
    ///////////////////////////////////////
    if (n > 1) {
        u32 prevDim = 1, currDim = 1, nextDim = dims[0];
        void **ptrs = block.ptr;
        void **current = ptrs;
        void **tail = ptrs + nextDim; // make room for first array
        for (i=1; i<n-1; i++) {
            // Slide the window
            prevDim = currDim;
            currDim = nextDim;
            nextDim = dims[i];
            // One array for each parent
            for (j=0; j<prevDim; j++) {
                // Initialize each current pointer
                for (k=0; k<currDim; k++) {
                    *current = (void*)tail;
                    tail += nextDim; // Make room for new array
                    ++current;
                }
            }
        }
        u8 **itemCurrent = (u8**)current;
        u8 *itemTail = (u8*)tail;
        // Slide the window
        prevDim = currDim;
        currDim = nextDim;
        nextDim = dims[i];
        // One array for each last-level parent
        for (j=0; j<prevDim; j++) {
            // Initialize each current pointer
            for (k=0; k<currDim; k++) {
                *itemCurrent = itemTail;
                itemTail += itemSize * nextDim; // Make room for new items
                ++itemCurrent;
            }
        }
        ASSERT(itemTail == ((u8*)ptrs + sum));
    }
    ///////////////////////////////////////
    // Return the initialized datablock
    ///////////////////////////////////////
    return block;
}

